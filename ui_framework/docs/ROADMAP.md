# Development Roadmap

This document defines the planned development order of the framework. The source code remains the source of truth for current behavior.

## Current Development Status

### Current Phase

**Phase 6 — Framework Core / Subsystem Development**

The repository is now consolidated on `main`. Phase 5 component development produced the current standard component set and the framework-level requirements that were carried into Phase 6.

### Current branch

```text
main
```

### Phase 5 result

Phase 5 remains historically the component-development phase. Its final component scope was:

```text
Button
ToggleButton
Menu / MenuItem
TabControl / TabItem
Checkbox
RadioButton
Slider
Dropdown
```

The following were not promoted as standalone components:

```text
Paper
Label
Card
```

TextField/Input and Image remain dependent on framework infrastructure that is not yet complete.

Modal was not promoted as the primary API surface. Modality is implemented through `ModalManager` as a framework service.

Scroll is likewise treated as framework-level behavior/infrastructure rather than as a mandatory standalone Scroll component.

### Current Phase 6 source-level status

The current source contains framework-level implementations for:

```text
ModalManager
ScrollManager
SDL logical viewport synchronization
SDL render/logical input-coordinate conversion
scroll coordinate transformation
wheel-to-scroll routing
nested scroll chaining
content/viewport extent calculation
hover refresh after scroll
```

These are **source-level implementations**, not yet runtime-validated releases.

The project is now at the **validation/stabilization boundary** for this core work. New subsystem expansion should not precede source/documentation consistency review and later runtime validation.

## Phase 1 — Runtime

Completed at source level and accepted as the runtime baseline.

## Phase 2 — Layout

Completed at source level. Current layout responsibilities include flow layout, measurement, constraints, padding, border, position, alignment, gap, visibility and absolute-child separation.

## Phase 3 — Input / Events

Completed at source level. Current source includes mouse input, hit-test, hover/click generation, capture, focus, keyboard routing, event propagation and modal-root filtering.

## Phase 4 — Rendering / Backend

Completed at source level. Current source uses SDL3 rendering, layout final geometry, renderer state isolation and clipping.

## Phase 5 — Component Development

Completed as the focused component-development phase. The phase should not be expanded retroactively to absorb every framework subsystem discovered afterwards.

Its important architectural result is that components do not redefine ownership, layout orchestration, input dispatch, hit-testing, rendering, clipping, scrolling, text editing or resource ownership.

## Phase 6 — Framework Core / Subsystem Development

### Current status

**Core implementation at validation/stabilization boundary.**

Phase 6 is where framework-level infrastructure discovered during Phase 5 is implemented and validated. The current core behavior for modality, scrolling, viewport handling and input-coordinate conversion is implemented at source level.

### Current subsystem work

#### Modality

`ModalManager` is the framework-level modality service.

Current responsibilities include modal registration/stack handling, backdrop state, modal-root input filtering, Escape handling, pointer/backdrop handling and viewport synchronization.

A separate public Modal component is not currently required.

#### Scrolling

`ScrollManager` now owns scroll state and provides:

```text
viewport extent
content extent
scroll offset
maximum offset
clamping
nested scroll accumulation
wheel routing
layout-derived content extent
```

`UIManager` applies the accumulated scroll offset as a coordinate transform during input/render traversal. Pointer input is normalized to renderer/logical coordinates before entering the framework, and hover is refreshed after a handled scroll operation.

A standalone `Scroll` / `ScrollArea` component and scrollbar presentation remain intentionally deferred.

#### Viewport

The framework viewport is the SDL logical presentation size when logical presentation is configured. The framework obtains it directly from the renderer rather than requiring a client-side `UIManager::setViewportSize()` call.

If logical presentation is unavailable, the renderer output size is used as fallback.

### Remaining framework work before broader feature expansion

```text
Source/documentation consistency review
Validation/stabilization of existing core behavior
Text input / editing
Image / resource ownership
Overlay/popup infrastructure only if concretely required
```

Not every candidate must become a subsystem. Each must be justified by an actual framework requirement.

## Validation and Completion

The final Phase 6 completion procedure is:

1. Audit all active source files.
2. Verify public APIs against actual subsystem responsibilities.
3. Verify render/input/layout integration in runtime.
4. Run full compilation and tests.
5. Perform source/include/dead-file cleanup.
6. Review all living `.md` documents against the resulting source.
7. Manually review `ARCHITECTURE.md` for any architectural decisions that should be incorporated.

`ARCHITECTURE.md` remains a separately maintained architectural document and should not be mechanically rewritten by routine implementation work.

## Development Order

```text
Phase 1 — Runtime
        ↓
Phase 2 — Layout
        ↓
Phase 3 — Input / Events
        ↓
Phase 4 — Rendering / Backend
        ↓
Phase 5 — Component Development
        ↓
Phase 6 — Framework Core / Subsystem Development
        ↓
Validation / stabilization
        ↓
Only then decide on additional framework capabilities
```
