# Phase 4 — Rendering / Backend Start Context

## State

- `main` contains completed source-level Phases 1–3.
- Working branch: `phase4-rendering`.
- Do not modify `main` until Phase 4 is complete.
- Do not run compilation, tests or runtime validation before Phase 6.

## Phase order

```text
1 Runtime
2 Layout
3 Input / Events          [source-level complete]
4 Rendering / Backend     [current]
5 Component Model
6 Modal / Navigation / full validation
```

## Phase 4 goal

Stabilize the SDL-first rendering contract before building the new Component Model.

Current responsibility split:

```text
UIManager
    → frame orchestration

NodeTree
    → render traversal, ordering, clipping, mutation safety

Node
    → node-specific visual rendering

SDL3
    → actual backend
```

Existing `NodeTree::draw()` / `drawSubtree()` already provide root/overlay/top-modal ordering, clipping, renderer-state isolation and deferred-mutation safety. Do not rewrite Phase 1–3 behavior without a concrete need.

`Node::draw(SDL_Renderer*)` may remain the rendering hook. Do not introduce `RenderContext` or a generic backend abstraction merely for symmetry; a second backend is not currently required.

`primitives` is a low-level SDL drawing utility layer, not the framework's rendering architecture.

Do not introduce a `ResourceManager`, `TextureManager`, `FontManager` or similar abstraction without a concrete requirement.

## Legacy warning

`src/components/*` and `include/ui_framework/components/*` contain the old Component/Widget/Button/Label/Modal model, including obsolete public `measure()/arrange()` contracts. This legacy layer is not the basis for Phase 5 and must not be used to redefine completed Phase 2 semantics.

The existing `ModalManager` is infrastructure. A future Phase 5 Modal component may consume it; Phase 6 owns higher-level modal/navigation semantics.

## Required process

1. Read `ROADMAP.md`, `ARCHITECTURE.md`, `INSTRUCTIONS.md`, `FRAMEWORK_SCOPE.md`.
2. Audit current rendering/resource source before changing code.
3. Fix or extend only the rendering contract required by the current architecture.
4. Reconcile source and docs after substantial changes.
5. Complete a final branch-vs-`main` audit before merging.
