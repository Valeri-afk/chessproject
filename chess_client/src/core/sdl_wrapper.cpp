#include "sdl_wrapper.hpp"

#include <algorithm>
#include <iostream>

SDLWrapper::SDLWrapper(
    const char *title,
    int logicalW,
    int logicalH,
    bool startFullscreen)
    : window(nullptr),
      renderer(nullptr),
      width(logicalW),
      height(logicalH),
      lastFPSTime(0),
      frameCount(0),
      currentFPS(0)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cerr << "SDL_Init Error: "
                  << SDL_GetError()
                  << std::endl;
        return;
    }

    Uint32 flags =
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIGH_PIXEL_DENSITY;

    window = SDL_CreateWindow(
        title,
        logicalW * 3,
        logicalH * 3,
        flags);

    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: "
                  << SDL_GetError()
                  << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(
        window,
        nullptr);

    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: "
                  << SDL_GetError()
                  << std::endl;
        return;
    }

    SDL_SetRenderLogicalPresentation(
        renderer,
        logicalW,
        logicalH,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    if (startFullscreen)
    {
        SDL_SetWindowFullscreen(window, true);
    }

    lastFPSTime = SDL_GetTicks();
}

SDLWrapper::~SDLWrapper()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}

void SDLWrapper::updateFPS() const
{
    frameCount++;

    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsed = currentTime - lastFPSTime;

    if (elapsed >= 1000)
    {
        currentFPS = frameCount;
        frameCount = 0;
        lastFPSTime = currentTime;
    }
}

void SDLWrapper::clear(
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a)
{
    SDL_SetRenderDrawColor(
        renderer,
        r,
        g,
        b,
        a);

    SDL_RenderClear(renderer);
}

void SDLWrapper::present()
{
    SDL_RenderPresent(renderer);
}

void SDLWrapper::drawRect(
    float x,
    float y,
    int w,
    int h,
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a,
    bool filled)
{
    SDL_FRect rect{
        x,
        y,
        (float)w,
        (float)h};

    SDL_SetRenderDrawColor(
        renderer,
        r,
        g,
        b,
        a);

    if (filled)
        SDL_RenderFillRect(renderer, &rect);
    else
        SDL_RenderRect(renderer, &rect);
}

void SDLWrapper::drawFullscreenOverlay(
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a)
{
    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(
        renderer,
        r,
        g,
        b,
        a);

    SDL_FRect rect{
        0.0f,
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height)};

    SDL_RenderFillRect(renderer, &rect);
}