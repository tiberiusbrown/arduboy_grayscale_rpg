#ifndef ARDUINO

#include <SDL.h>

#include <stdint.h>
#include <string.h>

#include "common.hpp"

int gplane;
uint8_t pixels[2][128 * 64];
uint8_t tex_pixels[128 * 63 * 3];

int main(int argc, char** argv)
{
    constexpr int ZOOM = 4;
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
            uint8_t p = uint8_t((p0 * 0x55 + p1 * 0xaa) & 0xff);
            tex_pixels[i * 3 + 0] = p;
            tex_pixels[i * 3 + 1] = p;
            tex_pixels[i * 3 + 2] = p;
        }

        SDL_UpdateTexture(tex, nullptr, tex_pixels, 128 * 3);
        SDL_RenderCopy(renderer, tex, nullptr, nullptr);

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
