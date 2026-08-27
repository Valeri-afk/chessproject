#include "animation_system.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

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
        case AnimationEasing::Linear: return t;
        case AnimationEasing::EaseIn: return t * t;
        case AnimationEasing::EaseOut:
        {
            const float inverse = 1.0f - t;
            return 1.0f - inverse * inverse;
        }
        case AnimationEasing::EaseInOut: return t * t * (3.0f - 2.0f * t);
        }
        return t;
    }

    void AnimationSystem::removeFor(OwnerKey owner, PropertyKey property) noexcept
    {
        std::erase_if(animations_, [owner, property](const ActiveAnimation &animation)
        {
            return animation.owner == owner && animation.property == property;
        });
    }

    void AnimationSystem::animateFloat(
        OwnerKey owner,
        PropertyKey property,
        std::weak_ptr<void> lifetime,
        float currentValue,
        float targetValue,
        float duration,
        AnimationEasing easing,
        Setter setter)
    {
        if (!owner || !property || lifetime.expired() || !setter)
            return;

        duration = std::max(0.0f, duration);
        removeFor(owner, property);

        if (duration <= 0.0f || nearlyEqual(currentValue, targetValue))
        {
            setter(targetValue);
            return;
        }

        ActiveAnimation animation;
        animation.id = nextAnimationId_++;
        animation.owner = owner;
        animation.property = property;
        animation.lifetime = std::move(lifetime);
        animation.startValue = currentValue;
        animation.currentValue = currentValue;
        animation.targetValue = targetValue;
        animation.duration = duration;
        animation.easing = easing;
        animation.setter = std::move(setter);
        animations_.push_back(std::move(animation));
    }

    void AnimationSystem::cancel(OwnerKey owner, PropertyKey property) noexcept
    {
        removeFor(owner, property);
    }

    void AnimationSystem::advance(float dt) noexcept
    {
        dt = std::max(0.0f, dt);

        for (std::size_t index = 0; index < animations_.size();)
        {
            // The setter belongs to component code and can start, replace, or
            // cancel animations. Work from a copy so vector reallocation cannot
            // invalidate the animation currently being advanced.
            ActiveAnimation animation = animations_[index];

            if (animation.lifetime.expired())
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

            auto current = std::find_if(
                animations_.begin(),
                animations_.end(),
                [id = animation.id](const ActiveAnimation &candidate)
                {
                    return candidate.id == id;
                });

            // The setter replaced or cancelled this animation. The replacement
            // must not be advanced again during this same tick.
            if (current == animations_.end())
            {
                if (index < animations_.size())
                    ++index;
                continue;
            }

            if (animation.elapsed >= animation.duration ||
                nearlyEqual(animation.currentValue, animation.targetValue))
            {
                current->setter(animation.targetValue);
                current = std::find_if(
                    animations_.begin(),
                    animations_.end(),
                    [id = animation.id](const ActiveAnimation &candidate)
                    {
                        return candidate.id == animation.id;
                    });
                if (current != animations_.end())
                    animations_.erase(current);
                continue;
            }

            current->elapsed = animation.elapsed;
            current->currentValue = animation.currentValue;
            ++index;
        }
    }
}
