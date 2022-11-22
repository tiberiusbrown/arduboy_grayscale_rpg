#include "common.hpp"

#ifdef ARDUINO
#include "ArduboyFX.h"
#define SPRITESU_IMPLEMENTATION
#define SPRITESU_OVERWRITE
#define SPRITESU_FX
#include "SpritesU.hpp"
#include <ArduboyTones.h>
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

#ifndef ARDUINO
extern float fade_factor;
#endif

void platform_fade(uint8_t f)
{
    if(f > 15) f = 15;
    f *= 17;

    static uint8_t const VCOM_DES[] PROGMEM = { 0x10, 0x10, 0x10, 0x20 };
#ifdef ARDUINO
    static uint8_t const CONTRAST[] PROGMEM = { 0x20, 0x60, 0x90, 0xff };
#else
    static uint8_t const CONTRAST[] PROGMEM = { 0x6f, 0x9f, 0xcf, 0xff };
#endif
    uint8_t vcom_des = pgm_read_byte(&VCOM_DES[savefile.brightness]);
    uint8_t brightness = pgm_read_byte(&CONTRAST[savefile.brightness]);
    f = (f * brightness + f) >> 8;

#ifdef ARDUINO
    FX::enableOLED();
    abg_detail::send_cmds(0xDB, vcom_des, 0x81, f);
    FX::disableOLED();
#else
    fade_factor = float(f) / 255;
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
    SpritesU::drawOverwrite(x, y, bitmap, frame * PLANES + a.currentPlane());
#else
    uint8_t w = pgm_read_byte(bitmap++);
    uint8_t h = pgm_read_byte(bitmap++);
    platform_drawoverwritemonochrome(
        x, y, w, h, bitmap + ((frame * PLANES + gplane) * (w * h / 8)));
#endif
}

void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h)
{
#ifdef ARDUINO
    SpritesU::drawOverwriteFX(x, y, w, h, addr, frame * PLANES + a.currentPlane());
#else
    auto now = SDL_GetTicks64();
    assert(now >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr];
    bitmap += w * h / 8 * (frame * PLANES + gplane);
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
    SpritesU::drawPlusMaskFX(x, y, w, h, addr, frame * PLANES + a.currentPlane());
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr];
    bitmap += w * h / 4 * (frame * PLANES + gplane);
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
    c = (c > gplane) ? 1 : 0;
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
    ticks_when_ready = now + 200;
    for(int i = 0; i < 4096; ++i)
        SAVE_BLOCK[i] = 0xff;
#endif
}
void platform_fx_write_save_page(uint16_t page, void const* data, size_t num)
{
#ifdef ARDUINO
    FX::writeEnable();
    FX::seekCommand(SFC_WRITE, (uint24_t)(FX::programSavePage + page) << 8);
    uint8_t i = 0;
    uint8_t const* buffer = (uint8_t const*)data;
    do
    {
        FX::writeByte(buffer[i]);
    } while(i++ < num);
    FX::disable();
#else
    uint64_t now = SDL_GetTicks64();
    assert(now >= ticks_when_ready);
    ticks_when_ready = now + 2;
    uint8_t const* u8data = (uint8_t const*)data;
    for(size_t i = 0; i < num; ++i)
        SAVE_BLOCK[page * 256 + i] &= u8data[i];
#endif
}
void platform_fx_read_save_bytes(uint24_t addr, void* data, size_t num)
{
#ifdef ARDUINO
    FX::readSaveBytes(addr, (uint8_t*)data, num);
#else
    uint8_t* u8data = (uint8_t*)data;
    for(size_t i = 0; i < num; ++i)
        u8data[i] = SAVE_BLOCK[addr + i];
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
    return SDL_GetTicks64() < ticks_when_ready;
#endif
}

void platform_audio_toggle()
{
    if(platform_audio_enabled())
        platform_audio_off();
    else
        platform_audio_on();
}

#ifdef ARDUINO

void platform_audio_init()
{
    Arduboy2Audio::begin();
    ArduboyTones::ArduboyTones(Arduboy2Audio::enabled);
}

void platform_audio_on()
{
    Arduboy2Audio::on();
    static_assert(1 == VOLUME_ALWAYS_NORMAL, "");
    static_assert(2 == VOLUME_ALWAYS_HIGH, "");
    ArduboyTones::volumeMode(savefile.music_volume);
}

void platform_audio_off()
{
    Arduboy2Audio::off();
}

bool platform_audio_enabled()
{
    return Arduboy2Audio::enabled();
}

void platform_audio_play(uint16_t const* song)
{
    ArduboyTones::tonesInRAM(song);
}

void platform_audio_play_prog(uint16_t const* song)
{
    ArduboyTones::tones(song);
}

void platform_audio_stop()
{
    ArduboyTones::noTone();
}

bool platform_audio_playing()
{
    return ArduboyTones::playing();
}

#else

static bool audio_enabled;
void platform_audio_init() {}
void platform_audio_on() { audio_enabled = true; }
void platform_audio_off() { audio_enabled = false; }
bool platform_audio_enabled() { return audio_enabled; }
void platform_audio_play(uint16_t const* song) {}
void platform_audio_play_prog(uint16_t const* song) {}
void platform_audio_stop() {}
bool platform_audio_playing() { return false; }

#endif
