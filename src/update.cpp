#include "common.hpp"

void update()
{
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
    update_chunks();

    ++nframe;
}
