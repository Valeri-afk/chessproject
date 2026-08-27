#include "animation_system.hpp"
#include "node_tree.hpp"

#include <cassert>
#include <cmath>

namespace
{
    void expectNear(float actual, float expected)
    {
        assert(std::fabs(actual - expected) < 0.0001f);
    }
}

int main()
{
    ui::Animation animation(0.0f);
    assert(!animation.isActive());
    expectNear(animation.value(), 0.0f);

    animation.setTarget(1.0f, 1.0f, ui::AnimationEasing::Linear);
    assert(animation.isActive());
    animation.advance(0.25f);
    expectNear(animation.value(), 0.25f);
    animation.advance(0.75f);
    expectNear(animation.value(), 1.0f);
    assert(!animation.isActive());
    assert(animation.isAtTarget());

    animation.setTarget(0.0f, 1.0f, ui::AnimationEasing::EaseOut);
    animation.advance(0.5f);
    expectNear(animation.value(), 0.25f);

    animation.setValue(0.0f);
    animation.setTarget(1.0f, 1.0f);
    animation.advance(-0.5f);
    expectNear(animation.value(), 0.0f);
    animation.advance(0.5f);
    expectNear(animation.value(), 0.5f);

    animation.setValue(3.0f);
    animation.setTarget(7.0f, 0.0f);
    expectNear(animation.value(), 7.0f);
    assert(!animation.isActive());
    assert(animation.isAtTarget());

    ui::NodeTree tree;
    ui::Animation registered(0.0f);
    registered.setTarget(10.0f, 1.0f);
    tree.registerAnimation(registered);
    tree.registerAnimation(registered);
    tree.advanceTime(0.5f);
    expectNear(registered.value(), 5.0f);
    tree.advanceTime(0.5f);
    expectNear(registered.value(), 10.0f);
    assert(!registered.isActive());

    return 0;
}
