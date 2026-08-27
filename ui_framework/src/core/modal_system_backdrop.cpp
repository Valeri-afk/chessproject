#include "modal_system.hpp"
#include "node_tree.hpp"
#include "input_system.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

#include "ui_framework/primitives.hpp"

namespace ui
{
    class ModalSystem::BackdropNode final : public Node
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
            const uint8_t alpha = static_cast<uint8_t>(std::clamp(static_cast<int>(std::lround(static_cast<float>(color_.a) * opacity_)), 0, 255));
            const LayoutPosition position = getActualPosition();
            const LayoutSize size = getActualSize();
            primitives::boxRGBA(renderer, position.x, position.y, position.x + size.width, position.y + size.height, color_.r, color_.g, color_.b, alpha);
        }

    private:
        Color color_{0, 0, 0, 160};
        float opacity_ = 0.0f;
    };

    void ModalSystem::setViewportSize(const LayoutSize &size) noexcept
    {
        viewportSize_ = {std::max(0.0f, size.width), std::max(0.0f, size.height)};
        if (backdropNode_)
            backdropNode_->setViewport(viewportSize_);
    }

    void ModalSystem::setBackdropColor(const Color &color) noexcept
    {
        backdropColor_ = color;
        if (backdropNode_)
            backdropNode_->setBackdrop(backdropColor_, backdropOpacity_);
    }

    Color ModalSystem::getBackdropColor() const noexcept { return backdropColor_; }
    void ModalSystem::setBackdropFadeDuration(float seconds) noexcept { backdropFadeDuration_ = std::max(0.0f, seconds); }
    float ModalSystem::getBackdropFadeDuration() const noexcept { return backdropFadeDuration_; }

    void ModalSystem::ensureBackdrop(NodeTree &nodeTree)
    {
        if (backdropId_)
        {
            Node *liveBackdrop = nodeTree.findNode(*backdropId_);
            if (liveBackdrop && nodeTree.isOverlay(liveBackdrop))
            {
                backdropNode_ = dynamic_cast<BackdropNode *>(liveBackdrop);
                if (backdropNode_)
                {
                    backdropNode_->setViewport(viewportSize_);
                    backdropNode_->setBackdrop(backdropColor_, backdropOpacity_);
                    return;
                }
            }
        }

        backdropNode_ = nullptr;
        auto backdrop = std::make_unique<BackdropNode>();
        backdrop->setFocusable(false);
        backdrop->setCapturable(false);
        backdrop->setViewport(viewportSize_);
        backdrop->setBackdrop(backdropColor_, backdropOpacity_);
        backdropId_ = backdrop->getId();
        Node *raw = nodeTree.attachOverlay(nodeTree.overlaysCount(), std::move(backdrop));
        if (!raw)
        {
            backdropId_.reset();
            return;
        }
        backdropNode_ = static_cast<BackdropNode *>(raw);
    }

    void ModalSystem::removeBackdrop(NodeTree &nodeTree) noexcept
    {
        Node *node = backdropId_ ? nodeTree.findNode(*backdropId_) : backdropNode_;
        backdropNode_ = nullptr;
        backdropId_.reset();
        if (node && nodeTree.isOverlay(node))
            nodeTree.removeOverlay(node);
    }

    void ModalSystem::updateBackdropState() noexcept
    {
        const bool shouldShow = !modals_.empty() && modals_.back().options.showBackdrop;
        backdropTargetOpacity_ = shouldShow ? 1.0f : 0.0f;

        if (backdropNode_)
        {
            backdropNode_->setViewport(viewportSize_);
            backdropNode_->setBackdrop(backdropColor_, backdropOpacity_);
        }
    }

    void ModalSystem::startBackdropAnimation(NodeTree &nodeTree) noexcept
    {
        updateBackdropState();

        if (backdropTargetOpacity_ > 0.0f)
            ensureBackdrop(nodeTree);

        if (!backdropNode_)
            return;

        backdropNode_->animateFloat(
            &backdropOpacity_,
            backdropOpacity_,
            backdropTargetOpacity_,
            backdropFadeDuration_,
            AnimationEasing::EaseOut,
            [this](float value)
            {
                backdropOpacity_ = std::clamp(value, 0.0f, 1.0f);
                if (backdropNode_)
                    backdropNode_->setBackdrop(backdropColor_, backdropOpacity_);
            });

        if (backdropTargetOpacity_ <= 0.0f && backdropOpacity_ <= 0.0f)
            removeBackdrop(nodeTree);
    }

    void ModalSystem::clear(NodeTree &nodeTree, InputSystem &input) noexcept
    {
        modals_.clear();
        input.cancelPointerInteraction(nodeTree);
        input.clearFocus(nodeTree);
        backdropTargetOpacity_ = 0.0f;
        backdropOpacity_ = 0.0f;
        if (backdropNode_)
            backdropNode_->cancelAnimation(&backdropOpacity_);
        removeBackdrop(nodeTree);
    }
}
