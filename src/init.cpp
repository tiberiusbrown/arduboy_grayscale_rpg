#include "common.hpp"

#include <string.h>

void initialize()
{
    nframe = 0;
    chunks_are_running = false;

    px = 120;
    py = 135;
    pdir = 0;
    pmoving = false;
    change_state(STATE_MAP);
    for(auto& f : story_flags)
        f = 0;
    for(auto& ac : active_chunks)
    {
        memset(&ac, 0, sizeof(ac));
        ac.cx = ac.cy = 255;
    }

    nparty = 1;
    party[0].id = 0;
    party[0].hp = 10;
    party[1].id = 255;
    party[2].id = 255;
    party[3].id = 255;
}
