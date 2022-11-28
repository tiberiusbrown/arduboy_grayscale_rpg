#include "common.hpp"

#include "generated/fxdata.h"
#include "generated/num_game_over_messages.hpp"

static int8_t const DIRX[8] PROGMEM = {
    0, -1, -1, -1, 0, 1, 1, 1,
};
static int8_t const DIRY[8] PROGMEM = {
    1, 1, 0, -1, -1, -1, 0, 1,
};

void back_to_map()
{
    if(!(chunks_are_running && run_chunks()))
        change_state(STATE_MAP);
}

bool enemy_contacts_player(active_chunk_t const& c)
{
    auto const& e = c.enemy;
    if(!e.active) return false;
    uint16_t ex = c.cx * 128 + e.x;
    uint16_t ey = c.cy * 64 + e.y;
    // TODO: make these uint8_t
    uint16_t dx = px - ex + 12;
    uint16_t dy = py - ey + 12;
    return dx <= 24 && dy <= 24;
}

static inline void update_enemy(enemy_t& e)
{
    if(!e.active) return;
    if(nframe & 1) return;

    if(e.dir < 8)
    {
        e.x += (int8_t)pgm_read_byte(&DIRX[e.dir]);
        e.y += (int8_t)pgm_read_byte(&DIRY[e.dir]);
    }

    if(--e.frames_rem == 0)
    {
        if(!(e.dir & 0x80))
        {
            uint8_t t = e.path[e.path_index];
            if(t & 0xe0)
            {
                // delay
                e.frames_rem = (t >> 5) * 16;
                e.dir = 0xff;
                return;
            }
        }
        if(++e.path_index == e.path_num) e.path_index = 0;
        uint8_t t = e.path[e.path_index];
        uint8_t x = (t & 7) * 16;
        uint8_t y = ((t >> 3) & 3) * 16;
        if(x < e.x)
        {
            e.dir = 2;
            e.frames_rem = e.x - x;
        }
        else if(x > e.x) {
            e.dir = 6;
            e.frames_rem = x - e.x;
        }
        else if(y < e.y) {
            e.dir = 4;
            e.frames_rem = e.y - y;
        }
        else {
            e.dir = 0;
            e.frames_rem = y - e.y;
        }
    }
}

static void update_enemies()
{
    for(auto& c : active_chunks)
    {
        update_enemy(c.enemy);
    }
}

static void update_map()
{
    if(chunks_are_running && run_chunks()) return;

    if(btns_pressed & BTN_B)
    {
        change_state(STATE_PAUSE);
        return;
    }

    selx = sely = uint16_t(-1);
    if(btns_pressed & BTN_A)
    {
        int8_t dx = (int8_t)pgm_read_byte(&DIRX[pdir]) * 8;
        int8_t dy = (int8_t)pgm_read_byte(&DIRY[pdir]) * 8;
        selx = (px + 8 + dx) >> 4;
        sely = (py + 8 + dy) >> 4;
    }

    int8_t dx = 0, dy = 0;
    if(btns_down & BTN_UP) dy -= 1;
    if(btns_down & BTN_DOWN) dy += 1;
    if(btns_down & BTN_LEFT) dx -= 1;
    if(btns_down & BTN_RIGHT) dx += 1;

    pmoving = !(dx == 0 && dy == 0);

    if(pmoving)
    {
        if(dy < 0)
        {
            if(dx < 0) pdir = 3;
            else if(dx == 0) pdir = 4;
            else pdir = 5;
        }
        else if(dy == 0) {
            if(dx < 0) pdir = 2;
            else pdir = 6;
        }
        else {
            if(dx < 0) pdir = 1;
            else if(dx == 0) pdir = 0;
            else pdir = 7;
        }

        // movement
        px += dx;
        py += dy;

        uint8_t m = 0;
        int8_t nx = 0, ny = 0;
        if(tile_is_solid(px + 5, py + 5)) ++nx, ++ny, m |= 1;
        if(tile_is_solid(px + 11, py + 5)) --nx, ++ny, m |= 2;
        if(tile_is_solid(px + 5, py + 11)) ++nx, --ny, m |= 4;
        if(tile_is_solid(px + 11, py + 11)) --nx, --ny, m |= 8;

        if(nx > 1) nx = 1;
        if(nx < -1) nx = -1;
        if(ny > 1) ny = 1;
        if(ny < -1) ny = -1;
        if(nx == dx) nx = 0;
        if(ny == dy) ny = 0;

        // diagonal corrections: if the player is running into a corner
        if(m == 1 && pdir == 3)
        {
            if(tile_is_solid(px + 5, py + 6)) ny = 0;
            else nx = 0;
        }
        if(m == 2 && pdir == 5)
        {
            if(tile_is_solid(px + 11, py + 6)) ny = 0;
            else nx = 0;
        }
        if(m == 4 && pdir == 1)
        {
            if(tile_is_solid(px + 5, py + 10)) ny = 0;
            else nx = 0;
        }
        if(m == 8 && pdir == 7)
        {
            if(tile_is_solid(px + 11, py + 10)) ny = 0;
            else nx = 0;
        }

        // collision correction
        px += nx;
        py += ny;
    }

    load_chunks();
    if(run_chunks()) return;

    update_enemies();
}

