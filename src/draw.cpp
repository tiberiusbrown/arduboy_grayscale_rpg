#include "common.hpp"

#include "font_adv.hpp"
#include "generated/font_img.hpp"
#include "generated/fxdata.h"
#include "generated/tile_img.hpp"
#include "generated/rounded_borders_white_img.hpp"
#include "generated/rounded_borders_black_img.hpp"

static uint8_t add_sprite_entry(draw_sprite_entry* entry, uint8_t ci,
                                      int16_t ox, int16_t oy)
{
    auto const& e = active_chunks[ci].enemy;
    if(!e.active) return 0;
    uint8_t f = e.type * 16;
    uint8_t d = e.dir;
    if(!(d & 0x80))
    {
        f += d * 2;
        if(state == STATE_MAP)
            f += ((nframe >> 2) & 3);
    }
    entry->addr = SPRITES_IMG;
    entry->frame = f;
    entry->x = ox + e.x;
    entry->y = oy + e.y - 4;
    return 1;
}

void sort_sprites(draw_sprite_entry* entries, uint8_t n)
{
    for(uint8_t i = 1; i < n; ++i)
    {
        for(uint8_t j = i; j > 0 && entries[j - 1].y > entries[j].y; --j)
        {
            auto t = entries[j];
            entries[j] = entries[j - 1];
            entries[j - 1] = t;
        }
    }
}

void sort_and_draw_sprites(draw_sprite_entry* entries, uint8_t n)
{
    sort_sprites(entries, n);

    for(uint8_t i = 0; i < n; ++i)
    {
        platform_fx_drawplusmask(entries[i].x, entries[i].y, entries[i].addr,
                                 entries[i].frame, 16, 16);
    }
}

void draw_sprites()
{
    draw_sprite_entry entries[5]; // increase as necessary
    uint8_t n = 0;

    // player sprite
    {
        uint8_t f = pdir * 4;
        if(pmoving) f += ((nframe >> 2) & 3);
        entries[n++] = {PLAYER_IMG, f, 64 - 8, 32 - 8 - 4};
    }

    // chunk enemies
    {
        uint16_t tx = px - 64 + 8;
        uint16_t ty = py - 32 + 8;
        uint8_t cx = uint8_t(tx >> 7);
        uint8_t cy = uint8_t(ty >> 6);
        int16_t ox = -int16_t(tx & 0x7f);
        int16_t oy = -int16_t(ty & 0x3f);
        for(uint8_t i = 0; i < 4; ++i)
        {
            uint8_t dox = (i &  1) * 128;
            uint8_t doy = (i >> 1) * 64;
            n += add_sprite_entry(&entries[n], i, ox + dox, oy + doy);
        }
    }

    sort_and_draw_sprites(entries, n);
}

void draw_player()
{
    uint8_t f = pdir * 4;
    if(pmoving) f += ((nframe >> 2) & 3);
    platform_fx_drawplusmask(64 - 8, 32 - 8 - 4, PLAYER_IMG, f, 16, 16);
}

void draw_tile(int16_t x, int16_t y, uint8_t t)
{
    if(t < 64)
        platform_drawoverwrite(x, y, TILE_IMG_PROG, t);
    else
        platform_fx_drawoverwrite(x, y, TILE_IMG, t - 64, 16, 16);
}

static void draw_chunk_tiles(uint8_t i, int16_t ox, int16_t oy)
{
    auto const& ac = active_chunks[i];
    uint8_t const* tiles = ac.chunk.tiles_flat;
    for(uint8_t r = 0, n = 0; r < 64; r += 16)
    {
        int16_t y = oy + r;
        if(y <= -16 || y >= 64)
        {
            n += 8;
            continue;
        }
        if(state == STATE_DIALOG && y >= 35) break;
        for(uint8_t c = 0; c < 128; c += 16, ++n)
        {
            int16_t x = ox + c;
            draw_tile(x, y, tiles[n]);
        }
    }
}

