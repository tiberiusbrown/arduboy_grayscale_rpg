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

void save()
{
    uint8_t const* p = &IDENTIFIER[0];
    for(uint8_t i = 0; i < 8; ++i)
        savefile.identifier[i] = pgm_read_byte_inc(p);
    savefile.loaded = true;
    savefile.checksum = compute_checksum();
    platform_save_game_state(&savefile, sizeof(savefile));
#ifdef ARDUINO
    Arduboy2Audio::saveOnOff();
#endif
}

void load(bool first)
{
    auto old_settings = savefile.settings;
    platform_load_game_state(&savefile, sizeof(savefile));
    //platform_fx_read_save_bytes(0, &savefile, sizeof(savefile));
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
    
    set_music_from_position();
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
