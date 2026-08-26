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
    ui::TextInput *attachTextInput(ui::NodeTree &tree, ui::Node::Id id = 0)
    {
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(id, std::move(input));
        return inputPtr;
    }

    void dispatchFocusGained(ui::NodeTree &tree, ui::TextInput *input)
    {
        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, input, gained, false, false);
    }

    void dispatchFocusLost(ui::NodeTree &tree, ui::TextInput *input)
    {
        ui::FocusLostEvent lost;
        ui::EventDispatcher::dispatch(tree, input, lost, false, false);
    }

    void testTextInputPublishesSemanticTextChanges()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        int changes = 0;
        input->on<ui::TextChangedEvent>([&](ui::TextChangedEvent &, ui::Node &node)
        {
            ++changes;
            auto &field = static_cast<ui::TextInput &>(node);
            assert(field.getText() == "hello");
        });

        input->setText("hello");
        assert(input->getText() == "hello");
        assert(changes == 1);

        input->setText("hello");
        assert(changes == 1);
    }

    void testTextInputAllCommittedMutationsNotifyOnce()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        int changes = 0;
        input->on<ui::TextChangedEvent>([&](ui::TextChangedEvent &, ui::Node &node)
        {
            ++changes;
            assert(!static_cast<ui::TextInput &>(node).getText().empty());
        });

        input->setText("abc");
        assert(changes == 1);

        input->setCaretPosition(3);
        input->backspace();
        assert(input->getText() == "ab");
        assert(changes == 2);

        input->deleteForward();
        assert(changes == 2);

        input->setCaretPosition(1);
        input->insertText("X");
        assert(input->getText() == "aXb");
        assert(changes == 3);

        input->insertText("");
        assert(changes == 3);
    }

    void testTextInputHandlesFocusedTextEvent()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        dispatchFocusGained(tree, input);

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

        dispatchFocusGained(tree, input);

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

        dispatchFocusGained(tree, input);

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

    void testTextInputCompositionDoesNotMutateCommittedText()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        dispatchFocusGained(tree, input);

        ui::TextEditingEvent editing;
        editing.composition = "hel";
        editing.cursor = 2;
        editing.selectionLength = 1;
        ui::EventDispatcher::dispatch(tree, input, editing, false, false);

        assert(input->getText().empty());

        editing.composition = "hell";
        editing.cursor = 4;
        editing.selectionLength = 0;
        ui::EventDispatcher::dispatch(tree, input, editing, false, false);

        assert(input->getText().empty());

        ui::TextInputEvent committed;
        committed.text = "hello";
        ui::EventDispatcher::dispatch(tree, input, committed, false, false);

        assert(input->getText() == "hello");
    }

    void testTextInputCompositionIsDroppedOnFocusLoss()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        dispatchFocusGained(tree, input);

        ui::TextEditingEvent editing;
        editing.composition = "temporary";
        ui::EventDispatcher::dispatch(tree, input, editing, false, false);
        assert(input->getText().empty());

        dispatchFocusLost(tree, input);
        dispatchFocusGained(tree, input);

        ui::TextInputEvent committed;
        committed.text = "abc";
        ui::EventDispatcher::dispatch(tree, input, committed, false, false);

        assert(input->getText() == "abc");
    }

    void testTextInputFocusLossStopsFurtherTextInput()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);

        dispatchFocusGained(tree, input);
        dispatchFocusLost(tree, input);

        ui::TextInputEvent textEvent;
        textEvent.text = "ignored";
        ui::EventDispatcher::dispatch(tree, input, textEvent, false, false);

        assert(input->getText().empty());
        assert(!textEvent.propagationStopped);
    }

    void testTextInputFirstMouseDownPlacesCaretBeforeFocus()
    {
        ui::NodeTree tree;
        ui::TextInput *input = attachTextInput(tree);
        input->setText("hello");

        ui::MouseDownEvent mouseDown;
        mouseDown.position = {0.0f, 0.0f};
        mouseDown.button = ui::MouseButton::Left;
        ui::EventDispatcher::dispatch(tree, input, mouseDown, false, false);

        assert(input->getCaretPosition() == 0);
        assert(!input->hasSelection());

        ui::InputSystem inputSystem;
        assert(inputSystem.focus(tree, *input));
        assert(inputSystem.focusedNode() == input);
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
        ui::TextInput *first = attachTextInput(tree, 0);
        ui::TextInput *second = attachTextInput(tree, 1);

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
    testTextInputAllCommittedMutationsNotifyOnce();
    testTextInputHandlesFocusedTextEvent();
    testTextInputIgnoresTextEventWithoutFocus();
    testTextInputKeyboardEditingAndShiftSelection();
    testTextInputCtrlASelectsAll();
    testTextInputCompositionDoesNotMutateCommittedText();
    testTextInputCompositionIsDroppedOnFocusLoss();
    testTextInputFocusLossStopsFurtherTextInput();
    testTextInputFirstMouseDownPlacesCaretBeforeFocus();
    testSDLTextInputRoutesThroughInputSystemFocus();
    testSDLTextInputDoesNotReachUnfocusedTextInput();
    testSDLKeyDownRoutesModifiersToTextInput();
    return 0;
}
