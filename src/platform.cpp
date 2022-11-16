#include "common.hpp"

#ifdef ARDUINO
#include "ArduboyFX.h"
#define SPRITESU_IMPLEMENTATION
#define SPRITESU_OVERWRITE
#define SPRITESU_FX
#include "SpritesU.hpp"
#else
#include "generated/fxdata_emulated.hpp"
#include <SDL.h>
#include <assert.h>
#include <string.h>
static uint8_t SAVE_BLOCK[4096];
static uint64_t ticks_when_ready = 0;
#endif

void platform_fx_read_data_bytes(uint24_t addr, void* dst, size_t num)
{
#ifdef ARDUINO
    FX::readDataBytes(addr, dst, num);
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    memcpy(dst, &FXDATA[addr], num);
#endif
}

#ifndef ARDUINO
extern int gplane;
extern uint8_t pixels[2][128 * 64];

static uint8_t get_bitmap_bit(uint8_t const* bitmap, uint8_t w, uint8_t h,
    uint8_t x, uint8_t y)
{
    uint8_t page = y / 8;
    uint8_t byte = bitmap[page * w + x];
    return (byte >> (y % 8)) & 1;
}
#endif

#if FADE_USING_CONTRAST
extern float fade_factor;
#else
static uint8_t const FADE[] PROGMEM =
{
    0x00, 0x22, 0x00, 0x00, 0x00, 0x22, 0x00, 0x44,
    0x88, 0x22, 0x00, 0x44, 0x88, 0x22, 0x88, 0x44,
    0x99, 0x22, 0x88, 0x44, 0x99, 0x22, 0x88, 0x55,
    0x99, 0x66, 0x88, 0x55, 0x99, 0x66, 0x99, 0x55,
    0x99, 0x66, 0x99, 0x77, 0xdd, 0x66, 0x99, 0x77,
    0xdd, 0xee, 0x99, 0x77, 0xdd, 0xee, 0xdd, 0x77,
    0xdd, 0xff, 0xdd, 0x77, 0xff, 0xff, 0xdd, 0x77,
    0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xff, 0xff,
};
#endif

void platform_fade(uint8_t f)
{
#if FADE_USING_CONTRAST
    if(f > 15) f = 15;
    f *= 17;
#ifdef ARDUINO
    a.setContrast(f);
#else
    fade_factor = float(f) / 255;
#endif
#else
    uint8_t const* fade = &FADE[f * 4];
#ifdef ARDUINO
    uint8_t* b = a.getBuffer();
    for(uint16_t i = 0; i < 1024; i += 4)
    {
        b[i + 0] &= pgm_read_byte(fade + 0);
        b[i + 1] &= pgm_read_byte(fade + 1);
        b[i + 2] &= pgm_read_byte(fade + 2);
        b[i + 3] &= pgm_read_byte(fade + 3);
}
#else
    for(uint8_t r = 0; r < 64; ++r)
    {
        for(uint8_t c = 0; c < 128; ++c)
        {
            if(!((fade[c % 4] >> (r % 8)) & 1))
                pixels[gplane][r * 128 + c] = 0;
        }
    }
#endif
#endif
}

void platform_drawoverwritemonochrome(int16_t x, int16_t y, uint8_t w,
    uint8_t h, uint8_t const* bitmap)
{
#ifdef ARDUINO
    SpritesU::drawOverwrite(x, y, w, h, bitmap);
#else
    for(uint8_t r = 0; r < h; ++r)
    {
        for(uint8_t c = 0; c < w; ++c)
        {
            int tpy = y + r;
            int tpx = x + c;
            if(tpx < 0 || tpy < 0) continue;
            if(tpx >= 128 || tpy >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w, h, c, r);
            pixels[gplane][tpy * 128 + tpx] = p;
        }
    }
#endif
}

void platform_drawoverwrite(int16_t x, int16_t y, uint8_t const* bitmap,
    uint8_t frame)
{
#ifdef ARDUINO
    SpritesU::drawOverwrite(x, y, bitmap, frame * 2 + a.currentPlane());
#else
    uint8_t w = pgm_read_byte(bitmap++);
    uint8_t h = pgm_read_byte(bitmap++);
    platform_drawoverwritemonochrome(
        x, y, w, h, bitmap + ((frame * 2 + gplane) * (w * h / 8)));
#endif
}

