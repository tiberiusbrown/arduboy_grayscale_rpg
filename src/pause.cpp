#include "common.hpp"

#include "generated/fxdata.h"

static uint8_t const OPTION_X[] PROGMEM =
{
    5, 25, 32, 54, 61, 96, 103, 121,
};

void update_pause()
{
    auto& d = sdata.pause;
    if(btns_pressed & BTN_B)
    {
        if(d.state == OS_MENU) d.state = OS_RESUMING;
        else if(d.state == OS_OPTIONS) d.state = OS_MENU;
    }
    if(d.state == OS_RESUMING)
    {
        if((d.ally | d.menuy) == 0)
        {
            change_state(STATE_MAP);
            return;
        }
    }
    else if(d.state == OS_MENU)
    {
        if(d.menuy >= 16 && d.ally == 0)
        {
            uint8_t menui = d.menui;
            if((btns_pressed & BTN_LEFT) && menui-- == 0)
                menui = 3;
            if((btns_pressed & BTN_RIGHT) && menui++ == 3)
                menui = 0;
            if(btns_pressed & BTN_A)
            {
                if(menui == 0) d.state = OS_SAVE;
                if(menui == 1) d.state = OS_PARTY;
                if(menui == 2) d.state = OS_OPTIONS;
                if(menui == 3) d.state = OS_QUIT;
            }
            d.menui = menui;
        }
    }
    else if(d.state == OS_OPTIONS)
    {
        if(d.optionsy >= 64)
        {
            uint8_t optionsi = d.optionsi;
            if(btns_pressed & BTN_B)
                d.state = OS_MENU;
            else if((btns_pressed & BTN_UP) && optionsi-- == 0)
                optionsi = 4;
            else if((btns_pressed & BTN_DOWN) && optionsi++ == 4)
                optionsi = 0;
            else if(btns_pressed & (BTN_A | BTN_LEFT | BTN_RIGHT))
            {
                if(optionsi == 0)
                {
                    savefile.settings.sound ^= 2;
                    platform_audio_update();
                }
                else if(optionsi == 1)
                {
                    savefile.settings.sound ^= 1;
                    platform_audio_update();
                }
                else if(optionsi == 2)
                {
                    if(savefile.settings.game_speed < 6 && (btns_pressed & (BTN_A | BTN_RIGHT)))
                        ++savefile.settings.game_speed;
                    if(savefile.settings.game_speed > 0 && (btns_pressed & BTN_LEFT))
                        --savefile.settings.game_speed;
                }
                else if(optionsi == 3)
                {
                    if(savefile.settings.brightness < 3 && (btns_pressed & (BTN_A | BTN_RIGHT)))
                        ++savefile.settings.brightness;
                    if(savefile.settings.brightness > 0 && (btns_pressed & BTN_LEFT))
                        --savefile.settings.brightness;
                    platform_fade(15);
                }
                else if(optionsi == 4)
                    savefile.settings.no_battery_alert = !savefile.settings.no_battery_alert;
            }
            d.optionsi = optionsi;
        }
    }
    else if(d.state == OS_QUIT)
    {
        if(d.quitfade > 0)
        {
            d.quitfade += FADE_SPEED;
            if(d.quitfade >= 16 * FADE_SPEED + 16)
            {
                d.quitfade = d.quitf = d.quitft = 0;
#ifdef ARDUINO
                    if(d.quiti == 0) a.exitToBootloader();
#else
                    if(d.quiti == 0) d.quiti = 1;
#endif
                    if(d.quiti == 1)
                    {
                        change_state(STATE_TITLE);
                        return;
                    }
                    if(d.quiti == 2)
                    {
                        change_state(STATE_RESUME);
                        new_game();
                        return;
                    }
            }
        }
        else if(d.quity >= 64)
        {
            if(btns_pressed & BTN_A)
                d.quitp = true;
            if(d.quitp && (btns_down & BTN_A))
            {
                if((d.quitft += 3) >= 96)
                {
                    d.quitft = 96;
                    d.quitfade = 1;
                }
            }
            else
            {
                d.quitft = 0;
                if(btns_pressed & BTN_B)
                    d.state = OS_MENU;
                if((btns_pressed & BTN_UP) && d.quiti-- == 0)
                    d.quiti = 2;
                if((btns_pressed & BTN_DOWN) && d.quiti++ == 2)
                    d.quiti = 0;
            }
        }
    }
    else if(d.state == OS_SAVE)
    {
        if(d.savey >= 64)
        {
            if(d.save_wait > 0)
            {
                if(++d.save_wait == 16)
                    d.state = OS_RESUMING;
            }
            else if(is_saving())
            {
                if(save_done())
                    d.save_wait = 1;
            }
            else
            {
                save_begin();
            }
        }
    }
    else if(d.state == OS_PARTY)
    {
        update_pause_party();
    }
    adjust(d.menuy, d.state == OS_MENU ? 16 : 0);
    adjust(d.optionsy, d.state == OS_OPTIONS ? 64 : 0);
    adjust(d.quity, d.state == OS_QUIT ? 64 : 0);
    adjust(d.quitf, d.quitft);
    adjust(d.savey, d.state == OS_SAVE ? 64 : 0);
    adjust(d.partyy, d.state == OS_PARTY ? 64 : 0);
    {
        uint8_t const* ptr = &OPTION_X[uint8_t(d.menui * 2)];
        uint8_t ax = pgm_read_byte_inc(ptr);
        uint8_t bx = pgm_read_byte(ptr);
        if(d.ax == 0) d.ax = ax, d.bx = bx;
        adjust(d.ax, ax);
        adjust(d.bx, bx);
    }
    adjust(d.optionsiy, d.optionsi * 12);
    adjust(d.quitiy, d.quiti * 13);
    adjust(d.brightnessx, savefile.settings.brightness * 16);
    adjust(d.speedx, savefile.settings.game_speed * 8);
    d.ally = d.optionsy | d.quity | d.savey | d.partyy;
}

