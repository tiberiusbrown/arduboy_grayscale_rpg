#include "common.hpp"

decltype(sdata) sdata;

uint8_t nframe;

uint8_t pdir;
bool pmoving;
uint16_t px, py;

active_chunk_t active_chunks[4];
uint8_t loaded_cx, loaded_cy;

uint8_t btns_down, btns_pressed;
