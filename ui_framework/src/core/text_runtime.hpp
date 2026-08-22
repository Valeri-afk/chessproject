#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"
#include "text_primitive.hpp"

namespace ui
{
    class TextRuntime
    {
    public:
        TextRuntime() = default;
        ~TextRuntime() = default;

        TextRuntime(const TextRuntime &) = delete;
        TextRuntime &operator=(const TextRuntime &) = delete;

        LayoutSize measure(const std::string &text, TTF_Font *font, float availableWidth) const noexcept;

        void draw(
            SDL_Renderer *renderer,
            const std::string &text,
            TTF_Font *font,
            TextAlignment horizontalAlignment,
            TextAlignment verticalAlignment,
            Color color,
            const LayoutPosition &position,
            const LayoutSize &size);

    private:
        TextPrimitive primitive_;
    };
}
