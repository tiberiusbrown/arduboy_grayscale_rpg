#include "common.hpp"

#include "generated/font_adv.hpp"
#include "generated/font_img.hpp"

#include "generated/tile_img.hpp"

#if PLAYER_IMG_IN_PROG
#include "generated/player_img.hpp"
#else
#include "generated/fxdata.h"
#endif

void draw_player()
{
    uint8_t f = pdir * 3;
    if(pmoving) {
        uint8_t t = ((nframe >> 2) & 3);
        if(t >= 2) t = (t - 2) * 2;
        f += t;
    }
#if PLAYER_IMG_IN_PROG
    platform_drawplusmask(64 - 8, 32 - 8, PLAYER_IMG, f);
#else
    platform_fx_drawplusmask(64 - 8, 32 - 8, PLAYER_IMG, f);
#endif
}

static void draw_chunk(uint8_t i, int16_t ox, int16_t oy)
{
    uint8_t* tiles = active_chunks[i].chunk.tiles_flat;
    for(uint8_t r = 0, n = 0; r < 64; r += 16) {
        int16_t y = oy + r;
        if(y <= -16 || y >= 64) {
            n += 8;
            continue;
        }
        for(uint8_t c = 0; c < 128; c += 16, ++n) {
            int16_t x = ox + c;
            platform_drawoverwrite(x, y, TILE_IMG, tiles[n]);
        }
    }
}

void draw_tiles()
{
    uint16_t tx = px - 64;
    uint16_t ty = py - 32;
    uint8_t cx = uint8_t(tx >> 7);
    uint8_t cy = uint8_t(ty >> 6);
    int16_t ox = -int16_t(tx & 0x7f);
    int16_t oy = -int16_t(ty & 0x3f);
    draw_chunk(0, ox, oy);
    draw_chunk(1, ox + 128, oy);
    draw_chunk(2, ox, oy + 64);
    draw_chunk(3, ox + 128, oy + 64);
}

void draw_text(uint8_t x, uint8_t y, char const* str)
{
    char t;
    uint8_t cx = x;
    while((t = *str++) != '\0') {
        if(t == '\n') {
            y += 9;
            cx = x;
            continue;
        }
        t -= ' ';
        uint8_t const* bitmap = &FONT_IMG[t * 24];
        platform_drawplusmask(cx, y, 8, 8, bitmap);
        cx += pgm_read_byte(&FONT_ADV[t]);
    }
}

void wrap_text(char* str, uint8_t w)
{
    uint8_t i = 0;
    uint8_t x = 0;
    char t;
    while((t = str[i++]) != '\0') {
        if(t == '\n') {
            x = 0;
            continue;
        }
        t -= ' ';
        x += pgm_read_byte(&FONT_ADV[t]);
        if(x > w) {
            --i;
            while(t != ' ' && i != 0)
                t = str[--i];
            if(i != 0) str[i] = '\n';
        }
    }
}

void draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    platform_fillrect(x, y, w, h, BLACK);
    platform_drawrect(x, y, w, h, WHITE);
    return;
}
