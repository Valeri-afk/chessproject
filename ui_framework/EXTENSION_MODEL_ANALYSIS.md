# EXTENSION MODEL ANALYSIS

> **Status:** architecture analysis / design exploration  
> **Branch:** `fix/sharp-logical-text`  
> **Scope:** `ui_framework` extension points, component model, framework-aware state, structural participation, lifecycle, input, layout, notifications, and batching.  
> **Purpose:** identify where the current closed architecture is strong, where it becomes restrictive, and which extension points may need to become explicit.  
> **Not an ADR:** this document deliberately does not select a final implementation architecture.

---

## 1. Executive summary

The current framework is best described as a **retained-mode framework with imperative C++ syntax, declarative semantics, and framework-owned execution/lifecycle**.

The current design is strong where the framework already understands the semantics it exposes:

- retained `Node` identity and ownership;
- central `NodeTree` authority;
- safe deferred structural mutation;
- lifecycle ownership;
- query-time hit testing;
- centralized input routing;
- framework-controlled layout orchestration;
- framework-controlled rendering traversal.

The main weakness is not that the runtime is too closed in general. The weakness is that **framework-aware semantics are currently encoded directly into framework-provided node classes and their setters/hooks**.

This produces a chain:

```text
framework-owned invalidation/lifecycle
        ↓
framework must know framework-relevant state
        ↓
framework-relevant state lives in framework-defined classes
        ↓
new framework semantics require new framework node types / bases
        ↓
inheritance becomes a capability/authorization mechanism
        ↓
custom component composition becomes constrained
        ↓
special cases appear, e.g. TextNode
```

The architectural goal under investigation is therefore not a switch from declarative to imperative, nor a switch from retained to immediate mode.

The current target is:

```text
closed retained declarative semantics
                ↓
controlled extensibility
```

while keeping:

```text
framework owns:
    lifecycle
    scheduling
    runtime invariants
    tree integration
    input routing
    layout orchestration
    rendering orchestration
```

and opening selected points through:

```text
framework contracts
virtual hooks
semantic notifications
framework-recognized properties
custom structural participation
custom layout participation
batch/transaction semantics
```

The key distinction is:

> **Developer should be able to describe semantic facts and provide behavior; framework should continue to own the consequences and timing.**

---

# 2. Current framework model

## 2.1. Retained mode

The framework maintains a persistent runtime tree. Nodes have stable identity, parent relations, ownership, lifecycle, geometry, input state, and rendering behavior.

`NodeTree` is the central runtime authority for:

- live-node registry;
- ownership;
- roots and overlays;
- structural mutation;
- mutation queue;
- mount/unmount;
- traversal;
- layout queue;
- tree-level hit testing.

This is a retained architecture, not a description-only tree rebuilt each frame.

## 2.2. Imperative syntax

The user writes normal C++ mutations:

```cpp
node.setPadding(...);
node.setVisible(false);
panel.addChild(...);
panel.removeChild(...);
```

The API therefore looks imperative.

## 2.3. Declarative semantics

The meaning of many operations is not an immediate runtime command. It is a statement about semantic state or structure:

```text
visible = false
child X belongs to parent Y
padding = P
orientation = vertical
```

Framework subsystems decide what this means operationally and when the consequences are applied.

This gives the current framework the useful combination:

```text
imperative syntax
+
declarative semantics
+
framework-owned execution
```

This distinction should be preserved during refactoring unless there is a strong reason to change it.

---

# 3. Extension model: current state

The current extension surface can be grouped into several categories.

| Domain | Current extension mechanism | Framework knowledge | Current weakness |
|---|---|---|---|
| Local component state | arbitrary fields in custom Node | none | fine when state is local only |
| Framework properties | setters/fields on framework Node types | explicit | custom framework-aware properties are hard |
| Rendering | `draw()` virtual | high | generally good |
| Measurement | `measureContent()` virtual | high | good hook, but semantics are tied to framework layout model |
| Arrangement | `arrangeContent()` virtual | high | same limitation |
| Hit test | `hitTest()` virtual | high | good query-time model |
| Lifecycle | `onMount()` / `onUnmount()` | high | narrow but useful |
| Event handling | generic `Node::on<Event>()` | high | flexible, increasingly broad |
| Tree ownership | `PanelNode` / `NodeTree` | high | structural capability tied to base type |
| Tree mutation | `addChild/removeChild` | high | closed protocol; custom structural participation is constrained |
| Input state | `InputSystem` | high | framework-owned and mostly appropriate |
| Layout invalidation | `deferLayoutMutation()` | high for known properties | no generic path for custom framework-relevant state |
| Batching | partial/internal | framework | no general semantic transaction model exposed |

