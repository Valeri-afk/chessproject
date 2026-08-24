# Current Refactor Direction

> **Status:** current working direction / not an ADR  
> **Branch:** `fix/sharp-logical-text`  
> **Purpose:** record the architecture currently chosen after the investigation of imperative/declarative UI models, `TextNode`, framework properties, invalidation, custom layout, structural composition, batching, and the existing layout queue.

---

# 1. Current architectural direction

The framework remains a **retained-mode UI framework with framework-owned runtime execution and deliberately opened imperative extension points**.

The objective is not to turn the framework into an unconstrained imperative toolkit. The objective is to remove unnecessary closure where component-specific behavior cannot be expressed naturally.

The current target is:

```text
Framework owns:
    runtime lifecycle
    tree ownership/invariants
    layout execution
    constraints
    layout scheduling
    invalidation consequences
    geometry commit
    framework-known semantics

Developer owns:
    component-specific state
    custom Measure/Arrange behavior
    custom Draw behavior
    explicit notifications about semantic changes
```

---

# 2. Layout model

The selected layout model is **two-pass Measure → Arrange**.

```text
Measure
    ↓
desired size
    ↓
Arrange
    ↓
actual geometry
```

Measure is universal for both leaf `Node` and `PanelNode`.

```text
Node:
    Measure derives own desired content size

PanelNode:
    Measure may recursively measure children
    aggregate their desired sizes
    incorporate its own component state
    return its desired content size
```

Arrange is also a universal phase, but only structural components need child-arrangement capability.

```text
Node:
    use final content geometry for its own behavior/drawing

PanelNode:
    use final content geometry
    decide child allocations
    arrange framework-visible children
```

The framework remains responsible for when Measure/Arrange execute, traversal, scheduling, and geometry commit.

---

# 3. Existing border-box semantics are retained

The historical full Measure/Arrange implementation in `Valeri-afk/ui-framework` and the current `fix/sharp-logical-text` implementation both establish the same important semantics:

```text
outer Node geometry = border box
padding + border = framework-owned layout semantics
component Measure = content-space measurement
component Arrange = content-space arrangement
framework converts between content box and border box
```

The refactor must preserve these semantics rather than inventing a second layout math model.

Conceptually:

```text
parent border-box proposal
    ↓
framework resolves Node size/min/max/padding/border semantics
    ↓
effective content proposal
    ↓
component Measure
    ↓
desired content size
    ↓
framework adds padding/border
    ↓
desired border-box size
```

Arrange follows the corresponding border-box → content-box transformation before calling component layout behavior.

---

# 4. Framework-known properties remain framework-known

The new model does **not** attempt to make every property custom.

The framework must continue to understand properties that its own subsystems directly interpret.

Typical examples:

```text
visible
enabled
focusable
capturable
overflow
position mode
size
min/max size
padding
border
other framework-defined layout/input/painting state
```

Principle:

```text
Framework-known property
    = framework itself needs the semantic value

Component-owned property
    = component can express its consequences through a framework behavior contract
```

The physical storage location is secondary to the semantic ownership.

---

# 5. Component-owned custom properties

Component-specific state should remain inside the component whenever the framework does not need to interpret the property directly.

Examples:

```text
text
font
font size
icon
text/icon spacing
custom sizing modes
variant
colors
selection state
component-specific layout state
```

A custom property may affect one or more framework phases without becoming a framework-known property.

Example:

```text
text
    → Measure
    → Draw

textColor
    → Draw

custom internal spacing
    → Measure / Arrange / Draw
```

This removes the need for a universal property-registration/property-metadata system at the current stage.

---

# 6. Explicit invalidation becomes a public contract

The developer should explicitly report semantic changes that require framework work.

The current public API is:

```cpp
UIManager::invalidateLayout(node)
```

Additional notification APIs are deliberately deferred until the corresponding runtime pipeline has been audited and shown to require them.

`invalidateLayout()` means:

```text
"this node's layout-derived state is no longer necessarily reflected in
framework-computed layout state"
```

It does **not** mean:

```text
run layout immediately
flush the framework
manually mutate NodeTree
```

The framework owns those consequences.

