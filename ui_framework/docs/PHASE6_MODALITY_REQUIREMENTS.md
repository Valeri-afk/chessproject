# Phase 6 — Modality Requirements from Legacy Modal

This document records requirements extracted from the legacy `Modal` implementation and how they were translated into the current framework-level modality service.

## 1. What the legacy Modal contained

The old component combined several responsibilities:

```text
visual
    modal background/color
    border radius/padding
    backdrop rendering
    transition/fade effect

interaction
    close on mouse click
    close on Escape

framework behavior
    modal-exclusive input
    focus/capture interaction
    interaction with the rest of NodeTree
    potential scroll-lock behavior
```

These responsibilities are no longer expected to live inside a standalone Modal component.

## 2. Current framework ownership

The current source implements modality through `ModalManager`.

The service owns framework-level behavior such as:

```text
modal registration
modal stack/order
active modal root
exclusive input boundary
focus restriction
pointer/capture restriction
Escape routing
outside/backdrop interaction
backdrop behavior policy
backdrop visual overlay state
fade state
nested focus restoration
modal-session cleanup
```

The current backdrop policy is:

```text
BackdropClickBehavior
    Consume
    Close
```

The backdrop is an internal framework overlay node, not a public standard UI component.

## 3. Current component decision

A standalone public `Modal` component is **not currently required**.

Client code may compose modal content from ordinary framework nodes while `ModalManager` provides the modality behavior.

The legacy Modal implementation is therefore deprecated/inactive and is retained only as historical/design reference.

## 4. What should not be copied literally

The old Modal handled `MouseClickEvent` and `KeyDownEvent` directly in the component. That is not the target architecture.

The current design keeps event routing and interaction boundaries in framework infrastructure.

The old implementation also mixed backdrop alpha animation into component background state. Current modality keeps backdrop/fade state in the modality subsystem instead.

## 5. Remaining validation

The modality subsystem is considered source-level implemented, but it still requires:

```text
full compilation
runtime modal interaction tests
outside-click tests
Escape tests
focus restoration tests
nested modal tests
backdrop fade/lifecycle tests
mutation/lifetime tests
```

Any architectural changes should be driven by failures found during this validation rather than by recreating the legacy Modal component.
