#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfe1b;
constexpr uint24_t FX_DATA_BYTES = 124072;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x018000;
constexpr uint24_t PLAYER_IMG = 0x018500;
constexpr uint24_t ENEMY_IMG = 0x019500;
constexpr uint24_t TILE_IMG = 0x01AD00;
constexpr uint24_t BATTLE_MENU_IMG = 0x01DD00;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x01DE40;
constexpr uint24_t BATTLE_ARROW_IMG = 0x01DE58;
constexpr uint24_t BATTLE_START_IMG = 0x01DE74;
constexpr uint24_t BATTLE_BANNER_IMG = 0x01E0B4;
constexpr uint24_t STRINGDATA = 0x01E414;
