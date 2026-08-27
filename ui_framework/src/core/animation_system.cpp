#include "animation_system.hpp"

#include "ui_framework/node.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ui
{
    FloatAnimationProperty FloatAnimationProperty::from(float &value) noexcept
    {
        return FloatAnimationProperty(
            nullptr,
            &value,
            [&value]() noexcept { return value; },
            [&value](float next) noexcept { value = next; },
            {});
    }

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

    void AnimationSystem::removeFor(Node *owner, PropertyKey property) noexcept
    {
        std::erase_if(animations_, [owner, property](const ActiveAnimation &animation)
        {
            return animation.owner == owner && animation.property == property;
        });
    }

    void AnimationSystem::animate(const FloatAnimationProperty &property, float targetValue,
                                  float duration, AnimationEasing easing)
    {
        if (!property || (property.owner_ && property.lifetime_.expired()))
            return;

        duration = std::max(0.0f, duration);
        removeFor(property.owner_, property.key_);

        const float currentValue = property.getter_();
        if (duration <= 0.0f || nearlyEqual(currentValue, targetValue))
        {
            property.setter_(targetValue);
            return;
        }

        ActiveAnimation animation;
        animation.id = nextAnimationId_++;
        animation.owner = property.owner_;
        animation.property = property.key_;
        animation.lifetime = property.lifetime_;
        animation.startValue = currentValue;
        animation.currentValue = currentValue;
        animation.targetValue = targetValue;
        animation.duration = duration;
        animation.easing = easing;
        animation.setter = property.setter_;
        animations_.push_back(std::move(animation));
    }

    void AnimationSystem::cancel(const FloatAnimationProperty &property) noexcept
    {
        if (property)
            removeFor(property.owner_, property.key_);
    }

    void AnimationSystem::advance(float dt) noexcept
    {
        dt = std::max(0.0f, dt);

        std::vector<AnimationId> ids;
        ids.reserve(animations_.size());
        for (const ActiveAnimation &animation : animations_)
            ids.push_back(animation.id);

        for (const AnimationId id : ids)
        {
            auto current = std::find_if(animations_.begin(), animations_.end(),
                [id](const ActiveAnimation &animation) { return animation.id == id; });
            if (current == animations_.end())
                continue;

            if (current->owner && current->lifetime.expired())
            {
                animations_.erase(current);
                continue;
            }

            ActiveAnimation animation = *current;
            animation.elapsed = std::min(animation.duration, animation.elapsed + dt);
            const float t = animation.duration > 0.0f ? animation.elapsed / animation.duration : 1.0f;
            const float eased = applyEasing(t, animation.easing);
            animation.currentValue = animation.startValue +
                                      (animation.targetValue - animation.startValue) * eased;

            animation.setter(animation.currentValue);

            current = std::find_if(animations_.begin(), animations_.end(),
                [id](const ActiveAnimation &candidate) { return candidate.id == id; });
            if (current == animations_.end())
                continue;

            if (animation.elapsed >= animation.duration ||
                nearlyEqual(animation.currentValue, animation.targetValue))
            {
                current->setter(animation.targetValue);
                current = std::find_if(animations_.begin(), animations_.end(),
                    [id](const ActiveAnimation &candidate) { return candidate.id == id; });
                if (current != animations_.end())
                    animations_.erase(current);
                continue;
            }

            current->elapsed = animation.elapsed;
            current->currentValue = animation.currentValue;
        }
    }
}
