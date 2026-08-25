# Phase 6 — Scope and Status

This is the living Phase 6 scope/status document. It describes the current framework-core boundary and should be preferred over older phase planning notes.

## 1. Modality — implemented and runtime-validated

`ModalManager` provides framework-level modality infrastructure.

Current responsibilities include:

```text
active modal registration
modal stack/order
modal-root hit-test restrictions
input routing restrictions
focus/capture policy
Escape routing
background interaction blocking
focus restoration
backdrop interaction policy
modal rendering/presentation order
```

The old standalone `Modal` component is deprecated/inactive. A public Modal component remains unnecessary unless the service-level API later proves insufficient.

## 2. Scrolling — implemented, pending dedicated behavioral validation

`ScrollManager` is active framework infrastructure.

Current responsibilities include:

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

The core integration is present and the application is running correctly, but dedicated scroll behavior should still be validated before declaring the subsystem completely closed.

A standalone `Scroll` / `ScrollArea` component is not currently required. Scrollbar visuals are deferred.

Reference: `SCROLL_ARCHITECTURE.md`.

## 3. Layout / invalidation — implemented

The framework now has the intended two-phase layout execution:

```text
measure
  ↓
arrange
  ↓
final geometry
```

Layout invalidation is explicit. A normal setter does not automatically invalidate layout merely because it changes state. A component may invalidate internally when a semantic operation necessarily changes layout, while client code can also invalidate explicitly when required.

The important runtime rule is:

```text
layout change
   +
no invalidation
   ↓
no required re-measure/re-arrange

layout change
   +
invalidation
   ↓
layout is recomputed on the next layout pass
```

Current layout/invalidation details are documented in `LAYOUT_REFACTOR_CHECKPOINT.md`.

## 4. Text layout — implemented

The text architecture is now based on explicit content/layout/render responsibilities:

```text
Typography / text-bearing controls
              ↓
         TextContent
          ↙       ↘
   TextLayout   TextRenderer
```

Text measurement and wrapping are no longer treated as an ad-hoc component-local concern.

Current details are documented in `TEXT_ARCHITECTURE_CHECKPOINT.md`.

## 5. Text input / editing — pending

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

Do not implement a full TextField component by extending the existing key-event path alone.

## 6. Image / resource infrastructure — pending

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

## 7. Overlay / popup infrastructure — conditional

`Dropdown` currently works as a local composite using existing tree/layout mechanisms.

Do not introduce a global overlay subsystem unless concrete requirements appear for:

```text
escaping parent clipping
root-level popup placement
global popup ordering
outside-click dismissal across unrelated subtrees
popup collision/placement policy
```

## 8. Current validation boundary

The next work is validation/stabilization, not another architectural rewrite:

```text
full build
runtime smoke tests
scroll interaction tests
nested scroll tests
NodeTree/input/layout/render integration tests
lifetime/memory checks
source/include consistency checks
documentation consistency checks
```

The current chess client already provides a useful visual/runtime validation target for rendering, logical presentation, components, callbacks and modal behavior.

## 9. Explicit non-goals unless new evidence appears

```text
full theme system
generic animation manager
generic resource manager as a dumping ground
universal content model
CSS/Flexbox compatibility
large widget catalog
application-specific chess components
```

## 10. Promotion rule

New framework infrastructure should be added only when there is:

```text
concrete component/application requirement
        +
framework-level responsibility
        +
reusable contract
        +
clear ownership boundary
```

Phase 6 exists to complete reusable framework infrastructure and validate it in the real application, not to absorb every possible UI feature.