#include "common.hpp"

#define TILES_IN_PROG 0

#include "font_adv.hpp"
#include "generated/font_img.hpp"
#include "generated/fxdata.h"
#if TILES_IN_PROG > 0
#include "generated/tile_img.hpp"
#endif
#include "generated/rounded_borders_white_img.hpp"
#include "generated/rounded_borders_black_img.hpp"

static uint8_t add_sprite_entry(draw_sprite_entry* entry, uint8_t ci,
                                      int16_t ox, int16_t oy)
{
    auto const& e = chunk_sprites[ci];
    if(!e.active) return 0;
    uint8_t f = e.type * 16;
    uint8_t d = e.dir;
    if(e.walking && !(d & 0x80))
    {
        f += d * 2;
        if(state == STATE_MAP)
            f += (((uint8_t)nframe >> 2) & 3);
    }
    entry->addr = SPRITES_IMG;
    entry->frame = f;
    entry->x = ox + e.x;
    entry->y = oy + e.y - 2;
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
        if(pmoving) f += (((uint8_t)nframe >> 2) & 3);
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
    if(pmoving) f += (((uint8_t)nframe >> 2) & 3);
    platform_fx_drawplusmask(64 - 8, 32 - 8 - 4, PLAYER_IMG, f, 16, 16);
}

void draw_tile(int16_t x, int16_t y, uint8_t t, uint8_t n)
{
#if TILES_IN_PROG > 0
    if(t < TILES_IN_PROG)
        platform_drawoverwrite(x, y, TILE_IMG_PROG, t);
    else
        platform_fx_drawoverwrite(x, y, TILE_IMG, t - TILES_IN_PROG, 16, 16);
#else
    uint16_t f = t;
    uint8_t nf = (uint8_t)nframe;
#if 0
#ifdef ARDUINO
    asm volatile(R"ASM(
        mov  r0, %[n]
        lsr  r0
        lsr  r0
        eor  %[n], r0
        mul  %[n], %[k]
        mov  %[n], r0
        lsr  r0
        eor  %[n], r0
        clr  r1
        lsr  %[nf]
        lsr  %[nf]
        add  %[n], %[nf]
        andi %[n], 63
        cpi  %[n], 4
        brge .+2
        add  %B[f], %[n]
        )ASM"
        : [n] "+&r" (n), [f] "+&r" (f), [nf] "+&r" (nf)
        : [k] "r" (uint8_t(223))
        );
#else
    n ^= (n >> 2);
    n *= 223;
    n ^= (n >> 1);
    n += (nf >> 2);
    n &= 63;
    if(n < 4) f += (uint16_t(n) << 8);
#endif
#endif
    platform_fx_drawoverwrite(x, y, TILE_IMG, f, 16, 16);
#endif
}

