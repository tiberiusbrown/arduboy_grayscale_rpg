
#include <string.h>
#include <avr/pgmspace.h>

#include "atm_synth.h"

/* #define log_cmd() to oblivion */
#define log_cmd(a,b,c,d)

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

struct atm_libstate atmlib_state;

#define ATMLIB_TICKRATE_MAX (255)
#define ATMLIB_TICKRATE_DEFAULT (25)
#define MAX_VOLUME (127)
#define LAST_NOTE (63)
const uint16_t noteTable[64] PROGMEM = {
	0,
	268,  284,  301,  319,  338,  358,  379,  401,  425,  451,  477,  506,
	536,  568,  601,  637,  675,  715,  758,  803,  851,  901,  955,  1011,
	1072, 1135, 1203, 1274, 1350, 1430, 1515, 1606, 1701, 1802, 1909, 2023,
	2143, 2271, 2406, 2549, 2700, 2861, 3031, 3211, 3402, 3604, 3819, 4046,
	4286, 4541, 4811, 5098, 5401, 5722, 6062, 6422, 6804, 7209, 7638, 8092,
	8573, 9083, 9623
};

static uint16_t note_index_2_phase_inc(const uint8_t note_idx)
{
	return pgm_read_word(&noteTable[(note_idx) & 0x3F]);
}

//static void atm_synth_ext_tick_handler(uint8_t cb_index, void *priv);

#define pattern_index(ch_ptr) ((ch_ptr)->pstack[(ch_ptr)->pstack_index].pattern_index)
#define pattern_cmd_ptr(ch_ptr) ((ch_ptr)->pstack[(ch_ptr)->pstack_index].next_cmd_ptr)
#define pattern_repetition_counter(ch_ptr) ((ch_ptr)->pstack[(ch_ptr)->pstack_index].repetitions_counter)

/* ---- */

#if ATM_HAS_FX_GLISSANDO || ATM_HAS_FX_SLIDE || ATM_HAS_FX_LFO
/* flags: bit 7 = 0 clamp, 1 overflow */
static uint16_t slide_quantity(int8_t amount, int16_t value, int16_t bottom, int16_t top, uint8_t flags)
{
	const bool clamp = !(flags & 0x80);
	const int16_t res = value + amount;
	if (clamp) {
		if (res < bottom) {
			return bottom;
		} else if (res > top) {
			return top;
		}
	}
	return res;
}
#endif

#if ATM_HAS_FX_SLIDE || ATM_HAS_FX_LFO
static void addto_osc_param(const int8_t amount, const uint8_t param, struct osc_params *osc_params, const uint8_t flags)
{
	const uint8_t v = param & 0xC0;
	if (!v) {
		osc_params->vol = slide_quantity(amount, osc_params->vol, 0, MAX_VOLUME, flags);
	} else if (v == 0x40) {
		osc_params->phase_increment = (osc_params->phase_increment & 0x8000) | slide_quantity(amount, osc_params->phase_increment & OSC_PHASE_INC_MAX, 0, OSC_PHASE_INC_MAX, flags);
	} else if (v == 0xC0 ) {
		osc_params->mod = slide_quantity(amount, osc_params->mod, 0, OSC_MOD_MAX, flags);
	}
}
#endif

#if ATM_HAS_FX_SLIDE
static void slidefx(struct atm_slide_params *slide_params, struct osc_params *osc_params)
{
	if (slide_params->slide_amount) {
		if ((slide_params->slide_count & 0x3F) >= (slide_params->slide_config & 0x3F)) {
			addto_osc_param(slide_params->slide_amount, slide_params->slide_count & 0xC0, osc_params, slide_params->slide_config & 0x80);
			slide_params->slide_count &= 0xC0;
		} else {
			slide_params->slide_count++;
		}
	}
}
#endif

union fmt_hdr {
	uint16_t u16;
	struct {
		uint8_t fmt;
		uint8_t data;
	} f;
};

