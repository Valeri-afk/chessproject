# Semantic notifications through `UIManager`

> **Status:** architecture exploration / working hypothesis  
> **Branch:** `fix/sharp-logical-text`  
> **Purpose:** record the current conclusion about where semantic notifications should live and how they should interact with the existing runtime facade.  
> **Not an ADR:** this document records a direction worth investigating, not a final API commitment.

---

# 1. Current observation

The current framework already has a public `UIManager` facade which owns the principal runtime subsystems:

```text
UIManager
 ├── NodeTree
 ├── InputSystem
 ├── ModalSystem
 ├── LayoutSystem
 └── ScrollSystem
```

`UIManager` also already exposes public framework operations such as:

```text
processEvent()
runFrame()
addRoot()
addOverlay()
removeRoot()
removeOverlay()
showModal()
closeModal()
enableScrolling()
setScrollOffset()
```

Internally it also defines the ordering of major runtime phases, including state synchronization, mutation preparation, layout, scrolling, modal synchronization, node update, and rendering.

Therefore `UIManager` is already the natural public facade between client code and the framework's internal runtime.

---

# 2. Current `Node` → `NodeTree` path

At present `Node` contains a private:

```cpp
NodeTree *owner_ = nullptr;
```

and `NodeTree` is a friend of `Node`.

For layout-related properties, the actual path is approximately:

```text
Node::setPadding()
    ↓
Node::deferLayoutMutation()
    ↓
private owner_
    ↓
NodeTree::enqueueNodeMutation()
    ↓
NodeTree mutation executes
    ↓
NodeTree::insertLayoutQueueById()
```

Thus `Node` already acts as a small public/property-side bridge into the framework runtime.

The important observation is that the **mechanism itself is already owned by `NodeTree`**. `Node` does not independently implement the scheduling policy.

---

# 3. Current structural path

Structural mutation is even more strongly routed through `NodeTree`:

```text
PanelNode::addChild/removeChild
    ↓
NodeTree when owned
    ↓
registration / ownership
mount / unmount
layout scheduling
mutation safety
```

`NodeTree` also currently assumes a `PanelNode` parent for child operations, resolving live parents through `dynamic_cast<PanelNode*>`.

The current structural protocol therefore has a clear centralized authority.

---

# 4. Proposed semantic-notification boundary

The current working hypothesis is that semantic notifications should be exposed through the existing `UIManager` facade rather than proxied through `Node`.

Conceptually:

```cpp
uiManager.contentChanged(node);
uiManager.structureChanged(node);
```

The intention is **not** to expose runtime operations.

The intention is to report semantic facts to the framework:

```text
Developer:
    "this semantic domain has changed"

UIManager:
    receives the fact

Internal framework:
    decides what consequences follow
    and when they are processed
```

---

# 5. Why `UIManager` is a natural boundary

`UIManager` already owns the major runtime subsystems and acts as their public facade.

This avoids adding another public object solely to route notifications.

A separate `FrameworkContext` would be justified only if the public `UIManager` eventually becomes too broad or if a distinct lifetime/context model becomes necessary.

At the current stage, an additional facade appears unnecessary.

---

# 6. What should NOT be exposed through this API

The notification API must not turn `UIManager` into a runtime control panel.

Avoid exposing developer-facing operations such as:

```text
invalidateLayout()
requestFullLayout()
flushMutationQueue()
mount()
unmount()
rebuildHitTest()
runLayoutNow()
runInputPhaseNow()
```

Those are implementation/control mechanisms.

The desired public boundary is semantic.

---

# 7. Semantic notification vs runtime command

A semantic notification means:

```text
"I changed a domain of state that the framework understands."
```

A runtime command means:

```text
"Execute this framework phase or internal operation now."
```

The first should be allowed; the second should remain framework-owned.

Therefore:

```text
contentChanged()
structureChanged()
```

are examples of the intended category.

Whereas:

```text
invalidateLayout()
```

is closer to framework implementation terminology and should not automatically become the public contract.

---

# 8. Notification semantics

The current intended meaning of a notification is closer to a **semantic commit boundary** than to an immediate invalidation command.

