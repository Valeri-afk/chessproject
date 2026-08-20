# Phase 6 — Core Development Status Checkpoint

This document records the exact development point reached after the Phase 5 component work and the initial Phase 6 modality and scrolling work.

It is intentionally separate from `ARCHITECTURE.md`. It is a working checkpoint for continuing development and for knowing when the framework core is ready for real compilation/runtime validation.

## 1. Current development point

The repository is currently in:

```text
Phase 6 — Framework Core / Subsystem Development
```

Phase 5 is considered complete as a component-development phase.

## 2. Phase 5 result

The active standard component set established during Phase 5 is:

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

The following remain deferred or are not framework components at this stage:

```text
TextField / Input    → deferred to text-input infrastructure
Image                → deferred to resource/texture infrastructure
Scroll               → framework-level scrolling behavior is now core infrastructure; a public Scroll / ScrollArea component remains deferred
Modal                → currently not required as a standalone component
Paper                → not a framework component
Label                → covered by text primitives / text nodes
Card                 → client-side composition/style pattern
```

Phase 5 also removed the old `Widget` / `ControlNode` direction from the active source architecture.

## 3. Phase 6 subsystem status

### Modality — completed as core infrastructure

The current framework has a service-level modality foundation through `ModalManager`.

Implemented responsibilities:

```text
modal registration
modal stack/order
modal input boundary
exclusive hit-testing for the active modal
focus restriction
pointer capture restriction
Escape routing
outside-click interception
backdrop interaction policy
backdrop visual layer
backdrop fade state
backdrop lifecycle
nested modal focus restoration
direct modal invalidation / removal cleanup
deferred-mutation-safe backdrop ownership
```

The current policy model is:

```text
BackdropClickBehavior
    Consume
    Close
```

The backdrop itself is an internal framework overlay node, not a public standard UI component.

A separate `Modal` component is intentionally not required yet. A client may use ordinary framework nodes/panels as modal content through the modality service.

### Scrolling — core behavior implemented; public component deferred

Scrolling is now implemented as framework-level behavior through `ScrollManager`, without introducing a standalone `Scroll` / `ScrollArea` component.

Implemented responsibilities:

```text
scroll state
viewport/content extent relationship
scroll offset/range and clamping
SDL wheel routing
nested scroll chaining with residual delta
presentation coordinate transform
hit-testing through transformed content
existing Overflow::HIDDEN clipping integration
layout-derived viewport/content extent
nested scroll container boundaries
scroll-state lifecycle cleanup
```

The current ownership model is:

```text
ScrollManager → scroll state and input routing
Node / UIManager → presentation coordinate transform
NodeTree → traversal, clipping and hit-test integration
LayoutManager → layout geometry; scroll does not mutate layout positions
```

Layout positions remain unchanged by scrolling. Scroll offset is stored separately from layout state.

A standard `Scroll` / `ScrollArea` component and scrollbar visuals remain intentionally deferred until the framework-level contract is validated by compilation/runtime behavior.

Reference: `SCROLL_ARCHITECTURE.md`.

### Text input / editing — not implemented yet

`TextField / Input` remains blocked on a proper framework-level text-input/editing contract.

Expected infrastructure:

```text
text input events
composition / IME
caret
selection
editing commands
clipboard
keyboard navigation
repeat/backspace/delete behavior
focus integration
text-input lifecycle
```

### Image / resource infrastructure — not implemented yet

`Image` remains blocked on a minimal reusable renderer/resource ownership model.

The goal is not a generic resource manager. The required abstraction should be limited to stable texture/resource ownership, lifetime and presentation needs.

`primitives` remain a low-level drawing layer and are not a resource system.

### Generic overlay / popup infrastructure — deferred

Do not create a separate overlay subsystem merely because `Dropdown` exists.

Introduce one only if a concrete requirement appears for:

```text
escaping parent clipping
root-level popup placement
global popup ordering
outside-click handling across unrelated subtrees
collision/placement policy
```

Modality currently uses the existing overlay mechanism and does not require a second overlay architecture.

## 4. What remains before core development is considered complete

The remaining core-development work is:

```text
1. Text input / editing infrastructure
2. Image / minimal resource ownership infrastructure
3. Re-evaluate generic overlay/popup infrastructure only if evidence requires it
4. Final subsystem integration review
5. Source-tree consistency review
6. Documentation checkpoint for completed Phase 6 core work
```

Scrolling is no longer on the implementation queue; it is now at the validation/stabilization boundary for core behavior.

Not every deferred component is guaranteed to become a framework component. Infrastructure is implemented only when a concrete reusable framework contract exists.

## 5. Validation boundary

Scroll and modality are now treated as implemented core infrastructure. The project should move to validation/stabilization rather than adding scrollbar visuals or a public Scroll component first.

The following work is required before declaring the current core boundary fully validated:

```text
full compilation
accumulated build validation
runtime smoke tests
component interaction tests
NodeTree/input/layout/render integration tests
modal interaction tests
scroll interaction tests
memory/lifetime checks
source/include consistency checks
```

The project should then use real build/runtime behavior to expose integration defects that static/source inspection cannot reliably prove.

## 6. Current operating rule

Until the remaining core-development boundary:

```text
Do not optimize for build cleanliness.
Do not expand the framework's component catalog unnecessarily.
Do not create application-specific chess functionality.
Do not add infrastructure without a concrete reusable responsibility.
Do not add a public Scroll / ScrollArea component before validation of the core behavior.
```

At the core-complete boundary:

```text
Stop adding new framework capabilities.
Run the full build.
Run the accumulated tests.
Fix integration defects.
Only then decide whether additional Phase 6 work is justified.
```

## 7. Immediate next step

The immediate next step is:

```text
Validation / stabilization of the implemented framework core,
starting with compilation and source/include consistency.
```

Modality and scrolling should be treated as completed infrastructure unless new evidence appears during validation.

Reference set:

```text
PHASE6_SCOPE_CANDIDATES.md
PHASE6_MODALITY_REQUIREMENTS.md
SCROLL_ARCHITECTURE.md
PRIMITIVES_ROLE.md
COMPONENT_DESIGN_GUIDE.md
```