static const uint8_t *get_entry_pattern_array_ptr(const uint8_t *score)
{
	union fmt_hdr hdr;

	hdr.u16 = pgm_read_word(score);
	const uint8_t pattern_count = hdr.f.fmt & 0x02 ? hdr.f.data : 0;
	return hdr.f.fmt & 0x01 ? score + sizeof(uint16_t)*pattern_count + 3 : NULL;
}

static const uint8_t *get_track_start_ptr(const struct atm_player_state *player_state, const uint8_t track_index)
{
	const uint8_t *s = player_state->score_start;
	union fmt_hdr hdr;

	hdr.u16 = pgm_read_word(s);
	if (hdr.f.fmt & 0x02) {
		return s + pgm_read_word(s+2+sizeof(uint16_t)*track_index);
	}
	return hdr.f.fmt & 0x01 ? s+1+hdr.f.data : s+1;
}

void atm_synth_setup(void)
{
	osc_setup();
	osc_set_isr_active(1);
	struct atm_score_state *score_state = &atmlib_state.score_state;
	/* no channel is active */
	score_state->player_state.channel_active_mask = 0;
	/* take over all oscillators */
	for (unsigned n = 0; n < ARRAY_SIZE(score_state->channel_state); n++) {
		score_state->channel_state[n].dst_osc_params = &osc_params_array[n];
	}
}

static void atm_synth_init_channel(struct atm_channel_state *ch, struct osc_params *dst, const struct atm_player_state *player, uint8_t pattern_index)
{
	memset(ch, 0, sizeof(*ch));
#if ATM_HAS_FX_NOTE_RETRIG
	ch->arpCount = 0x80;
#endif
	ch->mod = 0x7F;
#if ATM_HAS_FX_LOOP
	ch->loop_pattern_index = 255;
#endif
	ch->dst_osc_params = dst;
	ch->pstack[0].next_cmd_ptr = get_track_start_ptr(player, pattern_index);
	ch->pstack[0].pattern_index = pattern_index;
}

static void atm_player_init_state(const uint8_t *score, struct atm_player_state *dst)
{
	dst->score_start = score;
	dst->tick_rate = OSC_TICKRATE/ATMLIB_TICKRATE_DEFAULT-1;
}

void atm_synth_grab_channel(const uint8_t channel_index, struct osc_params *save)
{
	*save = osc_params_array[channel_index];
	memset(&osc_params_array[channel_index], 0, sizeof(osc_params_array[0]));
	atmlib_state.score_state.channel_state[channel_index].dst_osc_params = save;
}

void atm_synth_release_channel(const uint8_t channel_index)
{
	struct atm_channel_state *channel = &atmlib_state.score_state.channel_state[channel_index];
	osc_params_array[channel_index] = *channel->dst_osc_params;
	channel->dst_osc_params = &osc_params_array[channel_index];
}

void atm_synth_play_sfx_track(const uint8_t ch_index, const uint8_t sfx_slot, const uint8_t *sfx)
{
	atm_synth_stop_sfx_track(sfx_slot);
	struct atm_sfx_state *sfx_state = &atmlib_state.sfx_slot[sfx_slot];
	sfx_state->ch_index = ch_index;
	atm_synth_grab_channel(ch_index, &sfx_state->osc_params);
	atm_player_init_state((const uint8_t*)sfx, &sfx_state->player_state);
	atm_synth_init_channel(sfx_state->channel_state, &osc_params_array[ch_index], &sfx_state->player_state, 0);
	/* Start SFX */
	/* override active flags so only one channel is active for the SFX player */
	sfx_state->player_state.channel_active_mask = 1 << ch_index;
}

void atm_synth_stop_sfx_track(const uint8_t sfx_slot)
{
	struct atm_sfx_state *sfx_state = &atmlib_state.sfx_slot[sfx_slot];
	sfx_state->player_state.channel_active_mask = 0;
	atm_synth_release_channel(sfx_state->ch_index);
}

