# Phase 5 — Minimal Standard UI Component Catalog

## Scope

Phase 5 established a small reusable standard UI layer without expanding into unresolved framework subsystems.

The chess application is a validation target only. Chess-specific components remain client code.

The `components` layer is an active, supported framework layer. It is **not deprecated**.

## Base layer

```text
Node
PanelNode
StackPanelNode
TextPrimitive / TextNode
```

## Standard UI components

### Established in Phase 5

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

These components have distinct generic contracts and are intended to be reusable independently of the chess application.

`Dropdown` remains a local composite using existing tree/layout mechanisms. It does not by itself justify a global overlay subsystem.

## Framework-level behavior

### Scroll

Scroll is no longer treated as a deferred component-only concern. Framework-level scrolling behavior is implemented through `ScrollManager` with `UIManager`/NodeTree integration.

The current source-level behavior covers scroll state, extents, clamping, wheel routing, nested chaining and coordinate transformation. It still requires runtime/build validation.

A standalone `Scroll` / `ScrollArea` component remains optional and deferred.

### Modal

Modality is implemented through `ModalManager` as framework infrastructure.

The legacy Modal component is deprecated/inactive. A standalone public Modal component is not currently required.

## Analyze before implementation

### TextField / Input

Requires proper text-input/editing infrastructure, including text input events and potentially composition/IME, caret, selection and clipboard/editing behavior.

### Image

Requires a stable framework resource/texture ownership contract. Do not expose ad-hoc `SDL_Texture*` ownership from the component.

## Cancelled as standalone framework components

```text
Paper
Label
Card
```

`Paper` is a visual surface/elevation styling pattern.

`Label` duplicates the generic text role already provided by `TextNode` / `TextPrimitive`.

`Card` is a composition/style pattern that can be built from `PanelNode`, layout primitives and visual properties.

## Not currently required

```text
List
IconButton
```

`List` has no sufficiently distinct generic contract yet.

`IconButton` remains dependent on a stable graphics/icon/resource contract.

## Promotion rule

A new component should enter the framework only when all of the following are reasonably true:

```text
standard reusable UI concept
        +
independent generic contract
        +
not merely a renamed existing Node/layout primitive
        +
implementation fits the framework/component responsibility boundary
```

Do not add a framework component merely because an application contains a visually similar element.
