#pragma once

#include <optional>
#include <unordered_map>

#include "ui_framework/core/node.hpp"
#include "ui_framework/types.hpp"

namespace ui
{
    class NodeTree;

    struct ScrollOffset
    {
        float x = 0.0f;
        float y = 0.0f;

        ScrollOffset operator+(const ScrollOffset &other) const noexcept
        {
            return {x + other.x, y + other.y};
        }

        bool operator==(const ScrollOffset &) const = default;
    };

    struct ScrollState
    {
        LayoutSize viewport{};
        LayoutSize content{};
        ScrollOffset offset{};

        ScrollOffset maxOffset() const noexcept;
        void clampOffset() noexcept;
    };

    class ScrollManager
    {
    public:
        ScrollManager() = default;

        ScrollManager(const ScrollManager &) = delete;
        ScrollManager &operator=(const ScrollManager &) = delete;

        bool registerScrollNode(Node &node);
        bool unregisterScrollNode(
            NodeTree &nodeTree,
            Node::Id nodeId);

        bool isRegistered(Node::Id nodeId) const noexcept;

        bool setOffset(
            Node::Id nodeId,
            const ScrollOffset &offset);

        bool scrollBy(
            Node::Id nodeId,
            const ScrollOffset &delta);

        std::optional<ScrollState> getState(
            Node::Id nodeId) const;

        ScrollOffset getOffset(Node::Id nodeId) const noexcept;
        ScrollOffset getMaxOffset(Node::Id nodeId) const noexcept;
        ScrollOffset getAccumulatedOffset(const Node &node) const noexcept;

        Node *findNearestScrollableAncestor(
            NodeTree &nodeTree,
            Node *target) const noexcept;

        bool handleWheel(
            NodeTree &nodeTree,
            float x,
            float y,
            float deltaX,
            float deltaY,
            const Node *modalRoot = nullptr);

        void sync(NodeTree &nodeTree);
        void clear(NodeTree &nodeTree) noexcept;

    private:
        std::unordered_map<Node::Id, ScrollState> states_;
    };
}
