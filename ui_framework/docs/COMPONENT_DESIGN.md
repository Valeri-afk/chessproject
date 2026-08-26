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
semantic actions
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

A component is responsible for implementing its own default interaction semantics. The client should not have to reconstruct those semantics with low-level input handlers merely to make a standard component work.

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

### State categories

Component state should be understood as three categories:

```text
Persistent semantic state
    state that remains meaningful after an interaction
    examples: checked, value, selected item, committed text

Interaction state
    temporary state used by the component's interaction/presentation
    examples: pressed, hovered, dragging, pointer-selecting, focused

Events
    transient facts describing a transition or occurrence
    examples: activated, toggled, value-changed, text-changed, mouse-enter
```

An event and a state are not interchangeable. For example, `MouseEnterEvent` means that an enter transition occurred, while `isHovered()` describes the current interaction state.

Expose a state getter when the current value is a useful part of the component contract. Do not create state getters for transient events such as `clicked` merely because an equivalent event exists.

### Ownership of interactive state

The standard component owns the invariants and default transitions of its own interactive state.

For example:

```text
Checkbox
    MouseClickEvent
        ↓
    component toggles checked
        ↓
    ToggledEvent
```

The client is allowed to set component state explicitly through its public API, but it is not required to implement the component's normal interaction behavior itself.

Therefore this is not the intended way to implement the default Checkbox behavior:

```cpp
checkbox->on<MouseClickEvent>([](auto &, Node &node) {
    auto &checkbox = static_cast<Checkbox &>(node);
    checkbox.setChecked(!checkbox.isChecked());
});
```

A client may still subscribe to low-level input events when a concrete application needs that information, but normal component use should prefer the component's semantic state and semantic events.

Controlled/external-authoritative component state is not a general framework model at present. Do not introduce a React-style controlled/uncontrolled state system unless a concrete reusable game/client requirement justifies it.

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
TextInput
Image
StackPanelNode / PanelNode
```

`Image` is intentionally a thin visual component. It references an externally owned `SDL_Texture`; it does not own texture lifetime or implement asset loading/caching. Its contract covers intrinsic size, tint and a small set of presentation fit modes (`STRETCH`, `CONTAIN`, `COVER`).

`Paper`, `Label` and `Card` remain composition/styling patterns rather than mandatory framework components.

## Event registration and ownership

There is one event-handler registration mechanism:

```cpp
node->on<ConcreteEvent>(callback);
```

`on()` is used both by client code and by component implementations. Components do not need a second registration primitive with different semantics.

Component-internal handlers are implementation details. They may listen to low-level framework input events in order to implement the component's default behavior. A helper such as `handleMouseDown()` or `handleKeyDown()` may be used as a private implementation function, but it is not a second event system and is not part of the public component API.

The conceptual roles are:

```text
Event
    message/fact

on<Event>()
    register an event handler on a Node

Input event
    external/runtime input delivered by InputSystem

Semantic component event
    component-level notification emitted after a meaningful state/action transition
```

The client should normally consume the component through semantic state and semantic component events rather than reimplementing the component with low-level input events.

### Input events vs semantic component events

InputSystem knows about framework input/lifecycle events such as:

```text
MouseDownEvent
MouseUpEvent
MouseMoveEvent
MouseWheelEvent
KeyDownEvent
KeyUpEvent
TextInputEvent
TextEditingEvent
FocusGainedEvent
FocusLostEvent
```

InputSystem should not need to know every component-specific semantic event.

A component may create and deliver its own semantic events locally when its state/action changes. Such events do not need to travel back through InputSystem merely because their original cause was an input event.

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
    ↓
client handlers registered with on<TextChangedEvent>()
```

Component-specific semantic event types should live with the component that owns their meaning when they are not shared framework infrastructure. General input/lifecycle events remain in the common events header.

### Semantic events instead of component-specific callback setters

Components should not introduce parallel `setOn...` callback APIs when the generic event mechanism already expresses the same relationship.

Prefer:

```cpp
textInput->on<TextChangedEvent>(callback);
button->on<ActivatedEvent>(callback);
checkbox->on<ToggledEvent>(callback);
slider->on<ValueChangedEvent>(callback);
```

over:

```cpp
setOnTextChanged(...)
setOnActivate(...)
setOnToggle(...)
setOnValueChanged(...)
```

Semantic events are delivered through the same Node event mechanism as all other events. Adding a new semantic event must not require changes to InputSystem unless the event is itself an external input/lifecycle event.

### Multiple handlers and propagation

Multiple handlers for the same concrete event type on the same Node are valid and must not conflict merely because they share the same event type.

Each registered handler is an independent listener. `stopPropagation()` controls propagation through the Node tree; it does not mean "cancel the other handlers registered on this same target Node".

Dispatch should use a stable handler snapshot for the current delivery so that handler registration/removal during a callback does not invalidate iteration or partially corrupt the current dispatch.

The event registration mechanism therefore supports both:

```text
component-internal handlers
client handlers
```

without requiring separate dispatch systems.

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

## Presentation and client responsibility

A component owns its presentation based on its own semantic and interaction state plus its configurable visual properties.

For example:

```text
Button
    pressed + hovered + enabled + configured colors/variant
        ↓
    component presentation
```

The client should configure component properties and react to semantic events. It should not need to mirror `pressed`, `hovered`, `checked`, `selected` or similar state into separate application-side UI state merely to keep the standard presentation correct.

Client code may intentionally change component properties or semantic state in response to application logic. That does not transfer ownership of the component's invariants or default interaction behavior to the client.

## Implementation style

Keep component code small and semantic. A component normally:

```text
stores local state
exposes semantic properties/actions
participates in Measure/Arrange/Draw
uses the existing event API
coordinates explicitly owned specialized children
implements its standard interaction behavior
emits semantic events after meaningful transitions
```

It should not:

```text
reimplement NodeTree
reimplement hit-testing
reimplement global event dispatch
reimplement generic layout engines
manage global modality/scroll state
expose backend caches
require the client to implement standard component interaction
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
11. Which low-level input events are consumed internally by the component?
12. Which semantic state is public to the client?
13. Which semantic events should the component emit?
14. Does the component need any public callback API beyond `on<Event>()`? If yes, why is the generic event system insufficient?
15. Is default interaction behavior implemented by the component rather than reconstructed by client code?
