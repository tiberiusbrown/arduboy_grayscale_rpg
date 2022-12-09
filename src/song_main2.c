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
    ATM_CMD_M_SET_VOLUME(64), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-8, 0x40 + 1), \
    /* 32 measures */ \
    ATM_CMD_M_CALL_REPEAT(5, 2), \
    ATM_CMD_M_ADD_TRANSPOSITION(+4), \
    ATM_CMD_M_CALL_REPEAT(5, 2), \
    ATM_CMD_M_ADD_TRANSPOSITION(-4), \
    /* 16 measures */ \
    ATM_CMD_M_CALL_REPEAT(6, 2), \
    ATM_CMD_M_ADD_TRANSPOSITION(+4), \
    ATM_CMD_M_CALL_REPEAT(6, 2), \
    ATM_CMD_M_ADD_TRANSPOSITION(-4), \
    \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern0_array, pattern0_data);

#define pattern1_data { \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_A5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_F5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern1_array, pattern1_data);

#define pattern2_data { \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern2_array, pattern2_data);

#define pattern3_data { \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern3_array, pattern3_data);

#define pattern4_data { \
    ATM_CMD_I_NOTE_F5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_A5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern4_array, pattern4_data);

// 8 measures
#define pattern5_data { \
    ATM_CMD_M_CALL_REPEAT(1, 4), \
    ATM_CMD_M_ADD_TRANSPOSITION(+2), \
    ATM_CMD_M_CALL_REPEAT(1, 4), \
    ATM_CMD_M_ADD_TRANSPOSITION(-2), \
    ATM_CMD_M_CALL_REPEAT(1, 4), \
    ATM_CMD_M_ADD_TRANSPOSITION(-1), \
    ATM_CMD_M_CALL_REPEAT(1, 4), \
    ATM_CMD_M_ADD_TRANSPOSITION(+1), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern5_array, pattern5_data);

#define pattern6_data { \
    ATM_CMD_M_CALL_REPEAT(3, 4), \
    ATM_CMD_M_CALL_REPEAT(4, 4), \
    ATM_CMD_M_CALL_REPEAT(3, 4), \
    ATM_CMD_M_ADD_TRANSPOSITION(-3), \
    ATM_CMD_M_CALL_REPEAT(4, 4), \
    ATM_CMD_M_ADD_TRANSPOSITION(+3), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern6_array, pattern6_data);

#define pattern7_data { \
    ATM_CMD_M_SET_VOLUME(96), \
    ATM_CMD_M_TREMOLO_ON(16, 3), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-2, 0x40 + 1), \
    ATM_CMD_M_DELAY_TICKS_2(48 * 32), \
    ATM_CMD_M_CALL_REPEAT(8, 2), \
    ATM_CMD_M_ADD_TRANSPOSITION(+4), \
    ATM_CMD_M_CALL_REPEAT(8, 2), \
    ATM_CMD_M_ADD_TRANSPOSITION(-4), \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern7_array, pattern7_data);

#define pattern8_data { \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS_1(48), \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS_1(48), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS_1(48), \
    ATM_CMD_I_NOTE_A5, \
    ATM_CMD_M_DELAY_TICKS_1(48), \
    ATM_CMD_I_RETURN, \
}
DEFINE_PATTERN(pattern8_array, pattern8_data);

/* 1/2 measure */
#define pattern9_data {               \
    ATM_CMD_M_SET_VOLUME(30),         \
    ATM_CMD_I_NOTE_C6,                \
    ATM_CMD_M_SLIDE_VOL_ON(-3),       \
    ATM_CMD_M_DELAY_TICKS(9),         \
    ATM_CMD_M_SET_VOLUME(30),         \
    ATM_CMD_M_SLIDE_VOL_ON(-9),      \
    ATM_CMD_M_DELAY_TICKS(3),         \
    ATM_CMD_M_SET_VOLUME(30),         \
    ATM_CMD_M_SLIDE_VOL_ON(-5),       \
    ATM_CMD_M_DELAY_TICKS(6),         \
    ATM_CMD_M_SET_VOLUME(30),         \
    ATM_CMD_M_SLIDE_VOL_ON(-5),       \
    ATM_CMD_M_DELAY_TICKS(6),         \
    ATM_CMD_M_SLIDE_VOL_OFF,          \
    ATM_CMD_I_NOTE_OFF,               \
    ATM_CMD_I_RETURN,                 \
}
 DEFINE_PATTERN(pattern9_array, pattern9_data);

/* 1/2 measure */
#define pattern10_data { \
    ATM_CMD_M_SET_WAVEFORM(1), \
    /* 32 measures */ \
    ATM_CMD_M_DELAY_TICKS_2(48 * 8), \
    ATM_CMD_M_CALL_REPEAT(9, 16), \
    ATM_CMD_M_DELAY_TICKS_2(48 * 8), \
    ATM_CMD_M_CALL_REPEAT(9, 16), \
    \
    /* 16 measures */ \
    ATM_CMD_M_DELAY_TICKS_1(48 * 4), \
    ATM_CMD_M_CALL_REPEAT(9, 8), \
    ATM_CMD_M_DELAY_TICKS_1(48 * 4), \
    ATM_CMD_M_CALL_REPEAT(9, 8), \
    \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern10_array, pattern10_data);

static const PROGMEM struct score_data {
  uint8_t fmt;
  uint8_t num_patterns;
  uint16_t patterns_offset[11];
  uint8_t num_channels;
  uint8_t start_patterns[4];
  uint8_t pattern0[sizeof(pattern0_array)];
  uint8_t pattern1[sizeof(pattern1_array)];
  uint8_t pattern2[sizeof(pattern2_array)];
  uint8_t pattern3[sizeof(pattern3_array)];
  uint8_t pattern4[sizeof(pattern4_array)];
  uint8_t pattern5[sizeof(pattern5_array)];
  uint8_t pattern6[sizeof(pattern6_array)];
  uint8_t pattern7[sizeof(pattern7_array)];
  uint8_t pattern8[sizeof(pattern8_array)];
  uint8_t pattern9[sizeof(pattern9_array)];
  uint8_t pattern10[sizeof(pattern10_array)];
} score = {
  .fmt = ATM_SCORE_FMT_FULL,
  .num_patterns = NUM_PATTERNS(struct score_data),
  .patterns_offset = {
      offsetof(struct score_data, pattern0),
      offsetof(struct score_data, pattern1),
      offsetof(struct score_data, pattern2),
      offsetof(struct score_data, pattern3),
      offsetof(struct score_data, pattern4),
      offsetof(struct score_data, pattern5),
      offsetof(struct score_data, pattern6),
      offsetof(struct score_data, pattern7),
      offsetof(struct score_data, pattern8),
      offsetof(struct score_data, pattern9),
      offsetof(struct score_data, pattern10),
  },
  .num_channels = 4,
  .start_patterns = {
    0,                         // Channel 0 entry track (PULSE)
    7,                         // Channel 1 entry track (SQUARE)
    2,                         // Channel 2 entry track (TRIANGLE)
    10,                        // Channel 3 entry track (NOISE)
  },
  .pattern0 = pattern0_data,
  .pattern1 = pattern1_data,
  .pattern2 = pattern2_data,
  .pattern3 = pattern3_data,
  .pattern4 = pattern4_data,
  .pattern5 = pattern5_data,
  .pattern6 = pattern6_data,
  .pattern7 = pattern7_data,
  .pattern8 = pattern8_data,
  .pattern9 = pattern9_data,
  .pattern10 = pattern10_data,
};

uint8_t const* song_main2() { return (uint8_t const*)&score; }
