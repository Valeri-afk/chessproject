# Rendering Primitives Role

## Purpose

`core/primitives.hpp` and `src/core/primitives.cpp` are low-level rendering helpers for framework components and framework rendering code.

They are **not UI components**, do not participate in the Node tree, and do not define component semantics.

## Current role

The module provides immediate SDL rendering operations such as:

```text
pixel
line
horizontal/vertical line
rectangle outline
filled rectangle
rounded rectangle outline
filled rounded rectangle
arc
circle / ellipse helpers
```

The purpose is to centralize small drawing algorithms that would otherwise be duplicated across components.

For example, a `Button` may use primitive drawing operations for its background and border while remaining a `Node` component.

## Architectural boundary

Primitives should remain below the component layer:

```text
Node / component
    ↓
component-specific visual state
    ↓
rendering primitives
    ↓
SDL renderer
```

A primitive should not know about:

```text
Node
PanelNode
component state
layout ownership
input events
selection
focus
modality
application semantics
```

Likewise, a component should not expose the primitive namespace as its public semantic API merely because it uses it internally.

## Primitive versus visual component

A primitive is appropriate when the operation is:

- stateless or nearly stateless;
- directly expressible as a drawing operation;
- reusable by multiple components;
- independent from Node lifecycle and layout;
- not independently interactive.

A `Node`/component is appropriate when the object has its own:

- semantic state;
- layout participation;
- event handling;
- lifecycle;
- hit-testing;
- independent presentation contract.

This distinction is important for future icon/graphics work. An icon renderer may use primitive/resource infrastructure, while an `IconButton` remains a standard UI component with its own interaction semantics.

## What should remain in this module

Keep generic drawing algorithms here when they are genuinely reusable.

Do not add helpers merely to move component-specific drawing code out of a component. A helper should represent a useful rendering primitive rather than a hidden component implementation.

## What should not be added here

Do not turn `primitives` into a general graphics/resource system.

The following require separate architectural decisions if they become necessary:

```text
textures/resources
icons
fonts/text layout
images
clipping
transforms
shadows
animations
styling/theme system
```

Some of these may eventually have lower-level rendering infrastructure, but they should not be inserted into this module simply because SDL can technically render them.

## Current conclusion

The primitive module is useful and should remain as a small rendering layer. It should be kept deliberately narrow.

The current `TextPrimitive` is intentionally separate because text measurement and rendering have state and intrinsic-size behavior that make a dedicated reusable primitive useful to components.

The existence of rendering primitives does not imply that every visual element needs a corresponding primitive class or every primitive needs a public UI component.
