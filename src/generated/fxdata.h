#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfded;
constexpr uint24_t FX_DATA_BYTES = 135873;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x018000;
constexpr uint24_t PLAYER_IMG = 0x018500;
constexpr uint24_t SPRITES_IMG = 0x019500;
constexpr uint24_t ASLEEP_IMG = 0x01B500;
constexpr uint24_t TILE_IMG = 0x01B700;
constexpr uint24_t BATTLE_MENU_IMG = 0x01F700;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x01F840;
constexpr uint24_t BATTLE_ARROW_IMG = 0x01F84C;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x01F870;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x01F894;
constexpr uint24_t BATTLE_ALERT_IMG = 0x01FD14;
constexpr uint24_t TITLE_IMG = 0x020054;
constexpr uint24_t GAME_OVER_IMG = 0x020854;
constexpr uint24_t GAME_OVER_MESSAGES = 0x021054;
constexpr uint24_t STRINGDATA = 0x0212C1;
