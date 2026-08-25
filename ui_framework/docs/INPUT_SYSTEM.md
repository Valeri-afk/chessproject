# Input System

## Role

Input is framework runtime infrastructure. Client code and components consume the resulting framework events; they do not reimplement global routing, hit testing, capture or focus.

## Main responsibilities

The input system owns:

```text
SDL event ingestion
pointer coordinate normalization
hit testing
hover state
pressed/drag state
pointer capture
focus
keyboard routing
modal-root filtering
wheel routing to scroll infrastructure
```

## Coordinate normalization

The chess client currently uses a fixed logical UI space:

```text
1920 × 1080
```

SDL logical presentation is configured with:

```cpp
SDL_LOGICAL_PRESENTATION_LETTERBOX
```

Physical window dimensions may differ. Pointer coordinates are converted into renderer/logical coordinates before framework hit testing and event dispatch.

Scroll transforms are applied separately from the base logical coordinate conversion.

## Hit testing

Hit testing uses current retained tree state and effective transformed coordinates. It is not backed by a separate mandatory cache at the current stage.

Therefore there is no public `rebuildHitTest()` operation.

## Hover

Pointer movement updates the hovered target and generates the corresponding enter/leave behavior.

After a handled scroll operation, hover may be refreshed using the new transformed coordinates. This refresh should not synthesize a mouse-move event or disturb pointer capture/drag state.

## Pointer interaction

The input state vocabulary includes:

```text
pressed node
captured node
focused node
dragging state
```

Pointer capture is framework-owned so controls can continue receiving the relevant interaction even when the pointer leaves their normal hit-test region.

## Focus

Focus is framework state. Keyboard routing uses the focused node where appropriate.

Components expose focusability/capturability through Node-level properties; the input system owns the actual focus/capture transitions.

## Modal filtering

When a modal root is active, input routing is restricted according to the modality contract. Underlying application nodes must not accidentally receive input through normal hit testing.

## Wheel

Wheel input is routed through the framework scroll system rather than being treated as ordinary pointer clicks.

Remaining wheel delta can propagate through nested scroll containers.

## Event generation

The input system translates SDL input into framework event types such as:

```text
MouseMoveEvent
MouseDownEvent
MouseUpEvent
MouseClickEvent
MouseEnterEvent
MouseLeaveEvent
MouseWheelEvent
KeyDownEvent
KeyUpEvent
```

The exact event payload contract lives in `events.hpp`; dispatching is a separate concern described by the Event Dispatching document.

## Validation boundary

Visual rendering validation has passed. Dedicated input validation remains separate and should cover:

```text
MouseDown → MouseUp → MouseClick sequencing
MouseEnter / MouseLeave
pointer capture
focus
keyboard routing
dragging
modal restriction
overlay hit testing
scroll + hit-test interaction
```

## Non-goals

Do not add a second client-side input routing system or expose internal input phase controls through the public API.
