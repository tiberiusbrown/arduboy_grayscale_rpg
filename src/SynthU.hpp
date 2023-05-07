#pragma once

#ifndef SYNTHU_UPDATE_EVERY_N_FRAMES
#define SYNTHU_UPDATE_EVERY_N_FRAMES 1
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

#ifndef SYNTHU_NOISE_CHANNEL_HZ
#define SYNTHU_NOISE_CHANNEL_HZ 0
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

constexpr uint16_t OCR_MIN = 1024;
constexpr uint16_t OCR_MAX = (65535 - OCR_MIN) >> ADV_SHIFT;

#if SYNTHU_NOISE_CHANNEL_HZ
constexpr uint16_t NOISE_PERIOD = 16000000ul / (1 << ADV_SHIFT) / SYNTHU_NOISE_CHANNEL_HZ;
#endif

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

static volatile tick_t g_tick;
static volatile channel_t g_channels[SYNTHU_NUM_CHANNELS];
#if SYNTHU_ENABLE_VOLUME
static volatile uint8_t g_volume;
#endif
static volatile bool g_playing;
static uint8_t g_tick_frame;
static volatile uint24_t g_buffer_addr;
static volatile uint8_t g_phase_adv;
static volatile int16_t g_tbase;

template<class T>
static uint8_t ld_u8_inc(T*& ptr)
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
static uint16_t ld_u16_inc(T*& ptr)
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

static void tick()
{    
    {
        uint8_t reps = g_tick.reps;
        if(reps == 0) return;
        reps -= 1;
        g_tick.reps = reps;
        if(reps != 0) return;
    }
    
    g_buffer_addr += sizeof(tick_t);
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
    synthu_detail::g_tick.reps = 0;
    synthu_detail::g_playing = true;
    update();
    OCR3A = synthu_detail::OCR_MIN;
    TCNT3 = 0;
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
    if(!p) return false;
    uint8_t f = synthu_detail::g_tick_frame;
    if(++f < SYNTHU_UPDATE_EVERY_N_FRAMES)
    {
        synthu_detail::g_tick_frame = f;
        return true;
    }
    synthu_detail::g_tick_frame = 0;
    if(p && synthu_detail::g_tick.reps == 0)
    {
        FX::readDataBytes(
            synthu_detail::g_buffer_addr,
            (uint8_t*)&synthu_detail::g_tick,
            sizeof(synthu_detail::g_tick));
        //synthu_detail::g_tick.cmds[0] = { 100, 3405 };
        //synthu_detail::g_tick.cmds[1] = { 75, 5102 };
        //synthu_detail::g_tick.reps = 1;
        
        auto* bptr = synthu_detail::g_tick.cmds;
        int16_t t = 0;
        for(uint8_t i = 0; i < SYNTHU_NUM_CHANNELS; ++i)
        {
            uint8_t vol = synthu_detail::ld_u8_inc(bptr);
            uint16_t period = synthu_detail::ld_u16_inc(bptr);
            if(period == 0) stop();
    #ifdef ARDUINO_ARCH_AVR
            asm volatile(R"ASM(
                    lsr %[vol]
                    sub %A[t], %[vol]
                    sbc %B[t], __zero_reg__
                )ASM"
                : [vol] "+&r" (vol)
                , [t]   "+&r" (t)
                );
    #else
            vol >>= 1;
            t -= vol;
    #endif
        }
        synthu_detail::g_tbase = t;
    }
    synthu_detail::tick();
    return true;
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
    int16_t t = synthu_detail::g_tbase;
    auto const* cmd = synthu_detail::g_tick.cmds;
    auto const* channel = synthu_detail::g_channels;
    for(uint8_t i = 0; i < SYNTHU_NUM_CHANNELS; ++i)
    {
        uint8_t vol = synthu_detail::ld_u8_inc(cmd);
        uint16_t period = synthu_detail::ld_u16_inc(cmd);
        if(vol == 0) { channel += 1; continue; }
        
        uint16_t pha = channel->pha;
        pha += adv;
        while(pha >= period)
            pha -= period;
        uint16_t half_period = period / 2;
        if(pha < half_period)
        {
            t += vol;
            period = half_period;
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
}

#endif
