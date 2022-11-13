#include "common.hpp"

#include "generated/fxdata.h"

void render_map()
{
    draw_tiles();
    draw_sprites();
}

static void render_dialog()
{
    render_map();

    auto& d = sdata.dialog;
    if(d.portrait != 255)
    {
        platform_fillrect(0, 2, 33, 33, BLACK);
        platform_fx_drawoverwrite(0, 3, PORTRAIT_IMG, d.portrait, 32, 32);
    }

    platform_fillrect(0, 35, 128, 28, BLACK);
    char c = d.message[d.char_progress];
    d.message[d.char_progress] = '\0';
    draw_text(0, 37, d.message);
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
    platform_fx_drawoverwrite(0, 0, GAME_OVER_IMG, 0, 128, 64);

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

    if(d.fade_frame < 24)
        platform_fade(d.fade_frame - 8);
}

void render()
{
    using render_func = void (*)();
    static render_func const FUNCS[] PROGMEM = {
        render_map,
        render_dialog,
        render_tp,
        render_battle,
        render_game_over,
    };

    (pgmptr(&FUNCS[state]))();
}
