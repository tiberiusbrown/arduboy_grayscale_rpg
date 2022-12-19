#include "common.hpp"

#include <string.h>

#include "generated/locations.hpp"

void new_game()
{
    auto old_settings = savefile.settings;
    memset(&savefile, 0, sizeof(savefile));
    savefile.settings = old_settings;
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

    LOC_home_outside();
    story_flag_set(SFLAG_story_charlie_job);
    story_flag_set(SFLAG_tip_objective);
    //story_flag_set(SFLAG_charlie_raptor);
    //story_flag_set(SFLAG_story_got_fish);
}

void initialize()
{
    nframe = 0;
    rand_seed = 0xcafe;
    platform_audio_init();
    change_state(STATE_TITLE);
}
