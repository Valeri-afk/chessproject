# Phase 5 — Final Checkpoint

Phase 5 was the component-development phase. Its purpose was to establish a small, reusable standard UI layer without forcing unresolved framework subsystems into component classes.

## Active component set established in Phase 5

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

These components have distinct generic contracts and remain active framework components.

## Deferred component work

```text
TextField / Input
Image
List
IconButton
```

Reasons:

- `TextField / Input` requires framework text-input/editing infrastructure beyond key events.
- `Image` requires a stable resource/texture ownership and lifetime contract.
- `List` has not yet demonstrated a sufficiently distinct generic contract over existing panel/layout primitives.
- `IconButton` depends on a stable graphics/icon/resource contract.

## Framework-level requirements extracted from Phase 5

Two concerns were deliberately moved below the component layer during subsequent Phase 6 work:

```text
Modality → ModalManager
Scrolling → ScrollManager + UIManager/NodeTree integration
```

### Modality

`ModalManager` now provides the framework-level modality behavior. The legacy Modal component is deprecated/inactive and remains only as historical reference.

A standalone public Modal component is not currently required.

### Scrolling

`ScrollManager` now provides framework-level scrolling behavior, including scroll state, viewport/content extents, offset/range clamping, wheel routing, nested chaining and coordinate transformation.

The implementation is still source-level only until full build/runtime validation.

A standalone `Scroll` / `ScrollArea` component and scrollbar visuals remain deferred.

## Cancelled standalone framework components

```text
Paper
Label
Card
```

They are not required as independent runtime concepts:

- `Paper` is a surface/style pattern.
- `Label` is covered by `TextNode` / `TextPrimitive`.
- `Card` is a composition/style pattern that can be built from panels, layout and visual properties.

## Component architecture established in Phase 5

```text
Node
 ├── leaf visual/interactive components
 └── PanelNode
      └── StackPanelNode
           └── composite standard components when the child structure is semantic
```

Components own semantic state and presentation. Framework subsystems own generic traversal, input routing, event propagation, layout processing, renderer state, modality and scrolling.

A component must not reimplement framework-level behavior merely to simplify its own implementation.

## Presentation rule

Simple geometric presentation is drawn through `primitives`.

Complex texture-based presentation should use a future Image/resource layer rather than forcing component-specific SDL texture ownership into the component layer.

A component may eventually combine both approaches.

## Phase 5 source boundary

Legacy `Widget` / `ControlNode` component infrastructure has been removed from the active source tree.

`components/` is an active framework layer and is not deprecated.

## Completion status

Phase 5 is considered complete as a component-development phase.

The project is now in Phase 6 framework-core/subsystem development. Remaining work is infrastructure completion plus build/runtime validation, not expansion of the Phase 5 component catalog.
