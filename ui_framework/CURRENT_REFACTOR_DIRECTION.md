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

The current candidate API is:

```text
UIManager::invalidateLayout(node)
UIManager::invalidatePaint(node)
UIManager::treeStructureChanged(node)
...
```

The final names are not yet fixed.

Notifications mean:

```text
"this semantic state is no longer reflected in framework-derived state"
```

They do **not** mean:

```text
run layout now
run paint now
manually mutate NodeTree
flush the framework
```

The framework owns those consequences.

---

# 7. Existing layout batching/root promotion is retained

The current `fix/sharp-logical-text` implementation already has:

```text
mutationQueue_
layoutQueue_
layoutQueueSet_
```

and the layout queue already promotes an affected node to the top-level root/overlay before queueing it, with deduplication. Therefore the current queue should not be replaced merely to introduce explicit invalidation.

Existing semantics are effectively:

```text
changed Node
    ↓
walk parent chain
    ↓
top-level layout root
    ↓
queue once
    ↓
whole root subtree is Measure/Arrange processed
```

This existing mechanism is an important foundation for the refactor.

The future `invalidateLayout(node)` should reuse this behavior rather than create a separate propagation system.

---

# 8. Batching model

Repeated notifications are expected to be coalesced by the framework.

For example:

```text
setText()
    → invalidateLayout()

setFont()
    → invalidateLayout()

setCustomSpacing()
    → invalidateLayout()
```

may result in a single queued top-level layout root because `layoutQueueSet_` already deduplicates it.

Therefore batching does not require the developer to manually surround every sequence with a special batch transaction merely to avoid repeated layout requests.

A future explicit batch API may still be considered later, but it is not required for the basic notification model.

---

# 9. Mutation batching and structural consistency

The existing `NodeTree` already has deferred mutation scopes and mutation queue processing.

The target invariant is:

```text
structural mutation completes
    ↓
tree ownership/lifecycle invariants are valid
    ↓
layout invalidation is queued
    ↓
future Measure/Arrange sees the stable tree state
```

`PanelNode` remains the framework-provided structural capability:

```text
Node
    no framework-visible children

PanelNode
    owns framework-visible child Nodes
```

The framework remains responsible for child ownership, parent relationships, lifecycle, tree registration, and traversal.

---

# 10. `PanelNode` remains the structural capability

At this stage **arbitrary structural composition from plain `Node` is intentionally rejected**.

The current decision is:

```text
MyLeaf : Node
    custom state
    custom Measure/Arrange/Draw
    no framework-visible children

MyContainer : PanelNode
    framework-visible children
    custom Measure/Arrange/Draw
```

`PanelNode` is therefore best understood as a **structural capability implementation**, not as a mandatory layout-policy base class.

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

No general `Node` structural contract is being introduced at this stage.

---

# 11. Custom layout contract

The intended extension model is:

```text
Node
    Measure
    Arrange
    Draw

PanelNode
    Measure/Arrange may use framework-managed child operations
```

The exact C++ API is still under design.

The important semantic boundary is already decided:

```text
Component:
    owns layout policy

Framework:
    owns child measurement execution
    owns child arrangement execution
    owns constraint semantics
    owns geometry commit
```

The custom component must not receive direct access to:

```text
NodeTree internals
layout queues
mutation queues
phase flushing
raw geometry storage
framework scheduling internals
```

---

# 12. Measure contract

Measure has one universal meaning:

> Determine this component's desired content size under the effective constraints supplied by the framework.

For a leaf:

```text
custom state
    ↓
Measure
    ↓
desired content size
```

For a `PanelNode`:

```text
own state
    +
child measurements
    ↓
aggregate desired sizes
    ↓
own desired content size
```

The historical layout implementation measured a child through a single proposal and stored one desired size for the normal layout pass. The working contract for the current scope is therefore:

```text
one child
    → one ordinary Measure invocation
    → one effective proposal
    → one desired result
```

Multi-pass/intrinsic measurement is not part of the current contract. It can be introduced later as a separate capability if real layout requirements demand it.

---

# 13. Arrange contract

Arrange has one universal meaning:

> Use the final content geometry assigned by the framework to establish the component's actual internal geometry.

For a leaf this may involve little or no internal work.

For a `PanelNode` it includes:

```text
custom allocation policy
    ↓
child positions
child allocations
    ↓
framework-managed child arrangement
```

The framework remains responsible for final constraint resolution and actual geometry commit.

---

# 14. Constraints

`Constraints` are conceptually a framework-level layout concept, even if the current implementation still represents some parts through `Node::size`, `minSize`, `maxSize`, padding, border, and helper functions.

The refactor must preserve the existing semantic distinction:

```text
measurement proposal
    ≠
final arrangement allocation
```

Framework-owned properties are resolved by the framework before the component's custom layout behavior is executed.

The component should not be forced to duplicate framework `size/min/max/padding/border` semantics.

Whether the public API exposes a dedicated `Constraints` type or another equivalent abstraction remains an implementation/API design question.

---

# 15. `TextNode` and text architecture

`TextNode` remains a primary validation case, not necessarily the final architecture.

The desired result is that a custom text-bearing component can own:

```text
text
font
font size
color
alignment
other text state
```

and express the consequences through:

```text
Measure
Arrange
Draw
invalidateLayout()
invalidatePaint()
```

`TextPrimitive` can remain a lower-level physical text measurement/rendering mechanism.

The architecture should distinguish:

