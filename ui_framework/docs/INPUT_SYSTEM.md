# Input System

## Role

Input is framework runtime infrastructure. Client code and components consume framework events; they do not reimplement global routing, hit testing, capture, focus or scroll routing.

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

The framework consumes input in its UI coordinate space. The chess client currently configures a fixed logical presentation of:

```text
1920 × 1080
```

using:

```cpp
SDL_LOGICAL_PRESENTATION_LETTERBOX
```

Physical window dimensions may differ. Pointer coordinates are converted into the renderer/framework logical coordinate space before framework hit testing and dispatch.

The distinction between physical window resolution and logical UI resolution is a presentation concern. Framework nodes operate in framework coordinates and do not need physical pixel dimensions.

Scroll transforms are applied separately from the base logical coordinate conversion.

## Hit testing and clipping

Hit testing uses current retained tree geometry and effective transformed coordinates.

`clipToBounds` is a Node-level clipping contract. When enabled, hit testing must respect the same bounds that constrain the rendered subtree. Scrolling changes effective coordinates but does not rewrite retained layout positions.

There is no public `rebuildHitTest()` operation.

## Hover

Pointer movement updates the hovered target and generates the corresponding enter/leave behavior.

After a handled scroll operation, hover is refreshed using the same physical pointer position and the new effective transformed coordinates. This must not synthesize a mouse-move event or disturb pointer capture/drag state.

## Pointer interaction

The input state vocabulary includes:

```text
pressed node
captured node
focused node
dragging state
```

Pointer capture is framework-owned so controls can continue receiving the relevant interaction even when the pointer leaves their normal hit-test region.

When a modal opens, incompatible pointer capture outside the new modal boundary is cancelled so the previous interaction cannot leak through the modal.

## Focus

Focus is framework state. Keyboard routing uses the focused node where appropriate.

Components expose focusability/capturability through Node-level properties; the input system owns actual focus/capture transitions.

When a modal opens, modality establishes the focus scope. The modal itself receives focus when focusable; otherwise the first focusable descendant is selected. `Tab` traversal wraps within the active modal subtree.

## Modal filtering

When a modal root is active, input routing is restricted to the active modal subtree and its configured outside-click behavior.

Lower modals and underlying application nodes may continue framework updates, but they cannot receive restricted pointer or keyboard input through normal dispatch.

Escape is routed to the focused component first. If it does not consume the event, modality applies the active modal's Escape policy.

## Wheel

Wheel input is routed through the framework scroll service rather than being treated as ordinary pointer clicks.

The nearest registered scroll ancestor receives the available delta first. If it reaches a scroll boundary, remaining delta may propagate to an outer scroll container.

A handled scroll updates effective hit-test coordinates and hover state without changing retained layout geometry.

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

The current regression suites cover the major input boundaries through focused and integration scenarios:

```text
MouseDown → MouseUp → MouseClick sequencing
MouseEnter / MouseLeave
pointer capture
focus
keyboard routing
dragging
modal restriction and keyboard isolation
focus trap / Tab traversal
outside-click policy
scroll + clipping + hit-test interaction
hover after scroll
nested scroll wheel chaining
```

Visual rendering validation remains separate from input correctness validation.

## Non-goals

Do not add a second client-side input routing system or expose internal input phase controls through the public API.
