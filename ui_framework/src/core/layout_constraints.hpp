#pragma once

#include "ui_framework/core/node.hpp"

namespace ui::internal
{

    LayoutSize resolveMeasurementProposal(
        const Node &node,
        LayoutSize proposal) noexcept;

    LayoutSize resolveFinalSize(
        const Node &node,
        LayoutSize allocated) noexcept;

}
