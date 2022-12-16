#include "common.hpp"

#define TILES_IN_PROG 0

#include "generated/font_adv.hpp"
#include "generated/font_img.hpp"
#include "generated/fxdata.h"
#if TILES_IN_PROG > 0
#include "generated/tile_img.hpp"
#endif
#include "generated/rounded_borders_white_img.hpp"
#include "generated/rounded_borders_black_img.hpp"

#ifdef ARDUINO
#include "SpritesU.hpp"
#endif

static int8_t fmulshi(int8_t x, int8_t y)
{
    return int8_t(fmuls(x, y) >> 8);
}

inline int8_t i8abs(int8_t x) { return x < 0 ? -x : x; }

void draw_objective()
{
    if(py < 128 * 16) return;

    uint8_t objx = savefile.objx;
    uint8_t objy = savefile.objy;
    if((objx | objy) == 0) return;

    if(rframe & 16) return;

    static_assert(MAP_CHUNK_W <= 32, "expand calculations to 16-bit");
    static_assert(MAP_CHUNK_H <= 64, "expand calculations to 16-bit");

    // direction to objective
    int8_t dx = objx - div16_u16(px);
    int8_t dy = objy - div16_u16(py);

    // rotate by 22.5 degrees
    constexpr int8_t M00 = int8_t(+0.9239 * 127);
    constexpr int8_t M01 = int8_t(-0.3827 * 127);
    constexpr int8_t M10 = int8_t(+0.3827 * 127);
    constexpr int8_t M11 = int8_t(+0.9239 * 127);
    int8_t rx = fmulshi(M00, dx) + fmulshi(M01, dy);
    int8_t ry = fmulshi(M10, dx) + fmulshi(M11, dy);

    uint8_t f;
    int16_t x, y;

    if(i8abs(dx) <= 5 && i8abs(dy - 1) <= 3)
    {
        // objective in view
        f = 3;
        x = objx * 16 - px + 56;
        y = objy * 16 - py + 8;
    }
    else
    {
        // calculate arrow image frame
        f = 0;
        if(rx < 0) f |= 2, rx = -rx;
        if(ry < 0) f |= 4, ry = -ry;
        if(ry > rx) f |= 1;

        constexpr int16_t XC = 56;
        constexpr int16_t YC = 24;
        x = objx * 16 - px;
        y = objy * 16 - py;

        // project arrow image to screen edges
        if(x < -XC)
        {
            y = (int24_t)y * -XC / x;
            x = -XC;
        }
        else if(x > XC)
        {
            y = (int24_t)y * XC / x;
            x = XC;
        }
        if(y < -YC)
        {
            x = (int24_t)x * -YC / y;
            y = -YC;
        }
        else if(y > YC)
        {
            x = (int24_t)x * YC / y;
            y = YC;
        }

        x += XC;
        y += YC;
    }

#if 0
    // animate arrow
    static int8_t const DIRS[16] PROGMEM =
    {
        1, 0, 1, 1, -1, 1, 0, 1, 1, -1, 0, -1, -1, 0, -1, -1,
    };
    uint8_t const* ptr = (uint8_t const*)(&DIRS[f * 2]);
    int8_t ax = (int8_t)pgm_read_byte_inc(ptr);
    int8_t ay = (int8_t)pgm_read_byte(ptr);
    uint8_t af = (rframe >> 2) & 7;
    if(af >= 4) af = 7 - af;
    x += ax * af;
    y += ay * af;
#endif

    platform_fx_drawplusmask(x, y, OBJECTIVE_ARROWS_IMG, f);
}

