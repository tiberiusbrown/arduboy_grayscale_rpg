#include "common.hpp"

#include "generated/fxdata.h"

enum
{
    OS_MENU,
    OS_RESUMING,
    OS_OPTIONS,
    OS_QUIT,
    OS_SAVE,
};

static uint8_t const OPTION_X[] PROGMEM =
{
    5, 25, 32, 54, 61, 96, 103, 121,
};

static uint8_t adjust(uint8_t x, uint8_t tx)
{
    if(x < tx)
        x += (uint8_t(tx - x + 1) >> 1);
    else
        x -= (uint8_t(x - tx + 1) >> 1);
    return x;
}

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
        if((d.menuy | d.optionsy | d.quity | d.savey) == 0)
        {
            change_state(STATE_MAP);
            return;
        }
    }
    else if(d.state == OS_MENU)
    {
        if(d.menuy >= 16 && d.optionsy == 0 && d.quity == 0)
        {
            uint8_t menui = d.menui;
            if((btns_pressed & BTN_LEFT) && menui-- == 0)
                menui = 3;
            if((btns_pressed & BTN_RIGHT) && menui++ == 3)
                menui = 0;
            if(btns_pressed & BTN_A)
            {
                if(menui == 0) d.state = OS_SAVE;
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
                optionsi = 2;
            else if((btns_pressed & BTN_DOWN) && optionsi++ == 2)
                optionsi = 0;
            else if(btns_pressed & (BTN_A | BTN_LEFT | BTN_RIGHT))
            {
                if(optionsi == 0)
                {
                    if(savefile.music_volume < 2 && (btns_pressed & (BTN_A | BTN_RIGHT)))
                        ++savefile.music_volume;
                    if(savefile.music_volume > 0 && (btns_pressed & BTN_LEFT))
                        --savefile.music_volume;
                    if(savefile.music_volume > 0)
                        platform_audio_on();
                    else
                        platform_audio_off();
                }
                else if(optionsi == 2)
                    savefile.no_battery_alert = !savefile.no_battery_alert;
                else if(optionsi == 1)
                {
                    if(savefile.brightness < 3 && (btns_pressed & (BTN_A | BTN_RIGHT)))
                        ++savefile.brightness;
                    if(savefile.brightness > 0 && (btns_pressed & BTN_LEFT))
                        --savefile.brightness;
                    platform_fade(15);
                }
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
                        // change to title just for the fade in effect
                        change_state(STATE_TITLE);
                        new_game();
                        sdata.title.fade_frame = 16;
                        sdata.title.going_to_resume = true;
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
    if(d.state == OS_MENU)
        d.menuy += uint8_t(17 - d.menuy) >> 1;
    else
        d.menuy >>= 1;
    if(d.state == OS_OPTIONS)
        d.optionsy += uint8_t(65 - d.optionsy) >> 1;
    else
        d.optionsy >>= 1;
    if(d.state == OS_QUIT)
        d.quity += uint8_t(65 - d.quity) >> 1;
    else
        d.quity >>= 1, d.quitp = false, d.quitfade = 0;
    d.quitf = adjust(d.quitf, d.quitft);
    if(d.state == OS_SAVE)
        d.savey += uint8_t(65 - d.savey) >> 1;
    else
        d.savey >>= 1;
    {
        uint8_t ax = pgm_read_byte(&OPTION_X[d.menui * 2 + 0]);
        uint8_t bx = pgm_read_byte(&OPTION_X[d.menui * 2 + 1]);
        if(d.ax == 0) d.ax = ax, d.bx = bx;
        d.ax = adjust(d.ax, ax);
        d.bx = adjust(d.bx, bx);
    }
    {
        uint8_t ty = d.optionsi * 16 + 17;
        if(d.optionsiy == 0) d.optionsiy = ty;
        d.optionsiy = adjust(d.optionsiy, ty);
    }
    {
        uint8_t ty = d.quiti * 13 + 25;
        if(d.quitiy == 0) d.quitiy = ty;
        d.quitiy = adjust(d.quitiy, ty);
    }
    {
        uint8_t tx = savefile.music_volume * 24 + 70;
        if(d.musicx == 0) d.musicx = tx;
        d.musicx = adjust(d.musicx, tx);
    }
    {
        uint8_t tx = savefile.brightness * 16 + 70;
        if(d.brightnessx == 0) d.brightnessx = tx;
        d.brightnessx = adjust(d.brightnessx, tx);
    }
}

void render_pause()
{
    auto const& d = sdata.pause;
    // render darkened map (exclude plane 0)
    if((d.optionsy | d.quity | d.savey) < 64 &&
        (d.state == OS_RESUMING || plane() > 0))
        render_map();
    if(d.menuy > 0)
    {
        platform_fx_drawoverwrite(0, d.menuy - 16, PAUSE_MENU_IMG, 0, 128, 16);
        platform_fillrect(d.ax, d.menuy - 5, (d.bx - d.ax + 1), 2, WHITE);
    }
    if(d.optionsy > 0)
    {
        int16_t y = 64 - d.optionsy;
        platform_fx_drawoverwrite(0, y, OPTIONS_IMG, 0, 128, 64);
        platform_fx_drawoverwrite(d.musicx, y + 17, SLIDER_IMG, 0, 7, 8);
        platform_fx_drawoverwrite(d.brightnessx, y + 33, SLIDER_IMG, 0, 7, 8);
        if(!savefile.no_battery_alert)
            platform_fx_drawoverwrite(71, y + 52, CHECK_IMG, 0, 8, 8);
        if(plane() == 0)
            platform_drawrect(1, y + d.optionsiy, 126, 14, DARK_GRAY);
    }
    if(d.quity > 0)
    {
        int16_t y = 64 - d.quity;
        platform_fx_drawoverwrite(0, y, QUIT_IMG, 0, 128, 64);
        if(plane() == 0)
        {
            platform_drawrect(16, y + d.quitiy, 96, 12, DARK_GRAY);
            platform_fillrect(16, y + d.quitiy, d.quitf, 12, DARK_GRAY);
        }
        if(d.quitfade > 16 * FADE_SPEED)
            platform_fade(16 * FADE_SPEED + 15 - d.quitfade);
    }
    if(d.savey > 0)
    {
        int16_t y = 64 - d.savey;
        platform_fillrect(0, y, 128, 64, BLACK);
        static char const SAVE_MSG[] PROGMEM = "Saving...";
        draw_text_prog(39, y + 28, SAVE_MSG);
        static char const DONE_MSG[] PROGMEM = "Done!";
        if(d.save_wait > 0)
            draw_text_prog(71, y + 28, DONE_MSG);
    }
}
