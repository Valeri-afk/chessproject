#pragma once

#include <cstddef>
#include <memory>

#include <SDL3/SDL.h>

#include "ui_framework/panel_node.hpp"
#include "ui_framework/types.hpp"
#include "../../src/detail/scroll_system.hpp"
#include "../../src/detail/layout_system.hpp"

namespace ui
{
    class NodeTree;
    class InputManager;
    class ModalManager;

    enum class BackdropClickBehavior;

    class UIManager
    {
    public:
        UIManager();
        ~UIManager();

        UIManager(const UIManager &) = delete;
        UIManager &operator=(const UIManager &) = delete;

        void runFrame(float dt, SDL_Renderer *renderer);
        void processEvent(const SDL_Event &event, SDL_Renderer *renderer);

        Node *addRoot(std::unique_ptr<Node> node);
        Node *addOverlay(std::unique_ptr<Node> node);
        void removeRoot(Node *node);
        void removeOverlay(Node *node);

        bool enableScrolling(Node &node);
        bool disableScrolling(Node &node);
        bool isScrollingEnabled(const Node &node) const noexcept;
        bool setScrollOffset(Node &node, const ScrollOffset &offset);
        ScrollOffset getScrollOffset(const Node &node) const noexcept;
        ScrollOffset getMaximumScrollOffset(const Node &node) const noexcept;

        bool showModal(Node &node);
        bool showModal(Node &node, BackdropClickBehavior behavior);
        bool closeModal();
        bool isModal(const Node &node) const noexcept;
        Node *getActiveModal() const noexcept;

        void setBackdropColor(const Color &color) noexcept;
        Color getBackdropColor() const noexcept;
        void setBackdropFadeDuration(float seconds) noexcept;
        float getBackdropFadeDuration() const noexcept;

    private:
        void update(float dt);
        void draw(SDL_Renderer *renderer);
        void prepareForTreeOperation();
        void syncModalInputState();
        void drawNodesForFrame(SDL_Renderer *renderer);
        void applyMutationQueue();
        void syncState();

        std::unique_ptr<NodeTree> nodeTree_;
        std::unique_ptr<InputManager> inputManager_;
        std::unique_ptr<ModalManager> modalManager_;
        std::unique_ptr<LayoutSystem> layoutSystem_;
        std::unique_ptr<ScrollSystem> scrollSystem_;
    };
}
