#include "common.hpp"

#include "generated/fxdata.h"

void render_map()
{
    draw_tiles();
    draw_sprites();
}

static void render_dialog()
{
    auto& d = sdata.dialog;
    char c;
    bool portrait = (d.portrait != 255);

    if(!d.questiondraw)
    {
        render_map();
        platform_fillrect(0, 36, 128, 28, BLACK);
        if(portrait)
            platform_fillrect(0, 0, 35, 35, BLACK);
    }
    else
    {
        uint8_t x = portrait ? 39 : 2;
        c = d.message[d.char_progress];
        d.message[d.char_progress] = '\0';
        draw_text_noclip(x, 2, &d.message[d.question_msg], true);
        d.message[d.char_progress] = c;
        if(d.questiondone && plane() == 0)
        {
            uint8_t w = 128 - x;
            uint8_t f = (d.questionfill * w + 16) >> 5;
            platform_drawrect(x - 2, d.questioniy, w + 2, 12, DARK_GRAY);
            platform_fillrect(x - 1, d.questioniy + 1, f, 10, DARK_GRAY);
        }
    }
    platform_fillrect(0, 35, 128, 1, LIGHT_GRAY);

    if(portrait)
    {
        platform_fx_drawoverwrite(2, 2, PORTRAIT_IMG, d.portrait);
        platform_drawrect(0, 0, 36, 36, LIGHT_GRAY);
    }

    c = d.message[d.char_progress];
    d.message[d.char_progress] = '\0';
    draw_text_noclip(0, 37, d.message);
    d.message[d.char_progress] = c;
}

static void render_tp()
{
    auto const& d = sdata.tp;
    uint8_t t = d.frame;
    if(t > TELEPORT_TRANSITION_FRAMES) t = TELEPORT_TRANSITION_FRAMES * 2 - t;
    constexpr uint8_t XB = 64 / TELEPORT_TRANSITION_FRAMES;
    constexpr uint8_t YB = 32 / TELEPORT_TRANSITION_FRAMES;
    if(t != TELEPORT_TRANSITION_FRAMES)
    {
        uint8_t w = XB * t;
        uint8_t h = YB * t;
        uint8_t h2 = 64 - h * 2;
        draw_tiles();
        draw_sprites();
        platform_fillrect(0, 0, 128, h, BLACK);
        platform_fillrect(0, 64 - h, 128, h, BLACK);
        platform_fillrect(0, h, w, h2, BLACK);
        platform_fillrect(128 - w, h, w, h2, BLACK);
    }
    draw_player();
}

static void render_game_over()
{
    auto const& d = sdata.game_over;

    if(d.fade_frame < 8) return;
    platform_fx_drawoverwrite(0, 0, GAME_OVER_IMG, 0);

    uint8_t n = d.msg_lines;
    uint8_t y = 39 - n * 4;
    char const* t = d.msg;
    while(n != 0)
    {
        uint8_t w = text_width(t);
        draw_text(64 - w / 2, y, t);
        y += 9;
        while(*t++ != '\0')
            ;
        --n;
    }

    if(d.going_to_resume)
        platform_fade(24 - d.fade_frame);
    else if(d.fade_frame < 24)
        platform_fade(d.fade_frame - 8);
}

static void render_title()
{
    auto const& d = sdata.title;
    if(d.going_to_resume)
    {
        if(d.fade_frame < 16)
        {
            platform_fx_drawoverwrite(0, 0, TITLE_IMG, 0);
            platform_fade(15 - d.fade_frame);
        }
        else if(d.fade_frame >= 24)
        {
            render_map();
            platform_fade(d.fade_frame - 24);
        }
    }
    else
    {
        if(d.fade_frame < 8) return;
        platform_fx_drawoverwrite(0, 0, TITLE_IMG, 0);
        if(d.fade_frame < 24)
            platform_fade(d.fade_frame - 8);
    }
}

static void render_resume()
{
    auto const& d = sdata.resume;
    if(d.fade_frame < 8) return;
    render_map();
    platform_fade(d.fade_frame - 8);
}

#if DEBUG_LIPO_DISCHARGE
static void uint16_to_str(char* str, uint16_t v)
{
    uint8_t n;
    for(n = 0; v >= 10000; ++n, v -= 10000);
    str[0] = '0' + n;
    for(n = 0; v >= 1000; ++n, v -= 1000);
    str[1] = '0' + n;
    for(n = 0; v >= 100; ++n, v -= 100);
    str[2] = '0' + n;
    for(n = 0; v >= 10; ++n, v -= 10);
    str[3] = '0' + n;
    str[4] = '0' + (uint8_t)v;
    str[5] = '\0';
}
static void draw_uint16(uint8_t x, uint8_t y, uint16_t v)
{
    char str[6];
    uint16_to_str(str, v);
    draw_text(x, y, str);
}
static void draw_int16(uint8_t x, uint8_t y, int16_t v)
{
    char str[7];
    if(v < 0)
    {
        str[0] = '-';
        uint16_to_str(&str[1], uint16_t(-v));
    }
    else
    {
        str[0] = '+';
        uint16_to_str(&str[1], uint16_t(v));
    }
    draw_text(x, y, str);
}
#endif

static void render_battery()
{
#if DEBUG_LIPO_DISCHARGE
    draw_int16(0,  0, bat.raw);
    draw_int16(0,  8, bat.r);
    draw_int16(0, 16, bat.dr);
    draw_int16(0, 24, bat.ddr);
    draw_uint16(0, 36, bat.stage);
#endif

    if(!savefile.no_battery_alert && bat.low_battery)
    {
        uint8_t f = (nframe & 0x10) ? 0 : 1;
        platform_fx_drawplusmask(118, 0, BATTERY_IMG, f, 10, 8);
    }
}

void render()
{
    using render_func = void (*)();
    static render_func const FUNCS[] PROGMEM = {
        render_title,
        render_resume,
        render_map,
        render_pause,
        render_dialog,
        render_tp,
        render_battle,
        render_game_over,
    };

    (pgmptr(&FUNCS[state]))();

    render_battery();
}
