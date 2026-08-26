# Deferred Operations and Phase Safety

## Purpose

Framework runtime phases must remain traversal-safe. Structural operations that would mutate the hierarchy while it is being traversed are deferred through `NodeTree` mutation handling.

## Framework-controlled frame

`UIManager::runFrame()` currently performs the major work in this order:

```text
synchronize input state
    ↓
flush pending tree mutations
    ↓
process layout queue
    ↓
synchronize scrolling
    ↓
synchronize/update modality
    ↓
NodeTree update traversal
    ↓
draw traversal
```

If the renderer's logical presentation size changes, a full layout is requested before the layout queue is processed.

Exact private helper behavior remains an implementation detail; clients do not schedule these phases.

## Structural operations

`PanelNode::addChild/removeChild` and root/overlay attachment/removal are routed through `NodeTree`. When a mutation scope is active, the structural change is queued instead of being applied re-entrantly.

The queue uses snapshot-swap draining. Mutations generated while a batch is executing are placed into a subsequent batch and drained by the same flush operation after the current mutation completes.

## Layout operations

`invalidateLayout()` schedules layout; it does not execute Measure/Arrange immediately.

NodeTree promotes invalidation to the containing root/overlay and deduplicates queued roots. LayoutSystem consumes the queued roots during its layout phase.

A layout mutation that queues more work does not recursively restart the current traversal.

## Event operations

Input dispatch establishes a `NodeTree::ScopedMutationGuard` around event propagation. Handlers may request structural changes, but those changes are deferred until the guarded dispatch completes.

`Node::on<Event>()` registration and handler removal use a snapshot of matching callbacks for the current delivery. Changes to the live handler table therefore do not invalidate the current callback iteration.

## Modal and scroll operations

Modal and scroll state are framework services coordinated through `UIManager`. Their APIs do not expose internal queues or require the client to flush framework work.

Examples:

```cpp
showModal(node)
closeModal()
enableScrolling(panel)
setScrollOffset(panel, offset)
```

Scroll synchronization derives content extent from committed layout geometry. Modal synchronization validates live modal sessions and focus boundaries.

## Client responsibility

Client/component code should not attempt to flush or manually execute framework phases. `NodeTree::flushMutationQueue()` exists internally for runtime coordination and is not part of the public `UIManager` API.

There is intentionally no public API for:

```text
runLayoutNow()
runInputPhaseNow()
flushFramework()
flushScrollSystem()
flushModalSystem()
```

## Re-entrancy rule

Framework callbacks may change state and request future work. They must not assume that a structural mutation or layout consequence is committed synchronously when invoked from a guarded phase.

## General principle

> Components describe state and semantic changes; the framework controls execution timing, ordering, batching and phase safety.
