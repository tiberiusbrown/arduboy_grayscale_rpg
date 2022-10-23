#pragma once

#include <stdint.h>

#ifdef ARDUINO
#define ABG_SYNC_PARK_ROW
#define ABG_PRECHARGE_CYCLES 1
#define ABG_DISCHARGE_CYCLES 1
#include "ArduboyG.h"
extern ArduboyGBase a;
using int24_t = __int24;
using uint24_t = __uint24;
#else
#define PROGMEM
inline uint8_t pgm_read_byte(void const* p)
{
    return *(uint8_t const*)p;
}
inline uint16_t pgm_read_word(void const* p)
{
    return *(uint16_t const*)p;
}
constexpr uint8_t BLACK = 0;
constexpr uint8_t DARK_GRAY = 1;
constexpr uint8_t LIGHT_GRAY = 2;
constexpr uint8_t WHITE = 3;
using int24_t = int32_t;
using uint24_t = uint32_t;
#endif

constexpr uint8_t BTN_UP = 0x80;
constexpr uint8_t BTN_DOWN = 0x10;
constexpr uint8_t BTN_LEFT = 0x20;
constexpr uint8_t BTN_RIGHT = 0x40;
constexpr uint8_t BTN_A = 0x08;
constexpr uint8_t BTN_B = 0x04;

constexpr uint8_t MAP_CHUNK_H = 32;
constexpr uint8_t MAP_CHUNK_W = 32;
constexpr uint8_t CHUNK_SCRIPT_SIZE = 32;

struct map_chunk_t {
    // number of tiles in a chunk must match the screen
    // dimensions when using 16x16 tiles
    union {
        uint8_t tiles_flat[32];
        uint8_t tiles[4][8];
    };
    uint8_t script[CHUNK_SCRIPT_SIZE];
};

struct sdata_dialog {
    char message[256];
};

extern uint8_t nframe;

extern union {
    sdata_dialog dialog;
} sdata;

extern uint8_t pdir;    // player direction
extern bool pmoving;    // whether player is moving
extern uint16_t px, py; // player position (in pixels)

struct active_chunk_t {
    map_chunk_t chunk;
};
// 0 1
// 2 3
extern active_chunk_t active_chunks[4];
extern uint8_t loaded_cx, loaded_cy;

extern uint8_t btns_down, btns_pressed;

// platform.cpp
//     platform abstraction methods
void platform_drawoverwrite(int16_t x, int16_t y, uint8_t const* bitmap,
                            uint8_t frame);
void platform_drawoverwrite(int16_t x, int16_t y, uint8_t w, uint8_t h,
                            uint8_t const* bitmap);
void platform_drawplusmask(int16_t x, int16_t y, uint8_t const* bitmap,
                           uint8_t frame);
void platform_drawplusmask(int16_t x, int16_t y, uint8_t w, uint8_t h,
                           uint8_t const* bitmap);
void platform_fx_read_data_bytes(uint24_t addr, void* dst, size_t num);
void platform_fillrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c);
void platform_drawrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c);

// draw.cpp
void draw_text(uint8_t x, uint8_t y, char const* str);
void draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void draw_tiles();
void draw_player();

// map.cpp
bool tile_is_solid(uint16_t tx, uint16_t ty); // tx,ty in pixels
void update_chunks();

// update.cpp
void update();

// render.cpp
void render();

// init.cpp
void initialize();
