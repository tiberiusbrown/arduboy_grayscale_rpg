#include "common.hpp"

#include <string.h>

void change_state(uint8_t new_state)
{
    state = new_state;
    memset(&sdata, 0, sizeof(sdata));
}

void story_flag_set(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = 1 << (index & 7);
    story_flags[i] |= m;
}

bool story_flag_get(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = 1 << (index & 7);
    return (story_flags[i] & m) != 0;
}

uint8_t state;
sdata_t sdata;

uint8_t nframe;

uint8_t pdir;
bool pmoving;
uint16_t px, py;
uint16_t selx, sely;

active_chunk_t active_chunks[4];

uint8_t btns_down, btns_pressed;

bool chunks_are_running;
uint8_t running_chunk;
uint8_t chunk_instr;
int8_t chunk_regs[16];

uint8_t story_flags[STORY_FLAG_BYTES];
