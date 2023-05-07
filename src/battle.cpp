#include "common.hpp"

#include "generated/fxdata.h"

constexpr uint8_t HP_BAR_WIDTH = 14;
constexpr uint8_t DEFEND_Y = 28;
constexpr uint8_t DEFEND_X1 = 35;
constexpr uint8_t DEFEND_X2 = 77;
constexpr uint8_t ASLEEP_FRAMES = 24;
constexpr uint8_t DAMAGED_FRAMES = 16;

static FORCE_NOINLINE battle_member_t& member(uint8_t id)
{
    auto& d = sdata.battle;
    return id < 4 ? party[id].battle : d.enemies[id - 4];
}

static uint8_t const BATTLE_POS[] PROGMEM =
{
    18, 20, 18, 45,   1, 12,   1, 34,
    94, 20, 94, 45, 111, 12, 111, 34,
};

static FORCE_NOINLINE void move_sprite(uint8_t i, uint8_t x, uint8_t y)
{
    auto& d = sdata.battle;
    auto& s = d.sprites[i];
    s.tx = x;
    s.ty = y;
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
    {
        uint16_t r = party_att(id) + d.att_bonus[id];
        return r > 99 ? 99 : (uint8_t)r;
    }
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
        s.sprite = pgm_read_byte(i < 4 ?
            &PARTY_INFO[id].sprite : &ENEMY_INFO[id].sprite);
    }
    init_hp_bars();
}

static uint8_t calc_attack_damage(uint8_t attacker, uint8_t defender)
{
    auto const& d = sdata.battle;
    uint8_t att = get_att(attacker);
    uint8_t def = get_def(defender);

    // initial damage, scaled up 16x
    int16_t dam = (att * att * 16) / uint8_t(att + def);
    
    // test if attacker is Lucy (double damage to back row)
    if(attacker < 4 && party[attacker].battle.id == 2 && defender >= 6)
        dam *= 2;

    // Ardu's Fury does double damage
    if(d.special_attack == CIT_Ardu_s_Fury)
        dam *= 2;

    // scale back down 16x
    dam >>= 4;

    uint8_t rdam = (uint8_t)dam;
    if(rdam <= 0) rdam = 1;
    if(rdam > 99) rdam = 99;
    return rdam;
}

static void take_damage(uint8_t i, int8_t dam)
{
    uint8_t mhp = get_mhp(i);
    auto& d = sdata.battle;
    auto& s = d.sprites[i];
    uint8_t& hp = member(i).hp;

    // defenders take half damage
    if(dam > 0 && (i == d.pdef || i == d.edef))
        dam = asr(dam + 1);

    int8_t new_hp = hp - dam;
    if(new_hp < 0) new_hp = 0;
    if(new_hp > mhp) new_hp = mhp;

    // tutorial fight: ensure player doesn't die
    if(new_hp == 0 && i < 4 && d.flag == SFLAG_first_cave_guard2)
        new_hp = 1;

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
            set_music_from_position();
            platform_audio_play_song(SONG_VICTORY);
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
        uint8_t attacker_index = d.attacker_index;
        if(++attacker_index >= d.num_attackers)
            attacker_index = 0;
        id = d.attacker_id = d.attack_order[attacker_index];
        d.attacker_index = attacker_index;
        auto& s = d.sprites[id];
        if(s.flags & BFLAG_STUNNED)
            s.flags &= ~BFLAG_STUNNED; // unset flag and skip turn
        else if(member(id).hp > 0)
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
        uint8_t shp = s.hp, shpt = s.hpt;
        if(shp > shpt) shp -= uint8_t(shp - shpt + 3) / 4;
        if(shp < shpt) shp += uint8_t(shpt - shp + 3) / 4;
        s.hp = shp;
        int8_t sx = s.x;
        int8_t sy = s.y;
        if((uint8_t)sx == s.tx && (uint8_t)sy == s.ty)
        {
            s.frame_dir = 0;
            continue;
        }
        d.sprites_done = false;

        constexpr uint8_t move_speed = 2;
        if(s.ty < sy)
        {
            s.frame_dir = 8;
            sy -= move_speed;
            if(sy < (int8_t)s.ty) sy = (int8_t)s.ty;
        }
        else if(s.ty > sy)
        {
            s.frame_dir = 0;
            sy += move_speed;
            if(sy > (int8_t)s.ty) sy = (int8_t)s.ty;
        }
        s.y = sy;
        if(s.tx < sx)
        {
            s.frame_dir = 4;
            sx -= move_speed;
            if(sx < (int8_t)s.tx) sx = (int8_t)s.tx;
        }
        else if(s.tx > sx)
        {
            s.frame_dir = 12;
            sx += move_speed;
            if(sx > (int8_t)s.tx) sx = (int8_t)s.tx;
        }
        s.x = sx;
    }
}

