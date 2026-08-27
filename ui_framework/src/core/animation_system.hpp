#pragma once

#include <memory>
#include <vector>

#include "ui_framework/animation.hpp"

namespace ui
{
    class AnimationSystem
    {
    public:
        AnimationSystem() = default;
        AnimationSystem(const AnimationSystem &) = delete;
        AnimationSystem &operator=(const AnimationSystem &) = delete;

        void registerAnimation(Animation &animation);
        void advance(float dt) noexcept;

    private:
        std::vector<std::weak_ptr<Animation::State>> animations_;
    };
}
