#pragma once

#ifndef SYNTHU_TICKS_HZ
#define SYNTHU_TICKS_HZ 48
#endif

#ifndef SYNTHU_MIN_UPDATE_HZ
#define SYNTHU_MIN_UPDATE_HZ 48
#endif

#ifndef SYNTHU_NUM_CHANNELS
#define SYNTHU_NUM_CHANNELS 4
#endif

#ifndef SYNTHU_AUDIO_ENABLED_FUNC
#define SYNTHU_AUDIO_ENABLED_FUNC Arduboy2Audio::enabled
#endif

#ifndef SYNTHU_ENABLE_VOLUME
#define SYNTHU_ENABLE_VOLUME 1
#endif

#ifndef SYNTHU_ENABLE_CLIP
#define SYNTHU_ENABLE_CLIP 1
#endif

#include <Arduboy2Audio.h>
#include <ArduboyFX.h>
#include <string.h>

struct SynthU
{
    static void setup();
    
    static void playSong(uint24_t song);
    static bool playing();
    static void stop();
    static void resume();

    static uint8_t volume();
    static void setVolume(uint8_t vol);
    
    // fill buffer from FX chip
    // test for audio enabled
    // return true if playing
    static bool update();
};

#ifdef SYNTHU_IMPLEMENTATION

namespace synthu_detail
{

constexpr uint8_t ADV_SHIFT = 2;
    
constexpr uint8_t NUM_BUFFER_TICKS =
    (SYNTHU_TICKS_HZ + SYNTHU_MIN_UPDATE_HZ - 1) / SYNTHU_MIN_UPDATE_HZ;

constexpr uint24_t TICK_PERIOD = uint24_t((16000000ull >> ADV_SHIFT) / SYNTHU_TICKS_HZ);

constexpr uint8_t FLAG_VALID = 0x01;

constexpr uint16_t OCR_MIN = 1024;
constexpr uint16_t OCR_MAX = (65535 - OCR_MIN) >> ADV_SHIFT;

struct command_t
{
    uint8_t  vol;
    uint16_t period;
};

struct tick_t
{
    command_t cmds[SYNTHU_NUM_CHANNELS];
    uint8_t reps;
};

struct channel_t
{
    uint16_t pha;
};

static volatile tick_t g_buffers[NUM_BUFFER_TICKS];
static volatile channel_t g_channels[SYNTHU_NUM_CHANNELS];
#if SYNTHU_ENABLE_VOLUME
static volatile uint8_t g_volume;
#endif
static volatile bool g_playing;
static volatile uint24_t g_tick_pha;
static volatile uint24_t g_buffer_addr;
static volatile uint8_t g_phase_adv;

template<class T>
static uint8_t ld_u8_inc(T const*& ptr)
{
    uint8_t r;
#if ARDUINO_ARCH_AVR
    asm volatile(
        "ld %[r], %a[ptr]+\n"
        : [r] "=&r" (r), [ptr] "+&e" (ptr));
#else
    r = *(uint8_t const*)ptr;
    ptr = (uint8_t const*)ptr + 1;
#endif
    return r;
}

template<class T>
static uint16_t ld_u16_inc(T const*& ptr)
{
    uint16_t r;
#if ARDUINO_ARCH_AVR
    asm volatile(
        "ld %A[r], %a[ptr]+\n"
        "ld %B[r], %a[ptr]+\n"
        : [r] "=&r" (r), [ptr] "+&e" (ptr));
#else
    r = *(uint16_t const*)ptr;
    ptr = (uint16_t const*)ptr + 1;
#endif
    return r;
}

template<class T>
static void st_u16_inc(T*& ptr, uint16_t x)
{
#if ARDUINO_ARCH_AVR
    asm volatile(
        "st %a[ptr]+, %A[x]\n"
        "st %a[ptr]+, %B[x]\n"
        : [ptr] "+&e" (ptr) : [x] "r" (x));
#else
    *(uint16_t const*)ptr = x;
    ptr = (uint16_t const*)ptr + 1;
#endif
}

static void tick(uint16_t adv)
{
    uint8_t sreg;
    command_t* bptr;
    channel_t* cptr;
    
    uint24_t pha = g_tick_pha;
    pha += adv;
    if(pha < TICK_PERIOD)
    {
        g_tick_pha = pha;
        return;
    }
    pha -= TICK_PERIOD;
    g_tick_pha = pha;
    
    {
        uint8_t reps = g_buffers[0].reps;
        if(reps == 0) return;
        reps -= 1;
        g_buffers[0].reps = reps;
        if(reps != 0) return;
    }
    
    g_buffer_addr += sizeof(tick_t);
    
    bptr = g_buffers[0].cmds;
    cptr = g_channels;
    for(uint8_t i = 0; i < SYNTHU_NUM_CHANNELS; ++i, ++bptr)
    {
        uint16_t pha = cptr->pha;
        uint16_t period = bptr->period;
        if(period == 0) SynthU::stop();
        if(pha >= period) pha = 0;
        st_u16_inc(cptr, pha);
    }
    
    // rotate buffer if needed
    if(NUM_BUFFER_TICKS > 1)
    {
        sei();
        
        memmove(
            &g_buffers[0],
            &g_buffers[1],
            sizeof(tick_t) * (NUM_BUFFER_TICKS - 1));
    }
}

static inline uint8_t hi(uint16_t x)
{
    return uint8_t(x >> 8);
}

}

