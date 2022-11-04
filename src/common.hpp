#pragma once

#define TILES_IN_PROG 1
#define TELEPORT_TRANSITION_FRAMES 16

#include <stdint.h>

#include "generated/story_flags.hpp"

#ifdef ARDUINO
#define ABG_SYNC_PARK_ROW
#define ABG_UPDATE_EVERY_N_DEFAULT 2
#define ABG_PRECHARGE_CYCLES 1
#define ABG_DISCHARGE_CYCLES 1
#define ABG_FPS_DEFAULT 120
#include "ArduboyG.h"
extern ArduboyGBase a;
using int24_t = __int24;
using uint24_t = __uint24;
inline uint8_t plane()
{
    return a.currentPlane();
}
#else
#define PROGMEM
inline char const* PSTR(char const* str)
{
    return str;
}
inline uint8_t pgm_read_byte(void const* p)
{
    return *(uint8_t*)p;
}
inline uint16_t pgm_read_word(void const* p)
{
    return *(uint16_t*)p;
}
inline void const* pgm_read_ptr(void const* p)
{
    return *(void const**)p;
}
constexpr uint8_t BLACK = 0;
constexpr uint8_t DARK_GRAY = 1;
constexpr uint8_t LIGHT_GRAY = 2;
constexpr uint8_t WHITE = 3;
using int24_t = int32_t;
using uint24_t = uint32_t;
extern int gplane;
inline uint8_t plane()
{
    return (uint8_t)gplane;
}
#endif

// useful when T is a pointer type, like function pointer or char const*
template <class T> inline T pgmptr(T const* p)
{
    return (T)pgm_read_ptr(p);
}

template<class T> void tswap(T& a, T& b)
{
    T t = a;
    a = b;
    b = t;
}

constexpr uint8_t BTN_UP = 0x80;
constexpr uint8_t BTN_DOWN = 0x10;
constexpr uint8_t BTN_LEFT = 0x20;
constexpr uint8_t BTN_RIGHT = 0x40;
constexpr uint8_t BTN_A = 0x08;
constexpr uint8_t BTN_B = 0x04;

constexpr uint8_t MAP_CHUNK_H = 32;
constexpr uint8_t MAP_CHUNK_W = 32;
constexpr uint8_t CHUNK_SCRIPT_SIZE = 64;
constexpr uint8_t CHUNK_SPRITE_PATH_SIZE = 8;

struct map_chunk_t
{
    // number of tiles in a chunk must match the screen
    // dimensions when using 16x16 tiles
    union
    {
        uint8_t tiles_flat[32];
        uint8_t tiles[4][8];
    };
    uint8_t script[CHUNK_SCRIPT_SIZE];
};

extern uint8_t nframe;

enum
{
    STATE_MAP,    // moving around on the map
    STATE_DIALOG, // message or dialog
    STATE_TP,     // player is teleporting (e.g., entering building or cave)
    STATE_BATTLE,
};
extern uint8_t state;

extern bool chunks_are_running;
extern uint8_t running_chunk;
extern uint8_t chunk_instr;
extern int8_t chunk_regs[16];

struct sdata_dialog
{
    uint8_t portrait;
    uint8_t char_progress;
    char message[254];
};
struct sdata_tp
{
    uint16_t tx, ty;
    uint8_t frame;
};

struct party_member_t
{
    uint8_t id;
    uint8_t hp;
    uint8_t ap;
};
extern party_member_t party[4];
extern uint8_t nparty;

struct enemy_info_t
{
    uint8_t sprite;
    uint8_t speed;
};
extern enemy_info_t const ENEMY_INFO[] PROGMEM;

