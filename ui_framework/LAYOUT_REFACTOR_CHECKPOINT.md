# Measure / Arrange Refactor Checkpoint

> **Status:** active implementation checkpoint
> **Branch:** `fix/sharp-logical-text`
> **Purpose:** freeze the architectural decisions reached during the Measure/Arrange and imperative invalidation investigation, distinguish settled semantics from implementation work, and define the completion criteria.

## 1. Goal

The refactor is intended to make component-specific layout behavior open and imperative without making the framework's runtime model open-ended.

The target is:

```text
Framework owns
    tree ownership and lifecycle
    traversal
    framework-known layout semantics
    measurement constraints
    layout scheduling
    geometry commit
    rendering traversal
    input/runtime machinery

Component owns
    component-specific state
    custom Measure policy
    custom Arrange policy
    custom Draw policy
    explicit semantic notifications
```

The concrete motivation is to stop requiring the framework to know and centrally register every component-specific property merely because that property can affect layout or paint.

## 2. Decisions already settled

### 2.1 Measure is universal

`Node` has Measure semantics regardless of whether it owns children.

```text
Leaf Node
    → measure own content

PanelNode
    → may recursively measure children
    → aggregates child desired sizes
    → returns own desired content size
```

`measureContent()` remains a useful default primitive for ordinary leaf components.

### 2.2 Arrange is universal

`Node` has Arrange semantics regardless of whether it owns children.

```text
Leaf Node
    → use assigned content geometry

PanelNode
    → choose child allocations
    → ask framework to arrange children
```

Only structural components need child-arrangement capability.

### 2.3 PanelNode is a structural capability

Plain `Node` does not acquire framework-visible children at this stage.

```text
Leaf : Node
Container : PanelNode
```

`PanelNode` primarily means framework-managed child ownership/structure. It is not required to impose one particular layout algorithm.

`StackPanelNode` is a specialized `PanelNode` with a predefined linear layout policy.

### 2.4 Custom layout policy belongs to the component

The framework owns execution and constraints; the component owns the policy.

```text
Custom Panel
    measure()
        → decide how children contribute to desired size

    arrange()
        → decide child allocations
```

The component does not receive direct access to:

```text
NodeTree internals
layout queue internals
mutation queue internals
phase flushing
raw geometry storage
```

### 2.5 Framework-known properties remain framework-owned semantics

The framework continues to interpret properties such as:

```text
size
min/max size
padding
border
position
position mode
overflow
visibility and input state
```

This does not require every framework-known property to be physically stored outside component classes; semantic ownership is what matters.

### 2.6 Component-specific properties remain component-owned

Examples include:

```text
text
font
icon
text/icon spacing
variant
custom sizing modes
colors
selection state
custom layout state
```

Their effects are expressed through the component phase contracts.

### 2.7 Explicit invalidation is the change notification model

The public layout notification is:

```cpp
uiManager.invalidateLayout(node);
```

Property setters do not need to silently schedule layout. Component/client code changes state and explicitly reports the consequence.

This applies to framework-known layout properties as well as custom layout-affecting state for the current design direction.

Exceptions already chosen:

```text
enabled
focusable
capturable
```

do not require layout invalidation merely because their values changed.

### 2.8 `invalidateLayout()` is root-based and coalesced

The existing `NodeTree` mechanism is already the intended implementation:

```text
changed node
    ↓
walk to top-level root / overlay
    ↓
queue root once
```

`layoutQueueSet_` deduplicates repeated invalidations.

Detached/non-live nodes do not become layout jobs.

`UIManager` is only the public facade; `NodeTree` remains the authoritative source of tree membership and queue semantics.

### 2.9 Invalidation is not synchronous

`invalidateLayout()` schedules future work. It does not run Measure/Arrange immediately and does not expose a flush operation.

### 2.10 Re-invalidation during layout is deferred

If `measure()` or `arrange()` causes an invalidation, the current pass is not recursively restarted.

The current queued roots are consumed first; a new invalidation queues the root again for a later framework-controlled pass.

A component that invalidates itself on every Measure/Arrange invocation can therefore cause repeated future passes. This is component behavior, not recursive scheduler execution.

### 2.11 Structural child mutation remains framework-managed

