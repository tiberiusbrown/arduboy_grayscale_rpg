#include "common.hpp"

#include "generated/fxdata.h"

static uint8_t const OPTION_X[] PROGMEM =
{
    0, 20, 25, 47, 52, 69, 73, 106, 111, 127,
};

constexpr int16_t PAUSE_MAP_PIXELS_W = MAP_CHUNK_COLS * 8 * EXPLORED_SCALE;
constexpr int16_t PAUSE_MAP_PIXELS_H = MAP_CHUNK_ROWS * 4 * EXPLORED_SCALE / 2;

constexpr uint8_t PAUSE_MAP_FRAMES_W = PAUSE_MAP_PIXELS_W / 128;
constexpr uint8_t PAUSE_MAP_FRAMES_H = PAUSE_MAP_PIXELS_H / 64;

void update_pause()
{
    auto& d = sdata.pause;
    if(btns_pressed & BTN_B)
    {
        if(d.state == OS_MENU) d.state = OS_RESUMING;
        else if(d.state == OS_OPTIONS) d.state = OS_MENU;
    }
    uint8_t state = d.state;
    if(state == OS_RESUMING)
    {
        if((d.ally | d.menuy) == 0)
        {
            change_state(STATE_MAP);
            return;
        }
    }
    else if(state == OS_MENU)
    {
        if(d.menuy >= 16 && d.ally == 0)
        {
            uint8_t menui = d.menui;
            if((btns_pressed & BTN_LEFT) && menui-- == 0)
                menui = 4;
            if((btns_pressed & BTN_RIGHT) && menui++ == 4)
                menui = 0;
            if(btns_pressed & BTN_A)
            {
                static_assert(1 == OS_SAVE, "");
                static_assert(2 == OS_PARTY, "");
                static_assert(3 == OS_MAP, "");
                static_assert(4 == OS_OPTIONS, "");
                static_assert(5 == OS_QUIT, "");
                state = menui + 1;
                if((menui == 0 || menui == 2) && !player_is_outside())
                    state = OS_MENU;
            }
            d.menui = menui;
        }
    }
    else if(state == OS_MAP)
    {
        int16_t msx = d.mapscrollx;
        int16_t msy = d.mapscrolly;
        if(!d.map_first)
        {
            d.map_first = true;
            msx = px / 8 - 64;
            msy = py / 8 - 32;
        }
        uint8_t mapfade = d.mapfade;
        if(!d.back_to_menu && (mapfade += FADE_SPEED) >= 32)
            mapfade = 32;
        if(d.back_to_menu && (mapfade -= FADE_SPEED) == 0)
            d.back_to_menu = false, state = OS_MENU;
        d.mapfade = mapfade;
        if(mapfade >= 16)
        {
            if(btns_pressed & BTN_A)
                d.allow_obj = true;
            else if(!(btns_down & BTN_A))
                d.allow_obj = false;
            if(btns_pressed & BTN_B)
                d.back_to_menu = true;
            if(btns_down & BTN_UP   ) msy -= 1;
            if(btns_down & BTN_DOWN ) msy += 1;
            if(btns_down & BTN_LEFT ) msx -= 1;
            if(btns_down & BTN_RIGHT) msx += 1;
        }
        if(msx < 0) msx = 0;
        if(msy < 0) msy = 0;
        if(msx >= PAUSE_MAP_PIXELS_W - 128)
            msx = PAUSE_MAP_PIXELS_W - 128;
        if(msy >= PAUSE_MAP_PIXELS_H - 64)
            msy = PAUSE_MAP_PIXELS_H - 64;
        d.mapscrollx = msx;
        d.mapscrolly = msy;
    }
    else if(state == OS_OPTIONS)
    {
        if(d.optionsy >= 64)
        {
            uint8_t optionsi = d.optionsi;
            if(btns_pressed & BTN_B)
                state = OS_MENU;
            else if((btns_pressed & BTN_UP) && optionsi-- == 0)
                optionsi = 4;
            else if((btns_pressed & BTN_DOWN) && optionsi++ == 4)
                optionsi = 0;
            else if(btns_pressed & (BTN_A | BTN_LEFT | BTN_RIGHT))
            {
                if(optionsi == 0)
                {
                    savefile.settings.sound ^= 2;
                    platform_audio_from_savefile();
                }
                else if(optionsi == 1)
                {
                    savefile.settings.sound ^= 1;
                    platform_audio_from_savefile();
                }
                else if(optionsi == 2)
                {
                    uint8_t game_speed = savefile.settings.game_speed;
                    if(game_speed < 6 && (btns_pressed & (BTN_A | BTN_RIGHT)))
                        ++game_speed;
                    if(game_speed > 0 && (btns_pressed & BTN_LEFT))
                        --game_speed;
                    savefile.settings.game_speed = game_speed;
                }
                else if(optionsi == 3)
                {
                    uint8_t brightness = savefile.settings.brightness;
                    if(brightness < 3 && (btns_pressed & (BTN_A | BTN_RIGHT)))
                        ++brightness;
                    if(brightness > 0 && (btns_pressed & BTN_LEFT))
                        --brightness;
                    savefile.settings.brightness = brightness;
                }
                else if(optionsi == 4)
                    savefile.settings.no_battery_alert = !savefile.settings.no_battery_alert;
            }
            d.optionsi = optionsi;
        }
    }
    else if(state == OS_QUIT)
    {
        if(d.quitfade > 0)
        {
            d.quitfade += FADE_SPEED;
            if(d.quitfade >= 16 * FADE_SPEED + 16)
            {
                uint8_t qi = d.quiti;
#ifdef ARDUINO
                if(qi == 0) a.exitToBootloader();
#else
                if(qi == 0) qi = 1;
#endif
                static_assert(0 == STATE_TITLE, "");
                static_assert(1 == STATE_RESUME, "");

                if(qi == 2)
                    new_game();
                change_state(qi - 1);
                return;
            }
        }
        else if(d.quity >= 64)
        {
            uint8_t quitft = d.quitft;
            if(btns_pressed & BTN_A)
                d.quitp = true;
            if(d.quitp && (btns_down & BTN_A))
            {
                if((quitft += 3) >= 96)
                {
                    quitft = 96;
                    d.quitfade = 1;
                }
            }
            else
            {
                uint8_t quiti = d.quiti;
                quitft = 0;
                if(btns_pressed & BTN_B)
                    d.quitp = false, state = OS_MENU;
                if((btns_pressed & BTN_UP) && quiti-- == 0)
                    quiti = 2;
                if((btns_pressed & BTN_DOWN) && quiti++ == 2)
                    quiti = 0;
                d.quiti = quiti;
            }
            d.quitft = quitft;
        }
    }
    else if(state == OS_SAVE)
    {
        if((d.savefade += FADE_SPEED) > 16)
        {
            save();
            change_state(STATE_RESUME);
            return;
        }
    }
    else if(state == OS_PARTY)
    {
        if(update_pause_party())
            state = OS_MENU;
    }
    {
        uint8_t const* ptr = &OPTION_X[uint8_t(d.menui * 2)];
        uint8_t ax = pgm_read_byte_inc(ptr);
        uint8_t bx = pgm_read_byte(ptr);
        if(d.menuy == 0) d.ax = ax, d.bx = bx;
        adjust(d.ax, ax);
        adjust(d.bx, bx);
    }
    adjust(d.menuy, state == OS_MENU ? 16 : 0);
    adjust(d.optionsy, state == OS_OPTIONS ? 64 : 0);
    adjust(d.quity, state == OS_QUIT ? 64 : 0);
    adjust(d.quitf, d.quitft);
    adjust(d.partyy, state == OS_PARTY ? 64 : 0);
    adjust(d.optionsiy, d.optionsi * 12);
    adjust(d.quitiy, d.quiti * 13);
    adjust(d.brightnessx, savefile.settings.brightness * 16);
    adjust(d.speedx, savefile.settings.game_speed * 8);
    d.ally = d.optionsy | d.quity | d.partyy;
    d.state = state;
}