void update_battle()
{
    auto& d = sdata.battle;
    adjust(d.menuy, d.menuy_target);
    adjust(d.msely, d.msel * 8);
    ++d.frame;
    //if(++d.selframe >= 7) d.selframe = 0;
    d.sprites_done = false;
    if(d.itemsy == 0)
        update_battle_sprites();
    battle_phase_t phase = d.phase;
    switch(phase)
    {
    case BPHASE_ALERT:
        if(d.frame == 24)
        {
            d.frame = 0;
            phase = BPHASE_INTRO;
            set_music(music::battle);
        }
        break;
    case BPHASE_INTRO:
        if(d.frame == 8 && d.remove_enemy)
        {
            chunk_sprites[running_chunk].active = false;
            init_sprites();
        }
        if(d.frame == 33)
        {
            init_attack_order();
            d.esel = 4;
            d.frame = 0;
            d.attacker_index = d.num_attackers;
            d.attacker_id = INVALID;
            phase = BPHASE_NEXT;
        }
        break;
    case BPHASE_NEXT:
        battle_next_turn();
        phase = d.phase;
        break;
    case BPHASE_MENU:
    {
        d.menuy_target = 32;
        uint8_t msel = d.msel;
        if(btns_pressed & BTN_A)
        {
            if(msel == 0)
            {
                d.frame = 0;
                d.prev_phase = BPHASE_MENU;
                d.next_phase = BPHASE_ATTACK1;
                if(d.edef != INVALID) d.esel = d.edef;
                phase = BPHASE_ESEL;
            }
            else if(msel == 1)
            {
                if(d.pdef != d.attacker_id)
                    phase = BPHASE_DEFEND;
            }
            else
            {
                d.items.cat = IT_CONSUMABLE;
                phase = BPHASE_ITEM;
                update_items_numcat(d.items);
            }
            if(msel != BPHASE_MENU)
            {
                d.menuy_target = 0;
                msel = 0;
            }
        }
        if(btns_pressed & BTN_DOWN && ++msel == 3) msel = 0;
        if(btns_pressed & BTN_UP && msel-- == 0) msel = 2;
        d.msel = msel;
        break;
    }
    case BPHASE_ESEL:
    {
        uint8_t esel = d.esel;
        if(btns_pressed & BTN_UP) esel &= ~1;
        if(btns_pressed & BTN_DOWN) esel |= 1;
        if(btns_pressed & BTN_LEFT) esel &= ~2;
        if(btns_pressed & BTN_RIGHT) esel |= 2;
        if(btns_pressed & BTN_B) phase = d.prev_phase;
        if(d.edef != INVALID) esel = d.edef;
        auto const& e = d.enemies[esel - 4];
        if(e.id != INVALID && e.hp > 0) d.esel = esel;
        esel = d.esel;
        if((btns_pressed & BTN_A) && d.enemies[esel - 4].id != INVALID)
            d.defender_id = esel, d.frame = 0, phase = d.next_phase;
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
        phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_ATTACK2;
        break;
    }
    case BPHASE_ATTACK2:
    {
        d.frame = -24;
        phase = BPHASE_DELAY;
        d.next_phase = BPHASE_ATTACK3;
        uint8_t attacker = d.attacker_id;
        uint8_t defender = d.defender_id;
        uint8_t dam;
        if(d.special_attack == CIT_Ardu_s_Frenzy)
        {
            dam = get_att(attacker);
            dam = lsr(dam + 1);
            for(uint8_t i = 4; i < 8; ++i)
                if(d.sprites[i].active)
                    take_damage(i, (int8_t)dam);
        }
        else
        {
            dam = calc_attack_damage(attacker, defender);
            take_damage(defender, (int8_t)dam);
        }
        //platform_audio_play_sfx(SFX_HIT, 1);

        // Dismas innate: strike back at 50% damage when defending
        if(d.pdef == defender &&
            party[defender].battle.id == 3)
        {
            uint8_t dismas_dam = calc_attack_damage(defender, attacker);
            dismas_dam = lsr(dismas_dam + 1);
            take_damage(attacker, (int8_t)dismas_dam);
        }

        // Catherine innate: after attacking, heal a wounded ally for 50% damage
        if(attacker < 4 && party[attacker].battle.id == 1)
        {
            uint8_t ally = 0;
            uint8_t hp = 0;
            for(uint8_t i = 1; i < nparty; ++i)
            {
                if(!d.sprites[i].active) continue;
                uint8_t uhp = party_mhp(i) - party[i].battle.hp;
                if(uhp > hp)
                    ally = i, hp = uhp;
            }
            uint8_t catherine_heal = lsr(dam + 1);
            take_damage(ally, -int8_t(catherine_heal));
        }

        // check for stun item
        {
            uint8_t n = user_item_count<
                SFLAG_ITEM_Boxing_Gloves>
                (attacker);
            if(n > 0)
            {
                if(d.special_attack == CIT_Ardu_s_Frenzy)
                {
                    for(uint8_t i = 4; i < 8; ++i)
                        d.sprites[i].flags |= BFLAG_STUNNED;
                }
                else d.sprites[defender].flags |= BFLAG_STUNNED;
            }
        }

        // check for Vampiric item (heals 1 health after each attack)
        {
            uint8_t v = user_item_count<
                SFLAG_ITEM_Small_Vampiric_Dagger,
                SFLAG_ITEM_Lifedrain_Necklace>
                (attacker);
            take_damage(attacker, -(int8_t)v);
        }

        d.special_attack = 0;

        break;
    } 
    case BPHASE_ATTACK3:
    {
        if(d.attacker_id == d.pdef)
            move_sprite(d.attacker_id, DEFEND_X1, DEFEND_Y);
        else
            move_sprite_to_base(d.attacker_id);
        phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_NEXT;
        break;
    }
    case BPHASE_DEFEND:
    {
        uint8_t di;
        uint8_t tx;
        uint8_t attacker_id = d.attacker_id;
        if(attacker_id < 4)
        {
            tx = DEFEND_X1, di = d.pdef, d.pdef = attacker_id;
            uint8_t n = user_item_count<
                SFLAG_ITEM_River_Amulet>
                (attacker_id);
            take_damage(attacker_id, -3 * n);
        }
        else
            tx = DEFEND_X2, di = d.edef, d.edef = attacker_id;
        move_sprite(attacker_id, tx, DEFEND_Y);
        if(di != INVALID && di != attacker_id)
            move_sprite_to_base(di);
        phase = BPHASE_SPRITES;
        d.next_phase = BPHASE_NEXT;
        break;
    }
    case BPHASE_ITEM:
    {
        if(btns_pressed & BTN_B)
            phase = BPHASE_MENU, d.msel = 2;
        if(d.itemsy != 64)
            break;
        if(!update_items(d.items))
            break;
        uint8_t consumed = d.items.consumed;
        if(consumed == CIT_Ardu_s_Frenzy)
        {
            uint8_t i = d.attacker_id;
            uint8_t tx = DEFEND_X2 - 18;
            uint8_t ty = 24;
            move_sprite(i, tx, ty);
            phase = BPHASE_SPRITES;
            d.next_phase = BPHASE_ATTACK2;
            d.special_attack = CIT_Ardu_s_Frenzy;
            break;
        }
        if(consumed == CIT_Ardu_s_Fury)
        {
            d.special_attack = CIT_Ardu_s_Fury;
            // target highest health enemy
            uint8_t defender = 4;
            uint8_t dhp = d.enemies[0].hp;
            for(uint8_t i = 1; i < 4; ++i)
                if(d.enemies[i].hp > dhp)
                    dhp = d.enemies[i].hp, defender = i + 4;
            d.esel = defender;
            phase = BPHASE_ATTACK1;
            break;
        }
        if(consumed == CIT_Potion_of_Attack)
        {
            uint8_t i = d.attacker_id;
            uint8_t b = d.att_bonus[i];
            if(b <= 100)
                b += 5;
            d.att_bonus[i] = b;
        }
        init_hp_bars();
        battle_next_turn();
        phase = d.phase;
        break;
    }
    case BPHASE_SPRITES:
        if(d.sprites_done)
            phase = d.next_phase;
        break;
    case BPHASE_DELAY:
        if(d.frame == 0)
            phase = d.next_phase;
        break;
    case BPHASE_DEFEAT:
        change_state(STATE_GAME_OVER);
        return;
    case BPHASE_OUTRO:
        if(d.frame == 33)
        {
            back_to_map();
            return;
        }
        break;
    default: break;
    }
    adjust(d.itemsy, phase == BPHASE_ITEM ? 64 : 0);
    d.phase = phase;
}

