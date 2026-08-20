# Phase 4 — Implementation Reconciliation / Scope

## Purpose

This document translates the agreed Phase 4 A–G architecture into a source-level implementation status and a minimal implementation scope for `phase4-rendering`.

It must be read together with:

- `docs/PHASE4_START_CONTEXT.md`
- `ai_bootstrap.md`
- `docs/PHASE4_CHECKPOINT_BCE.md`

Current branch:

```text
phase4-rendering
```

`main` remains unchanged until Phase 4 is complete.

Compilation, tests and runtime validation are intentionally deferred until Phase 6.

---

## A — Geometry → Rendering

**Status: IMPLEMENTED / inherited from existing layout-rendering boundary.**

Current source already produces final arranged geometry through:

```text
position_
    ↓
desiredSize_
    ↓
arrange
    ↓
actualPosition_ / actualSize_
    ↓
rendering
```

`Node` exposes `getActualPosition()` / `getActualSize()`. `rendering_state.hpp` converts those final values into SDL rectangles through `toSDLRect()`.

No new A implementation is required.

Do not change `PositionMode::Absolute` semantics during Phase 4.

---

## B — Render Traversal

**Status: IMPLEMENTED at architecture/source level; no new traversal system required.**

Current source provides:

```text
roots → overlays → top modal
```

for rendering, with forward child order and NodeId snapshots for mutation-safe subtree traversal.

`NodeTree::update()` uses the same `roots → overlays` top-level order. Hit-testing uses reverse top-level order and recursive reverse child traversal through `hitTestSubtree()`.

No `zIndex` / global stacking system is required.

### Existing source inconsistency

`src/core/nodetree.cpp` currently contains an older `NodeTree::hitTest(...)` implementation in addition to the recursive `hitTestSubtree()`-based `NodeTree::hitTest(...)` implementation later in the same file.

This is an existing source-level inconsistency also present on `main`; it is not a new Phase 4 architecture requirement and is not treated as a Phase 1–3 redesign item.

It must, however, be resolved before the project can be considered validated in Phase 6 because the intended canonical traversal is the recursive `hitTestSubtree()` path.

Do not use compilation as the means of discovering this during Phase 4; it is already recorded as a source finding.

---

## C — Renderer State / Clipping

**Status: IMPLEMENTED by current Phase 4 branch.**

Implemented source:

```text
include/ui_framework/core/rendering_state.hpp
```

`RendererStateScope` captures/restores:

- render target;
- viewport;
- viewport-set state;
- clip state/rectangle;
- scale;
- draw color;
- blend mode;
- color scale.

`NodeTree::drawSubtree()` uses one scope per node subtree and applies `Overflow::HIDDEN` as a nested rectangular clip intersection.

Zero-area intersections skip the subtree.

No additional C implementation is required unless the source audit reveals a concrete contradiction with the agreed semantics.

---

## D — Node State / Geometry / Visual Semantics

**Status: CONTRACT-ONLY for Phase 4; no broad property implementation required now.**

The architecture is settled:

```text
Node property
   ├── Layout / geometry
   ├── Hit-test / interaction
   └── Rendering / visual output
```

Canonical semantics are required for framework-defined geometry. `Node::hitTest()` remains an extension point for genuinely custom geometry.

### Current source

Already implemented or inherited:

- visibility;
- enabled/focus/capture interaction state;
- position/size;
- padding;
- border widths;
- overflow;
- final arranged geometry;
- virtual `Node::hitTest()` default rectangle behavior.

### Contract-only future properties

Not required to be added in this implementation step merely to close D:

- opacity;
- border radius API;
- transforms;
- scroll behavior.

Their semantics have been established in `PHASE4_CHECKPOINT_BCE.md` so later implementation cannot accidentally redefine their relationship with layout, hit-test or rendering.

### Important future implementation rule

If `borderRadius` or transforms are later added as framework-defined properties, their canonical geometry must be consumed centrally by rendering, hit-testing and clipping. Component authors must not duplicate framework geometry math for standard shapes.

---

## E — Render Backend

**Status: IMPLEMENTED / ARCHITECTURE SETTLED.**

Current framework is deliberately SDL3-only.

The source directly includes SDL3 types and uses:

```text
SDL_Renderer*
SDL3_ttf
SDL drawing APIs
```

The current dependency model already gives the framework access to SDL3 headers/libraries. The application owns SDL runtime lifecycle.

No backend abstraction is required.

No code changes are required for E unless a concrete source inconsistency is later found.

---

## F — Resource Boundary

**Status: IMPLEMENTED for current concrete resources; generic resource subsystem remains contract-only/deferred.**

Current concrete resource contract:

```text
Borrowed:
    SDL_Renderer*
    TTF_Font*

Framework-owned / renderer-bound:
    TTF_TextEngine*
    TTF_Text*
```

`TextNode` already releases renderer-bound text objects and recreates them if the renderer changes.

No generic resource manager is required.

No additional code is required for F at current scope.

Future SDL_Texture/offscreen resource pipelines remain deferred until a concrete requirement exists.

The canonical distinction is:

```text
semantic resource
    ↓
backend-bound representation
    ↓
SDL3
```

---

## G — Animation Boundary

**Status: CONTRACT-ONLY; no animation implementation in Phase 4.**

Animation is defined as a state-transition mechanism:

```text
Animation
    ↓
changes state
    ↓
existing framework property semantics
```

If the property is layout-affecting, normal layout processing applies. If it is visual-only, layout does not become involved. Animation does not directly perform SDL rendering, clipping or hit-testing.

No `AnimationManager`, timeline system, easing API or mandatory animation container in `Node` is required now.

A future animation subsystem is permitted if concrete requirements justify it.

No G implementation is required in the current Phase 4 source.

---

## Phase 4 implementation summary

| Item | Status | Code work required now? |
|---|---|---|
| A — Geometry → Rendering | Implemented/inherited | No |
| B — Render Traversal | Implemented/inherited | No new architecture; existing duplicate hitTest needs eventual cleanup |
| C — Renderer State / Clipping | Implemented | No |
| D — Node State / Geometry Semantics | Contract-only | No broad property additions |
| E — SDL3 Render Backend | Implemented/settled | No |
| F — Resource Boundary | Implemented for current resources | No |
| G — Animation Boundary | Contract-only | No animation subsystem |

### Current actual Phase 4 implementation delta

The substantive Phase 4 rendering implementation already present on the branch is:

```text
RendererStateScope
    ↓
per-node renderer state isolation
    ↓
nested clipping via existing drawSubtree traversal
```

The branch-vs-`main` source delta also contains the supporting `NodeTree` include/source changes required to use that state scope.

### Minimal remaining implementation scope

Before considering Phase 4 source complete, the likely remaining code-level task is limited to **reconciling the existing `NodeTree::hitTest()` duplicate/inconsistency** if the project policy requires a clean source tree before Phase 5.

No new resource manager, animation manager, z-order system, RenderContext, second backend, transform system, scroll system, or broad visual-property API should be introduced as part of this Phase 4 implementation pass.

### Validation gate

Do not run:

- compilation;
- automated tests;
- runtime validation;
- full-build validation

until Phase 6.

A Phase 4 final branch-vs-`main` source audit is still required before Phase 4 completion/merge.