static void draw_chunk_tiles(uint8_t i, int16_t ox, int16_t oy)
{
    auto const& ac = active_chunks[i];
    uint8_t const* tiles = ac.chunk.tiles_flat;
    uint8_t maxy = 64;
    if(state == STATE_PAUSE) maxy -= sdata.pause.ally;
    else if(state == STATE_DIALOG) maxy = 35;
    for(uint8_t r = 0, n = 0; r < 64; r += 16)
    {
        int16_t y = oy + r;
        if(y <= -16)
        {
            n += 8;
            continue;
        }
        if(y >= maxy) break;
        for(uint8_t c = 0; c < 128; c += 16, ++n)
        {
            int16_t x = ox + c;
            draw_tile(x, y, tiles[n], n);
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

void draw_text_noclip(int16_t x, int16_t y, char const* str, uint8_t f)
{
    char t;
    uint8_t cx = (uint8_t)x;
    uint8_t plane8 = plane() * 8;
    uint8_t const* font_img = FONT_IMG + plane() * 8 + -(' ' * (8 * PLANES)) + 2;
    uint8_t const* font_adv = FONT_ADV - ' ';
    uint8_t page = (uint8_t)y;
#ifdef ARDUINO
    uint8_t shift_coef = FX::bitShiftLeftUInt8(y);
    asm volatile(
        "lsr %[page]\n"
        "lsr %[page]\n"
        "lsr %[page]\n"
        : [page] "+&r" (page));
#else
    uint8_t shift_coef = 1 << (page & 7);
    page >>= 3;
#endif
    if(page >= 8) return;
    for(;;)
    {
        if(f & NOCLIPFLAG_PROG)
            t = (char)pgm_read_byte_inc(str);
        else
            t = (char)deref_inc(str);
        if(t == '\0') return;
        if(t == '\n')
        {
            if(f & NOCLIPFLAG_BIGLINES)
            {
                // advance 11 rows
                if(shift_coef >= 0x20)
                    page += 2, shift_coef >>= 5;
                else
                    page += 1, shift_coef <<= 3;
            }
            else
            {
                // advance 9 rows
                if(shift_coef & 0x80)
                    page += 2, shift_coef = 1;
                else
                    page += 1, shift_coef <<= 1;
            }
            if(page >= 8) return;
            cx = (uint8_t)x;
            continue;
        }
        uint8_t const* bitmap = font_img + (t * (8 * PLANES));
        uint8_t adv = pgm_read_byte(&font_adv[t]);
        if(cx < uint8_t(128 - adv))
        {
            platform_drawoverwritemonochrome_noclip(
                cx, page, shift_coef, adv, 1, bitmap);
        }
        cx += adv;
    }
}

static void draw_text_ex(int16_t x, int16_t y, char const* str, bool prog)
{
    char t;
    int16_t cx = x;
    uint8_t plane8 = plane() * 8;
    uint8_t const* font_img = FONT_IMG + plane() * 8 + - (' ' * (8 * PLANES)) + 2;
    uint8_t const* font_adv = FONT_ADV - ' ';
    for(;;)
    {
        if(prog)
            t = (char)pgm_read_byte_inc(str);
        else
            t = (char)deref_inc(str);
        if(t == '\0') return;
        if(t == '\n')
        {
            y += 9;
            cx = x;
            continue;
        }
        uint8_t const* bitmap = font_img + (t * (8 * PLANES));
        uint8_t adv = pgm_read_byte(&font_adv[t]);
        platform_drawoverwritemonochrome(cx, y, adv, 8, bitmap);
        cx += adv;
    }
}

void draw_text(int16_t x, int16_t y, char const* str)
{
    draw_text_ex(x, y, str, false);
}

void draw_text_prog(int16_t x, int16_t y, char const* str)
{
    draw_text_ex(x, y, str, true);
}

uint8_t dec_to_str(char* dst, uint8_t val)
{
    char* t = dst;

    uint8_t val_orig = val;
    if(val >= 200)
        *t++ = '2', val -= 200;
    if(val >= 100)
        *t++ = '1', val -= 100;
    if(val_orig >= 100 || val >= 10)
    {
        char n = '0';
        while(val >= 10)
            ++n, val -= 10;
        *t++ = n;
    }
    *t++ = '0' + val;
    *t++ = '\0';

    return (uint8_t)(uintptr_t)(t - dst - 1);
}

void draw_dec(int16_t x, int16_t y, uint8_t val)
{
    char b[7];
    (void)dec_to_str(b, val);
    draw_text_noclip(x, y, b);
}

static uint8_t text_width_ex(char const* str, bool prog)
{
    uint8_t w = 0;
    uint8_t t;
    while((t = (prog ? pgm_read_byte_inc(str) : deref_inc(str))) != '\0')
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
}

void draw_rounded_frame_black(int16_t x, int16_t y, uint8_t w, uint8_t h)
{
    draw_frame_black(x, y, w, h);
    platform_drawoverwrite(x        , y        , ROUNDED_BORDERS_BLACK_IMG_PROG, 0);
    platform_drawoverwrite(x + w - 2, y        , ROUNDED_BORDERS_BLACK_IMG_PROG, 1);
    platform_drawoverwrite(x        , y + h - 8, ROUNDED_BORDERS_BLACK_IMG_PROG, 2);
    platform_drawoverwrite(x + w - 2, y + h - 8, ROUNDED_BORDERS_BLACK_IMG_PROG, 3);
}
