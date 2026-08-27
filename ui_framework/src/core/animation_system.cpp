#include "animation_system.hpp"

#include <algorithm>
#include <cmath>

namespace ui
{
    bool AnimationSystem::nearlyEqual(float a, float b) noexcept
    {
        return std::fabs(a - b) <= 0.000001f;
    }

    float AnimationSystem::applyEasing(float t, AnimationEasing easing) noexcept
    {
        t = std::clamp(t, 0.0f, 1.0f);

        switch (easing)
        {
        case AnimationEasing::Linear:
            return t;
        case AnimationEasing::EaseIn:
            return t * t;
        case AnimationEasing::EaseOut:
        {
            const float inverse = 1.0f - t;
            return 1.0f - inverse * inverse;
        }
        case AnimationEasing::EaseInOut:
            return t * t * (3.0f - 2.0f * t);
        }

        return t;
    }

    void AnimationSystem::removeFor(Node &owner, PropertyKey property) noexcept
    {
        std::erase_if(
            animations_,
            [&owner, property](const ActiveAnimation &animation)
            {
                return animation.owner == &owner && animation.property == property;
            });
    }

    void AnimationSystem::animateFloat(
        Node &owner,
        PropertyKey property,
        float currentValue,
        float targetValue,
        float duration,
        AnimationEasing easing,
        Setter setter)
    {
        if (!property || !setter)
            return;

        duration = std::max(0.0f, duration);

        // A property has at most one active animation. A new transition
        // replaces the old one and starts from the value currently visible
        // to the component.
        removeFor(owner, property);

        if (duration <= 0.0f || nearlyEqual(currentValue, targetValue))
        {
            setter(targetValue);
            return;
        }

        ActiveAnimation animation;
        animation.id = nextAnimationId_++;
        animation.owner = &owner;
        animation.property = property;
        animation.startValue = currentValue;
        animation.currentValue = currentValue;
        animation.targetValue = targetValue;
        animation.duration = duration;
        animation.easing = easing;
        animation.setter = std::move(setter);

        animations_.push_back(std::move(animation));
    }

    void AnimationSystem::cancel(Node &owner, PropertyKey property) noexcept
    {
        removeFor(owner, property);
    }

    void AnimationSystem::advance(float dt) noexcept
    {
        dt = std::max(0.0f, dt);

        for (std::size_t index = 0; index < animations_.size();)
        {
            ActiveAnimation &animation = animations_[index];

            // Nodes can be removed while other deferred framework work is
            // being processed. Never invoke a property setter after its Node
            // has ceased to belong to this tree. The owning NodeTree removes
            // the animation record on the next advance pass.
            if (!animation.owner)
            {
                animations_.erase(animations_.begin() + static_cast<std::ptrdiff_t>(index));
                continue;
            }

            animation.elapsed = std::min(animation.duration, animation.elapsed + dt);
            const float t = animation.duration > 0.0f
                                ? animation.elapsed / animation.duration
                                : 1.0f;
            const float eased = applyEasing(t, animation.easing);
            animation.currentValue = animation.startValue +
                                     (animation.targetValue - animation.startValue) * eased;

            animation.setter(animation.currentValue);

            if (animation.elapsed >= animation.duration ||
                nearlyEqual(animation.currentValue, animation.targetValue))
            {
                animation.setter(animation.targetValue);
                animations_.erase(animations_.begin() + static_cast<std::ptrdiff_t>(index));
                continue;
            }

            ++index;
        }
    }
}
