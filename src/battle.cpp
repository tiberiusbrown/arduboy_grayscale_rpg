#include "common.hpp"

#include "generated/fxdata.h"

constexpr uint8_t SPRITE_MOVE_SPEED = 2;

static constexpr uint8_t PPOS[] PROGMEM = {
    16, 14, 16, 38, 0, 14, 0, 38,
};
static constexpr uint8_t EPOS[] PROGMEM = {
    96, 14, 96, 38, 112, 14, 112, 38,
};

static uint8_t get_party_speed(uint8_t id)
{
    return 10;
};

static uint8_t get_enemy_speed(uint8_t id)
{
    return pgm_read_byte(&ENEMY_INFO[id].speed);
}

static void init_attack_order() {
    auto& d = sdata.battle;
    uint8_t speeds[8];
    uint8_t n = 0;
    for(uint8_t i = 0; i < 4; ++i)
    {
        if(party[i].id != 255)
        {
            speeds[n] = get_party_speed(party[i].id);
            d.attack_order[n++] = i;
        }
        if(d.enemies[i].id != 255)
        {
            speeds[n] = get_enemy_speed(d.enemies[i].id);
            d.attack_order[n++] = i + 4;
        }
    }
    // sort ascending by speed
    for(uint8_t i = 1; i < n; ++i)
    {
        for(uint8_t j = i; j > 0 && speeds[j - 1] < speeds[j]; --j)
        {
            tswap(speeds[j], speeds[j - 1]);
            tswap(d.attack_order[j], d.attack_order[j - 1]);
        }
    }
    d.num_attackers = n;
}

static void init_sprites()
{
    auto& d = sdata.battle;
    for(uint8_t i = 0; i < nparty; ++i)
    {
        auto& s = d.sprites[i];
        s.active = true;
        s.x = pgm_read_byte(&PPOS[i * 2 + 0]);
        s.y = pgm_read_byte(&PPOS[i * 2 + 1]);
        s.addr = SPRITES_IMG;
        s.frame_base = party[i].id * 16;
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        uint8_t id = d.enemies[i].id;
        if(id == 255) continue;
        auto& s = d.sprites[i + 4];
        s.active = true;
        s.x = pgm_read_byte(&EPOS[i * 2 + 0]);
        s.y = pgm_read_byte(&EPOS[i * 2 + 1]);
        s.addr = SPRITES_IMG;
        s.frame_base = id * 16;
    }
    for(auto& s : d.sprites)
    {
        s.bx = s.tx = s.x;
        s.by = s.ty = s.y;
    }
}

static void battle_enemy_attack(uint8_t i)
{
    auto& d = sdata.battle;
    auto& e = d.enemies[i];

    // TODO
    d.defender_id = 0;
    if(d.pdef != 255)
        d.defender_id = d.pdef;
    d.phase = BPHASE_ATTACK1;
}

static void battle_next_turn()
{
    auto& d = sdata.battle;
    if(++d.current_attacker >= d.num_attackers)
        d.current_attacker = 0;
    uint8_t id = d.attacker_id = d.attack_order[d.current_attacker];
    if(id < 4)
    {
        d.phase = BPHASE_MENU;
    }
    else
    {
        battle_enemy_attack(id - 4);
    }
    auto& s = d.sprites[d.current_attacker];
    s.tx = s.bx;
    s.ty = s.by;
    d.next_phase = d.phase;
    d.phase = BPHASE_SPRITES;
}

static void update_battle_sprites()
{
    auto& d = sdata.battle;
    uint8_t nf = (d.frame >> 2) & 3;
    d.sprites_done = true;
    for(auto& s : d.sprites)
    {
        if(!s.active) continue;
        if(s.x == s.tx && s.y == s.ty)
        {
            s.frame_dir = 0;
            continue;
        }
        d.sprites_done = false;

        if(s.ty < s.y)
        {
            s.frame_dir = 8;
            s.y -= SPRITE_MOVE_SPEED;
            if(s.y < s.ty) s.y = s.ty;
        }
        else if(s.ty > s.y)
        {
            s.frame_dir = 0;
            s.y += SPRITE_MOVE_SPEED;
            if(s.y > s.ty) s.y = s.ty;
        }
        if(s.tx < s.x)
        {
            s.frame_dir = 4;
            s.x -= SPRITE_MOVE_SPEED;
            if(s.x < s.tx) s.x = s.tx;
        }
        else if(s.tx > s.x)
        {
            s.frame_dir = 12;
            s.x += SPRITE_MOVE_SPEED;
            if(s.x > s.tx) s.x = s.tx;
        }
        s.frame_dir += nf;
    }
}

