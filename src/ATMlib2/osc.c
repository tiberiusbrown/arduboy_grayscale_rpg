
#include <stddef.h>
#include <string.h>
#include <avr/interrupt.h>
#include "osc.h"

/*
  Mixing 4 8bit channels requires 10 bits but by setting ENHC4 in TCCR4E
  bit 0 of OCR4A selects the timer clock edge so while the
  resolution is 10 bits the number of timer bits used to count
  (set in OCR4C) is 9.

  See section "15.6.2 Enhanced Compare/PWM mode" in the ATmega16U4/ATmega32U4
  datasheet. This means the PWM frequency can be double of what it would be
  if we used 10 timer bits for counting.

  Also, 8 8bit channels can be mixed (11bits resolution, 10bits
  timer counter) if the PWM rate is halved (or fast PWM mode is used
  instead of dual slope PWM).
*/

#define OSC_COMPARE_RESOLUTION_BITS (10)
#define OSC_DC_OFFSET (1<<(OSC_COMPARE_RESOLUTION_BITS-1))

#define OSC_TIMER_BITS (9)
#define OSC_PWM_TOP ((1<<OSC_TIMER_BITS)-1)
#define OSC_HI(v) ((v)>>8)
#define OSC_LO(v) ((v)&0xFF)

static void osc_reset(void);

static uint8_t osc_isr_reenter = 0;
static uint8_t osc_int_count;
struct osc_params osc_params_array[OSC_CH_COUNT];
static uint16_t osc_pha_acc_array[OSC_CH_COUNT];

void osc_setup(void)
{
	osc_reset();
	/* PWM setup using timer 4 */
	PLLFRQ = 0b01011010;    /* PINMUX:16MHz XTAL, PLLUSB:48MHz, PLLTM:1, PDIV:96MHz */
	PLLCSR = 0b00010010;
	/* Wait for PLL lock */
	while (!(PLLCSR & 0x01)) {}
	TCCR4A = 0b01000010;    /* PWM mode */
	/* TCCR4B will be se to 0b00000001 for clock source/1, 96MHz/(OCR4C+1)/2 ~ 95703Hz */
	TCCR4D = 0b00000001;    /* Dual Slope PWM (the /2 in the eqn. above is because of dual slope PWM) */
	TCCR4E = 0b01000000;    /* Enhanced mode (bit 0 in OCR4C selects clock edge) */
	TC4H   = OSC_HI(OSC_PWM_TOP);
	OCR4C  = OSC_LO(OSC_PWM_TOP); /* Use 9-bits for counting (TOP=0x1FF) */

	TCCR3A = 0b00000000;
	TCCR3B = 0b00001001;    /* Mode CTC, clock source 16MHz */
	OCR3A  = (16E6/OSC_SAMPLERATE)-1; /* 16MHz/1k = 16kHz */
}

static void osc_reset(void)
{
	osc_set_isr_active(0);
	memset(osc_params_array, 0, sizeof(osc_params_array));
	for (uint8_t i = 0; i < OSC_CH_COUNT; i++) {
		/* set modulation to 50% duty cycle */
		osc_params_array[i].mod = 0x7F;
	}
}

void osc_set_isr_active(const uint8_t active_flag)
{
	if (active_flag) {
		TC4H   = OSC_HI(OSC_DC_OFFSET);
		OCR4A  = OSC_LO(OSC_DC_OFFSET);
		TCCR4B = 0b00000001;    /* clock source/1, 96MHz/(OCR4C+1)/2 ~ 95703Hz */
		TIMSK3 = 0b00000010;    /* interrupts on */
	} else {
		TIMSK3 = 0b00000000;    /* interrupts off */
		TCCR4B = 0b00000000;    /* PWM = off */
	}
}

ISR(TIMER3_COMPA_vect)
{
	uint16_t pcm = OSC_DC_OFFSET;
	struct osc_params *p = osc_params_array;
	for (uint8_t i = 0; i < OSC_CH_COUNT ; i++,p++) {
		int8_t vol = (int8_t)p->vol;
		if (!vol) {
			/* skip if volume or phase increment is zero and save some cycles */
			continue;
		}
		const uint16_t phi = p->phase_increment;
		if (!(phi & 0x7FFF)) {
			/* skip if volume or phase increment is zero and save some cycles */
			continue;
		}
		{
			uint16_t pha = osc_pha_acc_array[i];
			if (OSC_HI(phi) & 0x80) {
				// TODO: try moving seeding from the ISR to where the waveform is selected
				pha = pha ? pha : 0x0001; // Seed LFSR
				const uint8_t msb = OSC_HI(pha) & 0x80;
				pha += pha;
				if (msb) {
					// TODO: make sure this compiles to a sngle XOR operation on the lower 8 bits
					pha ^= 0x002D;
				} else {
					vol = -vol;
				}
			} else {
				pha += phi;
				vol = (OSC_HI(pha) > p->mod) ? vol : -vol;
			}
			osc_pha_acc_array[i] = pha;
			pcm += vol;
		}
	}

	TC4H = OSC_HI(pcm);
	OCR4A = OSC_LO(pcm);

	if (!(--osc_int_count)) {
		osc_int_count = OSC_ISR_PRESCALER_DIV;
		osc_tick_handler();
	}
}