static uint16_t battle_sprite_frame(battle_sprite_t const& s)
{
    uint16_t f = s.sprite;
    uint8_t flags = pgm_read_byte(&SPRITE_FLAGS[f]);
    uint8_t nf = (nframe / 4) & 3;
    if(!(flags & SF_ALWAYS_ANIM) && (uint8_t)s.x == s.tx && (uint8_t)s.y == s.ty)
        nf = 0;
    if(f >= 16) f = uint8_t(f - 15) * 16 + s.frame_dir + nf;
    return f;
}

static void draw_battle_background()
{
    auto const& d = sdata.battle;
    if(player_is_outside())
    {
        // outdoor background
        static constexpr uint8_t TS[] PROGMEM = { 6, 7, 22, 23 };
        for(uint8_t y = 0, t = 0x23; y < 64; y += 16)
        {
            if(y + d.itemsy >= 64) break;
            for(uint8_t c = 0; c < 8; ++c, t ^= (t >> 3) ^ (t << 1))
                draw_tile(c * 16, y, pgm_read_byte(&TS[t & 3]));
        }
    }
    else
    {
        // dungeon background
        for(uint8_t x = 0; x < 128; x += 16)
            draw_tile(x, 0, 514);
        for(uint8_t y = 0; y < 64; y += 16)
        {
            if(y + d.itemsy >= 64) break;
            for(uint8_t x = 0; x < 128; x += 16)
                draw_tile(x, y, 530);
        }
    }
    // sleeping sprites
    for(uint8_t i = 0; i < nparty; ++i)
    {
        auto const& s = d.sprites[i];
        if(s.damaged > 0 || member(i).hp > 0) continue;
        if(plane() > 1)
            platform_fx_drawplusmask(s.bx, s.by, 16, 16, SPRITES_IMG, battle_sprite_frame(s));
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
        )ASM"
        : [f]     "=&r" (f)
        : [frame] ""    (&d.frame)
        );
