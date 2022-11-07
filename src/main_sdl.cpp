#ifndef ARDUINO

#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#include "common.hpp"

#define MUTED_PALETTE 1
#define EXCLUDE_LAST_ROW 1

constexpr int FBH = EXCLUDE_LAST_ROW ? 63 : 64;

#ifndef NDEBUG
#include "gif.h"
static GifWriter gif;
static uint64_t gif_frame_time = 0;
#endif
static bool gif_recording = false;

int gplane;
uint8_t pixels[2][128 * 64];
uint8_t tex_pixels[128 * 63 * 4];

static void send_gif_frame(int ds = 3)
{
#ifndef NDEBUG
    if(gif_recording)
    {
        uint64_t t = SDL_GetTicks64();
        double dt = double(t - gif_frame_time) / 1000.0;
        gif_frame_time = t;
        // GifWriteFrame(&gif, tex_pixels, 128, FBH, int(dt * 100 + 1.5), 2);
        GifWriteFrame(&gif, tex_pixels, 128, FBH, ds);
    }
#endif
}

static void screen_recording_toggle()
{
#ifndef NDEBUG
    if(gif_recording)
    {
        GifEnd(&gif);
        gif_recording = false;
    }
    else {
        char fname[256];
        time_t rawtime;
        struct tm* ti;
        time(&rawtime);
        ti = localtime(&rawtime);
        (void)snprintf(fname, sizeof(fname),
                       "recording_%04d%02d%02d%02d%02d%02d.gif",
                       ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday,
                       ti->tm_hour + 1, ti->tm_min, ti->tm_sec);
        GifBegin(&gif, fname, 128, FBH, 33);
        gif_frame_time = SDL_GetTicks64();
        gif_recording = true;
        send_gif_frame(0);
    }
#endif
}

int main(int argc, char** argv)
{
    constexpr int ZOOM = 4;
    bool fullscreen = false;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    window = SDL_CreateWindow("arduboy_grayscale_rpg", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, 128 * ZOOM, FBH * ZOOM,
                              SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if(!window) goto error_destroy_window;
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) goto error_destroy_renderer;

    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                         SDL_TEXTUREACCESS_STREAMING, 128, FBH);

    initialize();

    bool quit = false;
    while(!quit)
    {

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) quit = true;
            if(e.type == SDL_KEYDOWN &&
               e.key.keysym.scancode == SDL_SCANCODE_F11)
            {
                fullscreen = !fullscreen;
                if(fullscreen)
                {
                    SDL_SetWindowFullscreen(window,
                                            SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                else {
                    SDL_SetWindowFullscreen(window, 0);
                }
            }
            if(e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_R)
                screen_recording_toggle();
        }

        static int frame = 0;

        if(frame == 0)
        {
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

        if(frame == 1) send_gif_frame();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for(int i = 0; i < 128 * FBH; ++i)
        {
            int p0 = pixels[0][i];
            int p1 = pixels[1][i];
#if MUTED_PALETTE
            uint8_t p = uint8_t((p0 * 0x40 + p1 * 0x80 + 0x10) & 0xff);
#else
            uint8_t p = uint8_t((p0 * 0x55 + p1 * 0xaa) & 0xff);
#endif
            tex_pixels[i * 4 + 0] = p;
            tex_pixels[i * 4 + 1] = p;
            tex_pixels[i * 4 + 2] = p;
            tex_pixels[i * 4 + 3] = 0xff;
        }

        SDL_UpdateTexture(tex, nullptr, tex_pixels, 128 * 4);

        {
            int x = 0, y = 0, w = 0, h = 0, z = 0;
            float r = 128.f / float(FBH);
            SDL_GetWindowSize(window, &w, &h);
            float wr = (float)w / (float)h;
            while(128 * z <= w && FBH * z <= h)
                ++z;
            --z;
            if(z == 0)
            {
                if(wr < r)
                {
                    h = int((float)w / r + 0.5f);
                    y = h / 2;
                }
                else {
                    w = int((float)h * r + 0.5f);
                    x = w / 2;
                }
            }
            else {
                x = (w - 128 * z) / 2;
                y = (h - FBH * z) / 2;
                w = 128 * z;
                h = FBH * z;
            }
            SDL_Rect rect{x, y, w, h};
            SDL_RenderCopy(renderer, tex, nullptr, &rect);
        }

        if(gif_recording)
        {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
            SDL_Rect rect{10, 10, 20, 20};
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    if(gif_recording) screen_recording_toggle();

    SDL_DestroyTexture(tex);

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    return 0;

error_destroy_renderer:
    SDL_DestroyRenderer(renderer);
error_destroy_window:
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
}

#endif
