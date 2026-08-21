#pragma once

#include <algorithm>
#include <limits>

#include "ui_framework/types.hpp"

namespace ui
{
    struct LayoutConstraints
    {
        float minWidth = 0.0f;
        float maxWidth = std::numeric_limits<float>::max();
        float minHeight = 0.0f;
        float maxHeight = std::numeric_limits<float>::max();

        LayoutSize clamp(LayoutSize size) const noexcept
        {
            return {
                std::clamp(size.width, minWidth, maxWidth),
                std::clamp(size.height, minHeight, maxHeight)};
        }
    };
}
