#include "videocontext.h"

#include <stdexcept>
#include <eggcpt/icon.h>

#include "base/bit.h"

void VideoContext::raise()
{
    SDL_RaiseWindow(window);
}

void VideoContext::fullscreen()
{
    SDL_ShowCursor(SDL_ShowCursor(SDL_QUERY) ^ 0x1);
    SDL_SetWindowFullscreen(window, SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void VideoContext::title(const std::string& title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

void VideoContext::renderCopyTexture()
{
    SDL_UpdateTexture(texture, NULL, buffer.data(), sizeof(u32) * kScreenW);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void VideoContext::renderPresent()
{
    SDL_RenderPresent(renderer);
}

void VideoContext::renderIcon()
{
    int w;
    int h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);
    SDL_RenderSetLogicalSize(renderer, 18, 18);

    for (const auto& pixel : eggcpt::icon::pixels())
    {
        SDL_SetRenderDrawColor(
            renderer,
            pixel.r(),
            pixel.g(),
            pixel.b(),
            SDL_ALPHA_OPAQUE);

        SDL_RenderDrawPoint(
            renderer,
            pixel.x() + 1,
            pixel.y() + 1);
    }

    SDL_RenderSetLogicalSize(renderer, w, h);
}

u32* VideoContext::scanline(uint line)
{
    EGGCPT_ASSERT(line < kScreenH);

    return buffer.data() + (kScreenW * line);
}

void VideoContext::renderClear(u32 color)
{
    SDL_SetRenderDrawColor(
        renderer,
        bit::seq<16, 8>(color),
        bit::seq< 8, 8>(color),
        bit::seq< 0, 8>(color),
        SDL_ALPHA_OPAQUE);

    SDL_RenderClear(renderer);
}

void VideoContext::init()
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO))
        throw std::runtime_error("Cannot init video context");

    if (!initWindow())   throw std::runtime_error("Cannot init window");
    if (!initRenderer()) throw std::runtime_error("Cannot init renderer");
    if (!initTexture())  throw std::runtime_error("Cannot init texture");
}

void VideoContext::deinit()
{
    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

bool VideoContext::initWindow()
{
    return window = SDL_CreateWindow(
        "eggvance",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        2 * kScreenW, 2 * kScreenH,
        SDL_WINDOW_RESIZABLE);
}

bool VideoContext::initRenderer()
{
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    SDL_RenderSetLogicalSize(renderer, kScreenW, kScreenH);

    return renderer;
}

bool VideoContext::initTexture()
{
    return texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        kScreenW, kScreenH);
}
