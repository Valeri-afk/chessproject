#include "ui_framework/core/node.hpp"

#include "nodetree.hpp"

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
        if (size.width.type == ui::LayoutValueType::Value) size.width.value = finiteOrZero(size.width.value);
        if (size.height.type == ui::LayoutValueType::Value) size.height.value = finiteOrZero(size.height.value);
        return size;
    }
    ui::LayoutPosition sanitizePosition(ui::LayoutPosition position) noexcept { return {finiteOrZero(position.x), finiteOrZero(position.y)}; }
    ui::Padding sanitizePadding(ui::Padding padding) noexcept
    {
        padding.left = finiteOrZero(padding.left); padding.right = finiteOrZero(padding.right);
        padding.top = finiteOrZero(padding.top); padding.bottom = finiteOrZero(padding.bottom); return padding;
    }
    ui::Border sanitizeBorder(ui::Border border) noexcept
    {
        border.left = finiteOrZero(border.left); border.right = finiteOrZero(border.right);
        border.top = finiteOrZero(border.top); border.bottom = finiteOrZero(border.bottom); return border;
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
        minSize.width = finiteOrZero(width); keepMaxAtLeastMin(minSize, maxSize);
    }
    void setMinHeightValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float height) noexcept
    {
        minSize.height = finiteOrZero(height); keepMaxAtLeastMin(minSize, maxSize);
    }
    void setMaxWidthValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float width) noexcept
    {
        maxSize.width = finiteOrZero(width); keepMinAtMostMax(minSize, maxSize);
    }
    void setMaxHeightValue(ui::LayoutSize &minSize, ui::LayoutSize &maxSize, float height) noexcept
    {
        maxSize.height = finiteOrZero(height); keepMinAtMostMax(minSize, maxSize);
    }
}

namespace ui
{
    void Node::deferLayoutMutation(std::function<void(Node &)> fn)
    {
        if (!owner_) { fn(*this); return; }
        NodeTree *tree = owner_;
        const Id nodeId = id_;
        tree->enqueueNodeMutation(*this, [tree, nodeId, fn = std::move(fn)]() mutable
        {
            if (Node *node = tree->findNode(nodeId)) fn(*node);
            if (tree->findNode(nodeId)) tree->insertLayoutQueueById(nodeId);
        });
    }

    Node::Node() = default;
    Node::~Node() = default;
    Node::Id Node::id() const noexcept { return id_; }
    Node *Node::parent() const noexcept { return parent_; }

    void Node::setVisible(bool visible)
    {
        if (visible_ == visible) return;
        deferLayoutMutation([visible](Node &node) { node.visible_ = visible; });
    }
    bool Node::isVisible() const noexcept { return visible_; }
    void Node::setEnabled(bool enabled) noexcept { enabled_ = enabled; }
    bool Node::isEnabled() const noexcept { return enabled_; }
    void Node::setFocusable(bool focusable) noexcept { focusable_ = focusable; }
    bool Node::isFocusable() const noexcept { return focusable_; }
    void Node::setCapturable(bool capturable) noexcept { capturable_ = capturable; }
    bool Node::isCapturable() const noexcept { return capturable_; }

    void Node::setPosition(const LayoutPosition &position)
    {
        auto safePosition = sanitizePosition(position);
        deferLayoutMutation([safePosition](Node &node) mutable { node.position_ = safePosition; });
    }
    LayoutPosition Node::getPosition() const noexcept { return position_; }

    void Node::setPositionMode(PositionMode positionMode)
    {
        deferLayoutMutation([positionMode](Node &node) mutable { node.positionMode_ = positionMode; });
    }
    PositionMode Node::getPositionMode() const noexcept { return positionMode_; }

    void Node::setSize(const LayoutSizeValue &size)
    {
        const LayoutSizeValue safe = sanitizeSizeValue(size);
        deferLayoutMutation([safe](Node &node) { node.size_ = safe; });
    }
    LayoutSizeValue Node::getSize() const noexcept { return size_; }
    LayoutSize Node::getDesiredSize() const noexcept { return desiredSize_; }

