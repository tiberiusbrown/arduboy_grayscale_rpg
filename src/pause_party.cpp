#include "common.hpp"

#include "generated/fxdata.h"

void update_pause_party()
{
    auto& d = sdata.pause;

    if(d.partyy < 64) return;

    if(d.partyx == d.partyxt)
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

        }
    }
    else
        d.partyx = adjust(d.partyx, d.partyxt);
}

static void render_pause_party_offset(int16_t x, int16_t y, uint8_t i)
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
        char t[] = "While defending, Dismas has a 50% chance to strike back.";
        wrap_text(t, 128);
        draw_text_noclip(x, y + 37, t);
    }
#endif

    uint8_t sprite = pgm_read_byte(&pi->sprite);
    uint8_t portrait = pgm_read_byte(&pi->portrait);
    platform_drawrect(x, y, 36, 36, LIGHT_GRAY);
    platform_fx_drawoverwrite(x + 2, y + 2, PORTRAIT_IMG, portrait);
    draw_text_noclip(x + 38, y, name, NOCLIPFLAG_PROG);
    {
        // health
        char buf[8];
        char* t = buf;
        t += dec_to_str(t, b.hp);
        *t++ = '/';
        (void)dec_to_str(t, party_mhp(i));
        draw_text_noclip(uint8_t(x + 76), uint8_t(y), buf);
    }
    draw_ap(x + 99, y + 8, b.ap);
    {
        // health bar
        constexpr uint8_t W = 58;
        constexpr uint8_t H = 2;
        platform_drawrect(x + 38, y + 8, W + 2, H + 2, DARK_GRAY);
        uint8_t mhp = party_mhp(i);
        uint8_t f = (b.hp * W + mhp / 2) / mhp;
        platform_fillrect(x + 39, y + 9, f, H, WHITE);
    }
    draw_text_noclip(x + 44, y + 14, PSTR("Attack:"), NOCLIPFLAG_PROG);
    draw_text_noclip(x + 38, y + 23, PSTR("Defense:"), NOCLIPFLAG_PROG);
    draw_text_noclip(x + 88, y + 23, PSTR("Speed:"), NOCLIPFLAG_PROG);
    draw_dec(x + 72, y + 14, party_att(i));
    draw_dec(x + 72, y + 23, party_def(i));
    draw_dec(x + 114, y + 23, party_spd(i));
}

void render_pause_party()
{
    auto const& d = sdata.pause;
    int16_t y = 64 - d.partyy;
    //platform_fx_drawoverwrite(0, y, PARTY_MEMBERS_IMG, 0);
    platform_fillrect(0, y, 128, 64, BLACK);
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
    platform_fx_drawoverwrite(49, y + 55, A_ITEMS_IMG, 0);

    if(d.partyi > 0)
        platform_fx_drawoverwrite(0, y + 56, ARROWS_IMG, 0);
    if(d.partyi < nparty - 1)
        platform_fx_drawoverwrite(120, y + 56, ARROWS_IMG, 1);
}
