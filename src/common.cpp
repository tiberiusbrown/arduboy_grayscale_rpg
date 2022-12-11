#include "common.hpp"

#include <string.h>

#include "generated/item_info_prog.hpp"

static char const EN_DARK_GUARD[] PROGMEM = "Dark Guard";
static char const EN_DARK_WIZARD[] PROGMEM = "Dark Wizard";

enemy_info_t const ENEMY_INFO[] PROGMEM =
{
    { 4, 4, 0, 4, 10,  64, 128, EN_DARK_GUARD },
    { 5, 4, 0, 4,  6,   0, 255, EN_DARK_WIZARD },
};

static char const PN_MATTHIAS[] PROGMEM = "Matthias";
static char const PN_CATHERINE[] PROGMEM = "Catherine";
static char const PN_LUCY[] PROGMEM = "Lucy";
static char const PN_DISMAS[] PROGMEM = "Dismas";

party_info_t const PARTY_INFO[4] PROGMEM =
{
    { 0, 0, 4, 10, 4, 0, 10, PN_MATTHIAS },
    { 1, 1, 4, 10, 4, 0, 10, PN_CATHERINE },
    { 2, 2, 4, 10, 4, 0, 10, PN_LUCY },
    { 3, 3, 4, 10, 4, 0, 10, PN_DISMAS },
};

uint8_t party_att(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    int8_t r = (int8_t)pgm_read_byte(&PARTY_INFO[id].base_att);
    r += items_att(i);
    if(r < 1) r = 1;
    if(r > 99) r = 99;
    return (uint8_t)r;
}

uint8_t party_def(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    int8_t r = (int8_t)pgm_read_byte(&PARTY_INFO[id].base_def);
    r += items_def(i);
    if(r < 0) r = 0;
    if(r > 99) r = 99;
    return r;
}

uint8_t party_mhp(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    int8_t r = (int8_t)pgm_read_byte(&PARTY_INFO[id].base_mhp);
    r += items_mhp(i);
    if(r < 1) r = 1;
    if(r > 99) r = 99;
    return r;
}

uint8_t party_spd(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    int8_t r = (int8_t)pgm_read_byte(&PARTY_INFO[id].base_spd);
    r += items_spd(i);
    if(r < 0) r = 0;
    if(r > 99) r = 99;
    return r;
}

void party_clip_hp()
{
    for(uint8_t i = 0; i < nparty; ++i)
    {
        uint8_t mhp = party_mhp(i);
        if(party[i].battle.hp > mhp)
            party[i].battle.hp = mhp;
    }
}

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

void adjust(uint8_t& rx, uint8_t tx)
{
    uint8_t x = rx;
    uint8_t dx = x - tx;
    if(x < tx) dx = -dx;
    ++dx;
    dx /= 2;
    if(x < tx) dx = -dx;
    x -= dx;
    //if(x < tx)
    //    x += (uint8_t(tx - x + 1) / 2);
    //else
    //    x -= (uint8_t(x - tx + 1) / 2);
    rx = x;
}

uint16_t rand_seed;

uint8_t state;
sdata_t sdata;

uint8_t nframe;

bool pmoving;
uint16_t selx, sely;

active_chunk_t active_chunks[4];

uint8_t btns_down, btns_pressed;

bool chunks_are_running;
uint8_t running_chunk;
uint8_t chunk_instr;

savefile_t savefile;
