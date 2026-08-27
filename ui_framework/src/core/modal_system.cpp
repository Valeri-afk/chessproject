#include "modal_system.hpp"
#include "node_tree.hpp"
#include "input_system.hpp"
#include "ui_framework/panel_node.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>

namespace ui
{
    ModalSystem::ModalSystem() = default;

    bool ModalSystem::showModal(NodeTree &nodeTree, InputSystem &input, Node &node)
    {
        return showModal(nodeTree, input, node, ModalOptions{});
    }

    bool ModalSystem::showModal(NodeTree &nodeTree, InputSystem &input, Node &node, BackdropClickBehavior backdropClickBehavior)
    {
        ModalOptions options;
        options.outsideClick = backdropClickBehavior;
        return showModal(nodeTree, input, node, options);
    }

    bool ModalSystem::showModal(NodeTree &nodeTree, InputSystem &input, Node &node, const ModalOptions &options)
    {
        if (!nodeTree.isNodeLive(node.getId()) ||
            !nodeTree.isOverlay(&node) ||
            !node.isVisible() ||
            !node.isEnabled() ||
            isModal(&node))
            return false;

        const Node::Id modalId = node.getId();
        const std::optional<Node::Id> previousFocusId = input.focusedNodeId();
        const std::optional<Node::Id> previousModalId =
            modals_.empty() ? std::nullopt : std::optional<Node::Id>(modals_.back().modalId);

        input.cancelPointerInteraction(nodeTree);

        Node *liveModal = nodeTree.findNode(modalId);
        if (!liveModal || !liveModal->isVisible() || !liveModal->isEnabled())
        {
            input.syncState(nodeTree);
            return false;
        }

        modals_.push_back({modalId, previousFocusId, previousModalId, options});
        input.setModalRoot(liveModal);

        if (options.showBackdrop)
            ensureBackdrop(nodeTree);

        startBackdropAnimation(nodeTree);

        if (liveModal->isFocusable())
        {
            if (!input.focus(nodeTree, *liveModal))
                focusOrClear(nodeTree, input, findFirstFocusable(*liveModal));
        }
        else
        {
            focusOrClear(nodeTree, input, findFirstFocusable(*liveModal));
        }

        input.syncState(nodeTree);
        return true;
    }

    bool ModalSystem::closeModal(NodeTree &nodeTree, InputSystem &input)
    {
        if (modals_.empty())
            return false;

        const ModalSession session = modals_.back();
        modals_.pop_back();

        input.cancelPointerInteraction(nodeTree);
        input.clearFocus(nodeTree);

        if (Node *nextModal = topModalNode(nodeTree))
            input.setModalRoot(nextModal);
        else
            input.setModalRoot(nullptr);

        restoreFocusAfterClose(nodeTree, input, session);
        startBackdropAnimation(nodeTree);
        input.syncState(nodeTree);
        return true;
    }

    bool ModalSystem::handleKeyDown(NodeTree &nodeTree, InputSystem &input, KeyCode key)
    {
        return handleKeyEvent(nodeTree, input, key, false);
    }

    bool ModalSystem::handleKeyEvent(NodeTree &nodeTree, InputSystem &input, KeyCode key, bool propagationStopped)
    {
        if (modals_.empty() || propagationStopped)
            return false;

        Node *topModal = topModalNode(nodeTree);
        if (!topModal)
        {
            sync(nodeTree, input);
            return false;
        }

        if (key == KeyCode::ESCAPE)
        {
            if (!modals_.back().options.closeOnEscape)
                return false;
            return closeModal(nodeTree, input);
        }

        if (key == KeyCode::TAB)
        {
            Node *focused = input.focusedNode();
            if (!focused || !isNodeUnder(focused, topModal))
            {
                focusOrClear(nodeTree, input, findFirstFocusable(*topModal));
                return true;
            }

            Node *next = findNextFocusableInModal(nodeTree, *topModal, *focused);
            if (!next)
                next = findFirstFocusable(*topModal);

            if (next)
            {
                input.focus(nodeTree, *next);
                return true;
            }

            return true;
        }

        return false;
    }