static void skip_dialog_animation(uint8_t third_newline)
{
    auto& d = sdata.dialog;
    uint8_t i;
    for(i = 0; d.message[i] != '\0' && i != third_newline; ++i) {}
    d.char_progress = i;
}

static void advance_dialog_animation()
{
    auto& d = sdata.dialog;
    for(uint8_t i = 0; i < 2; ++i)
    {
        char c = d.message[d.char_progress];
        if(c != '\0') ++d.char_progress;
        if(c == '|')
        {
            // replace pipe char
            d.message[d.char_progress - 1] = '\0';
            // skip following newline
            MY_ASSERT(d.message[d.char_progress] == '\n');
            d.question_msg = ++d.char_progress;
            d.question = true;
            // count number of questions
            uint8_t n = 0, j = d.question_msg;
            for(uint8_t j = d.question_msg; (c = d.message[j]) != '\0'; ++j)
                if(c == '\n') ++n;
            d.numquestions = n;
            MY_ASSERT(n + 1 <= 3);
            return;
        }
    }
}

static void update_dialog()
{
    auto& d = sdata.dialog;
    uint8_t third_newline = 255;
    {
        uint8_t n = 0;
        for(uint8_t i = 0; i < sizeof(d.message) && d.message[i] != '\0'; ++i)
        {
            if(d.message[i] == '\n' && ++n == 3)
            {
                third_newline = i + 1;
                break;
            }
        }
    }
    char current_char = d.message[d.char_progress];
    bool message_done = (current_char == '\0');
    if(d.question)
    {
        d.questioniy = adjust(d.questioniy, d.questioni * 11);
        if(message_done)
        {
            d.questiondone = true;
            if(btns_down & BTN_A)
            {
                if(d.questionfill >= 32)
                {
                    chunk_regs[0] = 0;
                    chunk_regs[1] = 0;
                    chunk_regs[2] = 0;
                    chunk_regs[d.questioni + 1] = 1;
                    back_to_map();
                }
                else
                    ++d.questionfill;
            }
            else
            {
                d.questionfill = 0;
                if(btns_pressed & BTN_UP)
                {
                    if(d.questioni > 0)
                        --d.questioni;
                }
                else if(btns_pressed & BTN_DOWN)
                {
                    if(d.questioni < d.numquestions)
                        ++d.questioni;
                }
            }
        }
        else
        {
            advance_dialog_animation();
        }
        return;
    }
    if(btns_down & BTN_B) skip_dialog_animation(third_newline);
    if(btns_pressed & BTN_A)
    {
        if(d.char_progress == third_newline)
        {
            for(uint8_t i = 0;; ++i)
            {
                d.message[i] = d.message[i + third_newline];
                if(d.message[i] == '\0') break;
            }
            d.char_progress = 0;
        }
        else if(message_done)
        {
            if(!d.question)
                back_to_map();
        }
    }
    else
    {
        advance_dialog_animation();
        if(d.char_progress > third_newline) d.char_progress = third_newline;
    }
}

