#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfac3;
constexpr uint24_t FX_DATA_BYTES = 343049;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x038000;
constexpr uint24_t PLAYER_IMG = 0x039802;
constexpr uint24_t SPRITES_IMG = 0x03B004;
constexpr uint24_t TILE_IMG = 0x045806;
constexpr uint24_t BATTLE_MENU_IMG = 0x04B808;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x04B92A;
constexpr uint24_t BATTLE_ARROW_IMG = 0x04B93E;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x04B976;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x04B9AE;
constexpr uint24_t BATTLE_ALERT_IMG = 0x04C070;
constexpr uint24_t GAME_OVER_IMG = 0x04C552;
constexpr uint24_t BATTERY_IMG = 0x04D154;
constexpr uint24_t PAUSE_MENU_IMG = 0x04D1CE;
constexpr uint24_t QUIT_IMG = 0x04D4D0;
constexpr uint24_t OPTIONS_IMG = 0x04E0D2;
constexpr uint24_t CHECK_IMG = 0x04ECD4;
constexpr uint24_t SLIDER_IMG = 0x04ECEE;
constexpr uint24_t ARROWS_IMG = 0x04ED05;
constexpr uint24_t A_ITEMS_IMG = 0x04ED67;
constexpr uint24_t AP_IMG = 0x04EDC3;
constexpr uint24_t INNATES_IMG = 0x04EDDD;
constexpr uint24_t ITEM_CATS_IMG = 0x04FFDF;
constexpr uint24_t NO_ITEMS_IMG = 0x050B81;
constexpr uint24_t GOT_ITEM_IMG = 0x051783;
constexpr uint24_t TITLE_MASKED_IMG = 0x0518F9;
constexpr uint24_t PRESS_A_IMG = 0x051E53;
constexpr uint24_t OBJECTIVE_ARROWS_IMG = 0x051FED;
constexpr uint24_t GAME_OVER_MESSAGES = 0x0525EF;
constexpr uint24_t STRINGDATA = 0x0529BB;
constexpr uint24_t ITEM_STRINGS = 0x05355D;
constexpr uint24_t PORTRAIT_STRINGS = 0x053ADD;
