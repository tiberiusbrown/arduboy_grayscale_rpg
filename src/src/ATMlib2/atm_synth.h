#pragma once
#include <stddef.h>
#include <stdint.h>

#include "osc.h"
#include "atm_cmd_constants.h"

/* Adjust the following to reduce memory usage */
#define ATM_SCORE_CHANNEL_COUNT (2)
#define ATM_SFX_SLOT_COUNT (1)
#define ATM_PATTERN_STACK_DEPTH (1)

ct_assert(ATM_SCORE_CHANNEL_COUNT <= OSC_CH_COUNT, channel_count);
ct_assert(ATM_SFX_SLOT_COUNT <= OSC_CH_COUNT, channel_count);

#define ATM_SCORE_CH_MASK (0xFF >> (8-ATM_SCORE_CHANNEL_COUNT))

#define ATM_HAS_FX_NOTE_RETRIG (0)
#define ATM_HAS_FX_SLIDE (1)
#define ATM_HAS_FX_LFO (0)
#define ATM_HAS_FX_GLISSANDO (0)
#define ATM_HAS_FX_LOOP (0)

/* Public API - we will try not to break this, no promises! */

#define ATM_SCORE_FMT_MINIMAL_MONO (0x0)
#define ATM_SCORE_FMT_FULL_MONO (0x2)
#define ATM_SCORE_FMT_FULL (0x3)

#define ATM_CMD_BUF_SIZE 16

struct atm_channel_state;
struct atm_score_state;

extern struct atm_libstate atmlib_state;

/* ext synth */

struct atm_synth_ext;
struct atm_cmd_data;

typedef void (*atm_synth_ext_callback)(const uint8_t channel_count, struct atm_score_state *score_state, struct atm_channel_state *ch, struct atm_synth_ext *synth_ext);

struct atm_synth_ext {
	atm_synth_ext_callback cb;
	void *priv;
};

/*  */

void atm_synth_setup(void);
void atmlib_tick_handler(void);

void atm_synth_start_score();
uint8_t atm_synth_is_score_playing(void);

void atm_synth_set_score_paused(const uint8_t paused);

void atm_synth_grab_channel(const uint8_t channel_index, struct osc_params *save);
void atm_synth_release_channel(const uint8_t channel_index);

void atm_synth_play_ext(const struct atm_synth_ext *synth_ext);
void ext_synth_command(const uint8_t ch_index, const struct atm_cmd_data *cmd, struct atm_score_state *score_state, struct atm_channel_state *ch);

/* Play score as a sound effect on channel_index

Music playback (if any) is muted on channel_index for the duration of sound
effect playback and resumes when the sound effect is stopped or ran its course.

It is possible to start playback of a new sound effect while one is already
being played back, the active sound effect will stop and the new one will replace it.

Sound effect scores must be in ATM_SCORE_FMT_MINIMAL_MONO or ATM_SCORE_FMT_FULL_MONO format.
*/
void atm_synth_play_sfx_track(const uint8_t osc_channel_index, const uint8_t sfx_slot);

/* Stop a previously started sound effect score */
void atm_synth_stop_sfx_track(const uint8_t sfx_slot);

/* Check if a sound effect score is active */
uint8_t atm_synth_is_sfx_stopped(const uint8_t sfx_slot);

/* Private structures - anything beyond this point is considered internal to the library

structs are only made available in this file so clients can allocate them statically.
All fields should be considered private.

If you think you have a valid use case for exposing some feature please
get in touch with the maintainers.

*/

#if ATM_HAS_FX_SLIDE

struct atm_slide_params {
	int8_t slide_amount;
	uint8_t slide_config;
	uint8_t slide_count;
};

#endif

struct atm_pattern_state {
    uint8_t cmds[ATM_CMD_BUF_SIZE];
	uint8_t next_cmd_ptr;
    uint8_t prev_cmd_ptr;
    __uint24 addr;
};

struct atm_channel_state {
	uint8_t note;
	uint8_t vol;
	uint8_t mod;
	uint16_t delay;
	// Transposition FX
	int8_t trans_config;

	// Nesting
	struct atm_pattern_state pstack[ATM_PATTERN_STACK_DEPTH];
#if ATM_HAS_FX_LOOP
	uint8_t loop_pattern_index;
#endif

	struct osc_params *dst_osc_params;

#if ATM_HAS_FX_SLIDE
	// Volume & Frequency slide FX
	struct atm_slide_params vf_slide;
#endif

#if ATM_HAS_FX_NOTE_RETRIG
	// Arpeggio or Note Cut FX
	uint8_t arpNotes;       // notes: base, base+[7:4], base+[7:4]+[3:0], if FF => note cut ON
	uint8_t arpTiming;      // [7] = reserved, [6] = not third note ,[5] = retrigger, [4:0] = tick count
	uint8_t arpCount;
#endif

#if ATM_HAS_FX_LFO
	// Tremolo or Vibrato FX
	uint8_t treviDepth;
	uint8_t treviConfig;
	uint8_t treviCount;
#endif

#if ATM_HAS_FX_GLISSANDO
	// Glissando FX
	int8_t glisConfig;
	uint8_t glisCount;
#endif
};

struct atm_cmd_data {
	uint8_t id;
	uint8_t params[3];
};

struct atm_player_state {
	uint8_t tick_rate;
	uint8_t tick_counter;
	uint8_t channel_active_mask;
};

struct atm_score_state {
	struct atm_player_state player_state;
	struct atm_channel_state channel_state[ATM_SCORE_CHANNEL_COUNT];
};

struct atm_sfx_state {
	struct atm_player_state player_state;
	struct atm_channel_state channel_state[1];
	struct osc_params osc_params;
	uint8_t ch_index;
};

struct atm_libstate {
	struct atm_score_state score_state;
	struct atm_sfx_state sfx_slot[ATM_SFX_SLOT_COUNT];
};