void update_battle()
{
    auto& d = sdata.battle;
    d.menuy = (d.menuy + d.menuy_target) / 2;
    d.msely = (d.msely + d.msel * 10) / 2;
    ++d.frame;
    if(++d.selframe >= 7) d.selframe = 0;
    update_battle_sprites();
    switch(d.phase)
    {
    case BPHASE_INTRO:
        d.menuy = d.menuy_target = -51;
        if(d.frame == 8 && d.remove_enemy)
            active_chunks[d.enemy_chunk].enemy.active = false;
        if(d.frame == 33)
        {
            init_attack_order();
            init_sprites();
            d.esel = 4;
            d.frame = 0;
            d.current_attacker = d.num_attackers;
            d.phase = BPHASE_NEXT;
        }
        break;
    case BPHASE_NEXT:
        battle_next_turn();
        break;
    case BPHASE_MENU:
        d.menuy_target = 0;
        if(btns_pressed & BTN_A)
        {
            d.menuy_target = -51;
            if(d.msel == 0)
            {
                d.frame = 0;
                d.prev_phase = BPHASE_MENU;
                d.next_phase = BPHASE_ATTACK1;
                d.phase = BPHASE_ESEL;
            }
            else if(d.msel == 1)
            {
                d.phase = BPHASE_DEFEND;
            }
            else
            {
                d.phase = BPHASE_OUTRO;
            }
        }
        if(btns_pressed & BTN_DOWN && ++d.msel == 4) d.msel = 0;
        if(btns_pressed & BTN_UP && d.msel-- == 0) d.msel = 3;
        break;
    case BPHASE_ESEL:
        if(btns_pressed & (BTN_UP | BTN_DOWN)) d.esel ^= 1;
        if(btns_pressed & (BTN_LEFT | BTN_RIGHT)) d.esel ^= 2;
        if(btns_pressed & BTN_B) d.phase = d.prev_phase;
        if((btns_pressed & BTN_A) && d.enemies[d.esel].id != 255)
            d.frame = 0, d.phase = d.next_phase;
        break;
    case BPHASE_ATTACK1:
    {
        uint8_t i = d.attacker_id;
        auto& s = d.sprites[i];
        if(i < 4)
            s.tx = 64, d.defender_id = d.esel;
        else
            s.tx = 48;
        s.ty = d.sprites[d.defender_id].ty;
        d.phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_ATTACK2;
        break;
    }
    case BPHASE_ATTACK2:
        // TODO: attack/damage animation
        // just delay for now
        d.frame = -20;
        d.phase = BPHASE_DELAY;
        d.next_phase = BPHASE_ATTACK3;
        break;
    case BPHASE_ATTACK3:
    {
        auto& s = d.sprites[d.attacker_id];
        s.tx = s.bx;
        s.ty = s.by;
        d.phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_NEXT;
        break;
    }
    case BPHASE_DEFEND:
    {
        auto& s = d.sprites[d.attacker_id];
        uint8_t di;
        if(d.attacker_id < 4)
            s.tx = 32, di = d.pdef, d.pdef = d.attacker_id;
        else
            s.tx = 80, di = d.edef, d.edef = d.attacker_id;
        s.ty = 30;
        if(di != 255)
        {
            auto& ds = d.sprites[di];
            ds.tx = ds.bx;
            ds.ty = ds.by;
        }
        d.phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_NEXT;
        break;
    }
    case BPHASE_SPRITES:
        if(d.sprites_done)
            d.phase = d.next_phase;
        break;
    case BPHASE_DELAY:
        if(d.frame == 0)
            d.phase = d.next_phase;
        break;
    case BPHASE_OUTRO:
        // resume state
        if(!(chunks_are_running && run_chunks()))
            change_state(STATE_MAP);
    default: break;
    }
}

static void draw_battle_background()
{
    static constexpr uint8_t TS[] PROGMEM = { 10, 11, 26, 27 };
    uint8_t t = 0x23;
    for(uint8_t r = 0; r < 4; ++r)
        for(uint8_t c = 0; c < 8; ++c, t ^= (t >> 3) ^ (t << 1))
            draw_tile(c * 16, r * 16, pgm_read_byte(&TS[t & 3]));
    platform_fillrect(33, 37, 14, 12, WHITE);
    platform_drawrect(35, 39, 10, 8, LIGHT_GRAY);
    platform_fillrect(81, 37, 14, 12, WHITE);
    platform_drawrect(83, 39, 10, 8, LIGHT_GRAY);
}

static void draw_selection_arrow(uint8_t x, uint8_t y)
{
    auto const& d = sdata.battle;
    uint8_t f = (d.frame >> 1) & 7;
    f = (f < 4 ? f : 7 - f);
    platform_fx_drawplusmask(x + 4, y - 8 + f, BATTLE_ARROW_IMG, 0, 7, 8);
}

static void draw_selection_outline(uint8_t x, uint8_t y)
{
    auto const& d = sdata.battle;
    platform_fx_drawplusmask(x - 3, y - 3, BATTLE_SELECT_IMG, d.selframe, 22, 24);
}

static void draw_battle_sprites()
{
    auto const& d = sdata.battle;
    static constexpr uint8_t PARTY_IMG[] PROGMEM = { 0, 255, 255, 255, };
    draw_sprite_entry entries[8];
    uint8_t n = 0;

    for(auto const& s : d.sprites)
    {
        if(!s.active) continue;
        auto& e = entries[n++];
        e.x = s.x;
        e.y = s.y;
        e.addr = s.addr;
        e.frame = s.frame_base + s.frame_dir;
    }

    sort_and_draw_sprites(entries, n);
}

void render_battle()
{
    auto const& d = sdata.battle;
    if(d.phase == BPHASE_INTRO)
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
        draw_text_prog(50, d.menuy + 12 + d.msely, PSTR("\x7f"));
    }
    if(d.phase == BPHASE_MENU || d.phase == BPHASE_ESEL)
    {
        auto const& s = d.sprites[d.attacker_id];
        draw_selection_outline(s.x, s.y);
    }
    if(d.phase == BPHASE_ESEL)
    {
        auto const& s = d.sprites[d.esel];
        draw_selection_arrow(s.x, s.y);
    }
}
