#pragma once

#include "ui_framework/types.hpp"

#include <cstddef>
#include <functional>

namespace ui
{

    class StackPanelNode;

    namespace internal
    {
        struct LinearMeasureContext
        {
            LayoutSize availableSize;

            std::function<LayoutSize(
                size_t,
                const LayoutSize &)> measureChild;
        };

        struct LinearArrangeContext
        {
            LayoutPosition contentPosition;
            LayoutSize contentSize;

            std::function<void(
                size_t,
                const LayoutPosition &,
                const LayoutSize &)> placeChild;
        };

        LayoutSize measureLinearPanel(
            StackPanelNode &panel,
            LinearMeasureContext &ctx);

        void arrangeLinearPanel(
            StackPanelNode &panel,
            LinearArrangeContext &ctx);
    }

}
