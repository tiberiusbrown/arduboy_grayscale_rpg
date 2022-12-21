#include "common.hpp"

#include "generated/fxdata.h"

constexpr uint8_t HP_BAR_WIDTH = 14;
constexpr uint8_t DEFEND_Y = 28;
constexpr uint8_t DEFEND_X1 = 35;
constexpr uint8_t DEFEND_X2 = 77;
constexpr uint8_t ASLEEP_FRAMES = 16;
constexpr uint8_t DAMAGED_FRAMES = 8;

static battle_member_t& member(uint8_t id)
{
    auto& d = sdata.battle;
    return id < 4 ? party[id].battle : d.enemies[id - 4];
}

static uint8_t const BATTLE_POS[] PROGMEM =
{
    18, 20, 18, 45,   1, 12,   1, 34,
    94, 20, 94, 45, 111, 12, 111, 34,
};

static void move_sprite(uint8_t i, uint8_t x, uint8_t y)
{
    auto& d = sdata.battle;
    auto& s = d.sprites[i];
    s.tx = x;
    s.ty = y;
    int8_t dx = s.x - x;
    int8_t dy = s.y - y;
    if(dx < 0) dx = -dx;
    if(dy < 0) dy = -dy;
    uint8_t dd = uint8_t(dx < dy ? dy : dx);
    if(dd < 32)
        s.move_speed = 2;
    else
        s.move_speed = 3;
}

static void move_sprite_to_base(uint8_t i)
{
    auto& d = sdata.battle;
    auto& s = d.sprites[i];
    move_sprite(i, s.bx, s.by);
}

static uint8_t get_att(uint8_t id)
{
    auto const& d = sdata.battle;
    if(id < 4)
        return party_att(id);
    return pgm_read_byte(&ENEMY_INFO[d.enemies[id - 4].id].att);
}

static uint8_t get_def(uint8_t id)
{
    auto const& d = sdata.battle;
    if(id < 4)
        return party_def(id);
    return pgm_read_byte(&ENEMY_INFO[d.enemies[id - 4].id].def);
}

static uint8_t get_spd(uint8_t i)
{
    auto const& d = sdata.battle;
    if(i < 4)
        return party_spd(i);
    return pgm_read_byte(&ENEMY_INFO[d.enemies[i - 4].id].spd);
}

static uint8_t get_mhp(uint8_t id)
{
    auto const& d = sdata.battle;
    if(id < 4)
        return party_mhp(id);
    return pgm_read_byte(&ENEMY_INFO[d.enemies[id - 4].id].mhp);
}

static uint8_t calc_hp_bar_width(uint8_t hp, uint8_t mhp)
{
    uint8_t f = ((uint8_t(hp) * HP_BAR_WIDTH + mhp / 2) / mhp);
    if(f == 0 && hp > 0)
        f = 1;
    return f;
}

static void init_hp_bars()
{
    auto& d = sdata.battle;
    for(uint8_t i = 0; i < 8; ++i)
    {
        auto& s = d.sprites[i];
        if(!s.active) continue;
        s.hpt = calc_hp_bar_width(member(i).hp, get_mhp(i));
    }
}

