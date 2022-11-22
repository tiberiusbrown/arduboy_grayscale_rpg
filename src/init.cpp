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

    nparty = 2;
    party[0].battle.id = 0;
    party[0].battle.hp = 20;
    party[1].battle.id = 1;
    party[1].battle.hp = 10;
    party[2].battle.id = 255;
    party[3].battle.id = 255;
}

void initialize()
{
    nframe = 0;
    rand_seed = 0xcafe;
    platform_audio_init();
    change_state(STATE_TITLE);
}
