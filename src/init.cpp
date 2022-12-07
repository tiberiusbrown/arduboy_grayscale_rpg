#include "common.hpp"

#include <string.h>

void new_game()
{
    px = 100;
    py = 135;
    pdir = 0;
    pmoving = false;
    for(auto& f : story_flags) f = 0;
    memset(active_chunks, 0, sizeof(active_chunks));
    for(auto& ac : active_chunks)
        ac.cx = ac.cy = 255;
    chunks_are_running = false;

    nparty = 4;
    for(uint8_t i = 0; i < nparty; ++i)
    {
        party[i].battle.id = i;
        party[i].battle.hp = party_mhp(i);
    }

    story_flag_set(SFLAG_ITEM_Cloth_Tunic);
    story_flag_set(SFLAG_ITEM_Leather_Tunic);
}

void initialize()
{
    nframe = 0;
    rand_seed = 0xcafe;
    platform_audio_init();
    change_state(STATE_TITLE);
}
