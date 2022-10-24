#include "common.hpp"

#include <string.h>

void change_state(uint8_t new_state) {
    state = new_state;
    memset(&sdata, 0, sizeof(sdata));
}

uint8_t state;
sdata_t sdata;

uint8_t nframe;

uint8_t pdir;
bool pmoving;
uint16_t px, py;
uint16_t selx, sely;

active_chunk_t active_chunks[4];
uint8_t loaded_cx, loaded_cy;

uint8_t btns_down, btns_pressed;

bool chunks_are_running;
uint8_t running_chunk;
uint8_t chunk_instr;
