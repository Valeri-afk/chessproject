#include "ui_framework/core/linear_layout.hpp"
#include "ui_framework/core/layout_constraints.hpp"
#include "ui_framework/core/stackpanelnode.hpp"
#include "layoutmanager.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace
{
    constexpr float kInfinity = std::numeric_limits<float>::max();

    float finiteOrZero(float value) noexcept { return std::isfinite(value) ? value : 0.0f; }
    float finiteOrInfinity(float value) noexcept { return std::isfinite(value) ? value : kInfinity; }

    float safeAdd(float a, float b) noexcept
    {
        if (!std::isfinite(a) || !std::isfinite(b)) return kInfinity;
        const float result = a + b;
        return std::isfinite(result) ? result : kInfinity;
    }

    ui::Padding sanitizePadding(ui::Padding padding) noexcept
    {
        padding.left = std::max(0.0f, finiteOrZero(padding.left));
        padding.right = std::max(0.0f, finiteOrZero(padding.right));
        padding.top = std::max(0.0f, finiteOrZero(padding.top));
        padding.bottom = std::max(0.0f, finiteOrZero(padding.bottom));
        return padding;
    }

    ui::Border sanitizeBorder(ui::Border border) noexcept
    {
        border.left = std::max(0.0f, finiteOrZero(border.left));
        border.right = std::max(0.0f, finiteOrZero(border.right));
        border.top = std::max(0.0f, finiteOrZero(border.top));
        border.bottom = std::max(0.0f, finiteOrZero(border.bottom));
        return border;
    }

    ui::LayoutSize paddingBorderSize(const ui::Padding &padding, const ui::Border &border) noexcept
    {
        return {
            safeAdd(safeAdd(padding.left, padding.right), safeAdd(border.left, border.right)),
            safeAdd(safeAdd(padding.top, padding.bottom), safeAdd(border.top, border.bottom))};
    }

    float subtractInset(float value, float inset) noexcept
    {
        return std::max(0.0f, finiteOrInfinity(value) - finiteOrZero(inset));
    }

    ui::LayoutSize subtractPaddingBorder(ui::LayoutSize borderBoxSize, const ui::Padding &padding, const ui::Border &border) noexcept
    {
        const ui::LayoutSize insets = paddingBorderSize(padding, border);
        return {subtractInset(borderBoxSize.width, insets.width), subtractInset(borderBoxSize.height, insets.height)};
    }

    ui::LayoutSize toContentSize(const ui::Node &node, ui::LayoutSize borderBoxSize) noexcept
    {
        return subtractPaddingBorder(borderBoxSize, sanitizePadding(node.getPadding()), sanitizeBorder(node.getBorder()));
    }

    ui::LayoutSize toBorderBoxSize(const ui::Node &node, ui::LayoutSize contentSize) noexcept
    {
        const ui::Padding padding = sanitizePadding(node.getPadding());
        const ui::Border border = sanitizeBorder(node.getBorder());
        const ui::LayoutSize insets = paddingBorderSize(padding, border);
        return {safeAdd(contentSize.width, insets.width), safeAdd(contentSize.height, insets.height)};
    }

    ui::LayoutPosition getContentPosition(const ui::Node &node) noexcept
    {
        const ui::LayoutPosition position = node.getActualPosition();
        const ui::Padding padding = sanitizePadding(node.getPadding());
        const ui::Border border = sanitizeBorder(node.getBorder());
        return {finiteOrZero(position.x) + border.left + padding.left, finiteOrZero(position.y) + border.top + padding.top};
    }

    ui::LayoutSize getContentSize(const ui::Node &node) noexcept
    {
        const ui::LayoutSize size = node.getActualSize();
        const ui::Padding padding = sanitizePadding(node.getPadding());
        const ui::Border border = sanitizeBorder(node.getBorder());
        const ui::LayoutSize insets = paddingBorderSize(padding, border);
        return {std::max(0.0f, finiteOrZero(size.width) - insets.width), std::max(0.0f, finiteOrZero(size.height) - insets.height)};
    }

    bool getLogicalPresentationSize(
        SDL_Renderer *renderer,
        int *width,
        int *height)
    {
        if (SDL_GetRenderLogicalPresentation(
                renderer,
                width,
                height,
                nullptr) &&
            *width > 0 &&
            *height > 0)
        {
            return true;
        }
    
        return false;
    }
    
    bool getRenderOutputSize(
        SDL_Renderer *renderer,
        int *width,
        int *height)
    {
        return SDL_GetCurrentRenderOutputSize(
            renderer,
            width,
            height);
    }
}

namespace ui
{
    LayoutManager::LayoutManager() = default;

    void LayoutManager::setViewportSize(const LayoutSize &size) noexcept { viewportSize_ = sanitizeSize(size); }
    LayoutSize LayoutManager::getViewportSize() const noexcept { return viewportSize_; }

    bool LayoutManager::syncViewportFromRenderer(SDL_Renderer *renderer)
    {
        if (!renderer) return false;

        int width = 0;
        int height = 0;

        // Logical presentation is the framework UI coordinate space.
        // If the application has not configured logical presentation, use
        // the current render output size as a compatibility fallback.
        if (!getLogicalPresentationSize(renderer, &width, &height) &&
            !getRenderOutputSize(renderer, &width, &height))
        {
            return false;
        }

        const LayoutSize newSize{
            static_cast<float>(width),
            static_cast<float>(height)};

        if (newSize == viewportSize_)
            return false;

        viewportSize_ = sanitizeSize(newSize);
        return true;
    }

