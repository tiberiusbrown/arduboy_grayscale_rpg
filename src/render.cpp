#include "common.hpp"

#include "generated/fxdata.h"

void render_map()
{
    draw_tiles();
    draw_sprites();
}

static void render_map_and_objective()
{
    render_map();
    if(sdata.map.a_pressed >= 32 && (btns_down & BTN_A))
    {
        if(!player_is_outside()) return;

        uint8_t objx = savefile.objx;
        uint8_t objy = savefile.objy;
        if((objx | objy) == 0) return;

        static_assert(MAP_CHUNK_COLS <= 32, "expand calculations to 16-bit");
        static_assert(MAP_CHUNK_ROWS <= 64, "expand calculations to 16-bit");

        // direction to objective
        draw_objective(objx * 16 - px, objy * 16 - py);
    }
}

static void render_dialog()
{
    auto& d = sdata.dialog;
    char c;
    uint8_t portrait_id = (uint8_t)d.name[0];
    bool portrait = (portrait_id < 254);

    if(!d.questiondraw)
    {
        render_map();
        platform_fillrect_i8(0, 36, 128, 28, BLACK);
    }
    else
    {
        uint8_t x = portrait ? 39 : 2;
        c = d.message[d.char_progress];
        d.message[d.char_progress] = '\0';
        draw_text_noclip(x, 2, &d.message[d.question_msg], NOCLIPFLAG_BIGLINES);
        d.message[d.char_progress] = c;
        if(d.questiondone && plane() == 0)
        {
            uint8_t w = 128 - x;
            uint8_t f = (d.questionfillw * w + 128) >> 8;
            platform_drawrect_i8(x - 2, d.questioniy, w + 2, 12, DARK_GRAY);
            platform_fillrect_i8(x - 1, d.questioniy + 1, f, 10, DARK_GRAY);
        }
    }
    platform_fillrect_i8(0, 35, 128, 1, LIGHT_GRAY);

    if(portrait)
    {
        uint8_t p = portrait_id;
        uint8_t x = 0;
        if(p >= 0x80)
            p -= 0x80, x = 48;
        platform_fx_drawoverwrite(x + 2, 2, PORTRAIT_IMG, p);
        platform_drawrect_i8(x + 0, 0, 36, 36, LIGHT_GRAY);
        platform_drawrect_i8(x + 1, 1, 34, 34, BLACK);
        if(!d.questiondraw && portrait_id < 0x80)
        {
            uint8_t w = text_width(&d.name[1]);
            uint8_t x = 35;
            uint8_t y = 0;
            platform_fillrect_i8(x, y, w + 4, 12, BLACK);
            platform_drawrect_i8(x, y, w + 4, 12, LIGHT_GRAY);
            draw_text_noclip(x + 2, y + 2, &d.name[1]);
        }
    }
    else if(portrait_id == 254)
    {
        platform_fx_drawoverwrite(33, 16, GOT_ITEM_IMG);
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
        render_map();
        platform_fillrect_i8(0, 0, 128, h, BLACK);
        platform_fillrect_i8(0, 64 - h, 128, h, BLACK);
        platform_fillrect_i8(0, h, w, h2, BLACK);
        platform_fillrect_i8(128 - w, h, w, h2, BLACK);
    }
    draw_player();
}

static void render_game_over()
{
    auto const& d = sdata.game_over;

    if(d.fade_frame < 8) return;
    platform_fx_drawoverwrite(uint8_t(0), uint8_t(0), GAME_OVER_IMG);

    uint8_t n = d.msg_lines;
    uint8_t y = 39 - n * 4;
    char const* t = d.msg;
    while(n != 0)
    {
        uint8_t w = text_width(t);
        draw_text_noclip(64 - w / 2, y, t);
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

static void render_title_graphics()
{
    //platform_fx_drawoverwrite(0, 0, TITLE_IMG);
    auto const& d = sdata.title;
    render_map();
    platform_fx_drawplusmask(7, 0, 114, 16, TITLE_MASKED_IMG, 0);
    if(!d.going_to_resume && (nframe & 0x3f) < 0x30)
        platform_fx_drawplusmask(47, 48, 34, 16, PRESS_A_IMG, 0);
}

static void render_title()
{
    auto const& d = sdata.title;
#if DETECT_FX_CHIP
    if(d.no_fx_chip)
    {
        static char const ROTA[] PROGMEM = "Return of the Ardu";
        static char const NO_FX_CHIP[] PROGMEM = "NO FLASH CHIP DETECTED";
        draw_text_noclip(31, 16, ROTA, NOCLIPFLAG_PROG);
        draw_text_noclip(12, 40, NO_FX_CHIP, NOCLIPFLAG_PROG);
        platform_fade(15);
        return;
    }
#endif

    uint8_t ff = d.fade_frame;

    if(d.going_to_resume)
    {
        if(ff >= 16)
            return;
        ff = 15 - ff;
    }
    else
    {
        if(d.fade_frame < 8) return;
        ff -= 8;
    }

    render_title_graphics();
    platform_fade(ff);
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

    if(!savefile.settings.no_battery_alert && battery.low)
    {
        uint8_t f = (rframe & 0x20) ? 0 : 1;
        platform_fx_drawplusmask(118, 0, 10, 8, BATTERY_IMG, f);
    }
}

static void render_die()
{
    auto const& d = sdata.die;
    render_map();
    uint8_t f = d.frame;
    if(f >= 48)
        platform_fade(16 + (48 - f) * FADE_SPEED);
}

void render()
{
    using render_func = void (*)();
    static render_func const FUNCS[] PROGMEM = {
        render_title,
        render_resume,
        render_map_and_objective,
        render_pause,
        render_dialog,
        render_tp,
        render_battle,
        render_die,
        render_game_over,
    };

    (pgmptr(&FUNCS[state]))();

    update_battery();
    render_battery();

    if(plane() == 0)
        ++rframe;
}
