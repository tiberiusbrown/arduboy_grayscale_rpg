#pragma once

#include <stdint.h>
#include <string.h>

constexpr uint16_t VERSION = 1;

constexpr uint8_t TELEPORT_TRANSITION_FRAMES = 16;
constexpr uint8_t FADE_SPEED = 2;

constexpr uint8_t MAP_CHUNK_COLS = 32;
constexpr uint8_t MAP_CHUNK_ROWS = 64;
constexpr uint8_t CHUNK_SCRIPT_SIZE = 80;
constexpr uint8_t CHUNK_SPRITE_PATH_SIZE = 8;

constexpr uint8_t EXPLORED_SCALE = 2; // pixels per tile
constexpr uint8_t EXPLORED_TILES = 8; // tiles per explored square
constexpr uint8_t EXPLORED_COLS = MAP_CHUNK_COLS * 8 / EXPLORED_TILES;     // squares wide
constexpr uint8_t EXPLORED_ROWS = MAP_CHUNK_ROWS / 2 * 4 / EXPLORED_TILES; // squares high
constexpr uint8_t EXPLORED_PIXELS = EXPLORED_SCALE * EXPLORED_TILES;
constexpr uint8_t EXPLORED_CHUNK_W = 128 / EXPLORED_PIXELS;
constexpr uint8_t EXPLORED_CHUNK_H = 64 / EXPLORED_PIXELS;
constexpr uint8_t EXPLORED_CHUNK_COLS = EXPLORED_COLS / EXPLORED_CHUNK_W;
constexpr uint8_t EXPLORED_CHUNK_ROWS = EXPLORED_ROWS / EXPLORED_CHUNK_H;
constexpr uint8_t EXPLORED_BYTES = EXPLORED_COLS * EXPLORED_ROWS / 8; // bytes to store all squares
constexpr uint8_t EXPLORED_CHUNK_BYTES = EXPLORED_BYTES / (EXPLORED_CHUNK_COLS * EXPLORED_CHUNK_ROWS);

constexpr uint8_t NUM_SCORE_CHANNELS = 2;

#define DEBUG_LIPO_DISCHARGE 0
#ifdef ARDUINO
#define RECORD_LIPO_DISCHARGE 0
#else
#define RECORD_LIPO_DISCHARGE 0
#endif

#ifdef ARDUINO
#define DETECT_FX_CHIP 0
#else
#define DETECT_FX_CHIP 0
#endif

#include "generated/story_flags.hpp"
#include "generated/num_items.hpp"