Example:

```cpp
component.setText(...);
component.setFont(...);
component.setIcon(...);
uiManager.contentChanged(component);
```

The setters mutate local state.

The notification means:

```text
The component has finished the relevant local series of mutations.
The framework may now treat the semantic state as ready for processing.
```

The framework decides:

- whether processing happens immediately;
- whether it is deferred;
- whether it is coalesced with another notification;
- which internal subsystems are affected;
- which frame/lifecycle boundary is safe.

The developer is not responsible for these decisions.

---

# 9. Why the notification should not be universal

A single:

```cpp
uiManager.changed(node);
```

would force the framework to determine what actually changed:

```text
what semantic domain?
what subsystem?
layout?
render?
structure?
input?
```

That would introduce an additional inference/reconciliation layer.

Such a mechanism could become similar to change scanning, dependency tracking, or diffing, depending on implementation.

Therefore the current direction favors **a bounded set of semantic notification domains**.

The exact set is not fixed yet.

---

# 10. Notification vocabulary must remain semantic

Good candidates are concepts the framework already understands semantically, for example:

```text
contentChanged
structureChanged
```

Potentially other domains may emerge.

The framework should be careful not to expose every internal phase as a notification:

```text
measureInvalidated
arrangeInvalidated
hitTestInvalidated
renderInvalidated
inputDispatchInvalidated
```

That would leak internal scheduling terminology into the public contract and effectively give the developer a partial copy of the internal runtime model.

The preferred boundary is:

```text
Developer knows:
    which semantic domain changed

Framework knows:
    what that semantic domain means operationally
```

---

# 11. `contentChanged` as an example

A future `contentChanged(Node&)` could conceptually mean:

```text
content-related semantic state is now different
```

The framework might know that a given content contract can affect:

```text
measurement
rendering
```

The developer should not need to know the internal queues or dependency propagation used to process those consequences.

This is particularly relevant to the `TextNode` problem.

A custom component could conceptually own its own representation:

```cpp
class MyButton : public Node
{
    TextState text_;

    void setText(...)
    {
        text_ = ...;
    }
};
```

and report the semantic change through `UIManager` without requiring `TextNode` solely as an invalidation carrier.

The framework can still understand that "text/content" participates in layout and rendering.

---

# 12. `structureChanged` as an example

A future `structureChanged(Node&)` could mean:

```text
The framework-visible child structure associated with this component has changed.
```

The framework would then own:

- tree integration;
- ownership/registration;
- lifecycle consequences;
- layout consequences;
- any input-state reconciliation that is actually required.

The notification must not mean:

```text
"I changed internal NodeTree arrays directly; please repair them."
```

The exact structural contract still needs to be designed.

---

# 13. Hit-testing does not necessarily need its own notification

The current implementation performs hit testing from current retained state rather than from a dedicated hit-test cache.

Therefore structural or semantic changes can often become visible to hit testing simply because the next query reads current tree/state.

This is an example of a subsystem that may not need another public notification domain.

The framework should only add such notifications if a future cached/optimized input representation creates a real need.

---

# 14. Layout remains different

Layout produces derived state:

```text
semantic state
    ↓
measurement
    ↓
desired size
    ↓
parent measurement
    ↓
arrangement
```

Therefore layout consequences do need controlled scheduling.

The intended public contract is not:

```text
"developer, invalidate layout"
```

but potentially:

```text
"developer, report the relevant semantic change"
```

with `UIManager`/internal framework determining the correct layout work.

The existing `deferLayoutMutation()` model demonstrates why the framework should be able to postpone work until a safe point.

---

# 15. Relation to the existing `Node` API

Framework-owned properties can continue to use convenient `Node` setters:

```text
setPadding()
setSize()
setPosition()
setVisible()
...
```

Those setters may continue to use private `owner_` and internal framework machinery.

A new semantic notification path does not require removing this convenience layer.

Instead, the architecture may have two complementary mechanisms:

```text
framework-defined property API
    ↓
Node setter
    ↓
framework runtime
```

and:

