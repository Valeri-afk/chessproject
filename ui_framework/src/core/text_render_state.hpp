#pragma once

#include "ui_framework/text_layout.hpp"
#include "ui_framework/types.hpp"

namespace ui
{
    struct TextRenderState
    {
        const TextLayout *layout = nullptr;
        TextAlignment horizontalAlignment = TextAlignment::START;
        TextAlignment verticalAlignment = TextAlignment::START;
        Color color = Colors::white;
        LayoutPosition position{};
        LayoutSize size{};
    };
}
