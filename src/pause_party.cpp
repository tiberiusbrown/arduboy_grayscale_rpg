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
    uint8_t sprite = pgm_read_byte(&pi->sprite);
    uint8_t portrait = pgm_read_byte(&pi->portrait);
    platform_drawrect(x + 9, y + 19, 36, 36, LIGHT_GRAY);
    platform_fx_drawoverwrite(x + 11, y + 21, PORTRAIT_IMG, portrait);
    draw_text_prog(x + 47, y + 19, name);
    {
        // health bar
        constexpr uint8_t W = 64;
        constexpr uint8_t H = 3;
        platform_drawrect(x + 47, y + 29, W + 2, H + 2, DARK_GRAY);
        uint8_t mhp = party_mhp(i);
        uint8_t f = (b.hp * W + mhp / 2) / mhp;
        platform_fillrect(x + 48, y + 30, f, 3, WHITE);
    }
    draw_text_prog(x + 47, y + 37, PSTR("Attack:"));
    draw_text_prog(x + 47, y + 46, PSTR("Defense:"));
    draw_dec(x + 82, y + 37, party_att(i));
    draw_dec(x + 82, y + 46, party_def(i));
    if(i == d.partyi)
    {
        if(i > 0)
            platform_fx_drawoverwrite(x + 0, y + 36, ARROWS_IMG, 0);
        if(i < nparty - 1)
            platform_fx_drawoverwrite(x + 120, y + 36, ARROWS_IMG, 1);
    }
}

void render_pause_party()
{
    auto const& d = sdata.pause;
    int16_t y = 64 - d.partyy;
    platform_fx_drawoverwrite(0, y, PARTY_MEMBERS_IMG, 0);
    platform_fillrect(0, y + 16, 128, 48, BLACK);
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
}
