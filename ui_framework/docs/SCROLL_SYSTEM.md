# Scroll System

## Role

Scrolling is framework-level infrastructure. A standalone public `Scroll` / `ScrollArea` component is not currently required.

A scroll container is a `PanelNode`. A plain `Node` cannot be registered as scrollable because scrolling requires descendants/content geometry.

## Ownership

The scroll service owns:

```text
scroll offset
content extent
maximum offset
clamping
ancestor accumulated offsets
wheel routing
layout-derived content extent
```

`UIManager` owns/routes the service and applies the scroll coordinate transform during traversal.

## Coordinate model

Scrolling never rewrites stored layout positions.

```text
retained layout geometry
        ↓
accumulated scroll offset
        ↓
effective render/input coordinate
```

Nested scroll containers accumulate ancestor offsets.

ScrollSystem never needs physical window pixels or SDL renderer coordinates. It operates entirely in framework layout coordinates.

## Viewport semantics

There are two different viewport concepts:

```text
framework viewport
    = global logical UI coordinate area supplied by LayoutSystem

scroll viewport
    = the scroll container's current content-box geometry
```

The global framework viewport is derived from the renderer's logical presentation. Physical window resolution and logical presentation configuration remain client/SDL presentation concerns.

A scroll container does not receive a viewport size from the client and does not keep a duplicate viewport value in `ScrollState`. Its viewport is derived from the node's current actual size, padding, and border.

## State

`ScrollState` contains only mutable/derived scroll data:

```text
content extent
offset
```

The viewport and maximum offset are derived from current node geometry:

```text
maxOffsetX = max(0, content.width - viewport.width)
maxOffsetY = max(0, content.height - viewport.height)
```

Offsets are clamped to `[0, maxOffset]`.

## Content extent

Content extent is derived from committed layout geometry. Visible descendants contribute their actual bounds. A nested registered scroll container contributes its own viewport bounds to its parent content calculation; its internal scroll content does not expand the outer container's content extent.

Client code changes scroll offset through `UIManager`; it does not maintain a second content/viewport size model.

## Box model

The scroll viewport uses the existing Node box model. The effective viewport is the node's content box after subtracting padding and border from its actual size.

There is no separate Scroll-specific box model.

## Wheel routing

Wheel input is normalized by `UIManager` before entering framework coordinates. `UIManager` gives the wheel position to `ScrollSystem`, which performs hit testing and starts with the nearest registered scroll ancestor.

```text
SDL wheel
   ↓
UIManager
   ↓
ScrollSystem
   ↓
hit-test target
   ↓
nearest scrollable ancestor
   ↓
consume available delta
   ↓
clamp
   ↓
remaining delta
   ↓
next scrollable ancestor
```

A scroll container consumes only the part of the wheel delta that it can apply. If it is already at a limit, the remaining delta can continue to an outer scroll container. This gives nested scrolling chaining without introducing a second input-dispatch system.

If a wheel event is consumed by scrolling, `UIManager` refreshes hover at the same pointer coordinates after the offset changes. Pointer capture and drag state are not synthesized or reset by scrolling.

## Rendering and clipping

Scrolling is a coordinate transform. Stored layout geometry remains unchanged.

Clipping is a separate concern from scrolling. The intended Node-level semantic is `clipToBounds`: when enabled, the node's rendered subtree is constrained to its own bounds. Scroll containers require an effective viewport clip, but scrolling itself does not mean that arbitrary nodes are scrollable.

The existing `Overflow::HIDDEN` implementation in `NodeTree` still represents the current clipping mechanism. Migration to the explicit `clipToBounds` property requires the corresponding `NodeTree` clipping checks to be updated together; `node_tree.cpp` is intentionally not changed automatically.

## Scrollbars

Scrollbar visuals are intentionally outside the current scroll core. A scrollbar should only be added after behavior is runtime validated and a concrete reusable visual contract is established.

## Public API decision

Do not create a public Scroll component merely because a scroll service exists. Add one only when repeated application usage demonstrates a reusable semantic abstraction beyond the service/infrastructure.

## Validation

Dedicated regression coverage should cover:

```text
scroll-container registration
layout-derived content extent
clamping
programmatic offset changes
wheel behavior
nested scroll chaining
modal-restricted hit testing
hit testing after scroll
hover transitions after scroll
scroll/input coordinate consistency
mutation/removal
```
