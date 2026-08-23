# Current Refactor Direction

> **Status:** current working direction / not an ADR  
> **Branch:** `fix/sharp-logical-text`  
> **Purpose:** record the architecture currently chosen after the investigation of imperative/declarative UI models, `TextNode`, framework properties, invalidation, custom layout, and structural composition.

---

# 1. Current direction

The current direction is a **retained-mode framework with framework-owned runtime execution and deliberately opened imperative extension points**.

The framework remains responsible for coordinated runtime mechanisms, while developers receive explicit contracts and notifications where hidden framework knowledge is creating unnecessary restrictions.

The current working layout direction is:

```text
Two-pass Measure → Arrange

Framework owns:
    constraints
    layout traversal
    layout scheduling
    invalidation consequences
    geometry commit

Developer owns:
    custom component layout behavior
```

This is a working direction, not yet a final implementation decision.

---

# 2. Layout model

The selected layout model for further investigation is a two-pass model:

```text
Measure
    ↓
desired size
    ↓
Arrange
    ↓
actual geometry
```

The framework continues to own the layout engine and the execution of these passes.

The developer may provide custom layout behavior for a component through the layout contract.

The developer does **not** own:

```text
when layout runs
layout traversal
constraint propagation outside the component contract
layout queue management
ancestor invalidation propagation
geometry commit
```

These remain framework responsibilities.

The chosen model is motivated by the actual target application class and current framework requirements:

```text
intrinsic content
text measurement
width-dependent text wrapping
nested panels
stack/linear layout
padding/border
min/max constraints
auto/content-sized components
absolute positioning
```

The project deliberately does not attempt to become a CSS/Flex/Grid-equivalent layout system at this stage.

---

# 3. Custom layout

Custom layout is being considered as a primary extension mechanism.

A custom component may own arbitrary state and use that state while implementing its own Measure/Arrange behavior.

Conceptually:

```text
Custom component
    owns:
        text
        font
        icon
        custom spacing
        custom sizing state
        other component-specific state

    Measure
        ↓
    desired size

    Arrange
        ↓
    child/component geometry
```

The purpose is to avoid introducing a general property-registration/property-metadata system solely to make arbitrary custom properties visible to the framework.

This is an explicit trade-off:

```text
Framework does not need to understand every custom property.
Component expresses the effect of its private state through Measure/Arrange/Draw contracts.
```

This hypothesis must still be validated against real components and realistic custom properties.

---

# 4. Custom property hypothesis

A core experiment is:

```text
custom property
    ↓
component-owned state
    ↓
Measure / Arrange / Draw uses the state
    ↓
explicit notification when the relevant semantic state is committed
    ↓
framework reruns the appropriate work
```

Example:

```cpp
text_ = ...;
font_ = ...;
iconGap_ = ...;
uiManager.invalidateLayout(*this);
```

The framework does not need to know the internal representation of `text_`, `font_`, or `iconGap_` to perform layout correctly if the component's Measure/Arrange implementation correctly translates those values into desired size and geometry.

The experiment is successful only if this model gives enough freedom for realistic custom components without creating an excessive or fragile layout contract.

---

# 5. Framework-known properties remain

The custom-property model does **not** mean that the framework stops understanding properties.

Some state must remain framework-readable because framework subsystems directly interpret it.

Examples include, depending on the final implementation:

```text
visible
enabled
focusable
capturable
overflow
position mode
framework-owned geometry constraints
other input/layout/painting state that the framework itself interprets
```

The exact set is intentionally not fixed by this document.

The principle is:

```text
Framework-known property
    = framework itself needs the semantic value

Component-owned property
    = component can express its effect through a framework behavior contract
```

This distinction is more important than whether a property is physically stored in `Node`.

---

# 6. Notifications

The framework will expose explicit notifications for important semantic/runtime changes.

The current conceptual candidates are:

```text
invalidateLayout()
invalidatePaint()
treeStructureChanged()
...
```

The exact final API is not yet fixed.

These notifications are intentionally separate rather than a universal `changed()` operation.

The reason is to avoid requiring the framework to inspect an opaque change and infer which subsystem is affected.

The developer reports the relevant semantic/runtime fact.

The framework owns the consequences.

For example:

```text
invalidateLayout(node)
    ≠
run layout now

invalidatePaint(node)
    ≠
run render now

treeStructureChanged(node)
    ≠
manually repair NodeTree
```

Notifications are reports/requests for framework-managed work, not direct control of internal phases.

---

# 7. Responsibility boundary for notifications

The developer is responsible for using the notification contract correctly.

The framework is responsible for:

```text
queuing/coalescing work
choosing a safe processing point
propagating consequences
preserving lifecycle ordering
preserving tree ownership invariants
```

A developer can misuse the contract by forgetting a required notification or calling a notification at an unsafe point.

The current architectural preference is **runtime integrity over attempting to make every contract misuse self-correcting**.

A notification may be documented as unsafe or invalid during particular lifecycle phases where the current runtime cannot guarantee correct behavior.

The framework remains responsible for not corrupting its own runtime state.

---

# 8. `UIManager` as public notification facade

The current direction is to expose semantic notifications through the existing `UIManager`, which already acts as the public facade over the internal runtime.

Conceptually:

```cpp
uiManager.invalidateLayout(node);
uiManager.invalidatePaint(node);
uiManager.treeStructureChanged(node);
```

The reason to use `UIManager` rather than proxy every notification through `Node` is architectural:

```text
Node
    = runtime object + state

UIManager
    = public framework facade

NodeTree / LayoutManager / InputManager / ...
    = internal mechanisms
```

