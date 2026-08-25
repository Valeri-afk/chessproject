# Phase 5 — Source Audit (Historical Boundary)

This document records the final Phase 5 source cleanup and the boundary that was carried into Phase 6. It is historical context, not a current implementation checklist.

## Removed / obsolete at the Phase 5 boundary

The following legacy abstractions were removed from the active architecture:

```text
components/component.hpp
components/paper.hpp / paper.cpp
components/label.hpp / label.cpp
components/flex_panel.hpp / flex_panel.cpp
core/controlnode.hpp / controlnode.cpp
core/gridnode.cpp
include/ui_framework/components/modal.hpp
```

The old `Widget` / `ControlNode` direction is not part of the active framework architecture.

## Phase 5 standard component set

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

The `components` layer remains an active supported framework layer.

## Framework infrastructure extracted from component requirements

Two responsibilities were deliberately moved below the component layer during the subsequent core work:

```text
Modality  → ModalManager
Scrolling → ScrollManager + UIManager/NodeTree integration
```

A standalone public `Modal` or `Scroll/ScrollArea` component is not currently required.

## Current text architecture

The historical Phase 5 text implementation is no longer authoritative. The current text path is documented in `TEXT_ARCHITECTURE_CHECKPOINT.md`:

```text
Typography / text-bearing controls
              ↓
         TextContent
          ↙       ↘
   TextLayout   TextRenderer
```

`TextPrimitive`, `TextNode`, and `TextRuntime` are not part of the active text architecture/build graph.

## Current ownership boundary

```text
UIManager       → public facade / frame orchestration
NodeTree        → live tree, ownership, traversal, mutation safety
InputSystem     → input state and routing
EventDispatcher → event propagation
LayoutSystem    → Measure / Arrange execution
ModalSystem     → modality infrastructure
ScrollManager   → scroll state / wheel routing
TextLayout      → logical text measurement and wrapping
TextContent     → component-facing text state and layout bridge
TextRenderer    → internal SDL_ttf rendering/backend state
```

## Deferred capabilities

These remain deferred because they require concrete reusable infrastructure beyond the current core:

```text
TextField / Input → text editing / IME / caret / selection infrastructure
Image             → stable resource / texture ownership
List              → distinct generic contract not yet demonstrated
IconButton        → stable graphics/icon/resource contract
```

## Historical completion state

Phase 5 is complete. The project has moved beyond component-catalog work into framework-core validation/stabilization.

For current work, use:

```text
CURRENT_REFACTOR_DIRECTION.md
LAYOUT_REFACTOR_CHECKPOINT.md
TEXT_ARCHITECTURE_CHECKPOINT.md
ROADMAP.md
SCROLL_ARCHITECTURE.md
```