#ifdef ARDUINO
#include <ArduboyFX.h>
#include "src/ATMlib2/ATMlib2.h"
#define ABG_TIMER1
#define ABG_SYNC_PARK_ROW
#define ABG_UPDATE_EVERY_N_MOD 11
#define ABG_UPDATE_EVERY_N_DENOM_MOD 7
#define ABG_UPDATE_EVERY_N_DEFAULT 1
#define ABG_UPDATE_EVERY_N_DENOM_DEFAULT 1
#define ABG_PRECHARGE_CYCLES 1
#define ABG_DISCHARGE_CYCLES 2
#define ABG_FPS_DEFAULT 156
#include "ArduboyG.h"
#define FORCE_INLINE __attribute__((always_inline))
#define FORCE_NOINLINE __attribute__((noinline))
extern ArduboyGBase_Config<ABG_Mode::L4_Triplane> a;
using int24_t = __int24;
using uint24_t = __uint24;
FORCE_INLINE inline uint8_t plane()
{
    return a.currentPlane();
}
#define MY_ASSERT(cond__) (void)0
template<class T>
FORCE_INLINE inline uint8_t pgm_read_byte_inc(T const*& p)
{
    uint8_t r;
    asm volatile("lpm %[r], %a[p]+\n" : [p] "+&z" (p), [r] "=&r" (r));
    return r;
}
template<class T>
FORCE_INLINE inline uint8_t deref_inc(T const*& p)
{
    uint8_t r;
    asm volatile("ld %[r], %a[p]+\n" : [p] "+&e" (p), [r] "=&r" (r) :: "memory");
    return r;
}
FORCE_INLINE inline uint8_t bitmask(uint8_t x) { return FX::bitShiftLeftUInt8(x); }
FORCE_INLINE inline int16_t fmuls(int8_t x, int8_t y) { return __builtin_avr_fmuls(x, y); }
FORCE_INLINE inline uint8_t nibswap(uint8_t x) { return __builtin_avr_swap(x); }
#else
#include <assert.h>
#define MY_ASSERT(cond__) assert(cond__)
#define PROGMEM
inline char const* PSTR(char const* str)
{
    return str;
}
inline uint8_t pgm_read_byte(void const* p)
{
    return *(uint8_t const*)p;
}
template<class T>
inline uint8_t pgm_read_byte_inc(T const*& p)
{
    uint8_t r = *(uint8_t const*)p;
    p = (T const*)((uint8_t const*)p + 1);
    return r;
}
template<class T>
inline uint8_t deref_inc(T const*& p)
{
    uint8_t r = *(uint8_t const*)p;
    p = (T const*)((uint8_t const*)p + 1);
    return r;
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
#define FORCE_INLINE
#define FORCE_NOINLINE
inline uint8_t bitmask(uint8_t x) { return 1 << (x & 7); }
inline void* memcpy_P(void* dst, void const* src, size_t num)
{
    return memcpy(dst, src, num);
}
inline int16_t fmuls(int8_t x, int8_t y) { return (x * y) << 1; }
inline uint8_t nibswap(uint8_t x) { return (x >> 4) | (x << 4); }
#endif

constexpr uint8_t PLANES = 3;
constexpr uint8_t INVALID = -1;

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

extern uint8_t nframe; // update frame
extern uint8_t rframe; // render frame

enum
{
    STATE_TITLE,
    STATE_RESUME,  // fading in to gameplay
    STATE_MAP,     // moving around on the map
    STATE_PAUSE,   // pause menu
    STATE_DIALOG,  // message or dialog
    STATE_TP,      // player is teleporting (e.g., entering building or cave)
    STATE_BATTLE,
    STATE_DIE,
    STATE_GAME_OVER,
};
extern uint8_t state;

extern bool chunks_are_running;
extern uint8_t running_chunk;
extern uint8_t chunk_instr;

struct sdata_title
{
    uint8_t fade_frame;
    bool going_to_resume;
    bool no_fx_chip;

    bool    path_started;
    uint8_t path_index;
    uint8_t path_dir;
    uint8_t path_frames;
};
struct sdata_resume
{
    uint8_t fade_frame;
};
struct sdata_dialog
{
    uint8_t char_progress;
    bool question;
    bool questiondraw;
    uint8_t question_msg;
    uint8_t questioni;
    uint8_t questioniy;
    uint8_t numquestions;
    bool questiondone;
    bool questionpause;
    uint8_t questionfill;
    uint8_t questionfillw;
    char name[20];
    char message[220];
};
struct sdata_tp
{
    static_assert(MAP_CHUNK_COLS <= 32, "expand to 16-bit");
    static_assert(MAP_CHUNK_ROWS <= 64, "expand to 16-bit");
    uint8_t tx, ty;
    uint8_t frame;
};

enum {
    IT_WEAPON,
    IT_SHIELD,
    IT_ARMOR,
    IT_HELM,
    IT_SHOES,
    IT_RING,
    IT_AMULET,
    
    IT_CONSUMABLE,
    IT_NUM_CATS
};
struct item_info_t
{
    // modifiers
    int8_t att, def, spd, mhp;
    uint8_t type;
};
extern item_info_t const ITEM_INFO[] PROGMEM;

constexpr size_t ITEM_BYTES = (NUM_ITEMS + 7) / 8;
static_assert(NUM_ITEMS <= 255, "revisit item_id");
using item_t = uint8_t;
constexpr item_t INVALID_ITEM = NUM_ITEMS;

struct sdata_items
{
    uint8_t user_index;  // who is using an item?
    uint8_t n;           // offset selected
    uint8_t off;         // list offset
    uint8_t x, xt;       // page offset / target
    uint8_t pcat;        // previous category
    uint8_t cat;         // current category
    uint8_t cat_nums[IT_NUM_CATS]; // number of items in each category
    item_t  item_count;  // total number of items
    uint8_t consw;
    uint8_t consfill;
    uint8_t conspause;
    bool battle;         // whether we are in battle
    char str[ITEM_TOTAL_LEN];
};
struct sdata_pause
{
    uint8_t state;
    uint8_t menuy;
    uint8_t menui;
    uint8_t ax, bx;

    uint8_t optionsy;
    uint8_t optionsi;
    uint8_t optionsiy;
    uint8_t brightnessx;
    uint8_t speedx;

    uint8_t quity;
    uint8_t quiti;
    uint8_t quitiy;
    uint8_t quitf;
    uint8_t quitft;
    bool quitp;
    uint8_t quitfade;

    uint8_t savey;
    uint8_t save_wait;

    uint8_t partyy;
    uint8_t partyi;
    uint8_t partyx;
    uint8_t partyxt;

    uint8_t ally;

    sdata_items items;
    uint8_t itemsy;
    uint8_t itemsyt;

    uint8_t mapfade;
    bool map_first;
    bool back_to_menu;
    bool allow_obj;
    int16_t mapscrollx;
    int16_t mapscrolly;
};
constexpr auto SIZEOF_PAUSE_SDATA = sizeof(sdata_pause);

struct battle_member_t
{
    uint8_t id;
    uint8_t hp;
};

struct enemy_info_t
{
    uint8_t sprite;
    uint8_t att;
    uint8_t def;
    uint8_t spd;
    uint8_t mhp;
    uint8_t defend;         // chance to defend
    uint8_t target_weakest; // chance to target lowest hp party member
    char const* name;
};
extern enemy_info_t const ENEMY_INFO[] PROGMEM;

struct party_info_t
{
    uint8_t sprite;
    uint8_t portrait;
    uint8_t base_mhp;
    uint8_t base_att;
    uint8_t base_def;
    uint8_t base_spd;
    char const* name;
};
extern party_info_t const PARTY_INFO[4] PROGMEM;

uint8_t party_att(uint8_t i);
uint8_t party_def(uint8_t i);
uint8_t party_mhp(uint8_t i);
uint8_t party_spd(uint8_t i);
void party_clip_hp();

enum battle_phase_t
{
    BPHASE_ALERT,   // battle alert animation
    BPHASE_INTRO,   // fancy "Battle Start!"
    BPHASE_NEXT,
    BPHASE_MENU,
    BPHASE_ESEL,    // select enemy
    BPHASE_ATTACK1, // attack animation (charge)
    BPHASE_ATTACK2, // attack animation (damage)
    BPHASE_ATTACK3, // attack animation (return)
    BPHASE_DEFEND,  // move to defense
    BPHASE_ITEM,    // consume an item
    BPHASE_SPRITES, // wait until sprites are done
    BPHASE_DELAY,   // delay until frame == 0 (set frame to -N)
    BPHASE_DEFEAT,
    BPHASE_OUTRO,
};
struct battle_sprite_t
{
    bool active;
    uint8_t damaged;
    int8_t x, y;   // current pos
    uint8_t tx, ty; // target pos
    uint8_t bx, by; // base pos
    uint8_t move_speed;
    uint16_t frame_base;
    uint8_t frame_dir;
    uint8_t hp;     // health bar width
    uint8_t hpt;    // health bar width target
};
struct sdata_battle
{
    uint8_t frame;
    //uint8_t selframe;
    uint16_t flag;
    bool remove_enemy;
    uint8_t enemy_chunk;
    battle_member_t enemies[4];
    uint8_t pdef, edef; // party/enemy defender (-1 for none)

    uint8_t esel;       // enemy select
    uint8_t psel;       // party member select
    uint8_t msel;       // menu select
    uint8_t msely;
    uint8_t menuy;       // menu position
    uint8_t menuy_target;

    battle_phase_t phase;
    battle_phase_t prev_phase;
    battle_phase_t next_phase;

    uint8_t attack_order[9];
    uint8_t num_attackers;
    uint8_t attacker_index;
    uint8_t attacker_id;
    uint8_t defender_id;

    battle_sprite_t sprites[8];
    bool sprites_done;

    sdata_items items;
    uint8_t itemsy;
};
struct sdata_game_over
{
    uint8_t fade_frame;
    uint8_t msg_frame;
    uint8_t msg_lines;
    bool going_to_resume;
    char msg[128];
};
struct sdata_map
{
    uint8_t a_pressed;
};
struct sdata_die
{
    uint8_t frame;
};
extern union sdata_t
{
    sdata_map map;
    sdata_title title;
    sdata_resume resume;
    sdata_dialog dialog;
    sdata_tp tp;
    sdata_battle battle;
    sdata_die die;
    sdata_game_over game_over;
    sdata_pause pause;
} sdata;
constexpr auto SIZEOF_SDATA = sizeof(sdata);
//static_assert(SIZEOF_SDATA <= 256, "state data too large");
void change_state(uint8_t new_state);

extern bool pmoving;        // whether player is moving
extern uint16_t selx, sely; // selected tile

struct sprite_t
{
    uint8_t x, y, dir;
    uint8_t type;
    uint8_t path_num;
    // path byte:
    //     bits 0-5: destination tile (0-31)
    //     bits 5-7: delay cycles
    uint8_t path[CHUNK_SPRITE_PATH_SIZE];
    uint8_t path_index; // index of current destination
    uint8_t path_dir; // 0: closed loop, 1: forward, 2: backward
    uint8_t frames_rem;
    bool active;
    bool walking;
};

struct active_chunk_t
{
    map_chunk_t chunk;
    uint8_t cx, cy;
};
// 0 1
// 2 3
extern active_chunk_t active_chunks[4];

struct party_member_t
{
    battle_member_t battle;
    item_t equipped_items[IT_NUM_CATS - 1];
};

struct settings_t
{
    uint8_t sound;
    uint8_t brightness;
    uint8_t game_speed;
    bool no_battery_alert;
};

struct savefile_t
{
    uint16_t checksum;
    uint8_t identifier[8];
    bool loaded;
    uint8_t objx, objy;  // objective marker position
    uint16_t px, py;     // player position (in pixels)
    uint8_t pdir;        // player direction
    uint8_t nparty;
    party_member_t party[4];
    uint8_t story_flags[STORY_FLAG_BYTES];
    settings_t settings;
    int8_t chunk_regs[8 + NUM_CONSUMABLES];
    sprite_t chunk_sprites[4];
    uint8_t explored[EXPLORED_BYTES];
};
extern savefile_t savefile;
static_assert(sizeof(savefile.chunk_regs) <= 16, "revisit reg command encoding");

static auto& px = savefile.px;
static auto& py = savefile.py;
static auto& pdir = savefile.pdir;
static auto& party = savefile.party;
static auto& nparty = savefile.nparty;
static auto& story_flags = savefile.story_flags;
static auto& chunk_sprites = savefile.chunk_sprites;
constexpr auto* consumables = &savefile.chunk_regs[8];

constexpr auto SIZEOF_SAVEFILE = sizeof(savefile);

bool player_is_outside();

void story_flag_set(uint16_t index);
void story_flag_clr(uint16_t index);
void story_flag_tog(uint16_t index);
bool story_flag_get(uint16_t index);

extern uint8_t btns_down, btns_pressed;

extern uint16_t rand_seed;
uint8_t u8rand();
uint8_t u8rand(uint8_t m);

void adjust(uint8_t& x, uint8_t tx);

// battery.cpp
extern struct battery_info_t
{
    int16_t  raw, r32, r, dr, ddr;
    uint8_t  stage;
    bool low;
} battery;
void update_battery();

// pause.cpp
enum
{
    OS_MENU,
    OS_RESUMING,
    OS_OPTIONS,
    OS_QUIT,
    OS_SAVE,
    OS_PARTY,
    OS_MAP,
};
void update_pause();
void render_pause();

// pause_party.cpp
void update_pause_party();
void render_pause_party();

// save.cpp
void save_begin();
bool save_done();
bool is_saving();
void load(bool first);
uint16_t compute_checksum();

// platform.cpp
//     platform abstraction methods
void platform_fade(uint8_t f);
void platform_drawoverwrite(int16_t x, int16_t y, uint8_t const* bitmap,
    uint8_t frame);
void platform_drawoverwritemonochrome(int16_t x, int16_t y, uint8_t w,
    uint8_t h, uint8_t const* bitmap);
// this method assumes the image never clips outside the display
// and can be much faster because of that assumption
void platform_drawcharfast(
    uint8_t x, uint8_t page_start, uint8_t shift_coef,
    uint8_t w, uint16_t shift_mask, uint8_t const* bitmap);
void platform_fx_read_data_bytes(uint24_t addr, void* dst, size_t num);
void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h);
void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame);
void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr) FORCE_NOINLINE;
void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h);
void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame);
void platform_fillrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c);
void platform_fillrect_i8(int8_t x, int8_t y, uint8_t w, uint8_t h, uint8_t c) FORCE_NOINLINE;
void platform_drawrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c);
void platform_drawrect_i8(int8_t x, int8_t y, uint8_t w, uint8_t h, uint8_t c) FORCE_NOINLINE;
void platform_fx_erase_save_sector();
void platform_fx_write_save_page(uint16_t page, void const* data, size_t num);
void platform_fx_read_save_bytes(uint24_t addr, void* data, size_t num);
bool platform_fx_busy();
void platform_audio_init();
void platform_audio_deinit();
void platform_audio_on();
void platform_audio_off();
void platform_audio_toggle();
void platform_audio_from_savefile(); // update on/off from savefile.sound
bool platform_audio_enabled();
void platform_audio_play_song(uint24_t song); // set loop song
void platform_audio_play_song_now(uint24_t song); // force stop music and change to song
void platform_audio_play_song_now_once(uint24_t song); // force stop music and play song
void platform_audio_pause_song();
void platform_audio_resume_song();
void platform_audio_stop_sfx();
bool platform_audio_song_playing();
bool platform_audio_sfx_playing();
void platform_audio_play_sfx(uint24_t sfx, uint8_t channel);
void platform_audio_update();
void platform_set_game_speed(uint8_t num, uint8_t denom);
void platform_set_game_speed_default();
void platform_set_game_speed_saved();

