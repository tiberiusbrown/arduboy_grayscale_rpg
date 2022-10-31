#include "common.hpp"

#include "generated/fxdata.h"

void update_battle()
{
    auto& d = sdata.battle;
    switch(d.phase) {
    case sdata_battle::PHASE_INTRO:
        if(d.frame == 8 && d.remove_enemy)
            active_chunks[d.enemy_chunk].enemy.active = false;
        if(d.frame == 33) {
            d.frame = 0;
            d.phase = sdata_battle::PHASE_BATTLE;
        } else ++d.frame;
        break;
    case sdata_battle::PHASE_BATTLE:
        if(btns_pressed & BTN_A) d.phase = sdata_battle::PHASE_OUTRO;
        break;
    case sdata_battle::PHASE_OUTRO:
        // resume state
        if(!(chunks_are_running && run_chunks())) change_state(STATE_MAP);
    default: break;
    }
}

static void draw_battle_background()
{
    static constexpr uint8_t TS[] PROGMEM = {162, 163, 178, 179};
    uint8_t t = 0x23;
    for(uint8_t r = 0; r < 4; ++r)
        for(uint8_t c = 0; c < 8; ++c, t ^= (t >> 3) ^ (t << 1))
            platform_fx_drawoverwrite(c * 16, r * 16, TILE_IMG,
                                      pgm_read_byte(&TS[t & 3]), 16, 16);
    platform_fillrect(33, 37, 14, 12, WHITE);
    platform_drawrect(35, 39, 10, 8, LIGHT_GRAY);
    platform_fillrect(81, 37, 14, 12, WHITE);
    platform_drawrect(83, 39, 10, 8, LIGHT_GRAY);
}

static void draw_battle_sprites()
{
    auto const& d = sdata.battle;
    static constexpr uint8_t PPOS[] PROGMEM = {
        16, 8, 16, 32, 0, 8, 0, 32,
    };
    static constexpr uint8_t EPOS[] PROGMEM = {
        0, 8, 0, 32, 16, 8, 16, 32,
    };
    draw_sprite_entry entries[8];
    uint8_t n = 0;

    // party
    {
        auto& e = entries[n++];
        e.addr = PLAYER_IMG;
        e.frame = 0;
        e.x = PPOS[0];
        e.y = PPOS[1] + 6;
    }

    // enemies
    for(uint8_t i = 0; i < 4; ++i) {
        uint8_t t = d.enemies[i];
        if(t == 255) continue;
        auto& e = entries[n++];
        e.addr = ENEMY_IMG;
        e.frame = t * 16;
        e.x = pgm_read_byte(&EPOS[i * 2 + 0]) + 96;
        e.y = pgm_read_byte(&EPOS[i * 2 + 1]) + 6;
    }

    sort_and_draw_sprites(entries, n);
}

void render_battle()
{
    auto const& d = sdata.battle;
    if(d.phase == sdata_battle::PHASE_INTRO) {
        int16_t x;
        if(d.frame <= 8) x = 128 - d.frame * 16;
        else if(d.frame <= 24) x = (8 - d.frame);
        else x = -16 - (d.frame - 24) * 16;
        if(d.frame <= 8) {
            draw_tiles();
            draw_sprites();
        } else {
            draw_battle_background();
            draw_battle_sprites();
        }
        platform_fx_drawoverwrite(-(x + 16), 0, BATTLE_BANNER_IMG, 0, 144, 24);
        platform_fx_drawoverwrite(x, 24, BATTLE_START_IMG, 0, 144, 16);
        platform_fx_drawoverwrite(-(x + 16), 40, BATTLE_BANNER_IMG, 0, 144, 24);
        return;
    }
    draw_battle_background();
    draw_battle_sprites();
}
