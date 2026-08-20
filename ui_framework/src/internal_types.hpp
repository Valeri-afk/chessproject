#pragma once

#include "ui_framework/types.hpp"

namespace ui
{
    struct LayoutProps
    {
        LayoutSize size;
        LayoutPosition position;

        LayoutSize minSize_;
        LayoutSize maxSize_{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()};

        LayoutSize desiredSize;

        LayoutSize actualSize;
        LayoutPosition actualPosition;
    };
}
