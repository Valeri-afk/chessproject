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

    void test_nested_wheel_chains_to_outer_scroll_container()
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

        expect(manager.getScrollOffset(*innerRaw).y > 0.0f, "wheel must scroll the nearest inner container first");
        const float innerMaximum = manager.getMaximumScrollOffset(*innerRaw).y;
        expect(manager.getScrollOffset(*innerRaw).y == innerMaximum, "inner scroll must consume up to its limit");
        expect(manager.getScrollOffset(*outerRaw).y > 0.0f, "remaining wheel delta must chain to the outer container");
    }
}

int main()
{
    try
    {
        test_only_panels_can_be_scroll_containers();
        test_content_extent_and_max_offset_are_derived_from_layout();
        test_offset_is_clamped_against_current_geometry();
        test_nested_wheel_chains_to_outer_scroll_container();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "ScrollSystem regression tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "ScrollSystem regression tests passed\n";
    return EXIT_SUCCESS;
}
