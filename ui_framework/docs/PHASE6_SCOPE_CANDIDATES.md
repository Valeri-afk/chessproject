# Phase 6 — Scope and Status

This document is the current Phase 6 scope derived from Phase 5 component requirements. It is a living scope/status document, not a replacement for `ARCHITECTURE.md`.

## 1. Modality — implemented at source level

`ModalManager` provides framework-level modality infrastructure.

Current responsibilities include:

```text
active modal registration
modal stack/order
modal-root hit-testing restrictions
input routing restrictions
focus/capture policy
Escape routing
background interaction blocking
focus restoration after close
backdrop interaction policy
modal rendering/presentation order
```

The old standalone `Modal` component is deprecated/inactive. A public Modal component remains deferred unless the service-level API later proves insufficient.

Reference: `PHASE6_MODALITY_REQUIREMENTS.md`.

## 2. Scrolling — implemented at source level

`ScrollManager` is active framework infrastructure.

Current source-level responsibilities include:

```text
viewport/content extent derived from layout
scroll offset/range
clamping
wheel routing
nested residual-delta chaining
coordinate transformation
layout-derived content extent
hover refresh after handled scroll
```

Pointer input is normalized into renderer/logical coordinates before entering the framework input pipeline. Scroll presentation is then applied separately through the framework coordinate transform.

The implementation still requires build/runtime validation. A standalone `Scroll` / `ScrollArea` component is not currently required.

Reference: `SCROLL_ARCHITECTURE.md`.

## 3. Text input / editing — pending

`TextField / Input` remains blocked on a proper framework text-input/editing contract.

Candidate requirements:

```text
text input events
composition / IME
caret position
selection range
editing commands
clipboard interaction
keyboard navigation
repeat/backspace/delete behavior
focus integration
text-input lifecycle
```

Do not implement a full TextField component by extending the existing `KeyDown/KeyUp` path alone.

## 4. Image / resource infrastructure — pending

`Image` remains blocked on a stable resource/texture ownership model.

Candidate requirements:

```text
resource ownership/lifetime
shared texture references
loading/import boundary
safe renderer/resource relationship
resource handle abstraction
source rectangle
fit/crop/scale modes
opacity/tint/flip/rotation
presentation lifecycle
```

Keep this separate from `primitives`.

Reference: `PRIMITIVES_ROLE.md`.

## 5. Overlay / popup infrastructure — conditional

`Dropdown` currently works as a local composite using existing tree/layout mechanisms.

Do not introduce a global overlay subsystem unless concrete requirements appear for:

```text
escaping parent clipping
root-level popup placement
global popup ordering
outside-click dismissal across unrelated subtrees
popup collision/placement policy
```

## 6. Validation and stabilization — current boundary

The currently implemented Phase 6 infrastructure is at the validation/stabilization boundary.

The validation set is:

```text
full compilation
runtime smoke tests
automated component tests
modal interaction tests
scroll interaction tests
NodeTree/input/layout/render integration tests
lifetime/memory checks
source/include consistency checks
documentation consistency checks
```

No additional scrolling or modality subsystem should be introduced before this boundary is validated.

## 7. Explicit non-goals unless new evidence appears

```text
full theme system
generic animation manager
generic resource manager as a dumping ground
universal content model
CSS/Flexbox compatibility
large widget catalog
application-specific chess components
```

## 8. Scope decision rule

Phase 6 infrastructure should be promoted only when there is:

```text
concrete component requirement
        +
framework-level responsibility
        +
reusable contract
        +
clear ownership boundary
```

The purpose of Phase 6 is to implement missing reusable infrastructure and validate it in the real framework, not to absorb every deferred component automatically.