`UIManager` is the public facade. It delegates to `NodeTree`, which already owns live-node validation, root promotion, queue insertion, and queue deduplication.

---

# 7. `invalidateLayout()` contract

The public call is:

```cpp
uiManager.invalidateLayout(node);
```

## 7.1 Live-node validation

The request is accepted only for a node that is currently owned/live in this `UIManager` runtime.

The public facade does not duplicate tree membership checks. `NodeTree::insertLayoutQueue()` performs the authoritative live-node check.

```text
invalidateLayout(node)
    ↓
NodeTree::insertLayoutQueue(node)
    ↓
node must be live
```

A detached node therefore produces no queued layout work.

## 7.2 Root promotion

The requested node is not queued as an independent layout job.

```text
changed node
    ↓
walk parent chain
    ↓
top-level root / overlay
    ↓
queue root
```

This ensures a layout pass always sees the full affected subtree with the correct parent constraints.

## 7.3 Queue deduplication

The same root is queued at most once until the layout queue is consumed.

```text
invalidateLayout(A)
invalidateLayout(A)
invalidateLayout(child-of-A)
    ↓
one queued root
```

`layoutQueueSet_` remains the deduplication authority.

## 7.4 Timing

`invalidateLayout()` never executes layout synchronously. It only schedules the root for the next framework-controlled layout phase.

The current `UIManager::runFrame()` ordering remains:

```text
sync state
    ↓
update
    ↓
process layout queue
    ↓
scroll/modal synchronization
    ↓
node update
    ↓
draw
```

## 7.5 Mutation scopes

Calling `invalidateLayout()` while a tree mutation scope is active does not flush or bypass the scope. The notification only contributes a root to the existing layout queue.

Structural mutations remain deferred by `NodeTree`; the next Measure/Arrange observes the stable post-mutation tree.

## 7.6 Re-invalidation during an active Measure/Arrange pass

If a component calls `invalidateLayout()` from inside its current `measure()` or `arrange()`, the current pass is **not restarted recursively**.

The layout queue is consumed into a temporary local queue before the root callback begins. Therefore a new invalidation during the callback inserts the root into the now-empty framework queue again.

Conceptually:

```text
layout queue
    ↓
consume current roots
    ↓
Measure / Arrange(root)
    ↓
component invalidates root
    ↓
root is queued again
    ↓
finish current pass
    ↓
next framework layout phase processes it
```

This preserves traversal stability and avoids re-entrant layout execution.

A component that repeatedly invalidates itself from every Measure/Arrange invocation can therefore cause repeated layout passes across frames. That is component logic, not framework scheduling failure.

## 7.7 Root and overlay nodes

Calling `invalidateLayout()` on an ordinary root or overlay is valid and queues that node itself.

Calling it on a descendant of either is promoted to that root/overlay.

## 7.8 Custom properties

The framework does not observe arbitrary component fields.

A component-owned change that affects layout must follow the explicit contract:

```cpp
customState_ = newValue;
uiManager.invalidateLayout(*this);
```

For framework-known layout properties the same public notification contract is preferred: property mutation changes the semantic state; `invalidateLayout()` reports that the computed geometry may now be stale.

## 7.9 Coalescing example

```cpp
button.setText("A");
button.setFont(font);
customPanel.setCustomSpacing(8.0f);

uiManager.invalidateLayout(button);
uiManager.invalidateLayout(button);
uiManager.invalidateLayout(customPanel);
```

When these nodes belong to the same top-level root, the framework should perform one layout pass for that root.

---

# 8. Framework-known properties remain framework-known

The framework must continue to understand properties that its own subsystems directly interpret.

Typical examples include `size`, `min/max size`, `padding`, `border`, `position mode`, and `overflow`. The custom component should not duplicate those semantics merely because it owns its layout policy.

---

# 9. Component-owned properties and phase semantics

A property remains component-owned when the component can express its semantic consequences through the existing phase contracts.

```text
component property
    ↓
Measure / Arrange / Draw / update / input behavior
    ↓
explicit notification when framework-derived state becomes stale
```

Framework does not need property metadata for these fields.

---

