#include "common.hpp"

#ifdef ARDUINO
#define SPRITESU_IMPLEMENTATION
#define SPRITESU_OVERWRITE
#define SPRITESU_FX
#define SPRITESU_RECT
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

#ifndef ARDUINO
extern float fade_factor;
#endif

void platform_fade(uint8_t f)
{
    if(f > 15) f = 15;
    f *= 17;

#ifdef ARDUINO
    static uint8_t const CONTRAST[] PROGMEM = { 0x20, 0x60, 0x90, 0xff };
#else
    static uint8_t const CONTRAST[] PROGMEM = { 0x6f, 0x9f, 0xcf, 0xff };
#endif
    uint8_t brightness = pgm_read_byte(&CONTRAST[savefile.settings.brightness]);
    f = (f * brightness + f) >> 8;

#ifdef ARDUINO
    FX::enableOLED();
    abg_detail::send_cmds(/* 0xDB, vcom_des, */ 0x81, f);
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

// specialized draw method for 8px high characters
void platform_drawcharfast(
    uint8_t x, uint8_t page_start, uint8_t shift_coef,
    uint8_t w, uint16_t shift_mask, uint8_t const* bitmap)
{
#ifdef ARDUINO
    uint16_t image_data;
    uint8_t buf_data;
    uint8_t count;

    uint8_t* buf = Arduboy2Base::sBuffer;
    asm volatile(
        "mul %[page_start], %[c128]\n"
        "add %A[buf], r0\n"
        "adc %B[buf], r1\n"
        "clr __zero_reg__\n"
        "add %A[buf], %[x]\n"
        "adc %B[buf], __zero_reg__\n"
        :
        [buf] "+&x" (buf)
        :
        [page_start] "r"   (page_start),
        [x]          "r"   (x),
        [c128]       "r"   ((uint8_t)128)
        );

    asm volatile(R"ASM(

                cpi %[page_start], 7
                breq L%=_bottom

                ; tests if 8-px aligned vertically
                ;cpi %[shift_coef], 1
                ;breq L%=_bottom

                ; need Y pointer for middle pages
                ;push r28
                ;push r29
                movw r28, %[buf]
                subi r28, lo8(-128)
                sbci r29, hi8(-128)
                mov %[count], %[cols]

            L%=_middle:

                ; write one page from image to buf/buf+128
                lpm %A[image_data], %a[image]+
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %A[shift_mask]
                or %[buf_data], r0
                st %a[buf]+, %[buf_data]
                ld %[buf_data], Y
                and %[buf_data], %B[shift_mask]
                or %[buf_data], r1
                st Y+, %[buf_data]
                dec %[count]
                brne L%=_middle

                ; done with Y pointer
                ;pop r29
                ;pop r28

                jmp L%=_finish

            L%=_bottom:

                ; write one page from image to buf
                lpm %A[image_data], %a[image]+
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %A[shift_mask]
                or %[buf_data], r0
                st %a[buf]+, %[buf_data]
                dec %[cols]
                brne L%=_bottom

            L%=_finish:

                clr __zero_reg__

            )ASM"
        :
        [buf]        "+&x" (buf),
        [image]      "+&z" (bitmap),
        [count]      "=&r" (count),
        [buf_data]   "=&r" (buf_data),
        [image_data] "=&r" (image_data)
        :
        [cols]       "r"   (w),
        [shift_mask] "r"   (shift_mask),
        [shift_coef] "r"   (shift_coef),
        [page_start] "r"   (page_start)
        :
        "r28", "r29", "memory"
        );
#else
    uint8_t oy = 0;
    while(shift_coef > 1)
        ++oy, shift_coef >>= 1;
    MY_ASSERT(page_start <= 7);
    MY_ASSERT(x + w <= 128);
    if(page_start > 7 || x + w > 128)
        return;
    platform_drawoverwritemonochrome(
        x, page_start * 8 + oy, w, 8, bitmap);
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
    uint8_t const* bitmap = &FXDATA[addr + 2];
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

void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame)
{
#ifdef ARDUINO
    SpritesU::drawOverwriteFX(x, y, addr, frame * PLANES + a.currentPlane());
#else
    uint8_t w = FXDATA[addr + 0];
    uint8_t h = FXDATA[addr + 1];
    platform_fx_drawoverwrite(x, y, addr, frame, w, h);
#endif
}

void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr)
{
    platform_fx_drawoverwrite(x, y, addr, 0);
}

void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h)
{
#ifdef ARDUINO
    SpritesU::drawPlusMaskFX(x, y, w, h, addr, frame * PLANES + a.currentPlane());
#else
    assert(SDL_GetTicks64() >= ticks_when_ready);
    uint8_t const* bitmap = &FXDATA[addr + 2];
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

void platform_fx_drawplusmask(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame)
{
#ifdef ARDUINO
    SpritesU::drawPlusMaskFX(x, y, addr, frame * PLANES + a.currentPlane());
#else
    uint8_t w = FXDATA[addr + 0];
    uint8_t h = FXDATA[addr + 1];
    platform_fx_drawplusmask(x, y, addr, frame, w, h);
#endif
}

void platform_fillrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c)
{
#ifdef ARDUINO
    c = (c > plane()) ? 1 : 0;
    SpritesU::fillRect(x, y, w, h, c);
#else
    c = (c > plane()) ? 1 : 0;
    for(uint8_t i = 0; i != h; ++i)
        for(uint8_t j = 0; j != w; ++j)
            if(unsigned(y + i) < 64 && unsigned(x + j) < 128)
                pixels[gplane][(y + i) * 128 + (x + j)] = c;
#endif
}

