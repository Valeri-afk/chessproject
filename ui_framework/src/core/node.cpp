#include "ui_framework/node.hpp"

#include "layout_constraints.hpp"
#include "node_tree.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace
{
    float finiteOrZero(float value) noexcept { return std::isfinite(value) ? value : 0.0f; }
    float finiteOrInfinity(float value) noexcept { return std::isfinite(value) ? value : std::numeric_limits<float>::max(); }
    ui::LayoutSize sanitizeSize(ui::LayoutSize size) noexcept { return {finiteOrZero(size.width), finiteOrZero(size.height)}; }
    ui::LayoutSizeValue sanitizeSizeValue(ui::LayoutSizeValue size) noexcept
    {
        if (size.width.type == ui::LayoutValueType::Value)
            size.width.value = finiteOrZero(size.width.value);
        if (size.height.type == ui::LayoutValueType::Value)
            size.height.value = finiteOrZero(size.height.value);
        return size;
    }
    ui::LayoutPosition sanitizePosition(ui::LayoutPosition position) noexcept { return {finiteOrZero(position.x), finiteOrZero(position.y)}; }
    ui::Padding sanitizePadding(ui::Padding padding) noexcept
    {
        padding.left = finiteOrZero(padding.left);
        padding.right = finiteOrZero(padding.right);
        padding.top = finiteOrZero(padding.top);
        padding.bottom = finiteOrZero(padding.bottom);
        return padding;
    }
    ui::Border sanitizeBorder(ui::Border border) noexcept
    {
        border.left = finiteOrZero(border.left);
        border.right = finiteOrZero(border.right);
        border.top = finiteOrZero(border.top);
        border.bottom = finiteOrZero(border.bottom);
        return border;
    }
    void keepMaxAtLeastMin(ui::LayoutSize &minSize, ui::LayoutSize &maxSize) noexcept
    {
        maxSize.width = std::max(finiteOrInfinity(maxSize.width), minSize.width);
        maxSize.height = std::max(finiteOrInfinity(maxSize.height), minSize.height);
    }
    void keepMinAtMostMax(ui::LayoutSize &minSize, ui::LayoutSize &maxSize) noexcept
    {
        minSize.width = std::min(finiteOrInfinity(minSize.width), maxSize.width);
        minSize.height = std::min(finiteOrInfinity(minSize.height), maxSize.height);
    }
    void setMinWidthValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float width) noexcept
    {
        minSize.width = finiteOrZero(width);
        keepMaxAtLeastMin(minSize, maxSize);
    }
    void setMinHeightValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float height) noexcept
    {
        minSize.height = finiteOrZero(height);
        keepMaxAtLeastMin(minSize, maxSize);
    }
    void setMaxWidthValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float width) noexcept
    {
        maxSize.width = finiteOrZero(width);
        keepMinAtMostMax(minSize, maxSize);
    }
    void setMaxHeightValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float height) noexcept
    {
        maxSize.height = finiteOrZero(height);
        keepMinAtMostMax(minSize, maxSize);
    }
}

namespace ui
{
    void Node::invalidateLayout() noexcept
    {
        if (owner_)
            owner_->insertLayoutQueue(this);
    }

    void Node::animateFloat(
        const void *propertyKey,
        float currentValue,
        float targetValue,
        float duration,
        AnimationEasing easing,
        std::function<void(float)> setter)
    {
        if (owner_)
        {
            owner_->animationSystem_.animateFloat(
                this,
                propertyKey,
                animationLifetimeToken_,
                currentValue,
                targetValue,
                duration,
                easing,
                std::move(setter));
        }
        else if (setter)
        {
            setter(targetValue);
        }
    }

    void Node::cancelAnimation(const void *propertyKey) noexcept
    {
        if (owner_)
            owner_->animationSystem_.cancel(this, propertyKey);
    }

    void Node::dispatchEventImpl(UIEvent &event, const std::type_index &eventType, NodeTree &nodeTree)
    {
        (void)nodeTree;
        std::vector<std::function<void(UIEvent &, Node &)>> callbacks;
        callbacks.reserve(eventHandlers_.size());
        for (const EventHandlerRecord &record : eventHandlers_)
            if (record.eventType == eventType)
                callbacks.push_back(record.callback);
        for (auto &callback : callbacks)
            callback(event, *this);
    }

    Node::Node() = default;
    Node::~Node()
    {
        animationLifetimeToken_.reset();
    }