uint8_t SynthU::volume()
{
#if SYNTHU_ENABLE_VOLUME
    return synthu_detail::g_volume;
#else
    return 8;
#endif
}

void SynthU::setVolume(uint8_t vol)
{
#if SYNTHU_ENABLE_VOLUME
    if(vol > 16) vol = 16;
    synthu_detail::g_volume = vol;
#endif
}

void SynthU::setup()
{
    stop();
    
    PLLFRQ = 0x56; // 96 MHz
    TCCR4A = 0x42; // PWM mode
    TCCR4D = 0x01; // dual slope PWM
    TCCR4E = 0x40; // enhanced mode
    TC4H   = 0x00;
    OCR4C  = 0x7f; // 8-bit PWM precision
    
    TCCR3A = 0x00;
    TCCR3B = 0x09; // CTC 16 MHz
}

void SynthU::stop()
{
    TIMSK3 = 0x00;
    TCCR4B = 0x00;
    synthu_detail::g_playing = false;
}

void SynthU::resume()
{
    synthu_detail::g_buffers[0].reps = 0;
    synthu_detail::g_playing = true;
    update();
    OCR3A = synthu_detail::OCR_MIN;
    TCCR4B = 0x01;
    TIMSK3 = 0x02;
}

void SynthU::playSong(uint24_t song)
{
    stop();
    synthu_detail::g_buffer_addr = song;
    resume();
}

bool SynthU::playing()
{
    return synthu_detail::g_playing;
}

bool SynthU::update()
{
    bool p = playing();
    if(p && synthu_detail::g_buffers[0].reps == 0)
    {
        uint8_t sreg = SREG;
        cli();
        FX::readDataBytes(
            synthu_detail::g_buffer_addr,
            (uint8_t*)synthu_detail::g_buffers,
            sizeof(synthu_detail::g_buffers));
        SREG = sreg;
    }
    return p;
}

static inline uint16_t tmin(uint16_t a, uint16_t b)
{
    return a < b ? a : b;
}

ISR(TIMER3_COMPA_vect)
{
    if(!synthu_detail::g_playing || !SYNTHU_AUDIO_ENABLED_FUNC())
    {
        OCR3A = 65535;
        return;
    }
    uint16_t ocr = synthu_detail::OCR_MAX;
    uint16_t adv = OCR3A;
    adv >>= synthu_detail::ADV_SHIFT;
    int16_t t = 0;
    auto const* cmd = synthu_detail::g_buffers;
    auto const* channel = synthu_detail::g_channels;
    for(uint8_t i = 0; i < SYNTHU_NUM_CHANNELS; ++i)
    {
        uint8_t vol = synthu_detail::ld_u8_inc(cmd);
        uint16_t period = synthu_detail::ld_u16_inc(cmd);
        if(vol == 0) { channel += 1; continue; }
        
        uint16_t pha = channel->pha;
        uint16_t half_period = period / 2;
        pha += adv;
        if(pha >= period)
            pha -= period;
        if(pha < half_period)
        {
            t += vol;
            period = half_period;
        }
        else 
        {
            t -= vol;
        }
        st_u16_inc(channel, pha);
        period -= pha;
        ocr = tmin(ocr, period);
    }
    ocr <<= synthu_detail::ADV_SHIFT;
    
    if(ocr <= synthu_detail::OCR_MIN)
        ocr = synthu_detail::OCR_MIN;
    else
    {
        ocr -= 1;
        ocr &= ~(synthu_detail::OCR_MIN - 1);
        ocr += synthu_detail::OCR_MIN;
    }

    OCR3A = ocr;
#if SYNTHU_ENABLE_VOLUME
    t *= synthu_detail::g_volume;
#if ARDUINO_ARCH_AVR
    asm volatile(R"ASM(
        asr %B[t]
        ror %A[t]
        asr %B[t]
        ror %A[t]
        asr %B[t]
        ror %A[t]
        asr %B[t]
        ror %A[t]
        )ASM"
        : [t] "+&r" (t)
        );
#else
    t >>= 4;
#endif
#else
    t >>= 1;
#endif
    uint8_t tc = uint8_t(t);
#if SYNTHU_ENABLE_CLIP
    if(t > +120) tc = uint8_t(+120);
    if(t < -120) tc = uint8_t(-120);
#endif
    tc += 128;
    TC4H  = 0;
    OCR4A = tc;
    synthu_detail::tick(adv);
}

#endif
