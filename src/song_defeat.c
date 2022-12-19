#include <avr/pgmspace.h>
#include "src/ATMlib2/atm_synth.h"

#define LONGER_VERSION 0

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

#ifndef NUM_PATTERNS
#define NUM_PATTERNS(struct_) (ARRAY_SIZE( ((struct_ *)0)->patterns_offset))
#endif

#ifndef DEFINE_PATTERN
#define DEFINE_PATTERN(pattern_id, values) static const uint8_t pattern_id[] = values;
#endif

#define MELODY_NOTES \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(2), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_F5_, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_F5_, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_F5_, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_A5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_A5, \
    ATM_CMD_M_DELAY_TICKS(2), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_C6, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4) \

#define HARMONY_HI_NOTES \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(2), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_D5_, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_F5_, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_F5_, \
    ATM_CMD_M_DELAY_TICKS(2), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_G5, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_D5_, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4) \

#define HARMONY_LO_NOTES \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(2), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_E5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    \
    ATM_CMD_I_NOTE_B5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_D5, \
    ATM_CMD_M_DELAY_TICKS(6), \
    ATM_CMD_I_NOTE_D5, \
    ATM_CMD_M_DELAY_TICKS(2), \
    ATM_CMD_I_NOTE_D5, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_G4, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_A4, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_G4, \
    ATM_CMD_M_DELAY_TICKS(4), \
    ATM_CMD_I_NOTE_F4_, \
    ATM_CMD_M_DELAY_TICKS(8), \
    ATM_CMD_I_NOTE_G4, \
    ATM_CMD_M_DELAY_TICKS(12), \
    ATM_CMD_I_NOTE_OFF, \
    ATM_CMD_M_DELAY_TICKS(4) \

#if LONGER_VERSION
#define pattern0_data { \
    ATM_CMD_M_SET_VOLUME(72), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-6, 0x40 + 1), \
    ATM_CMD_M_CALL_REPEAT(1, 2), \
    ATM_CMD_I_STOP, \
}
#else
#define pattern0_data { \
    ATM_CMD_M_SET_VOLUME(72), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-6, 0x40 + 1), \
    MELODY_NOTES, \
    ATM_CMD_I_STOP, \
}
#endif    
DEFINE_PATTERN(pattern0_array, pattern0_data);

#if LONGER_VERSION
#define pattern1_data { \
    MELODY_NOTES, \
    ATM_CMD_I_RETURN, \
}
#else
#define pattern1_data { \
    ATM_CMD_M_SET_VOLUME(72), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-6, 0x40 + 1), \
    HARMONY_HI_NOTES, \
    ATM_CMD_I_STOP, \
}
#endif
DEFINE_PATTERN(pattern1_array, pattern1_data);

#if LONGER_VERSION
#define pattern2_data { \
    ATM_CMD_M_SET_VOLUME(72), \
    ATM_CMD_M_SLIDE_VOL_ADV_ON(-6, 0x40 + 1), \
    HARMONY_HI_NOTES, \
    HARMONY_LO_NOTES, \
    ATM_CMD_I_STOP, \
}
DEFINE_PATTERN(pattern2_array, pattern2_data);
#endif

static const PROGMEM struct score_data {
  uint8_t fmt;
  uint8_t num_patterns;
  uint16_t patterns_offset[LONGER_VERSION ? 3 : 2];
  uint8_t num_channels;
  uint8_t start_patterns[2];
  uint8_t pattern0[sizeof(pattern0_array)];
  uint8_t pattern1[sizeof(pattern1_array)];
#if LONGER_VERSION
  uint8_t pattern2[sizeof(pattern2_array)];
#endif
} score = {
  .fmt = ATM_SCORE_FMT_FULL,
  .num_patterns = NUM_PATTERNS(struct score_data),
  .patterns_offset = {
      offsetof(struct score_data, pattern0),
      offsetof(struct score_data, pattern1),
#if LONGER_VERSION
      offsetof(struct score_data, pattern2),
#endif
  },
  .num_channels = 2,
  .start_patterns = {
    0,                         // Channel 0 entry track (PULSE)
    LONGER_VERSION ? 2 : 1,                         // Channel 1 entry track (SQUARE)
    //0x02,                         // Channel 2 entry track (TRIANGLE)
    //0x02,                         // Channel 3 entry track (NOISE)
  },
  .pattern0 = pattern0_data,
  .pattern1 = pattern1_data,
#if LONGER_VERSION
  .pattern2 = pattern2_data,
#endif
};

uint8_t const* song_defeat() { return (uint8_t const*)&score; }
