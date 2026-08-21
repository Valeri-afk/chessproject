#include <algorithm>
#include <optional>
#include <utility>

#include "nodetree.hpp"
#include "inputmanager.hpp"
#include "modalmanager.hpp"
#include "layoutmanager.hpp"
#include "ui_framework/src/detail/scroll_system.hpp"
#include "ui_framework/ui_manager.hpp"

namespace ui
{
    namespace
    {
        Node::CoordinateTransform makeScrollTransform(const ScrollSystem *scrollSystem)
        {
            return [scrollSystem](const Node &node, const LayoutPosition &position)
            {
                if (!scrollSystem)
                    return position;

                const ScrollOffset offset = scrollSystem->getAccumulatedOffset(node);
                return LayoutPosition{position.x - offset.x, position.y - offset.y};
            };
        }
    }

    UIManager::UIManager()
        : nodeTree_(std::make_unique<NodeTree>()),
          inputManager_(std::make_unique<InputManager>()),
          modalManager_(std::make_unique<ModalManager>()),
          layoutManager_(std::make_unique<LayoutManager>()),
          scrollSystem_(std::make_unique<ScrollSystem>())
    {
    }

    UIManager::~UIManager() = default;

    void UIManager::runFrame(float dt, SDL_Renderer *renderer)
    {
        if (!nodeTree_)
            return;

        if (layoutManager_ && layoutManager_->syncViewportFromRenderer(renderer))
            nodeTree_->requestFullLayout();

        syncState();
        prepareForTreeOperation();
        update(dt);
        draw(renderer);
    }

    void UIManager::processEvent(const SDL_Event &sdlEvent, SDL_Renderer *renderer)
    {
        if (!nodeTree_)
            return;

        SDL_Event event = sdlEvent;

        if (renderer && !SDL_ConvertEventToRenderCoordinates(renderer, &event))
            return;

        prepareForTreeOperation();

        if (inputManager_ && modalManager_ && event.type == SDL_EVENT_KEY_DOWN &&
            convertSDLKeyCodeToKeyCode(event.key.key) == KeyCode::ESCAPE)
        {
            if (modalManager_->handleKeyDown(*nodeTree_, *inputManager_, KeyCode::ESCAPE))
            {
                prepareForTreeOperation();
                return;
            }
        }

        if (inputManager_ && modalManager_ && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && getActiveModal())
        {
            const MousePosition position{event.button.x, event.button.y};
            const MouseButton button = static_cast<MouseButton>(event.button.button);
            bool modalHandled = false;

            {
                Node::ScopedCoordinateTransform scrollTransform(makeScrollTransform(scrollSystem_.get()));
                modalHandled = modalManager_->handlePointerDown(*nodeTree_, *inputManager_, position, button);
            }

            if (modalHandled)
            {
                prepareForTreeOperation();
                return;
            }
        }

        if (scrollSystem_ && event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            const float mouseX = static_cast<float>(event.wheel.mouse_x);
            const float mouseY = static_cast<float>(event.wheel.mouse_y);
            const float deltaX = -event.wheel.x;
            const float deltaY = -event.wheel.y;
            bool wheelHandled = false;

            {
                Node::ScopedCoordinateTransform scrollTransform(makeScrollTransform(scrollSystem_.get()));
                wheelHandled = scrollSystem_->handleWheel(*nodeTree_, mouseX, mouseY, deltaX, deltaY, getActiveModal());
            }

            if (wheelHandled)
            {
                prepareForTreeOperation();

                {
                    Node::ScopedCoordinateTransform scrollTransform(makeScrollTransform(scrollSystem_.get()));
                    inputManager_->refreshHover(*nodeTree_, mouseX, mouseY, getActiveModal());
                }

                return;
            }
        }

        if (inputManager_)
        {
            inputManager_->setModalRoot(getActiveModal());

            {
                Node::ScopedCoordinateTransform scrollTransform(makeScrollTransform(scrollSystem_.get()));
                inputManager_->processEvent(event, *nodeTree_, getActiveModal());
            }

            inputManager_->setModalRoot(getActiveModal());
        }

        prepareForTreeOperation();
    }

    void UIManager::update(float dt)
    {
        if (!nodeTree_)
            return;

        if (modalManager_)
            modalManager_->update(*nodeTree_, dt);

        nodeTree_->update(dt);

        if (layoutManager_)
            layoutManager_->processLayoutQueue(*nodeTree_);

        syncState();
    }

    void UIManager::draw(SDL_Renderer *renderer)
    {
        drawNodesForFrame(renderer);
    }

    Node *UIManager::addRoot(std::unique_ptr<Node> node)
    {
        return nodeTree_ ? nodeTree_->attachRoot(nodeTree_->rootsCount(), std::move(node)) : nullptr;
    }

    Node *UIManager::addOverlay(std::unique_ptr<Node> node)
    {
        return nodeTree_ ? nodeTree_->attachOverlay(nodeTree_->overlaysCount(), std::move(node)) : nullptr;
    }