    bool ModalSystem::handlePointerDown(NodeTree &nodeTree, InputSystem &input, const MousePosition &position, MouseButton)
    {
        if (modals_.empty())
            return false;

        Node *modalRoot = topModalNode(nodeTree);
        if (!modalRoot)
        {
            sync(nodeTree, input);
            return false;
        }

        if (nodeTree.hitTest(position.x, position.y, modalRoot))
            return false;

        const OutsideClickBehavior behavior = modals_.back().options.outsideClick;
        input.cancelPointerInteraction(nodeTree, position);

        if (behavior == OutsideClickBehavior::Close)
            closeModal(nodeTree, input);

        return true;
    }

    bool ModalSystem::isModal(const Node *node) const noexcept
    {
        if (!node)
            return false;
        const Node::Id id = node->getId();
        return std::any_of(modals_.begin(), modals_.end(),
            [id](const ModalSession &session) { return session.modalId == id; });
    }

    Node *ModalSystem::topModalNode(NodeTree &nodeTree) const noexcept
    {
        return modals_.empty() ? nullptr : nodeTree.findNode(modals_.back().modalId);
    }

    const Node *ModalSystem::topModalNode(const NodeTree &nodeTree) const noexcept
    {
        return modals_.empty() ? nullptr : nodeTree.findNode(modals_.back().modalId);
    }

    Node *ModalSystem::backdropNode(NodeTree &nodeTree) const noexcept
    {
        if (!backdropId_)
            return nullptr;
        return nodeTree.findNode(*backdropId_);
    }

    const Node *ModalSystem::backdropNode(const NodeTree &nodeTree) const noexcept
    {
        if (!backdropId_)
            return nullptr;
        return nodeTree.findNode(*backdropId_);
    }

    void ModalSystem::sync(NodeTree &nodeTree, InputSystem &input)
    {
        for (size_t i = modals_.size(); i > 0; --i)
        {
            if (eraseInvalidModalSession(nodeTree, input, i - 1))
                break;
        }

        updateBackdropState();

        if (Node *topModal = topModalNode(nodeTree))
        {
            input.setModalRoot(topModal);
            syncFocusForTopModal(nodeTree, input);
        }
        else
        {
            input.setModalRoot(nullptr);
        }
    }

    Node *ModalSystem::findFirstFocusable(Node &node) const
    {
        if (!node.isVisible() || !node.isEnabled())
            return nullptr;
        if (node.isFocusable())
            return &node;

        auto *panel = dynamic_cast<PanelNode *>(&node);
        if (!panel)
            return nullptr;

        Node *result = nullptr;
        panel->forEachChild([this, &result](Node &child)
        {
            result = findFirstFocusable(child);
            return result != nullptr;
        });
        return result;
    }

    bool ModalSystem::collectFocusable(Node &node, const Node *, std::vector<Node *> &nodes) const
    {
        if (!node.isVisible() || !node.isEnabled())
            return true;

        if (node.isFocusable())
            nodes.push_back(&node);

        auto *panel = dynamic_cast<PanelNode *>(&node);
        if (!panel)
            return true;

        panel->forEachChild([this, &nodes](Node &child)
        {
            collectFocusable(child, nullptr, nodes);
            return false;
        });
        return true;
    }

    Node *ModalSystem::findNextFocusableInModal(NodeTree &, Node &modal, const Node &current) const
    {
        std::vector<Node *> focusables;
        focusables.reserve(16);
        collectFocusable(modal, &current, focusables);

        if (focusables.empty())
            return nullptr;

        const auto it = std::find(focusables.begin(), focusables.end(), &current);
        if (it == focusables.end())
            return focusables.front();

        const std::size_t index = static_cast<std::size_t>(std::distance(focusables.begin(), it));
        return focusables[(index + 1) % focusables.size()];
    }