The most important issue is that **framework participation is currently concentrated in inheritance and framework-owned properties**.

---

# 4. Properties: the main architectural pressure point

## 4.1. Current model

Framework properties such as:

- position;
- size;
- min/max size;
- padding;
- border;
- overflow;
- visibility;
- enabled;
- focusable;
- capturable;

live in `Node` and are changed through framework-aware methods.

For layout-related state the pattern is effectively:

```text
property setter
    ↓
deferred framework mutation
    ↓
layout queue / invalidation
    ↓
future framework processing
```

For input-only state such as `enabled`, `focusable`, and `capturable`, the framework often uses query-time interpretation rather than explicit hit-test/input invalidation.

This is an important distinction:

> **Not every framework semantic requires invalidation.**

Some can be interpreted from current state when the subsystem needs them.

## 4.2. The restriction

A custom component can freely define:

```cpp
std::string title_;
float spacing_;
bool customState_;
```

when the framework does not care about them.

But if a custom property affects a framework-owned phase, the current system has no general channel for declaring that fact.

For example:

```cpp
class CustomLabel : public Node
{
    std::string text_;
};
```

If `text_` affects measurement, the framework needs to know:

```text
text changed
    ↓
old measurement is stale
    ↓
layout may be stale
```

The current API does not provide a general way for the custom component to communicate this while keeping invalidation closed.

## 4.3. Resulting pressure

The framework therefore tends to force framework-relevant state into framework-defined classes.

That makes inheritance carry two meanings:

```text
semantic identity
+
framework capability
```

This is the main reason the current component hierarchy becomes restrictive.

---

# 5. TextNode as a representative failure case

The text problem is not merely a text rendering problem.

Text state participates in derived framework state:

```text
text
font
wrapping
        ↓
measurement
        ↓
desired size
        ↓
parent layout
        ↓
rendering
```

The current framework cannot let an arbitrary custom component own text state and still transparently participate in this chain because its invalidation semantics are not known to the framework.

This led naturally to a framework-owned `TextNode`.

The text architecture then accumulated pressure around `TextPrimitive` ownership and exposure.

The broader architectural lesson is:

> **TextNode is the first serious example of a custom semantic capability that wants to become framework-aware without wanting to become a framework-defined base class.**

The final target should not be chosen here yet. The problem is a general extension-model problem.

---

# 6. Structural extension: current `PanelNode` model

## 6.1. Current ownership model

`PanelNode` stores `children_`.

If the node is already in a `NodeTree`, `PanelNode::addChild()` / `removeChild()` delegate to `NodeTree`.

If the node has no owner, local attach/detach is used.

This provides two modes:

```text
unowned PanelNode
    local structural mode

owned PanelNode
    framework-managed structural mode
```

The model works, but it introduces a conceptual duality.

## 6.2. Why `PanelNode` exists

`PanelNode` is currently the place where the framework's child-tree invariants are integrated:

- parent assignment;
- child ownership;
- cycle checks;
- interaction with `NodeTree`;
- subtree registration;
- lifecycle;
- layout scheduling.

Therefore:

```text
Node
    cannot simply become a child-owning structural node

PanelNode
    is the framework-approved structural node
```

This is functional but places structural capability into inheritance.

## 6.3. The architectural smell

Inheritance is effectively an authorization mechanism:

```text
inherit PanelNode
    ⇒
obtain framework structural participation
```

That can become restrictive when a custom component wants children but is not semantically a conventional panel.

---

# 7. Potential structural notification model

A possible extension point is an explicit semantic notification such as:

```text
children structure changed
```

The intention would be:

```text
Developer/component:
    reports the semantic fact

Framework:
    owns all consequences
```

The notification must NOT mean:

```text
please rebuild whatever is broken
```

Nor should it mean that the developer can directly mutate framework internals and then ask `NodeTree` to repair them.

A useful contract would conceptually mean:

> The component's framework-visible child structure has changed; framework should now perform its own structural reconciliation/integration.

Potential use cases:

- custom containers;
- virtualized containers;
- dynamic child generation;
- non-standard child composition.

For ordinary `PanelNode::addChild/removeChild` there is no need for an additional notification because the framework already knows the mutation directly.

---

# 8. Structural notification vs structural mutation

This distinction is central.

### Current direct framework mutation

```text
client
    ↓
addChild/removeChild
    ↓
NodeTree knows exactly what happened
    ↓
registration/lifecycle/layout bookkeeping
```

### Potential custom structural participation

