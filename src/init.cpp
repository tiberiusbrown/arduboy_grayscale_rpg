#include "common.hpp"

#include <string.h>

void initialize()
{
    nframe = 0;
    chunks_are_running = false;

    px = 128;
    py = 112;
    pdir = 0;
    pmoving = false;
    change_state(STATE_MAP);
    for(auto& f : story_flags)
        f = 0;
}