`PanelNode::addChild/removeChild` already use `NodeTree` mutation handling and already trigger the appropriate layout invalidation through the parent/root.

A separate public `treeStructureChanged()` notification is intentionally not part of this stage.

### 2.12 Structural mutations during framework phases are deferred

Current mutation scopes preserve traversal stability.

A structural mutation during Measure/Arrange/Draw is not made visible in the middle of the current traversal. The stable post-mutation tree is observed by later work.

### 2.13 Measure proposal and final allocation are different concepts

This is one of the central contracts:

```text
Measure proposal
    ≠
Arrange allocation
```

A Measure constraint is an upper bound / available bound. A component may report a desired size larger than it.

Arrange receives a size allocation selected by the parent layout policy.

### 2.14 `Auto` is not `fill parent`

Historical Phase 2 documentation and the real linear layout implementation agree:

```text
Auto
    → intrinsic measurement / parent allocation
```

The raw `Auto/Value` representation is not part of generic `MeasureContext`.

`Auto` is interpreted by the surrounding framework/layout policy.

Top-level root sizing may have its own viewport semantics; child `Auto` is parent-layout-dependent.

### 2.15 Constraint ownership is fixed

The framework owns framework-known constraint resolution.

Canonical current semantics:

```text
Fixed size
    → measurement proposal + final size

Max size
    → measurement proposal + final size

Min size
    → final size only

Auto
    → intrinsic measurement / parent allocation
```

In particular, `min` does not automatically become a Measure proposal.

`max` may narrow Measure before width-sensitive content is measured.

### 2.16 Border-box model is retained

The node's outer geometry is a border box.

Measure/Arrange hooks work with content-space geometry while the framework converts between outer and content boxes using padding and border.

Conceptually:

```text
outer proposal
    ↓
subtract padding + border
    ↓
component Measure
    ↓
desired content size
    ↓
add padding + border
    ↓
desired outer size
```

### 2.17 Final constraints happen after parent allocation

Custom layout may choose stretch, centering, spacing, or another allocation policy.

The framework then applies framework-owned final constraints:

```text
parent allocation
    ↓
fixed/min/max resolution
    ↓
actual child size
```

A custom component should not duplicate `resolveFinalSize()` semantics.

Historical stretch behavior confirms this ordering.

### 2.18 `Overflow` is render traversal state

`Overflow::HIDDEN` does not participate in Measure/Arrange.

The current `NodeTree::drawSubtree()` applies nested clipping through renderer-state RAII:

```text
previous clip
    ∩
node rect
    ↓
draw node and subtree
    ↓
restore previous renderer state
```

This is effectively stack semantics.

`PanelNode` does not own child render traversal. `NodeTree` owns clipping, traversal ordering, mutation safety, root/overlay ordering, and the recursive draw traversal.

### 2.19 No separate `invalidatePaint()` at this stage

Rendering currently runs every frame. There is no demonstrated need for a separate paint-dirty queue yet.

A separate paint invalidation API is deferred until the render pipeline actually requires one.

### 2.20 Derived geometry is cached state, not live computation

`getDesiredSize()` and `getActualSize()` expose the latest committed framework geometry.

After invalidation and before the next layout pass, the previous value remains readable but may be stale.

No explicit geometry-validity flag is required at this stage.

## 3. Historical evidence used to settle the model

The old `Valeri-afk/ui-framework` repository remains the primary historical reference for previous Measure/Arrange behavior.

The most relevant evidence includes:

```text
docs/PHASE2_CONSTRAINT_SEMANTICS.md
docs/PHASE2_NUMERICAL_LAYOUT_CASES.md
src/core/linear_layout.cpp
```

These establish the minimal constraint model, numeric acceptance cases, Auto semantics, min/max ordering, stretch behavior, padding/border composition, absolute children, and text wrapping constraints.

The current `fix/sharp-logical-text` branch is the primary reference for the present architecture and runtime integration.

## 4. Current implementation status

### Architecture / semantics

```text
Imperative invalidation                DONE / settled
Measure on Node                       DONE / settled
Measure on PanelNode                  DONE / settled
Arrange on Node                       DONE / settled
Arrange on PanelNode                  DONE / settled
PanelNode as structural capability    DONE / settled
Auto/min/max semantics                DONE / validated
Border-box semantics                  DONE / validated
Overflow clipping model               DONE / validated
Root-based layout queue               DONE / validated
Re-invalidation semantics             DONE / validated
```

