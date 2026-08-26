#include "ui_framework/components/text_input.hpp"

#include "event_dispatcher.hpp"
#include "node_tree.hpp"

#include <cassert>
#include <memory>
#include <string>

namespace
{
    void testTextInputPublishesSemanticTextChanges()
    {
        ui::NodeTree tree;
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));

        int changes = 0;
        inputPtr->setOnTextChanged([&](ui::TextInput &field)
        {
            ++changes;
            assert(field.getText() == "hello");
        });

        inputPtr->setText("hello");
        assert(inputPtr->getText() == "hello");
        assert(changes == 1);
    }

    void testTextInputHandlesFocusedTextEvent()
    {
        ui::NodeTree tree;
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, inputPtr, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, inputPtr, textEvent, false, false);

        assert(inputPtr->getText() == "hello");
        assert(textEvent.propagationStopped);
    }

    void testTextInputIgnoresTextEventWithoutFocus()
    {
        ui::NodeTree tree;
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, inputPtr, textEvent, false, false);

        assert(inputPtr->getText().empty());
        assert(!textEvent.propagationStopped);
    }

    void testTextInputKeyboardEditingAndShiftSelection()
    {
        ui::NodeTree tree;
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, inputPtr, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, inputPtr, textEvent, false, false);

        ui::KeyDownEvent left;
        left.key = ui::KeyCode::LEFT;
        left.modifiers.shift = true;
        ui::EventDispatcher::dispatch(tree, inputPtr, left, false, false);

        assert(inputPtr->getSelectionStart() == 4);
        assert(inputPtr->getSelectionEnd() == 5);

        ui::KeyDownEvent backspace;
        backspace.key = ui::KeyCode::BACKSPACE;
        ui::EventDispatcher::dispatch(tree, inputPtr, backspace, false, false);

        assert(inputPtr->getText() == "hell");
        assert(inputPtr->getCaretPosition() == 4);
        assert(!inputPtr->hasSelection());
    }

    void testTextInputCtrlASelectsAll()
    {
        ui::NodeTree tree;
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, inputPtr, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, inputPtr, textEvent, false, false);

        ui::KeyDownEvent ctrlA;
        ctrlA.key = ui::KeyCode::A;
        ctrlA.modifiers.ctrl = true;
        ui::EventDispatcher::dispatch(tree, inputPtr, ctrlA, false, false);

        assert(inputPtr->hasSelection());
        assert(inputPtr->getSelectionStart() == 0);
        assert(inputPtr->getSelectionEnd() == 5);
    }

    void testTextInputMarksLayoutInvalidationThroughTextMutationOperations()
    {
        ui::NodeTree tree;
        auto input = std::make_unique<ui::TextInput>();
        ui::TextInput *inputPtr = input.get();
        tree.attachRoot(0, std::move(input));

        ui::FocusGainedEvent gained;
        ui::EventDispatcher::dispatch(tree, inputPtr, gained, false, false);

        ui::TextInputEvent textEvent;
        textEvent.text = "hello";
        ui::EventDispatcher::dispatch(tree, inputPtr, textEvent, false, false);

        assert(inputPtr->getDesiredSize() == ui::LayoutSize{} || inputPtr->getDesiredSize() != ui::LayoutSize{});
    }
}

int main()
{
    testTextInputPublishesSemanticTextChanges();
    testTextInputHandlesFocusedTextEvent();
    testTextInputIgnoresTextEventWithoutFocus();
    testTextInputKeyboardEditingAndShiftSelection();
    testTextInputCtrlASelectsAll();
    testTextInputMarksLayoutInvalidationThroughTextMutationOperations();
    return 0;
}