static uint8_t add_sprite_entry(draw_sprite_entry* entry, uint8_t ci,
                                      int16_t ox, int16_t oy)
{
    auto const& e = chunk_sprites[ci];
    if(!e.active) return 0;
    uint8_t f = e.type * 16;
    uint8_t d = e.dir;
    bool walking = e.walking;
    if(d & 0x80) walking = false;
    if(e.type == 13) walking = true;
    if(walking)
    {
        f += (d & 7) * 2;
        if(state == STATE_MAP || state == STATE_TITLE)
            f += (div8(nframe) & 3);
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
        if(pmoving) f += (div8(nframe) & 3);
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

void draw_tile(int16_t x, int16_t y, uint8_t t)
{
#if TILES_IN_PROG > 0
    if(t < TILES_IN_PROG)
        platform_drawoverwrite(x, y, TILE_IMG_PROG, t);
    else
        platform_fx_drawoverwrite(x, y, TILE_IMG, t - TILES_IN_PROG, 16, 16);
#else
    platform_fx_drawoverwrite(x, y, TILE_IMG, t, 16, 16);
#endif
}

static void draw_chunk_tiles(uint8_t const* tiles, int16_t ox, int16_t oy)
{
    uint8_t nx;
    uint8_t ny;
    uint8_t t;
    uint24_t tile_img = TILE_IMG + 2;
    int16_t x;
#ifdef ARDUINO
//#if 0
    asm volatile(R"ASM(
            clr %[t]
            sbrs %B[ox], 7
            rjmp 1f
            mov  %[t], %A[ox]
            neg  %[t]
            andi %[t], 0xf0
            add  %A[ox], %[t]
            adc  %B[ox], __zero_reg__
            swap %[t]
            ldi  %[nx], 8
            sub  %[nx], %[t]
            rjmp 2f
        1:
            ldi  %[nx], 128+15
            sub  %[nx], %A[ox]
            swap %[nx]
            andi %[nx], 0x0f
        2:
            brne .+2
            rjmp draw_chunk_tiles_return
            add  %A[tiles], %[t]
            adc  %B[tiles], __zero_reg__
            clr  %[t]
            sbrs %B[oy], 7
            rjmp 3f
            mov  %[t], %A[oy]
            neg  %[t]
            andi %[t], 0xf0
            add  %A[oy], %[t]
            adc  %B[oy], __zero_reg__
            mov  __tmp_reg__, %[t]
            swap __tmp_reg__
            lsr  %[t]
            ldi  %[ny], 4
            sub  %[ny], __tmp_reg__
            rjmp 4f
        3:
            ldi  %[ny], 64+15
            sub  %[ny], %A[oy]
            swap %[ny]
            andi %[ny], 0x0f
        4:
            brne .+2
            rjmp draw_chunk_tiles_return
            add  %A[tiles], %[t]
            adc  %B[tiles], __zero_reg__
            
            lds  __tmp_reg__, %[plane]
            swap __tmp_reg__
            lsl  __tmp_reg__
            add  %A[tile_img], __tmp_reg__
            adc  %B[tile_img], __zero_reg__
            adc  %C[tile_img], __zero_reg__
        )ASM"
        :
        [nx]       "=&d" (nx),
        [ny]       "=&d" (ny),
        [t]        "=&d" (t),
        [ox]       "+&r" (ox),
        [oy]       "+&r" (oy),
        [tiles]    "+&r" (tiles),
        [tile_img] "+&r" (tile_img)
        :
        [plane] ""    (&abg_detail::current_plane)
        );
#else

    t = 0;
    if(ox < 0)
    {
        t = uint8_t(-int8_t(ox));
        t &= 0xf0;
        ox += t;
        t = nibswap(t); // t >>= 4
        nx = 8 - t;
    }
    else
        nx = div16(uint8_t(128 + 15 - uint8_t(ox)));
    if(nx == 0)
        return;
    tiles += t;
    t = 0;
    if(oy < 0)
    {
        t = uint8_t(-int8_t(oy));
        t &= 0xf0;
        oy += t;
        uint8_t tmp = t;
        tmp = nibswap(tmp); // tmp >>= 4
        t >>= 1;
        ny = 4 - tmp;
    }
    else
        ny = div16(uint8_t(64 + 15 - oy));
    if(ny == 0) return;
    tiles += t;
    tile_img += 32 * plane();
#endif
#ifdef ARDUINO
//#if 0
    register int16_t moved_x __asm__("r14");
    register int16_t moved_oy __asm__("r12") = oy;
    // call to SpritesU::drawBasicNoChecks needs:
    // ======== lo ======== hi ========
    //     r24: 16          16
    //     r22: C[tile_img]
    //     r20: A[tile_img] B[tile_img]
    //     r18: 0           0
    //     r16: 2
    //     r14: A[x]        B[x]        <-- already there
    //     r12: A[oy]       B[oy]       <-- already there

    asm volatile(R"ASM(
            movw r28, %[tiles]
        1:
            mov  %[t], %[nx]
            movw %[x], %[ox]
        2:
            ldi  r24, 16
            ldi  r25, 16
            ldi  r18, 32*3
            ld   r19, Y+
            mul  r19, r18
            movw r20, %A[tile_img]
            mov  r22, %C[tile_img]
            add  r20, r0
            adc  r21, r1
            clr  __zero_reg__
            adc  r22, __zero_reg__
            ldi  r18, 0
            ldi  r19, 0
            ldi  r16, 2
            %~call %x[drawfunc]
            ldi  r18, 16
            add  %A[x], r18
            adc  %B[x], __zero_reg__
            dec  %[t]
            brne 2b
            add  %A[oy], r18
            adc  %B[oy], __zero_reg__
            ldi  r18, 8
            sub  r18, %[nx]
            add  r28, r18
            adc  r29, __zero_reg__
            dec  %[ny]
            brne 1b
        )ASM"
        : 
        [ny]        "+&l" (ny),
        [t]         "=&l" (t),
        [x]         "=&l" (moved_x),
        [oy]        "+&l" (moved_oy)
        :
        [nx]        "l"   (nx),
        [ox]        "l"   (ox),
        [tile_img]  "l"   (tile_img),
        [tiles]     "r"   (tiles),
        [drawfunc]  "i"   (SpritesU::drawBasicNoChecks)
        :
        "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
        "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
        );
#else
    do {
        t = nx;
        x = ox;
        do {
            MY_ASSERT(x > -16 && x < 128);
            MY_ASSERT(oy > -16 && oy < 64);
#ifdef ARDUINO
            SpritesU::drawBasicNoChecks(
                (16 << 8) | 16,
                tile_img + (PLANES * 32) * deref_inc(tiles),
                0, SpritesU::MODE_OVERWRITEFX,
                x, oy);
#else
            draw_tile(x, oy, *tiles++);
#endif
            x += 16;
        } while(--t != 0);
        oy += 16;
        tiles += (8 - nx);
    } while(--ny != 0);
#endif

#ifdef ARDUINO
    asm volatile("draw_chunk_tiles_return:\n");
#endif
}

