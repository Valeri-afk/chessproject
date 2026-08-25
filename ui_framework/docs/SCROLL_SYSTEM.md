# Scroll System

## Role

Scrolling is framework-level infrastructure. A standalone public `Scroll` / `ScrollArea` component is not currently required.

## Ownership

The scroll service owns:

```text
viewport extent
content extent
offset
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

## Range

For each scrollable container:

```text
maxOffsetX = max(0, content.width - viewport.width)
maxOffsetY = max(0, content.height - viewport.height)
```

Offsets are clamped to `[0, maxOffset]`.

Viewport/content extents are derived from layout geometry during scroll synchronization. Client code directly changes scroll offset; it does not maintain a second content/viewport size model.

## Box model

Scroll uses the framework's existing border/padding model. There is no separate Scroll-specific box model.

## Wheel routing

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
apply available delta
   ↓
clamp
   ↓
remaining delta may propagate outward
```

Nested scrolling therefore supports chaining when an inner container reaches its limit.

## Input and hover

Pointer coordinates are normalized into renderer/logical coordinates before entering the framework input path. Scroll transforms are applied separately.

After scrolling changes the offset, hover is refreshed against the transformed coordinate space so the framework can produce the necessary enter/leave transitions without synthesizing a mouse-move event or changing pointer capture/drag state.

## Rendering and clipping

Scroll reuses existing `Overflow::HIDDEN` / NodeTree clipping semantics. It does not introduce a second clipping architecture.

The scroll offset is applied as a coordinate transform while stored layout geometry remains unchanged.

## Content extent

Scroll synchronization derives content extent from actual visible descendant geometry while respecting nested registered scroll containers.

This is a derived framework value, not a client-maintained property.

## Scrollbars

Scrollbar visuals are intentionally outside the current scroll core. A scrollbar should only be added after behavior is runtime validated and a concrete reusable visual contract is established.

## Public API decision

Do not create a public Scroll component merely because a scroll service exists. Add one only when repeated application usage demonstrates a reusable semantic abstraction beyond the service/infrastructure.

## Validation

Dedicated validation should cover:

```text
wheel behavior
clamping
content extent
Overflow clipping
nested scrolling
hit testing after scroll
hover transitions after scroll
scroll/input coordinate consistency
```
