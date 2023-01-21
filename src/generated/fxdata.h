#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xf8f5;
constexpr uint24_t FX_DATA_BYTES = 461528;

constexpr uint24_t MAPDATA = 0x000000;
constexpr uint24_t PORTRAIT_IMG = 0x038000;
constexpr uint24_t PLAYER_IMG = 0x039802;
constexpr uint24_t SPRITES_IMG = 0x03B004;
constexpr uint24_t TILE_IMG = 0x047006;
constexpr uint24_t BATTLE_MENU_IMG = 0x059008;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x05912A;
constexpr uint24_t BATTLE_ARROW_IMG = 0x05913E;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x059176;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x0591AE;
constexpr uint24_t BATTLE_ALERT_IMG = 0x059870;
constexpr uint24_t BATTLE_ZZ_IMG = 0x059D52;
constexpr uint24_t BATTLE_ORDER_IMG = 0x059D69;
constexpr uint24_t GAME_OVER_IMG = 0x059DE9;
constexpr uint24_t BATTERY_IMG = 0x05A9EB;
constexpr uint24_t PAUSE_MENU_IMG = 0x05AA65;
constexpr uint24_t QUIT_IMG = 0x05AD67;
constexpr uint24_t OPTIONS_IMG = 0x05B969;
constexpr uint24_t CHECK_IMG = 0x05C56B;
constexpr uint24_t SLIDER_IMG = 0x05C585;
constexpr uint24_t ARROWS_IMG = 0x05C59C;
constexpr uint24_t A_ITEMS_IMG = 0x05C5FE;
constexpr uint24_t AP_IMG = 0x05C65A;
constexpr uint24_t INNATES_IMG = 0x05C674;
constexpr uint24_t ITEM_CATS_IMG = 0x05F076;
constexpr uint24_t NO_ITEMS_IMG = 0x05FC18;
constexpr uint24_t GOT_ITEM_IMG = 0x06081A;
constexpr uint24_t TITLE_MASKED_IMG = 0x060990;
constexpr uint24_t PRESS_A_IMG = 0x060EEA;
constexpr uint24_t OBJECTIVE_ARROWS_IMG = 0x061084;
constexpr uint24_t WORLD_IMG = 0x061686;
constexpr uint24_t SONG_PEACEFUL = 0x06D688;
constexpr uint24_t SONG_VICTORY = 0x06DC38;
constexpr uint24_t SONG_DEFEAT = 0x06DCF4;
constexpr uint24_t SFX_HIT = 0x06DDED;
constexpr uint24_t SFX_SOLVED = 0x06DDF7;
constexpr uint24_t ITEM_INFO = 0x06DE08;
constexpr uint24_t PORTRAIT_STRINGS = 0x06F1BE;
constexpr uint24_t GAME_OVER_MESSAGES = 0x06F2EA;
constexpr uint24_t STRINGDATA = 0x06F707;
constexpr uint24_t FX_IDENTIFIER = 0x070AD0;
