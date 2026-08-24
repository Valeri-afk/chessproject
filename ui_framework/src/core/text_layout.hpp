#pragma once

#include <string>
#include <utility>

#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"

namespace ui
{
    class TextLayout
    {
    public:
        TextLayout() = default;
        TextLayout(const TextLayout &) = default;
        TextLayout &operator=(const TextLayout &) = default;

        void setText(std::string text) { text_ = std::move(text); }
        const std::string &getText() const noexcept { return text_; }

        void setFont(TTF_Font *font) noexcept { font_ = font; }
        TTF_Font *getFont() const noexcept { return font_; }

        LayoutSize measure(float availableWidth) const noexcept;

    private:
        std::string text_;
        TTF_Font *font_ = nullptr;
    };
}
