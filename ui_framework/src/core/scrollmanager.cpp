#include "scrollmanager.hpp"

#include "ui_framework/core/panelnode.hpp"

#include <algorithm>
#include <functional>

namespace
{
    float sanitizeNonNegative(float value) noexcept
    {
        return std::max(0.0f, value);
    }

    ui::LayoutSize getClientSize(const ui::Node &node)
    {
        const ui::LayoutSize size = node.getActualSize();
        const ui::Padding padding = node.getPadding();
        const ui::Border border = node.getBorder();

        return {
            sanitizeNonNegative(
                size.width -
                std::max(0.0f, padding.left) -
                std::max(0.0f, padding.right) -
                std::max(0.0f, border.left) -
                std::max(0.0f, border.right)),
            sanitizeNonNegative(
                size.height -
                std::max(0.0f, padding.top) -
                std::max(0.0f, padding.bottom) -
                std::max(0.0f, border.top) -
                std::max(0.0f, border.bottom))};
    }

    ui::LayoutPosition getContentOrigin(const ui::Node &node)
    {
        const ui::LayoutPosition position = node.getActualPosition();
        const ui::Padding padding = node.getPadding();
        const ui::Border border = node.getBorder();

        return {
            position.x + std::max(0.0f, border.left) + std::max(0.0f, padding.left),
            position.y + std::max(0.0f, border.top) + std::max(0.0f, padding.top)};
    }
}

namespace ui
{
    ScrollOffset ScrollState::maxOffset() const noexcept
    {
        return {
            std::max(0.0f, content.width - viewport.width),
            std::max(0.0f, content.height - viewport.height)};
    }

    void ScrollState::clampOffset() noexcept
    {
        const ScrollOffset maximum = maxOffset();
        offset.x = std::clamp(offset.x, 0.0f, maximum.x);
        offset.y = std::clamp(offset.y, 0.0f, maximum.y);
    }

    bool ScrollManager::registerScrollNode(Node &node)
    {
        return states_.try_emplace(node.id()).second;
    }

    bool ScrollManager::unregisterScrollNode(
        NodeTree &nodeTree,
        Node::Id nodeId)
    {
        if (!nodeTree.findNode(nodeId))
        {
            states_.erase(nodeId);
            return false;
        }

        return states_.erase(nodeId) != 0;
    }

    bool ScrollManager::isRegistered(Node::Id nodeId) const noexcept
    {
        return states_.find(nodeId) != states_.end();
    }

    bool ScrollManager::setOffset(
        Node::Id nodeId,
        const ScrollOffset &offset)
    {
        auto it = states_.find(nodeId);
        if (it == states_.end())
            return false;

        it->second.offset = offset;
        it->second.clampOffset();
        return true;
    }

    bool ScrollManager::scrollBy(
        Node::Id nodeId,
        const ScrollOffset &delta)
    {
        auto it = states_.find(nodeId);
        if (it == states_.end())
            return false;

        it->second.offset.x += delta.x;
        it->second.offset.y += delta.y;
        it->second.clampOffset();
        return true;
    }

    std::optional<ScrollState> ScrollManager::getState(Node::Id nodeId) const
    {
        const auto it = states_.find(nodeId);
        if (it == states_.end())
            return std::nullopt;
        return it->second;
    }

    ScrollOffset ScrollManager::getOffset(Node::Id nodeId) const noexcept
    {
        const auto it = states_.find(nodeId);
        return it == states_.end() ? ScrollOffset{} : it->second.offset;
    }

    ScrollOffset ScrollManager::getMaxOffset(Node::Id nodeId) const noexcept
    {
        const auto it = states_.find(nodeId);
        return it == states_.end() ? ScrollOffset{} : it->second.maxOffset();
    }

    ScrollOffset ScrollManager::getAccumulatedOffset(const Node &node) const noexcept
    {
        ScrollOffset result{};
        const Node *current = node.parent();

        while (current)
        {
            const auto it = states_.find(current->id());
            if (it != states_.end())
                result = result + it->second.offset;

            current = current->parent();
        }

        return result;
    }

    Node *ScrollManager::findNearestScrollableAncestor(
        NodeTree &nodeTree,
        Node *target) const noexcept
    {
        Node *current = target;
        while (current)
        {
            if (nodeTree.findNode(current->id()) != current)
                return nullptr;

            if (isRegistered(current->id()))
                return current;

            current = current->parent();
        }
        return nullptr;
    }

    bool ScrollManager::handleWheel(
        NodeTree &nodeTree,
        float x,
        float y,
        float deltaX,
        float deltaY,
        const Node *modalRoot)
    {
        Node *target = nodeTree.hitTest(x, y, modalRoot);
        if (!target)
            return false;

        ScrollOffset remaining{deltaX, deltaY};
        bool consumed = false;

        for (Node *current = target; current; current = current->parent())
        {
            if (nodeTree.findNode(current->id()) != current)
                return consumed;

            auto it = states_.find(current->id());
            if (it == states_.end())
                continue;

            const ScrollOffset before = it->second.offset;
            const ScrollOffset maximum = it->second.maxOffset();

            it->second.offset.x = std::clamp(
                it->second.offset.x + remaining.x,
                0.0f,
                maximum.x);
            it->second.offset.y = std::clamp(
                it->second.offset.y + remaining.y,
                0.0f,
                maximum.y);

            const ScrollOffset applied{
                it->second.offset.x - before.x,
                it->second.offset.y - before.y};

            remaining.x -= applied.x;
            remaining.y -= applied.y;

            if (applied.x != 0.0f || applied.y != 0.0f)
                consumed = true;

            if (remaining.x == 0.0f && remaining.y == 0.0f)
                return true;
        }

        return consumed;
    }

    void ScrollManager::sync(NodeTree &nodeTree)
    {
        for (auto it = states_.begin(); it != states_.end();)
        {
            Node *scrollNode = nodeTree.findNode(it->first);
            if (!scrollNode)
            {
                it = states_.erase(it);
                continue;
            }

            ScrollState &state = it->second;
            state.viewport = getClientSize(*scrollNode);
            state.content = state.viewport;

            const LayoutPosition contentOrigin = getContentOrigin(*scrollNode);

            std::function<void(const Node &, bool)> measureSubtree;
            measureSubtree =
                [this, &measureSubtree, &state, &contentOrigin]
                (const Node &node, bool isRoot)
                {
                    if (!node.isVisible())
                        return;

                    if (!isRoot)
                    {
                        const LayoutPosition position = node.getActualPosition();
                        const LayoutSize size = node.getActualSize();

                        state.content.width = std::max(
                            state.content.width,
                            std::max(0.0f, position.x + size.width - contentOrigin.x));
                        state.content.height = std::max(
                            state.content.height,
                            std::max(0.0f, position.y + size.height - contentOrigin.y));

                        if (isRegistered(node.id()))
                            return;
                    }

                    const auto *panel = dynamic_cast<const PanelNode *>(&node);
                    if (!panel)
                        return;

                    for (size_t i = 0; i < panel->childCount(); ++i)
                    {
                        const Node *child = panel->getChildAt(i);
                        if (child)
                            measureSubtree(*child, false);
                    }
                };

            measureSubtree(*scrollNode, true);
            state.clampOffset();
            ++it;
        }
    }

    void ScrollManager::clear(NodeTree &nodeTree) noexcept
    {
        (void)nodeTree;
        states_.clear();
    }
}
