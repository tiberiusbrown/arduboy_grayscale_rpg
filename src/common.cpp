#include "common.hpp"

#include <string.h>

#include "generated/item_info_prog.hpp"

static char const EN_DARK_GUARD[] PROGMEM = "Dark Guard";
static char const EN_DARK_WIZARD[] PROGMEM = "Dark Wizard";
static char const EN_SKELETON[] PROGMEM = "Skeleton";
static char const EN_PSY_RAPTOR[] PROGMEM = "Psy-Raptor";

enemy_info_t const ENEMY_INFO[] PROGMEM =
{
    {  4, 4, 0, 4, 10,  64, 128, EN_DARK_GUARD },
    {  5, 4, 0, 4,  6,   0, 255, EN_DARK_WIZARD },
    { 10, 1, 0, 1,  6,   0,   0, EN_SKELETON },
    { 13, 1, 0, 3,  3,   0,   0, EN_PSY_RAPTOR },
};

static char const PN_MATTHIAS[] PROGMEM = "Matthias";
static char const PN_CATHERINE[] PROGMEM = "Catherine";
static char const PN_LUCY[] PROGMEM = "Lucy";
static char const PN_DISMAS[] PROGMEM = "Dismas";

party_info_t const PARTY_INFO[4] PROGMEM =
{
    { 0, 0, 6, 1, 0, 1, PN_MATTHIAS },
    { 1, 1, 6, 1, 0, 1, PN_CATHERINE },
    { 2, 2, 6, 1, 0, 1, PN_LUCY },
    { 3, 3, 6, 1, 0, 1, PN_DISMAS },
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
    {
        // Dryad items
        uint8_t n = 0;
        static item_t const DRYAD_ITEMS[] PROGMEM =
        {
            SFLAG_ITEM_Dryad_Amulet,
            SFLAG_ITEM_Dryad_Ring,
            SFLAG_ITEM_Dryad_Armor,
            SFLAG_ITEM_Dryad_Shoes,
            SFLAG_ITEM_Dryad_Shield,
            SFLAG_ITEM_Dryad_Helm,
        };
        uint8_t const* ptr = DRYAD_ITEMS;
        for(uint8_t j = 0; j < 6; ++j)
            if(user_is_wearing(i, pgm_read_byte_inc(ptr)))
                n += 5;
        if(n >= 3 * 5)
            r += n;
    }
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
    if(state == STATE_MAP)
        platform_set_game_speed_saved();
    else
        platform_set_game_speed_default();
}

void story_flag_set(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = bitmask((uint8_t)index);
    story_flags[i] |= m;
}

void story_flag_clr(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = bitmask((uint8_t)index);
    story_flags[i] &= ~m;
}

void story_flag_tog(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = bitmask((uint8_t)index);
    story_flags[i] ^= m;
}

bool story_flag_get(uint16_t index)
{
    uint8_t i = index >> 3;
    uint8_t m = bitmask((uint8_t)index);
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
    rx = x;
}

bool player_is_outside()
{
    constexpr uint16_t MAP_HALF_Y = MAP_CHUNK_ROWS / 2 * 4 * 16;
    static_assert(MAP_HALF_Y == 2048, "revert");

#ifdef ARDUINO
    uint8_t t;
    asm volatile("lds  %[t], %[py]+1\n" : [t] "=&r" (t) : [py] "i" (&py));
    return (t & 0xf8) == 0;
#else
    return (py & 0xf800) == 0;
#endif
    //return py < MAP_HALF_Y;
}

uint16_t rand_seed;

uint8_t state;
sdata_t sdata;

uint8_t nframe;
uint8_t rframe;

bool pmoving;
uint16_t selx, sely;

active_chunk_t active_chunks[4];

uint8_t btns_down, btns_pressed;

bool chunks_are_running;
uint8_t running_chunk;
uint8_t chunk_instr;

savefile_t savefile;
