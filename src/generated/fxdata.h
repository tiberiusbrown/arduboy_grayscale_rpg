#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfcf8;
constexpr uint24_t FX_DATA_BYTES = 198648;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x018000;
constexpr uint24_t PLAYER_IMG = 0x019802;
constexpr uint24_t SPRITES_IMG = 0x01B004;
constexpr uint24_t TILE_IMG = 0x023406;
constexpr uint24_t BATTLE_MENU_IMG = 0x029408;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x02952A;
constexpr uint24_t BATTLE_ARROW_IMG = 0x02953E;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x029576;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x0295AE;
constexpr uint24_t BATTLE_ALERT_IMG = 0x029C70;
constexpr uint24_t TITLE_IMG = 0x02A152;
constexpr uint24_t GAME_OVER_IMG = 0x02AD54;
constexpr uint24_t BATTERY_IMG = 0x02B956;
constexpr uint24_t PAUSE_MENU_IMG = 0x02B9D0;
constexpr uint24_t QUIT_IMG = 0x02BCD2;
constexpr uint24_t OPTIONS_IMG = 0x02C8D4;
constexpr uint24_t CHECK_IMG = 0x02D4D6;
constexpr uint24_t SLIDER_IMG = 0x02D4F0;
constexpr uint24_t ARROWS_IMG = 0x02D507;
constexpr uint24_t A_ITEMS_IMG = 0x02D569;
constexpr uint24_t AP_IMG = 0x02D5C5;
constexpr uint24_t INNATES_IMG = 0x02D5DF;
constexpr uint24_t ITEM_CATS_IMG = 0x02E7E1;
constexpr uint24_t NO_ITEMS_IMG = 0x02F383;
constexpr uint24_t GOT_ITEM_IMG = 0x02FF85;
constexpr uint24_t GAME_OVER_MESSAGES = 0x0300FB;
constexpr uint24_t STRINGDATA = 0x03031B;
constexpr uint24_t ITEM_STRINGS = 0x030430;