### Implementation / migration

```text
Final public Measure/Arrange API
    IN PROGRESS

Migration of all real components
    NOT DONE

Removal of old deferLayoutMutation paths
    NOT DONE

Full framework build/runtime validation
    NOT DONE

Real chess client validation
    NOT DONE
```

The project is therefore **past the architecture-definition stage and entering the main implementation/migration stage**.

## 5. Remaining implementation work

### Step 1 — Finalize the public Measure/Arrange API

Make the intended API concrete and stable:

```text
Node::measure(const MeasureContext&)
Node::arrange(const ArrangeContext&)
Node::draw(...)

MeasureContext
    availableContentSize
    measureChild(...)

ArrangeContext
    contentPosition
    contentSize
    desiredSize(...)
    arrangeChild(...)
```

Preserve the semantic contracts above while removing experimental or duplicated paths.

### Step 2 — Make the framework layout engine use the final hooks

Ensure the production layout traversal:

```text
root
    ↓
Measure subtree
    ↓
resolve desired outer sizes
    ↓
Arrange subtree
    ↓
commit actual geometry
```

uses the final custom hooks without exposing framework internals.

### Step 3 — Migrate real components

Audit and migrate at least:

```text
TextNode
Button
MenuItem
StackPanelNode
Dropdown
Checkbox
other real layout-affecting components
```

Validate leaf components through `measureContent()` where appropriate and containers through custom Measure/Arrange.

### Step 4 — Remove old hidden layout mutation mechanisms

Find every remaining use of:

```text
deferLayoutMutation
property-driven deferred layout callbacks
implicit layout scheduling that contradicts the new public contract
```

Replace them with:

```text
state mutation
    ↓
explicit invalidateLayout()
```

where appropriate.

### Step 5 — Ensure framework-known property setters are consistent

Framework-known layout properties may mutate state directly; the current contract expects explicit invalidation to report the semantic change.

Do not reintroduce hidden per-setter scheduling merely to avoid explicit client notifications.

### Step 6 — Validate custom component behavior

Keep a focused custom-panel test that combines:

```text
custom layout property
custom Measure/Arrange
min/max
fixed size
padding
border
invalidateLayout()
```

The test must exercise real constraint conflicts, not only non-conflicting values.

### Step 7 — Runtime/build validation

After migration:

```text
build framework
build chess client
run existing tests
run custom layout acceptance tests
verify layout under resize
verify text wrapping
verify clipping
verify overlays/modals
verify structural mutation behavior
```

The historical Phase 2 documents explicitly leave runtime/build validation for the final validation stage; it must now become part of the implementation phase.

### Step 8 — Final cleanup

Remove obsolete documentation and code paths only after the new path passes real validation.

Do not delete historical reference material from the old repository; that repository remains useful as a compatibility/reference source.

## 6. Definition of done

The refactor is complete when all of the following are true:

```text
1. A leaf Node can define custom Measure/Arrange without PanelNode.

2. A PanelNode can define a custom child layout policy without framework
   knowledge of its custom properties.

3. Framework-known size/min/max/padding/border semantics remain centralized
   in framework constraint resolution.

4. Custom components do not need to duplicate framework constraint math.

5. Layout-affecting state changes use the explicit invalidateLayout contract.

6. Old deferLayoutMutation-based component layout scheduling is removed.

7. Structural mutation remains owned by PanelNode/NodeTree and does not need
   a second notification path.

8. Overflow clipping remains framework-owned and stack-scoped during Draw.

9. Re-invalidation during Measure/Arrange remains deferred rather than
   recursively re-entering layout.

10. The real chess client builds and runs correctly on the new model.

11. Historical numerical acceptance cases remain semantically satisfied.
```

## 7. Working mode from this checkpoint onward

From this checkpoint onward the investigation phase is considered largely complete.

New questions should be investigated only when they block implementation or reveal a contradiction with the settled historical semantics.

Default workflow:

```text
inspect current code
    ↓
compare against settled contract / historical reference
    ↓
make the smallest implementation change
    ↓
validate
    ↓
update this checkpoint if the architectural contract changes
```

The next work should therefore prioritize **implementation and migration**, not further speculative redesign.
