#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfadc;
constexpr uint24_t FX_DATA_BYTES = 336643;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x038000;
constexpr uint24_t PLAYER_IMG = 0x039802;
constexpr uint24_t SPRITES_IMG = 0x03B004;
constexpr uint24_t TILE_IMG = 0x044C06;
constexpr uint24_t BATTLE_MENU_IMG = 0x04AC08;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x04AD2A;
constexpr uint24_t BATTLE_ARROW_IMG = 0x04AD3E;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x04AD76;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x04ADAE;
constexpr uint24_t BATTLE_ALERT_IMG = 0x04B470;
constexpr uint24_t GAME_OVER_IMG = 0x04B952;
constexpr uint24_t BATTERY_IMG = 0x04C554;
constexpr uint24_t PAUSE_MENU_IMG = 0x04C5CE;
constexpr uint24_t QUIT_IMG = 0x04C8D0;
constexpr uint24_t OPTIONS_IMG = 0x04D4D2;
constexpr uint24_t CHECK_IMG = 0x04E0D4;
constexpr uint24_t SLIDER_IMG = 0x04E0EE;
constexpr uint24_t ARROWS_IMG = 0x04E105;
constexpr uint24_t A_ITEMS_IMG = 0x04E167;
constexpr uint24_t AP_IMG = 0x04E1C3;
constexpr uint24_t INNATES_IMG = 0x04E1DD;
constexpr uint24_t ITEM_CATS_IMG = 0x04F3DF;
constexpr uint24_t NO_ITEMS_IMG = 0x04FF81;
constexpr uint24_t GOT_ITEM_IMG = 0x050B83;
constexpr uint24_t TITLE_MASKED_IMG = 0x050CF9;
constexpr uint24_t PRESS_A_IMG = 0x051253;
constexpr uint24_t GAME_OVER_MESSAGES = 0x0513ED;
constexpr uint24_t STRINGDATA = 0x05171D;
constexpr uint24_t ITEM_STRINGS = 0x051CAF;
constexpr uint24_t PORTRAIT_STRINGS = 0x0521D7;