```text
custom framework-aware semantics
    ↓
UIManager notification
    ↓
framework runtime
```

This lets existing infrastructure survive while giving custom components a public extension boundary.

---

# 16. Important ownership boundary

`UIManager` should not expose the `NodeTree` itself to custom components merely because it is the notification entry point.

The intended boundary is:

```text
Developer
    ↓
UIManager semantic API
    ↓
NodeTree / LayoutSystem / InputSystem / ...
```

not:

```text
Developer
    ↓
NodeTree directly
    ↓
raw runtime manipulation
```

This preserves the current runtime invariants while opening selected semantic contracts.

---

# 17. Relation to contracts

The notification is only one type of contract.

Other contracts already exist:

```text
measureContent()
arrangeContent()
draw()
hitTest()
onMount()
onUnmount()
event handlers
```

These answer:

> **How does this component participate in a framework phase?**

Semantic notifications answer:

> **Why should the framework reconsider the component's semantic state?**

This yields two separate extension dimensions:

```text
phase participation
+
change reporting
```

They should not be collapsed into one universal API.

---

# 18. Contract responsibility

The developer may be required to obey contracts such as:

```text
measureContent()
    returns desired size under provided constraints

arrangeContent()
    positions component/children inside allocated space

draw()
    performs only the intended rendering behavior

notification
    is issued when required semantic state has reached a committed state
```

The framework's responsibility remains:

```text
runtime integrity
ownership
lifecycle ordering
scheduling
safe traversal
subsystem orchestration
```

If a developer violates a semantic contract, the current direction favors preserving **runtime integrity** as the framework guarantee rather than attempting to make every misuse automatically correct.

Diagnostic/debug behavior can be considered separately.

---

# 19. Relationship to batching

The semantic-notification model naturally supports batching:

```text
local mutation A
local mutation B
local mutation C
    ↓
semantic notification
    ↓
framework-controlled scheduling
```

The developer decides when the local semantic state is sufficiently complete to notify the framework.

This introduces an explicit responsibility:

```text
forgetting the notification may leave framework-derived state stale
```

That is accepted as a possible contract cost of greater extensibility.

The framework still owns the actual batching/coalescing/scheduling behavior after notification.

---

# 20. Current working architecture hypothesis

The current direction can be summarized as:

```text
                       UIManager
                  public framework facade
                           │
          ┌────────────────┼────────────────┐
          │                │                │
      semantic         framework        runtime
    notifications       commands        control
          │                │                │
          ▼                ▼                ▼
      NodeTree         subsystems       lifecycle
      Layout           Input            scheduling
      Rendering        Modal            invariants
```

Custom component model:

```text
Node
 │
 ├── local/custom state
 │
 ├── virtual phase hooks
 │
 └── semantic notifications through UIManager
```

The important boundary is:

```text
Developer controls:
    local state
    phase behavior
    semantic notification timing

Framework controls:
    interpretation
    scheduling
    lifecycle
    consequences
    runtime invariants
```

---

# 21. Open questions

This document does not resolve:

- exact notification names;
- exact number of notification domains;
- whether notifications take `Node&`, node ID, or another identity representation;
- whether a notification is always deferred or may be processed immediately when safe;
- whether `UIManager` should expose notifications directly or through a smaller public sub-interface later;
- how custom structural participation is represented;
- whether `PanelNode` becomes a standard implementation of a broader structural contract;
- how framework-recognized properties are declared and communicated to developers;
- how text semantics are represented independently of a concrete `TextNode` storage model;
- how batching and transaction boundaries interact with notifications.

These should be resolved through focused architectural experiments rather than assumed in advance.

---

# 22. Working principle

The current strongest hypothesis is:

> **Open the framework at semantic boundaries, not at runtime-control boundaries.**

The developer should be able to say:

```text
"this semantic state changed"
"this semantic structure changed"
"this is how my component participates in measure/arrange/draw/hit-test"
```

while the framework continues to decide:

```text
when
how
in what order
how often
which subsystems
```

This is intended to preserve the strongest properties of the current retained framework while reducing the architectural pain caused by excessive closure.
