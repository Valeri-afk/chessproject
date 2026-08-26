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

Exact internal ordering belongs to `UIManager` and private systems; it is not a client-side scheduling API.

Scroll and modal synchronization are framework work. Client code requests semantic state changes through `UIManager` rather than manually invoking service phases.

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

Scroll geometry follows committed layout. A changed viewport or content extent is reconciled during scroll synchronization rather than forcing synchronous layout from the scroll API.

## Modal and scroll operations

Opening/closing modality and enabling/disabling scrolling are semantic service operations. They must remain compatible with NodeTree lifetime and traversal safety.

Examples:

```text
showModal(node)
closeModal()
enableScrolling(panel)
setScrollOffset(panel, offset)
```

These operations do not expose internal service queues or require the client to flush the framework.

When a modal opens, modality establishes a new input boundary and cancels incompatible pointer capture. Focus initialization and modal filtering are framework-managed consequences of the operation.

When scrolling changes effective coordinates, the framework refreshes hover after the scroll state has been applied. It does not synthesize a mouse-move event or implicitly reset pointer capture.

If a modal or scroll node is structurally removed, the corresponding service state is cleaned/inactivated through framework lifecycle synchronization rather than by requiring the client to manipulate internal registries.

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
flushScrollSystem()
flushModalSystem()
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
