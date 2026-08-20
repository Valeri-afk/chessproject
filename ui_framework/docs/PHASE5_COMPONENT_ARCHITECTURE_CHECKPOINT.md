# Phase 5 — Component Architecture Checkpoint

This document records the component architecture established during Phase 5. It is historical/operational context for the current framework and does not replace `ARCHITECTURE.md`.

## Framework layers

```text
Framework infrastructure
    ↓
Base nodes / layout / rendering primitives
    ↓
Standard UI components
    ↓
Application-specific composition
```

The framework is intentionally minimal. A chess application is a validation target, not a source of chess-specific framework components.

## Base types

### `Node`

Default base for visual components. Use it unless the component genuinely requires structural child ownership and child layout.

### `PanelNode`

Structural/layout base for components that own children and manage their geometry/composition.

Text, icons, images, borders, or multiple visual primitives do not by themselves justify `PanelNode`.

### `StackPanelNode`

Reusable child-flow layout primitive. Composite components should reuse it when its layout policy matches their semantics instead of reimplementing measurement/arrangement.

## Component responsibility

Components own:

```text
component-specific semantic state
component-specific visual properties
presentation
semantic actions/callbacks
coordination of intentionally owned specialized children
```

Framework infrastructure owns coordinated generic mechanisms such as:

```text
NodeTree lifecycle/traversal
layout/geometry processing
hit-testing
input/event dispatch
focus/capture
render traversal
clipping
mutation/update scheduling
modality
scroll mechanics
```

Boundary rule: if a component cannot reasonably implement a behavior with the tools exposed by its framework APIs, that behavior is a strong infrastructure candidate.

## Primitives

A visual primitive does not have to be a Node.

`TextPrimitive` is an internal reusable text measurement/rendering implementation. `TextNode` adapts that implementation to the NodeTree.

```text
TextPrimitive
    ↓
TextNode / text-bearing components
```

A primitive becomes a Node/component only when independent lifecycle, layout participation, hit-testing, or semantic behavior makes that useful.

`primitives` remain the low-level geometric drawing layer. They do not define component semantics or resource ownership.

## Children and content

There is no universal `content` model.

Specialized components may define explicit semantic child contracts:

```text
Menu       → MenuItem
TabControl → TabItem
```

Structural child ownership and semantic content are separate concepts.

## State ownership

Composite components may coordinate their intentionally specialized children:

```text
Menu       → active/selected MenuItem
TabControl → selected/active TabItem
```

This does not justify generic `Node::selected`, `Node::active`, or `SelectableNode` abstractions. Similar property names are not sufficient evidence of a shared semantic contract.

## Inheritance

Use inheritance only when the specialized component genuinely extends a stable parent contract.

Current justified example:

```text
ToggleButton : Button
```

Do not introduce `ButtonBase`, `ActionNode`, `SelectableNode`, or similar abstractions until concrete components prove a stable reusable contract.

## Layout rule

Ask:

> Does this component require structural child ownership/layout as part of its semantics?

If no → `Node`.

If yes → `PanelNode` or an existing specialized panel such as `StackPanelNode`.

## Phase 5 standard components

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

These are the standard component layer established by Phase 5.

## Concepts intentionally not promoted to components

```text
Paper
Label
Card
```

They are currently styling/composition patterns covered by generic nodes, text primitives and layout/visual properties.

## Deferred components

```text
TextField / Input
Image
List
IconButton
```

Their implementation depends on unresolved or incomplete framework infrastructure rather than a missing simple component class.

## Phase 6 infrastructure discovered from Phase 5

The following responsibilities have already moved below the component layer:

```text
Modality → ModalManager
Scrolling → ScrollManager + UIManager/NodeTree integration
```

The legacy `Modal` implementation is deprecated/inactive and is retained only as historical reference. The `components` layer itself is **not deprecated**.

A public `Modal` component is not currently required. A public `Scroll` / `ScrollArea` component is also not currently required; the framework-level scrolling behavior is the reusable contract being validated first.

## Current phase boundary

Phase 5 is complete as a component-development phase.

Current framework work belongs to Phase 6 and is focused on:

```text
text input/editing infrastructure
image/resource infrastructure
validation/stabilization of modality and scrolling
full source/build/runtime integration
```
