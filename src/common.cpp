#include "common.hpp"

#include <string.h>

static char const EN_DARK_GUARD[] PROGMEM = "Dark Guard";
static char const EN_DARK_WIZARD[] PROGMEM = "Dark Wizard";

enemy_info_t const ENEMY_INFO[] PROGMEM =
{
    { 1, 10, 10, EN_DARK_GUARD },
    { 2, 10, 10, EN_DARK_WIZARD },
};

static char const PN_HERO[] PROGMEM = "Matthias";
static char const PN_GIRL[] PROGMEM = "Catherine";
static char const PN_GIRL2[] PROGMEM = "Lucy";
static char const PN_BOY[] PROGMEM = "Dismas";

party_info_t const PARTY_INFO[4] PROGMEM =
{
    { 0, 0, 10, 20, PN_HERO },
    { 3, 2, 10, 10, PN_GIRL },
    { 0, 0, 10, 10, PN_GIRL2 },
    { 0, 0, 10, 10, PN_BOY },
};

static uint8_t simple_mod(uint8_t n, uint8_t d)
{
    while(n >= d) n -= d;
    return n;
}

uint8_t u8rand()
{
    rand_seed ^= rand_seed << 7;
    rand_seed ^= rand_seed >> 9;
    rand_seed ^= rand_seed << 8;
    return (uint8_t)rand_seed;
}

uint8_t u8rand(uint8_t m)
{
    return simple_mod(u8rand(), m);
}

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

void story_flag_clr(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = 1 << (index & 7);
    story_flags[i] &= ~m;
}

void story_flag_tog(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = 1 << (index & 7);
    story_flags[i] ^= m;
}

bool story_flag_get(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = 1 << (index & 7);
    return (story_flags[i] & m) != 0;
}

uint8_t adjust(uint8_t x, uint8_t tx)
{
    if(x < tx)
        x += (uint8_t(tx - x + 1) >> 1);
    else
        x -= (uint8_t(x - tx + 1) >> 1);
    return x;
}

uint16_t rand_seed;

uint8_t state;
sdata_t sdata;

uint16_t nframe;

bool pmoving;
uint16_t selx, sely;

active_chunk_t active_chunks[4];

uint8_t btns_down, btns_pressed;

bool chunks_are_running;
uint8_t running_chunk;
uint8_t chunk_instr;
int8_t chunk_regs[16];

savefile_t savefile;
