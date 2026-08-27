#include "node_tree.hpp"
#include "ui_framework/node.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace
{
    struct TestFailure
    {
        std::string message;
    };

    void expect(bool condition, const char *message)
    {
        if (!condition)
            throw TestFailure{message};
    }

    void expectNear(float actual, float expected, float epsilon, const char *message)
    {
        if (std::fabs(actual - expected) > epsilon)
            throw TestFailure{message};
    }

    class AnimationNode final : public ui::Node
    {
    public:
        void animate(float &property, float target, float duration,
                     ui::AnimationEasing easing = ui::AnimationEasing::Linear)
        {
            animateFloat(&property, property, target, duration, easing,
                         [&property](float value) { property = value; });
        }

        void cancel(float &property) noexcept
        {
            cancelAnimation(&property);
        }
    };

    void test_linear_animation_advances_through_node_tree()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<AnimationNode>();
        AnimationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        float value = 0.0f;
        nodePtr->animate(value, 10.0f, 1.0f);

        tree.advanceTime(0.25f);
        expectNear(value, 2.5f, 0.0001f, "linear animation must interpolate after partial time advance");

        tree.advanceTime(0.75f);
        expectNear(value, 10.0f, 0.0001f, "animation must reach target at duration");
    }

    void test_easing_is_applied()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<AnimationNode>();
        AnimationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        float easeIn = 0.0f;
        float easeOut = 0.0f;
        float easeInOut = 0.0f;

        nodePtr->animate(easeIn, 1.0f, 1.0f, ui::AnimationEasing::EaseIn);
        nodePtr->animate(easeOut, 1.0f, 1.0f, ui::AnimationEasing::EaseOut);
        nodePtr->animate(easeInOut, 1.0f, 1.0f, ui::AnimationEasing::EaseInOut);

        tree.advanceTime(0.5f);

        expectNear(easeIn, 0.25f, 0.0001f, "EaseIn must square normalized time");
        expectNear(easeOut, 0.75f, 0.0001f, "EaseOut must invert EaseIn");
        expectNear(easeInOut, 0.5f, 0.0001f, "EaseInOut must preserve midpoint");
    }

    void test_same_property_replaces_previous_animation()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<AnimationNode>();
        AnimationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        float value = 0.0f;
        nodePtr->animate(value, 10.0f, 1.0f);
        tree.advanceTime(0.4f);
        expectNear(value, 4.0f, 0.0001f, "first animation must advance before replacement");

        nodePtr->animate(value, 8.0f, 0.6f);
        tree.advanceTime(0.3f);
        expectNear(value, 6.0f, 0.0001f, "replacement animation must start from current value");

        tree.advanceTime(0.3f);
        expectNear(value, 8.0f, 0.0001f, "replacement animation must reach its new target");
    }

    void test_different_properties_can_animate_concurrently()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<AnimationNode>();
        AnimationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        float x = 0.0f;
        float y = 100.0f;
        nodePtr->animate(x, 100.0f, 1.0f);
        nodePtr->animate(y, 0.0f, 2.0f);

        tree.advanceTime(0.5f);

        expectNear(x, 50.0f, 0.0001f, "first property must advance independently");
        expectNear(y, 75.0f, 0.0001f, "second property must advance independently");
    }

    void test_cancel_keeps_current_value()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<AnimationNode>();
        AnimationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        float value = 0.0f;
        nodePtr->animate(value, 10.0f, 1.0f);
        tree.advanceTime(0.4f);
        nodePtr->cancel(value);
        const float cancelledValue = value;

        tree.advanceTime(0.6f);
        expectNear(value, cancelledValue, 0.0001f, "cancel must leave the current property value unchanged");
    }

    void test_zero_duration_applies_target_immediately()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<AnimationNode>();
        AnimationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        float value = 2.0f;
        nodePtr->animate(value, 7.0f, 0.0f);

        expectNear(value, 7.0f, 0.0001f, "zero-duration animation must apply target immediately");
        tree.advanceTime(1.0f);
        expectNear(value, 7.0f, 0.0001f, "completed zero-duration animation must remain finished");
    }

    void test_animation_is_dropped_when_node_is_destroyed()
    {
        ui::NodeTree tree;
        float value = 0.0f;
        {
            auto node = std::make_unique<AnimationNode>();
            AnimationNode *nodePtr = node.get();
            tree.attachRoot(0, std::move(node));
            nodePtr->animate(value, 10.0f, 1.0f);
            tree.removeRoot(nodePtr);
        }

        tree.advanceTime(1.0f);
        expectNear(value, 0.0f, 0.0001f,
                   "destroyed node animation must not invoke its setter");
    }
}

int main()
{
    try
    {
        test_linear_animation_advances_through_node_tree();
        test_easing_is_applied();
        test_same_property_replaces_previous_animation();
        test_different_properties_can_animate_concurrently();
        test_cancel_keeps_current_value();
        test_zero_duration_applies_target_immediately();
        test_animation_is_dropped_when_node_is_destroyed();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "Animation tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Animation tests passed\n";
    return EXIT_SUCCESS;
}
