#pragma once

#include <optional>
#include <vector>

#include "input_system.hpp"
#include "node_tree.hpp"
#include "ui_framework/panel_node.hpp"

namespace ui
{
    class ModalSystem
    {
    public:
        ModalSystem();
        ModalSystem(const ModalSystem &) = delete;
        ModalSystem &operator=(const ModalSystem &) = delete;

        bool showModal(NodeTree &nodeTree, InputSystem &input, Node &node);
        bool showModal(NodeTree &nodeTree, InputSystem &input, Node &node, BackdropClickBehavior backdropClickBehavior);
        bool closeModal(NodeTree &nodeTree, InputSystem &input);
        bool handleKeyDown(NodeTree &nodeTree, InputSystem &input, KeyCode key);
        bool handlePointerDown(NodeTree &nodeTree, InputSystem &input, const MousePosition &position, MouseButton button);
        void update(NodeTree &nodeTree, float dt) noexcept;
        void setViewportSize(const LayoutSize &size) noexcept;
        void setBackdropColor(const Color &color) noexcept;
        Color getBackdropColor() const noexcept;
        void setBackdropFadeDuration(float seconds) noexcept;
        float getBackdropFadeDuration() const noexcept;
        void clear(NodeTree &nodeTree, InputSystem &input) noexcept;
        bool isModal(const Node *node) const noexcept;
        Node *topModalNode(NodeTree &nodeTree) const noexcept;
        const Node *topModalNode(const NodeTree &nodeTree) const noexcept;
        Node *backdropNode(NodeTree &nodeTree) const noexcept;
        const Node *backdropNode(const NodeTree &nodeTree) const noexcept;
        void sync(NodeTree &nodeTree, InputSystem &input);

    private:
        class BackdropNode;
        struct ModalSession
        {
            Node::Id modalId{};
            std::optional<Node::Id> previousFocusId;
            std::optional<Node::Id> previousModalId;
            BackdropClickBehavior backdropClickBehavior = BackdropClickBehavior::Consume;
        };
        std::vector<ModalSession> modals_;
        BackdropNode *backdropNode_ = nullptr;
        std::optional<Node::Id> backdropId_;
        LayoutSize viewportSize_{};
        Color backdropColor_{0, 0, 0, 160};
        float backdropOpacity_ = 0.0f;
        float backdropTargetOpacity_ = 0.0f;
        float backdropFadeDuration_ = 0.15f;

        Node *findFirstFocusable(Node &node) const;
        Node *findFirstFocusableInTree(NodeTree &nodeTree) const;
        Node *findValidFocus(NodeTree &nodeTree, std::optional<Node::Id> preferredFocusId) const;
        Node *findFirstFocusableInModal(NodeTree &nodeTree, std::optional<Node::Id> modalId) const;
        bool isNodeUnder(const Node *node, const Node *ancestor) const noexcept;
        void restoreFocusAfterClose(NodeTree &nodeTree, InputSystem &input, const ModalSession &session) const;
        void syncFocusForTopModal(NodeTree &nodeTree, InputSystem &input) const;
        bool isLiveVisibleEnabledModal(NodeTree &nodeTree, const Node &node) const noexcept;
        bool eraseInvalidModalSession(NodeTree &nodeTree, InputSystem &input, size_t index);
        void focusOrClear(NodeTree &nodeTree, InputSystem &input, Node *focus) const;
        void ensureBackdrop(NodeTree &nodeTree);
        void removeBackdrop(NodeTree &nodeTree) noexcept;
        void updateBackdropState() noexcept;
    };
}
