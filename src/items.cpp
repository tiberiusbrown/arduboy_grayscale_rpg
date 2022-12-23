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

static int8_t items_stat(uint8_t user, uint8_t offset)
{
    int8_t const* ptr = (int8_t const*)ITEM_INFO + offset;
    int16_t total = 0;

    for(auto i : party[user].equipped_items)
        if(i != INVALID_ITEM)
            total += (int8_t)pgm_read_byte(ptr + sizeof(item_info_t) * i);

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
    uint8_t cat = item_cat(i);
    return party[user].equipped_items[cat] == i;
}

void toggle_item(uint8_t user, item_t i)
{
    uint8_t cat = item_cat(i);
    item_t& eq = party[user].equipped_items[cat];
    bool equipped = (eq == i);
    if(equipped)
        eq = INVALID_ITEM;
    else
    {
        for(uint8_t u = 0; u < 4; ++u)
        {
            auto& ueq = party[u].equipped_items[cat];
            if(ueq == i)
                ueq = INVALID_ITEM;
        }
        eq = i;
    }
    party_clip_hp();
}

void update_items_numcat(sdata_items& d)
{
    for(auto& n : d.cat_nums) n = 0;
    for(uint8_t i = 0; i < NUM_CONSUMABLES; ++i)
        if(savefile.chunk_regs[8 + i] != 0)
            ++d.cat_nums[IT_CONSUMABLE];
    if(d.cat_nums[IT_CONSUMABLE] == 0)
        d.cat_nums[IT_CONSUMABLE] = 1;
    d.item_count = 0;

    // restrict access to equippable items in battle
    if(d.battle)
        return;

    ROTA_FOREACH_ITEM(i, {
        ++d.cat_nums[item_cat(i)];
        ++d.item_count;
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

static uint8_t selected_consumable(sdata_items const& d)
{
    uint8_t n = 0;
    for(uint8_t ni = 0; ni < NUM_CONSUMABLES; ++ni)
    {
        if(consumables[ni] == 0) continue;
        if(n == d.n) return ni;
        ++n;
    }
    return INVALID_ITEM;
}

bool update_items(sdata_items& d)
{
    if(d.x != d.xt)
    {
        adjust(d.x, d.xt);
        return false;
    }

    if(d.item_count != 0)
    {
        d.pcat = d.cat;
        if(btns_pressed & BTN_LEFT)
        {
            do
            {
                d.off = d.n = 0;
                if(d.cat-- == 0) d.cat = IT_NUM_CATS - 1;
            } while(d.cat_nums[d.cat] == 0);
            d.x = 0;
            d.xt = 128;
        }
        if(btns_pressed & BTN_RIGHT)
        {
            do
            {
                d.off = d.n = 0;
                if(d.cat++ == IT_NUM_CATS - 1) d.cat = 0;
            } while(d.cat_nums[d.cat] == 0);
            d.x = 128;
            d.xt = 0;
        }
    }
    if((btns_pressed & BTN_UP) && d.n > 0)
        --d.n;
    if((btns_pressed & BTN_DOWN) && uint8_t(d.n + 1) < d.cat_nums[d.cat])
        ++d.n;
    if(d.off > d.n)
        d.off = d.n;
    if(d.off < d.n - 2)
        d.off = d.n - 2;
    if((btns_pressed & BTN_A) && d.cat != IT_CONSUMABLE)
    {
        item_t selitem = selected_item(d);
        if(selitem != INVALID_ITEM)
            toggle_item(d.user_index, selitem);
    }
    if(d.conspause > 0)
    {
        if(--d.conspause == 0)
        {
            uint8_t selitem = selected_consumable(d);
            if(selitem != INVALID_ITEM)
            {
                use_consumable(d.user_index, selitem);
                if(consumables[selitem] == 0)
                {
                    --d.cat_nums[IT_CONSUMABLE];
                    if(d.n == d.cat_nums[IT_CONSUMABLE])
                    {
                        if(d.n != 0) --d.n;
                    }
                }
            }
            d.consfill = d.consw = 0;
            if(d.battle) return true;
        }
    }
    else if((btns_down & BTN_A) && d.cat == IT_CONSUMABLE)
    {
        if(d.consfill >= 128)
            d.conspause = 16;
        else
            d.consfill += 4;
    }
    else
        d.consfill = 0;

    adjust(d.consw, d.consfill);
    return false;
}

static inline void render_item_row(
    int8_t x, int8_t y, uint8_t cat, sdata_items& d,
    item_t i, uint8_t& n)
{
    if(item_cat(i) != cat) return;
    uint8_t row = n - d.off;
    uint8_t pn = n;
    ++n;
    if(row >= 3) return;
    size_t num;
    int8_t rowy = y + row * 10 + 13;
    if(d.n == pn)
    {
        num = ITEM_TOTAL_LEN;
        platform_drawrect_i8((int8_t)x, rowy, 128, 10, DARK_GRAY);
    }
    else
        num = ITEM_NAME_LEN;
    platform_fx_read_data_bytes(
        ITEM_STRINGS + ITEM_TOTAL_LEN * i, d.str, num);
    // find if there is a user who has the item equipped
    for(uint8_t user = 0; user < nparty; ++user)
    {
        if(!user_is_wearing(user, i)) continue;
        char const* name = pgmptr(&PARTY_INFO[party[user].battle.id].name);
        uint8_t w = text_width_prog(name);
        draw_text_noclip(x + 127 - w, rowy + 1, name, NOCLIPFLAG_PROG);
    }
    draw_text_noclip(x + 2, rowy + 1, d.str);
    if(d.n == pn)
        draw_text_noclip(x + 2, y + 46, &d.str[ITEM_NAME_LEN]);
}

static inline void render_consumable_row(
    int8_t x, int8_t y, sdata_items& d, int8_t n, uint8_t ni)
{
    int8_t rowy = y + n * 10 + 13;
    size_t num;
    bool selected = (d.n == d.off + n);
    if(selected)
    {
        num = ITEM_TOTAL_LEN;
        platform_drawrect_i8(x, rowy, 128, 10, DARK_GRAY);
    }
    else
        num = ITEM_NAME_LEN;
    platform_fx_read_data_bytes(
        ITEM_STRINGS + ITEM_TOTAL_LEN * NUM_ITEMS + ITEM_TOTAL_LEN * ni,
        d.str, num);
    draw_text_noclip(x + 2, rowy + 1, d.str);
    {
        char buf[5];
        char* ptr = buf;
        buf[0] = 'x';
        dec_to_str(&buf[1], consumables[ni]);
        uint8_t w = text_width(buf);
        draw_text_noclip(x + 126 - w, rowy + 1, buf);
    }
    if(selected)
    {
        draw_text_noclip(x + 2, y + 46, &d.str[ITEM_NAME_LEN]);
        if(plane() == 0)
            platform_fillrect_i8(int8_t(x), int8_t(rowy + 1), d.consw, 8, DARK_GRAY);
    }
}

static void render_items_page(
    int8_t x, int8_t y, uint8_t cat, sdata_items& d)
{
    platform_fx_drawoverwrite(x + 33, y + 0, ITEM_CATS_IMG, cat);

    if(cat == IT_CONSUMABLE)
    {
        int8_t n = -d.off;
        for(uint8_t ni = 0; ni < NUM_CONSUMABLES; ++ni)
        {
            if(consumables[ni] == 0) continue;
            if(n < 0) continue;
            if(n >= 3) break;
            render_consumable_row(x, y, d, n, ni);
            ++n;
        }
    }
    else
    {
        uint8_t n = 0;
        ROTA_FOREACH_ITEM(i, {
            render_item_row(x, y, cat, d, i, n);
        });
    }
}

void render_items(int8_t y, sdata_items& d)
{
    if(d.x < d.xt)
    {
        render_items_page(d.x - 128, y, d.cat, d);
        render_items_page(d.x, y, d.pcat, d);
    }
    else if(d.x > d.xt)
    {
        render_items_page(d.x, y, d.cat, d);
        render_items_page(d.x - 128, y, d.pcat, d);
    }
    else
    {
        render_items_page(0, y, d.cat, d);
    }

    if(d.item_count != 0)
    {
        platform_fillrect_i8(0, 0, 8, 10, BLACK);
        platform_fillrect_i8(120, 0, 8, 10, BLACK);
        platform_fx_drawoverwrite(0, y + 1, ARROWS_IMG);
        platform_fx_drawoverwrite(120, y + 1, ARROWS_IMG, 1);
    }

    platform_fillrect_i8(0, y + 11, 128, 1, WHITE);
    platform_fillrect_i8(0, y + 44, 128, 1, WHITE);
}

void use_consumable(uint8_t user, uint8_t i)
{
    auto& u = party[user];
    MY_ASSERT(i < NUM_CONSUMABLES);
    --consumables[i];
    switch(i)
    {
    case CIT_Healing_Salve:
        u.battle.hp += 5;
        break;
    case CIT_Elixir_of_Health:
        u.battle.hp += 10;
        break;
    default:
        break;
    }
    party_clip_hp();
}
