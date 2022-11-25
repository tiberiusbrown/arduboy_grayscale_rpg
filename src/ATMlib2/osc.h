
#include <stdbool.h>
#include <stdint.h>

#define ct_assert(condition, name) typedef char name##_failed[1]; typedef char name##_failed[(condition)?1:0]

#ifndef OSC_SAMPLERATE
#define OSC_SAMPLERATE (16000)
#endif

#ifndef OSC_CH_COUNT
#define OSC_CH_COUNT (4)
#endif

ct_assert(OSC_CH_COUNT <= 8 && OSC_CH_COUNT > 0, osc_channel_count);

#define OSC_MOD_MIN (0)
#define OSC_MOD_MAX (255)

/* osc tick duration is 1ms */
#define OSC_TICKRATE (1000)
#define OSC_ISR_PRESCALER_DIV (OSC_SAMPLERATE/OSC_TICKRATE)

/*
OSC period is 2**16 so Nyquist corresponds to 2**16/2. Note that's way too high
for square waves to sound OK.
*/
#define OSC_PHASE_INC_MAX (0x3FFF)

struct osc_params {
	uint8_t  mod;
	uint8_t  vol;
	uint16_t phase_increment;
};

enum osc_channels_e {
	OSC_CH_0 = 0,
	OSC_CH_1,
	OSC_CH_2,
	OSC_CH_3,
	OSC_CH_4,
	OSC_CH_5,
	OSC_CH_6,
	OSC_CH_7,
};

extern struct osc_params osc_params_array[OSC_CH_COUNT];

/* osc_tick_handler must be defined by the user of this library

It is called at OSC_TICKRATE Hz (1kHz by default) when there is at least one active
OSC channel.

*/
extern void osc_tick_handler(void);

void osc_setup(void);
void osc_set_isr_active(const uint8_t active_flag);
