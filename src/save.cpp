#include "common.hpp"

#include <string.h>

#ifdef ARDUINO
#include <EEPROM.h>
#endif

uint8_t const IDENTIFIER[8] PROGMEM =
{
    '_', 'R', 'o', 't', 'A', '_',
    uint8_t(VERSION >> 8),
    uint8_t(VERSION >> 0),
};

enum { SS_IDLE, SS_ERASE, SS_PROGRAM, SS_DONE };
//static uint8_t save_stage = SS_IDLE;
//static uint8_t save_page = 0;
//static uint8_t save_sound;
//static bool save_sound_resume;
static auto& save_stage = sdata.pause.save_stage;
static auto& save_page = sdata.pause.save_page;
static auto& save_sound = sdata.pause.save_sound;
static auto& save_sound_resume = sdata.pause.save_sound_resume;

void save_begin()
{
    save_sound_resume = platform_audio_song_playing();
    save_sound = savefile.settings.sound;
    savefile.settings.sound = 0;
    platform_audio_pause_song();
    platform_audio_stop_sfx();
    platform_fx_erase_save_sector();
    save_stage = SS_ERASE;
    save_page = 0;
}

bool is_saving()
{
    return save_stage != SS_IDLE;
}

bool save_done()
{
    if(platform_fx_busy()) return false;
    if(save_stage == SS_DONE)
    {
        save_stage = SS_IDLE;
        savefile.settings.sound = save_sound;
        if(save_sound_resume)
            platform_audio_resume_song();
        return true;
    }
    if(save_stage == SS_ERASE)
    {
        save_stage = SS_PROGRAM;
        save_page = 0;
        uint8_t const* p = &IDENTIFIER[0];
        for(uint8_t i = 0; i < 8; ++i)
            savefile.identifier[i] = pgm_read_byte_inc(p);
        savefile.loaded = true;
        savefile.checksum = compute_checksum();
        return false;
    }
    constexpr uint8_t pages = (sizeof(savefile) + 255) / 256;
    if(save_page >= pages)
    {
#ifdef ARDUINO
        Arduboy2Audio::saveOnOff();
#endif
        save_stage = SS_DONE;
        return false; // wait for next frame
    }
    uint8_t const* p = (uint8_t const*)&savefile;
    p += (256 * save_page);
    size_t n = sizeof(savefile) - (256 * save_page);
    if(n >= 256) n = 256;
    platform_fx_write_save_page(save_page, p, n);
    ++save_page;
    return false;
}

void load(bool first)
{
    auto old_settings = savefile.settings;
    platform_fx_read_save_bytes(0, &savefile, sizeof(savefile));
    bool id = true;
    for(uint8_t i = 0; i < 8; ++i)
        if(savefile.identifier[i] != pgm_read_byte(&IDENTIFIER[i]))
            id = false;
    if(!id || compute_checksum() != savefile.checksum)
    {
        new_game();
        savefile.settings.no_battery_alert = false;
        savefile.settings.sound = 3;
        savefile.settings.brightness = 3;
        savefile.settings.game_speed = 3;
    }

    if(first)
    {
#ifdef ARDUINO
#if !TITLE_ONLY_DEMO
        Arduboy2Audio::begin();
#endif
        if(!Arduboy2Audio::enabled())
            savefile.settings.sound = 0;
#endif
    }
    else
    {
        savefile.settings = old_settings;
    }
    platform_audio_from_savefile();
}

uint16_t compute_checksum()
{
    // CRC16
    uint8_t x;
    uint16_t crc = 0xffff;
    for(size_t i = 2; i < sizeof(savefile); ++i)
    {
        x = (crc >> 8) ^ ((uint8_t const*)&savefile)[i];
        x ^= div16(x);
        crc = (crc << 8) ^
            (uint16_t(x) << 12) ^
            (uint16_t(x) << 5) ^
            (uint16_t(x) << 0);
    }
    return crc;
}
