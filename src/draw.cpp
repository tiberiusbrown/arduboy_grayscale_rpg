#include "common.hpp"

#include "font_adv.hpp"
#include "generated/font_img.hpp"
#include "generated/fxdata.h"

static uint8_t add_enemy_sprite_entry(draw_sprite_entry* entry, uint8_t ci,
                                      int16_t ox, int16_t oy)
{
    auto const& e = active_chunks[ci].enemy;
    if(!e.active) return 0;
    uint8_t f = e.type * 16;
    uint8_t d = e.dir;
    if(!(d & 0x80)) {
        f += d * 2;
        f += ((nframe >> 2) & 3);
    }
    entry->addr = ENEMY_IMG;
    entry->frame = f;
    entry->x = ox + e.x;
    entry->y = oy + e.y - 4;
    return 1;
}

void sort_and_draw_sprites(draw_sprite_entry* entries, uint8_t n)
{
    // sort sprites
    for(uint8_t i = 1; i < n; ++i) {
        for(uint8_t j = i; j > 0 && entries[j - 1].y > entries[j].y; --j) {
            auto t = entries[j];
            entries[j] = entries[j - 1];
            entries[j - 1] = t;
        }
    }

    // draw sprites
    for(uint8_t i = 0; i < n; ++i) {
        platform_fx_drawplusmask(entries[i].x, entries[i].y, entries[i].addr,
                                 entries[i].frame, 16, 16);
    }
}

void draw_sprites()
{
    draw_sprite_entry entries[5]; // increase as necessary
    uint8_t n = 0;

    // player sprite
    {
        uint8_t f = pdir * 4;
        if(pmoving) f += ((nframe >> 2) & 3);
        entries[n++] = {PLAYER_IMG, f, 64 - 8, 32 - 8 - 4};
    }

    // chunk enemies
    {
        uint16_t tx = px - 64 + 8;
        uint16_t ty = py - 32 + 8;
        uint8_t cx = uint8_t(tx >> 7);
        uint8_t cy = uint8_t(ty >> 6);
        int16_t ox = -int16_t(tx & 0x7f);
        int16_t oy = -int16_t(ty & 0x3f);
        n += add_enemy_sprite_entry(&entries[n], 0, ox, oy);
        n += add_enemy_sprite_entry(&entries[n], 1, ox + 128, oy);
        n += add_enemy_sprite_entry(&entries[n], 2, ox, oy + 64);
        n += add_enemy_sprite_entry(&entries[n], 3, ox + 128, oy + 64);
    }

    sort_and_draw_sprites(entries, n);
}

void draw_player()
{
    uint8_t f = pdir * 4;
    if(pmoving) f += ((nframe >> 2) & 3);
    platform_fx_drawplusmask(64 - 8, 32 - 8 - 4, PLAYER_IMG, f, 16, 16);
}

static void draw_chunk_tiles(uint8_t i, int16_t ox, int16_t oy)
{
    auto const& ac = active_chunks[i];
    uint8_t const* tiles = ac.chunk.tiles_flat;
    for(uint8_t r = 0, n = 0; r < 64; r += 16) {
        int16_t y = oy + r;
        if(y <= -16 || y >= 64) {
            n += 8;
            continue;
        }
        if(state == STATE_DIALOG && y >= 35) break;
        for(uint8_t c = 0; c < 128; c += 16, ++n) {
            int16_t x = ox + c;
            platform_fx_drawoverwrite(x, y, TILE_IMG, tiles[n], 16, 16);
        }
    }
}

void draw_tiles()
{
    uint16_t tx = px - 64 + 8;
    uint16_t ty = py - 32 + 8;
    uint8_t cx = uint8_t(tx >> 7);
    uint8_t cy = uint8_t(ty >> 6);
    int16_t ox = -int16_t(tx & 0x7f);
    int16_t oy = -int16_t(ty & 0x3f);
    draw_chunk_tiles(0, ox, oy);
    draw_chunk_tiles(1, ox + 128, oy);
    draw_chunk_tiles(2, ox, oy + 64);
    draw_chunk_tiles(3, ox + 128, oy + 64);
}

void draw_text(uint8_t x, uint8_t y, char const* str)
{
    char t;
    uint8_t cx = x;
    while((t = *str++) != '\0') {
        if(t == '\n') {
            y += 9;
            cx = x;
            continue;
        }
        t -= ' ';
        uint8_t const* bitmap = &FONT_IMG[t * 24];
        platform_drawplusmask(cx, y, 8, 8, bitmap);
        cx += pgm_read_byte(&FONT_ADV[t]);
    }
}

void wrap_text(char* str, uint8_t w)
{
    uint8_t i = 0;
    uint8_t x = 0;
    char t;
    while((t = str[i++]) != '\0') {
        if(t == '\n') {
            x = 0;
            continue;
        }
        t -= ' ';
        x += pgm_read_byte(&FONT_ADV[t]);
        if(x > w) {
            --i;
            while(t != ' ' && i != 0)
                t = str[--i];
            if(i != 0) str[i] = '\n';
        }
    }
}

void draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    platform_fillrect(x, y, w, h, BLACK);
    platform_drawrect(x, y, w, h, WHITE);
    return;
}