void draw_tiles()
{
    uint16_t tx = px - 64 + 8;
    uint16_t ty = py - 32 + 8;
    uint8_t cx = uint8_t(tx >> 7);
    uint8_t cy = uint8_t(ty >> 6);
    int16_t ox = -int16_t(tx & 0x7f);
    int16_t oy = -int16_t(ty & 0x3f);
    draw_chunk_tiles(0, ox, oy);
    draw_chunk_tiles(1, ox + 128, oy);
    draw_chunk_tiles(2, ox, oy + 64);
    draw_chunk_tiles(3, ox + 128, oy + 64);
}

static void draw_text_ex(uint8_t x, uint8_t y, char const* str, bool prog)
{
    char t;
    uint8_t cx = x;
    while((t = (prog ? (char)pgm_read_byte(str++) : *str++)) != '\0')
    {
        if(t == '\n')
        {
            y += 9;
            cx = x;
            continue;
        }
        t -= ' ';
        uint8_t const* bitmap = &FONT_IMG[t * 16];
        uint8_t adv = pgm_read_byte(&FONT_ADV[t]);
        platform_drawoverwritemonochrome(cx, y, adv, 8, bitmap + plane() * 8);
        cx += adv;
    }
}

void draw_text(uint8_t x, uint8_t y, char const* str)
{
    draw_text_ex(x, y, str, false);
}

void draw_text_prog(uint8_t x, uint8_t y, char const* str)
{
    draw_text_ex(x, y, str, true);
}

static uint8_t text_width_ex(char const* str, bool prog)
{
    uint8_t w = 0;
    char t;
    while((t = (prog ? (char)pgm_read_byte(str++) : *str++)) != '\0')
        w += pgm_read_byte(&FONT_ADV[t - ' ']);
    return w;
}

uint8_t text_width(char const* str)
{
    return text_width_ex(str, false);
}

uint8_t text_width_prog(char const* str)
{
    return text_width_ex(str, true);
}

void wrap_text(char* str, uint8_t w)
{
    uint8_t i = 0;
    uint8_t x = 0;
    char t;
    while((t = str[i++]) != '\0')
    {
        if(t == '\n')
        {
            x = 0;
            continue;
        }
        t -= ' ';
        x += pgm_read_byte(&FONT_ADV[t]);
        if(x > w)
        {
            --i;
            while(t != ' ' && i != 0)
                t = str[--i];
            if(i != 0) str[i] = '\n';
        }
    }
}

void draw_frame_white(int16_t x, int16_t y, uint8_t w, uint8_t h)
{
    platform_fillrect(x, y, w, h, BLACK);
    platform_drawrect(x, y, w, h, WHITE);
    return;
}

void draw_rounded_frame_white(int16_t x, int16_t y, uint8_t w, uint8_t h)
{
    draw_frame_white(x, y, w, h);
    platform_drawoverwrite(x        , y        , ROUNDED_BORDERS_WHITE_IMG_PROG, 0);
    platform_drawoverwrite(x + w - 3, y        , ROUNDED_BORDERS_WHITE_IMG_PROG, 1);
    platform_drawoverwrite(x        , y + h - 8, ROUNDED_BORDERS_WHITE_IMG_PROG, 2);
    platform_drawoverwrite(x + w - 3, y + h - 8, ROUNDED_BORDERS_WHITE_IMG_PROG, 3);
}

void draw_frame_black(int16_t x, int16_t y, uint8_t w, uint8_t h)
{
    platform_fillrect(x, y, w, h, BLACK);
    return;
}

void draw_rounded_frame_black(int16_t x, int16_t y, uint8_t w, uint8_t h)
{
    draw_frame_black(x, y, w, h);
    platform_drawoverwrite(x        , y        , ROUNDED_BORDERS_BLACK_IMG_PROG, 0);
    platform_drawoverwrite(x + w - 2, y        , ROUNDED_BORDERS_BLACK_IMG_PROG, 1);
    platform_drawoverwrite(x        , y + h - 8, ROUNDED_BORDERS_BLACK_IMG_PROG, 2);
    platform_drawoverwrite(x + w - 2, y + h - 8, ROUNDED_BORDERS_BLACK_IMG_PROG, 3);
}