static void update_tp()
{
    auto& d = sdata.tp;
    ++d.frame;
    if(d.frame == TELEPORT_TRANSITION_FRAMES)
    {
        px = d.tx * 16;
        py = d.ty * 16;
        // terminate old chunk scripts
        chunks_are_running = false;
        load_chunks();
        run_chunks();
    }
    if(d.frame == TELEPORT_TRANSITION_FRAMES * 2) change_state(STATE_MAP);
}

static void update_game_over()
{
    auto& d = sdata.game_over;
    if(d.going_to_resume)
    {
        if(d.fade_frame < 24)
            d.fade_frame += FADE_SPEED;
        else
        change_state(STATE_RESUME);
    }
    else
    {
    if(d.msg[0] == '\0')
    {
        uint8_t n = u8rand(NUM_GAME_OVER_MESSAGES);
        platform_fx_read_data_bytes(
            GAME_OVER_MESSAGES + n * GAME_OVER_MESSAGE_LEN, d.msg, 128);
        wrap_text(d.msg, 106);
        d.msg_lines = 1;
        for(auto& c : d.msg)
            if(c == '\n') ++d.msg_lines, c = '\0';
    }
    if(d.fade_frame < 24)
        d.fade_frame += FADE_SPEED;
    else if(btns_pressed & (BTN_A | BTN_B))
        d.fade_frame = 8, d.going_to_resume = true;
    }
}

#if RECORD_LIPO_DISCHARGE
#include <hardwareSerial.h>
#endif

static void update_title()
{
    static bool first_loaded = false;
    auto& d = sdata.title;
    if(d.going_to_resume)
    {
        if(d.fade_frame < 16)
            d.fade_frame += FADE_SPEED;
        else
            change_state(STATE_RESUME);
    }
    else
    {
        if(d.fade_frame == 0)
        {
            load(!first_loaded);
            first_loaded = true;
        }
        if(d.fade_frame < 24)
            d.fade_frame += FADE_SPEED;
        else if(btns_pressed & BTN_A)
            d.fade_frame = 0, d.going_to_resume = true;
    }
}

static void update_resume()
{
    auto& d = sdata.resume;
    if(d.fade_frame == 0)
    {
        load_chunks();
        run_chunks();
    }
    if(d.fade_frame < 24)
        d.fade_frame += FADE_SPEED;
    else
        change_state(STATE_MAP);
}

void update()
{
    using update_func = void (*)();
    static update_func const FUNCS[] PROGMEM = {
        update_title,
        update_resume,
        update_map,
        update_pause,
        update_dialog,
        update_tp,
        update_battle,
        update_game_over,
    };

    pmoving = false;
    (pgmptr(&FUNCS[state]))();

    update_battery();

    ++nframe;

#if DEBUG_LIPO_DISCHARGE
    if((nframe & 0x7ff) == 0x7ff)
    {
        uint8_t fade = u8rand() & 15;
        if(fade == 0) fade = 1;
        platform_fade(fade);
    }
#endif
#if RECORD_LIPO_DISCHARGE
    if(state == STATE_TITLE) && (btns_pressed & BTN_B))
    {
        int16_t v[10];
        for(uint24_t i = 0; i < 7200 * 2; i += 2)
        {
            FX::readSaveBytes(i, (uint8_t*)v, 2);
            for(uint8_t i = 0; i < 1; ++i)
            {
                Serial.print(v[i]);
                Serial.print(' ');
            }
            Serial.print('\n');
        }
    }
#endif
}