`UIManager` should not expose raw `NodeTree` control or internal phase execution.

The public API should express semantic framework contracts, not implementation mechanics.

---

# 9. `Node` / `PanelNode` structural question remains open

The layout direction is selected, but structural extensibility is intentionally unresolved.

Current model:

```text
Node
    no framework-owned children

PanelNode
    owns child Nodes
```

The current framework uses `PanelNode` as the standard structural child container.

Two possible future directions remain:

### A. Keep `PanelNode` as the structural capability

```text
CustomContainer : PanelNode
```

This preserves a simple and strongly controlled ownership model.

### B. Introduce a broader structural contract

```text
CustomContainer : Node
    framework-visible children
    structural notification
```

This could permit custom container components without inheriting from `PanelNode`.

However, `treeStructureChanged()` by itself is insufficient. The framework would also need a reliable structural contract describing how framework-visible children are exposed and integrated with `NodeTree` ownership/lifecycle/traversal.

Therefore structural extensibility must be studied separately from custom layout and custom properties.

---

# 10. `TextNode` as validation case

`TextNode` remains the primary test case for the new model.

The target hypothesis is not necessarily to eliminate `TextNode` as a standard component.

Instead, determine whether `TextNode` is required merely because framework-aware text state currently has to live in a framework-provided Node subclass.

A successful custom-layout model should allow a custom text-bearing component to conceptually own:

```text
text
font
font size
color
alignment
other text presentation state
```

and use that state in:

```text
Measure
Arrange
Draw
```

while explicitly notifying the framework when layout/paint-relevant state has changed.

`TextPrimitive` can remain a low-level physical text measurement/rendering implementation.

The architecture should distinguish:

```text
text representation/storage
text semantic participation
layout/render phase contract
change notification
```

These do not have to be represented by one `TextNode` type.

---

# 11. Validation experiment: custom properties

The next architectural experiment is not a generic property system.

Use real component scenarios and classify their state:

```text
framework-known
component-owned
```

For component-owned state, verify that the following remains sufficient:

```text
local state mutation
    ↓
explicit notification
    ↓
framework-controlled Measure/Arrange/Draw
```

Priority examples:

```text
text
font
font size
text color
icon
custom spacing
custom sizing rules
internal layout mode
```

The question is:

> Can real component-specific state remain completely local while still expressing all framework-visible consequences through the existing phase contracts?

If yes, a general property-registration infrastructure may not be necessary for the current framework scope.

If no, the missing category of framework-recognized property/capability must be identified precisely before introducing a registration system.

---

# 12. Validation experiment: notifications

Start with a small semantic set rather than a universal notification mechanism.

Candidate domains:

```text
layout
paint
structure
```

For every candidate, determine:

```text
what fact it represents
who may call it
when it is safe
what work it schedules
whether it can be coalesced
what the framework guarantees after the call
```

Do not add a notification solely because a framework subsystem exists. A notification should correspond to a real public extension contract.

---

# 13. Validation experiment: custom layout

Build hypothetical/real custom components that require non-standard layout behavior without requiring new framework properties.

Examples:

```text
text-bearing button
custom badge
chess board view
custom information panel
custom composite with specialized internal geometry
```

For each:

```text
component-owned state
    ↓
Measure
    ↓
Arrange
    ↓
Draw
    ↓
notifications
```

The contract must be sufficient without giving the component direct access to internal layout scheduling.

---

# 14. What is intentionally NOT being introduced yet

The current direction does not require, at this stage:

```text
universal property registration
property metadata system
property map / dynamic property storage
global dependency graph
diffing/reconciliation engine
automatic observation of arbitrary component fields
React-style tree reconciliation
full WPF DependencyProperty clone
full CSS layout semantics
universal structural composition
```

These remain possible future responses to concrete requirements, not assumptions of the current refactor.

---

# 15. Why this direction is different from the previous architecture

The previous closed approach tried to guarantee framework correctness by keeping framework-relevant state and mechanisms inside framework-provided Node subclasses.

That produced pressure toward:

```text
framework-known property
    ↓
framework base Node class
    ↓
inheritance requirement
    ↓
TextNode / PanelNode / other specialized types
```

The current direction intentionally moves one boundary outward:

```text
framework
    retains:
        runtime control
        layout engine
        scheduling
        framework-known semantics

component
    gains:
        custom layout behavior
        custom component state
        explicit notifications
```

The objective is not to turn the runtime into an unconstrained imperative toolkit.

It is to remove the parts of closure that prevent natural custom component design.

---

# 16. Immediate next step

The next concrete investigation is:

```text
Audit current components and their properties.

For each relevant property:
    1. Is it directly interpreted by framework infrastructure?
    2. Can its effect be expressed entirely through Measure/Arrange/Draw?
    3. What notification does its mutation require?
    4. Does the component need framework-readable access to the value?
    5. Does the property reveal a missing general framework capability?
```

This audit should be performed against real current components before designing additional infrastructure.

The resulting classification will determine whether the selected model is actually sufficient or where it breaks.

---

# 17. Current architectural thesis

The current working thesis is:

> **Keep the framework closed where it must preserve runtime invariants and subsystem semantics; open it where a component can safely provide behavior and explicitly report semantic changes.**

For layout, the preferred experiment is:

```text
framework-owned two-pass Measure/Arrange
+
custom component-owned layout behavior
+
explicit framework notifications
+
framework-owned scheduling and consequences
```

The success criterion is not maximal extensibility.

The success criterion is:

```text
more natural custom components
without reintroducing
client-owned runtime coordination.
```
