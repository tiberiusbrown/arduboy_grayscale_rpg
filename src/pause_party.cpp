#include "common.hpp"

#include "generated/fxdata.h"

#include <stddef.h>

bool update_pause_party()
{
    auto& d = sdata.pause;

    if(d.partyy < 64) return false;

    if(d.itemsy == 64)
    {
        update_items(d.items);
        if(btns_pressed & BTN_B)
            d.itemsyt = 0;
    }
    else if(d.itemsy == 0 && d.partyx == d.partyxt)
    {
        uint8_t partyi = d.partyi;
        if(btns_pressed & BTN_B)
            return true;
        else if(partyi > 0 && (btns_pressed & BTN_LEFT))
        {
            --partyi;
            d.partyx = 0;
            d.partyxt = 128;
        }
        else if(partyi < nparty - 1 && (btns_pressed & BTN_RIGHT))
        {
            ++partyi;
            d.partyx = 128;
            d.partyxt = 0;
        }
        else if(btns_pressed & BTN_A)
        {
            update_items_numcat(d.items);
            d.items.cat = IT_CONSUMABLE;
            d.itemsyt = 64;
            d.items.user_index = partyi;
        }
        d.partyi = partyi;
    }
    else
    {
        adjust(d.partyx, d.partyxt);
    }
    adjust(d.itemsy, d.itemsyt);
    return false;
}

static void render_pause_party_offset(int8_t x, int8_t y, uint8_t i)
{
    auto const& d = sdata.pause;
    auto const& p = savefile.party[i];
    auto const& b = p.battle;
    uint8_t id = b.id;
    auto const* pi = &PARTY_INFO[id];
    //char const* name = pgmptr(&pi->name);

#if !DEBUG_PARTY_MENU
    platform_fx_drawoverwrite(x, y, INNATES_IMG, id);
#else
    platform_fx_drawoverwrite(x, y, INNATES_IMG, 3);
    platform_fillrect_i8(0, 37, 128, 16, BLACK);
    platform_fillrect_i8(36, 5, 128, 10, BLACK);
    platform_fillrect_i8(36, 16, 128, 18, BLACK);
    {
        static char const T[] PROGMEM = "While defensing, Dismas strikes his attacker at half damage.";
        char t[sizeof(T)];
        memcpy_P(t, T, sizeof(t));
        wrap_text(t, 128);
        draw_text_noclip(x, y + 37, t);
    }
    {
        static char const T[] PROGMEM = "Dismas";
        char t[sizeof(T)];
        memcpy_P(t, T, sizeof(t));
        draw_text_noclip(x + 38, y + 6, t);
    }
    {
        static char const T[] PROGMEM = "Attack:";
        char t[sizeof(T)];
        memcpy_P(t, T, sizeof(t));
        draw_text_noclip(x + 43, y + 17, t);
    }
    {
        static char const T[] PROGMEM = "Defense:";
        char t[sizeof(T)];
        memcpy_P(t, T, sizeof(t));
        draw_text_noclip(x + 38, y + 26, t);
    }
    {
        static char const T[] PROGMEM = "Speed:";
        char t[sizeof(T)];
        memcpy_P(t, T, sizeof(t));
        draw_text_noclip(x + 87, y + 26, t);
    }
#endif

#if 1
    static_assert(
        offsetof(party_info_t, portrait) ==
        offsetof(party_info_t, sprite) + 1,
        "revisit following block");
    uint8_t sprite, portrait;
    {
        uint8_t const* ptr = &pi->sprite;
        sprite = pgm_read_byte_inc(ptr);
        portrait = pgm_read_byte(ptr);
    }
#else
    uint8_t sprite = pgm_read_byte(&pi->sprite);
    uint8_t portrait = pgm_read_byte(&pi->portrait);
#endif

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
        //platform_drawrect_i8(x + 38, y + 0, W + 2, H + 2, DARK_GRAY);
        uint8_t mhp = party_mhp(i);
        uint8_t f = (b.hp * W + mhp / 2) / mhp;
        platform_fillrect_i8(x + 39, y + 1, f, H, WHITE);
    }
    draw_dec(x + 72, y + 17, party_att(i));
    draw_dec(x + 72, y + 26, party_def(i));
    draw_dec(x + 114, y + 26, party_spd(i));
}

void render_pause_party()
{
    auto& d = sdata.pause;

    uint8_t itemsy = d.itemsy;
    if(itemsy > 0)
        render_items(64 - itemsy, d.items);

    int8_t y = 64 - d.partyy - itemsy;
    if(y <= -64)
        return;

    //platform_fx_drawoverwrite(0, y, PARTY_MEMBERS_IMG, 0);
    platform_fillrect_i8(0, y, 128, 64, BLACK);
    uint8_t partyx = d.partyx;
    uint8_t partyi = d.partyi;
    if(partyx < d.partyxt)
    {
        render_pause_party_offset(partyx - 128, y, partyi);
        render_pause_party_offset(partyx, y, partyi + 1);
    }
    else if(partyx > d.partyxt)
    {
        render_pause_party_offset(partyx, y, partyi);
        render_pause_party_offset(partyx - 128, y, partyi - 1);
    }
    else
    {
        render_pause_party_offset(0, y, partyi);
    }
    platform_fx_drawoverwrite_i8(49, y + 55, A_ITEMS_IMG);
    platform_fillrect_i8(0, y + 35, 128, 1, LIGHT_GRAY);

    if(partyi > 0)
        platform_fx_drawoverwrite_i8(0, y + 56, ARROWS_IMG);
    if(partyi < nparty - 1)
        platform_fx_drawoverwrite(120, y + 56, ARROWS_IMG, 1);
}
