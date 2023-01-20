#include "common.hpp"

#define TILES_IN_PROG 0

#include "generated/font_adv.hpp"
#include "generated/font_img.hpp"
#include "generated/fxdata.h"
#if TILES_IN_PROG > 0
#include "generated/tile_img.hpp"
#endif

#ifdef ARDUINO
#include "SpritesU.hpp"
#endif

static int8_t fmulshi(int8_t x, int8_t y)
{
    return int8_t(fmuls(x, y) >> 8);
}

inline int8_t i8abs(int8_t x)
{
#if ARDUINO_ARCH_AVR
    asm volatile(R"ASM(
            sbrc %[x], 7
            neg  %[x]
        )ASM"
        : [x] "+&r" (x)
        );
    return x;
#else
    return x < 0 ? -x : x;
#endif
}

void draw_objective(int16_t diffx, int16_t diffy)
{
    int8_t dx, dy;
    {
        int16_t tx = diffx;
        int16_t ty = diffy;
        dual_shift_s16<4>(tx, ty);
        dx = (int8_t)tx;
        dy = (int8_t)ty;
    }
    //int8_t dx = diffx >> 4;
    //int8_t dy = diffy >> 4;

    uint8_t f;
    int16_t x, y;

    if(i8abs(dx) <= 5 && i8abs(dy - 1) <= 3)
    {
        // objective in view
        f = 3;
        x = diffx + 58;
        y = diffy + 8;
    }
    else
    {
        // rotate by 22.5 degrees
        constexpr int8_t M00 = int8_t(+0.9239 * 127);
        constexpr int8_t M01 = int8_t(-0.3827 * 127);
        constexpr int8_t M10 = int8_t(+0.3827 * 127);
        constexpr int8_t M11 = int8_t(+0.9239 * 127);
        int8_t rx = fmulshi(M00, dx) + fmulshi(M01, dy);
        int8_t ry = fmulshi(M10, dx) + fmulshi(M11, dy);

        // calculate arrow image frame
        f = 0;
        if(rx < 0) f |= 2, rx = -rx;
        if(ry < 0) f |= 4, ry = -ry;
        if(ry > rx) f |= 1;

        constexpr int16_t XC = 56;
        constexpr int16_t YC = 24;
        x = diffx;
        y = diffy;

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

#if 1
    // animate arrow
    static int8_t const DIRS[16] PROGMEM =
    {
        1, 0, 1, 1, -1, 1, 0, 1, 1, -1, 0, -1, -1, 0, -1, -1,
    };
    uint8_t const* ptr = (uint8_t const*)(&DIRS[f * 2]);
    int8_t ax = (int8_t)pgm_read_byte_inc(ptr);
    int8_t ay = (int8_t)pgm_read_byte(ptr);
    uint8_t af;
#if ARDUINO_ARCH_AVR
    asm volatile(R"ASM(
        lds  %[af], %[rframe]
        lsr  %[af]
        lsr  %[af]
        sbrc %[af], 2
        com  %[af]
        andi %[af], 7
        muls %[af], %[ax]
        sub  %A[x], r0
        sbc  %B[x], r1
        muls %[af], %[ay]
        sub  %A[y], r0
        sbc  %B[y], r1
        clr __zero_reg__
        )ASM"
        : [af]     "=&d" (af),
          [x]      "+&r" (x),
          [y]      "+&r" (y)
        : [rframe] "i"   (&rframe),
          [ax]     "d"   (ax),
          [ay]     "d"   (ay)
        );
#else
    af = rframe >> 2;
    if(af & 4) af = ~af;
    af &= 7;
    x -= ax * af;
    y -= ay * af;
#endif
#endif

    platform_fx_drawplusmask(x, y, 16, 16, OBJECTIVE_ARROWS_IMG, f);
}

static bool add_sprite_entry(draw_sprite_entry* entry, uint8_t ci,
                                      int16_t ox, int16_t oy)
{
    auto const& e = chunk_sprites[ci];
    if(!e.active) return false;
    uint16_t f = e.type * 16;
    uint8_t d = e.dir;
    bool walking = e.walking;
    if(d & 0x80) walking = false;
    uint8_t nf = lsr(nframe);
    uint8_t flags = pgm_read_byte(&SPRITE_FLAGS[e.type]);
    if(flags & SF_ALWAYS_ANIM)
        walking = true;
    if(!(flags & SF_FAST_ANIM))
        nf >>= 1;
    if(!(flags & SF_FAST_ANIM2))
        nf >>= 1;
    if(walking)
    {
        f += (d & 7) * 2;
        if(state == STATE_MAP || state == STATE_TITLE)
            f += (nf & 3);
    }
    entry->addr = SPRITES_IMG;
    entry->frame = f;
    entry->x = ox + e.x;
    entry->y = oy + e.y - 2;
    return true;
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
        platform_fx_drawplusmask(entries[i].x, entries[i].y, 16, 16,
            entries[i].addr, entries[i].frame);
    }
}

