#pragma once

#include <vector>

#include "ui_framework/core/node.hpp"
#include "nodetree.hpp"
#include "ui_framework/event_types.hpp"

namespace ui
{

    class EventDispatcher
    {
    public:
        template <typename Event>
        static void dispatch(
            NodeTree &nodeTree,
            Node *target,
            Event &event,
            bool tunneling,
            bool bubbling)
        {
            if (!target)
                return;

            if (nodeTree.findNode(target->id()) != target)
                return;

            event.target = target;
            event.currentTarget = nullptr;
            event.propagationStopped = false;

            if (!tunneling && !bubbling)
            {
                dispatchTarget(nodeTree, target, event);
                return;
            }

            std::vector<Node::Id> pathIds;
            pathIds.reserve(32);

            Node *current = target;

            while (current)
            {
                pathIds.push_back(current->id());
                current = current->parent();
            }

            if (tunneling)
            {
                event.phase = UIEvent::Phase::TUNNELING;

                for (auto it = pathIds.rbegin(); it != pathIds.rend(); ++it)
                {
                    const Node::Id nodeId = *it;

                    if (nodeId == target->id())
                        break;

                    Node *node = nodeTree.findNode(nodeId);

                    if (!node)
                        return;

                    event.currentTarget = node;

                    node->dispatchEvent(event, nodeTree);

                    if (event.propagationStopped)
                        return;

                    if (!nodeTree.findNode(nodeId))
                        return;
                }
            }

            if (!nodeTree.findNode(target->id()))
                return;

            dispatchTarget(nodeTree, target, event);

            if (event.propagationStopped)
                return;

            if (!nodeTree.findNode(target->id()))
                return;

            if (bubbling)
            {
                event.phase = UIEvent::Phase::BUBBLING;

                for (std::size_t index = 1; index < pathIds.size(); ++index)
                {
                    const Node::Id nodeId = pathIds[index];

                    Node *node = nodeTree.findNode(nodeId);

                    if (!node)
                        return;

                    event.currentTarget = node;

                    node->dispatchEvent(event, nodeTree);

                    if (event.propagationStopped)
                        return;

                    if (!nodeTree.findNode(nodeId))
                        return;
                }
            }
        }

    private:
        template <typename Event>
        static void dispatchTarget(
            NodeTree &nodeTree,
            Node *target,
            Event &event)
        {
            const Node::Id targetId = target->id();

            event.phase = UIEvent::Phase::TARGET;
            event.currentTarget = target;

            target->dispatchEvent(event, nodeTree);

            if (!nodeTree.findNode(targetId))
                return;
        }
    };

}