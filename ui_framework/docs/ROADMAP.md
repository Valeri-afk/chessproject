# Development Roadmap

This document defines the planned development order of the framework. The source code remains the source of truth for current behavior.

## Current Development Status

### Current Phase

**Phase 6 — Framework Core / Subsystem Development**

The framework core is now at the **validation/stabilization boundary** on the recovery branch. The major runtime, layout, event, rendering, modality, scrolling, and logical text work relevant to the current boundary is implemented and has been visually/runtime validated in the chess client.

### Current branch

```text
recovery-before-node-tree-break
```

The recovery branch is the current working baseline for continued framework development. It preserves the working `NodeTree` implementation and the validated core behavior.

### Phase 5 result

Phase 5 component development produced the current standard component set:

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

### Current Phase 6 source/runtime status

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
Measure → Arrange layout execution
layout invalidation and root-based coalescing
TextLayout / TextContent / TextRenderer integration
```

These have now reached the validation/stabilization boundary rather than being merely planned architecture.

## Phase 1 — Runtime

Completed and accepted as the runtime baseline.

## Phase 2 — Layout

Completed. Current layout responsibilities include flow layout, measurement, constraints, padding, border, position, alignment, gap, visibility and absolute-child separation.

The Measure → Arrange contract and explicit layout invalidation model are considered complete for the current architecture.

## Phase 3 — Input / Events

Completed at source level. Current source includes mouse input, hit-test, hover/click generation, capture, focus, keyboard routing, event propagation and modal-root filtering.

Event handlers are registered through the framework event system; component/client code supplies the custom behavior at the registration boundary.

## Phase 4 — Rendering / Backend

Completed. Current source uses SDL3 rendering, logical presentation, layout final geometry, renderer state isolation and clipping.

The chess client uses SDL logical presentation with `SDL_LOGICAL_PRESENTATION_LETTERBOX` and a 1920×1080 logical UI coordinate space.

## Phase 5 — Component Development

Completed as the focused component-development phase. The phase should not be expanded retroactively to absorb every framework subsystem discovered afterwards.

Its important architectural result is that components do not redefine ownership, layout orchestration, input dispatch, hit-testing, rendering, clipping, scrolling, text editing or resource ownership.

## Phase 6 — Framework Core / Subsystem Development

### Current status

**Validation / stabilization boundary.**

The current core implementation is working in the real chess client. The next step is documentation/source consistency review and targeted validation, not another large architecture rewrite.

### Current subsystem work

#### Modality

`ModalManager` is the framework-level modality service.

Current responsibilities include modal registration/stack handling, backdrop state, modal-root input filtering, Escape handling, pointer/backdrop handling and viewport synchronization.

A separate public Modal component is not currently required.

#### Scrolling

`ScrollManager` owns scroll state and provides:

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

The framework viewport is the SDL logical presentation size when logical presentation is configured. The framework obtains it directly from the renderer rather than requiring a client-side `UIManager::setViewportSize()` source of truth.

If logical presentation is unavailable, the renderer output size is used as fallback.

#### Text

Logical text layout and rendering are now implemented through the current `Typography` / `TextContent` / `TextLayout` / `TextRenderer` direction.

The old `TextPrimitive` / `TextNode` architecture is no longer the active target.

### Remaining framework work before broader feature expansion

```text
Source/documentation consistency review
Targeted validation/stabilization
Text input / editing
Image / resource ownership
Overlay/popup infrastructure only if concretely required
```

Not every candidate must become a subsystem. Each must be justified by an actual framework requirement.

## Validation and Completion

The current completion procedure is:

1. Audit active source and public APIs.
2. Review all living `.md` documents against the resulting source.
3. Run full compilation and tests.
4. Perform targeted runtime validation for layout, events, rendering, modality, scrolling and text.
5. Perform source/include/dead-file cleanup.
6. Manually review `ARCHITECTURE.md` for architectural decisions that should be incorporated.

`ARCHITECTURE.md` remains a separately maintained architectural document and should not be mechanically rewritten by routine implementation work.

`UI_FRAMEWORK_ARCHITECTURE_CONTEXT.md` is likewise a large reference document: read and use it as architectural context, but do not mechanically rewrite it during routine cleanup.

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