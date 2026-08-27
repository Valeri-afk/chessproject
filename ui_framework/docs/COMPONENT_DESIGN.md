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

Components express semantic and visual state while infrastructure supplies coordinated mechanisms.

## Developer vs framework responsibility

```text
Developer/component:
    local semantic state
    visual properties
    semantic actions/events
    custom Measure/Arrange/Draw behavior
    explicit invalidation when derived layout becomes stale

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

## Node vs PanelNode

Use `Node` by default.

Use `PanelNode` when structural children are part of the component's semantics and it needs framework-managed child ownership/layout.

These do not by themselves justify `PanelNode`:

```text
text
icons/images
borders/backgrounds
multiple drawing primitives
```

`StackPanelNode` should be reused when its existing linear layout policy matches the required behavior.

## Component responsibilities

Components own:

```text
component-specific semantic state
component-specific visual properties
presentation
semantic actions/events
custom Measure/Arrange/Draw policy
coordination of intentionally specialized children
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

A component implements its own default interaction semantics. The client should not have to reconstruct those semantics with low-level input handlers merely to make a standard component work.

## Primitive vs Node

A primitive is appropriate for a reusable stateless/nearly-stateless drawing operation independent from Node lifecycle and interaction.

A Node/component is appropriate when it has independent semantic state, layout participation, event handling, lifecycle, hit-testing or a presentation contract.

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

### State categories

```text
Persistent semantic state
    state that remains meaningful after interaction
    examples: checked, value, selected item, committed text

Interaction state
    temporary state used by interaction/presentation
    examples: pressed, hovered, dragging, pointer-selecting, focused

Events
    transient facts describing a transition or occurrence
    examples: activated, toggled, value-changed, text-changed, mouse-enter
```

An event and a state are not interchangeable.

### Ownership of interactive state

The standard component owns the invariants and default transitions of its own interactive state.

For example:

```text
Checkbox
    MouseClickEvent
        ↓
    component toggles checked
        ↓
    CheckboxToggledEvent
```

The client may set component state explicitly but does not need to implement the standard interaction behavior.

## Inheritance

Inheritance is justified only when the specialized component genuinely extends a stable parent contract.

Current example:

```text
ToggleButton : Button
```

Do not introduce generic bases such as `ButtonBase`, `SelectableNode` or `ContentNode` without a concrete stable shared contract.

## Standard component layer

The active standard components are:

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
TextInput
Image
StackPanelNode / PanelNode
```

`Image` is a thin visual component. It references an externally owned `SDL_Texture*`, does not own texture lifetime or asset loading, and supports intrinsic size, tint and `STRETCH` / `CONTAIN` / `COVER` fit modes.

`TextInput` is a single-line editable component. Its committed text/caret/selection state is component-owned, while private IME composition state remains internal.

## Event registration

There is one event-handler registration mechanism:

```cpp
node->on<ConcreteEvent>(callback);
```

`on()` is used by both client code and component implementations. Components may use private helper methods for internal event handling; that is not a second event system.

Semantic component events are also delivered through the same Node event mechanism.

## Input events vs semantic component events

Framework input/lifecycle events are defined in `events.hpp` and include mouse, keyboard, focus and text-input events. Components may define semantic events for meaningful state/action transitions.

For example:

```text
SDL text input
    ↓
InputSystem
    ↓
TextInputEvent
    ↓
TextInput internal handler
    ↓
committed text changes
    ↓
TextChangedEvent
```

Component-specific semantic event types live with their owning component when they are not shared framework infrastructure.

## Multiple handlers and propagation

Multiple handlers for the same event type on one Node are independent listeners. `stopPropagation()` controls propagation between nodes/phases; it does not cancel other callbacks already present in the current target's handler snapshot.

`Node` snapshots matching callbacks before invoking them. `EventDispatcher` separately controls tunneling/target/bubbling propagation.

## Setter and invalidation philosophy

Ordinary setters do not universally imply layout invalidation. The framework does not observe arbitrary component fields or maintain a global dependency graph.

When a change has framework-derived consequences, the responsible operation must use the explicit invalidation contract. This may happen inside a component method when the layout consequence is intrinsic to that operation.

## Presentation and client responsibility

A component owns presentation derived from its own semantic/interaction state and configured visual properties. The client configures the component and reacts to semantic events; it should not mirror component interaction state merely to keep standard presentation correct.

## Implementation style

Keep component code small and semantic. A component normally:

```text
stores local state
exposes semantic properties/actions
participates in Measure/Arrange/Draw
uses the existing event API
coordinates intentionally owned specialized children
implements its standard interaction behavior
emits semantic events after meaningful transitions
```

It should not:

```text
reimplement NodeTree
reimplement global hit-testing
reimplement global event propagation
reimplement the layout engine
manage global modality/scroll state
expose backend caches
require the client to implement standard interaction
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
11. Which low-level input events are consumed internally?
12. Which semantic state is public?
13. Which semantic events should be emitted?
14. Does the component need a public callback API beyond `on<Event>()`? If yes, why?
15. Is default interaction behavior implemented by the component rather than reconstructed by client code?