    void UIManager::removeRoot(Node *node)
    {
        if (nodeTree_ && scrollSystem_ && node)
            scrollSystem_->unregisterScrollNode(*nodeTree_, node->getId());
        if (nodeTree_)
            nodeTree_->removeRoot(node);
    }

    void UIManager::removeOverlay(Node *node)
    {
        if (nodeTree_ && scrollSystem_ && node)
            scrollSystem_->unregisterScrollNode(*nodeTree_, node->getId());
        if (nodeTree_)
            nodeTree_->removeOverlay(node);
    }

    bool UIManager::enableScrolling(Node &node)
    {
        if (!nodeTree_ || !scrollSystem_ || nodeTree_->findNode(node.getId()) != &node)
            return false;
        return scrollSystem_->registerScrollNode(node);
    }

    bool UIManager::disableScrolling(Node &node)
    {
        if (!nodeTree_ || !scrollSystem_ || nodeTree_->findNode(node.getId()) != &node)
            return false;
        return scrollSystem_->unregisterScrollNode(*nodeTree_, node.getId());
    }

    bool UIManager::isScrollingEnabled(const Node &node) const noexcept
    {
        return scrollSystem_ && scrollSystem_->isRegistered(node.getId());
    }

    bool UIManager::setScrollOffset(Node &node, const ScrollOffset &offset)
    {
        return scrollSystem_ && scrollSystem_->setOffset(node.getId(), offset);
    }

    ScrollOffset UIManager::getScrollOffset(const Node &node) const noexcept
    {
        return scrollSystem_ ? scrollSystem_->getOffset(node.getId()) : ScrollOffset{};
    }

    ScrollOffset UIManager::getMaximumScrollOffset(const Node &node) const noexcept
    {
        return scrollSystem_ ? scrollSystem_->getMaxOffset(node.getId()) : ScrollOffset{};
    }

    bool UIManager::showModal(Node &node)
    {
        return showModal(node, BackdropClickBehavior::Consume);
    }

    bool UIManager::showModal(Node &node, BackdropClickBehavior behavior)
    {
        if (!nodeTree_ || !modalManager_ || !inputManager_)
            return false;
        prepareForTreeOperation();
        const bool shown = modalManager_->showModal(*nodeTree_, *inputManager_, node, behavior);
        if (shown)
            prepareForTreeOperation();
        return shown;
    }

    bool UIManager::closeModal()
    {
        if (!nodeTree_ || !modalManager_ || !inputManager_)
            return false;
        prepareForTreeOperation();
        const bool closed = modalManager_->closeModal(*nodeTree_, *inputManager_);
        if (closed)
            prepareForTreeOperation();
        return closed;
    }

    bool UIManager::isModal(const Node &node) const noexcept
    {
        return nodeTree_ && modalManager_ && modalManager_->isModal(&node);
    }

    Node *UIManager::getActiveModal() const noexcept
    {
        return nodeTree_ && modalManager_ ? modalManager_->topModalNode(*nodeTree_) : nullptr;
    }

    void UIManager::setBackdropColor(const Color &color) noexcept
    {
        if (modalManager_)
            modalManager_->setBackdropColor(color);
    }

    Color UIManager::getBackdropColor() const noexcept
    {
        return modalManager_ ? modalManager_->getBackdropColor() : Color{};
    }

    void UIManager::setBackdropFadeDuration(float seconds) noexcept
    {
        if (modalManager_)
            modalManager_->setBackdropFadeDuration(seconds);
    }

    float UIManager::getBackdropFadeDuration() const noexcept
    {
        return modalManager_ ? modalManager_->getBackdropFadeDuration() : 0.0f;
    }

    void UIManager::prepareForTreeOperation()
    {
        if (!nodeTree_)
            return;
        applyMutationQueue();
        if (layoutManager_)
            layoutManager_->processLayoutQueue(*nodeTree_);
        syncState();
    }

    void UIManager::syncModalInputState()
    {
        if (inputManager_)
            inputManager_->setModalRoot(getActiveModal());
    }

    void UIManager::drawNodesForFrame(SDL_Renderer *renderer)
    {
        if (!renderer || !nodeTree_)
            return;

        std::optional<Node::Id> topModalId;
        if (modalManager_)
        {
            if (Node *topModal = modalManager_->topModalNode(*nodeTree_))
                topModalId = topModal->getId();
        }

        {
            Node::ScopedCoordinateTransform scrollTransform(makeScrollTransform(scrollSystem_.get()));
            nodeTree_->draw(renderer, topModalId);
        }

        syncState();
    }

    void UIManager::applyMutationQueue()
    {
        if (nodeTree_)
            nodeTree_->flushMutationQueue();
    }

    void UIManager::syncState()
    {
        if (!nodeTree_ || !inputManager_)
            return;

        if (modalManager_)
        {
            if (layoutManager_)
                modalManager_->setViewportSize(layoutManager_->getViewportSize());
            modalManager_->sync(*nodeTree_, *inputManager_);
        }

        if (scrollSystem_)
            scrollSystem_->sync(*nodeTree_);

        inputManager_->syncState(*nodeTree_);
        syncModalInputState();
    }
}
