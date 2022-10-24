#include "common.hpp"

#include <string.h>

void initialize()
{
    nframe = 0;
    px = 128;
    py = 112;
    pdir = 0;
    pmoving = false;
    chunks_are_running = false;
    change_state(STATE_MAP);
}
