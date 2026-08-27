#pragma once

#include <algorithm>
#include <cmath>
#include <memory>

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
        explicit Animation(float initialValue = 0.0f) noexcept;
        ~Animation() = default;

        Animation(const Animation &) = delete;
        Animation &operator=(const Animation &) = delete;
        Animation(Animation &&) noexcept = default;
        Animation &operator=(Animation &&) noexcept = default;

        void setValue(float value) noexcept;
        void setTarget(float target, float duration, AnimationEasing easing = AnimationEasing::Linear) noexcept;
        void advance(float dt) noexcept;

        float value() const noexcept;
        float target() const noexcept;
        bool isActive() const noexcept;
        bool isAtTarget() const noexcept;

    private:
        struct State
        {
            float currentValue = 0.0f;
            float startValue = 0.0f;
            float targetValue = 0.0f;
            float duration = 0.0f;
            float elapsed = 0.0f;
            AnimationEasing easing = AnimationEasing::Linear;
            bool active = false;
        };

        std::shared_ptr<State> state_;

        static bool nearlyEqual(float a, float b) noexcept;
        static float applyEasing(float t, AnimationEasing easing) noexcept;
        static void advanceState(State &state, float dt) noexcept;

        friend class AnimationSystem;
    };
}
