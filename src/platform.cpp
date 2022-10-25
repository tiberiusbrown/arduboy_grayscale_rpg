#include "common.hpp"

#ifdef ARDUINO
#include "ArduboyFX.h"
#else
#include "generated/fxdata_emulated.hpp"
#include <string.h>
#include <assert.h>
#include <SDL.h>
static uint8_t SAVE_BLOCK[8192];
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

void platform_drawoverwrite(int16_t x, int16_t y, uint8_t w, uint8_t h,
                            uint8_t const* bitmap)
{
#ifdef ARDUINO
    a.drawOverwrite(x, y, w, h, bitmap);
#else
    for(uint8_t r = 0; r < h; ++r) {
        for(uint8_t c = 0; c < w; ++c) {
            int py = y + r;
            int px = x + c;
            if(px < 0 || py < 0) continue;
            if(px >= 128 || py >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w, h, c, r);
            pixels[gplane][py * 128 + px] = p;
        }
    }
#endif
}

void platform_drawplusmask(int16_t x, int16_t y, uint8_t w, uint8_t h,
                           uint8_t const* bitmap)
{
#ifdef ARDUINO
    a.drawPlusMask(x, y, w, h, bitmap);
#else
    for(uint8_t r = 0; r < h; ++r) {
        for(uint8_t c = 0; c < w; ++c) {
            int py = y + r;
            int px = x + c;
            if(px < 0 || py < 0) continue;
            if(px >= 128 || py >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w * 3, h, c * 3 + gplane, r);
            uint8_t m = get_bitmap_bit(bitmap, w * 3, h, c * 3 + 2, r);
            if(m) pixels[gplane][py * 128 + px] = p;
        }
    }
#endif
}

void platform_drawoverwrite(int16_t x, int16_t y, uint8_t const* bitmap,
                            uint8_t frame)
{
#ifdef ARDUINO
    a.drawOverwrite(x, y, bitmap, frame);
#else
    uint8_t w = pgm_read_byte(bitmap++);
    uint8_t h = pgm_read_byte(bitmap++);
    platform_drawoverwrite(x, y, w, h,
                           bitmap + ((frame * 2 + gplane) * (w * h / 8)));
#endif
}

void platform_drawplusmask(int16_t x, int16_t y, uint8_t const* bitmap,
                           uint8_t frame)
{
#ifdef ARDUINO
    a.drawPlusMask(x, y, bitmap, frame);
#else
    uint8_t w = pgm_read_byte(bitmap++);
    uint8_t h = pgm_read_byte(bitmap++);
    platform_drawplusmask(x, y, w, h, bitmap + (frame * 3 * (w * h / 8)));
#endif
}

void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
                               uint8_t frame)
{
#ifdef ARDUINO
    FX::drawBitmap(x, y, addr, frame * 2 + a.currentPlane(), dbmOverwrite);
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr];
    bitmap++;
    uint8_t w = *bitmap++;
    bitmap++;
    uint8_t h = *bitmap++;
    bitmap += w * h / 8 * (frame * 2 + gplane);
    for(uint8_t r = 0; r < h; ++r) {
        for(uint8_t c = 0; c < w; ++c) {
            int py = y + r;
            int px = x + c;
            if(px < 0 || py < 0) continue;
            if(px >= 128 || py >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w, h, c, r);
            pixels[gplane][py * 128 + px] = p;
        }
    }
#endif
}

void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
                              uint8_t frame)
{
#ifdef ARDUINO
    FX::drawBitmap(x, y, addr, frame * 2 + a.currentPlane(), dbmMasked);
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr];
    bitmap++;
    uint8_t w = *bitmap++;
    bitmap++;
    uint8_t h = *bitmap++;
    bitmap += w * h / 4 * (frame * 2 + gplane);
    for(uint8_t r = 0; r < h; ++r) {
        for(uint8_t c = 0; c < w; ++c) {
            int py = y + r;
            int px = x + c;
            if(px < 0 || py < 0) continue;
            if(px >= 128 || py >= 64) continue;
            uint8_t p = get_bitmap_bit(bitmap, w * 2, h, c * 2 + 0, r);
            uint8_t m = get_bitmap_bit(bitmap, w * 2, h, c * 2 + 1, r);
            if(m) pixels[gplane][py * 128 + px] = p;
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

#if 0

void platform_fx_clear_flag(uint16_t index)
{
    uint16_t addr = 4096 + index / 8;
    uint8_t mask = ~(1 << (index % 8));
#ifdef ARDUINO
    FX::writeEnable();
    FX::seekCommand(SFC_WRITE, (uint24_t(FX::programSavePage) << 8) + addr);
    FX::writeByte(mask);
    FX::disable();
#else
    SAVE_BLOCK[addr] &= mask;
#endif
}

bool platform_fx_get_flag(uint16_t index)
{
    uint16_t addr = 4096 + index / 8;
    uint8_t mask = 1 << (index % 8);
#ifdef ARDUINO
    uint8_t data;
    FX::readSaveBytes(addr, &data, 1);
    return (data & mask) != 0;
#else
    return (SAVE_BLOCK[addr] & mask) != 0;
#endif
}

void platform_fx_erase_save_sector(uint16_t page)
{
#ifdef ARDUINO
    FX::eraseSaveBlock(page);
#else
    uint64_t now = SDL_GetTicks64();
    assert(now >= ticks_when_ready);
    ticks_when_ready = now + 150;
    for(int i = 0; i < 256; ++i)
        SAVE_BLOCK[page * 256 + i] = 0xff;
#endif
}
void platform_fx_write_save_page(uint16_t page, void const* data)
{
#ifdef ARDUINO
    // eww cast to non const
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

#endif