    void LayoutManager::requestFullLayout(NodeTree &nodeTree) { nodeTree.requestFullLayout(); }

    void LayoutManager::processLayoutQueue(NodeTree &nodeTree)
    {
        {
            NodeTree::ScopedMutationGuard guard(nodeTree);
            nodeTree.forEachLayoutQueue([this, &nodeTree](Node &root)
            {
                if (!root.isVisible()) return;
                const Node::Id rootId = root.id();
                const LayoutSize rootAvailable = makeRootAvailableSize(root);
                measureRecursive(root, rootAvailable, nodeTree);
                Node *liveRoot = nodeTree.findNode(rootId);
                if (!liveRoot) return;
                liveRoot->actualSize_ = internal::resolveFinalSize(*liveRoot, rootAvailable);
                liveRoot->actualPosition_ = liveRoot->position_;
                arrangeRecursive(*liveRoot, nodeTree);
            });
        }
        nodeTree.flushMutationQueue();
    }

    void LayoutManager::measureRecursive(Node &node, const LayoutSize &availableBorderBoxSize, NodeTree &nodeTree)
    {
        if (!node.isVisible()) return;
        const Node::Id nodeId = node.id();
        const LayoutSize availableBorder = internal::resolveMeasurementProposal(node, sanitizeProposal(availableBorderBoxSize));
        const LayoutSize availableContent = toContentSize(node, availableBorder);

        internal::LinearMeasureContext ctx;
        ctx.availableSize = availableContent;
        ctx.measureChild = [this, &nodeTree, &node](size_t visibleChildIndex, const LayoutSize &childAvailableContent) -> LayoutSize
        {
            Node *child = node.getVisibleChild(visibleChildIndex);
            if (!child) return {};
            const Node::Id childId = child->id();
            const LayoutSize childAvailableBorder = toBorderBoxSize(*child, childAvailableContent);
            measureRecursive(*child, childAvailableBorder, nodeTree);
            Node *liveChild = nodeTree.findNode(childId);
            return liveChild ? liveChild->desiredSize_ : LayoutSize{};
        };

        LayoutSize desiredContent{};
        if (auto *stackPanel = dynamic_cast<StackPanelNode *>(&node))
        {
            desiredContent = sanitizeSize(internal::measureLinearPanel(*stackPanel, ctx));
            for (size_t i = 0; i < stackPanel->childCount(); ++i)
            {
                Node *child = stackPanel->getChildAt(i);
                if (!child || !child->isVisible() || child->getPositionMode() != PositionMode::Absolute) continue;
                measureRecursive(*child, toBorderBoxSize(*child, availableContent), nodeTree);
            }
        }
        else
        {
            desiredContent = sanitizeSize(node.measureContent(availableContent));
        }

        Node *liveNode = nodeTree.findNode(nodeId);
        if (!liveNode) return;
        liveNode->desiredSize_ = sanitizeSize(toBorderBoxSize(*liveNode, desiredContent));
    }

    void LayoutManager::arrangeRecursive(Node &node, NodeTree &nodeTree)
    {
        if (!node.isVisible()) return;
        internal::LinearArrangeContext ctx;
        ctx.contentPosition = getContentPosition(node);
        ctx.contentSize = getContentSize(node);
        ctx.placeChild = [this, &nodeTree, &node](size_t visibleChildIndex, const LayoutPosition &position, const LayoutSize &size)
        {
            Node *child = node.getVisibleChild(visibleChildIndex);
            if (!child || !child->isVisible()) return;
            child->actualPosition_ = position;
            child->actualSize_ = internal::resolveFinalSize(*child, sanitizeSize(size));
            arrangeRecursive(*child, nodeTree);
        };

        if (auto *stackPanel = dynamic_cast<StackPanelNode *>(&node))
        {
            internal::arrangeLinearPanel(*stackPanel, ctx);
            for (size_t i = 0; i < stackPanel->childCount(); ++i)
            {
                Node *child = stackPanel->getChildAt(i);
                if (!child || !child->isVisible() || child->getPositionMode() != PositionMode::Absolute) continue;

                const LayoutSize parentContentSize = ctx.contentSize;
                const LayoutSize absoluteProposal = internal::resolveMeasurementProposal(*child, parentContentSize);
                const LayoutSize allocatedSize = child->getSize().width.isValue() || child->getSize().height.isValue()
                    ? absoluteProposal
                    : child->desiredSize_;

                child->actualPosition_ = ctx.contentPosition + child->position_;
                child->actualSize_ = internal::resolveFinalSize(*child, sanitizeSize(allocatedSize));
                arrangeRecursive(*child, nodeTree);
            }
        }

        node.arrangeContent(ctx.contentPosition, ctx.contentSize);
    }

    LayoutSize LayoutManager::makeRootAvailableSize(const Node &root) const
    {
        LayoutSize size = viewportSize_;
        if (root.size_.width.isValue()) size.width = root.size_.width.value;
        if (root.size_.height.isValue()) size.height = root.size_.height.value;
        return sanitizeSize(size);
    }
}
