#include "common.hpp"

#include <string.h>

#include "generated/locations.hpp"
#include "generated/fxdata.h"

void new_game()
{
    auto old_settings = savefile.settings;
    memset(&savefile, 0, sizeof(savefile));
    savefile.settings = old_settings;
    LOC_new_game();
    pdir = 0;
    pmoving = false;
    for(auto& f : story_flags) f = 0;
    memset(active_chunks, 0, sizeof(active_chunks));
    for(auto& ac : active_chunks)
        ac.cx = ac.cy = 255;
    chunks_are_running = false;
    savefile.music_type = music::peaceful;

    for(uint8_t n = 0; n < 4; ++n)
    {
        party[n].battle.id = INVALID;
        for(auto& i : party[n].equipped_items)
            i = INVALID_ITEM;
    }

    nparty = 1;
    party[0].battle.id = 0;
    party[0].battle.hp = party_mhp(0);

#if 0
    savefile.chunk_regs[8] = 2;
    savefile.chunk_regs[9] = 2;
    savefile.chunk_regs[10] = 2;
    savefile.chunk_regs[11] = 2;
    savefile.chunk_regs[12] = 2;
    savefile.chunk_regs[13] = 2;
    LOC_first_cave_exit();
    story_flag_set(SFLAG_first_cave_guard2);
    //px = 1336, py = 87;
#endif
}

static bool check_identifier(uint24_t addr)
{
    uint8_t b[8];
    platform_fx_read_data_bytes(addr, b, 8);
    uint8_t const* ptr0 = b;
    uint8_t const* ptr1 = IDENTIFIER;
    for(uint8_t i = 0; i < 8; ++i)
        if(deref_inc(ptr0) != pgm_read_byte_inc(ptr1))
            return false;
    return true;
}

void initialize()
{
    nframe = 0;
    rand_seed = 0xcafe;
#if !TITLE_ONLY_DEMO
    platform_audio_init();
#endif
    uint8_t t = STATE_TITLE;
    if(!check_identifier(FX_IDENTIFIER_A) || !check_identifier(FX_IDENTIFIER_B))
    {
        //savefile.settings.brightness = 3;
        t = STATE_BADFX;
    }
    change_state(t);
    
#if DEBUG_PARTY_MENU   
    change_state(STATE_PAUSE);
    sdata.pause.state = OS_PARTY;
#endif
}
