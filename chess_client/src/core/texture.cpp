#include "texture.hpp"
#include <cassert>
#include <iostream>

Texture::Texture(SDL_Renderer *renderer, SDL_Texture *tex)
    : texture(tex), ownerRenderer(renderer)
{
    assert(texture && renderer);
    SDL_GetTextureSize(texture, &width, &height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
}
Texture::~Texture()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }
}

Texture::Texture(Texture &&other) noexcept
    : texture(other.texture), width(other.width), height(other.height), ownerRenderer(other.ownerRenderer)
{
    other.texture = nullptr;
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    if (this != &other)
    {
        if (texture)
            SDL_DestroyTexture(texture);
        texture = other.texture;
        width = other.width;
        height = other.height;
        ownerRenderer = other.ownerRenderer;
        other.texture = nullptr;
    }
    return *this;
}

void Texture::render(float x, float y, const SDL_FRect *srcRect, float scale, float angle) const
{
    if (!texture)
        return;

    SDL_FRect dstRect = {x, y, width * scale, height * scale};
    if (srcRect)
    {
        dstRect.w = srcRect->w * scale;
        dstRect.h = srcRect->h * scale;
    }

    SDL_RenderTextureRotated(ownerRenderer, texture, srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);
}