enum battle_phase_t
{
    BPHASE_ALERT,   // '!' over player
    BPHASE_INTRO,   // fancy "Battle Start!"
    BPHASE_MENU,
    BPHASE_ESEL,    // select enemy
    BPHASE_PATTACK, // party attack animation
    BPHASE_EATTACK, // enemy attack animation
    BPHASE_OUTRO,   // fancy "Victory!"
};
struct sdata_battle
{
    uint8_t frame;
    uint16_t flag;
    bool remove_enemy;
    uint8_t enemy_chunk;
    party_member_t enemies[4];
    uint8_t pdef, edef; // party/enemy defender (-1 for none)
    uint8_t esel;       // enemy select
    uint8_t psel;       // party member select
    uint8_t msel;       // menu select
    uint8_t msely;
    int8_t menuy;       // menu position
    int8_t menuy_target;
    battle_phase_t phase;
    battle_phase_t prev_phase;
    battle_phase_t next_phase;
    uint8_t attack_order[8];
    uint8_t num_attackers;
    uint8_t current_attacker;
};
extern union sdata_t
{
    sdata_dialog dialog;
    sdata_tp tp;
    sdata_battle battle;
} sdata;
void change_state(uint8_t new_state);

extern uint8_t pdir;        // player direction
extern bool pmoving;        // whether player is moving
extern uint16_t px, py;     // player position (in pixels)
extern uint16_t selx, sely; // selected tile

extern uint8_t story_flags[STORY_FLAG_BYTES];
void story_flag_set(uint16_t index);
void story_flag_clr(uint16_t index);
void story_flag_tog(uint16_t index);
bool story_flag_get(uint16_t index);

struct enemy_t
{
    uint8_t x, y, dir;
    uint8_t type;
    uint8_t path_num;
    // path byte:
    //     bits 0-5: destination tile (0-31)
    //     bits 5-7: delay cycles
    uint8_t path[CHUNK_SPRITE_PATH_SIZE];
    uint8_t path_index; // index of current destination
    uint8_t frames_rem;
    bool active;
};

struct active_chunk_t
{
    map_chunk_t chunk;
    uint8_t cx, cy;
    enemy_t enemy;
};
// 0 1
// 2 3
extern active_chunk_t active_chunks[4];

extern uint8_t btns_down, btns_pressed;

// platform.cpp
//     platform abstraction methods
void platform_drawoverwrite(int16_t x, int16_t y, uint8_t const* bitmap,
    uint8_t frame);
void platform_drawoverwritemonochrome(int16_t x, int16_t y, uint8_t w,
    uint8_t h, uint8_t const* bitmap);
void platform_fx_read_data_bytes(uint24_t addr, void* dst, size_t num);
void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h);
void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h);
void platform_fillrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c);
void platform_drawrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c);
#if 0
void platform_fx_clear_flag(uint16_t index);
bool platform_fx_get_flag(uint16_t index);
void platform_fx_erase_save_sector(uint16_t page);
void platform_fx_write_save_page(uint16_t page, void const* data);
void platform_fx_read_save_page(uint16_t page, void* data);
bool platform_fx_busy();
#endif

// draw.cpp
void draw_tile(int16_t x, int16_t y, uint8_t t);
void draw_text(uint8_t x, uint8_t y, char const* str);      // str in RAM
void draw_text_prog(uint8_t x, uint8_t y, char const* str); // str in PROGMEM
void wrap_text(char* str, uint8_t w); // replace ' ' with '\n' to wrap to width
void draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void draw_tiles();
void draw_player();
void draw_sprites();
struct draw_sprite_entry
{
    uint24_t addr;
    uint8_t frame;
    int16_t x, y;
};
void sort_and_draw_sprites(draw_sprite_entry* entries, uint8_t n);

// map.cpp
bool enemy_contacts_player(active_chunk_t const& c);
bool tile_is_solid(uint16_t tx, uint16_t ty); // tx,ty in pixels
void load_chunks();
bool run_chunks(); // returns true if state interrupt occurred

// update.cpp
void update();

// render.cpp
void render();

// battle.cpp
void update_battle();
void render_battle();

// init.cpp
void initialize();
