#ifndef ARDUINO

#include <SDL.h>

#include <stdint.h>
#include <string.h>

#include "common.hpp"

#define MUTED_PALETTE 1

int gplane;
uint8_t pixels[2][128 * 64];
uint8_t tex_pixels[128 * 63 * 3];

int main(int argc, char** argv)
{
    constexpr int ZOOM = 4;
    bool fullscreen = false;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if(0 != SDL_CreateWindowAndRenderer(128 * ZOOM, 63 * ZOOM,
                                        SDL_WINDOW_ALLOW_HIGHDPI |
                                            SDL_WINDOW_RESIZABLE,
                                        &window, &renderer))
        goto error_quit;

    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                         SDL_TEXTUREACCESS_STREAMING, 128, 63);

    initialize();

    bool quit = false;
    while(!quit) {

        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) quit = true;
            if(e.type == SDL_KEYDOWN &&
               e.key.keysym.scancode == SDL_SCANCODE_F11) {
                fullscreen = !fullscreen;
                if(fullscreen) {
                    SDL_SetWindowFullscreen(window,
                                            SDL_WINDOW_FULLSCREEN_DESKTOP);
                } else {
                    SDL_SetWindowFullscreen(window, 0);
                }
            }
        }

        static int frame = 0;

        if(frame == 0) {
            auto const* k = SDL_GetKeyboardState(nullptr);
            uint8_t b = 0;
            if(k[SDL_SCANCODE_UP]) b |= BTN_UP;
            if(k[SDL_SCANCODE_DOWN]) b |= BTN_DOWN;
            if(k[SDL_SCANCODE_LEFT]) b |= BTN_LEFT;
            if(k[SDL_SCANCODE_RIGHT]) b |= BTN_RIGHT;
            if(k[SDL_SCANCODE_A]) b |= BTN_A;
            if(k[SDL_SCANCODE_B]) b |= BTN_B;
            btns_pressed = b & ~btns_down;
            btns_down = b;
            update();
        }
        if(++frame == 2) frame = 0;

        memset(pixels, 0, sizeof(pixels));

        gplane = 0;
        render();

        gplane = 1;
        render();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for(int i = 0; i < 128 * 63; ++i) {
            int p0 = pixels[0][i];
            int p1 = pixels[1][i];
#if MUTED_PALETTE
            uint8_t p = uint8_t((p0 * 0x40 + p1 * 0x80 + 0x10) & 0xff);
#else
            uint8_t p = uint8_t((p0 * 0x55 + p1 * 0xaa) & 0xff);
#endif
            tex_pixels[i * 3 + 0] = p;
            tex_pixels[i * 3 + 1] = p;
            tex_pixels[i * 3 + 2] = p;
        }

        SDL_UpdateTexture(tex, nullptr, tex_pixels, 128 * 3);

        {
            int x = 0, y = 0, w = 0, h = 0, z = 0;
            float r = 128.f / 63.f;
            SDL_GetWindowSize(window, &w, &h);
            float wr = (float)w / (float)h;
            while(128 * z <= w && 63 * z <= h)
                ++z;
            --z;
            if(z == 0) {
                if(wr < r) {
                    h = int((float)w / r + 0.5f);
                    y = h / 2;
                } else {
                    w = int((float)h * r + 0.5f);
                    x = w / 2;
                }
            } else {
                x = (w - 128 * z) / 2;
                y = (h - 63 * z) / 2;
                w = 128 * z;
                h = 63 * z;
            }
            SDL_Rect rect{x, y, w, h};
            SDL_RenderCopy(renderer, tex, nullptr, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(tex);

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    return 0;

error_quit:
    SDL_Quit();
    return -1;
}

#endif
