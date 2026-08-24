#include "modal_system.hpp"
#include "node_tree.hpp"
#include "input_system.hpp"
#include "panel_node.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>

namespace ui
{
    ModalSystem::ModalSystem() = default;

    bool ModalSystem::showModal(NodeTree &nodeTree, InputSystem &input, Node &node)
    {
        return showModal(nodeTree, input, node, BackdropClickBehavior::Consume);
    }

    bool ModalSystem::showModal(NodeTree &nodeTree, InputSystem &input, Node &node, BackdropClickBehavior backdropClickBehavior)
    {
        if (!nodeTree.isNodeLive(node.getId()) || !nodeTree.isOverlay(&node) || !node.isVisible() || !node.isEnabled() || isModal(&node)) return false;
        const Node::Id modalId = node.getId();
        const std::optional<Node::Id> previousFocusId = input.focusedNodeId();
        const std::optional<Node::Id> previousModalId = modals_.empty() ? std::nullopt : std::optional<Node::Id>(modals_.back().modalId);
        input.cancelPointerInteraction(nodeTree);
        Node *liveModal = nodeTree.findNode(modalId);
        if (!liveModal || !liveModal->isVisible() || !liveModal->isEnabled()) { input.syncState(nodeTree); return false; }
        modals_.push_back({modalId, previousFocusId, previousModalId, backdropClickBehavior});
        ensureBackdrop(nodeTree);
        updateBackdropState();
        if (Node *focus = findFirstFocusable(*liveModal)) { if (!input.focus(nodeTree, *focus)) input.clearFocus(nodeTree); }
        else input.clearFocus(nodeTree);
        return true;
    }

    bool ModalSystem::closeModal(NodeTree &nodeTree, InputSystem &input)
    {
        if (modals_.empty()) return false;
        ModalSession session = modals_.back();
        modals_.pop_back();
        input.cancelPointerInteraction(nodeTree);
        input.clearFocus(nodeTree);
        restoreFocusAfterClose(nodeTree, input, session);
        updateBackdropState();
        return true;
    }

    bool ModalSystem::handleKeyDown(NodeTree &nodeTree, InputSystem &input, KeyCode key)
    {
        if (key != KeyCode::ESCAPE || modals_.empty()) return false;
        if (!topModalNode(nodeTree)) { sync(nodeTree, input); return false; }
        return closeModal(nodeTree, input);
    }

    bool ModalSystem::handlePointerDown(NodeTree &nodeTree, InputSystem &input, const MousePosition &position, MouseButton)
    {
        if (modals_.empty()) return false;
        Node *modalRoot = topModalNode(nodeTree);
        if (!modalRoot) { sync(nodeTree, input); return false; }
        if (nodeTree.hitTest(position.x, position.y, modalRoot)) return false;
        const BackdropClickBehavior behavior = modals_.back().backdropClickBehavior;
        input.cancelPointerInteraction(nodeTree, position);
        if (behavior == BackdropClickBehavior::Close) closeModal(nodeTree, input);
        return true;
    }

    bool ModalSystem::isModal(const Node *node) const noexcept
    {
        if (!node) return false;
        const Node::Id id = node->getId();
        return std::any_of(modals_.begin(), modals_.end(), [id](const ModalSession &session) { return session.modalId == id; });
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
        if (!backdropId_) return nullptr;
        return nodeTree.findNode(*backdropId_);
    }

    const Node *ModalSystem::backdropNode(const NodeTree &nodeTree) const noexcept
    {
        if (!backdropId_) return nullptr;
        return nodeTree.findNode(*backdropId_);
    }

    void ModalSystem::sync(NodeTree &nodeTree, InputSystem &input)
    {
        for (size_t i = modals_.size(); i > 0; --i)
            eraseInvalidModalSession(nodeTree, input, i - 1);
        updateBackdropState();
        if (!modals_.empty())
            syncFocusForTopModal(nodeTree, input);
    }

    Node *ModalSystem::findFirstFocusable(Node &node) const
    {
        if (!node.isVisible() || !node.isEnabled()) return nullptr;
        if (node.isFocusable()) return &node;
        auto *panel = dynamic_cast<PanelNode *>(&node);
        if (!panel) return nullptr;
        Node *result = nullptr;
        panel->forEachChild([this, &result](Node &child) { result = findFirstFocusable(child); return result != nullptr; });
        return result;
    }

    Node *ModalSystem::findFirstFocusableInTree(NodeTree &nodeTree) const
    {
        Node *result = nullptr;
        nodeTree.forEachRoot([this, &result](Node &root) { result = findFirstFocusable(root); return result != nullptr; });
        if (!result) nodeTree.forEachOverlay([this, &result](Node &overlay) { result = findFirstFocusable(overlay); return result != nullptr; });
        return result;
    }

    Node *ModalSystem::findFirstFocusableInModal(NodeTree &nodeTree, std::optional<Node::Id> modalId) const
    {
        if (!modalId) return nullptr;
        Node *modal = nodeTree.findNode(*modalId);
        if (!modal || !nodeTree.isOverlay(modal) || !modal->isVisible() || !modal->isEnabled()) return nullptr;
        return findFirstFocusable(*modal);
    }

    Node *ModalSystem::findValidFocus(NodeTree &nodeTree, std::optional<Node::Id> preferredFocusId) const
    {
        if (preferredFocusId)
        {
            Node *preferred = nodeTree.findNode(*preferredFocusId);
            if (preferred && preferred->isVisible() && preferred->isEnabled() && preferred->isFocusable()) return preferred;
        }
        return findFirstFocusableInTree(nodeTree);
    }

    bool ModalSystem::isNodeUnder(const Node *node, const Node *ancestor) const noexcept
    {
        for (const Node *current = node; current; current = current->getParent())
            if (current == ancestor) return true;
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
        if (modals_.empty()) return;
        Node *topModal = nodeTree.findNode(modals_.back().modalId);
        if (!topModal || !topModal->isVisible()) return;
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
        if (input.focusedNode() == topModal)
        {
            if (!topModal->isFocusable())
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
        ModalSession &session = modals_[index];
        Node *modalNode = nodeTree.findNode(session.modalId);
        const bool wasTop = index + 1 == modals_.size();
        if (modalNode && isLiveVisibleEnabledModal(nodeTree, *modalNode)) return false;
        const ModalSession removedSession = session;
        modals_.erase(modals_.begin() + static_cast<std::ptrdiff_t>(index));
        if (wasTop)
        {
            input.cancelPointerInteraction(nodeTree);
            input.syncState(nodeTree);
            restoreFocusAfterClose(nodeTree, input, removedSession);
        }
        return true;
    }

    void ModalSystem::focusOrClear(NodeTree &nodeTree, InputSystem &input, Node *focus) const
    {
        if (focus) input.focus(nodeTree, *focus);
        else input.clearFocus(nodeTree);
    }
}
