# Component Design Guide

This document is the working guide for designing and reviewing new framework components. It is intentionally independent from the large architecture document and should be used as the practical checklist during Phase 5 and later component work.

## 1. Framework scope

The framework is not a complete UI component library.

A component belongs in the framework only when it is a generic, reusable UI concept with a clear contract that can be implemented on top of the framework infrastructure.

The chess application is a validation target only. Application-specific components remain client code.

Do not add a framework component merely because the application has a visually similar element.

## 2. First question: infrastructure or component?

Before implementing a component, determine whether the required behavior belongs to framework infrastructure.

A behavior is a strong infrastructure candidate when the component does not have the necessary tools to implement it itself, for example:

```text
layout calculation
child hit-testing
common event dispatch
input routing
visibility/enabled filtering
capture/focus infrastructure
modality
scroll coordination
```

The component should express the relevant semantic or visual state; framework infrastructure should provide low-level behavior that the component cannot reasonably implement itself.

This is not an absolute prohibition against complex components. A component may own meaningful semantic coordination when that behavior is intrinsic to its UI concept.

## 3. Node or PanelNode?

Use `Node` by default.

Use `PanelNode` only when the component actually owns structural children and therefore needs child ownership/layout infrastructure.

The following do **not** by themselves justify `PanelNode`:

```text
text
icon/image primitive
border/background
multiple visual primitives rendered directly by the component
```

A component may remain `Node` while internally using framework primitives for its visual representation.

Promote the component to `PanelNode` when child composition and child layout are part of the component's semantics.

Prefer an existing specialized layout node such as `StackPanelNode` when its layout flow already matches the component instead of reimplementing layout locally.

## 4. Primitive versus child component

A visual primitive may be represented internally without becoming a child `Node`.

`TextPrimitive` is the current example: it centralizes text measurement/rendering without forcing every text-bearing component to expose a `TextNode` child.

A primitive becomes a child component when structural composition, independent lifecycle, hit-testing, or independent component semantics make a child `Node` useful.

The fact that a primitive can technically be represented as a child does not require that design.

## 5. Content model

Do not introduce a universal `content` model as a default architectural rule.

The retained-mode, imperative framework intentionally does not assume that every component can contain every other component.

A specialized component may define its own accepted child types through its API and structure.

Examples:

```text
Menu       → MenuItem
TabControl → TabItem
List       → ListItem
```

The framework should support these relationships through component design rather than relying only on informal developer conventions.

## 6. State ownership

A component owns the semantic state of its own subsystem.

A composite component may intentionally modify the semantic state of its specialized children when that relationship is part of the component's public design.

Examples:

```text
Menu       → active/selected MenuItem
TabControl → selected/active TabItem
```

This is not a generic parent-controls-child rule. Do not add generic state properties to `Node` merely because two components happen to share a word such as `selected`, `active`, or `highlighted`.

State changes should produce presentation changes. Rendering should be a consequence of state rather than a separate synchronization contract exposed to the client.

## 7. Component-specific properties

Component-specific semantic and visual properties should normally remain direct component properties.

Do not create a new shared property structure merely because two components happen to have similarly named values.

Create a reusable structure only when there is a concrete, stable contract shared by multiple components and keeping it separate materially reduces duplication without exposing an artificial abstraction.

Framework-wide properties such as `visible`, `enabled`, `overflow`, padding, border, size constraints, and other Node-level behavior belong to the base infrastructure even when a component simply consumes them.

## 8. Inheritance

Inheritance is justified when the specialized component genuinely reuses the parent's semantic and visual contract.

For example:

```text
ToggleButton : Button
```

is currently justified because ToggleButton adds persistent selected state while reusing Button interaction, activation, layout, and presentation pipeline.

Do not introduce generic bases such as `ButtonBase`, `SelectableNode`, or `ContentNode` until multiple concrete components demonstrate an independently useful shared contract.

## 9. Composite component design

A composite component should separate three concerns:

```text
component state / coordination
        ↓
child semantic state
        ↓
child/component presentation
```

The composite component should not reimplement framework services such as event dispatch, hit-testing, or layout calculation when an existing infrastructure primitive already provides them.

## 10. Validation before implementation

For a new standard component, check the following before writing code:

1. Is it a generic UI concept rather than application-specific behavior?
2. What responsibilities must remain in framework infrastructure?
3. Does it actually require structural children?
4. If it has children, what child types are semantically valid?
5. Which state belongs to the component and which state belongs to its children?
6. Can an existing framework node or component already provide the required layout/interaction?
7. Is a new primitive or abstraction genuinely required, or can the component use existing infrastructure?
8. Does a specialized variant justify inheritance, or should it remain a sibling component?
9. Does the implementation introduce a contract that custom component developers must know or manually maintain?
10. Is the abstraction simple enough to keep the framework minimal?

## 11. External references

Material UI and similar libraries may be used as sources of ideas, concrete component examples, and composition patterns.

They are not architectural authorities for this framework. Their declarative React environment differs fundamentally from this retained-mode C++/SDL framework.

Copy concepts selectively; do not copy infrastructure assumptions.

## 12. Phase boundaries

If a component depends on a framework subsystem that is not implemented yet, record the component contract and requirements, but do not force an incomplete implementation into the current phase.

Example:

```text
Modal
    visual contract can be described in Phase 5
    final implementation depends on Phase 6 modality infrastructure
```

The same principle applies to Scroll if its final behavior depends on framework-level scroll coordination that has not yet been designed.

## 13. Implementation style

Keep component implementation small and semantic.

A component should normally:

```text
store component-specific state
expose public semantic properties/actions
respond to framework events through the existing event API
measure/draw its own visual representation
coordinate explicitly owned specialized children
```

A component should not normally:

```text
reimplement the NodeTree
reimplement hit-testing
reimplement global event dispatch
reimplement generic layout engines
invent client-side synchronization protocols for visual properties
```

When repeated implementation patterns appear, prefer another concrete example before introducing a framework-wide abstraction.
