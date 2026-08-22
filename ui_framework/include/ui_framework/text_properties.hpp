#pragma once

#include <string_view>

#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"

namespace ui
{
    struct TextProperties
    {
        std::string_view text;
        TTF_Font *font = nullptr;
        TextAlignment horizontalAlignment = TextAlignment::START;
        TextAlignment verticalAlignment = TextAlignment::START;
        Color color = Colors::white;
    };
}
