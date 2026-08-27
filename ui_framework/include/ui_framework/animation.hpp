#pragma once

#include <algorithm>
#include <cmath>

namespace ui
{
    enum class AnimationEasing
    {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut
    };

    class Animation
    {
    public:
        explicit Animation(float initialValue = 0.0f) noexcept
            : currentValue_(initialValue),
              startValue_(initialValue),
              targetValue_(initialValue)
        {
        }

        void setValue(float value) noexcept
        {
            currentValue_ = value;
            startValue_ = value;
            targetValue_ = value;
            elapsed_ = 0.0f;
            duration_ = 0.0f;
            active_ = false;
        }

        void setTarget(
            float target,
            float duration,
            AnimationEasing easing = AnimationEasing::Linear) noexcept
        {
            duration = std::max(0.0f, duration);

            if (nearlyEqual(targetValue_, target) &&
                nearlyEqual(duration_, duration) &&
                easing_ == easing)
            {
                return;
            }

            targetValue_ = target;
            duration_ = duration;
            easing_ = easing;
            startValue_ = currentValue_;
            elapsed_ = 0.0f;

            if (duration_ <= 0.0f || nearlyEqual(currentValue_, targetValue_))
            {
                currentValue_ = targetValue_;
                active_ = false;
                return;
            }

            active_ = true;
        }

        void advance(float dt) noexcept
        {
            if (!active_)
                return;

            dt = std::max(0.0f, dt);
            elapsed_ = std::min(duration_, elapsed_ + dt);

            const float normalized = duration_ > 0.0f
                ? elapsed_ / duration_
                : 1.0f;
            const float eased = applyEasing(normalized, easing_);
            currentValue_ = startValue_ +
                            (targetValue_ - startValue_) * eased;

            if (elapsed_ >= duration_ || nearlyEqual(currentValue_, targetValue_))
            {
                currentValue_ = targetValue_;
                active_ = false;
            }
        }

        float value() const noexcept { return currentValue_; }
        float target() const noexcept { return targetValue_; }
        bool isActive() const noexcept { return active_; }
        bool isAtTarget() const noexcept { return nearlyEqual(currentValue_, targetValue_); }

    private:
        static bool nearlyEqual(float a, float b) noexcept
        {
            return std::fabs(a - b) <= 0.000001f;
        }

        static float applyEasing(float t, AnimationEasing easing) noexcept
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

        float currentValue_ = 0.0f;
        float startValue_ = 0.0f;
        float targetValue_ = 0.0f;
        float duration_ = 0.0f;
        float elapsed_ = 0.0f;
        AnimationEasing easing_ = AnimationEasing::Linear;
        bool active_ = false;
    };
}
