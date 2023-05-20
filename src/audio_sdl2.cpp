#ifndef ARDUINO

#include "common.hpp"

#include <SDL.h>

#include <stdlib.h>

constexpr int SAMPLES = 1024;
constexpr int FREQ = 16000;
constexpr float SAMPLE_TIME = 1.f / FREQ;

constexpr int BPS = 14;
constexpr float BEAT_TIME = 1.f / BPS;
constexpr int BEAT_PERIOD = int(BEAT_TIME / SAMPLE_TIME + 0.5f);

static int sfx_channel;
static SDL_AudioSpec spec;
static SDL_AudioDeviceID device;

struct audio_channel
{
    uint8_t const* ptr;
    uint16_t beat_period;     // 1/BPS in samples
    uint16_t beat_progress;
    uint16_t note_period;     // 1/freq in samples*256
    uint16_t note_progress;   // in samples*256
    uint16_t delay;           // in beats
    uint8_t volume;           // 0 to 127
    int8_t volume_slide;      // add to volume each beat
    uint8_t vol_slide_retrig; // 0 for no retrig, nonzero for reset volume
    bool noise;               // whether channel is noise
    bool paused;
};

static audio_channel channels[NUM_SCORE_CHANNELS + 1];

static uint16_t calc_note_period(uint8_t note)
{
    note &= 63;

    // 0: note off
    if(note == 0) return 0;

    // 1:C2 to 63:D7
    float const B1 = 61.74f;
    float freq = B1 * powf(2.f, float(note) * (1.f / 12));
    float period = 1.f / freq;
    float samples = period * 256 / SAMPLE_TIME;
    int isamples = int(roundf(samples));

    return uint16_t(isamples);
}

// produce one sample
static int8_t sample_channel(audio_channel& c)
{
    return 0;
    if(!c.ptr) return 0;
    if(c.paused) return 0;

    while(c.delay == 0)
    {
        uint8_t cmd = *c.ptr++;
        if(cmd < 64)
        {
            if(c.vol_slide_retrig != 0)
                c.volume = c.vol_slide_retrig;
            c.note_period = calc_note_period(cmd);
        }
        else if(cmd < 96)
            c.delay = cmd - 64 + 1;
        else if(cmd == 97)
            return c.ptr = nullptr, 0;
        else if(cmd == 116)
            c.volume = *c.ptr++;
        else if(cmd == 117)
            c.noise = (*c.ptr++ != 0);
        else if(cmd == 133)
        {
            c.delay = uint16_t(*c.ptr++) + 1;
        }
        else if(cmd == 145)
        {
            c.ptr++;
            c.volume_slide = int8_t(*c.ptr++);
        }
        else if(cmd == 149)
        {
            c.delay = uint16_t(*c.ptr++) << 8;
            c.delay += *c.ptr++;
        }
        else if(cmd == 161)
        {
            c.ptr++;
            c.volume_slide = int8_t(*c.ptr++);
            c.vol_slide_retrig = ((*c.ptr++) & 0x40) ? c.volume : 0;
        }
        else { MY_ASSERT(0); } // skip
    }

    int8_t r = 0;

    if(c.note_progress <= c.note_period / 2)
        r = -c.volume;
    else
        r = +c.volume;

    if(c.noise)
        r = (rand() & 1) ? -c.volume : +c.volume;

    
    if((c.note_progress += 256) >= c.note_period)
        c.note_progress -= c.note_period;

    if(++c.beat_progress >= c.beat_period)
    {
        c.beat_progress = 0;
        --c.delay;

        int v = c.volume + c.volume_slide;
        if(v < 0) v = 0;
        if(v > 127) v = 127;
        c.volume = (int8_t)v;
    }

    return r;
}

static void audio_callback(void* userdata, Uint8* stream, int len)
{
    for(int i = 0; i < len; ++i)
    {
        int8_t sample[NUM_SCORE_CHANNELS + 1];
        bool sfx = (channels[2].ptr != nullptr);
        for(int i = 0; i < NUM_SCORE_CHANNELS + 1; ++i)
            sample[i] = sample_channel(channels[i]);
        if(sfx) sample[sfx_channel] = 0;
        int s = 0;
        for(int i = 0; i < NUM_SCORE_CHANNELS + 1; ++i)
            s += sample[i];
        s /= 16;
        if(s < -127) s = -127;
        if(s > +127) s = +127;
        stream[i] = Uint8((int8_t)s);
    }
}

void platform_audio_init()
{
    spec.callback = audio_callback;
    spec.userdata = nullptr;
    spec.freq = FREQ;
    spec.format = AUDIO_S8;
    spec.channels = 1;
    spec.samples = SAMPLES;
    device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    SDL_PauseAudioDevice(device, 0);
}

void platform_audio_deinit()
{
    SDL_CloseAudioDevice(device);
}

extern uint8_t const FXDATA[];

void platform_audio_play_song(uint24_t song)
{
    if(!platform_audio_enabled() || savefile.settings.music == 0) return;
    SDL_LockAudioDevice(device);
    for(int i = 0; i < NUM_SCORE_CHANNELS; ++i)
    {
        memset(&channels[i], 0, sizeof(audio_channel));
        channels[i].beat_period = BEAT_PERIOD;
    }
    uint16_t buf[NUM_SCORE_CHANNELS];
    platform_fx_read_data_bytes(song, buf, NUM_SCORE_CHANNELS * 2);
    for(int i = 0; i < NUM_SCORE_CHANNELS; ++i)
        channels[i].ptr = &FXDATA[song + buf[i]];
    SDL_UnlockAudioDevice(device);
}

void platform_audio_play_sfx(uint24_t sfx, uint8_t channel)
{
    if(!platform_audio_enabled() || savefile.settings.sfx == 0) return;
    SDL_LockAudioDevice(device);
    memset(&channels[NUM_SCORE_CHANNELS], 0, sizeof(audio_channel));
    sfx_channel = channel;
    channels[NUM_SCORE_CHANNELS].beat_period = BEAT_PERIOD;
    channels[NUM_SCORE_CHANNELS].ptr = &FXDATA[sfx];
    SDL_UnlockAudioDevice(device);
}

void platform_audio_pause_song()
{
    SDL_LockAudioDevice(device);
    for(int i = 0; i < NUM_SCORE_CHANNELS; ++i)
        channels[i].paused = true;
    SDL_UnlockAudioDevice(device);
}

void platform_audio_resume_song()
{
    SDL_LockAudioDevice(device);
    for(int i = 0; i < NUM_SCORE_CHANNELS; ++i)
        channels[i].paused = false;
    SDL_UnlockAudioDevice(device);
}

void platform_audio_stop_sfx()
{
    channels[NUM_SCORE_CHANNELS].ptr = nullptr;
}

bool platform_audio_song_playing()
{
    for(int i = 0; i < NUM_SCORE_CHANNELS; ++i)
        if(!channels[i].paused && channels[i].ptr != nullptr) return true;
    return false;
}

bool platform_audio_sfx_playing()
{
    return channels[NUM_SCORE_CHANNELS].ptr != nullptr;
}

void platform_audio_on()
{
    SDL_PauseAudioDevice(device, 0);
}

void platform_audio_off()
{
    SDL_PauseAudioDevice(device, 1);
}

bool platform_audio_enabled()
{
    return SDL_GetAudioDeviceStatus(device) == SDL_AUDIO_PLAYING;
}

void platform_audio_from_savefile()
{
    // TODO
}

#endif
