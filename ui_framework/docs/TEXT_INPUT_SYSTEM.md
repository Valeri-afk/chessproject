# Text Input System — Future Contract

## Status

Text input/editing is not implemented yet. This document defines the architectural contract that should guide the implementation.

The subsystem is intentionally deferred until the rest of the UI framework is stable. The implementation must fit the existing `InputSystem`, focus model, `TextLayout`, `TextRenderer`, and layout invalidation contract rather than introducing parallel infrastructure.

## Architectural boundary

The intended responsibility split is:

```text
Node
  │
  └── focusability / normal Node lifecycle

InputSystem
  │
  ├── SDL keyboard input
  ├── SDL text input / IME events
  ├── focus routing
  └── dispatch to focused Node

TextInput component
  │
  ├── committed text
  ├── caret
  ├── selection
  ├── editing commands
  ├── composition / IME state
  └── text-input presentation policy

TextLayout
  │
  ├── measurement
  ├── wrapping
  ├── line metrics
  └── future text geometry queries such as caret hit-testing

TextRenderer
  │
  └── text rasterization / SDL_ttf rendering

UIManager
  │
  └── orchestration for window-level services such as SDL text-input activation
```

The central rule is that text editing remains a specialized component concern. `Node` must not gain generic editing state such as a caret, selection or composition buffer merely to support text input.

## Input flow

Keyboard commands and actual text insertion are separate concepts:

```text
SDL keyboard events
        ↓
KeyDown / KeyUp
        ↓
editing commands

SDL text input
        ↓
TextInputEvent
        ↓
insert committed Unicode text

SDL text editing / IME
        ↓
TextEditingEvent
        ↓
update temporary composition state
```

The framework should not reconstruct Unicode text from key codes. `InputSystem` should translate SDL text-input events into framework events and route them through the existing focused-node event dispatch path.

The framework event layer should expose semantic events rather than SDL-specific event types.

## TextInput event semantics

The expected framework-level events are conceptually:

```cpp
struct TextInputEvent : UIEvent
{
    std::string text;
};

struct TextEditingEvent : UIEvent
{
    std::string composition;
    int cursor = 0;
    int selectionLength = 0;
};
```

The exact field types remain open until implementation, but the distinction must remain:

```text
TextInputEvent   = committed user text
TextEditingEvent = temporary IME composition
```

Neither event should be coupled to a particular text-input component type.

## Focus integration

There is one focus model. `InputSystem` already owns actual focus transitions and keyboard routing.

The active editable control is simply the focused node capable of consuming text-input events:

```text
focus → TextInput
focus lost → stop editing input
```

The framework must not introduce a second keyboard bus or a separate text-focus subsystem.

SDL text input activation is window-level state, so the framework should keep one current text-input owner and enable/disable SDL text input as focus moves between editable and non-editable controls. Client code should not have to call `SDL_StartTextInput()` merely to make a text field work.

## Editing state

The first implementation should keep three logically separate categories of state:

```text
Committed text
    actual stored string

Selection / caret
    caret position
    selection anchor/range

IME composition
    temporary composition text
    composition cursor/selection metadata
```

IME composition must not be committed into the text buffer until the input system reports committed text.

## Editing commands

The initial editing contract should cover the ordinary single-field editing operations without attempting to become a full editor framework:

```text
Left / Right
Home / End
Backspace / Delete
select all
insert committed text
replace selection
copy / cut / paste
```

Additional navigation such as word-wise movement, richer mouse selection, undo/redo, and advanced editor behavior should be added only when concrete UI requirements justify them.

The primary public API should expose semantic operations rather than raw selection internals. Internal implementations may still store caret/anchor indices or equivalent structures.

## TextLayout relationship

`TextLayout` remains a measurement/layout abstraction. It does not own editing state.

The future text-input component will consume layout information to answer questions such as:

```text
where is the caret for this text position?
which text position is under this pointer?
what geometry represents the current selection?
```

These APIs should be added to `TextLayout` only when the TextInput implementation actually requires them. Do not turn `TextLayoutResult` into a universal editor-state object preemptively.

A likely future direction is a small set of focused geometry queries rather than exposing glyph runs or renderer internals:

```text
TextPosition → caret geometry
layout position → text position
text range → selection geometry
```

## Rendering relationship

Editing semantics remain outside `TextRenderer`.

Conceptually:

```text
TextInput::draw()
    ├── selection presentation
    ├── TextRenderer → committed/composition text
    └── caret presentation
```

`TextRenderer` continues to render text and own SDL_ttf/backend state. It must not become the owner of the editing buffer, caret, selection, or IME state.

Composition rendering belongs to the TextInput presentation layer, even when the actual text glyphs are drawn through `TextRenderer`.

## Layout invalidation

Text edits can change desired size, wrapping, and line count. The component must explicitly invalidate layout when the logical text or any layout-affecting text property changes.

```text
edit text
   ↓
text state changes
   ↓
layout may become stale
   ↓
invalidateLayout()
```

Caret blink, selection highlighting, and other render-only state changes should not require Measure/Arrange unless they alter the actual text geometry.

This follows the framework-wide rule that ordinary setters do not automatically perform client-visible layout invalidation; the operation that knows a layout-affecting change occurred is responsible for requesting it.

## Mouse interaction

Mouse selection should use the normal `InputSystem` pointer routing and focus model:

```text
pointer hit-test
     ↓
TextInput
     ↓
focus
     ↓
text-position hit test
     ↓
caret / selection update
```

Pointer capture should use the existing framework capture semantics for drag selection. TextInput must not introduce a second pointer-capture implementation.

## Clipboard

Clipboard support is part of editing behavior, not core input routing.

The first implementation should use the framework/client window integration already available for SDL clipboard operations. A dedicated general-purpose `ClipboardSystem` should not be introduced unless multiple framework features establish a real need for one.

## IME and text-input window geometry

IME integration may need the current caret position in SDL/window coordinates so the platform can place composition UI or an on-screen keyboard near the text cursor.

The conversion should be:

```text
TextInput caret geometry
        ↓
framework logical UI coordinates
        ↓
logical presentation / window coordinates
        ↓
SDL text-input area
```

TextInput must not store physical window pixels as its primary geometry model.

## Lifecycle of SDL text input

SDL text input is window-level state. The framework should manage it from focus ownership:

```text
focused editable control
        ↓
start SDL text input

focus moves away
        ↓
stop SDL text input
```

Only one control is the active text-input owner at a time.

The implementation must account for platform behavior where enabling text input affects the event stream or invokes an IME/virtual keyboard.

## Scope of the first implementation

The first production version should target:

```text
single-line and/or explicit multiline policy
committed text
caret
selection
keyboard navigation
backspace/delete
mouse caret positioning
clipboard
SDL text input
basic IME composition
TextLayout integration
caret/selection rendering
```

Do not add rich text editing, automatic undo/redo, advanced typography, or a generalized editor framework until concrete requirements demand them.

## Non-goals

```text
editable fields on base Node
second keyboard event bus
second focus system
public TextRenderer API
SDL-specific event types in component APIs
universal ClipboardSystem
rich text editing
undo/redo before required
backend-independent text engine before required
```