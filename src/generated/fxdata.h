#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xf9bd;
constexpr uint24_t FX_DATA_BYTES = 410207;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x038000;
constexpr uint24_t PLAYER_IMG = 0x039802;
constexpr uint24_t SPRITES_IMG = 0x03B004;
constexpr uint24_t TILE_IMG = 0x047006;
constexpr uint24_t BATTLE_MENU_IMG = 0x04D008;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x04D12A;
constexpr uint24_t BATTLE_ARROW_IMG = 0x04D13E;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x04D176;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x04D1AE;
constexpr uint24_t BATTLE_ALERT_IMG = 0x04D870;
constexpr uint24_t BATTLE_ZZ_IMG = 0x04DD52;
constexpr uint24_t GAME_OVER_IMG = 0x04DD69;
constexpr uint24_t BATTERY_IMG = 0x04E96B;
constexpr uint24_t PAUSE_MENU_IMG = 0x04E9E5;
constexpr uint24_t QUIT_IMG = 0x04ECE7;
constexpr uint24_t OPTIONS_IMG = 0x04F8E9;
constexpr uint24_t CHECK_IMG = 0x0504EB;
constexpr uint24_t SLIDER_IMG = 0x050505;
constexpr uint24_t ARROWS_IMG = 0x05051C;
constexpr uint24_t A_ITEMS_IMG = 0x05057E;
constexpr uint24_t AP_IMG = 0x0505DA;
constexpr uint24_t INNATES_IMG = 0x0505F4;
constexpr uint24_t ITEM_CATS_IMG = 0x052FF6;
constexpr uint24_t NO_ITEMS_IMG = 0x053B98;
constexpr uint24_t GOT_ITEM_IMG = 0x05479A;
constexpr uint24_t TITLE_MASKED_IMG = 0x054910;
constexpr uint24_t PRESS_A_IMG = 0x054E6A;
constexpr uint24_t OBJECTIVE_ARROWS_IMG = 0x055004;
constexpr uint24_t WORLD_IMG = 0x055606;
constexpr uint24_t SONG_PEACEFUL = 0x061608;
constexpr uint24_t SONG_VICTORY = 0x061BB7;
constexpr uint24_t SONG_DEFEAT = 0x061CEA;
constexpr uint24_t SFX_HIT = 0x061DE3;
constexpr uint24_t SFX_SOLVED = 0x061DED;
constexpr uint24_t ITEM_INFO = 0x061DFE;
constexpr uint24_t PORTRAIT_STRINGS = 0x062D36;
constexpr uint24_t GAME_OVER_MESSAGES = 0x062E62;
constexpr uint24_t STRINGDATA = 0x06322E;
