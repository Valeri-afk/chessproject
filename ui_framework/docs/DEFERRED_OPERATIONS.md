# Deferred Operations and Phase Safety

## Purpose

Framework runtime phases must remain traversal-safe. Operations that would mutate the structure or invalidate the current traversal are deferred rather than executed re-entrantly.

## Framework-controlled phases

The retained runtime owns the ordering of major work. The current frame conceptually includes:

```text
state synchronization
    ↓
update / preparation
    ↓
process queued layout
    ↓
scroll/modal synchronization
    ↓
node update
    ↓
draw traversal
```

Exact internal ordering belongs to `UIManager`/private systems and is not a client-side scheduling API.

## Structural operations

Child ownership changes during Measure, Arrange or Draw are deferred through NodeTree mutation handling.

The current traversal sees a stable tree. Later phases observe the resulting tree after the mutation becomes safe to commit.

## Layout operations

`invalidateLayout()` schedules work; it never executes layout immediately.

When a layout pass is active, a new invalidation is deferred to a subsequent framework-controlled pass.

```text
consume current layout roots
        ↓
run Measure / Arrange
        ↓
new invalidation
        ↓
queue root for later pass
```

This prevents recursive layout execution and unstable traversal.

## Why explicit deferral exists

Without deferral, a component could mutate the tree or restart layout while the framework is iterating the same structure. That would make ownership, iteration, geometry and event routing difficult to reason about.

The framework therefore guarantees runtime integrity and safe phase boundaries rather than immediate execution of every requested consequence.

## Client responsibility

Client/component code should not attempt to flush or manually execute framework phases.

There is intentionally no public API for:

```text
flushMutationQueue()
runLayoutNow()
runInputPhaseNow()
flushFramework()
```

A client reports semantic changes; the framework decides when and how to process them.

## Batching

Deferred processing naturally allows coalescing:

```text
local mutations
    ↓
semantic notification / invalidation
    ↓
framework queue
    ↓
coalesced processing at safe phase boundary
```

The developer may decide when a sequence of local semantic changes is complete enough to notify the framework. The framework owns subsequent scheduling.

## Re-entrancy rule

Framework callbacks may change state and request future work, but must not assume that the request is executed synchronously.

A component should therefore treat framework-derived geometry and other cached derived state as potentially stale until the framework completes the relevant phase.

## General principle

> Components describe state and semantic changes; the framework controls execution timing, ordering, batching and phase safety.
