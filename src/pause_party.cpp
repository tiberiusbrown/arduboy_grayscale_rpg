#include "common.hpp"

#include "generated/fxdata.h"

void update_pause_party()
{
    auto& d = sdata.pause;

    if(d.partyy < 64) return;

    if(btns_pressed & BTN_B)
        d.state = OS_MENU;
    else if(d.partyx == d.partyxt)
    {
        if(d.partyi > 0 && (btns_pressed & BTN_LEFT))
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
    char const* name = pgmptr(&PARTY_INFO[i].name);
    uint8_t sprite = pgm_read_byte(&PARTY_INFO[i].sprite);
    uint8_t portrait = pgm_read_byte(&PARTY_INFO[i].portrait);
    platform_drawrect(x + 9, y + 19, 36, 36, LIGHT_GRAY);
    platform_fx_drawoverwrite(x + 11, y + 21, PORTRAIT_IMG, portrait, 32, 32);
    draw_text_prog(x + 47, y + 19, name);
    if(i == d.partyi)
    {
        if(i > 0)
            platform_fx_drawoverwrite(x + 0, y + 36, ARROWS_IMG, 0, 8, 8);
        if(i < nparty - 1)
            platform_fx_drawoverwrite(x + 120, y + 36, ARROWS_IMG, 1, 8, 8);
    }
}

void render_pause_party()
{
    auto const& d = sdata.pause;
    int16_t y = 64 - d.partyy;
    platform_fx_drawoverwrite(0, y, PARTY_MEMBERS_IMG, 0, 128, 16);
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
