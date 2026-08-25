#include "input_system.hpp"
#include "layout_system.hpp"
#include "node_tree.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace
{
    struct TestFailure { std::string message; };

    void expect(bool condition, const char *message)
    {
        if (!condition) throw TestFailure{message};
    }

    struct Fixture
    {
        ui::NodeTree tree;
        ui::LayoutSystem layout;
        ui::Node *root = nullptr;
        ui::InputSystem input;

        Fixture()
        {
            auto node = std::make_unique<ui::Node>();
            node->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
            root = tree.attachRoot(0, std::move(node));
            layout.setViewportSize({100.0f, 100.0f});
            layout.requestFullLayout(tree);
            layout.processLayoutQueue(tree);
        }
    };

    SDL_Event mouseDown(float x, float y)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        event.button.x = x;
        event.button.y = y;
        event.button.button = SDL_BUTTON_LEFT;
        return event;
    }

    SDL_Event mouseUp(float x, float y)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        event.button.x = x;
        event.button.y = y;
        event.button.button = SDL_BUTTON_LEFT;
        return event;
    }

    SDL_Event keyDown()
    {
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_RETURN;
        return event;
    }

    SDL_Event mouseMotion(float x, float y)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.x = x;
        event.motion.y = y;
        return event;
    }

    void test_mouse_down_removes_target()
    {
        Fixture f;
        f.root->setFocusable(true);
        f.root->setCapturable(true);
        bool callbackCalled = false;

        f.root->on<ui::MouseDownEvent>([&](ui::MouseDownEvent &, ui::Node &node)
        {
            callbackCalled = true;
            f.tree.removeRoot(&node);
        });

        f.input.processEvent(mouseDown(10.0f, 10.0f), f.tree, nullptr);

        expect(callbackCalled, "MouseDown callback must run before queued removal");
        expect(f.tree.rootsCount() == 0, "queued target removal must be flushed after dispatch");
        expect(f.input.pressedNode() == nullptr, "removed MouseDown target must clear pressed state");
        expect(f.input.capturedNode() == nullptr, "removed MouseDown target must not remain captured");
        expect(f.input.focusedNode() == nullptr, "removed MouseDown target must not remain focused");
        expect(!f.input.isDragging(), "removed MouseDown target must clear drag state");
    }

    void test_mouse_up_removes_captured_target()
    {
        Fixture f;
        f.root->setCapturable(true);
        expect(f.input.capture(f.tree, *f.root, ui::MousePosition{10.0f, 10.0f}),
               "capture setup must succeed");

        bool callbackCalled = false;
        f.root->on<ui::MouseUpEvent>([&](ui::MouseUpEvent &, ui::Node &node)
        {
            callbackCalled = true;
            f.tree.removeRoot(&node);
        });

        f.input.processEvent(mouseUp(10.0f, 10.0f), f.tree, nullptr);

        expect(callbackCalled, "MouseUp callback must run before queued removal");
        expect(f.tree.rootsCount() == 0, "captured target removal must be flushed");
        expect(f.input.capturedNode() == nullptr, "removed captured node must clear capture");
        expect(f.input.pressedNode() == nullptr, "removed captured node must clear pressed state");
        expect(!f.input.isDragging(), "removed captured node must clear drag state");
    }

    void test_key_down_removes_focused_target()
    {
        Fixture f;
        f.root->setFocusable(true);
        expect(f.input.focus(f.tree, *f.root), "focus setup must succeed");

        bool callbackCalled = false;
        f.root->on<ui::KeyDownEvent>([&](ui::KeyDownEvent &, ui::Node &node)
        {
            callbackCalled = true;
            f.tree.removeRoot(&node);
        });

        f.input.processEvent(keyDown(), f.tree, nullptr);

        expect(callbackCalled, "KeyDown callback must run before queued removal");
        expect(f.tree.rootsCount() == 0, "focused target removal must be flushed");
        expect(f.input.focusedNode() == nullptr, "removed focused node must clear focus");
    }

    void test_drag_end_callback_can_replace_capture()
    {
        Fixture f;
        f.root->setCapturable(true);

        auto replacement = std::make_unique<ui::Node>();
        replacement->setPosition({0.0f, 0.0f});
        replacement->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        replacement->setCapturable(true);
        ui::Node *replacementPtr = f.tree.attachRoot(1, std::move(replacement));

        bool dragEndCalled = false;
        f.root->on<ui::MouseDragEndEvent>([&](ui::MouseDragEndEvent &, ui::Node &)
        {
            dragEndCalled = true;
            f.input.capture(f.tree, *replacementPtr, ui::MousePosition{20.0f, 20.0f});
        });

        f.input.processEvent(mouseDown(10.0f, 10.0f), f.tree, nullptr);
        f.input.processEvent(mouseMotion(30.0f, 10.0f), f.tree, nullptr);
        expect(f.input.isDragging(), "setup must enter drag state");

        f.input.processEvent(mouseUp(30.0f, 10.0f), f.tree, nullptr);

        expect(dragEndCalled, "DragEnd callback must run");
        expect(f.input.capturedNode() == replacementPtr,
               "capture created during DragEnd must not be overwritten by release cleanup");
    }
}

int main()
{
    try
    {
        test_mouse_down_removes_target();
        test_mouse_up_removes_captured_target();
        test_key_down_removes_focused_target();
        test_drag_end_callback_can_replace_capture();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "InputSystem mutation test failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "InputSystem mutation tests passed\n";
    return EXIT_SUCCESS;
}
