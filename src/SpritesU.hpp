#pragma once

#ifndef ARDUBOYFX_H
#error "Include ArduboyFX.h prior to SpritesU.hpp"
#endif

struct SpritesU
{
    static void drawOverwrite(
        int16_t x, int16_t y, uint8_t const* image, uint16_t frame);
    static void drawOverwrite(
        int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t const* image);
    static void drawPlusMask(
        int16_t x, int16_t y, uint8_t const* image, uint16_t frame);
    static void drawPlusMask(
        int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t const* image);
        
    static void drawOverwriteFX(
        int16_t x, int16_t y, uint8_t w, uint8_t h, uint24_t image, uint16_t frame);
    static void drawPlusMaskFX(
        int16_t x, int16_t y, uint8_t w, uint8_t h, uint24_t image, uint16_t frame);
};

#ifdef SPRITESU_IMPLEMENTATION

constexpr uint8_t SPRITESU_MODE_OVERWRITE   = 0;
constexpr uint8_t SPRITESU_MODE_PLUSMASK    = 1;
constexpr uint8_t SPRITESU_MODE_OVERWRITEFX = 2;
constexpr uint8_t SPRITESU_MODE_PLUSMASKFX  = 3;

static void SpritesU_DrawCommon(
    int16_t x, int16_t y, uint8_t w, uint8_t h,
    uint24_t image, uint16_t frame, uint8_t mode)
{
    if(x >= 128) return;
    if(y >= 64)  return;
    if(x + w <= 0) return;
    if(y + h <= 0) return;
    
    uint8_t pages = h;
    asm volatile(
        "lsr %[pages]\n"
        "lsr %[pages]\n"
        "lsr %[pages]\n"
        : [pages] "+&r" (pages));
    
    {
        uint8_t frame_pages = pages;
        if(mode & 1) frame_pages *= 2;
        image += uint16_t((frame_pages * w) * frame);
    }
    
    uint8_t shift_coef = FX::bitShiftLeftUInt8(y);
    uint16_t shift_mask = ~(0xff * shift_coef);

    // y /= 8 (round to -inf)
    asm volatile(
        "asr %B[y]\n"
        "ror %A[y]\n"
        "asr %B[y]\n"
        "ror %A[y]\n"
        "asr %B[y]\n"
        "ror %A[y]\n"
        : [y] "+&r" (y));
    
    // clip against top edge
    int8_t page_start = int8_t(y);
    if(page_start < -1)
    {
        uint8_t tp = (-1 - page_start);
        pages -= tp;
        if(mode & 1) tp *= 2;
        image += tp * w;
        page_start = -1;
    }
    
    // clip against left edge
    uint8_t cols = w;
    uint8_t col_start = x;
    if(x < 0)
    {
        int16_t t = x;
        if(mode & 1) t *= 2;
        image -= t;
        cols += x;
        col_start = 0;
    }
    
    // TODO: compiler for this is dumb, use asm
    uint8_t* buf = Arduboy2Base::sBuffer;
    buf += page_start * 128;
    buf += col_start;

    // clip against right edge
    {
        uint8_t max_cols = 128 - col_start;
        if(cols > max_cols)
            cols = max_cols;
    }
    
    // clip against bottom edge
    bool bottom = false; 
    if(pages > 7 - page_start)
    {
        pages = 7 - page_start;
        bottom = true;
    }
    
    uint8_t buf_adv = 128 - cols;
    uint16_t image_adv = w;
    if(!(mode & 2)) image_adv -= cols;
    if(mode & 1) image_adv *= 2;
    
    uint16_t image_data;
    uint16_t mask_data;
    uint8_t buf_data;
    uint8_t count;
    
    if(mode == SPRITESU_MODE_OVERWRITE)
    {
        uint8_t const* image_ptr = (uint8_t const*)image;
        asm volatile(R"ASM(
        
                cpi %[page_start], 0
                brge L%=_middle
                
                ; advance buf to next page
                subi %A[buf], lo8(-128)
                sbci %B[buf], hi8(-128)
                mov %[count], %[cols]
                
            L%=_top_loop:
            
                ; write one page from image to buf+128
                lpm %A[image_data], %a[image]+
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %B[shift_mask]
                or %[buf_data], r1
                st %a[buf]+, %[buf_data]
                dec %[count]
                brne L%=_top_loop
                
                ; decrement pages, reset buf back, advance image
                clr __zero_reg__
                dec %[pages]
                sub %A[buf], %[cols]
                sbc %B[buf], __zero_reg__
                add %A[image], %A[image_adv]
                adc %B[image], %B[image_adv]
        
            L%=_middle:
            
                tst %[pages]
                breq L%=_bottom
            
                ; need Y pointer for middle pages
                push r28
                push r29
                movw r28, %[buf]
                subi r28, lo8(-128)
                sbci r29, hi8(-128)
                
            L%=_middle_loop_outer:
            
                mov %[count], %[cols]
                
            L%=_middle_loop_inner:
            
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
                brne L%=_middle_loop_inner
                
                ; advance buf, buf+128, and image to the next page
                clr __zero_reg__
                add %A[buf], %[buf_adv]
                adc %B[buf], __zero_reg__
                add r28, %[buf_adv]
                adc r29, __zero_reg__
                add %A[image], %A[image_adv]
                adc %B[image], %B[image_adv]
                dec %[pages]
                brne L%=_middle_loop_outer
                
                ; done with Y pointer
                pop r29
                pop r28
                
            L%=_bottom:
            
                tst %[bottom]
                breq L%=_finish
                
            L%=_bottom_loop:
            
                ; write one page from image to buf
                lpm %A[image_data], %a[image]+
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %A[shift_mask]
                or %[buf_data], r0
                st %a[buf]+, %[buf_data]
                dec %[cols]
                brne L%=_bottom_loop
            
            L%=_finish:
            
                clr __zero_reg__
                
            )ASM"
            :
            [buf]        "+&x" (buf),
            [image]      "+&z" (image_ptr),
            [pages]      "+&a" (pages),
            [count]      "=&l" (count),
            [buf_data]   "=&a" (buf_data),
            [image_data] "=&a" (image_data)
            :
            [cols]       "l"   (cols),
            [buf_adv]    "l"   (buf_adv),
            [image_adv]  "l"   (image_adv),
            [shift_mask] "l"   (shift_mask),
            [shift_coef] "l"   (shift_coef),
            [bottom]     "a"   (bottom),
            [page_start] "a"   (page_start)
            );
    }
    else if(mode == SPRITESU_MODE_PLUSMASK)
    {
        uint8_t const* image_ptr = (uint8_t const*)image;
        asm volatile(R"ASM(
        
                cpi %[page_start], 0
                brge L%=_middle
                
                ; advance buf to next page
                subi %A[buf], lo8(-128)
                sbci %B[buf], hi8(-128)
                mov %[count], %[cols]
                
            L%=_top_loop:
                
                ; write one page from image to buf+128
                lpm %A[image_data], %a[image]+
                lpm %A[mask_data], %a[image]+
                
                mul %A[image_data], %[shift_coef]
                movw %[image_data], r0
                mul %A[mask_data], %[shift_coef]
                movw %[mask_data], r0
                
                ld %[buf_data], %a[buf]
                com %B[mask_data]
                and %[buf_data], %B[mask_data]
                or %[buf_data], %B[image_data]
                st %a[buf]+, %[buf_data]
                dec %[count]
                brne L%=_top_loop
                
                ; decrement pages, reset buf back, advance image and mask
                clr __zero_reg__
                dec %[pages]
                sub %A[buf], %[cols]
                sbc %B[buf], __zero_reg__
                add %A[image], %[image_adv]
                adc %B[image], __zero_reg__
        
            L%=_middle:
            
                tst %[pages]
                breq L%=_bottom
            
                ; need Y pointer for middle pages
                push r28
                push r29
                movw r28, %[buf]
                subi r28, lo8(-128)
                sbci r29, hi8(-128)
                
            L%=_middle_loop_outer:
            
                mov %[count], %[cols]
                
            L%=_middle_loop_inner:
                ; write one page from image to buf/buf+128
                lpm %A[image_data], %a[image]+
                lpm %A[mask_data], %a[image]+
                
                mul %A[image_data], %[shift_coef]
                movw %[image_data], r0
                mul %A[mask_data], %[shift_coef]
                movw %[mask_data], r0
                
                ld %[buf_data], %a[buf]
                com %A[mask_data]
                and %[buf_data], %A[mask_data]
                or %[buf_data], %A[image_data]
                st %a[buf]+, %[buf_data]
                ld %[buf_data], Y
                com %B[mask_data]
                and %[buf_data], %B[mask_data]
                or %[buf_data], %B[image_data]
                st Y+, %[buf_data]
                dec %[count]
                brne L%=_middle_loop_inner
                
                ; advance buf, buf+128, and image to the next page
                clr __zero_reg__
                add %A[buf], %[buf_adv]
                adc %B[buf], __zero_reg__
                add r28, %[buf_adv]
                adc r29, __zero_reg__
                add %A[image], %[image_adv]
                adc %B[image], __zero_reg__
                dec %[pages]
                brne L%=_middle_loop_outer
                
                ; done with Y pointer
                pop r29
                pop r28
                
            L%=_bottom:
            
                tst %[bottom]
                breq L%=_finish
                
            L%=_bottom_loop:
            
                ; write one page from image to buf
                lpm %A[image_data], %a[image]+
                lpm %A[mask_data], %a[image]+
                mul %A[image_data], %[shift_coef]
                movw %[image_data], r0
                mul %A[mask_data], %[shift_coef]
                movw %[mask_data], r0
                
                ld %[buf_data], %a[buf]
                com %A[mask_data]
                and %[buf_data], %A[mask_data]
                or %[buf_data], %A[image_data]
                st %a[buf]+, %[buf_data]
                dec %[cols]
                brne L%=_bottom_loop
            
            L%=_finish:
            
                clr __zero_reg__
                
            )ASM"
            :
            [buf]        "+&x" (buf),
            [image]      "+&z" (image_ptr),
            [pages]      "+&l" (pages),
            [count]      "=&l" (count),
            [buf_data]   "=&l" (buf_data),
            [image_data] "=&l" (image_data),
            [mask_data]  "=&l" (mask_data)
            :
            [cols]       "l"   (cols),
            [buf_adv]    "l"   (buf_adv),
            [image_adv]  "l"   (image_adv),
            [shift_coef] "l"   (shift_coef),
            [bottom]     "a"   (bottom),
            [page_start] "a"   (page_start)
            );
    }
    else
    {
#if 1
        uint8_t* bufn;
        uint8_t reseek = (w != cols);
        image += ((uint24_t)FX::programDataPage << 8);
        asm volatile(R"ASM(
                
                rjmp L%=_begin
                
            L%=_seek:
            
                ; seek subroutine
                cbi %[fxport], %[fxbit]
                nop
                out %[spdr], %[sfc_read]
                add %A[image], %A[image_adv]
                adc %B[image], %B[image_adv]
                adc %C[image], __zero_reg__
                in r0, %[spsr]
                sbrs r0, %[spif]
                rjmp .-6
                out %[spdr], %C[image]
                in r0, %[spsr]
                sbrs r0, %[spif]
                rjmp .-6
                out %[spdr], %B[image]
                in r0, %[spsr]
                sbrs r0, %[spif]
                rjmp .-6
                out %[spdr], %A[image]
                in r0, %[spsr]
                sbrs r0, %[spif]
                rjmp .-6
                out %[spdr], __zero_reg__
                in r0, %[spsr]
                sbrs r0, %[spif]
                rjmp .-6
                ret
                
            L%=_begin:
            
                ; initial seek
                sub %A[image], %A[image_adv]
                sbc %B[image], %B[image_adv]
                sbc %C[image], __zero_reg__
                rcall L%=_seek
                cpi %[page_start], 0
                brlt L%=_top
                tst %[pages]
                brne L%=_middle_skip_reseek
                rjmp L%=_bottom_dispatch
                
            L%=_top:
                
                ; init buf
                subi %A[buf], lo8(-128)
                sbci %B[buf], hi8(-128)
                mov %[count], %[cols]
                
                ; loop dispatch
                sbrc %[mode], 0
                rjmp L%=_top_loop_masked
                
            L%=_top_loop:
                
                in %A[image_data], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %B[shift_mask]
                or %[buf_data], r1
                st %a[buf]+, %[buf_data]
                lpm
                adiw r24, 0
                dec %[count]
                brne L%=_top_loop
                rjmp L%=_top_loop_done
                
            L%=_top_loop_masked:
                
                in %A[image_data], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[image_data], %[shift_coef]
                movw %A[image_data], r0
                lpm
                lpm
                lpm
                lpm
                nop
                in %A[shift_mask], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[shift_mask], %[shift_coef]
                movw %A[shift_mask], r0
                ld %[buf_data], %a[buf]
                com %B[shift_mask]
                and %[buf_data], %B[shift_mask]
                or %[buf_data], %B[image_data]
                st %a[buf]+, %[buf_data]
                lpm
                dec %[count]
                brne L%=_top_loop_masked
            
            L%=_top_loop_done:
                
                ; decrement pages, reset buf back
                clr __zero_reg__
                sub %A[buf], %[cols]
                sbc %B[buf], __zero_reg__
                dec %[pages]
                brne L%=_middle
                rjmp L%=_finish
            
            L%=_middle:
                
                ; only seek again if necessary
                tst %[reseek]
                breq L%=_middle_skip_reseek
                in r0, %[spsr]
                sbi %[fxport], %[fxbit]
                lpm
                rcall L%=_seek
            
            L%=_middle_skip_reseek:
            
                movw %[bufn], %[buf]
                subi %A[bufn], lo8(-128)
                sbci %B[bufn], hi8(-128)
            
            L%=_middle_loop_outer:
            
                mov %[count], %[cols]
                
                ; loop dispatch
                sbrc %[mode], 0
                rjmp L%=_middle_loop_inner_masked
                
            L%=_middle_loop_inner:
            
                ; write one page from image to buf/buf+128
                in %A[image_data], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %A[shift_mask]
                or %[buf_data], r0
                st %a[buf]+, %[buf_data]
                ld %[buf_data], %a[bufn]
                and %[buf_data], %B[shift_mask]
                or %[buf_data], r1
                st %a[bufn]+, %[buf_data]
                dec %[count]
                brne L%=_middle_loop_inner
                rjmp L%=_middle_loop_outer_next
                
            L%=_middle_loop_inner_masked:
            
                ; write one page from image to buf/buf+128
                in %A[image_data], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[image_data], %[shift_coef]
                movw %A[image_data], r0
                ld %[buf_data], %a[buf]
                ld %B[shift_mask], %a[bufn]
                lpm
                lpm
                lpm
                in %A[shift_mask], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[shift_mask], %[shift_coef]
                com r0
                and %[buf_data], r0
                or %[buf_data], %A[image_data]
                st %a[buf]+, %[buf_data]
                com r1
                and %B[shift_mask], r1
                or %B[shift_mask], %B[image_data]
                st %a[bufn]+, %B[shift_mask]
                nop
                dec %[count]
                brne L%=_middle_loop_inner_masked
                
            L%=_middle_loop_outer_next:
                
                ; advance buf to the next page
                clr __zero_reg__
                add %A[buf], %[buf_adv]
                adc %B[buf], __zero_reg__
                dec %[pages]
                brne L%=_middle
                
            L%=_bottom:
            
                tst %[bottom]
                breq L%=_finish
                
                ; seek if needed
                tst %[reseek]
                breq L%=_bottom_dispatch
                in r0, %[spsr]
                sbi %[fxport], %[fxbit]
                lpm
                rcall L%=_seek
                
            L%=_bottom_dispatch:
                
                ; loop dispatch
                sbrc %[mode], 0
                rjmp L%=_bottom_loop_masked
                
            L%=_bottom_loop:
            
                ; write one page from image to buf
                in %A[image_data], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[image_data], %[shift_coef]
                ld %[buf_data], %a[buf]
                and %[buf_data], %A[shift_mask]
                or %[buf_data], r0
                st %a[buf]+, %[buf_data]
                lpm
                adiw r24, 0
                dec %[cols]
                brne L%=_bottom_loop
                rjmp L%=_finish
                
            L%=_bottom_loop_masked:
            
                ; write one page from image to buf
                in %A[image_data], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[image_data], %[shift_coef]
                movw %A[image_data], r0
                lpm
                lpm
                lpm
                lpm
                nop
                in %A[shift_mask], %[spdr]
                out %[spdr], __zero_reg__
                mul %A[shift_mask], %[shift_coef]
                movw %A[shift_mask], r0
                ld %[buf_data], %a[buf]
                com %A[shift_mask]
                and %[buf_data], %A[shift_mask]
                or %[buf_data], %A[image_data]
                st %a[buf]+, %[buf_data]
                lpm
                dec %[cols]
                brne L%=_bottom_loop_masked
                lpm

            L%=_finish:
            
                clr __zero_reg__
                sbi %[fxport], %[fxbit]
                in r0, %[spsr]
                
            )ASM"
            :
            [buf]        "+&x" (buf),
            [bufn]       "=&z" (bufn),
            [image]      "+&r" (image),
            [pages]      "+&r" (pages),
            [count]      "=&r" (count),
            [buf_data]   "=&r" (buf_data),
            [image_data] "=&r" (image_data),
            [shift_mask] "+&r" (shift_mask)
            :
            [cols]       "r"   (cols),
            [buf_adv]    "r"   (buf_adv),
            [image_adv]  "r"   (image_adv),
            [shift_coef] "r"   (shift_coef),
            [bottom]     "d"   (bottom),
            [page_start] "d"   (page_start),
            [reseek]     "r"   (reseek),
            [mode]       "r"   (mode),
            [sfc_read]   "r"   (SFC_READ),
            [fxport]     "I"   (_SFR_IO_ADDR(FX_PORT)),
            [fxbit]      "I"   (FX_BIT),
            [spdr]       "I"   (_SFR_IO_ADDR(SPDR)),
            [datapage]   ""    (&FX::programDataPage),
            [spsr]       "I"   (_SFR_IO_ADDR(SPSR)),
            [spif]       "I"   (SPIF)
            );
