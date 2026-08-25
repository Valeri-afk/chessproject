# Event Dispatching

## One event mechanism

The framework uses one public registration mechanism on `Node`:

```cpp
node->on<ConcreteEvent>(callback);
```

`events.hpp` is the single public event-type header. Dispatcher and handler storage remain private.

## Concrete event types

The template parameter selects the event type at registration time:

```cpp
launcher->on<ui::MouseClickEvent>(
    [](ui::MouseClickEvent& event, ui::Node& node) {
        if (event.button == ui::MouseButton::Left) {
            // custom behavior
        }
    });
```

The callback already receives the concrete event. Client code does not need to inspect a generic event and cast it.

Current event families include mouse movement, button press/release/click, enter/leave, wheel, key down and key up.

## Framework flow

```text
SDL input
  ↓
InputSystem
  ↓
hit test / focus / capture / modality
  ↓
concrete framework event
  ↓
EventDispatcher
  ↓
registered Node handlers
```

Target selection and propagation are framework responsibilities.

## Component vs client handlers

The same event mechanism supports:

```text
component internal behavior
client custom behavior
```

Component-internal handlers are appropriate when they implement the component's own semantics.

Client handlers are the customization boundary. Client code does not need to know which low-level events a component uses internally.

## Semantic callbacks

Components may expose semantic callbacks such as:

```text
Button      → activated
Checkbox    → toggled
RadioButton → selected
Slider      → value changed
Menu        → item activated
Tabs        → selection changed
```

These are convenience APIs above the event mechanism, not a second dispatcher.

## Registration lifetime

Event registration belongs to the Node/component runtime lifetime. Handler storage and registration identities remain framework-owned. Public code should use the Node registration API rather than accessing handler storage directly.

## Constructor caution

A component constructor may register internal handlers when that is part of its runtime behavior, but construction must not accidentally simulate interaction. Registering a handler should describe future event handling; it should not immediately trigger semantic actions such as activation or modal presentation.

## Non-goals

```text
second event bus
manual NodeTree event traversal
client access to handler storage
generic runtime event casting
component-specific global dispatch systems
```
