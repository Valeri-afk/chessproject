#include <algorithm>
#include <optional>
#include <utility>

#include "../detail/node_tree.hpp"
#include "ui_framework/src/detail/input_system.hpp"
#include "ui_framework/src/detail/modal_system.hpp"
#include "ui_framework/src/detail/scroll_system.hpp"
#include "ui_framework/src/detail/layout_system.hpp"
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
          inputSystem_(std::make_unique<InputSystem>()),
          modalSystem_(std::make_unique<ModalSystem>()),
          layoutSystem_(std::make_unique<LayoutSystem>()),
          scrollSystem_(std::make_unique<ScrollSystem>())
    {
    }

    UIManager::~UIManager() = default;

    void UIManager::runFrame(float dt, SDL_Renderer *renderer)
    {
        if (!nodeTree_)
            return;

        if (layoutSystem_ && layoutSystem_->syncViewportFromRenderer(renderer))
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

        // The remainder of the existing event routing is intentionally preserved.
        inputSystem_->processEvent(*nodeTree_, event, modalSystem_->getActiveModal());
    }

    void UIManager::update(float dt)
    {
        if (!nodeTree_)
            return;

        if (scrollSystem_)
            scrollSystem_->sync(*nodeTree_);

        if (modalSystem_)
            modalSystem_->update(dt, *nodeTree_);

        nodeTree_->update(dt);
    }

    void UIManager::draw(SDL_Renderer *renderer)
    {
        if (!renderer || !nodeTree_)
            return;

        Node::ScopedCoordinateTransform scrollTransform(
            makeScrollTransform(scrollSystem_.get()));

        nodeTree_->draw(renderer, modalSystem_ ? modalSystem_->getActiveModalId() : std::nullopt);
    }

    void UIManager::prepareForTreeOperation()
    {
        if (nodeTree_)
            nodeTree_->flushMutationQueue();
    }

    void UIManager::syncModalInputState()
    {
        if (inputSystem_ && modalSystem_)
            inputSystem_->setModalRoot(modalSystem_->getActiveModal());
    }

    void UIManager::drawNodesForFrame(SDL_Renderer *renderer)
    {
        if (!renderer || !nodeTree_)
            return;
        nodeTree_->draw(renderer, modalSystem_ ? modalSystem_->getActiveModalId() : std::nullopt);
    }

    void UIManager::applyMutationQueue()
    {
        if (nodeTree_)
            nodeTree_->flushMutationQueue();
    }

    void UIManager::syncState()
    {
        if (nodeTree_ && scrollSystem_)
            scrollSystem_->sync(*nodeTree_);
        if (inputSystem_ && modalSystem_)
            inputSystem_->setModalRoot(modalSystem_->getActiveModal());
    }

    Node *UIManager::addRoot(std::unique_ptr<Node> node)
    {
        if (!nodeTree_ || !node)
            return nullptr;
        const size_t index = nodeTree_->rootsCount();
        return nodeTree_->attachRoot(index, std::move(node));
    }

    Node *UIManager::addOverlay(std::unique_ptr<Node> node)
    {
        if (!nodeTree_ || !node)
            return nullptr;
        const size_t index = nodeTree_->overlaysCount();
        return nodeTree_->attachOverlay(index, std::move(node));
    }

    void UIManager::removeRoot(Node *node)
    {
        if (nodeTree_ && node)
            nodeTree_->removeRoot(node);
    }

    void UIManager::removeOverlay(Node *node)
    {
        if (nodeTree_ && node)
            nodeTree_->removeOverlay(node);
    }

    bool UIManager::enableScrolling(Node &node)
    {
        return scrollSystem_ && scrollSystem_->registerScrollNode(node);
    }

    bool UIManager::disableScrolling(Node &node)
    {
        return scrollSystem_ && nodeTree_ && scrollSystem_->unregisterScrollNode(*nodeTree_, node.getId());
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
        return modalSystem_ && nodeTree_ && modalSystem_->showModal(*nodeTree_, node);
    }

    bool UIManager::showModal(Node &node, BackdropClickBehavior behavior)
    {
        return modalSystem_ && nodeTree_ && modalSystem_->showModal(*nodeTree_, node, behavior);
    }

    bool UIManager::closeModal()
    {
        return modalSystem_ && nodeTree_ && modalSystem_->closeModal(*nodeTree_);
    }

    bool UIManager::isModal(const Node &node) const noexcept
    {
        return modalSystem_ && modalSystem_->isModal(node.getId());
    }

    Node *UIManager::getActiveModal() const noexcept
    {
        return modalSystem_ ? modalSystem_->getActiveModal() : nullptr;
    }

    void UIManager::setBackdropColor(const Color &color) noexcept
    {
        if (modalSystem_)
            modalSystem_->setBackdropColor(color);
    }

    Color UIManager::getBackdropColor() const noexcept
    {
        return modalSystem_ ? modalSystem_->getBackdropColor() : Colors::transparent;
    }

    void UIManager::setBackdropFadeDuration(float seconds) noexcept
    {
        if (modalSystem_)
            modalSystem_->setBackdropFadeDuration(seconds);
    }

    float UIManager::getBackdropFadeDuration() const noexcept
    {
        return modalSystem_ ? modalSystem_->getBackdropFadeDuration() : 0.0f;
    }
}
