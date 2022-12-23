#include "common.hpp"

#include "generated/fxdata.h"

void update_pause_party()
{
    auto& d = sdata.pause;

    if(d.partyy < 64) return;

    if(d.itemsy == 64)
    {
        update_items(d.items);
        if(btns_pressed & BTN_B)
            d.itemsyt = 0;
    }
    else if(d.itemsy == 0 && d.partyx == d.partyxt)
    {
        if(btns_pressed & BTN_B)
            d.state = OS_MENU;
        else if(d.partyi > 0 && (btns_pressed & BTN_LEFT))
        {
            --d.partyi;
            d.partyx = 0;
            d.partyxt = 128;
        }
        else if(d.partyi < nparty - 1 && (btns_pressed & BTN_RIGHT))
        {
            ++d.partyi;
            d.partyx = 128;
            d.partyxt = 0;
        }
        else if(btns_pressed & BTN_A)
        {
            update_items_numcat(d.items);
            d.items.cat = IT_CONSUMABLE;
            d.itemsyt = 64;
            d.items.user_index = d.partyi;
        }
    }
    else
    {
        adjust(d.partyx, d.partyxt);
    }
    adjust(d.itemsy, d.itemsyt);
}

static void render_pause_party_offset(int8_t x, int8_t y, uint8_t i)
{
    auto const& d = sdata.pause;
    auto const& p = savefile.party[i];
    auto const& b = p.battle;
    auto const* pi = &PARTY_INFO[b.id];
    char const* name = pgmptr(&pi->name);

#if 1
    platform_fx_drawoverwrite(x, y + 32, INNATES_IMG, i);
#else
    {
        char t[] = "While defending, Dismas strikes his attacker at half damage.";
        wrap_text(t, 128);
        draw_text_noclip(x, y + 37, t);
    }
#endif

    uint8_t sprite = pgm_read_byte(&pi->sprite);
    uint8_t portrait = pgm_read_byte(&pi->portrait);
    platform_drawrect_i8(x, y, 36, 36, LIGHT_GRAY);
    platform_fillrect_i8(x + 36, y + 15, 92, 1, LIGHT_GRAY);
    platform_fx_drawoverwrite(x + 2, y + 2, PORTRAIT_IMG, portrait);
    draw_text_noclip(x + 38, y + 6, name, NOCLIPFLAG_PROG);
    {
        // health
        char buf[8];
        char* t = buf;
        t += dec_to_str(t, b.hp);
        store_inc(t, '/');
        (void)dec_to_str(t, party_mhp(i));
        draw_text_noclip(x + 90, y + 6, buf);
    }
    {
        // health bar
        constexpr uint8_t W = 75;
        constexpr uint8_t H = 2;
        platform_drawrect_i8(x + 38, y + 0, W + 2, H + 2, DARK_GRAY);
        uint8_t mhp = party_mhp(i);
        uint8_t f = (b.hp * W + mhp / 2) / mhp;
        platform_fillrect_i8(x + 39, y + 1, f, H, WHITE);
    }
    draw_text_noclip(x + 44, y + 17, PSTR("Attack:"), NOCLIPFLAG_PROG);
    draw_text_noclip(x + 38, y + 26, PSTR("Defense:"), NOCLIPFLAG_PROG);
    draw_text_noclip(x + 88, y + 26, PSTR("Speed:"), NOCLIPFLAG_PROG);
    draw_dec(x + 72, y + 17, party_att(i));
    draw_dec(x + 72, y + 26, party_def(i));
    draw_dec(x + 114, y + 26, party_spd(i));
}

void render_pause_party()
{
    auto& d = sdata.pause;

    if(d.itemsy > 0)
        render_items(64 - d.itemsy, d.items);

    int8_t y = 64 - d.partyy - d.itemsy;
    if(y <= -64)
        return;

    //platform_fx_drawoverwrite(0, y, PARTY_MEMBERS_IMG, 0);
    platform_fillrect_i8(0, y, 128, 64, BLACK);
    if(d.partyx < d.partyxt)
    {
        render_pause_party_offset(d.partyx - 128, y, d.partyi);
        render_pause_party_offset(d.partyx, y, d.partyi + 1);
    }
    else if(d.partyx > d.partyxt)
    {
        render_pause_party_offset(d.partyx, y, d.partyi);
        render_pause_party_offset(d.partyx - 128, y, d.partyi - 1);
    }
    else
    {
        render_pause_party_offset(0, y, d.partyi);
    }
    platform_fx_drawoverwrite(49, y + 55, A_ITEMS_IMG);
    platform_fillrect_i8(0, y + 35, 128, 1, LIGHT_GRAY);

    if(d.partyi > 0)
        platform_fx_drawoverwrite(0, y + 56, ARROWS_IMG);
    if(d.partyi < nparty - 1)
        platform_fx_drawoverwrite(120, y + 56, ARROWS_IMG, 1);
}
