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

### 2.21 TextPrimitive ownership and responsibility

`TextPrimitive` is **not** a layout owner and must not be stored in base `Node`.

Its current intended role is a low-level text utility:

```text
TextPrimitive::measure()
    → intrinsic text measurement / wrapping measurement

TextPrimitive::draw()
    → low-level text rendering
```

`TextNode` owns its own `TextPrimitive` and uses it from `measureContent()` and `draw()`.

The framework therefore owns the outer Measure/Arrange process, while `TextNode` owns its component-specific text state and delegates the low-level text operation to `TextPrimitive`.

`TextPrimitive` itself remains subject to a later focused review for wrapping, logical presentation, raster scaling, caching, and whether its API should eventually be split into narrower measurement/rendering responsibilities. That review is not allowed to reintroduce layout ownership into the primitive.

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
TextPrimitive ownership boundary      DONE / settled
```

### Implementation / migration

```text
Node public Measure/Arrange hooks
    PRESENT

Node → TextPrimitive coupling removed
    DONE

TextNode → own TextPrimitive
    DONE

Old deferLayoutMutation path removal
    IN PROGRESS / audit remaining uses

Migration of all real components
    NOT DONE

Framework-known property invalidation audit
    NOT DONE

TextPrimitive semantic/layout review
    NOT DONE

Full framework build/runtime validation
    NOT DONE

Real chess client validation
    NOT DONE
```

The project is now beyond the initial architecture-definition stage and is in the **active migration/cleanup stage**. The base layout contract is substantially settled; the remaining risk is implementation completeness and preserving the verified historical semantics while migrating real components.

## 5. Remaining implementation work

### Step 1 — Finish old-layout mechanism audit

Search the current branch for all remaining:

```text
deferLayoutMutation
property-driven deferred layout callbacks
implicit property → layout scheduling
old layout hook names
base-Node text/layout helpers that no longer belong there
```

Each remaining use must either be removed or explicitly justified as unrelated to the new layout contract.

### Step 2 — Finish public Measure/Arrange implementation

Confirm that the production layout traversal uses only the intended hooks:

```text
Node::measure(const MeasureContext&)
Node::arrange(const ArrangeContext&)
Node::draw(...)
```

and that no experimental duplicate layout path remains.

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

For each component verify its custom state remains local and that its layout consequences are expressed through Measure/Arrange plus explicit invalidation.

### Step 4 — Complete imperative invalidation migration

Audit framework-known layout properties and custom component setters.

Required rule:

```text
state mutation
    ↓
explicit invalidateLayout()
```

Do not restore hidden setter-driven `deferLayoutMutation()` scheduling merely to reduce client call sites.

### Step 5 — Finish TextPrimitive review

Do not redesign it speculatively. Validate first:

```text
measure(font, text, availableWidth)
    → wrapping semantics

TextNode Measure
    → content constraint semantics

TextNode Arrange
    → actual content box / alignment

TextPrimitive draw
    → actual allocated geometry
    → logical presentation / integer scaling
    → clipping compatibility
```

Then decide whether `TextPrimitive` needs an API cleanup. The default is to keep it as a rendering/measurement utility.

### Step 6 — Custom layout acceptance coverage

Maintain a focused custom-panel test that combines:

```text
custom layout property
custom Measure/Arrange
min/max
fixed size
Auto
stretch
padding
border
text wrapping
invalidateLayout()
```

The test must include real constraint conflicts, not only compatible values.

### Step 7 — Runtime/build validation

After migration:

```text
build framework
build chess client
run existing tests
run custom layout acceptance tests
verify resize
verify text wrapping
verify clipping
verify overlays/modals
verify structural mutation
verify repeated invalidation
```

Historical Phase 2 numerical cases remain semantic acceptance references.

### Step 8 — Final cleanup

Only after runtime validation:

```text
remove obsolete helpers
remove obsolete documentation about the old property/layout model
remove duplicate compatibility paths
update public documentation
```

Keep historical reference material in the old repository.

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

10. TextPrimitive is a lower-level utility and is not a base-Node layout owner.

11. The real chess client builds and runs correctly on the new model.

12. Historical numerical acceptance cases remain semantically satisfied.
```

## 7. Working mode from this checkpoint onward

The architecture-definition phase is considered largely complete.

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

Current priority order:

```text
1. remaining old-layout/deferred-mutation cleanup
2. real component migration
3. explicit invalidation audit
4. TextPrimitive semantic review
5. build + runtime + chess-client validation
6. final cleanup
```

The next work should therefore prioritize **implementation and migration**, not further speculative redesign.