void draw_tiles()
{
    uint8_t tx = (px - 64 + 8) & 0x7f;
    uint8_t ty = (py - 32 + 8) & 0x3f;
    int16_t ox = -int16_t(tx);
    int16_t oy = -int16_t(ty);
    draw_chunk_tiles(active_chunks[0].chunk.tiles_flat, ox, oy);
    draw_chunk_tiles(active_chunks[1].chunk.tiles_flat, ox + 128, oy);
    draw_chunk_tiles(active_chunks[2].chunk.tiles_flat, ox, oy + 64);
    draw_chunk_tiles(active_chunks[3].chunk.tiles_flat, ox + 128, oy + 64);
}

void draw_text_noclip(int8_t x, int8_t y, char const* str, uint8_t f)
{
    char t;
    uint8_t cx = (uint8_t)x;
    uint8_t plane8 = plane() * 8;
    uint8_t const* font_img = FONT_IMG + plane() * 8 - (' ' * (8 * PLANES)) + 2;
    uint8_t const* font_adv = FONT_ADV - ' ';
    uint8_t page = (uint8_t)y;
#ifdef ARDUINO
    //uint8_t shift_coef = FX::bitShiftLeftUInt8(y);
    uint8_t shift_coef = bitmask(page);
    asm volatile(
        "lsr %[page]\n"
        "lsr %[page]\n"
        "lsr %[page]\n"
        : [page] "+&r" (page));
#else
    uint8_t shift_coef = bitmask(page);
    page >>= 3;
#endif
    if(page >= 8) return;
    uint16_t shift_mask = ~(0xff * shift_coef);
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
            shift_mask = ~(0xff * shift_coef);
            cx = (uint8_t)x;
            continue;
        }
        uint8_t const* bitmap = font_img + (t * (8 * PLANES));
        uint8_t adv = pgm_read_byte(&font_adv[t]);
        if(cx <= uint8_t(128 - adv))
        {
            platform_drawcharfast(
                cx, page, shift_coef, adv, shift_mask, bitmap);
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

void draw_dec(int8_t x, int8_t y, uint8_t val)
{
    char b[4];
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
        x += pgm_read_byte(&FONT_ADV[t - ' ']);
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