    Node *ModalSystem::findFirstFocusableInTree(NodeTree &nodeTree) const
    {
        Node *result = nullptr;
        nodeTree.forEachRoot([this, &result](Node &root)
        {
            result = findFirstFocusable(root);
            return result == nullptr;
        });
        if (!result)
            nodeTree.forEachOverlay([this, &result](Node &overlay)
            {
                result = findFirstFocusable(overlay);
                return result == nullptr;
            });
        return result;
    }

    Node *ModalSystem::findFirstFocusableInModal(NodeTree &nodeTree, std::optional<Node::Id> modalId) const
    {
        if (!modalId)
            return nullptr;
        Node *modal = nodeTree.findNode(*modalId);
        if (!modal || !nodeTree.isOverlay(modal) || !modal->isVisible() || !modal->isEnabled())
            return nullptr;
        return findFirstFocusable(*modal);
    }

    Node *ModalSystem::findValidFocus(NodeTree &nodeTree, std::optional<Node::Id> preferredFocusId) const
    {
        if (preferredFocusId)
        {
            Node *preferred = nodeTree.findNode(*preferredFocusId);
            if (preferred && preferred->isVisible() && preferred->isEnabled() && preferred->isFocusable())
                return preferred;
        }
        return findFirstFocusableInTree(nodeTree);
    }

    bool ModalSystem::isNodeUnder(const Node *node, const Node *ancestor) const noexcept
    {
        for (const Node *current = node; current; current = current->getParent())
            if (current == ancestor)
                return true;
        return false;
    }

    void ModalSystem::restoreFocusAfterClose(NodeTree &nodeTree, InputSystem &input, const ModalSession &session) const
    {
        if (Node *previousModalFocus = findFirstFocusableInModal(nodeTree, session.previousModalId))
        {
            focusOrClear(nodeTree, input, previousModalFocus);
            return;
        }
        focusOrClear(nodeTree, input, findValidFocus(nodeTree, session.previousFocusId));
    }

    void ModalSystem::syncFocusForTopModal(NodeTree &nodeTree, InputSystem &input) const
    {
        if (modals_.empty())
            return;

        Node *topModal = nodeTree.findNode(modals_.back().modalId);
        if (!topModal || !topModal->isVisible())
            return;

        if (!topModal->isEnabled())
        {
            if (input.focusedNode() && isNodeUnder(input.focusedNode(), topModal))
                input.clearFocus(nodeTree);
            return;
        }

        if (!input.focusedNode())
        {
            focusOrClear(nodeTree, input, findFirstFocusable(*topModal));
            return;
        }

        if (!isNodeUnder(input.focusedNode(), topModal))
            focusOrClear(nodeTree, input, findFirstFocusable(*topModal));
    }

    bool ModalSystem::isLiveVisibleEnabledModal(NodeTree &nodeTree, const Node &node) const noexcept
    {
        const Node *liveNode = nodeTree.findNode(node.getId());
        return liveNode && nodeTree.isOverlay(liveNode) && liveNode->isVisible() && liveNode->isEnabled();
    }

    bool ModalSystem::eraseInvalidModalSession(NodeTree &nodeTree, InputSystem &input, size_t index)
    {
        if (index >= modals_.size())
            return false;

        const ModalSession session = modals_[index];
        Node *modalNode = nodeTree.findNode(session.modalId);
        if (modalNode && isLiveVisibleEnabledModal(nodeTree, *modalNode))
            return false;

        modals_.erase(modals_.begin() + static_cast<std::ptrdiff_t>(index), modals_.end());

        input.cancelPointerInteraction(nodeTree);
        input.clearFocus(nodeTree);
        restoreFocusAfterClose(nodeTree, input, session);
        startBackdropAnimation(nodeTree);
        return true;
    }

    void ModalSystem::focusOrClear(NodeTree &nodeTree, InputSystem &input, Node *focus) const
    {
        if (focus)
            input.focus(nodeTree, *focus);
        else
            input.clearFocus(nodeTree);
    }
}
