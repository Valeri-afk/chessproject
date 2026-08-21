#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"

namespace ui
{
    // Internal framework text primitive shared by TextNode and text-bearing components.
    // It is not a NodeTree object and is not intended as a client-facing service.
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

        void draw(
            SDL_Renderer *renderer,
            const LayoutPosition &position,
            const LayoutSize &size);

    private:
        void releaseTextObject() noexcept;
        bool ensureTextObject(SDL_Renderer *renderer);

        std::string text_;
        TTF_Font *font_ = nullptr;
        TextAlignment horizontalAlignment_ = TextAlignment::START;
        TextAlignment verticalAlignment_ = TextAlignment::START;
        Color color_ = Colors::white;

        SDL_Renderer *cachedRenderer_ = nullptr;
        TTF_TextEngine *textEngine_ = nullptr;
        TTF_Text *textObject_ = nullptr;
    };
}
