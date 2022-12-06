#include "common.hpp"

void unequip_item(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
#ifdef ARDUINO
    uint8_t mask = FX::bitShiftLeftUInt8(i);
#else
    uint8_t mask = 1 << (i & 7);
#endif
    party[user].other_items[index] &= ~mask;
}

void equip_item(uint8_t user, item_t i)
{
    uint8_t index = i >> 3;
#ifdef ARDUINO
    uint8_t mask = FX::bitShiftLeftUInt8(i);
#else
    uint8_t mask = 1 << (i & 7);
#endif
    for(uint8_t u = 0; u < 4; ++u)
        unequip_item(u, i);
    party[user].other_items[index] |= mask;
}

void update_items(sdata_items& d)
{

}

void render_items(int16_t y, sdata_items const& d)
{

}
