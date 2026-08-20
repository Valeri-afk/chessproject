# Phase 1 — Final Architecture Decisions

> **Status:** completed and reconciled with current Phase 2 source
> **Date:** 2026-08-19

This document records the Phase 1 runtime decisions that remain relevant to the current framework architecture. The active `include/` and `src/` trees are authoritative. Phase 1 is complete; Phase 2 has subsequently moved layout ownership into the framework-owned layout subsystem.

## 1. Framework boundary

The project is an embedded retained-mode UI framework/runtime, not an application framework.

The application owns application lifetime, the main/game loop, SDL runtime resources and event polling, and application/domain systems.

The UI framework owns UI runtime state, the node tree, live-node ownership and lifetime, lifecycle callbacks, input/event routing, layout coordination, modal coordination, mutation scheduling, traversal/rendering order, and the internal UI-frame protocol.

## 2. Application loop and `runFrame()`

The framework does not own the application's main loop. `UIManager::runFrame()` is the public UI-frame boundary and controls the internal update/layout/render ordering.

## 3. SDL relationship

The framework is SDL-backed. The application owns SDL initialization, window/renderer lifetime, and event polling; the framework consumes SDL events and the supplied renderer.

## 4. Node ownership

The selected public ownership model is:

```cpp
add(std::unique_ptr<Node>)
remove(Node&)
```

Once a node is live, the framework owns it. Public ownership-transfer `detach()` is not part of the current API.

Client-held `Node*` values are non-owning. `NodeId` is an identity/liveness mechanism and does not extend lifetime.

## 5. Deferred removal and mutation

Structural mutation during a guarded runtime scope is deferred. Removal resolves the target through `NodeId`, unmounts the subtree, unregisters it, clears runtime ownership metadata, and allows framework-owned destruction after the safe mutation boundary.

Self-removal is valid because the current callback completes before the queued destruction occurs.

## 6. Lifecycle

The runtime lifecycle is:

```text
construct
  → attach/register
  → onMount (pre-order)
  → live
  → remove
  → onUnmount (post-order)
  → unregister / clear ownership
  → destruction
```

## 7. Traversal

Traversal uses mutation guards together with snapshot/live-node resolution where callback mutation requires it. Mutations do not retroactively rewrite the current traversal, and stale node IDs are skipped safely.

## 8. Phase 1 boundaries retained by the current architecture

The following remain intentionally outside the completed Phase 1 scope:

- complete recursive hit-testing stabilization;
- complete event-dispatch semantics;
- component-model design;
- modal/navigation redesign;
- rendering backend abstraction;
- public reparenting.

Phase 2 subsequently introduced framework-owned layout orchestration; therefore layout is no longer described here as an unresolved Phase 1 design problem.

## 9. Current ownership boundary after Phase 2

```text
NodeTree
  → ownership, lifetime, traversal, mutation, scheduling

Node / PanelNode
  → runtime state, hierarchy and layout properties

LayoutManager
  → measurement, constraints, container layout and arrangement

UIManager
  → public facade and subsystem orchestration
```

The legacy `measure()` / `arrange()` lifecycle is not part of the current Node/PanelNode client contract.

## 10. Completion state

**Phase 1 is complete at source/architecture level.**

Build, compilation and runtime validation remain intentionally deferred to the project's Phase 6 validation stage.

The historical `phase1-worktree/` snapshot is obsolete and is not part of the current source architecture.
