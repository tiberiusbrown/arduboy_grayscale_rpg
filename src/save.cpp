#include "common.hpp"

#include <string.h>

static char const IDENTIFIER[8] PROGMEM = "ROTArdu";

void update_save()
{

}

void render_save()
{

}

void load()
{
    platform_fx_read_save_bytes(0, &savefile, sizeof(savefile));
    bool id = true;
    for(uint8_t i = 0; i < 8; ++i)
        if(savefile.identifier[i] != (char)pgm_read_byte(&IDENTIFIER[i]))
            id = false;
    if(!id || compute_checksum() != savefile.checksum)
    {
        memset(&savefile, 0, sizeof(savefile));
        new_game();
        savefile.brightness = 3;
    }
}

uint16_t compute_checksum()
{
    // CRC16
    uint8_t x;
    uint16_t crc = 0xffff;
    for(uint16_t i = 2; i < sizeof(savefile); ++i)
    {
        x = (crc >> 8) ^ ((uint8_t*)&savefile)[i];
        x ^= x >> 4;
        crc = (crc << 8) ^
            (uint16_t(x) << 12) ^
            (uint16_t(x) << 5) ^
            (uint16_t(x) << 0);
    }
    return crc;
}
