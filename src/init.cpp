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
    //story_flag_set(SFLAG_ITEM_Barbarian_s_Helm);
    //story_flag_set(SFLAG_ITEM_Barbarian_s_Footwraps);
    //story_flag_set(SFLAG_ITEM_Barbarian_s_Axe);
    //story_flag_set(SFLAG_ITEM_Spiked_Shield);
    //story_flag_set(SFLAG_ITEM_Brawler_s_Ring);
    //story_flag_set(SFLAG_ITEM_Boxing_Gloves);
    consumables[CIT_Healing_Salve] = 3;
    consumables[CIT_Ardu_s_Fury] = 3;
    consumables[CIT_Ardu_s_Frenzy] = 3;
    consumables[CIT_Potion_of_Attack] = 3;
#endif

    //LOC_test();
    //LOC_first_cave_outside();
    //LOC_home_outside();
    //story_flag_set(SFLAG_story_fish_delivered);

    //LOC_first_cave_outside();
    //story_flag_set(SFLAG_story_fish_delivered);
    //story_flag_set(SFLAG_story_met_catherine);
    //story_flag_set(SFLAG_party_catherine);

    //LOC_puzspikefollow_inside();
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
        savefile.settings.brightness = 3;
        t = STATE_BADFX;
    }
    change_state(t);
}