```text
text representation/storage
text semantic participation
layout/render phase behavior
change notification
```

These concepts do not have to be fused into a dedicated `TextNode` type.

---

# 16. Framework-known versus component-owned consequences

The working classification is:

```text
Framework-known state
    → framework directly interprets it

Component layout state
    → custom Measure / Arrange

Component paint state
    → custom Draw

Component interaction state
    → custom input/update behavior where supported

Notification
    → developer reports the semantic consequence
```

A component does not need to tell the framework what a property *means* when the component itself can express that meaning through its phase contracts.

---

# 17. Notification responsibilities

The developer is responsible for correct use of the notification contract.

The framework is responsible for:

```text
coalescing queued work
choosing a safe processing point
promoting layout invalidation to the correct layout root
running Measure/Arrange
preserving lifecycle ordering
preserving ownership/tree invariants
```

A forgotten notification may produce stale framework-derived state. This is an accepted part of opening imperative extension points.

The framework should protect runtime integrity, not attempt to infer arbitrary developer state changes automatically.

---

# 18. Notification scope remains semantic, not universal

The preferred model is several explicit notifications rather than one opaque `changed()` mechanism.

Candidate domains:

```text
layout
paint
structure
```

Each notification must define:

```text
what semantic fact it represents
who may call it
when it is safe
what framework work it may queue
whether it is coalesced
what guarantees exist after the call
```

Do not introduce notifications merely to expose implementation subsystems.

---

# 19. What is intentionally rejected for this stage

The refactor does **not** currently introduce:

```text
universal property registration
property metadata/dependency system
dynamic property map
automatic observation of arbitrary fields
global property dependency graph
layout diffing/reconciliation
React-style tree reconciliation
arbitrary child ownership from plain Node
full WPF DependencyProperty clone
full CSS/Flex/Grid semantics
multi-pass intrinsic measurement as default behavior
```

These remain possible future additions only when a concrete requirement demonstrates the need.

---

# 20. Concrete architectural work to perform

The refactor should proceed in this order:

## Step 1 — Public invalidation API

Expose semantic invalidation through `UIManager` and connect it to the existing `NodeTree` queue/root-promotion mechanics.

Target concepts:

```text
invalidateLayout(node)
invalidatePaint(node)
```

`treeStructureChanged(node)` remains a semantic option to evaluate alongside the existing `PanelNode → NodeTree` mutation path rather than replacing that path prematurely.

## Step 2 — Remove hidden invalidation from component-specific state

Audit existing components and replace framework-specific `deferLayoutMutation()` usage for component-owned state with explicit notifications.

Initial validation cases:

```text
Button::text
Button::font
MenuItem::text
MenuItem::font
Checkbox::boxSize
Menu::itemSpacing
other real component layout state
```

## Step 3 — Validate paint invalidation

Trace the current rendering pipeline and determine the smallest correct `invalidatePaint()` implementation and contract.

Validate on:

```text
textColor
backgroundColor
borderColor
variant
highlight/selection state
```

## Step 4 — Open Measure/Arrange behavior

Replace the current closed/specialized layout extension mechanism with a developer-facing custom layout contract while keeping framework ownership of execution.

First validate leaf components, then custom `PanelNode` containers.

## Step 5 — Generalize child layout operations

Derive the minimal child measurement/arrangement capability from the existing linear-layout implementation:

```text
measure child under proposal
obtain desired size
arrange child with allocation
```

Do not expose `NodeTree` or `LayoutSystem` directly.

## Step 6 — Preserve border-box semantics

Ensure custom Measure/Arrange receives the correct content-space geometry while padding/border/size/min/max remain framework semantics.

## Step 7 — Re-test batching and layout root behavior

Verify that repeated notifications coalesce into one top-level queued root and that mutations occurring before a layout phase are observed in their final stable state.

## Step 8 — Re-evaluate `TextNode`

Only after the new contracts work for real components, decide which role remains for `TextNode` and `TextPrimitive`.

## Step 9 — Re-evaluate documentation/API boundaries

Document the developer contracts, including misuse cases and lifecycle restrictions, after the runtime behavior is stable.

---

# 21. Immediate validation scenarios

The first complete end-to-end scenarios should be:

### Leaf

```text
Button
    setText
    invalidateLayout
    root queued
    Measure
    Arrange
    Draw
```

### Paint-only leaf state

```text
Button
    setTextColor
    invalidatePaint
    Draw
```

### Framework-known property

```text
Node
    setPadding / setMinSize / setMaxSize / etc.
    framework-owned invalidation
    existing layout semantics preserved
```

### Custom container

```text
CustomPanel : PanelNode
    custom property changes
    invalidateLayout
    Measure children
    aggregate desired sizes
    Arrange children
```

### Structural mutation

```text
PanelNode
    add/remove child
    ownership/lifecycle invariants
    parent/layout root invalidated
    next layout sees stable child list
```

---

# 22. Success criterion

The refactor is successful if a developer can build a genuinely custom component without requiring the framework to know every custom property, while the framework still controls runtime correctness.

Desired end state:

```text
Custom component
    owns arbitrary component state
    defines custom Measure/Arrange/Draw behavior
    explicitly reports semantic changes

Framework
    retains runtime control
    retains framework-known semantics
    retains batching/coalescing
    retains tree ownership/invariants
    retains border-box layout semantics
    retains lifecycle/input/render coordination
```

The goal is **not maximal openness**.

The goal is:

```text
open enough to make custom components natural
closed enough to preserve runtime invariants
```