# 10. PanelNode structural capability

At this stage **arbitrary structural composition from plain `Node` is intentionally rejected**.

```text
MyLeaf : Node
    custom state
    custom Measure/Arrange/Draw
    no framework-visible children

MyContainer : PanelNode
    framework-visible children
    custom Measure/Arrange/Draw
```

`PanelNode` is a structural capability implementation, not a mandatory layout-policy base class.

`StackPanelNode` is a specialized component:

```text
StackPanelNode
    = PanelNode + predefined linear layout policy
```

A future custom container can therefore be:

```text
CustomPanel : PanelNode
    = PanelNode + developer-defined layout policy
```

---

# 11. Custom layout contract

```text
Node
    Measure
    Arrange
    Draw

PanelNode
    Measure/Arrange may use framework-managed child operations
```

The component owns layout policy. The framework owns child measurement execution, child arrangement execution, constraint semantics, geometry commit, scheduling, and traversal.

Custom components do not receive direct access to layout queues, mutation queues, phase flushing, or raw geometry storage.

---

# 12. Measure contract

Measure means:

> Determine this component's desired content size under the effective constraints supplied by the framework.

The supplied available content size is an upper constraint, not a promise of allocation. A component may report a desired size larger than the supplied constraint.

A leaf derives its own desired content size. A `PanelNode` may recursively measure children and aggregate their desired sizes.

The current scope uses one ordinary Measure result per child invocation. Multi-pass/intrinsic measurement remains deferred until a concrete requirement demands it.

---

# 13. Arrange contract

Arrange means:

> Use the final content geometry assigned by the framework to establish the component's actual internal geometry.

A leaf may simply use that geometry. A `PanelNode` decides child allocations and delegates child arrangement back to framework-managed operations.

---

# 14. Rendering and Overflow

`Overflow::HIDDEN` is a render-traversal concern, not a Measure/Arrange concern.

The current `NodeTree::drawSubtree()` uses RAII renderer-state scopes and intersects the current renderer clip with the node rectangle before drawing the node and its descendants. The previous renderer state is restored when the subtree scope exits.

Conceptually:

```text
parent clip
    ∩
node clip
    ↓
draw node
    ↓
draw children
    ↓
restore previous clip
```

`PanelNode` does not own child render traversal. `NodeTree` owns clipping, traversal ordering, mutation safety, and root/overlay ordering; `Node::draw()` is responsible for the node's own visual content.

---

# 15. Current scope intentionally does not expose invalidatePaint()

Rendering currently executes as part of every frame's draw traversal. Therefore no separate paint dirty queue has been justified yet.

A future `invalidatePaint()` API may be introduced only if the rendering pipeline changes to require explicit paint scheduling.

For the current retained-mode renderer, render-only state can be changed without forcing a Measure/Arrange pass.

---

# 16. Structural mutation semantics

`PanelNode` and `NodeTree` already own child mutation, ownership, lifecycle, registration, traversal, and structural layout consequences.

Therefore an ordinary:

```text
PanelNode::addChild()
PanelNode::removeChild()
```

does not require a public `treeStructureChanged()` notification.

Structural changes during an active framework phase are deferred through `NodeTree` mutation scopes and observed by later phases after the mutation queue is flushed.

---

# 17. Framework-known `Auto` / `Value`

`LayoutValue` distinguishes `Auto` and explicit `Value`, but the meaning is resolved by framework layout policy rather than exposed as generic Measure metadata.

Custom layout receives effective Measure constraints, not the raw `Auto/Value` property representation.

This keeps `Auto` parent/layout-policy semantics framework-owned and prevents the generic Measure contract from becoming coupled to one particular framework property representation.

---

# 18. What is intentionally rejected for this stage

The refactor does not currently introduce:

```text
universal property registration
property metadata/dependency system
dynamic property map
automatic observation of arbitrary fields
global property dependency graph
diffing/reconciliation engine
React-style tree reconciliation
arbitrary child ownership from plain Node
full WPF DependencyProperty clone
full CSS/Flex/Grid semantics
multi-pass intrinsic measurement as default behavior
```

These remain possible future additions only when concrete requirements demand them.