```text
custom component state
    ↓
logical child structure changes
    ↓
structure-change contract
    ↓
framework owns integration consequences
```

The second model may enable custom structure without exposing raw `NodeTree` internals.

A future implementation may choose either:

- precise notifications (`child added`, `child removed`, `child moved`); or
- coarse notification plus framework reconciliation.

This document does not select one yet.

---

# 9. Lifecycle extension

The current framework already exposes narrow lifecycle hooks:

```cpp
virtual void onMount();
virtual void onUnmount();
```

This is a strong pattern:

```text
Developer implements WHAT
Framework decides WHEN
```

The developer does not invoke the lifecycle itself.

This principle is worth preserving.

The direction under investigation is therefore not:

```text
open lifecycle control
```

but:

```text
open lifecycle participation
```

A custom component may be notified of lifecycle transitions while the framework continues to own:

- ordering;
- traversal;
- mutation safety;
- ownership;
- timing.

---

# 10. Input and hit-test extension

Input is currently a relatively healthy extension domain.

## 10.1. Node-level semantics

`visible`, `enabled`, `focusable`, and `capturable` are semantic properties interpreted by `InputSystem`.

For input, the framework usually does not need to eagerly invalidate a separate structure. It can query current state at dispatch/validation time.

## 10.2. Hit-test

The current model is:

```text
pointer position
    ↓
NodeTree::hitTest()
    ↓
current retained structure
    ↓
visible/enabled/overflow
    ↓
Node::hitTest()
```

This is a good example of a framework extension point that does not require a general change-tracking system.

A custom Node can override `hitTest()` and the framework still owns when hit testing occurs and how the result is routed.

## 10.3. Event dispatch

The current dispatcher supports:

- target;
- tunneling;
- target phase;
- bubbling;
- propagation stop;
- mutation-safe path validation;
- handler snapshotting.

This is already a reasonable extension model.

The main open question is not whether input should become more open, but how far additional specialized input contracts should be introduced without exposing the full input state machine.

---

# 11. Layout extension

Layout is the most difficult extension domain because layout produces derived state.

A local custom property can affect:

```text
custom state
    ↓
measure
    ↓
desired size
    ↓
parent measure
    ↓
arrange
```

Therefore layout cannot always use the same cheap query-time model as hit testing.

Current layout already provides virtual hooks such as:

```cpp
measureContent(...);
arrangeContent(...);
```

This is valuable because it separates:

```text
Framework:
    owns when/how traversal and layout phases happen

Component:
    provides local layout behavior
```

The open question is whether the layout contract needs to become broader so a custom component can provide:

- custom measurement semantics;
- custom arrangement semantics;
- custom child layout behavior;
- custom intrinsic sizing;
- framework-visible dependencies;
- notifications/metadata for state that affects measurement.

Layout should be investigated separately from the generic component hierarchy because it has different invalidation requirements.

---

# 12. Rendering extension

Rendering is comparatively straightforward because custom state can often be consumed when the framework enters the draw phase.

Current model:

```text
NodeTree decides traversal
    ↓
Node::draw()
    ↓
component reads its own current state
    ↓
renders
```

This is phase-driven rather than necessarily change-driven.

As long as the framework draws when required, a custom rendering property does not necessarily need to be framework-recognized.

This suggests that a generalized property/change system should not be forced onto rendering if phase-driven evaluation is sufficient.

---

# 13. Component creation model

Current basic model:

```text
Node
  ↓
custom leaf component

PanelNode
  ↓
custom composite component
```

This is a valid C++ retained-mode model and is similar to traditional custom widget systems.

The concern is not that subclassing is wrong.

The concern is that the base class is being used to select runtime capabilities:

```text
Node
    → generic framework participation

PanelNode
    → structural participation

TextNode
    → text participation
```

This creates pressure toward a hierarchy where semantic identity and framework capability are the same thing.

That is the pattern to investigate.

---

# 14. Component model alternatives under consideration

No decision is made here, but the following models are worth comparing.

## A. Base-class capability model

```text
Node
├── PanelNode
├── TextNode
├── StackPanelNode
└── ...
```

Advantages:

- simple C++ API;
- strong static type identity;
- easy framework ownership.

Disadvantages:

- capability and semantics become coupled;
- inheritance pressure;
- new framework semantics may require new base types;
- composition becomes harder.

## B. Contract/virtual participation model

```text
Node
    + optional framework participation contracts
```

Possible contracts:

- measurement;
- structure;
- rendering;
- hit testing;
- lifecycle;
- framework-aware state.

Advantages:

- semantic component type is less constrained;
- framework capabilities can be orthogonal.

Disadvantages:

