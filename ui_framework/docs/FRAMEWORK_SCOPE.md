# Framework Scope and Purpose

## Why this framework exists

The project originated from a chess application, but the UI framework is not a chess framework and is not intended to become a complete widget library.

It provides a small retained-mode C++/SDL3 runtime in which independently implemented UI objects can participate in one coherent system with shared ownership, lifecycle, traversal, layout, input, events and rendering.

The chess application is the immediate validation target, not the source of application-specific framework components.

## What the framework is

The framework provides generic infrastructure for:

- hierarchical UI ownership and lifetime;
- lifecycle and safe traversal;
- deferred structural mutation;
- update and rendering traversal;
- input coordination, focus, capture and hit testing;
- event dispatching;
- Measure → Arrange layout and explicit invalidation;
- scrolling and modality as framework services;
- low-level rendering primitives;
- a small standard component set;
- custom `Node` and `PanelNode` extension points.

It is intentionally narrower than Qt, WPF or a universal application toolkit.

## Application boundary

```text
Chess Engine / Domain
        ↓
Chess Client / Application
        ↓
UI Framework
```

The chess engine owns chess state and rules. The client owns application meaning and behavior. The framework owns reusable UI runtime mechanisms and standard generic UI concepts.

## Framework responsibilities

The framework owns coordinated mechanisms required to make many UI objects operate as one runtime:

```text
runtime structure
  ownership / lifetime / registration / traversal / mutation safety

layout
  Measure / Arrange / constraints / scheduling / geometry commit

interaction
  input routing / focus / capture / hit testing / event propagation

rendering
  render traversal / clipping / ordering / renderer-state safety

services
  modality / scrolling
```

Custom components should use these mechanisms instead of implementing competing runtime systems.

## Client responsibilities

The client owns application-specific state and meaning, for example:

- chess rules and engine integration;
- game state and clocks;
- navigation intent;
- application-specific semantics and callbacks;
- custom component behavior.

Example:

```text
Framework:
    detect click
    dispatch event
    invoke registered behavior

Client:
    interpret "New Game"
    reset application state
```

## Current standard components

```text
Button
ToggleButton
Menu / MenuItem
TabControl / TabItem
Checkbox
RadioButton
Slider
Dropdown
Typography
StackPanelNode / PanelNode
```

The framework remains intentionally minimal. `Paper`, `Label` and `Card` are composition/styling patterns rather than mandatory framework components.

## Infrastructure vs component

Before adding a component, determine whether the required behavior is infrastructure:

```text
layout calculation
child hit-testing
common event dispatch
input routing
focus/capture
modality
scroll coordination
```

A component belongs in the framework when it is a generic reusable UI concept with a clear contract. Application-specific composition stays in client code.

Use `Node` by default. Use `PanelNode` only when structural children and child layout are part of the component's semantics. `StackPanelNode` should be reused when its linear layout policy matches the required behavior.

## Current service decisions

### Modality

Modality is framework infrastructure behind the public `UIManager` facade. A standalone public `Modal` component is not currently required.

### Scrolling

Scrolling is framework infrastructure. A standalone public `Scroll` / `ScrollArea` component is not currently required. Scrollbar presentation is also deferred until scroll behavior has dedicated runtime validation and a stable visual contract.

### Text

The active text architecture is:

```text
Typography / text-bearing controls
        ↓
    TextContent
      ↙     ↘
 TextLayout  TextRenderer
```

`TextLayout` owns logical measurement/wrapping; `TextRenderer` is internal backend rendering. Source `TTF_Font*` remains client-owned; derived rendering resources are framework-owned.

Text input/editing is not implemented yet and is intentionally isolated as a future subsystem rather than a base `Node` concern.

## Ownership and lifetime

The framework uses `std::unique_ptr` as the fundamental structural ownership mechanism.

Client-held `Node*` references are non-owning. Live membership is authoritative in `NodeTree` behind `UIManager`.

Structural mutations are framework-managed and deferred when required for traversal safety.

For text, the source `TTF_Font*` is non-owning from the framework perspective and must outlive its text users. No general ResourceManager is justified until concrete requirements such as shared ownership, unloading, replacement or hot reload appear.

## Reparenting

Reparenting is a future capability, not a current requirement. It should only be introduced after a concrete use case such as drag-and-drop, docking or tab transfer establishes the need.

## Design philosophy

1. Runtime correctness before component breadth.
2. Client extensibility without losing framework runtime control.
3. Minimal sufficient public API.
4. Concrete requirements before abstractions.
5. Application/domain logic stays outside the framework.
6. Components do not reimplement global runtime mechanisms.
7. The source code is authoritative for current behavior.
8. Documentation describes stable contracts and intentional future boundaries, not historical phase checkpoints.

## Non-goals

- chess rules or engine logic;
- application-specific business logic;
- universal arbitrary-content composition;
- CSS/Flex/Grid feature parity;
- a universal property-registration/dependency system;
- a second event system;
- mandatory generic component base classes;
- feature parity with larger UI toolkits.

## Future capability rule

A capability should be added when it is a real reusable responsibility of the UI runtime or is repeatedly required by the supported application class, and when its developer contract remains appropriately small.

Deferred examples include:

```text
TextField / editable text
Image / resource ownership
List
IconButton
scrollbar presentation
standalone Scroll / ScrollArea component
standalone Modal component
```

These are requirements to evaluate, not commitments to implement blindly.
