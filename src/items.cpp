#include "common.hpp"

#include "generated/fxdata.h"

// for offsetof
#include <stddef.h>

static int8_t items_stat(uint8_t user, uint8_t offset)
{
    int8_t const* ptr = (int8_t const*)(ITEM_INFO);
    ptr += offset;
    int16_t total = 0;

    uint8_t* eq = party[user].equipped_items;
    for(uint8_t byte = 0; byte < ITEM_BYTES; ++byte)
    {
        uint8_t f = eq[byte];
        for(uint8_t b = 0; b < 8; ++b, f >>= 1, ptr += sizeof(item_info_t))
        {
            if(!(f & 1)) continue;
            total += (int8_t)pgm_read_byte(ptr);
        }
    }

    if(total < -99) total = -99;
    if(total > +99) total = +99;
    return int8_t(total);
}

int8_t items_att(uint8_t user) { return items_stat(user, offsetof(item_info_t, att)); }
int8_t items_def(uint8_t user) { return items_stat(user, offsetof(item_info_t, def)); }
int8_t items_spd(uint8_t user) { return items_stat(user, offsetof(item_info_t, spd)); }
int8_t items_mhp(uint8_t user) { return items_stat(user, offsetof(item_info_t, mhp)); }

void unequip_item(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    party[user].equipped_items[index] &= ~mask;
    party_clip_hp();
}

void equip_item(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    for(uint8_t u = 0; u < 4; ++u)
        unequip_item(u, i);
    party[user].equipped_items[index] |= mask;
    party_clip_hp();
}

void toggle_item(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    uint8_t& byte = party[user].equipped_items[index];
    bool equip = (byte & mask) == 0;
    for(uint8_t u = 0; u < 4; ++u)
        party[u].equipped_items[index] &= ~mask;
    if(equip) byte |= mask;
    party_clip_hp();
}

void update_items_numcat(sdata_items& d)
{
    item_t i = 0;
    for(auto& n : d.cat_nums) n = 0;
    for(uint8_t byte = 0; byte < ITEM_BYTES; ++byte)
    {
        // item flags are guaranteed to be at the start of story flags
        uint8_t f = savefile.story_flags[byte];
        for(uint8_t b = 0; b < 8; ++b, ++i, f >>= 1)
        {
            if(!(f & 1)) continue;
            uint8_t cat = pgm_read_byte(&ITEM_INFO[i].type);
            ++d.cat_nums[cat];
        }
    }
}

static item_t selected_item(sdata_items const& d)
{
    item_t i = 0;
    uint8_t n = 0;
    for(uint8_t byte = 0; byte < ITEM_BYTES; ++byte)
    {
        uint8_t f = savefile.story_flags[byte];
        for(uint8_t b = 0; b < 8; ++b, ++i, f >>= 1)
        {
            if(!(f & 1)) continue;
            uint8_t cat = pgm_read_byte(&ITEM_INFO[i].type);
            if(cat != d.cat) continue;
            if(n == d.n) return i;
            ++n;
        }
    }
    return INVALID_ITEM;
}

void update_items(sdata_items& d)
{
    if((btns_pressed & BTN_LEFT) && d.cat-- == 0)
        d.off = 0, d.n = 0, d.cat = 6;
    if((btns_pressed & BTN_RIGHT) && d.cat++ == 6)
        d.off = 0, d.n = 0, d.cat = 0;
    if((btns_pressed & BTN_UP) && d.n > 0)
        --d.n;
    if((btns_pressed & BTN_DOWN) && uint8_t(d.n + 1) < d.cat_nums[d.cat])
        ++d.n;
    if(d.off > d.n    ) d.off = d.n;
    if(d.off < d.n - 2) d.off = d.n - 2;
    if(btns_pressed & BTN_A)
    {
        item_t selitem = selected_item(d);
        if(selitem != INVALID_ITEM)
            toggle_item(d.user_index, selitem);
    }
}

static inline void render_item_row(
    int16_t x, int16_t y, uint8_t cat, sdata_items& d,
    item_t i, uint8_t& n)
{
    uint8_t row = n - d.off;
    if(row >= 3) return;
    if(cat != pgm_read_byte(&ITEM_INFO[i].type)) return;
    size_t num;
    int16_t rowy = y + row * 10 + 13;
    if(d.n == n)
    {
        num = ITEM_TOTAL_LEN;
        platform_drawrect(x, rowy, 128, 10, DARK_GRAY);
    }
    else
        num = ITEM_NAME_LEN;
    platform_fx_read_data_bytes(
        ITEM_STRINGS + ITEM_TOTAL_LEN * i, d.str, num);
    draw_text_noclip(x + 2, rowy + 1, d.str);
    // find if a user who has equipped the item
    if(cat != IT_CONSUMABLE)
    {
        uint8_t user = INVALID;
        for(uint8_t u = 0; u < nparty; ++u)
        {
            uint8_t byte = party[u].equipped_items[i >> 3];
            uint8_t mask = bitmask((uint8_t)i);
            if(byte & mask) user = u;
        }
        if(user != INVALID)
        {
            char const* name = pgmptr(&PARTY_INFO[party[user].battle.id].name);
            uint8_t w = text_width_prog(name);
            draw_text_noclip(x + 127 - w, rowy + 1, name, NOCLIPFLAG_PROG);
        }
    }
    if(d.n == n)
        draw_text_noclip(x, y + 46, &d.str[ITEM_NAME_LEN]);
    ++n;
}

static void render_items_page(
    int16_t x, int16_t y, uint8_t cat, sdata_items& d)
{
    platform_fx_drawoverwrite(x + 33, y + 0, ITEM_CATS_IMG, cat);
    item_t i = 0;
    uint8_t n = 0;
    for(uint8_t byte = 0; byte < ITEM_BYTES; ++byte)
    {
        // item flags are guaranteed to be at the start of story flags
        uint8_t f = savefile.story_flags[byte];
        for(uint8_t b = 0; b < 8; ++b, ++i, f >>= 1)
            if(f & 1) render_item_row(x, y, cat, d, i, n);
    }
}

void render_items(int16_t y, sdata_items& d)
{
    render_items_page(0, y, d.cat, d);
    platform_fx_drawoverwrite(0, y + 1, ARROWS_IMG, 0);
    platform_fx_drawoverwrite(120, y + 1, ARROWS_IMG, 1);
    platform_fillrect(0, 11, 128, 1, WHITE);
    platform_fillrect(0, 44, 128, 1, WHITE);
}
