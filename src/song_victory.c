#include <avr/pgmspace.h>
#include "src/ATMlib2/atm_synth.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

#ifndef NUM_PATTERNS
#define NUM_PATTERNS(struct_) (ARRAY_SIZE( ((struct_ *)0)->patterns_offset))
#endif

#ifndef DEFINE_PATTERN
#define DEFINE_PATTERN(pattern_id, values) static const uint8_t pattern_id[] = values;
#endif

#define pattern0_data { \
    ATM_CMD_M_SET_VOLUME(127), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-8, 0x40 + 1), \
    ATM_CMD_M_CALL(1), \
    ATM_CMD_M_ADD_TRANSPOSITION(-4), \
    ATM_CMD_M_CALL(1), \
    ATM_CMD_M_ADD_TRANSPOSITION(+2), \
    ATM_CMD_M_CALL(1), \
    ATM_CMD_M_ADD_TRANSPOSITION(+2), \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS(3), \
    ATM_CMD_I_NOTE_A5_, \
    ATM_CMD_M_DELAY_TICKS(3), \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern0_array, pattern0_data);

#define pattern1_data { \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS(3), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(3), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(3), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(3), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern1_array, pattern1_data);

#define pattern2_data { \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern2_array, pattern2_data);

static const PROGMEM struct score_data {
  uint8_t fmt;
  uint8_t num_patterns;
  uint16_t patterns_offset[3];
  uint8_t num_channels;
  uint8_t start_patterns[4];
  uint8_t pattern0[sizeof(pattern0_array)];
  uint8_t pattern1[sizeof(pattern1_array)];
  uint8_t pattern2[sizeof(pattern2_array)];
} score = {
  .fmt = ATM_SCORE_FMT_FULL,
  .num_patterns = NUM_PATTERNS(struct score_data),
  .patterns_offset = {
      offsetof(struct score_data, pattern0),
      offsetof(struct score_data, pattern1),
      offsetof(struct score_data, pattern2),
  },
  .num_channels = 1,
  .start_patterns = {
    0x00,                         // Channel 0 entry track (PULSE)
    0x02,                         // Channel 1 entry track (SQUARE)
    0x02,                         // Channel 2 entry track (TRIANGLE)
    0x02,                         // Channel 3 entry track (NOISE)
  },
  .pattern0 = pattern0_data,
  .pattern1 = pattern1_data,
  .pattern2 = pattern2_data,
};

uint8_t const* song_victory() { return (uint8_t const*)&score; }
