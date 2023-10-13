#include "common.hpp"

#include <string.h>
#include <stddef.h>

#include "generated/fxdata.h"

uint8_t const SPRITE_FLAGS[] PROGMEM =
{
    0, // hooded figure
    SF_DONT_STOP, // magic block
    SF_DONT_STOP, // block
    SF_DONT_STOP | SF_FAST, // fast block

    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused
    0, // unused

    0, // matthias
    0, // catherine
    0, // lucy
    0, // dismas
    0, // dark guard
    0, // dark wizard
    0, // mother
    0, // old charlie
    0, // hermit
    0, // goblin
    0, // skeleton
    0, // cecilia
    SF_ALWAYS_ANIM | SF_FAST_ANIM, // psy raptor
    SF_DONT_STOP | SF_ALWAYS_ANIM | SF_FAST_ANIM | SF_FAST_ANIM2 | SF_FAST | SF_SMALL_RECT, // spike ball
    SF_ALWAYS_ANIM | SF_FAST_ANIM, // dark raptor
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static char const EN_DARK_GUARD[] PROGMEM = "Dark Guard";
static char const EN_DARK_WIZARD[] PROGMEM = "Dark Wizard";
static char const EN_SKELETON[] PROGMEM = "Skeleton";
static char const EN_PSY_RAPTOR[] PROGMEM = "Psy-Raptor";
static char const EN_DARK_RAPTOR[] PROGMEM = "Dark Raptor";

enemy_info_t const ENEMY_INFO[] PROGMEM =
{
    { 20, 4, 2, 4, 10,  64, 128, }, //EN_DARK_GUARD },
    { 21, 4, 0, 4,  6,   0, 255, }, //EN_DARK_WIZARD },
    { 26, 1, 0, 1,  6,   0,   0, }, //EN_SKELETON },
    { 28, 1, 0, 3,  3,   0,   0, }, //EN_PSY_RAPTOR },
    { 30, 3, 1, 5,  8,   0, 128, }, //EN_DARK_RAPTOR },
};

bool enemy_is_raptor(uint8_t id)
{
    return uint8_t(id - 3) <= 1;
}

static char const PN_MATTHIAS[] PROGMEM = "Matthias";
static char const PN_CATHERINE[] PROGMEM = "Catherine";
static char const PN_LUCY[] PROGMEM = "Lucy";
static char const PN_DISMAS[] PROGMEM = "Dismas";

party_info_t const PARTY_INFO[4] PROGMEM =
{
    { 16, 0, 6, 1, 0, 1, PN_MATTHIAS },
    { 17, 1, 6, 1, 0, 1, PN_CATHERINE },
    { 18, 2, 6, 1, 0, 1, PN_LUCY },
    { 19, 3, 6, 1, 0, 1, PN_DISMAS },
};

uint8_t user_item_count(uint8_t i, item_t const* items, uint8_t count)
{
    uint8_t n = 0;
    static_assert(sizeof(item_t) == 1, "change pgm_read_byte call below");
    do
    {
        if(user_is_wearing(i, pgm_read_byte_inc(items)))
            ++n;
    } while(--count != 0);
    return n;
}

static FORCE_NOINLINE int8_t party_stat(uint8_t i, uint8_t offset)
{
    uint8_t id = party[i].battle.id;
    return (int8_t)pgm_read_byte((uint8_t const*)&PARTY_INFO[id] + offset);
}

uint8_t party_att(uint8_t i)
{
    int8_t r = party_stat(i, offsetof(party_info_t, base_att));
    r += items_att(i);
    if(party[i].equipped_items[IT_ARMOR] == INVALID_ITEM)
    {
        // barbarian items
        r += user_item_count<
            SFLAG_ITEM_Barbarian_s_Axe,
            SFLAG_ITEM_Barbarian_s_Helm,
            SFLAG_ITEM_Barbarian_s_Footwraps>
            (i);
    }
    if(r < 1) r = 1;
    if(r > 99) r = 99;
    return (uint8_t)r;
}

uint8_t party_def(uint8_t i)
{
    int8_t r = party_stat(i, offsetof(party_info_t, base_def));
    r += items_def(i);
    if(user_is_wearing(i, SFLAG_ITEM_Amulet_of_Peace))
        r *= 2;
    if(r < 0) r = 0;
    if(r > 99) r = 99;
    return r;
}

uint8_t party_mhp(uint8_t i)
{
    int8_t r = party_stat(i, offsetof(party_info_t, base_mhp));
    r += items_mhp(i);
    {
        // Dryad items
        uint8_t n = user_item_count<
            SFLAG_ITEM_Dryad_Amulet,
            SFLAG_ITEM_Dryad_Ring,
            SFLAG_ITEM_Dryad_Armor,
            SFLAG_ITEM_Dryad_Shoes,
            SFLAG_ITEM_Dryad_Shield,
            SFLAG_ITEM_Dryad_Helm>
            (i);
        if(n >= 3)
            r += n * 5;
    }
    if(r < 1) r = 1;
    if(r > 99) r = 99;
    return r;
}

uint8_t party_spd(uint8_t i)
{
    int8_t r = party_stat(i, offsetof(party_info_t, base_spd));
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

#if ARDUINO_ARCH_AVR
constexpr uint16_t TS_MAP_HALF_X = MAP_CHUNK_COLS / 2 * 8 * 16;
constexpr uint16_t TS_MAP_HALF_Y = MAP_CHUNK_ROWS / 2 * 4 * 16;
static_assert(TS_MAP_HALF_X % 256 == 0, "invalid below code");
static_assert(TS_MAP_HALF_Y % 256 == 0, "invalid below code");
constexpr uint8_t TS_MAP_HALF_X_HI = uint8_t(TS_MAP_HALF_X >> 8);
constexpr uint8_t TS_MAP_HALF_Y_HI = uint8_t(TS_MAP_HALF_Y >> 8);
__attribute__((naked)) uint8_t tilesheet()
{
    asm volatile(R"ASM(
            lds  r24, %[py]+1
            cpi  r24, %[CY]
            brsh 1f
            ldi  r24, 0
            ret
        1:
            lds  r24, %[px]+1
            cpi  r24, %[CX]
            brsh 2f
            ldi  r24, 1
            ret
        2:
            ldi  r24, 2
            ret
        )ASM"
        :
        : [px] "i"   (&px),
          [py] "i"   (&py),
          [CX] ""    (TS_MAP_HALF_X_HI),
          [CY] ""    (TS_MAP_HALF_Y_HI)
        );
}
#else
uint8_t tilesheet()
{
    if(py >= MAP_CHUNK_ROWS / 2 * 64)
    {
        if(px >= MAP_CHUNK_COLS / 2 * 128)
            return 2;
        return 1;
    }
    return 0;
}
#endif

bool player_is_outside()
{
    return tilesheet() == 0;
}

void play_music()
{
    uint24_t const* ptr;
    uint8_t n;
    static uint24_t const MUSIC_PEACEFUL[] PROGMEM =
    {
        SONG_PEACEFUL,
    };
    static uint24_t const MUSIC_DUNGEON[] PROGMEM =
    {
        SONG_DUNGEON,
    };
    static uint24_t const MUSIC_TITLE[] PROGMEM =
    {
        SONG_TITLE,
    };
    static uint24_t const MUSIC_BATTLE[] PROGMEM =
    {
        SONG_BATTLE,
        SONG_BATTLE2,
    };
    static uint24_t const MUSIC_DEFEAT[] PROGMEM =
    {
        SONG_DEFEAT,
    };
    switch(savefile.music_type)
    {
    case music::peaceful:
        ptr = MUSIC_PEACEFUL;
        n = sizeof(MUSIC_PEACEFUL) / sizeof(uint24_t);
        break;
    case music::dungeon:
        ptr = MUSIC_DUNGEON;
        n = sizeof(MUSIC_DUNGEON) / sizeof(uint24_t);
        break;
    case music::title:
        ptr = MUSIC_TITLE;
        n = sizeof(MUSIC_TITLE) / sizeof(uint24_t);
        break;
    case music::battle:
        ptr = MUSIC_BATTLE;
        n = sizeof(MUSIC_BATTLE) / sizeof(uint24_t);
        break;
    case music::defeat:
        ptr = MUSIC_DEFEAT;
        n = sizeof(MUSIC_DEFEAT) / sizeof(uint24_t);
        break;
    }
    n = u8rand(n);
    ptr += n;
    uint24_t song = 0;
    song |= ((uint24_t)pgm_read_byte_inc(ptr) << 0);
    song |= ((uint24_t)pgm_read_byte_inc(ptr) << 8);
    song |= ((uint24_t)pgm_read_byte_inc(ptr) << 16);
    platform_audio_play_song(song);
}

void set_music(music m)
{
    if(savefile.music_type != m)
    {
        savefile.music_type = m;
        play_music();
    }
}

void set_music_from_position()
{
    static_assert(uint8_t(music::peaceful) == 0, "");
    static_assert(uint8_t(music::indoors ) == 1, "");
    static_assert(uint8_t(music::dungeon ) == 2, "");
    music m = music(tilesheet());
    if(m == music::indoors) m = music::peaceful;
    set_music(m);
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
