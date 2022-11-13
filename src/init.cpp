#include "common.hpp"

#include <string.h>

void new_game()
{
    px = 100;
    py = 135;
    pdir = 0;
    pmoving = false;
    memset(story_flags, 0, sizeof(story_flags));
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
    change_state(STATE_TITLE);
}
