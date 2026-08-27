#include "animation_system.hpp"
#include "node_tree.hpp"

#include <cassert>
#include <cmath>
#include <memory>

namespace
{
    void expectNear(float actual, float expected)
    {
        assert(std::fabs(actual - expected) < 0.0001f);
    }

    class AnimatedNode final : public ui::Node
    {
    public:
        float value() const noexcept { return value_; }

        void animateTo(float target, float duration, ui::AnimationEasing easing = ui::AnimationEasing::Linear)
        {
            animateFloat(
                &value_,
                value_,
                target,
                duration,
                easing,
                [this](float value)
                {
                    value_ = value;
                });
        }

        void cancelValueAnimation() noexcept
        {
            cancelAnimation(&value_);
        }

    private:
        float value_ = 0.0f;
    };
}

int main()
{
    ui::NodeTree tree;
    auto node = std::make_unique<AnimatedNode>();
    AnimatedNode *animated = node.get();
    assert(tree.attachRoot(0, std::move(node)) == animated);

    animated->animateTo(10.0f, 1.0f);
    tree.advanceTime(0.25f);
    expectNear(animated->value(), 2.5f);
    tree.advanceTime(0.75f);
    expectNear(animated->value(), 10.0f);

    animated->animateTo(0.0f, 1.0f, ui::AnimationEasing::EaseOut);
    tree.advanceTime(0.5f);
    expectNear(animated->value(), 2.5f);

    // Starting another transition for the same property replaces the old
    // transition and starts from the value currently visible to the node.
    animated->animateTo(20.0f, 1.0f);
    tree.advanceTime(0.25f);
    expectNear(animated->value(), 6.875f);

    animated->animateTo(30.0f, 0.0f);
    expectNear(animated->value(), 30.0f);

    animated->animateTo(40.0f, 1.0f);
    animated->cancelValueAnimation();
    tree.advanceTime(1.0f);
    expectNear(animated->value(), 30.0f);

    // A destroyed node leaves no live target for its deferred animation.
    auto temporary = std::make_unique<AnimatedNode>();
    AnimatedNode *temporaryRaw = temporary.get();
    assert(tree.attachRoot(1, std::move(temporary)) == temporaryRaw);
    temporaryRaw->animateTo(100.0f, 1.0f);
    tree.removeRoot(temporaryRaw);
    tree.advanceTime(1.0f);

    return 0;
}