#else
    uint8_t f = d.frame;
    f >>= 1;
#endif
    if(f & 4) f = ~f;
    f &= 7;
    platform_fx_drawplusmask(x + 4, y - 12 + f, 9, 8, BATTLE_ARROW_IMG, 0);
}

static void draw_selection_outline(uint8_t x, uint8_t y)
{
    //auto const& d = sdata.battle;
    //platform_fx_drawplusmask(x - 3, y - 3, 22, 24, BATTLE_SELECT_IMG, d.selframe);
    platform_fx_drawplusmask(x + 4, y - 12, 9, 8, BATTLE_ATTACKER_IMG, 0);
}

static void draw_health(uint8_t i)
{
    auto const& d = sdata.battle;
    auto const& s = d.sprites[i];
    if(!s.active) return;
    uint8_t x = uint8_t(s.x);
    uint8_t y = uint8_t(s.y - 5);
    constexpr uint8_t w = HP_BAR_WIDTH + 2;
    constexpr uint8_t h = 4;
    platform_drawrect_i8(x, y + 1, w, h, DARK_GRAY);

    if(s.flags & BFLAG_STUNNED)
        platform_fx_drawoverwrite_i8(x, y - 4, BATTLE_ZZ_IMG);

    uint8_t hp = s.hp, hpt = s.hpt;
    if(hpt < hp) tswap(hpt, hp);
    x += 1;
    y += 2;
    platform_fillrect_i8(x + hpt, y, w - 2 - hpt, h - 2, BLACK);
    platform_fillrect_i8(x + hp, y, hpt - hp, h - 2, LIGHT_GRAY);
    platform_fillrect_i8(x, y, hp, h - 2, WHITE);
}

