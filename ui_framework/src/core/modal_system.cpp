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
        if (!nodeTree.isNodeLive(node.getId()) || !nodeTree.isOverlay(&node) || !node.isVisible() || !node.isEnabled() || isModal(&node)) return false;
        const Node::Id modalId = node.getId();
        const std::optional<Node::Id> previousFocusId = input.focusedNodeId();
        input.cancelPointerInteraction(nodeTree);
        Node *liveModal = nodeTree.findNode(modalId);
        if (!liveModal || !liveModal->isVisible() || !liveModal->isEnabled()) { input.syncState(nodeTree); return false; }
        modals_.push_back({modalId, previousFocusId});
        if (Node *focus = findFirstFocusable(*liveModal)) { if (!input.focus(nodeTree, *focus)) input.clearFocus(nodeTree); }
        else input.clearFocus(nodeTree);
        return true;
    }

    bool ModalSystem::closeModal(NodeTree &nodeTree, InputSystem &input)
    {
        if (modals_.empty()) return false;
        input.cancelPointerInteraction(nodeTree); input.clearFocus(nodeTree);
        const ModalSession removed = modals_.back(); modals_.pop_back();
        restoreFocusAfterClose(nodeTree, input, removed);
        return true;
    }

    bool ModalSystem::handlePointerDown(NodeTree &nodeTree, InputSystem &input, const MousePosition &position, MouseButton)
    {
        if (modals_.empty()) return false;
        Node *modalRoot = topModalNode(nodeTree);
        if (!modalRoot) return false;
        if (nodeTree.hitTest(position.x, position.y, modalRoot)) return false;
        input.cancelPointerInteraction(nodeTree, position);
        return true;
    }

    bool ModalSystem::isModal(const Node *node) const noexcept
    {
        if (!node) return false;
        return std::any_of(modals_.begin(), modals_.end(), [id = node->getId()](const ModalSession &session) { return session.modalId == id; });
    }

    Node *ModalSystem::topModalNode(NodeTree &nodeTree) const
    {
        return modals_.empty() ? nullptr : nodeTree.findNode(modals_.back().modalId);
    }

    const Node *ModalSystem::topModalNode(const NodeTree &nodeTree) const
    {
        return modals_.empty() ? nullptr : nodeTree.findNode(modals_.back().modalId);
    }

    Node *ModalSystem::backdropNode(NodeTree &nodeTree) const
    {
        if (!backdropId_) return nullptr;
        return nodeTree.findNode(*backdropId_);
    }

    const Node *ModalSystem::backdropNode(const NodeTree &nodeTree) const
    {
        if (!backdropId_) return nullptr;
        return nodeTree.findNode(*backdropId_);
    }

    void ModalSystem::setBackdropId(Node::Id id) noexcept { backdropId_ = id; }
    void ModalSystem::clearBackdropId() noexcept { backdropId_.reset(); }

    Node *ModalSystem::findFirstFocusable(Node &node) const
    {
        if (node.isVisible() && node.isEnabled() && node.isFocusable()) return &node;
        auto *panel = dynamic_cast<PanelNode *>(&node);
        if (!panel) return nullptr;
        Node *result = nullptr;
        panel->forEachChild([this, &result](Node &child) { if (!result) result = findFirstFocusable(child); return result == nullptr; });
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
        if (Node *modalFocus = findFirstFocusableInModal(nodeTree, modals_.empty() ? std::nullopt : std::optional<Node::Id>(modals_.back().modalId))) return modalFocus;
        return findFirstFocusableInTree(nodeTree);
    }

    void ModalSystem::restoreFocusAfterClose(NodeTree &nodeTree, InputSystem &input, const ModalSession &session) const
    {
        Node *focus = findValidFocus(nodeTree, session.previousFocusId);
        if (focus) input.focus(nodeTree, *focus); else input.clearFocus(nodeTree);
    }

    void ModalSystem::syncFocusForTopModal(NodeTree &nodeTree, InputSystem &input) const
    {
        if (modals_.empty()) return;
        Node *topModal = nodeTree.findNode(modals_.back().modalId);
        if (!topModal) return;
        if (!topModal->isEnabled()) { if (input.focusedNode() && isNodeUnder(input.focusedNode(), topModal)) input.clearFocus(nodeTree); return; }
        if (!input.focusedNode()) { focusOrClear(nodeTree, input, findFirstFocusable(*topModal)); return; }
        if (input.focusedNode() == topModal) { if (!topModal->isFocusable()) focusOrClear(nodeTree, input, findFirstFocusable(*topModal)); return; }
        if (!isNodeUnder(input.focusedNode(), topModal)) focusOrClear(nodeTree, input, findFirstFocusable(*topModal));
    }

    bool ModalSystem::isLiveVisibleEnabledModal(NodeTree &nodeTree, const Node &node) const
    {
        const Node *liveNode = nodeTree.findNode(node.getId());
        return liveNode && nodeTree.isOverlay(liveNode) && liveNode->isVisible() && liveNode->isEnabled();
    }

    bool ModalSystem::eraseInvalidModalSession(NodeTree &nodeTree, InputSystem &input, size_t index)
    {
        if (index >= modals_.size()) return false;
        const bool wasTop = index + 1 == modals_.size();
        const ModalSession removed = modals_[index];
        Node *modalNode = nodeTree.findNode(removed.modalId);
        const bool valid = modalNode && nodeTree.isOverlay(modalNode) && modalNode->isVisible() && modalNode->isEnabled();
        if (valid) return false;
        modals_.erase(modals_.begin() + static_cast<std::ptrdiff_t>(index));
        if (wasTop) { input.cancelPointerInteraction(nodeTree); input.syncState(nodeTree); restoreFocusAfterClose(nodeTree, input, removed); }
        return true;
    }

    void ModalSystem::focusOrClear(NodeTree &nodeTree, InputSystem &input, Node *focus) const
    {
        if (focus) input.focus(nodeTree, *focus); else input.clearFocus(nodeTree);
    }

    bool ModalSystem::isNodeUnder(const Node *node, const Node *ancestor) const noexcept
    {
        if (!node || !ancestor) return false;
        const Node *current = node;
        while (current)
        {
            if (current == ancestor) return true;
            current = current->getParent();
        }
        return false;
    }
}
