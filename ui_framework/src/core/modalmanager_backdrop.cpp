#include "modalmanager.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

#include "ui_framework/core/primitives.hpp"

namespace ui
{
    class ModalManager::BackdropNode final : public Node
    {
    public:
        void setBackdrop(const Color &color, float opacity) noexcept
        {
            color_ = color;
            opacity_ = std::clamp(opacity, 0.0f, 1.0f);
        }

        void setViewport(const LayoutSize &size)
        {
            setPosition({0.0f, 0.0f});
            setPositionMode(PositionMode::Absolute);
            setSize(LayoutSizeValue::fixed(size.width, size.height));
        }

    protected:
        void draw(SDL_Renderer *renderer) override
        {
            if (!renderer || opacity_ <= 0.0f)
                return;

            const uint8_t alpha = static_cast<uint8_t>(
                std::clamp(
                    static_cast<int>(std::lround(
                        static_cast<float>(color_.a) * opacity_)),
                    0,
                    255));

            const LayoutPosition position = getActualPosition();
            const LayoutSize size = getActualSize();

            primitives::boxRGBA(
                renderer,
                position.x,
                position.y,
                position.x + size.width,
                position.y + size.height,
                color_.r,
                color_.g,
                color_.b,
                alpha);
        }

    private:
        Color color_{0, 0, 0, 160};
        float opacity_ = 0.0f;
    };

    void ModalManager::setViewportSize(const LayoutSize &size) noexcept
    {
        viewportSize_ = {
            std::max(0.0f, size.width),
            std::max(0.0f, size.height)};

        if (backdropNode_)
            backdropNode_->setViewport(viewportSize_);
    }

    void ModalManager::setBackdropColor(const Color &color) noexcept
    {
        backdropColor_ = color;

        if (backdropNode_)
        {
            backdropNode_->setBackdrop(
                backdropColor_,
                backdropOpacity_);
        }
    }

    Color ModalManager::getBackdropColor() const noexcept
    {
        return backdropColor_;
    }

    void ModalManager::setBackdropFadeDuration(float seconds) noexcept
    {
        backdropFadeDuration_ = std::max(0.0f, seconds);
    }

    float ModalManager::getBackdropFadeDuration() const noexcept
    {
        return backdropFadeDuration_;
    }

    void ModalManager::ensureBackdrop(NodeTree &nodeTree)
    {
        if (backdropId_)
        {
            Node *liveBackdrop =
                nodeTree.findNode(*backdropId_);

            if (liveBackdrop &&
                nodeTree.isOverlay(liveBackdrop))
            {
                backdropNode_ =
                    dynamic_cast<BackdropNode *>(liveBackdrop);

                if (backdropNode_)
                {
                    backdropNode_->setViewport(viewportSize_);
                    backdropNode_->setBackdrop(
                        backdropColor_,
                        backdropOpacity_);
                    return;
                }
            }
        }

        backdropNode_ = nullptr;

        auto backdrop = std::make_unique<BackdropNode>();
        backdrop->setFocusable(false);
        backdrop->setCapturable(false);
        backdrop->setViewport(viewportSize_);
        backdrop->setBackdrop(
            backdropColor_,
            backdropOpacity_);

        const Node::Id newBackdropId = backdrop->id();

        // Keep the ID even when attachOverlay is deferred by NodeTree's
        // mutation guard. The next synchronized frame will resolve it into
        // backdropNode_ after the queued mutation has been applied.
        backdropId_ = newBackdropId;

        Node *raw = nodeTree.attachOverlay(
            nodeTree.overlaysCount(),
            std::move(backdrop));

        if (!raw)
            return;

        backdropNode_ =
            static_cast<BackdropNode *>(raw);
    }

    void ModalManager::removeBackdrop(NodeTree &nodeTree) noexcept
    {
        Node *node = nullptr;

        if (backdropId_)
            node = nodeTree.findNode(*backdropId_);
        else
            node = backdropNode_;

        backdropNode_ = nullptr;
        backdropId_.reset();

        if (node && nodeTree.isOverlay(node))
            nodeTree.removeOverlay(node);
    }

    void ModalManager::updateBackdropState() noexcept
    {
        backdropTargetOpacity_ = modals_.empty() ? 0.0f : 1.0f;

        if (backdropNode_)
        {
            backdropNode_->setViewport(viewportSize_);
            backdropNode_->setBackdrop(
                backdropColor_,
                backdropOpacity_);
        }
    }

    void ModalManager::update(
        NodeTree &nodeTree,
        float dt) noexcept
    {
        backdropTargetOpacity_ = modals_.empty() ? 0.0f : 1.0f;

        if (backdropFadeDuration_ <= 0.0f)
        {
            backdropOpacity_ = backdropTargetOpacity_;
        }
        else
        {
            const float step = std::max(0.0f, dt) /
                               backdropFadeDuration_;

            if (backdropOpacity_ < backdropTargetOpacity_)
            {
                backdropOpacity_ = std::min(
                    backdropTargetOpacity_,
                    backdropOpacity_ + step);
            }
            else if (backdropOpacity_ > backdropTargetOpacity_)
            {
                backdropOpacity_ = std::max(
                    backdropTargetOpacity_,
                    backdropOpacity_ - step);
            }
        }

        if (!modals_.empty())
        {
            ensureBackdrop(nodeTree);
        }
        else if (backdropOpacity_ <= 0.0f)
        {
            removeBackdrop(nodeTree);
        }

        if (backdropNode_)
        {
            backdropNode_->setViewport(viewportSize_);
            backdropNode_->setBackdrop(
                backdropColor_,
                backdropOpacity_);
        }
    }

    void ModalManager::clear(
        NodeTree &nodeTree,
        InputManager &input) noexcept
    {
        modals_.clear();
        input.cancelPointerInteraction(nodeTree);
        input.clearFocus(nodeTree);

        backdropTargetOpacity_ = 0.0f;
        backdropOpacity_ = 0.0f;
        removeBackdrop(nodeTree);
    }

}