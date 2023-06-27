#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.07 ****/

#ifdef ARDUINO
using uint24_t = __uint24;
#else
#include <stdint.h>
using uint24_t = uint32_t;
#endif

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xf4e6;
constexpr uint24_t FX_DATA_BYTES = 727374;

constexpr uint24_t FX_IDENTIFIER_A = 0x000000;
constexpr uint24_t MAPDATA = 0x000008;
constexpr uint24_t PORTRAIT_IMG = 0x038008;
constexpr uint24_t PLAYER_IMG = 0x03980A;
constexpr uint24_t SPRITES_IMG = 0x03B00C;
constexpr uint24_t TILE_IMG = 0x04DC0E;
constexpr uint24_t BATTLE_MENU_IMG = 0x05FC10;
constexpr uint24_t BATTLE_MENU_CHAIN_IMG = 0x05FD32;
constexpr uint24_t BATTLE_ARROW_IMG = 0x05FD46;
constexpr uint24_t BATTLE_ATTACKER_IMG = 0x05FD7E;
constexpr uint24_t BATTLE_BANNERS_IMG = 0x05FDB6;
constexpr uint24_t BATTLE_ALERT_IMG = 0x060478;
constexpr uint24_t BATTLE_ZZ_IMG = 0x06095A;
constexpr uint24_t BATTLE_ORDER_IMG = 0x060971;
constexpr uint24_t GAME_OVER_IMG = 0x0609F1;
constexpr uint24_t BATTERY_IMG = 0x0615F3;
constexpr uint24_t PAUSE_MENU_IMG = 0x06166D;
constexpr uint24_t QUIT_IMG = 0x06196F;
constexpr uint24_t OPTIONS_IMG = 0x062571;
constexpr uint24_t CHECK_IMG = 0x063173;
constexpr uint24_t SLIDER_IMG = 0x06318D;
constexpr uint24_t ARROWS_IMG = 0x0631A4;
constexpr uint24_t A_ITEMS_IMG = 0x063206;
constexpr uint24_t AP_IMG = 0x063262;
constexpr uint24_t INNATES_IMG = 0x06327C;
constexpr uint24_t ITEM_CATS_IMG = 0x065C7E;
constexpr uint24_t NO_ITEMS_IMG = 0x066994;
constexpr uint24_t GOT_ITEM_IMG = 0x067596;
constexpr uint24_t TITLE_MASKED_IMG = 0x06770C;
constexpr uint24_t PRESS_A_IMG = 0x067C66;
constexpr uint24_t OBJECTIVE_ARROWS_IMG = 0x067E00;
constexpr uint24_t WORLD_IMG = 0x068402;
constexpr uint24_t AUDIO_IMG = 0x074404;
constexpr uint24_t SFX_CHATTER = 0x07448A;
constexpr uint24_t SFX_DAMAGE = 0x07458A;
constexpr uint24_t SFX_ITEM = 0x0745A6;
constexpr uint24_t SONG_TITLE = 0x07460A;
constexpr uint24_t SONG_PEACEFUL = 0x083E90;
constexpr uint24_t SONG_DUNGEON = 0x08FFFD;
constexpr uint24_t SONG_VICTORY = 0x09C81E;
constexpr uint24_t SONG_DEFEAT = 0x09C9CB;
constexpr uint24_t SONG_BATTLE = 0x09D62F;
constexpr uint24_t SONG_BATTLE2 = 0x0A8C3C;
constexpr uint24_t ITEM_INFO = 0x0AD834;
constexpr uint24_t PORTRAIT_STRINGS = 0x0AF888;
constexpr uint24_t GAME_OVER_MESSAGES = 0x0AF9C8;
constexpr uint24_t STRINGDATA = 0x0AFED8;
constexpr uint24_t FX_IDENTIFIER_B = 0x0B1946;