static void draw_battle_sprites()
{
    auto const& d = sdata.battle;
    draw_sprite_entry entries[8];
    uint8_t n = 0;

    for(auto const& s : d.sprites)
    {
        if(!s.active) continue;
        if((nframe & 2) && s.damaged > 0)
            continue;
        auto& e = entries[n++];
        e.x = (uint8_t)s.x;
        e.y = (uint8_t)s.y;
        e.addr = SPRITES_IMG;
        e.frame = battle_sprite_frame(s);
    }

    sort_and_draw_sprites(entries, n);

    for(uint8_t i = 0; i < 8; ++i)
        draw_health(i);
}

static void draw_order_numbers()
{
    auto const& d = sdata.battle;
    uint8_t iend = d.attacker_index;
    uint8_t i = iend;
    uint8_t pid = INVALID;
    uint8_t f = 0;
    for(;;)
    {
        if(++i >= d.num_attackers) i = 0;
        if(i == iend) break;
        uint8_t id = d.attack_order[i];
        if(id == pid) continue;
        pid = id;
        auto const& s = d.sprites[id];
        platform_fx_drawoverwrite(s.x + 5, s.y - 6, BATTLE_ORDER_IMG, f++, 6, 8);
    }
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
        platform_fx_drawplusmask(58, 10, 13, 16, BATTLE_ALERT_IMG, f);
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
        int16_t y = menuy - 32;
        platform_fx_drawplusmask(51, y, 3, 8, BATTLE_MENU_CHAIN_IMG, 0);
        platform_fx_drawplusmask(74, y, 3, 8, BATTLE_MENU_CHAIN_IMG, 0);
        platform_fx_drawoverwrite_i8(48, y + 8, BATTLE_MENU_IMG);
        draw_text_noclip(50, y + 8 + d.msely, PSTR("\x7f"), NOCLIPFLAG_PROG);
    }
    if(phase == BPHASE_MENU || phase == BPHASE_ESEL)
    {
        auto const& s = d.sprites[d.attacker_id];
        draw_selection_outline(s.x, s.y);
        if(phase == BPHASE_MENU && (btns_down & BTN_B))
        {
            draw_order_numbers();
        }
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
