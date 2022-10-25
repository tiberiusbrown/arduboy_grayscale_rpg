#include "common.hpp"

static int8_t const DIRX[8] PROGMEM = {
    0, -1, -1, -1, 0, 1, 1, 1,
};
static int8_t const DIRY[8] PROGMEM = {
    1, 1, 0, -1, -1, -1, 0, 1,
};

static void update_map()
{
    if(chunks_are_running && run_chunks()) return;

    selx = sely = uint16_t(-1);
    if(btns_pressed & BTN_A) {
        int8_t dx = (int8_t)pgm_read_byte(&DIRX[pdir]) * 8;
        int8_t dy = (int8_t)pgm_read_byte(&DIRY[pdir]) * 8;
        selx = (px + dx) >> 4;
        sely = (py + dy + 4) >> 4;
    }

    int8_t dx = 0, dy = 0;
    if(btns_down & BTN_UP) dy -= 1;
    if(btns_down & BTN_DOWN) dy += 1;
    if(btns_down & BTN_LEFT) dx -= 1;
    if(btns_down & BTN_RIGHT) dx += 1;

    pmoving = !(dx == 0 && dy == 0);

    if(pmoving) {
        if(dy < 0) {
            if(dx < 0) pdir = 3;
            else if(dx == 0) pdir = 4;
            else pdir = 5;
        } else if(dy == 0) {
            if(dx < 0) pdir = 2;
            else pdir = 6;
        } else {
            if(dx < 0) pdir = 1;
            else if(dx == 0) pdir = 0;
            else pdir = 7;
        }

        // movement
        px += dx;
        py += dy;

        int8_t nx = 0, ny = 0;
        if(tile_is_solid(px - 3, py + 1)) ++nx, ++ny;
        if(tile_is_solid(px + 3, py + 1)) --nx, ++ny;
        if(tile_is_solid(px - 3, py + 7)) ++nx, --ny;
        if(tile_is_solid(px + 3, py + 7)) --nx, --ny;

        if(nx > 1) nx = 1;
        if(nx < -1) nx = -1;
        if(ny > 1) ny = 1;
        if(ny < -1) ny = -1;
        if(nx == dx) nx = 0;
        if(ny == dy) ny = 0;

        // collision correction
        px += nx;
        py += ny;
    }

    load_chunks();
    if(run_chunks()) return;

    ++nframe;
}

static void skip_dialog_animation(uint8_t third_newline) {
    auto& d = sdata.dialog;
    uint8_t i;
    for(i = 0; d.message[i] != '\0' && i != third_newline; ++i) {}
    d.char_progress = i;
}

static void update_dialog()
{
    auto& d = sdata.dialog;
    uint8_t third_newline = 255;
    {
        uint8_t n = 0;
        for(uint8_t i = 0; i < sizeof(d.message) && d.message[i] != '\0'; ++i) {
            if(d.message[i] == '\n' && ++n == 3) {
                third_newline = i + 1;
                break;
            }
        }
    }
    if(btns_down & BTN_B) skip_dialog_animation(third_newline);
    if(btns_pressed & BTN_A) {
        if(d.char_progress == third_newline) {
            for(uint8_t i = 0; ; ++i) {
                d.message[i] = d.message[i + third_newline];
                if(d.message[i] == '\0') break;
            }
            d.char_progress = 0;
        }
        else if(d.message[d.char_progress] == '\0') {
            if(!(chunks_are_running && run_chunks()))
                change_state(STATE_MAP);
        } else {
            //skip_dialog_animation(third_newline);
        }
    } else {
        for(uint8_t i = 0; i < 2; ++i)
            if(d.message[d.char_progress] != '\0') ++d.char_progress;
        if(d.char_progress > third_newline) d.char_progress = third_newline;
    }
}

static void update_tp()
{
    auto& d = sdata.tp;
    ++d.frame;
    if(d.frame == TELEPORT_TRANSITION_FRAMES) {
        px = d.tx * 16 + 8;
        py = d.ty * 16 + 8;
        // terminate old chunk scripts
        chunks_are_running = false;
        load_chunks();
        run_chunks();
    }
    if(d.frame == TELEPORT_TRANSITION_FRAMES * 2) change_state(STATE_MAP);
}

void update()
{
    using update_func = void (*)();
    static update_func const FUNCS[] PROGMEM = {
        update_map,
        update_dialog,
        update_tp,
    };

    pmoving = false;
    (pgmptr(&FUNCS[state]))();
}