// draw.cpp
void draw_objective(int16_t diffx, int16_t diffy);
void draw_tile(int16_t x, int16_t y, uint8_t t);
void draw_text(int16_t x, int16_t y, char const* str);      // str in RAM
void draw_text_prog(int16_t x, int16_t y, char const* str); // str in PROGMEM
// this method assumes the text never clips outside the display
// and is much faster because of that assumption (str in RAM)
constexpr uint8_t NOCLIPFLAG_BIGLINES = 1;
constexpr uint8_t NOCLIPFLAG_PROG = 2;
void draw_text_noclip(int8_t x, int8_t y, char const* str, uint8_t f = 0);
void draw_dec(int8_t x, int8_t y, uint8_t val);
uint8_t dec_to_str(char* dst, uint8_t val);
void wrap_text(char* str, uint8_t w); // replace ' ' with '\n' to wrap to width
uint8_t text_width(char const* str);
uint8_t text_width_prog(char const* str);
void draw_frame_white(int16_t x, int16_t y, uint8_t w, uint8_t h);
void draw_rounded_frame_white(int16_t x, int16_t y, uint8_t w, uint8_t h);
void draw_frame_black(int16_t x, int16_t y, uint8_t w, uint8_t h);
void draw_rounded_frame_black(int16_t x, int16_t y, uint8_t w, uint8_t h);
void draw_tiles();
void draw_player();
void draw_sprites();
struct draw_sprite_entry
{
    uint24_t addr;
    uint8_t frame;
    int16_t x, y;
};
void sort_sprites(draw_sprite_entry* entries, uint8_t n);
void sort_and_draw_sprites(draw_sprite_entry* entries, uint8_t n);

