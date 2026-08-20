#pragma once

#include <SDL3/SDL.h>

#include "nodetree.hpp"
#include "ui_framework/types.hpp"

namespace ui
{
    class LayoutManager
    {
    public:
        LayoutManager();

        LayoutManager(const LayoutManager &) = delete;
        LayoutManager &operator=(const LayoutManager &) = delete;

        void setViewportSize(const LayoutSize &size) noexcept;
        LayoutSize getViewportSize() const noexcept;

        bool syncViewportFromRenderer(SDL_Renderer *renderer);

        void requestFullLayout(NodeTree &nodeTree);
        void processLayoutQueue(NodeTree &nodeTree);

    private:
        void measureRecursive(
            Node &node,
            const LayoutSize &availableBorderBoxSize,
            NodeTree &nodeTree);

        LayoutSize measureTextNode(
            class TextNode &node,
            const LayoutSize &availableContent) const;

        void arrangeRecursive(
            Node &node,
            NodeTree &nodeTree);

        LayoutSize makeRootAvailableSize(const Node &root) const;

        LayoutSize toContentSize(
            const Node &node,
            const LayoutSize &borderBoxSize) const;

        LayoutSize toBorderBoxSize(
            const Node &node,
            const LayoutSize &contentSize) const;

        LayoutSize applyMeasureRules(
            const Node &node,
            const LayoutSize &measuredBorderBoxSize) const;

        void sanitizeLayoutSize(LayoutSize &size) const noexcept;

    private:
        LayoutSize viewportSize_{};
    };

}