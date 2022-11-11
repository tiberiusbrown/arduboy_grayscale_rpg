#include "common.hpp"

#ifdef ARDUINO
#include "ArduboyFX.h"
#else
#include "generated/fxdata_emulated.hpp"
#include <SDL.h>
#include <assert.h>
#include <string.h>
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
    a.drawOverwriteMonochrome(x, y, w, h, bitmap);
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
    a.drawOverwrite(x, y, bitmap, frame);
#else
    uint8_t w = pgm_read_byte(bitmap++);
    uint8_t h = pgm_read_byte(bitmap++);
    platform_drawoverwritemonochrome(
        x, y, w, h, bitmap + ((frame * 2 + gplane) * (w * h / 8)));
#endif
}

#ifdef ARDUINO
static void platform_fx_drawbitmap(int16_t x, int16_t y, uint24_t address,
    uint16_t frame, uint8_t mode, int16_t width,
    int16_t height)
{
    // return if the bitmap is completely off screen
    if(x + width <= 0 || x >= WIDTH || y + height <= 0 || y >= HEIGHT) return;

    // determine render width
    int16_t skipleft = 0;
    uint8_t renderwidth;
    if(x < 0)
    {
        skipleft = -x;
        if(width - skipleft < WIDTH) renderwidth = width - skipleft;
        else renderwidth = WIDTH;
    }
    else {
        if(x + width > WIDTH) renderwidth = WIDTH - x;
        else renderwidth = width;
    }

    // determine render height
    int16_t skiptop;     // pixel to be skipped at the top
    int8_t renderheight; // in pixels
    if(y < 0)
    {
        skiptop = -y & -8; // optimized -y / 8 * 8
        if(height - skiptop <= HEIGHT) renderheight = height - skiptop;
        else renderheight = HEIGHT + (y & 7);
        skiptop >>= 3; // pixels to displayrows
    }
    else {
        skiptop = 0;
        if(y + height > HEIGHT) renderheight = HEIGHT - y;
        else renderheight = height;
    }
    uint24_t offset =
        (uint24_t)((frame * 2 + a.currentPlane()) * ((height + 7) / 8) +
            skiptop) *
        width +
        skipleft;
    if(mode & dbmMasked)
    {
        offset += offset; // double for masked bitmaps
        width += width;
    }
    address += offset; // skip non rendered pixels
    int8_t displayrow = (y >> 3) + skiptop;
    uint16_t displayoffset = displayrow * WIDTH + x + skipleft;
    uint8_t yshift = FX::bitShiftLeftUInt8(y); // shift by multiply
#ifdef ARDUINO_ARCH_AVR
    uint8_t rowmask;
    uint16_t bitmap;
    asm volatile(
        "1: ;render_row:                                \n"
        "   cbi     %[fxport], %[fxbit]                 \n"
        "   ldi     r24, %[cmd]                         \n" // writeByte(SFC_READ);
        "   out     %[spdr], r24                        \n"
        "   lds     r24, %[datapage]+0                  \n" // address +
                                                            // programDataPage;
        "   lds     r25, %[datapage]+1                  \n"
        "   add     r24, %B[address]                    \n"
        "   adc     r25, %C[address]                    \n"
        "   in      r0, %[spsr]                         \n" // wait()
        "   sbrs    r0, %[spif]                         \n"
        "   rjmp    .-6                                 \n"
        "   out     %[spdr], r25                        \n" // writeByte(address
                                                            // >> 16);
        "   in      r0, %[spsr]                         \n" // wait()
        "   sbrs    r0, %[spif]                         \n"
        "   rjmp    .-6                                 \n"
        "   out     %[spdr], r24                        \n" // writeByte(address
                                                            // >> 8);
        "   in      r0, %[spsr]                         \n" // wait()
        "   sbrs    r0, %[spif]                         \n"
        "   rjmp    .-6                                 \n"
        "   out     %[spdr], %A[address]                \n" // writeByte(address);
        "                                               \n"
        "   add     %A[address], %A[width]              \n" // address += width;
        "   adc     %B[address], %B[width]              \n"
        "   adc     %C[address], r1                     \n"
        "   in      r0, %[spsr]                         \n" // wait();
        "   sbrs    r0, %[spif]                         \n"
        "   rjmp    .-6                                 \n"
        "   out     %[spdr], r1                         \n" // SPDR = 0;
        "                                               \n"
        "   lsl     %[mode]                             \n" // 'clear' mode
                                                            // dbfExtraRow by
                                                            // shifting into
                                                            // carry
        "   cpi     %[displayrow], %[lastrow]           \n"
        "   brge    .+4                                 \n" // row >= lastrow,
                                                            // clear carry
        "   sec                                         \n" // row < lastrow set
                                                            // carry
        "   sbrc    %[yshift], 0                        \n" // yshift != 1,
                                                            // don't change
                                                            // carry state
        "   clc                                         \n" // yshift == 1,
                                                            // clear carry
        "   ror     %[mode]                             \n" // carry to mode
                                                            // dbfExtraRow
        "                                               \n"
        "   ldi     %[rowmask], 0x02                    \n" // rowmask = 0xFF >>
                                                            // (8 - (height &
                                                            // 7));
        "   sbrc    %[height], 1                        \n"
        "   ldi     %[rowmask], 0x08                    \n"
        "   sbrc    %[height], 2                        \n"
        "   swap    %[rowmask]                          \n"
        "   sbrs    %[height], 0                        \n"
        "   lsr     %[rowmask]                          \n"
        "   dec     %[rowmask]                          \n"
        "   breq    .+4                                 \n"
        "   cpi     %[renderheight], 8                  \n" // if (renderheight
                                                            // >= 8) rowmask =
                                                            // 0xFF;
        "   brlt    .+2                                 \n"
        "   ldi     %[rowmask], 0xFF                    \n"
        "                                               \n"
        "   mov     r25, %[renderwidth]                 \n" // for (c <
                                                            // renderwidth)
        "2: ;render_column:                             \n"
        "   in      r0, %[spdr]                         \n" // read bitmap data
        "   out     %[spdr], r1                         \n" // start next read
        "                                               \n"
        "   sbrc    %[mode], %[reverseblack]            \n" // test reverse mode
        "   eor     r0, %[rowmask]                      \n" // reverse bitmap
                                                            // data
        "   mov     r24, %[rowmask]                     \n" // temporary move
                                                            // rowmask
        "   sbrc    %[mode], %[whiteblack]              \n" // for black and
                                                            // white modes:
        "   mov     r24, r0                             \n" // rowmask = bitmap
        "   sbrc    %[mode], %[black]                   \n" // for black mode:
        "   clr     r0                                  \n" // bitmap = 0
        "   mul     r0, %[yshift]                       \n"
        "   movw    %[bitmap], r0                       \n" // bitmap *= yshift
        "   bst     %[mode], %[masked]                  \n" // if bitmap has no
                                                            // mask:
        "   brtc    3f ;render_mask                     \n" // skip next part
        "                                               \n"
        "   lpm                                         \n" // above code took
                                                            // 11 cycles, wait 7
                                                            // cycles more for
                                                            // SPI data ready
        "   lpm                                         \n"
        "   clr     r1                                  \n" // restore zero reg
        "                                               \n"
        "   in      r0, %[spdr]                         \n" // read mask data
        "   out     %[spdr],r1                          \n" // start next read
        "   sbrc    %[mode], %[whiteblack]              \n" //
        "3: ;render_mask:                               \n"
        "   mov     r0, r24                             \n" // get mask in r0
        "   mul     r0, %[yshift]                       \n" // mask *= yshift
        ";render_page0:                                 \n"
        "   cpi     %[displayrow], 0                    \n" // skip if
                                                            // displayrow < 0
        "   brlt    4f ;render_page1                    \n"
        "                                               \n"
        "   ld      r24, %a[buffer]                     \n" // do top row or to
                                                            // row half
        "   sbrs    %[mode],%[invert]                   \n" // skip 1st eor for
                                                            // invert mode
        "   eor     %A[bitmap], r24                     \n"
        "   and     %A[bitmap], r0                      \n" // and with mask LSB
        "   eor     %A[bitmap], r24                     \n"
        "   st      %a[buffer], %A[bitmap]              \n"
        "4: ;render_page1:                              \n"
        "   subi    %A[buffer], lo8(-%[displaywidth])   \n"
        "   sbci    %B[buffer], hi8(-%[displaywidth])   \n"
        "   sbrs    %[mode], %[extrarow]                \n" // test if ExtraRow
                                                            // mode:
        "   rjmp    5f ;render_next                     \n" // else skip
        "                                               \n"
        "   ld      r24, %a[buffer]                     \n" // do shifted 2nd
                                                            // half
        "   sbrs    %[mode], %[invert]                  \n" // skip 1st eor for
                                                            // invert mode
        "   eor     %B[bitmap], r24                     \n"
        "   and     %B[bitmap], r1                      \n" // and with mask MSB
        "   eor     %B[bitmap], r24                     \n"
        "   st      %a[buffer], %B[bitmap]              \n"
        "5: ;render_next:                               \n"
        "   clr     r1                                  \n" // restore zero reg
        "   subi    %A[buffer], lo8(%[displaywidth]-1)  \n"
        "   sbci    %B[buffer], hi8(%[displaywidth]-1)  \n"
        "   dec     r25                                 \n"
        "   brne    2b ;render_column                   \n" // for (c <
                                                            // renderheigt) loop
        "                                               \n"
        "   subi    %A[buffer], lo8(-%[displaywidth])   \n" // buffer += WIDTH -
                                                            // renderwidth
        "   sbci    %B[buffer], hi8(-%[displaywidth])   \n"
        "   sub     %A[buffer], %[renderwidth]          \n"
        "   sbc     %B[buffer], r1                      \n"
        "   subi    %[renderheight], 8                  \n" // reinderheight -=
                                                            // 8
        "   inc     %[displayrow]                       \n" // displayrow++
        "   in      r0, %[spsr]                         \n" // clear SPI status
        "   sbi     %[fxport], %[fxbit]                 \n" // disable external
                                                            // flash
        "   cp      r1, %[renderheight]                 \n" // while
                                                            // (renderheight >
                                                            // 0)
        "   brge    .+2                                 \n"
        "   rjmp    1b ;render_row                      \n"
        : [address] "+r"(address), [mode] "+r"(mode), [rowmask] "=&d"(rowmask),
        [bitmap] "=&r"(bitmap), [renderheight] "+d"(renderheight),
        [displayrow] "+d"(displayrow)
        : [width] "r"(width), [height] "r"(height), [yshift] "r"(yshift),
        [renderwidth] "r"(renderwidth),
        [buffer] "e"(Arduboy2Base::sBuffer + displayoffset),

        [fxport] "I"(_SFR_IO_ADDR(FX_PORT)), [fxbit] "I"(FX_BIT),
        [cmd] "I"(SFC_READ), [spdr] "I"(_SFR_IO_ADDR(SPDR)),
        [datapage] ""(&FX::programDataPage), [spsr] "I"(_SFR_IO_ADDR(SPSR)),
        [spif] "I"(SPIF), [lastrow] "I"(HEIGHT / 8 - 1),
        [displaywidth] ""(WIDTH), [reverseblack] "I"(dbfReverseBlack),
        [whiteblack] "I"(dbfWhiteBlack), [black] "I"(dbfBlack),
        [masked] "I"(dbfMasked), [invert] "I"(dbfInvert),
        [extrarow] "I"(dbfExtraRow)
        : "r24", "r25");
#else
    uint8_t lastmask =
        bitShiftRightMaskUInt8(8 - height); // mask for bottom most pixels
    do
    {
        seekData(address);
        address += width;
        mode &= ~(_BV(dbfExtraRow));
        if(yshift != 1 && displayrow < (HEIGHT / 8 - 1))
            mode |= _BV(dbfExtraRow);
        uint8_t rowmask = 0xFF;
        if(renderheight < 8) rowmask = lastmask;
        wait();
        for(uint8_t c = 0; c < renderwidth; c++)
        {
            uint8_t bitmapbyte = readUnsafe();
            if(mode & _BV(dbfReverseBlack)) bitmapbyte ^= rowmask;
            uint8_t maskbyte = rowmask;
            if(mode & _BV(dbfWhiteBlack)) maskbyte = bitmapbyte;
            if(mode & _BV(dbfBlack)) bitmapbyte = 0;
            uint16_t bitmap = multiplyUInt8(bitmapbyte, yshift);
            if(mode & _BV(dbfMasked))
            {
                wait();
                uint8_t tmp = readUnsafe();
                if((mode & dbfWhiteBlack) == 0) maskbyte = tmp;
            }
            uint16_t mask = multiplyUInt8(maskbyte, yshift);
            if(displayrow >= 0)
            {
                uint8_t pixels = bitmap;
                uint8_t display = Arduboy2Base::sBuffer[displayoffset];
                if((mode & _BV(dbfInvert)) == 0) pixels ^= display;
                pixels &= mask;
                pixels ^= display;
                Arduboy2Base::sBuffer[displayoffset] = pixels;
            }
            if(mode & _BV(dbfExtraRow))
            {
                uint8_t display = Arduboy2Base::sBuffer[displayoffset + WIDTH];
                uint8_t pixels = bitmap >> 8;
                if((mode & dbfInvert) == 0) pixels ^= display;
                pixels &= mask >> 8;
                pixels ^= display;
                Arduboy2Base::sBuffer[displayoffset + WIDTH] = pixels;
            }
            displayoffset++;
        }
        displayoffset += WIDTH - renderwidth;
        displayrow++;
        renderheight -= 8;
        readEnd();
    } while(renderheight > 0);
#endif
}
#endif

void platform_fx_drawoverwrite(int16_t x, int16_t y, uint24_t addr,
    uint16_t frame, uint8_t w, uint8_t h)
{
#ifdef ARDUINO
    platform_fx_drawbitmap(x, y, addr, frame, dbmOverwrite, w, h);
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
    platform_fx_drawbitmap(x, y, addr, frame, dbmMasked, w, h);
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
