#include "ui_framework/ui_manager.hpp"
#include "ui_framework/node.hpp"
#include "ui_framework/panel_node.hpp"
#include "ui_framework/stack_panel_node.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>
#include <memory>

namespace
{
    struct TestFailure
    {
        const char *message;
    };

    void expect(bool condition, const char *message)
    {
        if (!condition)
            throw TestFailure{message};
    }

    SDL_Event mouseWheel(float x, float y, float deltaY)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_WHEEL;
        event.wheel.mouse_x = x;
        event.wheel.mouse_y = y;
        event.wheel.x = 0.0f;
        event.wheel.y = deltaY;
        return event;
    }

    void test_only_panels_can_be_scroll_containers()
    {
        ui::UIManager manager;
        auto leaf = std::make_unique<ui::Node>();
        ui::Node *node = manager.addRoot(std::move(leaf));

        expect(node != nullptr, "root node must attach");
        expect(!manager.enableScrolling(*node), "leaf Node must not become a scroll container");
        expect(!manager.isScrollingEnabled(*node), "failed scroll registration must leave leaf unregistered");
    }

    void test_content_extent_and_max_offset_are_derived_from_layout()
    {
        ui::UIManager manager;
        auto panel = std::make_unique<ui::StackPanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));

        auto first = std::make_unique<ui::Node>();
        first->setSize(ui::LayoutSizeValue::fixed(100.0f, 80.0f));
        auto second = std::make_unique<ui::Node>();
        second->setSize(ui::LayoutSizeValue::fixed(100.0f, 80.0f));

        panel->addChild(std::move(first), 0);
        panel->addChild(std::move(second), 1);

        ui::Node *root = manager.addRoot(std::move(panel));
        expect(root != nullptr, "scroll panel must attach");
        expect(manager.enableScrolling(*root), "PanelNode must register as scroll container");

        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        const ui::ScrollOffset maximum = manager.getMaximumScrollOffset(*root);
        expect(maximum.y == 60.0f, "content extent must produce 60px vertical scroll range");
        expect(maximum.x == 0.0f, "content must not create horizontal scroll range");
    }

    void test_offset_is_clamped_against_current_geometry()
    {
        ui::UIManager manager;
        auto panel = std::make_unique<ui::StackPanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        auto child = std::make_unique<ui::Node>();
        child->setSize(ui::LayoutSizeValue::fixed(100.0f, 200.0f));
        panel->addChild(std::move(child), 0);

        ui::Node *root = manager.addRoot(std::move(panel));
        expect(manager.enableScrolling(*root), "panel must register for scrolling");
        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        expect(manager.setScrollOffset(*root, {0.0f, 500.0f}), "setScrollOffset must succeed for registered panel");
        expect(manager.getScrollOffset(*root).y == 100.0f, "scroll offset must clamp to content range");
    }

    void test_scroll_transform_and_clipping_control_hit_testing()
    {
        ui::UIManager manager;
        auto panel = std::make_unique<ui::PanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        panel->setClipToBounds(true);

        auto child = std::make_unique<ui::Node>();
        child->setSize(ui::LayoutSizeValue::fixed(30.0f, 30.0f));
        child->setPosition({10.0f, 120.0f});
        child->setPositionMode(ui::PositionMode::Absolute);
        int clicks = 0;
        child->on<ui::MouseDownEvent>([&](ui::MouseDownEvent &, ui::Node &)
        {
            ++clicks;
        });
        ui::Node *childPtr = child.get();
        panel->addChild(std::move(child), 0);

        ui::Node *root = manager.addRoot(std::move(panel));
        expect(manager.enableScrolling(*root), "panel must register for scrolling");
        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        manager.processEvent(mouseWheel(50.0f, 50.0f, 0.0f), nullptr);
        manager.processEvent(mouseWheel(50.0f, 50.0f, 0.0f), nullptr);
        expect(clicks == 0, "content outside the clipped viewport must not be hittable before scrolling");

        expect(manager.setScrollOffset(*root, {0.0f, 80.0f}), "scroll offset must be set");
        manager.processEvent(mouseWheel(20.0f, 50.0f, 0.0f), nullptr);
        expect(childPtr != nullptr, "scroll child must remain attached");

        SDL_Event click{};
        click.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        click.button.x = 20.0f;
        click.button.y = 50.0f;
        click.button.button = SDL_BUTTON_LEFT;
        manager.processEvent(click, nullptr);
        expect(clicks == 1, "scroll transform must make the newly visible child hittable");
    }

    void test_hover_is_refreshed_after_wheel_scroll()
    {
        ui::UIManager manager;
        auto panel = std::make_unique<ui::PanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        panel->setClipToBounds(true);

        auto first = std::make_unique<ui::Node>();
        first->setSize(ui::LayoutSizeValue::fixed(50.0f, 50.0f));
        first->setPosition({0.0f, 0.0f});
        first->setPositionMode(ui::PositionMode::Absolute);
        int firstEnter = 0;
        first->on<ui::MouseEnterEvent>([&](ui::MouseEnterEvent &, ui::Node &) { ++firstEnter; });

        auto second = std::make_unique<ui::Node>();
        second->setSize(ui::LayoutSizeValue::fixed(50.0f, 50.0f));
        second->setPosition({0.0f, 100.0f});
        second->setPositionMode(ui::PositionMode::Absolute);
        int secondEnter = 0;
        second->on<ui::MouseEnterEvent>([&](ui::MouseEnterEvent &, ui::Node &) { ++secondEnter; });

        ui::Node *firstPtr = first.get();
        ui::Node *secondPtr = second.get();
        panel->addChild(std::move(first), 0);
        panel->addChild(std::move(second), 1);

        ui::Node *root = manager.addRoot(std::move(panel));
        expect(manager.enableScrolling(*root), "panel must register for scrolling");
        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        SDL_Event move{};
        move.type = SDL_EVENT_MOUSE_MOTION;
        move.motion.x = 25.0f;
        move.motion.y = 25.0f;
        manager.processEvent(move, nullptr);
        expect(firstEnter == 1, "initial pointer position must hover the first child");

        manager.processEvent(mouseWheel(25.0f, 25.0f, 100.0f), nullptr);
        expect(manager.getScrollOffset(*root).y > 0.0f, "wheel must change the scroll offset");
        expect(secondEnter == 1, "wheel scrolling must refresh hover at the unchanged pointer position");
        (void)firstPtr;
        (void)secondPtr;
    }

    void test_layout_resize_reclamps_existing_scroll_offset()
    {
        ui::UIManager manager;
        auto panel = std::make_unique<ui::StackPanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        auto child = std::make_unique<ui::Node>();
        child->setSize(ui::LayoutSizeValue::fixed(100.0f, 300.0f));
        ui::Node *childPtr = child.get();
        panel->addChild(std::move(child), 0);

        ui::Node *root = manager.addRoot(std::move(panel));
        expect(manager.enableScrolling(*root), "panel must register for scrolling");
        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        expect(manager.setScrollOffset(*root, {0.0f, 200.0f}), "initial scroll offset must be accepted");
        expect(manager.getScrollOffset(*root).y == 200.0f, "initial offset must reach the old maximum");

        childPtr->setSize(ui::LayoutSizeValue::fixed(100.0f, 150.0f));
        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        expect(manager.getMaximumScrollOffset(*root).y == 50.0f,
               "layout mutation must reduce the maximum scroll range");
        expect(manager.getScrollOffset(*root).y == 50.0f,
               "sync must reclamp an offset after content shrinks");
    }

    void test_nested_wheel_chains_in_both_directions()
    {
        ui::UIManager manager;
        auto outer = std::make_unique<ui::StackPanelNode>();
        outer->setSize(ui::LayoutSizeValue::fixed(200.0f, 100.0f));

        auto inner = std::make_unique<ui::StackPanelNode>();
        inner->setSize(ui::LayoutSizeValue::fixed(100.0f, 50.0f));

        auto innerChild = std::make_unique<ui::Node>();
        innerChild->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        inner->addChild(std::move(innerChild), 0);

        auto outerChild = std::make_unique<ui::Node>();
        outerChild->setSize(ui::LayoutSizeValue::fixed(200.0f, 100.0f));

        ui::Node *innerRaw = outer->addChild(std::move(inner), 0);
        outer->addChild(std::move(outerChild), 1);
        ui::Node *outerRaw = manager.addRoot(std::move(outer));

        expect(innerRaw != nullptr && outerRaw != nullptr, "nested scroll containers must attach");
        expect(manager.enableScrolling(*outerRaw), "outer panel must register");
        expect(manager.enableScrolling(*innerRaw), "inner panel must register");

        manager.invalidateLayout(*outerRaw);
        manager.runFrame(1.0f / 60.0f, nullptr);

        manager.processEvent(mouseWheel(10.0f, 10.0f, 200.0f), nullptr);

        const float innerMaximum = manager.getMaximumScrollOffset(*innerRaw).y;
        const float outerMaximum = manager.getMaximumScrollOffset(*outerRaw).y;
        expect(manager.getScrollOffset(*innerRaw).y == innerMaximum,
               "downward wheel must first consume the inner scroll range");
        expect(manager.getScrollOffset(*outerRaw).y == outerMaximum,
               "remaining downward wheel delta must reach the outer container");

        manager.processEvent(mouseWheel(10.0f, 10.0f, -200.0f), nullptr);
        expect(manager.getScrollOffset(*innerRaw).y == 0.0f,
               "upward wheel must first consume the inner scroll range in reverse");
        expect(manager.getScrollOffset(*outerRaw).y == 0.0f,
               "remaining upward wheel delta must chain back to the outer container");
    }

    void test_removed_scroll_node_loses_registered_state()
    {
        ui::UIManager manager;
        auto panel = std::make_unique<ui::PanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        ui::Node *root = manager.addRoot(std::move(panel));

        expect(manager.enableScrolling(*root), "panel must register for scrolling");
        expect(manager.isScrollingEnabled(*root), "panel must report registered state");

        manager.removeRoot(root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        expect(!manager.isScrollingEnabled(*root),
               "removed node must not retain stale scroll registration");
        expect(manager.getScrollOffset(*root) == ui::ScrollOffset{},
               "removed node must not retain a stale scroll offset");
        expect(manager.getMaximumScrollOffset(*root) == ui::ScrollOffset{},
               "removed node must not retain a stale maximum offset");
    }
}

int main()
{
    try
    {
        test_only_panels_can_be_scroll_containers();
        test_content_extent_and_max_offset_are_derived_from_layout();
        test_offset_is_clamped_against_current_geometry();
        test_scroll_transform_and_clipping_control_hit_testing();
        test_hover_is_refreshed_after_wheel_scroll();
        test_layout_resize_reclamps_existing_scroll_offset();
        test_nested_wheel_chains_in_both_directions();
        test_removed_scroll_node_loses_registered_state();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "ScrollSystem regression tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "ScrollSystem regression tests passed\n";
    return EXIT_SUCCESS;
}