// map.cpp
bool sprite_contacts_player(active_chunk_t const& c, sprite_t const& e);
bool check_solid(uint16_t tx, uint16_t ty); // tx,ty in pixels
void load_chunks();
bool run_chunks(); // returns true if state interrupt occurred

// update.cpp
void back_to_map();
void update();

// render.cpp
void render_map();
void render();

// battle.cpp
void update_battle();
void render_battle();

// items.cpp
void use_consumable(uint8_t user, uint8_t i);
bool user_is_wearing(uint8_t user, item_t i);
int8_t items_att(uint8_t user);
int8_t items_def(uint8_t user);
int8_t items_spd(uint8_t user);
int8_t items_mhp(uint8_t user);
void update_items_numcat(sdata_items& d);
bool update_items(sdata_items& d); // returns true in battle mode after consuming item
void render_items(int8_t y, sdata_items& d);
void toggle_item(uint8_t user, item_t i);

// init.cpp
void initialize();
void new_game();

inline uint8_t div16_u16(uint16_t x)
{
#ifdef ARDUINO
    asm volatile(R"ASM(
        swap %A[x]
        andi %A[x], 0x0f
        swap %B[x]
        andi %B[x], 0xf0
        or   %A[x], %B[x]
        )ASM"
        : [x] "+&d" (x)
        );
    return (uint8_t)x;
#else
    return uint8_t(x >> 4);
#endif
}

inline uint8_t div16(uint8_t x)
{
#ifdef ARDUINO
    asm volatile(R"ASM(
        swap %[x]
        andi %[x], 0x0f
        )ASM"
        : [x] "+&d" (x)
        );
    return x;
#else
    return x >> 4;
#endif
}

FORCE_INLINE inline uint8_t div8(uint8_t x)
{
#ifdef ARDUINO
    asm volatile(R"ASM(
        lsr %[x]
        lsr %[x]
        lsr %[x]
        )ASM"
        : [x] "+&r" (x)
        );
    return x;
#else
    return x >> 3;
#endif
}