void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h)
{
#ifdef ARDUINO
    SpritesU::drawOverwriteFX(x, y, w, h, addr, frame * 2 + a.currentPlane());
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr];
    bitmap += w * h / 8 * (frame * 2 + gplane);
    for(uint8_t r = 0; r < h; ++r)
    {
        for(uint8_t c = 0; c < w; ++c)
        {
            int tpy = y + r;
            int tpx = x + c;
            if(tpx < 0 || tpy < 0) continue;
            if(tpx >= 128 || tpy >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w, h, c, r);
            pixels[gplane][tpy * 128 + tpx] = p;
        }
    }
#endif
}

void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h)
{
#ifdef ARDUINO
    SpritesU::drawPlusMaskFX(x, y, w, h, addr, frame * 2 + a.currentPlane());
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr];
    bitmap += w * h / 4 * (frame * 2 + gplane);
    for(uint8_t r = 0; r < h; ++r)
    {
        for(uint8_t c = 0; c < w; ++c)
        {
            int tpy = y + r;
            int tpx = x + c;
            if(tpx < 0 || tpy < 0) continue;
            if(tpx >= 128 || tpy >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w * 2, h, c * 2 + 0, r);
            uint8_t m = get_bitmap_bit(bitmap, w * 2, h, c * 2 + 1, r);
            if(m) pixels[gplane][tpy * 128 + tpx] = p;
        }
    }
#endif
}

void platform_fillrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c)
{
#ifdef ARDUINO
    a.fillRect(x, y, w, h, c);
#else
    c = (c & (gplane + 1)) ? 1 : 0;
    for(uint8_t i = 0; i != h; ++i)
        for(uint8_t j = 0; j != w; ++j)
            if(unsigned(y + i) < 64 && unsigned(x + j) < 128)
                pixels[gplane][(y + i) * 128 + (x + j)] = c;
#endif
}

void platform_drawrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c)
{
#ifdef ARDUINO
    a.drawRect(x, y, w, h, c);
#else
    platform_fillrect(x, y, w, 1, c);
    platform_fillrect(x, y + h - 1, w, 1, c);
    platform_fillrect(x, y, 1, h, c);
    platform_fillrect(x + w - 1, y, 1, h, c);
#endif
}

void platform_fx_erase_save_sector()
{
#ifdef ARDUINO
    FX::eraseSaveBlock(0);
#else
    uint64_t now = SDL_GetTicks64();
    assert(now >= ticks_when_ready);
    ticks_when_ready = now + 150;
    for(int i = 0; i < 4096; ++i)
        SAVE_BLOCK[i] = 0xff;
#endif
}
void platform_fx_write_save_page(uint16_t page, void const* data)
{
#ifdef ARDUINO
    // eww cast to non const (writeSavePage doesn't modify data though)
    FX::writeSavePage(page, (uint8_t*)data);
#else
    uint64_t now = SDL_GetTicks64();
    assert(now >= ticks_when_ready);
    ticks_when_ready = now + 2;
    uint8_t const* u8data = (uint8_t const*)data;
    for(int i = 0; i < 256; ++i)
        SAVE_BLOCK[page * 256 + i] &= u8data[i];
#endif
}
void platform_fx_read_save_page(uint16_t page, void* data)
{
#ifdef ARDUINO
    FX::readSaveBytes(uint24_t(page) << 8, (uint8_t*)data, 256);
#else
    uint8_t* u8data = (uint8_t*)data;
    for(int i = 0; i < 256; ++i)
        u8data[i] = SAVE_BLOCK[page * 256 + i];
#endif
}

bool platform_fx_busy()
{
#ifdef ARDUINO
    FX::enable();
    FX::writeByte(SFC_READSTATUS1);
    bool busy = ((FX::readByte() & 1) != 0);
    FX::disable();
    return busy;
#else
    return SDL_GetTicks64() >= ticks_when_ready;
#endif
}
