#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xf8c6;
constexpr uint24_t FX_DATA_BYTES = 473505;

constexpr uint24_t FX_IDENTIFIER_A = 0x000000;
constexpr uint24_t MAPDATA = 0x000008;
constexpr uint24_t PORTRAIT_IMG = 0x038008;
constexpr uint24_t PLAYER_IMG = 0x03980A;
constexpr uint24_t SPRITES_IMG = 0x03B00C;
constexpr uint24_t TILE_IMG = 0x047C0E;
constexpr uint24_t BATTLE_MENU_IMG = 0x059C10;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x059D32;
constexpr uint24_t BATTLE_ARROW_IMG = 0x059D46;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x059D7E;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x059DB6;
constexpr uint24_t BATTLE_ALERT_IMG = 0x05A478;
constexpr uint24_t BATTLE_ZZ_IMG = 0x05A95A;
constexpr uint24_t BATTLE_ORDER_IMG = 0x05A971;
constexpr uint24_t GAME_OVER_IMG = 0x05A9F1;
constexpr uint24_t BATTERY_IMG = 0x05B5F3;
constexpr uint24_t PAUSE_MENU_IMG = 0x05B66D;
constexpr uint24_t QUIT_IMG = 0x05B96F;
constexpr uint24_t OPTIONS_IMG = 0x05C571;
constexpr uint24_t CHECK_IMG = 0x05D173;
constexpr uint24_t SLIDER_IMG = 0x05D18D;
constexpr uint24_t ARROWS_IMG = 0x05D1A4;
constexpr uint24_t A_ITEMS_IMG = 0x05D206;
constexpr uint24_t AP_IMG = 0x05D262;
constexpr uint24_t INNATES_IMG = 0x05D27C;
constexpr uint24_t ITEM_CATS_IMG = 0x05FC7E;
constexpr uint24_t NO_ITEMS_IMG = 0x060820;
constexpr uint24_t GOT_ITEM_IMG = 0x061422;
constexpr uint24_t TITLE_MASKED_IMG = 0x061598;
constexpr uint24_t PRESS_A_IMG = 0x061AF2;
constexpr uint24_t OBJECTIVE_ARROWS_IMG = 0x061C8C;
constexpr uint24_t WORLD_IMG = 0x06228E;
constexpr uint24_t SONG_PEACEFUL = 0x06E290;
constexpr uint24_t SONG_PEACEFUL2 = 0x06E840;
constexpr uint24_t SONG_PEACEFUL3 = 0x06F15C;
constexpr uint24_t SONG_VICTORY = 0x06FB8C;
constexpr uint24_t SONG_DEFEAT = 0x06FC48;
constexpr uint24_t SONG_TEST2 = 0x06FD41;
constexpr uint24_t SFX_HIT = 0x07065D;
constexpr uint24_t SFX_SOLVED = 0x070667;
constexpr uint24_t ITEM_INFO = 0x070678;
constexpr uint24_t PORTRAIT_STRINGS = 0x071C62;
constexpr uint24_t GAME_OVER_MESSAGES = 0x071D8E;
constexpr uint24_t STRINGDATA = 0x07229E;
constexpr uint24_t FX_IDENTIFIER_B = 0x073999;