// x is uint8_t to avoid UB when overflowing
static void render_map_quad(uint8_t x, int8_t y, uint8_t f)
{
    platform_fx_drawoverwrite((int8_t)x, y, WORLD_IMG, f + 0);
    constexpr uint8_t SCALE =
        (EXPLORED_COLS / PAUSE_MAP_FRAMES_W) *
        (EXPLORED_ROWS / PAUSE_MAP_FRAMES_H) / 8;
    constexpr uint8_t ESIZE = EXPLORED_PIXELS;
    constexpr uint8_t ISTEP = (EXPLORED_COLS - EXPLORED_COLS / PAUSE_MAP_FRAMES_W) / 8;

    uint8_t i = (f / PAUSE_MAP_FRAMES_W) * (EXPLORED_COLS / 8 * EXPLORED_ROWS / PAUSE_MAP_FRAMES_H);
    i += (f & (PAUSE_MAP_FRAMES_W - 1)) * (EXPLORED_COLS / PAUSE_MAP_FRAMES_W / 8);
    uint8_t m = 1;
    uint8_t bx = x;
    for(uint8_t r = 0; r < (EXPLORED_ROWS / PAUSE_MAP_FRAMES_H); i += ISTEP, ++r, y += ESIZE)
    {
        x = bx;
        if(y <= -16)
        {
            i += EXPLORED_COLS / PAUSE_MAP_FRAMES_W / 8;
            continue;
        }
        if(y >= 64) break;
        for(uint8_t c = 0; c < uint8_t(EXPLORED_COLS / PAUSE_MAP_FRAMES_W); ++c, x += ESIZE)
        {
            if((int8_t)x > -ESIZE && (int8_t)x < 128 && !(savefile.explored[i] & m))
                platform_fillrect_i8((int8_t)x, y, ESIZE, ESIZE, BLACK);
            if((m <<= 1) == 0)
                m = 1, i += 1;
        }
    }
}