    void Node::setMinSize(const LayoutSize &size)
    {
        const LayoutSize safeSize = sanitizeSize(size);
        deferLayoutMutation([safeSize](Node &node) { node.minSize_ = safeSize; keepMaxAtLeastMin(node.minSize_, node.maxSize_); });
    }
    void Node::setMaxSize(const LayoutSize &size)
    {
        const LayoutSize safeSize = sanitizeSize(size);
        deferLayoutMutation([safeSize](Node &node) { node.maxSize_ = safeSize; keepMinAtMostMax(node.minSize_, node.maxSize_); });
    }
    void Node::setMinWidth(float width)
    {
        const float safeWidth = finiteOrZero(width);
        deferLayoutMutation([safeWidth](Node &node) { setMinWidthValue(node.minSize_, node.maxSize_, safeWidth); });
    }
    void Node::setMinHeight(float height)
    {
        const float safeHeight = finiteOrZero(height);
        deferLayoutMutation([safeHeight](Node &node) { setMinHeightValue(node.minSize_, node.maxSize_, safeHeight); });
    }
    void Node::setMaxWidth(float width)
    {
        const float safeWidth = finiteOrZero(width);
        deferLayoutMutation([safeWidth](Node &node) { setMaxWidthValue(node.minSize_, node.maxSize_, safeWidth); });
    }
    void Node::setMaxHeight(float height)
    {
        const float safeHeight = finiteOrZero(height);
        deferLayoutMutation([safeHeight](Node &node) { setMaxHeightValue(node.minSize_, node.maxSize_, safeHeight); });
    }
    LayoutSize Node::getMinSize() const noexcept { return minSize_; }
    LayoutSize Node::getMaxSize() const noexcept { return maxSize_; }
    float Node::getMinWidth() const noexcept { return finiteOrZero(minSize_.width); }
    float Node::getMinHeight() const noexcept { return finiteOrZero(minSize_.height); }
    float Node::getMaxWidth() const noexcept { return finiteOrInfinity(maxSize_.width); }
    float Node::getMaxHeight() const noexcept { return finiteOrInfinity(maxSize_.height); }

    LayoutSize Node::clampSize(LayoutSize size, LayoutSize minSize, LayoutSize maxSize) const
    {
        LayoutConstraints constraints;
        constraints.minWidth = finiteOrZero(minSize.width);
        constraints.maxWidth = finiteOrInfinity(maxSize.width);
        constraints.minHeight = finiteOrZero(minSize.height);
        constraints.maxHeight = finiteOrInfinity(maxSize.height);
        return constraints.clamp(sanitizeSize(size));
    }

    void Node::setPadding(const Padding &padding) { deferLayoutMutation([padding](Node &node) { node.padding_ = sanitizePadding(padding); }); }
    Padding Node::getPadding() const noexcept { return padding_; }
    void Node::setLeftPadding(float value) { deferLayoutMutation([value](Node &node) { node.padding_.left = finiteOrZero(value); }); }
    void Node::setRightPadding(float value) { deferLayoutMutation([value](Node &node) { node.padding_.right = finiteOrZero(value); }); }
    void Node::setTopPadding(float value) { deferLayoutMutation([value](Node &node) { node.padding_.top = finiteOrZero(value); }); }
    void Node::setBottomPadding(float value) { deferLayoutMutation([value](Node &node) { node.padding_.bottom = finiteOrZero(value); }); }
    void Node::setBorder(const Border &border) { deferLayoutMutation([border](Node &node) { node.border_ = sanitizeBorder(border); }); }
    Border Node::getBorder() const noexcept { return border_; }
    void Node::setLeftBorder(float value) { deferLayoutMutation([value](Node &node) { node.border_.left = finiteOrZero(value); }); }
    void Node::setRightBorder(float value) { deferLayoutMutation([value](Node &node) { node.border_.right = finiteOrZero(value); }); }
    void Node::setTopBorder(float value) { deferLayoutMutation([value](Node &node) { node.border_.top = finiteOrZero(value); }); }
    void Node::setBottomBorder(float value) { deferLayoutMutation([value](Node &node) { node.border_.bottom = finiteOrZero(value); }); }
    void Node::setOverflow(Overflow overflow) { deferLayoutMutation([overflow](Node &node) { node.overflow_ = overflow; }); }
    Overflow Node::getOverflow() const noexcept { return overflow_; }

    LayoutPosition Node::getActualPosition() const noexcept
    {
        const LayoutPosition position = actualPosition_;
        const CoordinateTransform &transform = coordinateTransform();
        return transform ? transform(*this, position) : position;
    }

    LayoutSize Node::getActualSize() const noexcept { return actualSize_; }
    Node *Node::getVisibleChild(size_t) const noexcept { return nullptr; }

    Node *Node::hitTest(float x, float y) noexcept
    {
        if (!isVisible() || !isEnabled()) return nullptr;
        const float safeX = finiteOrZero(x);
        const float safeY = finiteOrZero(y);
        const LayoutPosition position = getActualPosition();
        const float posX = finiteOrZero(position.x);
        const float posY = finiteOrZero(position.y);
        const float width = finiteOrZero(actualSize_.width);
        const float height = finiteOrZero(actualSize_.height);
        if (safeX >= posX && safeY >= posY && safeX < posX + width && safeY < posY + height) return this;
        return nullptr;
    }
}
