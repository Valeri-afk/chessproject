# Text Input System — Future Contract

## Status

Text input/editing is not implemented yet. This document is a prepared architectural boundary, not an implementation specification.

## Scope

A future text input subsystem should provide editable text behavior without moving text-editing semantics into base `Node`.

Expected responsibilities:

```text
editing buffer
caret position
selection
keyboard editing
text insertion/deletion
clipboard operations if required
composition / IME if required
focus integration
text-layout integration
caret/selection rendering
```

## Relationship to existing text

The future path should build on the existing logical text stack:

```text
TextInput component
      ↓
editable text state
      ↓
TextLayout
      ↓
TextRenderer
```

`TextLayout` remains responsible for logical measurement/wrapping/line metrics. The input subsystem should consume layout information needed for caret and selection once those concrete APIs are justified.

## Focus

Editable text must participate in the existing framework focus model. Keyboard input should route to the focused editable control through `InputSystem` and the normal event dispatch mechanism.

The framework should not introduce a second keyboard event bus solely for text editing.

## Editing vs rendering

Editing state and rendering state must remain separate:

```text
editing model
    ↓
layout resolution
    ↓
rendered caret/selection/text
```

The text renderer should not become the owner of the editing buffer or editing semantics.

## Layout invalidation

Text edits can change desired size, wrapping and line count. The component must explicitly report the resulting layout consequence according to the current invalidation contract.

```text
edit text
   ↓
text state changes
   ↓
layout may become stale
   ↓
invalidateLayout()
```

Render-only caret blink/state changes should not require Measure/Arrange.

## Future IME boundary

If IME/composition is required, composition state should be isolated inside the text-input subsystem rather than added to generic `Node` or `TextRenderer`.

## Intentionally deferred

```text
TextInput component API
caret metrics
selection geometry
IME implementation
clipboard abstraction
undo/redo
rich text editing
```

These should be designed from a concrete use case rather than preemptively generalized.
