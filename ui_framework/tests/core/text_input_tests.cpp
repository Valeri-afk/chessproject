#include "ui_framework/components/text_input.hpp"

#include "event_dispatcher.hpp"
#include "input_system.hpp"
#include "node_tree.hpp"

#include <SDL3/SDL.h>

#include <cassert>
#include <memory>
#include <string>

namespace
{
    ui::TextInput *attachTextInput(ui::NodeTree &tree)
    {
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));
        return inputPtr;
    }

    void testTextInputPublishesSemanticTextChanges()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        int changes = 0;
        input->setOnTextChanged([&](ui::TextInput &field)
        {
            ++changes;
            assert(field.getText() == "hello");
        });

        input->setText("hello");
        assert(input->getText() == "hello");
        assert(changes == 1);
    }

    void testTextInputHandlesFocusedTextEvent()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, input, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, input, textEvent, false, false);

        assert(input->getText() == "hello");
        assert(textEvent.propagationStopped);
    }

    void testTextInputIgnoresTextEventWithoutFocus()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, input, textEvent, false, false);

        assert(input->getText().empty());
        assert(!textEvent.propagationStopped);
    }

    void testTextInputKeyboardEditingAndShiftSelection()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, input, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, input, textEvent, false, false);

        ui::KeyDownEvent left;
        left.key = ui::KeyCode::LEFT;
        left.modifiers.shift = true;
        ui::EventDispatcher::dispatch(tree, input, left, false, false);

        assert(input->getSelectionStart() == 4);
        assert(input->getSelectionEnd() == 5);

        ui::KeyDownEvent backspace;
        backspace.key = ui::KeyCode::BACKSPACE;
        ui::EventDispatcher::dispatch(tree, input, backspace, false, false);

        assert(input->getText() == "hell");
        assert(input->getCaretPosition() == 4);
        assert(!input->hasSelection());
    }

    void testTextInputCtrlASelectsAll()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, input, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, input, textEvent, false, false);

        ui::KeyDownEvent ctrlA;
        ctrlA.key = ui::KeyCode::A;
        ctrlA.modifiers.ctrl = true;
        ui::EventDispatcher::dispatch(tree, input, ctrlA, false, false);

        assert(input->hasSelection());
        assert(input->getSelectionStart() == 0);
        assert(input->getSelectionEnd() == 5);
    }

    void testSDLTextInputRoutesThroughInputSystemFocus()
    {
        ui::NodeTree tree;
        ui::InputSystem inputSystem;
        ui::TextInput *input = attachTextInput(tree);

        assert(inputSystem.focus(tree, *input));

        SDL_Event event{};
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = const_cast<char *>("hello");

        inputSystem.processEvent(event, tree, nullptr);

        assert(input->getText() == "hello");
    }

    void testSDLTextInputDoesNotReachUnfocusedTextInput()
    {
        ui::NodeTree tree;
        ui::InputSystem inputSystem;
        ui::TextInput *first = attachTextInput(tree);

        auto secondNode = std::make_unique<ui::TextInput>();
        ui::TextInput *second = secondNode.get();
        tree.attachRoot(1, std::move(secondNode));

        assert(inputSystem.focus(tree, *first));

        SDL_Event event{};
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = const_cast<char *>("hello");

        inputSystem.processEvent(event, tree, nullptr);

        assert(first->getText() == "hello");
        assert(second->getText().empty());
    }

    void testSDLKeyDownRoutesModifiersToTextInput()
    {
        ui::NodeTree tree;
        ui::InputSystem inputSystem;
        ui::TextInput *input = attachTextInput(tree);

        bool receivedShift = false;
        input->on<ui::KeyDownEvent>([&](ui::KeyDownEvent &event, ui::Node &)
        {
            receivedShift = event.modifiers.shift;
        });

        assert(inputSystem.focus(tree, *input));

        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = SDLK_LEFT;

        inputSystem.processEvent(event, tree, nullptr);

        assert(!receivedShift);
    }
}

int main()
{
    testTextInputPublishesSemanticTextChanges();
    testTextInputHandlesFocusedTextEvent();
    testTextInputIgnoresTextEventWithoutFocus();
    testTextInputKeyboardEditingAndShiftSelection();
    testTextInputCtrlASelectsAll();
    testSDLTextInputRoutesThroughInputSystemFocus();
    testSDLTextInputDoesNotReachUnfocusedTextInput();
    testSDLKeyDownRoutesModifiersToTextInput();
    return 0;
}
