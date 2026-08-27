#include "animation_system.hpp"

#include <algorithm>

namespace ui
{
    Animation::Animation(float initialValue) noexcept
        : state_(std::make_shared<State>())
    {
        state_->currentValue = initialValue;
        state_->startValue = initialValue;
        state_->targetValue = initialValue;
    }

    void Animation::setValue(float value) noexcept
    {
        if (!state_)
            return;
        state_->currentValue = value;
        state_->startValue = value;
        state_->targetValue = value;
        state_->elapsed = 0.0f;
        state_->duration = 0.0f;
        state_->active = false;
    }

    void Animation::setTarget(float target, float duration, AnimationEasing easing) noexcept
    {
        if (!state_)
            return;

        duration = std::max(0.0f, duration);
        if (nearlyEqual(state_->targetValue, target) &&
            nearlyEqual(state_->duration, duration) &&
            state_->easing == easing)
            return;

        state_->targetValue = target;
        state_->duration = duration;
        state_->easing = easing;
        state_->startValue = state_->currentValue;
        state_->elapsed = 0.0f;

        if (duration <= 0.0f || nearlyEqual(state_->currentValue, target))
        {
            state_->currentValue = target;
            state_->active = false;
            return;
        }

        state_->active = true;
    }

    void Animation::advance(float dt) noexcept
    {
        if (!state_ || !state_->active)
            return;

        dt = std::max(0.0f, dt);
        state_->elapsed = std::min(state_->duration, state_->elapsed + dt);
        const float t = state_->duration > 0.0f
            ? state_->elapsed / state_->duration
            : 1.0f;
        const float eased = applyEasing(t, state_->easing);
        state_->currentValue = state_->startValue +
                               (state_->targetValue - state_->startValue) * eased;

        if (state_->elapsed >= state_->duration ||
            nearlyEqual(state_->currentValue, state_->targetValue))
        {
            state_->currentValue = state_->targetValue;
            state_->active = false;
        }
    }

    float Animation::value() const noexcept { return state_ ? state_->currentValue : 0.0f; }
    float Animation::target() const noexcept { return state_ ? state_->targetValue : 0.0f; }
    bool Animation::isActive() const noexcept { return state_ && state_->active; }
    bool Animation::isAtTarget() const noexcept { return !state_ || nearlyEqual(state_->currentValue, state_->targetValue); }

    bool Animation::nearlyEqual(float a, float b) noexcept
    {
        return std::fabs(a - b) <= 0.000001f;
    }

    float Animation::applyEasing(float t, AnimationEasing easing) noexcept
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

    void AnimationSystem::registerAnimation(Animation &animation)
    {
        if (!animation.state_ || !animation.isActive())
            return;

        for (const auto &weak : animations_)
        {
            if (auto state = weak.lock(); state && state == animation.state_)
                return;
        }

        animations_.push_back(animation.state_);
    }

    void AnimationSystem::advance(float dt) noexcept
    {
        for (auto it = animations_.begin(); it != animations_.end();)
        {
            if (auto state = it->lock())
            {
                if (state->active)
                {
                    Animation animation;
                    animation.state_ = std::move(state);
                    animation.advance(dt);
                }
                ++it;
            }
            else
            {
                it = animations_.erase(it);
            }
        }

        animations_.erase(
            std::remove_if(animations_.begin(), animations_.end(),
                [](const auto &weak)
                {
                    auto state = weak.lock();
                    return !state || !state->active;
                }),
            animations_.end());
    }
}
