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

    SDL_Event mouseMotion(float x, float y)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.x = x;
        event.motion.y = y;
        return event;
    }

    SDL_Event mouseDown(float x, float y, Uint8 button = SDL_BUTTON_LEFT)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        event.button.x = x;
        event.button.y = y;
        event.button.button = button;
        return event;
    }

    SDL_Event mouseUp(float x, float y, Uint8 button = SDL_BUTTON_LEFT)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
        event.button.x = x;
        event.button.y = y;
        event.button.button = button;
        return event;
    }

    SDL_Event keyDown(SDL_Keycode key, bool repeat = false)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = key;
        event.key.repeat = repeat;
        return event;
    }

    SDL_Event keyUp(SDL_Keycode key, bool repeat = false)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_UP;
        event.key.key = key;
        event.key.repeat = repeat;
        return event;
    }

    void test_hover_enter_leave()
    {
        Fixture f;
        int enters = 0;
        int leaves = 0;
        f.root->on<ui::MouseEnterEvent>([&](ui::MouseEnterEvent &, ui::Node &) { ++enters; });
        f.root->on<ui::MouseLeaveEvent>([&](ui::MouseLeaveEvent &, ui::Node &) { ++leaves; });

        f.input.processEvent(mouseMotion(10.0f, 10.0f), f.tree, nullptr);
        expect(enters == 1, "hover enter must be dispatched once");
        expect(leaves == 0, "leave must not fire while pointer remains inside");

        f.input.processEvent(mouseMotion(150.0f, 150.0f), f.tree, nullptr);
        expect(leaves == 1, "hover leave must be dispatched once");
    }

    void test_click_sequence_and_automatic_focus_capture()
    {
        Fixture f;
        int downs = 0;
        int ups = 0;
        int clicks = 0;
        f.root->setFocusable(true);
        f.root->setCapturable(true);
        f.root->on<ui::MouseDownEvent>([&](ui::MouseDownEvent &, ui::Node &) { ++downs; });
        f.root->on<ui::MouseUpEvent>([&](ui::MouseUpEvent &, ui::Node &) { ++ups; });
        f.root->on<ui::MouseClickEvent>([&](ui::MouseClickEvent &, ui::Node &) { ++clicks; });

        f.input.processEvent(mouseDown(10.0f, 10.0f), f.tree, nullptr);
        expect(downs == 1, "MouseDown must reach hit target");
        expect(f.input.focusedNode() == f.root, "focusable target must receive focus");
        expect(f.input.capturedNode() == f.root, "capturable target must receive capture");
        expect(f.input.pressedNode() == f.root, "MouseDown must establish pressed target");

        f.input.processEvent(mouseUp(10.0f, 10.0f), f.tree, nullptr);
        expect(ups == 1, "MouseUp must reach captured target");
        expect(clicks == 1, "matching MouseDown/MouseUp must produce Click");
        expect(f.input.capturedNode() == nullptr, "capture must be released after MouseUp");
        expect(f.input.pressedNode() == nullptr, "pressed state must be cleared after MouseUp");
    }

    void test_keyboard_routes_to_focused_node()
    {
        Fixture f;
        int downs = 0;
        int ups = 0;
        ui::KeyCode receivedDown = ui::KeyCode::UNKNOWN;
        ui::KeyCode receivedUp = ui::KeyCode::UNKNOWN;
        f.root->setFocusable(true);
        f.root->on<ui::KeyDownEvent>([&](ui::KeyDownEvent &event, ui::Node &) { ++downs; receivedDown = event.key; });
        f.root->on<ui::KeyUpEvent>([&](ui::KeyUpEvent &event, ui::Node &) { ++ups; receivedUp = event.key; });

        expect(f.input.focus(f.tree, *f.root), "focus must succeed for focusable node");
        f.input.processEvent(keyDown(SDLK_RETURN), f.tree, nullptr);
        f.input.processEvent(keyUp(SDLK_RETURN), f.tree, nullptr);

        expect(downs == 1, "KeyDown must reach focused node");
        expect(ups == 1, "KeyUp must reach focused node");
        expect(receivedDown == ui::KeyCode::ENTER, "SDL return must map to ENTER");
        expect(receivedUp == ui::KeyCode::ENTER, "SDL return must map to ENTER");
    }

    void test_capture_survives_pointer_leaving_target()
    {
        Fixture f;
        int moves = 0;
        f.root->setCapturable(true);
        f.root->on<ui::MouseMoveEvent>([&](ui::MouseMoveEvent &, ui::Node &) { ++moves; });

        expect(f.input.capture(f.tree, *f.root, ui::MousePosition{10.0f, 10.0f}), "capture must succeed");
        f.input.processEvent(mouseMotion(150.0f, 150.0f), f.tree, nullptr);
        expect(moves == 1, "captured node must receive movement outside its bounds");
        expect(f.input.capturedNode() == f.root, "capture must remain active");
    }

    void test_removed_captured_node_is_reconciled()
    {
        Fixture f;
        f.root->setCapturable(true);
        expect(f.input.capture(f.tree, *f.root, ui::MousePosition{10.0f, 10.0f}), "capture must succeed");
        f.tree.removeRoot(f.root);
        f.input.syncState(f.tree);
        expect(f.input.capturedNode() == nullptr, "removed captured node must not remain tracked");
        expect(f.input.pressedNode() == nullptr, "removed captured node must clear pressed state");
        expect(!f.input.isDragging(), "removed captured node must clear drag state");
    }

    void test_focus_callback_can_request_another_focus()
    {
        Fixture f;
        auto second = std::make_unique<ui::Node>();
        second->setSize(ui::LayoutSizeValue::fixed(20.0f, 20.0f));
        ui::Node *secondPtr = f.tree.attachRoot(1, std::move(second));
        secondPtr->setFocusable(true);
        f.layout.requestFullLayout(f.tree);
        f.layout.processLayoutQueue(f.tree);
        f.root->setFocusable(true);
        f.root->on<ui::FocusLostEvent>([&](ui::FocusLostEvent &, ui::Node &) { f.input.focus(f.tree, *secondPtr); });

        expect(f.input.focus(f.tree, *f.root), "initial focus must succeed");
        f.input.clearFocus(f.tree);
        expect(f.input.focusedNode() == secondPtr, "focus requested during FocusLost must be applied");
    }
}

int main()
{
    try
    {
        test_hover_enter_leave();
        test_click_sequence_and_automatic_focus_capture();
        test_keyboard_routes_to_focused_node();
        test_capture_survives_pointer_leaving_target();
        test_removed_captured_node_is_reconciled();
        test_focus_callback_can_request_another_focus();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "InputSystem test failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "InputSystem tests passed\n";
    return EXIT_SUCCESS;
}