void platform_fillrect_i8(int8_t x, int8_t y, uint8_t w, uint8_t h, uint8_t c)
{
#ifdef ARDUINO
    c = (c > plane()) ? 1 : 0;
    SpritesU::fillRect_i8(x, y, w, h, c);
#else
    platform_fillrect(x, y, w, h, c);
#endif
}

void platform_drawrect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t c)
{
    platform_fillrect(x, y, w, 1, c);
    platform_fillrect(x, y + h - 1, w, 1, c);
    platform_fillrect(x, y, 1, h, c);
    platform_fillrect(x + w - 1, y, 1, h, c);
}

void platform_drawrect_i8(int8_t x, int8_t y, uint8_t w, uint8_t h, uint8_t c)
{
    platform_fillrect_i8(x, y, w, 1, c);
    platform_fillrect_i8(x, y + h - 1, w, 1, c);
    platform_fillrect_i8(x, y, 1, h, c);
    platform_fillrect_i8(x + w - 1, y, 1, h, c);
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
    size_t i = 0;
    uint8_t const* buffer = (uint8_t const*)data;
    do
    {
        FX::writeByte(buffer[i]);
    } while(++i < num);
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

static uint24_t current_song;

void platform_audio_toggle()
{
    if(platform_audio_enabled())
        platform_audio_off();
    else
        platform_audio_on();
}

void platform_set_game_speed_default()
{
    platform_set_game_speed(1, 1);
}

void platform_set_game_speed_saved()
{
    static uint8_t const SPEEDS[14] PROGMEM =
    {
        2, 1, 11, 7, 26, 21, 1, 1, 2, 3, 1, 2, 1, 3,
    };
    uint8_t const* ptr = &SPEEDS[savefile.settings.game_speed * 2];
    uint8_t num = pgm_read_byte_inc(ptr);
    uint8_t denom = pgm_read_byte(ptr);
    platform_set_game_speed(num, denom);
}

void platform_audio_play_song(uint24_t song)
{
    current_song = song;
}

#ifdef ARDUINO

static void update_song_buffer(atm_pattern_state& p)
{
    uint8_t buf[ATM_CMD_BUF_SIZE];
    uint8_t i = p.next_cmd_ptr;
    uint24_t addr = p.addr;
    platform_fx_read_data_bytes(addr, buf, ATM_CMD_BUF_SIZE);
    uint8_t const* ptr = buf;
    for(uint8_t n = 0; n < ATM_CMD_BUF_SIZE; ++n)
        p.cmds[(i + n) & (ATM_CMD_BUF_SIZE - 1)] = deref_inc(ptr);
}

void update_score_channels()
{
    update_song_buffer(atmlib_state.score_state.channel_state[0].pstack[0]);
    update_song_buffer(atmlib_state.score_state.channel_state[1].pstack[0]);
}

void platform_audio_update()
{
    if(platform_audio_song_playing())
    {
        update_score_channels();
    }
    else
        platform_audio_play_song_now(current_song);
    if(atmlib_state.sfx_slot[0].player_state.channel_active_mask != 0)
        update_song_buffer(atmlib_state.sfx_slot[0].channel_state[0].pstack[0]);
}

void platform_audio_init()
{
    atm_synth_setup();
}

void platform_audio_on()
{
    Arduboy2Audio::on();
}

void platform_audio_off()
{
    Arduboy2Audio::off();
}

bool platform_audio_enabled()
{
    return Arduboy2Audio::enabled();
}

static void init_channel(atm_channel_state& c, uint24_t addr)
{
    auto* osc = c.dst_osc_params;
    memset(&c, 0, sizeof(atm_channel_state));
    c.dst_osc_params = osc;
    c.pstack[0].addr = addr;
    platform_fx_read_data_bytes(addr, c.pstack[0].cmds, ATM_CMD_BUF_SIZE);
}

void platform_audio_play_song_now(uint24_t song)
{
    current_song = song;
    if(savefile.settings.sound & 2)
    {
        uint16_t offsets[ATM_SCORE_CHANNEL_COUNT];
        platform_fx_read_data_bytes(song, offsets, sizeof(offsets));
        init_channel(atmlib_state.score_state.channel_state[0], song + offsets[0]);
        init_channel(atmlib_state.score_state.channel_state[1], song + offsets[1]);
        atm_synth_start_score();
    }
}

void platform_audio_play_sfx(uint24_t sfx)
{
    if(savefile.settings.sound & 1)
    {
        init_channel(atmlib_state.sfx_slot[0].channel_state[0], sfx);
        atm_synth_play_sfx_track(1, 0);
    }
}

void platform_audio_from_savefile()
{
    if(atm_synth_is_score_playing())
        atm_synth_set_score_paused((savefile.settings.sound & 2) == 0);
    if((savefile.settings.sound != 0) != platform_audio_enabled())
        platform_audio_toggle();
}

bool platform_audio_song_playing()
{
    return atm_synth_is_score_playing() != 0;
}

void platform_set_game_speed(uint8_t num, uint8_t denom)
{
    a.setUpdateEveryN(num, denom);
}

#else

static bool audio_enabled;
void platform_audio_from_savefile()
{
    if((savefile.settings.sound != 0) != platform_audio_enabled())
        platform_audio_toggle();
}
void platform_audio_init() {}
void platform_audio_on() { audio_enabled = true; }
void platform_audio_off() { audio_enabled = false; }
bool platform_audio_enabled() { return audio_enabled; }
void platform_audio_play_song_now(uint24_t song)
{
    current_song = song;
}
void platform_audio_play_sfx(uint24_t sfx) {}
bool platform_audio_song_playing() { return true; }

extern float target_frame_time, ft_rem;
void platform_set_game_speed(uint8_t num, uint8_t denom)
{
    target_frame_time = (1.f / 52) * num / denom;
    if(ft_rem >= target_frame_time) ft_rem = 0.f;
}

#endif
