#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfaf6;
constexpr uint24_t FX_DATA_BYTES = 330050;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x038000;
constexpr uint24_t PLAYER_IMG = 0x039802;
constexpr uint24_t SPRITES_IMG = 0x03B004;
constexpr uint24_t TILE_IMG = 0x043406;
constexpr uint24_t BATTLE_MENU_IMG = 0x049408;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x04952A;
constexpr uint24_t BATTLE_ARROW_IMG = 0x04953E;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x049576;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x0495AE;
constexpr uint24_t BATTLE_ALERT_IMG = 0x049C70;
constexpr uint24_t GAME_OVER_IMG = 0x04A152;
constexpr uint24_t BATTERY_IMG = 0x04AD54;
constexpr uint24_t PAUSE_MENU_IMG = 0x04ADCE;
constexpr uint24_t QUIT_IMG = 0x04B0D0;
constexpr uint24_t OPTIONS_IMG = 0x04BCD2;
constexpr uint24_t CHECK_IMG = 0x04C8D4;
constexpr uint24_t SLIDER_IMG = 0x04C8EE;
constexpr uint24_t ARROWS_IMG = 0x04C905;
constexpr uint24_t A_ITEMS_IMG = 0x04C967;
constexpr uint24_t AP_IMG = 0x04C9C3;
constexpr uint24_t INNATES_IMG = 0x04C9DD;
constexpr uint24_t ITEM_CATS_IMG = 0x04DBDF;
constexpr uint24_t NO_ITEMS_IMG = 0x04E781;
constexpr uint24_t GOT_ITEM_IMG = 0x04F383;
constexpr uint24_t TITLE_MASKED_IMG = 0x04F4F9;
constexpr uint24_t PRESS_A_IMG = 0x04FA53;
constexpr uint24_t GAME_OVER_MESSAGES = 0x04FBED;
constexpr uint24_t STRINGDATA = 0x04FE0D;
constexpr uint24_t ITEM_STRINGS = 0x05032A;
constexpr uint24_t PORTRAIT_STRINGS = 0x050852;
