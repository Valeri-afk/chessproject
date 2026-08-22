#pragma once

#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "ui_framework/types.hpp"

namespace ui
{
    class TextPrimitive
    {
    public:
        TextPrimitive() = default;
        ~TextPrimitive();
        TextPrimitive(const TextPrimitive &) = delete;
        TextPrimitive &operator=(const TextPrimitive &) = delete;
        const std::string &getText() const noexcept;
        void setText(std::string text);
        TTF_Font *getFont() const noexcept;
        void setFont(TTF_Font *font) noexcept;
        TextAlignment getHorizontalAlignment() const noexcept;
        void setHorizontalAlignment(TextAlignment alignment) noexcept;
        TextAlignment getVerticalAlignment() const noexcept;
        void setVerticalAlignment(TextAlignment alignment) noexcept;
        Color getColor() const noexcept;
        void setColor(Color color) noexcept;
        LayoutSize measure(float availableWidth = -1.0f) const noexcept;
        void draw(SDL_Renderer *renderer, const LayoutPosition &position, const LayoutSize &size);

    private:
        void releaseTextObject() noexcept;
        void releaseRasterFont() noexcept;
        bool ensureTextObject(SDL_Renderer *renderer, TTF_Font *font);
        bool ensureRasterFont(float scale);
        bool getIntegerPresentationScale(SDL_Renderer *renderer, float &scale, SDL_FRect &presentationRect) const noexcept;

        std::string text_;
        TTF_Font *font_ = nullptr;
        TTF_Font *rasterFont_ = nullptr;
        float rasterScale_ = 1.0f;
        Uint32 rasterFontGeneration_ = 0;
        TextAlignment horizontalAlignment_ = TextAlignment::START;
        TextAlignment verticalAlignment_ = TextAlignment::START;
        Color color_ = Colors::white;
        SDL_Renderer *cachedRenderer_ = nullptr;
        TTF_Font *cachedTextFont_ = nullptr;
        TTF_TextEngine *textEngine_ = nullptr;
        TTF_Text *textObject_ = nullptr;
    };
}
