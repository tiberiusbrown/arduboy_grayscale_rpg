#include "common.hpp"

#include "generated/fxdata.h"

enum
{
    OS_MENU,
    OS_RESUMING,
    OS_OPTIONS,
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
        if(d.menuy == 0)
        {
            change_state(STATE_MAP);
            return;
        }
    }
    else if(d.state == OS_MENU)
    {
        if(d.menuy >= 16)
        {
            if((btns_pressed & BTN_LEFT) && d.menui-- == 0)
                d.menui = 3;
            if((btns_pressed & BTN_RIGHT) && d.menui++ == 3)
                d.menui = 0;
            if(btns_pressed & BTN_A)
            {
                if(d.menui == 2) d.state = OS_OPTIONS;
            }
        }
    }
    else if(d.state == OS_OPTIONS)
    {
        if(d.optionsy >= 64)
        {
            if(btns_pressed & BTN_B)
                d.state = OS_MENU;
            if((btns_pressed & BTN_UP) && d.optionsi-- == 0)
                d.optionsi = 2;
            if((btns_pressed & BTN_DOWN) && d.optionsi++ == 2)
                d.optionsi = 0;
            if(btns_pressed & (BTN_A | BTN_RIGHT))
            {
                if(d.optionsi == 0) savefile.no_music ^= 1;
                if(d.optionsi == 2) savefile.no_battery_alert ^= 1;
                if(d.optionsi == 1)
                    savefile.brightness = (savefile.brightness + 1) & 3;
            }
            if(btns_pressed & BTN_LEFT)
            {
                if(d.optionsi == 0) savefile.no_music ^= 1;
                if(d.optionsi == 2) savefile.no_battery_alert ^= 1;
                if(d.optionsi == 1)
                    savefile.brightness = (savefile.brightness - 1) & 3;
            }
            if(d.optionsi == 1 && btns_pressed & (BTN_A | BTN_LEFT | BTN_RIGHT))
                platform_fade(15);
        }
    }
    if(d.state == OS_OPTIONS)
        d.optionsy += uint8_t(65 - d.optionsy) >> 1;
    else
        d.optionsy >>= 1;
    if(d.state == OS_MENU)
        d.menuy += uint8_t(17 - d.menuy) >> 1;
    else
        d.menuy >>= 1;
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
        platform_drawrect(1, y + d.optionsiy, 126, 14, DARK_GRAY);
    }
}
