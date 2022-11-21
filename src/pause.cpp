#include "common.hpp"

#include "generated/fxdata.h"

enum
{
    OS_MENU,
    OS_RESUMING,
    OS_OPTIONS,
    OS_QUIT,
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
        if((d.menuy | d.optionsy | d.quity) == 0)
        {
            change_state(STATE_MAP);
            return;
        }
    }
    else if(d.state == OS_MENU)
    {
        if(d.menuy >= 16 && d.optionsy == 0 && d.quity == 0)
        {
            if((btns_pressed & BTN_LEFT) && d.menui-- == 0)
                d.menui = 3;
            if((btns_pressed & BTN_RIGHT) && d.menui++ == 3)
                d.menui = 0;
            if(btns_pressed & BTN_A)
            {
                if(d.menui == 2) d.state = OS_OPTIONS;
                if(d.menui == 3) d.state = OS_QUIT;
            }
        }
    }
    else if(d.state == OS_OPTIONS)
    {
        if(d.optionsy >= 64)
        {
            if(btns_pressed & BTN_B)
                d.state = OS_MENU;
            else if((btns_pressed & BTN_UP) && d.optionsi-- == 0)
                d.optionsi = 2;
            else if((btns_pressed & BTN_DOWN) && d.optionsi++ == 2)
                d.optionsi = 0;
            else if(btns_pressed & (BTN_A | BTN_LEFT | BTN_RIGHT))
            {
                if(d.optionsi == 0) savefile.no_music ^= 1;
                else if(d.optionsi == 2) savefile.no_battery_alert ^= 1;
                else if(d.optionsi == 1)
                {
                    if(btns_pressed & (BTN_A | BTN_RIGHT))
                        ++savefile.brightness;
                    if(btns_pressed & BTN_LEFT)
                        --savefile.brightness;
                    savefile.brightness &= 3;
                    platform_fade(15);
                }
            }
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
                if((d.quitft += 2) >= 96)
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
        uint8_t tx = savefile.brightness * 16 + 70;
        if(d.sliderx == 0) d.sliderx = tx;
        d.sliderx = adjust(d.sliderx, tx);
    }
}

void render_pause()
{
    auto const& d = sdata.pause;
    // render darkened map (exclude plane 0)
    if(d.optionsy < 64 && (d.state == OS_RESUMING || plane() > 0))
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
        if(!savefile.no_music)
            platform_fx_drawoverwrite(71, y + 20, CHECK_IMG, 0, 8, 8);
        if(!savefile.no_battery_alert)
            platform_fx_drawoverwrite(71, y + 52, CHECK_IMG, 0, 8, 8);
        platform_fx_drawoverwrite(d.sliderx, y + 33, SLIDER_IMG, 0, 7, 8);
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
            platform_fade(15 - d.quitfade);
    }
}
