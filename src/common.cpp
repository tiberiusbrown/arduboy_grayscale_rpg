#include "common.hpp"

#include <string.h>

static char const EN_DARK_GUARD[] PROGMEM = "Dark Guard";
static char const EN_DARK_WIZARD[] PROGMEM = "Dark Wizard";

enemy_info_t const ENEMY_INFO[] PROGMEM =
{
    { 1, 10, 10, EN_DARK_GUARD },
    { 2, 10, 10, EN_DARK_WIZARD },
};

static char const PN_MATTHIAS[] PROGMEM = "Matthias";
static char const PN_CATHERINE[] PROGMEM = "Catherine";
static char const PN_LUCY[] PROGMEM = "Lucy";
static char const PN_DISMAS[] PROGMEM = "Dismas";

party_info_t const PARTY_INFO[4] PROGMEM =
{
    { 0, 0, 10, 20, 4, 4, 10, PN_MATTHIAS },
    { 3, 2, 10, 10, 3, 3, 10, PN_CATHERINE },
    { 4, 1, 10, 10, 4, 4, 10, PN_LUCY },
    { 0, 3, 10, 10, 4, 4, 10, PN_DISMAS },
};

static char const IN_CLOTH_TUNIC[] PROGMEM = "Cloth Tunic";
static char const IN_BRASS_KNUCKLES[] PROGMEM = "Brass Knuckles";
static char const IN_NINJA_SHOES[] PROGMEM = "Ninja Shoes";
static char const IN_BRAWLERS_RING[] PROGMEM = "Brawler's Ring";
static char const IN_AMULET_OF_ZHARTUL[] PROGMEM = "Amulet of Zhar-Tul";

item_info_t const ITEM_INFO[] PROGMEM =
{
    { 0, 1, 0, 0, IT_SHIRT , IN_CLOTH_TUNIC },
    { 1, 0, 0, 0, IT_WEAPON, IN_BRASS_KNUCKLES },
    { 0, 1, 0, 1, IT_SHOES , IN_NINJA_SHOES },
    { 0, 0, 0, 1, IT_OTHER , IN_BRAWLERS_RING },
    { 0, 0, 0, 0, IT_OTHER , IN_AMULET_OF_ZHARTUL },
};

uint8_t party_att(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    uint8_t r = pgm_read_byte(&PARTY_INFO[id].base_att);
    return r;
}

uint8_t party_def(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    uint8_t r = pgm_read_byte(&PARTY_INFO[id].base_def);
    return r;
}

uint8_t party_mhp(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    uint8_t r = pgm_read_byte(&PARTY_INFO[id].base_mhp);
    return r;
}

uint8_t party_spd(uint8_t i)
{
    uint8_t id = party[i].battle.id;
    uint8_t r = pgm_read_byte(&PARTY_INFO[id].base_spd);
    return r;
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

savefile_t savefile;
