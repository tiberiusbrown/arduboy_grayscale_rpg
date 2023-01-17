#include "common.hpp"

#include <string.h>

#include "generated/locations.hpp"

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
    for(int i = 0; i < 4; ++i)
    {
        party[i].battle.id = i;
        party[i].battle.hp = party_mhp(i);
    }
    nparty = 4;

    LOC_home_outside();
    story_flag_set(SFLAG_story_charlie_job);
    story_flag_set(SFLAG_tip_objective);
    story_flag_set(SFLAG_charlie_raptor);
    story_flag_set(SFLAG_story_got_fish);
    story_flag_set(SFLAG_story_fish_delivered);
    px = 500;
    py = 1300;
    savefile.chunk_regs[8] = 3;
    savefile.chunk_regs[9] = 3;

    story_flag_set(SFLAG_ITEM_Barbarian_s_Helm);
    story_flag_set(SFLAG_ITEM_Boxing_Gloves);
#endif

#if 0
    LOC_dismas_house_outside();
    story_flag_set(SFLAG_ITEM_Barbarian_s_Helm);
    story_flag_set(SFLAG_ITEM_Barbarian_s_Footwraps);
    story_flag_set(SFLAG_ITEM_Barbarian_s_Axe);
    story_flag_set(SFLAG_ITEM_Spiked_Shield);
    story_flag_set(SFLAG_ITEM_Brawler_s_Ring);
    savefile.chunk_regs[8] = 3;
#endif

    //LOC_puzwait_outside();

}

void initialize()
{
    nframe = 0;
    rand_seed = 0xcafe;
#if !TITLE_ONLY_DEMO
    platform_audio_init();
#endif
    change_state(STATE_TITLE);
}