void atm_synth_start_score(const uint8_t *score)
{
	/* stop current score if any */
	atm_synth_set_score_paused(1);
	/* Set default score data */
	struct atm_score_state *score_state = &atmlib_state.score_state;
	atm_player_init_state(score, &score_state->player_state);
	/* Read track count */
	const uint8_t *ep = get_entry_pattern_array_ptr(score);
	/* Fetch starting points for each track */
	for (unsigned n = 0; n < ARRAY_SIZE(score_state->channel_state); n++) {
		atm_synth_init_channel(&score_state->channel_state[n],
			score_state->channel_state[n].dst_osc_params, &score_state->player_state, pgm_read_byte(&ep[n]));
	}
	/* Start playback */
	atm_synth_set_score_paused(0);
}

/* FIXME: the corresponding 'stop' function is missing */
void atm_synth_play_ext(const struct atm_synth_ext *synth_ext)
{
	/* re-implement me! */
}

uint8_t atm_synth_is_score_playing(void)
{
	return atmlib_state.score_state.player_state.channel_active_mask;
}

void atm_synth_set_score_paused(const uint8_t paused)
{
	struct atm_score_state *score_state = &atmlib_state.score_state;
	if (paused) {
		atmlib_state.score_state.player_state.channel_active_mask = 0;
		for (unsigned n = 0; n < ARRAY_SIZE(score_state->channel_state); n++) {
				/* Volume for each channel should be saved here and restored upon unmute */
				/* but we want to save memory so after unpausing the fist note or effect */
				/* will update the volume again */
				score_state->channel_state[n].dst_osc_params->vol = 0;
		}
	} else {
		atmlib_state.score_state.player_state.channel_active_mask = ATM_SCORE_CH_MASK;
	}
}

/* include source file directly so the compiler can inline static functions therein */
#include "cmd_parse.c"

static void process_fx(struct atm_player_state *player_state, struct atm_channel_state *ch)
{
	(void)(player_state);

#if ATM_HAS_FX_GLISSANDO
	//Apply Glissando
	if (ch->glisConfig && (ch->glisCount++ >= (ch->glisConfig & 0x7F))) {
		const uint8_t amount = (ch->glisConfig & 0x80) ? -1 : 1;
		const uint8_t note = slide_quantity(amount, ch->note, 1, LAST_NOTE, 0);
		ch->note = note;
		ch->dst_osc_params->phase_increment &= 0x8000;
		ch->dst_osc_params->phase_increment |= note_index_2_phase_inc(note);
		ch->glisCount = 0;
	}
#endif

#if ATM_HAS_FX_SLIDE
	// Apply volume/frequency slides
	slidefx(&ch->vf_slide, ch->dst_osc_params);
#endif

#if ATM_HAS_FX_NOTE_RETRIG
	{
		uint8_t arp_flags = ch->arpCount & 0xE0;
		/* Apply Arpeggio or Note Cut */
		if ((arp_flags != 0x80) && ch->note) {
			if ((ch->arpCount & 0x1F) < (ch->arpTiming & 0x1F)) {
				ch->arpCount++;
			} else {
				const uint8_t note = ch->note;
				const uint8_t num_notes = ch->arpTiming & 0x40 ? 0x20 : 0x40;
				uint8_t arp_note;

				if (arp_flags == num_notes) {
					/* is re-trigger disabled ? */
					const uint8_t disable_auto_trigger = ch->arpTiming & 0x20;
					arp_flags = disable_auto_trigger ? 0x80 : 0;
					arp_note = disable_auto_trigger ? 0 : note;
				} else {
					const uint8_t note_inc = arp_flags ? ch->arpNotes & 0x0F : ch->arpNotes >> 4;
					arp_note = note_inc == 0x0F ? 0 : note + note_inc;
					arp_flags += 0x20;
				}
				trigger_note(arp_note, ch);
				ch->arpCount = arp_flags;
			}
		}
	}
#endif

#if ATM_HAS_FX_LFO
	// Apply Tremolo or Vibrato
	if (ch->treviDepth) {
		const int8_t amount = ch->treviCount & 0x80 ? ch->treviDepth : -ch->treviDepth;
		addto_osc_param(amount, ch->treviConfig & 0xC0, ch->dst_osc_params, 0);
		if ((ch->treviCount & 0x1F) < (ch->treviConfig & 0x1F)) {
			ch->treviCount++;
		} else {
			if (ch->treviCount & 0x80) {
				ch->treviCount = 0;
			} else {
				ch->treviCount = 0x80;
			}
		}
	}
#endif
}

