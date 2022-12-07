#include "common.hpp"

#include "generated/fxdata.h"

// for offsetof
#include <stddef.h>

static void foreach_next_helper(
    item_t& i, uint8_t const*& p, uint8_t& f) FORCE_INLINE;
void foreach_next_helper(
    item_t& i, uint8_t const*& p, uint8_t& f)
{
    ++i;
    f >>= 1;
    if(!(i & 7))
        f = deref_inc(p);
}
#define ROTA_FOREACH_ITEM_EX_(i__, body__, p_start__) do {       \
    item_t i__ = 0;                                              \
    uint8_t const* p__ = p_start__;                              \
    uint8_t f__ = deref_inc(p__);                                \
    for(; i__ < NUM_ITEMS; foreach_next_helper(i__, p__, f__)) { \
        if(f__ & 1) { body__; }                                  \
    }                                                            \
    } while(0)
#define ROTA_FOREACH_ITEM(i__, body__) \
    ROTA_FOREACH_ITEM_EX_(i__, body__, story_flags)
#define ROTA_FOREACH_USER_ITEM(user__, i__, body__) \
    ROTA_FOREACH_ITEM_EX_(i__, body__, party[user].equipped_items)

static int8_t items_stat(uint8_t user, uint8_t offset)
{
    int8_t const* ptr = (int8_t const*)ITEM_INFO + offset;
    int16_t total = 0;

    ROTA_FOREACH_USER_ITEM(user, i, {
        total += (int8_t)pgm_read_byte(ptr + sizeof(item_info_t) * i);
    });

    if(total < -99) total = -99;
    if(total > +99) total = +99;
    return int8_t(total);
}

int8_t items_att(uint8_t user) { return items_stat(user, offsetof(item_info_t, att)); }
int8_t items_def(uint8_t user) { return items_stat(user, offsetof(item_info_t, def)); }
int8_t items_spd(uint8_t user) { return items_stat(user, offsetof(item_info_t, spd)); }
int8_t items_mhp(uint8_t user) { return items_stat(user, offsetof(item_info_t, mhp)); }

static uint8_t item_cat(item_t i) FORCE_NOINLINE;
uint8_t item_cat(item_t i)
{
    return pgm_read_byte(&ITEM_INFO[i].type);
}

bool user_is_wearing(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    return (party[user].equipped_items[index] & mask) != 0;
}

void unequip_item_basic(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    party[user].equipped_items[index] &= ~mask;
}

void equip_item_basic(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    party[user].equipped_items[index] |= mask;
}

void toggle_item(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
    uint8_t mask = bitmask(i);
    uint8_t& byte = party[user].equipped_items[index];
    bool equip = (byte & mask) == 0;
    for(uint8_t u = 0; u < 4; ++u)
        party[u].equipped_items[index] &= ~mask;
    if(equip)
    {
        uint8_t cat = item_cat(i);
        if(uint8_t(cat - 1) < 6)
        {
            // remove other items of the same category
            ROTA_FOREACH_USER_ITEM(user, j, {
                if(item_cat(j) == cat)
                    unequip_item_basic(user, j);
            });
        }
        byte |= mask;
    }
    party_clip_hp();
}

void update_items_numcat(sdata_items& d)
{
    for(auto& n : d.cat_nums) n = 0;

    ROTA_FOREACH_ITEM(i, {
        ++d.cat_nums[item_cat(i)];
    });
}

static item_t selected_item(sdata_items const& d)
{
    uint8_t n = 0;
    ROTA_FOREACH_ITEM(i, {
        if(item_cat(i) != d.cat) continue;
        if(n == d.n) return i;
        ++n;
    });
    return INVALID_ITEM;
}

void update_items(sdata_items& d)
{
    uint8_t any_items = 0;
    for(auto c : d.cat_nums)
        any_items |= c;
    if(any_items == 0) return;
    if(btns_pressed & BTN_LEFT)
    {
        d.off = d.n = 0;
        if(d.cat-- == 0) d.cat = 6;
    }
    if(btns_pressed & BTN_RIGHT)
    {
        d.off = d.n = 0;
        if(d.cat++ == 6) d.cat = 0;
    }
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
    if(item_cat(i) != cat) return;
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

    uint8_t n = 0;
    ROTA_FOREACH_ITEM(i, {
        render_item_row(x, y, cat, d, i, n);
    });
}

void render_items(int16_t y, sdata_items& d)
{
    uint8_t any_items = 0;
    for(auto c : d.cat_nums)
        any_items |= c;
    if(any_items == 0)
    {
        platform_fx_drawoverwrite(0, 0, NO_ITEMS_IMG, 0);
        return;
    }
    render_items_page(0, y, d.cat, d);
    platform_fx_drawoverwrite(0, y + 1, ARROWS_IMG, 0);
    platform_fx_drawoverwrite(120, y + 1, ARROWS_IMG, 1);
    platform_fillrect(0, 11, 128, 1, WHITE);
    platform_fillrect(0, 44, 128, 1, WHITE);
}
