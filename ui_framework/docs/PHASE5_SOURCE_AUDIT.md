# Phase 5 — Source Audit

This document records the final Phase 5 source cleanup and the current source-level boundary entering Phase 6.

## Removed / obsolete

The following obsolete abstractions and orphan implementations are not part of the active source architecture:

```text
components/component.hpp
components/paper.hpp
components/paper.cpp
components/label.hpp
components/label.cpp
components/flex_panel.hpp
components/flex_panel.cpp
core/controlnode.hpp
core/controlnode.cpp
src/components/flex_panel.cpp
src/components/label.cpp
src/core/controlnode.cpp
src/core/gridnode.cpp
include/ui_framework/components/modal.hpp
```

Historical implementations may be kept only when they are explicitly useful as reference material for an unresolved design question.

## Active framework infrastructure

```text
UIManager
NodeTree
InputManager
EventDispatcher
EventHandlerStorage
LayoutManager
RenderingState
ModalManager
ScrollManager
```

Current responsibilities remain separated: UIManager coordinates frame/event flow; NodeTree owns live-node structure, traversal and mutation safety; InputManager owns transient input and SDL-to-framework input processing; EventDispatcher owns event propagation; LayoutManager owns measure/arrange; RenderingState owns renderer-state preservation; ModalManager owns modality; ScrollManager owns scroll state and wheel routing.

## Modality

`ModalManager` is active Phase 6 infrastructure, not merely preparation.

The active source contains modal stack handling, modal-root filtering, focus/capture restrictions, Escape routing, backdrop interaction, focus restoration and modal-session cleanup.

The legacy `Modal` component is removed from the active source tree. A standalone public `Modal` component remains intentionally deferred.

## Scrolling

`ScrollManager` is active Phase 6 core infrastructure.

The current source provides:

```text
scroll state
viewport/content extent relationship
offset / maximum offset
clamping
accumulated ancestor offsets
wheel routing
nested residual-delta chaining
layout-derived content extent
```

`UIManager` applies the accumulated scroll offset as a coordinate transform during input and render traversal. Stored layout positions remain unchanged.

The active implementation is still **source-level only** until full compilation and runtime validation are performed.

A standalone `Scroll` / `ScrollArea` component and scrollbar visuals remain deferred.

## Retained primitives

### `core/primitives.*`

Retained as the low-level SDL drawing helper layer. It provides reusable stateless drawing operations and does not own component semantics or resource ownership.

Its role is documented in `PRIMITIVES_ROLE.md`.

### `core/text_primitive.*`

Retained as the internal text measurement/rendering implementation.

### `core/textnode.*`

Retained as the NodeTree-facing text component that adapts text primitives to node geometry and lifecycle.

## Active standard components

The active Phase 5 component layer contains:

```text
Button
ToggleButton
Menu
MenuItem
TabControl
TabItem
Checkbox
RadioButton
Slider
Dropdown
```

The `components` layer is active and supported. It is not deprecated.

## Deferred components

```text
TextField / Input
Image
List
IconButton
```

`TextField / Input` remains blocked on framework text-input/editing infrastructure.

`Image` remains blocked on a stable resource/texture ownership contract.

`List` remains deferred because no distinct generic contract has yet justified a standalone abstraction.

`IconButton` remains deferred until a stable graphics/icon/resource contract exists.

## Cancelled standalone concepts

```text
Paper
Label
Card
```

These are currently treated as styling/composition patterns rather than independent framework components.

## Source-audit status

There are no currently accepted active references to the removed `Widget` / `ControlNode` model or the removed active Modal header.

The remaining important source-level work is integration validation rather than another Phase 5 component sweep:

```text
full compilation
runtime smoke tests
modal interaction tests
scroll interaction tests
render/input/layout integration tests
memory/lifetime checks
source/include consistency checks
```