- more contract surface;
- more lifecycle/reentrancy rules to document;
- risk of contract explosion.

## C. Property/semantic contract model

```text
custom component state
    ↓
framework-recognized property contract
    ↓
framework knows semantics and effects
```

Advantages:

- custom framework-aware state without dedicated Node subclasses;
- potentially good batching and automatic scheduling.

Disadvantages:

- introduces property infrastructure;
- metadata/dependency design can become complex;
- must preserve strong C++ typing.

## D. Mixed model

Most likely candidate for further investigation:

```text
base Node
+
small set of stable virtual contracts
+
selected semantic notifications
+
selected framework-recognized properties
+
composition
```

Framework still owns execution.

---

# 15. Batching and transaction semantics

The current framework has deferred mutation and queues, but those mechanisms are mostly coupled to individual framework operations.

A larger component update may logically contain:

```text
multiple property changes
+
multiple structural changes
+
possibly multiple semantic notifications
```

Treating every operation independently can create unnecessary repeated scheduling and makes it harder to express:

> "I am constructing one new semantic state; process it once when I am done."

A future extension model should therefore investigate a semantic transaction boundary:

```text
begin semantic update
    ↓
local mutations
    ↓
end semantic update
    ↓
framework processes combined consequences once
```

This does not imply that a public `begin()/end()` pair is necessarily the right API.

The architectural requirement is simply:

> multiple semantic mutations should be able to form one logical update.

---

# 16. Important distinction: fact notifications vs runtime commands

A healthy extension model should distinguish:

### Semantic notification

```text
children changed
content changed
custom semantic state changed
```

from:

### Runtime command

```text
invalidate layout
flush layout
rebuild hit-test
run lifecycle phase
```

The first can be a reasonable developer-facing contract while the second exposes framework control.

This suggests a useful principle:

> **Developer may report semantic facts; framework interprets and executes their consequences.**

---

# 17. What a future framework-aware property might mean

A property that participates in the framework need not be a framework-defined field on `Node`.

Conceptually it could declare:

```text
property:
    type
    ownership
    semantics
    affected phases
    change behavior
```

For example, a text-related semantic property might conceptually mean:

```text
text
    affects measurement
    affects rendering
```

The framework could then own:

```text
change detection
scheduling
invalidation
recomputation
```

The exact mechanism is open.

The important architectural requirement is to separate:

```text
property meaning
```

from:

```text
specific Node subclass that stores it
```

---

# 18. What should remain completely local

Most component state should remain invisible to the framework.

Examples:

```text
chess piece type
engine selection state
animation phase
internal state machine
cached local calculations
debug metadata
application-specific model references
```

Framework should not attempt to understand these.

The desired distinction is:

```text
local component state
    ↓
component owns semantics

framework-recognized state
    ↓
component declares participation
framework owns consequences
```

---

# 19. Design constraints for future extension points

Any new extension mechanism should satisfy most of the following:

1. **Framework still owns execution order.**
2. **Framework still owns runtime invariants.**
3. **Custom state can remain private when framework does not care about it.**
4. **Framework-relevant custom state can be declared without creating a new framework base class for every capability.**
5. **Structural participation should not require pretending that every custom container is a generic `PanelNode`.**
6. **Notifications should describe semantic facts rather than dictate runtime work.**
7. **Virtual hooks should define how a component participates, not when the framework runs.**
8. **Batching should be expressible without requiring a full reactive engine.**
9. **Contracts should remain small enough to understand and test.**
10. **C++ types and ownership should remain explicit rather than becoming a dynamic property bag.**

---

# 20. What should NOT happen during the refactor

Several tempting directions should be treated carefully.

## Do not make every property generic/dynamic

A universal:

```cpp
setProperty("foo", value)
```

system would weaken compile-time guarantees and obscure semantics.

## Do not add one base class for every capability

A hierarchy such as:

```text
Node
 ├── TextNode
 ├── SelectableNode
 ├── ScrollNode
 ├── ContentNode
 ├── FocusableNode
 └── ...
```

risks recreating the current problem under a larger inheritance tree.

## Do not expose arbitrary runtime control

Avoid turning every extension point into:

```text
custom invalidate
custom schedule
custom flush
custom traversal
custom dispatch
```

This would destroy the advantages of centralized runtime ownership.

## Do not introduce diffing solely because "declarative means diffing"

Diffing is one reconciliation strategy, not the definition of declarative architecture.

## Do not preserve current internals at all costs

If the desired extension model cannot be expressed cleanly with the current internal ownership/scheduling design, a substantial internal refactor may be justified.

---

# 21. Test components for the future model