#endif
    }
}

void SpritesU::drawOverwrite(
    int16_t x, int16_t y, uint8_t const* image, uint16_t frame)
{
    uint8_t w, h;
    asm volatile(
        "lpm %[w], Z+\n"
        "lpm %[h], Z+\n"
        : [w] "=r" (w), [h] "=r" (h), [image] "+z" (image));
    SpritesU_DrawCommon(x, y, w, h, (uint24_t)image, frame, SPRITESU_MODE_OVERWRITE);
}

void SpritesU::drawOverwrite(
    int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t const* image)
{
    SpritesU_DrawCommon(x, y, w, h, (uint24_t)image, 0, SPRITESU_MODE_OVERWRITE);
}

void SpritesU::drawPlusMask(
    int16_t x, int16_t y, uint8_t const* image, uint16_t frame)
{
    uint8_t w, h;
    asm volatile(
        "lpm %[w], Z+\n"
        "lpm %[h], Z+\n"
        : [w] "=r" (w), [h] "=r" (h), [image] "+z" (image));
    SpritesU_DrawCommon(x, y, w, h, (uint24_t)image, frame, SPRITESU_MODE_PLUSMASK);
}

void SpritesU::drawPlusMask(
    int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t const* image)
{
    SpritesU_DrawCommon(x, y, w, h, (uint24_t)image, 0, SPRITESU_MODE_PLUSMASK);
}

void SpritesU::drawOverwriteFX(
    int16_t x, int16_t y, uint8_t w, uint8_t h, uint24_t image, uint16_t frame)
{
    SpritesU_DrawCommon(x, y, w, h, image, frame, SPRITESU_MODE_OVERWRITEFX);
}

void SpritesU::drawPlusMaskFX(
    int16_t x, int16_t y, uint8_t w, uint8_t h, uint24_t image, uint16_t frame)
{
    SpritesU_DrawCommon(x, y, w, h, image, frame, SPRITESU_MODE_PLUSMASKFX);
}

#endif
