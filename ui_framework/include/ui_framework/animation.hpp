#pragma once

#include <functional>
#include <memory>

namespace ui
{
    class Node;

    enum class AnimationEasing
    {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut
    };

    // A small capability describing one float value that can be transitioned.
    // Properties created by Nodes carry the Node lifetime automatically. Properties
    // created for client-owned values are intentionally non-owning: the caller must
    // keep the referenced value alive while the animation is active.
    class FloatAnimationProperty
    {
    public:
        using PropertyKey = const void *;
        using Getter = std::function<float()>;
        using Setter = std::function<void(float)>;

        FloatAnimationProperty() = default;

        static FloatAnimationProperty from(float &value) noexcept;

        explicit operator bool() const noexcept { return key_ != nullptr && static_cast<bool>(setter_); }

    private:
        friend class Node;
        friend class AnimationSystem;
        friend class AnimationController;

        FloatAnimationProperty(
            Node *owner,
            PropertyKey key,
            Getter getter,
            Setter setter,
            std::weak_ptr<void> lifetime)
            : owner_(owner), key_(key), getter_(std::move(getter)), setter_(std::move(setter)), lifetime_(std::move(lifetime)) {}

        Node *owner_ = nullptr;
        PropertyKey key_ = nullptr;
        Getter getter_;
        Setter setter_;
        std::weak_ptr<void> lifetime_;
    };
}