static void init_attack_order() {
    auto& d = sdata.battle;
    uint8_t speeds[9]; // +1 slot for amulet of Zhar-Tul double turn
    uint8_t n = 0;
    for(uint8_t j = 0; j < 4; ++j)
    {
        uint8_t i = j;
        if(i < nparty && member(i).id != INVALID)
        {
            uint8_t spd = get_spd(i);
            speeds[n] = spd;
            d.attack_order[n++] = i;
            if(user_is_wearing(i, SFLAG_ITEM_Amulet_of_Zhar_Tul))
            {
                speeds[n] = spd;
                d.attack_order[n++] = i;
            }
        }
        i += 4;
        if(member(i).id != INVALID)
        {
            speeds[n] = get_spd(i);
            d.attack_order[n++] = i;
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

    for(uint8_t i = 0; i < 8; ++i)
    {
        uint8_t id = member(i).id;
        if(id == INVALID) continue;
        auto& s = d.sprites[i];
        s.active = true;
        s.hp = s.hpt;
        uint8_t const* ptr = &BATTLE_POS[uint8_t(i * 2)];
        s.bx = s.tx = s.x = pgm_read_byte_inc(ptr);
        s.by = s.ty = s.y = pgm_read_byte(ptr);
        s.frame_base = pgm_read_byte(i < 4 ?
            &PARTY_INFO[id].sprite : &ENEMY_INFO[id].sprite) * 16;
    }
    init_hp_bars();
}

static uint8_t calc_attack_damage(uint8_t attacker, uint8_t defender)
{
    auto const& d = sdata.battle;
    uint8_t att = get_att(attacker);
    uint8_t def = get_def(defender);

    int16_t dam = (att * att + (uint8_t(att + def) / 2)) / (att + def);
    
    // test if attacker is Lucy (double damage to back row)
    if(attacker < 4 && party[attacker].battle.id == 2 && defender >= 6)
        dam *= 2;

    if(dam <= 0) dam = 1;
    if(dam > 99) dam = 99;
    return (uint8_t)dam;
}

static void take_damage(uint8_t i, int8_t dam)
{
    uint8_t mhp = get_mhp(i);
    auto& d = sdata.battle;
    auto& s = d.sprites[i];
    uint8_t& hp = member(i).hp;
    if(dam > 0 && (i == d.pdef || i == d.edef))
        dam = (dam + 1) / 2;
    int8_t new_hp = hp - dam;
    if(new_hp < 0) new_hp = 0;
    if(new_hp > mhp) new_hp = mhp;
    hp = new_hp;
    s.hpt = calc_hp_bar_width(hp, mhp);
    if(dam > 0)
        s.damaged = DAMAGED_FRAMES;
}

static void battle_enemy_attack(uint8_t i)
{
    auto& d = sdata.battle;
    auto& e = d.enemies[i];

    uint8_t num_enemies = 0;
    for(uint8_t n = 0; n < 4; ++n)
        if(d.enemies[n].hp > 0)
            ++num_enemies;

    if(num_enemies >= 2 &&
        d.edef == INVALID &&
        u8rand() < pgm_read_byte(&ENEMY_INFO[e.id].defend))
    {
        d.phase = BPHASE_DEFEND;
        return;
    }

    d.phase = BPHASE_ATTACK1;

    if(d.pdef != INVALID)
    {
        d.defender_id = d.pdef;
        return;
    }

    if(u8rand() < pgm_read_byte(&ENEMY_INFO[e.id].target_weakest))
    {
        // lowest HP target
        uint8_t hp = 255;
        d.defender_id = 0;
        for(uint8_t u = 1; u < 4; ++u)
        {
            uint8_t ihp = party[u].battle.hp;
            if(ihp <= 0) continue;
            if(ihp < hp) hp = ihp, d.defender_id = u;
        }
        return;
    }

    // random target
    uint8_t j[4];
    uint8_t n = 0;
    for(uint8_t u = 0; u < 4; ++u)
        if(party[u].battle.hp > 0) j[n++] = u;
    d.defender_id = j[u8rand(n)];
}

static void battle_next_turn()
{
    auto& d = sdata.battle;
    // check for defeat
    if(party[0].battle.hp == 0)
    {
        d.next_phase = BPHASE_DEFEAT;
        d.phase = BPHASE_DELAY;
        d.frame = -32;
        return;
    }
    {
        bool victory = true;
        for(auto const& e : d.enemies)
            if(e.id != INVALID && e.hp > 0) victory = false;
        if(victory)
        {
            platform_audio_play_song_now_once(SONG_VICTORY);
            d.next_phase = BPHASE_OUTRO;
            d.phase = BPHASE_DELAY;
            d.frame = -32;
            return;
        }
    }

    d.defender_id = INVALID;
    uint8_t id;
    for(;;)
    {
        if(++d.attacker_index >= d.num_attackers)
            d.attacker_index = 0;
        id = d.attacker_id = d.attack_order[d.attacker_index];
        if(member(id).hp > 0)
            break;
    }
    if(id < 4)
    {
        if(d.pdef == id && !user_is_wearing(id, SFLAG_ITEM_Defender_s_Breastplate))
            d.pdef = INVALID;
        d.phase = BPHASE_MENU;
    }
    else
    {
        if(d.edef == id) d.edef = INVALID;
        battle_enemy_attack(id - 4);
    }
    for(uint8_t i = 0; i < 8; ++i)
        if(d.pdef != i && d.edef != i)
            move_sprite_to_base(i);
    d.next_phase = d.phase;
    d.phase = BPHASE_SPRITES;
}

static void remove_enemy(uint8_t i)
{
    auto& d = sdata.battle;
    uint8_t index;
    MY_ASSERT(i >= 4);
    for(index = 0; index < d.num_attackers; ++index)
        if(d.attack_order[index] == i)
            break;
    MY_ASSERT(index < d.num_attackers);
    if(d.attacker_index > index)
        --d.attacker_index;
    --d.num_attackers;
    for(uint8_t i = index; i < d.num_attackers; ++i)
        d.attack_order[i] = d.attack_order[i + 1];
    for(uint8_t i = 7; i >= 4; --i)
        if(d.enemies[i - 4].hp > 0)
            d.esel = i;
    if(i < 6)
    {
        // slide back row forward
        uint8_t j = INVALID;
        if(d.enemies[i - 4 + 2].hp > 0) j = i + 2;
        else if(d.enemies[(i - 4 + 2) ^ 1].hp > 0) j = (i + 2) ^ 1;
        if(j != INVALID)
        {
            auto& si = d.sprites[i];
            auto& sj = d.sprites[j];
            uint8_t bx = si.bx, by = si.by;
            for(auto& x : d.attack_order)
                if(x == j) x = i;
            if(d.edef == j) d.edef = i;
            d.enemies[i - 4] = d.enemies[j - 4];
            si = sj;
            d.enemies[j - 4].hp = 0;
            sj.active = false;
            si.bx = bx, si.by = by;
        }
    }
}

static void update_battle_sprites()
{
    auto& d = sdata.battle;
    uint8_t nf = (nframe / 4) & 3;
    d.sprites_done = true;
    for(uint8_t i = 0; i < 8; ++i)
    {
        auto& s = d.sprites[i];
        if(!s.active) continue;
        if(s.damaged > 0) --s.damaged;
        else if(member(i).hp == 0)
        {
            if(d.pdef == i) d.pdef = INVALID;
            if(d.edef == i) d.edef = INVALID;
            if(d.defender_id == i) d.defender_id = INVALID;
            s.active = false;
            if(i >= 4) remove_enemy(i--);
            continue;
        }
        if(s.hp > s.hpt) s.hp -= uint8_t(s.hp - s.hpt + 3) / 4;
        if(s.hp < s.hpt) s.hp += uint8_t(s.hpt - s.hp + 3) / 4;
        if(s.x == s.tx && s.y == s.ty)
        {
            s.frame_dir = 0;
            if(s.frame_base == 13 * 16) // Psy-Bat
                s.frame_dir += nf;
            continue;
        }
        d.sprites_done = false;

        uint8_t move_speed = s.move_speed;
        if(s.ty < s.y)
        {
            s.frame_dir = 8;
            s.y -= move_speed;
            if(s.y < s.ty) s.y = s.ty;
        }
        else if(s.ty > s.y)
        {
            s.frame_dir = 0;
            s.y += move_speed;
            if(s.y > s.ty) s.y = s.ty;
        }
        if(s.tx < s.x)
        {
            s.frame_dir = 4;
            s.x -= move_speed;
            if(s.x < s.tx) s.x = s.tx;
        }
        else if(s.tx > s.x)
        {
            s.frame_dir = 12;
            s.x += move_speed;
            if(s.x > s.tx) s.x = s.tx;
        }
        s.frame_dir += nf;
    }
}

void update_battle()
{
    auto& d = sdata.battle;
    adjust(d.menuy, d.menuy_target);
    adjust(d.msely, d.msel * 8);
    ++d.frame;
    //if(++d.selframe >= 7) d.selframe = 0;
    if(d.itemsy == 0)
        update_battle_sprites();
    switch(d.phase)
    {
    case BPHASE_ALERT:
        if(d.frame == 24)
        {
            d.frame = 0;
            d.phase = BPHASE_INTRO;
        }
        break;
    case BPHASE_INTRO:
        if(d.frame == 8 && d.remove_enemy)
        {
            chunk_sprites[d.enemy_chunk].active = false;
            init_sprites();
        }
        if(d.frame == 33)
        {
            init_attack_order();
            d.esel = 4;
            d.frame = 0;
            d.attacker_index = d.num_attackers;
            d.attacker_id = INVALID;
            d.phase = BPHASE_NEXT;
        }
        break;
    case BPHASE_NEXT:
        battle_next_turn();
        break;
    case BPHASE_MENU:
        d.menuy_target = 32;
        if(btns_pressed & BTN_A)
        {
            if(d.msel == 0)
            {
                d.frame = 0;
                d.prev_phase = BPHASE_MENU;
                d.next_phase = BPHASE_ATTACK1;
                if(d.edef != INVALID) d.esel = d.edef;
                d.phase = BPHASE_ESEL;
            }
            else if(d.msel == 1)
            {
                if(d.pdef != d.attacker_id)
                    d.phase = BPHASE_DEFEND;
            }
            else
            {
                d.items.cat = IT_CONSUMABLE;
                d.items.battle = true;
                d.phase = BPHASE_ITEM;
                update_items_numcat(d.items);
            }
            if(d.msel != BPHASE_MENU)
            {
                d.menuy_target = 0;
                d.msel = 0;
            }
        }
        if(btns_pressed & BTN_DOWN && ++d.msel == 3) d.msel = 0;
        if(btns_pressed & BTN_UP && d.msel-- == 0) d.msel = 2;
        break;
    case BPHASE_ESEL:
    {
        uint8_t esel = d.esel;
        if(btns_pressed & BTN_UP) esel &= ~1;
        if(btns_pressed & BTN_DOWN) esel |= 1;
        if(btns_pressed & BTN_LEFT) esel &= ~2;
        if(btns_pressed & BTN_RIGHT) esel |= 2;
        if(btns_pressed & BTN_B) d.phase = d.prev_phase;
        if(d.edef != INVALID) esel = d.edef;
        auto const& e = d.enemies[esel - 4];
        if(e.id != INVALID && e.hp > 0) d.esel = esel;
        if((btns_pressed & BTN_A) && d.enemies[d.esel - 4].id != INVALID)
            d.defender_id = d.esel, d.frame = 0, d.phase = d.next_phase;
        break;
    }
    case BPHASE_ATTACK1:
    {
        uint8_t i = d.attacker_id;
        uint8_t tx, ty;
        if(i < 4)
            tx = DEFEND_X2 - 18, d.defender_id = d.esel;
        else
            tx = DEFEND_X1 + 18;
        ty = d.sprites[d.defender_id].ty;
        move_sprite(i, tx, ty);
        d.phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_ATTACK2;
        break;
    }
    case BPHASE_ATTACK2:
    {
        // TODO: attack/damage animation
        // just delay for now
        d.frame = -20;
        d.phase = BPHASE_DELAY;
        d.next_phase = BPHASE_ATTACK3;
        uint8_t attacker = d.attacker_id;
        uint8_t defender = d.defender_id;
        uint8_t dam = calc_attack_damage(attacker, defender);
        take_damage(defender, (int8_t)dam);
        platform_audio_play_sfx(SFX_HIT, 1);

        // Dismas innate: strike back at 50% damage when defending
        if(d.pdef == defender &&
            party[defender].battle.id == 3)
        {
            uint8_t dismas_dam = calc_attack_damage(defender, attacker);
            ++dismas_dam;
            dismas_dam /= 2;
            take_damage(attacker, (int8_t)dismas_dam);
        }

        // Catherine innate: after attacking, heal a wounded ally for 50% damage
        if(attacker < 4 && party[attacker].battle.id == 1)
        {
            uint8_t ally = 0;
            uint8_t hp = 0;
            for(uint8_t i = 1; i < nparty; ++i)
            {
                uint8_t uhp = party_mhp(i) - party[i].battle.hp;
                if(uhp > hp)
                    ally = i, hp = uhp;
            }
            uint8_t catherine_heal = dam;
            ++catherine_heal;
            catherine_heal /= 2;
            take_damage(ally, -int8_t(catherine_heal));
        }

        break;
    } 
    case BPHASE_ATTACK3:
    {
        if(d.attacker_id == d.pdef)
            move_sprite(d.attacker_id, DEFEND_X1, DEFEND_Y);
        else
            move_sprite_to_base(d.attacker_id);
        d.phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_NEXT;
        break;
    }
    case BPHASE_DEFEND:
    {
        uint8_t di;
        uint8_t tx;
        if(d.attacker_id < 4)
            tx = DEFEND_X1, di = d.pdef, d.pdef = d.attacker_id;
        else
            tx = DEFEND_X2, di = d.edef, d.edef = d.attacker_id;
        move_sprite(d.attacker_id, tx, DEFEND_Y);
        if(di != INVALID && di != d.attacker_id)
            move_sprite_to_base(di);
        d.phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_NEXT;
        break;
    }
    case BPHASE_ITEM:
        if(btns_pressed & BTN_B)
            d.phase = BPHASE_MENU, d.msel = 2;
        if(d.itemsy == 64)
        {
            bool consumed = update_items(d.items);
            if(consumed)
            {
                init_hp_bars();
                battle_next_turn();
            }
        }
        break;
    case BPHASE_SPRITES:
        if(d.sprites_done)
            d.phase = d.next_phase;
        break;
    case BPHASE_DELAY:
        if(d.frame == 0)
            d.phase = d.next_phase;
        break;
    case BPHASE_DEFEAT:
        platform_audio_play_song_now(SONG_DEFEAT);
        change_state(STATE_GAME_OVER);
        break;
    case BPHASE_OUTRO:
        if(d.frame == 33)
            back_to_map();
        break;
    default: break;
    }
    adjust(d.itemsy, d.phase == BPHASE_ITEM ? 64 : 0);
}

static void draw_battle_background()
{
    auto const& d = sdata.battle;
    if(py >= 128 * 16)
    {
        // outdoor background
        static constexpr uint8_t TS[] PROGMEM = { 10, 11, 26, 27 };
        for(uint8_t r = 0, t = 0x23; r < 4; ++r)
        {
            uint8_t y = r * 16;
            if(y + d.itemsy >= 64) break;
            for(uint8_t c = 0; c < 8; ++c, t ^= (t >> 3) ^ (t << 1))
                draw_tile(c * 16, y, pgm_read_byte(&TS[t & 3]));
        }
    }
    else
    {
        // dungeon background
        for(uint8_t x = 0; x < 128; x += 16)
            draw_tile(x, 0, 12);
        for(uint8_t y = 16; y < 64; y += 16)
        {
            if(y + d.itemsy >= 64) break;
            for(uint8_t x = 0; x < 128; x += 16)
                draw_tile(x, y, 28);
        }
    }
    // sleeping sprites
    for(uint8_t i = 0; i < nparty; ++i)
    {
        auto const& s = d.sprites[i];
        if(s.damaged > 0 || member(i).hp > 0) continue;
        if(plane() > 1)
            platform_fx_drawplusmask(s.bx, s.by, SPRITES_IMG, s.frame_base, 16, 16);
    }
    //platform_fillrect_i8(DEFEND_X1 + 1, DEFEND_Y + 7, 14, 12, WHITE);
    platform_drawrect_i8(DEFEND_X1 + 3, DEFEND_Y + 9, 10, 8, LIGHT_GRAY);
    //platform_fillrect_i8(DEFEND_X2 + 1, DEFEND_Y + 7, 14, 12, WHITE);
    platform_drawrect_i8(DEFEND_X2 + 3, DEFEND_Y + 9, 10, 8, LIGHT_GRAY);
}

static void draw_selection_arrow(uint8_t x, uint8_t y)
{
    auto const& d = sdata.battle;
#ifdef ARDUINO
    uint8_t f;
    asm volatile(
        R"ASM(
            lds %[f], %[frame]
            lsr %[f]
            andi %[f], 7
        )ASM"
        : [f]     "=&d" (f)
        : [frame] ""    (&d.frame)
        );
#else
    uint8_t f = d.frame;
    f = (f >> 1) & 7;
#endif
    if(f >= 4) f ^= 7;
    //f = (f < 4 ? f : 7 - f);
    platform_fx_drawplusmask(x + 4, y - 12 + f, BATTLE_ARROW_IMG, 0, 9, 8);
}

static void draw_selection_outline(uint8_t x, uint8_t y)
{
    //auto const& d = sdata.battle;
    //platform_fx_drawplusmask(x - 3, y - 3, BATTLE_SELECT_IMG, d.selframe, 22, 24);
    platform_fx_drawplusmask(x + 4, y - 12, BATTLE_ATTACKER_IMG, 0, 9, 8);
}

static void draw_health(uint8_t i)
{
    auto const& d = sdata.battle;
    auto const& s = d.sprites[i];
    //uint8_t x = (i < 4 ? 0 : 126 - HP_BAR_WIDTH);
    //constexpr uint8_t y = 58;
    uint8_t x = uint8_t(s.x);
    uint8_t y = uint8_t(s.y - 5);
    constexpr uint8_t w = HP_BAR_WIDTH + 2;
    constexpr uint8_t h = 4;
    platform_drawrect_i8(x, y + 1, w, h, DARK_GRAY);

    uint8_t hp = s.hp, hpt = s.hpt;
    if(hpt < hp) tswap(hpt, hp);
    platform_fillrect_i8(x + 1 + hpt, y + 2, w - 2 - hpt, h - 2, BLACK);
    platform_fillrect_i8(x + 1 + hp, y + 2, hpt - hp, h - 2, LIGHT_GRAY);
    platform_fillrect_i8(x + 1, y + 2, hp, h - 2, WHITE);
}

static void draw_battle_sprites()
{
    auto const& d = sdata.battle;
    static constexpr uint8_t PARTY_IMG[] PROGMEM = {
        0, INVALID, INVALID, INVALID,
    };
    draw_sprite_entry entries[8];
    uint8_t n = 0;

    for(auto const& s : d.sprites)
    {
        if(!s.active) continue;
        if((nframe & 1) && s.damaged > 0)
            continue;
        auto& e = entries[n++];
        e.x = (uint8_t)s.x;
        e.y = (uint8_t)s.y;
        e.addr = SPRITES_IMG;
        e.frame = s.frame_base + s.frame_dir;
    }

    sort_and_draw_sprites(entries, n);

    for(uint8_t i = 0; i < 8; ++i)
        if(d.sprites[i].active)
            draw_health(i);
}

void render_battle()
{
    auto& d = sdata.battle;
    if(d.itemsy == 64)
    {
        render_items(0, d.items);
        return;
    }
    uint8_t phase = d.phase;
    if(phase == BPHASE_ALERT)
    {
        render_map();
        uint8_t f = d.frame;
        if(f > 7) f = 7;
        platform_fx_drawplusmask(58, 10, BATTLE_ALERT_IMG, f, 13, 16);
        return;
    }
    if(phase == BPHASE_INTRO || phase == BPHASE_OUTRO)
    {
        int16_t x;
        if(d.frame <= 8) x = 127 - d.frame * 16;
        else if(d.frame <= 24) x = (8 - d.frame);
        else x = -16 - (d.frame - 24) * 16;
        if( (phase == BPHASE_INTRO && d.frame <= 8) ||
            (phase == BPHASE_OUTRO && d.frame > 24))
        {
            render_map();
        }
        else {
            draw_battle_background();
            draw_battle_sprites();
        }
        int16_t x2 = -(x + 16);
        if(int8_t(x2) == x2)
        {
            platform_fillrect_i8(int8_t(x2), 0, 144, 24, BLACK);
            platform_fillrect_i8(int8_t(x2), 40, 144, 24, BLACK);
        }
        uint8_t f = (phase == BPHASE_INTRO ? 0 : 1);
        platform_fx_drawoverwrite(x, 24, BATTLE_BANNERS_IMG, f);
        return;
    }
    draw_battle_background();
    draw_battle_sprites();
    uint8_t menuy = d.menuy;
    if(menuy > 0)
    {
        int16_t y = d.menuy - 32;
        platform_fx_drawplusmask(51, y, BATTLE_MENU_CHAIN_IMG, 0, 3, 8);
        platform_fx_drawplusmask(74, y, BATTLE_MENU_CHAIN_IMG, 0, 3, 8);
        platform_fx_drawoverwrite(48, y + 8, BATTLE_MENU_IMG);
        draw_text_noclip(50, y + 9 + d.msely, PSTR("\x7f"), NOCLIPFLAG_PROG);
    }
    if(phase == BPHASE_MENU || phase == BPHASE_ESEL)
    {
        auto const& s = d.sprites[d.attacker_id];
        draw_selection_outline(s.x, s.y);
    }
    if(phase == BPHASE_ESEL)
    {
        auto const& s = d.sprites[d.esel];
        draw_selection_arrow(s.x, s.y);
    }
    if(d.next_phase == BPHASE_DEFEAT)
    {
        uint8_t fade_frame = uint8_t(-d.frame) * FADE_SPEED;
        if(fade_frame < 16)
            platform_fade(fade_frame);
    }
    if(d.itemsy > 0)
    {
        uint8_t y = 64 - d.itemsy;
        platform_fillrect_i8(0, (int8_t)y, 128, 64, BLACK);
        render_items(y, d.items);
    }
}