Any proposed extension model should be evaluated against real stress cases rather than abstract examples.

### A. Custom text component

```text
CustomNode
    owns text state
    participates in measurement/rendering
```

Must not require:

- inheriting `TextNode` just for text semantics;
- storing a hidden `TextNode` purely for invalidation;
- exposing `TextPrimitive`.

### B. Composite component

```text
CustomNode
    owns several framework-visible children
```

Must not require pretending the component is semantically a `PanelNode` if it is not.

### C. Virtualized/custom container

```text
logical collection: 100000 items
runtime children: 20 visible nodes
```

Must have a way to expose framework-visible structure without exposing raw tree internals.

### D. Custom layout

```text
CustomNode
    owns arbitrary child/layout semantics
```

Framework controls when layout occurs; component controls its local layout behavior.

### E. Custom hit-test

```text
CustomNode
    hit region is not a simple rectangle
```

Framework continues to own input routing.

### F. Batch update

```text
10 properties
+
5 children
+
1 semantic notification
```

Framework should process consequences coherently rather than performing 16 unrelated framework transactions.

### G. Reentrant callback

A custom hook changes state or structure during another framework phase.

The framework must preserve its runtime invariants.

---

# 22. Questions to answer before implementation

The following questions should be answered explicitly before a major refactor:

### Properties

- What exactly makes a property "framework-aware"?
- Is framework awareness declared per property?
- Is it inferred through the type/contract?
- Who owns the value?
- Who detects its mutation?
- Who specifies affected phases?

### Structure

- Who owns logical children?
- Who owns runtime children?
- Can a custom component participate in structural semantics without inheriting `PanelNode`?
- Does structural notification trigger exact mutation processing or reconciliation?
- How is batching represented?

### Lifecycle

- Which lifecycle moments are safe extension points?
- Which callbacks are allowed to mutate state?
- Which mutations are deferred?
- What reentrancy guarantees exist?

### Layout

- Can custom components define layout behavior independently of base class?
- Which custom state can affect measurement?
- How does the framework learn that measurement is stale?

### Input

- Which input behavior is already sufficiently extensible?
- Which additional hooks are actually needed?
- Should focus/capture remain fully framework-owned?

### Rendering

- Is phase-driven evaluation sufficient?
- Are render invalidation notifications needed for future optimization?

### Batching

- What is a semantic transaction?
- When are notifications coalesced?
- What is the commit boundary?

---

# 23. Current architectural diagnosis

The main architectural smell is not:

```text
NodeTree exists
```

or:

```text
framework owns lifecycle
```

or:

```text
API is imperative C++
```

The stronger issue is:

```text
framework capability
        ≈
framework-defined base type
```

This creates pressure in three places:

```text
properties
structure
component composition
```

and `TextNode` is the first place where all three pressures intersect.

---

# 24. Current working direction

The present exploration favors the following architectural direction, without treating it as a final decision:

```text
Retained runtime
+
Imperative C++ API
+
Declarative semantic meaning
+
Framework-owned execution/lifecycle
+
Controlled extension contracts
+
Semantic notifications
+
Optional framework-recognized properties
+
Phase hooks
+
Explicit batching/transaction semantics
```

The objective is not maximum freedom.

The objective is:

> **enough openness that custom components can express new meaningful semantics without forcing every framework-aware capability into a new framework-provided Node type, while keeping enough central control that runtime correctness does not depend on manual lifecycle management.**

---

# 25. Next architectural work

The next step should not be immediate implementation.

First define a target extension model and compare at least several alternatives:

```text
Model A
current closed model + minimal notifications

Model B
explicit contracts + virtual hooks + notifications

Model C
framework-recognized property/semantic system
```

For each model, test:

- TextNode/text semantics;
- custom properties affecting layout;
- custom composite containers;
- virtualized structures;
- custom layout;
- custom hit testing;
- lifecycle participation;
- batching;
- reentrancy.

Then compare the resulting public contracts against the current `NodeTree`, `LayoutSystem`, `InputSystem`, and rendering architecture.

Only after this comparison should implementation changes be selected.

---

# 26. Working principle

For every proposed extension point, ask:

```text
1. What can the framework discover itself?
2. What semantic fact can only the developer/component know?
3. What is the smallest useful notification or contract for that fact?
4. What consequences must remain framework-owned?
5. Can the same mechanism support batching and reentrancy safely?
```

A contract is likely too closed if the only way to introduce a new framework semantic is:

```text
create a new framework base Node type
```

A contract is likely too open if the developer must manually control:

```text
lifecycle
scheduling
framework invalidation internals
traversal
```

The target area is the boundary between those extremes.