static void process_channel(const uint8_t ch_index, struct atm_player_state *player_state, struct atm_channel_state *ch)
{
	process_fx(player_state, ch);

	while (ch->delay == 0) {
		struct atm_cmd_data cmd;
		/*
		Reading the command first and then its parameters
		takes up more progmem so we read a fixed amount.
		maximum command size is 4 right now
		*/
		memcpy_P(&cmd, pattern_cmd_ptr(ch), sizeof(struct atm_cmd_data));
		process_cmd(&cmd, player_state, ch);
	}

	if (ch->delay != 0xFFFF) {
		ch->delay--;
	} else {
		player_state->channel_active_mask &= ~(1 << ch_index);
	}
}
#if 0
static void atm_synth_ext_tick_handler(uint8_t cb_index, void *priv) {
	(void)(cb_index);
	struct atm_synth_ext *ext = (struct atm_synth_ext *)priv;

	for (uint8_t ch_index = 0; ch_index < ARRAY_SIZE(channels); ch_index++)
	{
		process_fx(ch_index, &atmlib_state, &channels[ch_index]);
		ext->cb(OSC_CH_COUNT, &atmlib_state, channels, ext);
		osc_set_tick_rate(0, atmlib_state.tick_rate);
	}
}
#endif
static void atm_synth_sfx_tick_handler(void)
{

	for (int i = 0; i < ATM_SFX_SLOT_COUNT; i++) {
		struct atm_sfx_state *sfx_state = &atmlib_state.sfx_slot[i];
		if (!sfx_state->player_state.channel_active_mask) {
			continue;
		}

		if (--sfx_state->player_state.tick_counter != 255) {
			continue;
		}
		sfx_state->player_state.tick_counter = sfx_state->player_state.tick_rate;

		const uint8_t sfx_ch_index = sfx_state->ch_index;
		process_channel(sfx_ch_index, &sfx_state->player_state, sfx_state->channel_state);
		if (!(sfx_state->player_state.channel_active_mask)) {
			/* sfx done */
			atm_synth_stop_sfx_track(i);
		}
	}
}

static void atm_synth_score_tick_handler(void)
{

	struct atm_player_state *p = &atmlib_state.score_state.player_state;
	if (!p->channel_active_mask) {
		return;
	}

	if (--p->tick_counter != 255) {
		return;
	}

	p->tick_counter = p->tick_rate;

start_loop:

	// for every channel start working
	for (uint8_t ch_index = 0; ch_index < ARRAY_SIZE(atmlib_state.score_state.channel_state); ch_index++) {
		process_channel(ch_index, p, &atmlib_state.score_state.channel_state[ch_index]);
	}

	/* if all channels are inactive, stop playing or check for repeat */
	if (!p->channel_active_mask) {
#if ATM_HAS_FX_LOOP
		for (uint8_t ch_index = 0; ch_index < ARRAY_SIZE(atmlib_state.score_state.channel_state); ch_index++) {
			struct atm_channel_state *const ch = &atmlib_state.score_state.channel_state[ch_index];
			/* a quirk in the original implementation does not allow to loop to pattern 0 */
			if (ch->loop_pattern_index == 255) {
				continue;
			}
			/* restart channel */
			ch->delay = 0;
			p->channel_active_mask |= (1<<ch_index);
		}
		if (p->channel_active_mask) {
			goto start_loop;
		}
#endif
	}
}

void atmlib_tick_handler(void)
{
	atm_synth_score_tick_handler();
	atm_synth_sfx_tick_handler();
}

__attribute__((weak)) void osc_tick_handler(void)
{
	atmlib_tick_handler();
}