void render_pause()
{
    auto const& d = sdata.pause;
    // render darkened map (exclude plane 0)
    if((d.optionsy | d.quity | d.savey | d.partyy) < 64 &&
        (d.state == OS_RESUMING || plane() > 0))
        render_map();
    if(d.menuy > 0)
    {
        platform_fx_drawoverwrite(0, d.menuy - 16, PAUSE_MENU_IMG);
        platform_fillrect_i8(d.ax, d.menuy - 5, (d.bx - d.ax + 1), 2, WHITE);
    }
    if(d.optionsy > 0)
    {
        uint8_t y = 64 - d.optionsy;
        platform_fx_drawoverwrite(0, y, OPTIONS_IMG);
        platform_fx_drawoverwrite(d.brightnessx + 70, y + 38, SLIDER_IMG);
        platform_fx_drawoverwrite(d.speedx + 70, y + 26, SLIDER_IMG);

        if(savefile.settings.sound & 2)
            platform_fx_drawoverwrite(71, y + 4, CHECK_IMG);
        if(savefile.settings.sound & 1)
            platform_fx_drawoverwrite(71, y + 16, CHECK_IMG);
        if(!savefile.settings.no_battery_alert)
            platform_fx_drawoverwrite(71, y + 52, CHECK_IMG);
        if(plane() == 0)
            platform_drawrect_i8(0, y + d.optionsiy + 2, 128, 12, DARK_GRAY);
    }
    if(d.quity > 0)
    {
        uint8_t y = 64 - d.quity;
        platform_fx_drawoverwrite(0, y, QUIT_IMG);
        if(plane() == 0)
        {
            int8_t qy = (int8_t)y + d.quitiy + 25;
            platform_drawrect_i8(16, qy, 96, 12, DARK_GRAY);
            platform_fillrect_i8(16, qy, d.quitf, 12, DARK_GRAY);
        }
        if(d.quitfade > 16 * FADE_SPEED)
            platform_fade(16 * FADE_SPEED + 15 - d.quitfade);
    }
    if(d.savey > 0)
    {
        uint8_t y = 64 - d.savey;
        platform_fillrect_i8(0, (int8_t)y, 128, 64, BLACK);
        draw_text_noclip(39, y + 28, PSTR("Saving..."), NOCLIPFLAG_PROG);
        if(d.save_wait > 0)
            draw_text_noclip(71, y + 28, PSTR("Done!"), NOCLIPFLAG_PROG);
    }
    if(d.partyy > 0)
    {
        render_pause_party();
    }
}
