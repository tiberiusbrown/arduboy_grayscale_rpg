#include "common.hpp"

#include <string.h>

static void test_lightsout()
{
    px = 736;
    py = 176;
}

void new_game()
{
    uint8_t sound = savefile.sound;
    uint8_t brightness = savefile.brightness;
    bool no_battery_alert = savefile.no_battery_alert;
    memset(&savefile, 0, sizeof(savefile));
    savefile.sound = sound;
    savefile.brightness = brightness;
    savefile.no_battery_alert = no_battery_alert;
    px = 170;
    py = 71;
    pdir = 0;
    pmoving = false;
    for(auto& f : story_flags) f = 0;
    memset(active_chunks, 0, sizeof(active_chunks));
    for(auto& ac : active_chunks)
        ac.cx = ac.cy = 255;
    chunks_are_running = false;

    for(uint8_t n = 0; n < 4; ++n)
    {
        party[n].battle.id = INVALID;
        for(auto& i : party[n].equipped_items)
            i = INVALID_ITEM;
    }

    nparty = 1;
    party[0].battle.id = 0;
    party[0].battle.hp = party_mhp(0);

    test_lightsout();
}

void initialize()
{
    nframe = 0;
    rand_seed = 0xcafe;
    platform_audio_init();
    change_state(STATE_TITLE);
}
