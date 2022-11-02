#include "common.hpp"

#include "generated/fxdata.h"

void update_battle()
{
    auto& d = sdata.battle;
    if(d.menuy != d.menuy_target) d.menuy = (d.menuy + d.menuy_target) / 2;
    ++d.frame;
    switch(d.phase)
    {
    case sdata_battle::PHASE_INTRO:
        d.menuy = d.menuy_target = -51;
        if(d.frame == 8 && d.remove_enemy)
            active_chunks[d.enemy_chunk].enemy.active = false;
        if(d.frame == 33)
        {
            d.frame = 0;
            d.phase = sdata_battle::PHASE_MENU;
        }
        break;
    case sdata_battle::PHASE_MENU:
        d.menuy_target = 0;
        if(btns_pressed & BTN_A)
        {
            d.menuy_target = -51;
            d.phase = sdata_battle::PHASE_OUTRO;
            if(d.msel == 0)
            {
                d.prev_phase = sdata_battle::PHASE_MENU;
                d.next_phase = sdata_battle::PHASE_PATTACK;
                d.phase = sdata_battle::PHASE_ESEL;
            }
        }
        if(btns_pressed & BTN_DOWN && ++d.msel == 4) d.msel = 0;
        if(btns_pressed & BTN_UP && d.msel-- == 0) d.msel = 3;
        break;
    case sdata_battle::PHASE_ESEL:
        if(btns_pressed & (BTN_UP | BTN_DOWN)) d.esel ^= 1;
        if(btns_pressed & (BTN_LEFT | BTN_RIGHT)) d.esel ^= 2;
        if(btns_pressed & BTN_B) d.phase = d.prev_phase;
        if((btns_pressed & BTN_A) && d.enemies[d.esel].id != 255)
            d.phase = d.next_phase;
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
            draw_tile(c * 16, r * 16, pgm_read_byte(&TS[t & 3]));
    platform_fillrect(33, 37, 14, 12, WHITE);
    platform_drawrect(35, 39, 10, 8, LIGHT_GRAY);
    platform_fillrect(81, 37, 14, 12, WHITE);
    platform_drawrect(83, 39, 10, 8, LIGHT_GRAY);
}

static constexpr uint8_t PPOS[] PROGMEM = {
    16, 14, 16, 38, 0, 14, 0, 38,
};
static constexpr uint8_t EPOS[] PROGMEM = {
    96, 14, 96, 38, 112, 14, 112, 38,
};

static void draw_selection_arrow(uint8_t x, uint8_t y)
{
    auto const& d = sdata.battle;
    uint8_t f = (d.frame >> 1) & 7;
    f = (f < 4 ? f : 7 - f);
    platform_fx_drawplusmask(x + 4, y - 8 + f, BATTLE_ARROW_IMG, 0, 7, 8);
}

static void draw_battle_sprites()
{
    auto const& d = sdata.battle;
    static constexpr uint8_t PARTY_IMG[] PROGMEM = {
        2,
        2,
        2,
    };
    draw_sprite_entry entries[8];
    uint8_t n = 0;

    // party
    for(uint8_t i = 0; i < nparty; ++i)
    {
        auto& e = entries[n++];
        if(party[i].id == 0)
        {
            e.addr = PLAYER_IMG;
            e.frame = 0;
        }
        else {
            e.addr = ENEMY_IMG;
            e.frame = pgm_read_byte(&PARTY_IMG[party[i].id - 1]) * 16;
        }
        e.x = pgm_read_byte(&PPOS[i * 2 + 0]);
        e.y = pgm_read_byte(&PPOS[i * 2 + 1]);
    }

    // enemies
    for(uint8_t i = 0; i < 4; ++i)
    {
        uint8_t t = d.enemies[i].id;
        if(t == 255) continue;
        auto& e = entries[n++];
        e.addr = ENEMY_IMG;
        e.frame = t * 16;
        e.x = pgm_read_byte(&EPOS[i * 2 + 0]);
        e.y = pgm_read_byte(&EPOS[i * 2 + 1]);
    }

    sort_and_draw_sprites(entries, n);
}

void render_battle()
{
    auto const& d = sdata.battle;
    if(d.phase == sdata_battle::PHASE_INTRO)
    {
        int16_t x;
        if(d.frame <= 8) x = 128 - d.frame * 16;
        else if(d.frame <= 24) x = (8 - d.frame);
        else x = -16 - (d.frame - 24) * 16;
        if(d.frame <= 8)
        {
            draw_tiles();
            draw_sprites();
        }
        else {
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
    if(d.menuy > -48)
    {
        platform_fx_drawplusmask(51, d.menuy, BATTLE_MENU_CHAIN_IMG, 0, 3, 16);
        platform_fx_drawplusmask(74, d.menuy, BATTLE_MENU_CHAIN_IMG, 0, 3, 16);
        platform_fx_drawoverwrite(48, d.menuy + 11, BATTLE_MENU_IMG, 0, 32, 40);
        draw_text_prog(50, d.menuy + 12 + 10 * d.msel, PSTR("\x7f"));
    }
    if(d.phase == sdata_battle::PHASE_ESEL)
    {
        uint8_t x = pgm_read_byte(&EPOS[d.esel * 2 + 0]);
        uint8_t y = pgm_read_byte(&EPOS[d.esel * 2 + 1]);
        draw_selection_arrow(x, y);
    }
}
