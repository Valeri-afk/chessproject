#pragma once

#include <functional>

#include "ui_framework/types.hpp"

namespace ui
{
    class Node;

    struct MeasureContext
    {
        LayoutSize availableContentSize{};
        std::function<LayoutSize(Node &, const LayoutSize &)> measureChild;
    };

    struct ArrangeContext
    {
        LayoutPosition contentPosition{};
        LayoutSize contentSize{};
        std::function<LayoutSize(Node &)> desiredSize;
        std::function<void(Node &, const LayoutPosition &, const LayoutSize &)> arrangeChild;
    };
}
