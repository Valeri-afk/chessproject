#include "ui_framework/core/stackpanelnode.hpp"

#include <cmath>

namespace ui
{

    StackPanelNode::StackPanelNode(Orientation orientation)
        : orientation_(orientation)
    {
    }

    void StackPanelNode::setOrientation(Orientation orientation)
    {
        if (orientation_ == orientation)
            return;

        deferLayoutMutation(
            [orientation](Node &node)
            {
                static_cast<StackPanelNode &>(node).orientation_ =
                    orientation;
            });
    }

    StackPanelNode::Orientation StackPanelNode::getOrientation()
        const noexcept
    {
        return orientation_;
    }

    void StackPanelNode::setGap(float gap)
    {
        if (!std::isfinite(gap) || gap < 0.0f)
            return;

        if (gap_ == gap)
            return;

        deferLayoutMutation(
            [gap](Node &node)
            {
                static_cast<StackPanelNode &>(node).gap_ = gap;
            });
    }

    float StackPanelNode::getGap() const noexcept
    {
        return gap_;
    }

    void StackPanelNode::setMainAlignment(MainAxisAlignment alignment)
    {
        if (mainAlignment_ == alignment)
            return;

        deferLayoutMutation(
            [alignment](Node &node)
            {
                static_cast<StackPanelNode &>(node).mainAlignment_ =
                    alignment;
            });
    }

    MainAxisAlignment StackPanelNode::getMainAlignment() const noexcept
    {
        return mainAlignment_;
    }

    void StackPanelNode::setCrossAlignment(CrossAxisAlignment alignment)
    {
        if (crossAlignment_ == alignment)
            return;

        deferLayoutMutation(
            [alignment](Node &node)
            {
                static_cast<StackPanelNode &>(node).crossAlignment_ =
                    alignment;
            });
    }

    CrossAxisAlignment StackPanelNode::getCrossAlignment() const noexcept
    {
        return crossAlignment_;
    }

}
