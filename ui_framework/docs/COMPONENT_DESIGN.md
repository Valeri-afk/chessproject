# Component Design

## Framework boundary

The framework is intentionally minimal. A component belongs in the framework when it is a generic reusable UI concept with a clear contract. The chess client is a validation target, not a source of chess-specific framework components.

Before adding a component, first determine whether the required behavior is actually infrastructure:

```text
layout calculation
child hit-testing
common event dispatch
input routing
visibility/enabled filtering
focus/capture
modality
scroll coordination
```

Components should express semantic/visual state while infrastructure supplies coordinated mechanisms they cannot reasonably implement themselves.

## Developer vs framework responsibility

The component/client controls semantic meaning and component-specific state. The framework controls execution and runtime invariants.

```text
Developer/component:
    local semantic state
    visual properties
    semantic actions/callbacks
    custom Measure/Arrange/Draw behavior
    explicit notifications when derived framework state must be recomputed

Framework:
    lifecycle
    ownership / live-node state
    tree integration
    traversal
    scheduling
    layout execution
    hit-testing
    input routing
    event dispatch
    focus/capture
    render traversal
    clipping
    modality
    scroll mechanics
```

The imperative component API therefore does not mean that components control runtime phase ordering. Components participate in framework-owned phases through stable hooks and registration APIs.

## Node vs PanelNode

Use `Node` by default.

Use `PanelNode` only when structural children are part of the component's semantics and the component needs framework-managed child ownership/layout.

These do not by themselves justify `PanelNode`:

```text
text
icons/images
borders/backgrounds
multiple drawing primitives
```

`StackPanelNode` should be reused when its linear layout policy matches the component rather than reimplemented locally.

## Component responsibilities

Components own:

```text
component-specific semantic state
component-specific visual properties
presentation
semantic actions/callbacks
coordination of intentionally specialized children
custom Measure/Arrange/Draw policy
```

Framework infrastructure owns:

```text
NodeTree lifecycle/traversal
layout/geometry processing
hit-testing
input/event dispatch
focus/capture
render traversal
clipping
mutation/update scheduling
modality
scroll mechanics
```

## Primitive vs Node

A primitive is appropriate for a reusable, stateless/nearly-stateless drawing operation independent from Node lifecycle and interaction.

A component/Node is appropriate when it has independent semantic state, layout participation, event handling, lifecycle, hit-testing or a presentation contract.

Current primitives remain below the component layer:

```text
component
  ↓
visual state
  ↓
rendering primitive/backend
  ↓
SDL renderer
```

Text has a dedicated internal `TextContent`/`TextLayout`/`TextRenderer` path rather than exposing a low-level renderer as a public component API.

## Children and content

There is no universal `content` model. Specialized components may define explicit child relationships such as:

```text
Menu       → MenuItem
TabControl → TabItem
```

Structural ownership and semantic content are separate concepts.

## State ownership

A component owns its own semantic state. Composite components may coordinate intentionally specialized children when that relationship is intrinsic to their contract.

Examples:

```text
Menu       → active/selected MenuItem
TabControl → selected/active TabItem
```

Do not add generic `selected`, `active`, `highlighted` or similar Node properties merely because several components use the same word.

State changes should drive presentation; rendering should not require a separate client synchronization protocol.

## Inheritance

Inheritance is justified only when the specialized component genuinely extends a stable parent contract.

Current example:

```text
ToggleButton : Button
```

Do not introduce generic bases such as `ButtonBase`, `SelectableNode` or `ContentNode` until concrete components prove a stable shared contract.

## Standard component layer

Current standard components include:

```text
Button
ToggleButton
Menu / MenuItem
TabControl / TabItem
Checkbox
RadioButton
Slider
Dropdown
Typography
StackPanelNode / PanelNode
```

`Paper`, `Label` and `Card` are composition/styling patterns rather than mandatory framework components.

Text input/editing and Image are infrastructure-dependent future components.

## Semantic callbacks and events

There is one framework event registration mechanism:

```cpp
node->on<ConcreteEvent>(callback);
```

Components may register internal event handlers through the protected registration path because those handlers implement the component's own behavior.

Components may additionally expose semantic callbacks such as activation, toggle, selection or value-change callbacks. These are convenience/semantic APIs, not a second event-dispatch mechanism.

The client registers custom behavior at the same event/semantic callback boundary without needing to know which low-level events a component uses internally.

## Setter and invalidation philosophy

Ordinary setters do not universally imply automatic layout invalidation. The framework deliberately does not observe arbitrary component fields or maintain a global dependency graph.

When a change has framework-derived consequences that must be recomputed, the responsible component/client must use the explicit invalidation contract. A semantic method may call invalidation internally when that consequence is intrinsic to the method's implementation, but this is a deliberate component behavior rather than a universal setter rule.

## No universal property/dependency system

Do not introduce a generic system merely to make every property observable:

```text
universal property registration
property metadata/dependency graph
dynamic property maps
automatic observation of arbitrary fields
global change tracking
reconciliation/diffing
```

The current design keeps component-owned state local and makes framework participation explicit. A more general property/dependency abstraction requires a concrete reusable requirement that cannot be expressed cleanly with the existing contracts.

## Implementation style

Keep component code small and semantic. A component normally:

```text
stores local state
exposes semantic properties/actions
participates in Measure/Arrange/Draw
uses the existing event API
coordinates explicitly owned specialized children
```

It should not:

```text
reimplement NodeTree
reimplement hit-testing
reimplement global event dispatch
reimplement generic layout engines
manage global modality/scroll state
expose backend caches
```

## Review checklist

Before adding a component:

1. Is it a generic UI concept?
2. Which behavior is infrastructure?
3. Does it really require structural children?
4. Which child types are semantically valid?
5. Which state belongs to the component vs children?
6. Can an existing node/component already provide the required infrastructure?
7. Is a new primitive or abstraction actually necessary?
8. Does inheritance represent a real stable contract?
9. Does the API create unnecessary synchronization responsibilities for custom developers?
10. Is the abstraction simple enough to keep the framework minimal?