static FORCE_NOINLINE void draw_options_check(uint8_t y, uint8_t off)
{
    platform_fx_drawoverwrite_i8(71, int8_t(y + off), CHECK_IMG);
}

void render_pause()
{
    auto const& d = sdata.pause;
    // render darkened map (exclude plane 0)
    if(d.state == OS_SAVE)
    {
        uint8_t f = 16 - d.savefade;
        if(f & 0x80) f = 0;
        platform_fade(f);
        if(f == 0) return;
    }
    if(d.ally < 64 && d.mapfade < 16 / FADE_SPEED && (d.state == OS_RESUMING || plane() > 0))
        render_map();
    if(d.menuy > 0)
    {
        platform_fx_drawoverwrite_i8(0, d.menuy - 16, PAUSE_MENU_IMG);
        if(plane() == 0 && !player_is_outside())
        {
            platform_fillrect_i8(0, int8_t(d.menuy - 16), 21, 10, BLACK);
            platform_fillrect_i8(52, int8_t(d.menuy - 16), 18, 10, BLACK);
        }
        platform_fillrect_i8(d.ax, d.menuy - 5, (d.bx - d.ax + 1), 2, WHITE);
    }
    if(d.optionsy > 0)
    {
        uint8_t y = 64 - d.optionsy;
        platform_fx_drawoverwrite_i8(0, y, OPTIONS_IMG);
        platform_fx_drawoverwrite_i8(int8_t(d.brightnessx + 70), y + 38, SLIDER_IMG);
        platform_fx_drawoverwrite_i8(int8_t(d.speedx + 70), y + 26, SLIDER_IMG);

        if(savefile.settings.sound & 2)
            draw_options_check(y, 4);
        if(savefile.settings.sound & 1)
            draw_options_check(y, 16);
        if(!savefile.settings.no_battery_alert)
            draw_options_check(y, 52);
        if(plane() == 0)
            platform_drawrect_i8(0, y + d.optionsiy + 2, 128, 12, DARK_GRAY);
        platform_fade(15);
    }
    if(d.quity > 0)
    {
        uint8_t y = 64 - d.quity;
        platform_fx_drawoverwrite_i8(0, (int8_t)y, QUIT_IMG);
        if(plane() == 0)
        {
            int8_t qy = (int8_t)y + d.quitiy + 25;
            platform_drawrect_i8(16, qy, 96, 12, DARK_GRAY);
            platform_fillrect_i8(16, qy, d.quitf, 12, DARK_GRAY);
        }
        if(d.quitfade > 16 * FADE_SPEED)
            platform_fade(16 * FADE_SPEED + 15 - d.quitfade);
    }
    if(d.partyy > 0)
    {
        render_pause_party();
    }
    if(d.mapfade >= 16)
    {
        platform_fade(d.mapfade - 16);

        int16_t msx = d.mapscrollx;
        int16_t msy = d.mapscrolly;
        {
            uint8_t mx = uint16_t(msx) / 128;
            uint8_t my = uint16_t(msy) / 64;
            int8_t ox = -(uint8_t(msx) & 127);
            int8_t oy = -(uint8_t(msy) & 63);
            uint8_t f = my * PAUSE_MAP_FRAMES_W + mx;

            render_map_quad(uint8_t(ox), int8_t(oy), f);
            render_map_quad(uint8_t(ox + 128), int8_t(oy), f + 1);
            render_map_quad(uint8_t(ox), int8_t(oy + 64), f + PAUSE_MAP_FRAMES_W);
            render_map_quad(uint8_t(ox + 128), int8_t(oy + 64), f + PAUSE_MAP_FRAMES_W + 1);
        }

        if(player_is_outside())
        {
            if(d.allow_obj && (btns_down & BTN_A))
            {
                uint8_t objx = savefile.objx;
                uint8_t objy = savefile.objy;
                if((objx | objy) != 0)
                {
                    int16_t diffx = objx * 2 - msx - 64;
                    int16_t diffy = objy * 2 - msy - 20;
                    draw_objective(diffx, diffy);
                }
            }
            if((rframe & 63) < 48)
            {
                int16_t sx, sy;
                {
                    uint16_t tpx = px;
                    uint16_t tpy = py;
                    dual_shift_u16<3>(tpx, tpy);
                    sx = (int16_t)tpx - msx - 8;
                    sy = (int16_t)tpy - msy - 12;
                }
                //int16_t sx = px / 8 - msx - 8;
                //int16_t sy = py / 8 - msy - 12;
                platform_fx_drawplusmask(sx, sy, 16, 16, PLAYER_IMG, 0);
            }
        }

        {
            static_assert(32 == PAUSE_MAP_PIXELS_W / 16, "");
            static_assert(32 == PAUSE_MAP_PIXELS_H / 8, "");
            uint8_t x, y;
            {
                uint16_t tx = uint16_t(d.mapscrollx + (PAUSE_MAP_PIXELS_W / 16 / 2));
                uint16_t ty = uint16_t(d.mapscrolly + (PAUSE_MAP_PIXELS_H / 8 / 2));
                dual_shift_u16<5>(tx, ty);
                x = (uint8_t)tx;
                y = (uint8_t)ty;
            }
            platform_fillrect_i8(104, 48, 16, 8, LIGHT_GRAY);
            platform_fillrect_i8(104 + x, 48 + y, 4, 2, BLACK);
            platform_drawrect_i8(103, 47, 18, 10, DARK_GRAY);
        }
    }
    else if(d.state == OS_MAP)
        platform_fade(16 - d.mapfade);
}