void draw_sprites()
{
    draw_sprite_entry entries[5]; // increase as necessary
    uint8_t n = 0;

    draw_sprite_entry* ptr = &entries[0];

    // chunk enemies
    {
        uint16_t tx = px - 64 + 8;
        uint16_t ty = py - 32 + 8;
        uint8_t cx = uint8_t(tx >> 7);
        uint8_t cy = uint8_t(ty >> 6);
        int16_t ox = -int16_t(tx & 0x7f);
        int16_t oy = -int16_t(ty & 0x3f);
        for(uint8_t i = 0, dox = 0, doy = 0; i < 4; ++i, doy += lsr(dox), dox ^= 128)
        {
            //uint8_t dox = (i &  1) * 128;
            //uint8_t doy = (i >> 1) * 64;
            if(add_sprite_entry(ptr, i, ox + dox, oy + doy))
                ++n, ++ptr;
        }
    }

    // player sprite
    if(state != STATE_DIE || ((nframe & 2) && sdata.die.frame < 24))
    {
        uint8_t f = pdir * 4;
        if(pmoving) f += (div8(nframe) & 3);
        *ptr = {PLAYER_IMG, f, 64 - 8, 32 - 8 - 4};
        ++n;
    }

    sort_and_draw_sprites(entries, n);
}

void draw_player()
{
    uint8_t f = pdir * 4;
    //if(pmoving) f += (div4(nframe) & 3);
    platform_fx_drawplusmask(64 - 8, 32 - 8 - 4, 16, 16, PLAYER_IMG, f);
}

void draw_tile(int16_t x, int16_t y, uint16_t t)
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
    {
        uint8_t t = tilesheet();
        if(t & 1) tile_img += uint16_t(256 * 32 * 3);
        if(t & 2) tile_img += uint16_t(256 * 32 * 3 * 2);
    }
    int16_t x;
#if ARDUINO_ARCH_AVR
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
#ifdef ARDUINO
    tile_img += 32 * plane();
#endif
#endif

    uint8_t maxy = 64;
    if(state == STATE_PAUSE) maxy -= sdata.pause.ally;
    else if(state == STATE_DIALOG) maxy = 35;
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
                SpritesU::MODE_OVERWRITEFX,
                x, oy);
#else
            platform_fx_drawoverwrite(
                x, oy,
                tile_img - 2 + (PLANES * 32) * deref_inc(tiles),
                0, 16, 16);
#endif
            x += 16;
        } while(--t != 0);
        oy += 16;
        if(oy >= maxy) break;
        tiles += (8 - nx);
    } while(--ny != 0);

#if ARDUINO_ARCH_AVR
    asm volatile("draw_chunk_tiles_return:\n");
#endif
}

void draw_tiles()
{
    uint8_t tx = (px - 64 + 8) & 0x7f;
    uint8_t ty = (py - 32 + 8) & 0x3f;
    int16_t ox = -int16_t(tx);
    int16_t oy = -int16_t(ty);
    for(uint8_t i = 0; i < 4; ++i)
    {
        int16_t tox, toy;
#if ARDUINO_ARCH_AVR
        uint8_t ti;
        asm volatile(R"ASM(
                mov  %[ti], %[i]
                andi %[ti], 1
                mul  %[ti], %[c64]
                lsl  r0
                movw %A[tox], %A[ox]
                add  %A[tox], r0
                adc  %B[tox], r1
                mov  %[ti], %[i]
                lsr  %[ti]
                mul  %[ti], %[c64]
                movw %A[toy], %A[oy]
                add  %A[toy], r0
                adc  %B[toy], r1
                ; r1 already cleared (products < 256)
            )ASM"
            : [tox] "=&r" (tox)
            , [toy] "=&r" (toy)
            , [ti]  "=&d" (ti)
            : [i]   "r"   (i)
            , [ox]  "r"   (ox)
            , [oy]  "r"   (oy)
            , [c64] "r"   (64)
            );
#else
        tox = ox + (i & 1) * 128;
        toy = oy + (i >> 1) * 64;
#endif
        draw_chunk_tiles(
            active_chunks[i].chunk.tiles_flat, tox, toy);
    }
}

void draw_text_noclip(int8_t x, int8_t y, char const* str, uint8_t f)
{
    char t;
    uint8_t cx = (uint8_t)x;
    uint8_t const* font_img = FONT_IMG + plane() * 8 - (' ' * (8 * PLANES)) + 2;
    uint8_t const* font_adv = FONT_ADV - ' ';
    uint8_t page = (uint8_t)y;
#if ARDUINO_ARCH_AVR
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
        store_inc(t, '2'), val -= 200;
    if(val >= 100)
        store_inc(t, '1'), val -= 100;
    if(val_orig >= 100 || val >= 10)
    {
        char n = '0';
        while(val >= 10)
            ++n, val -= 10;
        store_inc(t, (uint8_t)n);
    }
    store_inc(t, uint8_t('0' + val));
    *t = '\0';

    return (uint8_t)(uintptr_t)(t - dst);
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
