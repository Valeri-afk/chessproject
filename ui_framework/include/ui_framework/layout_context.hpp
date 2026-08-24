#pragma once

#include <cstddef>
#include <functional>

#include "ui_framework/types.hpp"

namespace ui
{
    struct MeasureContext
    {
        LayoutSize availableContentSize{};

        std::function<LayoutSize(
            std::size_t,
            const LayoutSize &)> measureChild;
    };

    struct ArrangeContext
    {
        LayoutPosition contentPosition{};
        LayoutSize contentSize{};

        std::function<void(
            std::size_t,
            const LayoutPosition &,
            const LayoutSize &)> arrangeChild;
    };
}
