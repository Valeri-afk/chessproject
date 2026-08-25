# Scroll Architecture

Scroll is a framework-level behavior/infrastructure concern. A standalone public `Scroll` / `ScrollArea` component is not currently required.

## Current implementation status

The source contains `ScrollManager` and `UIManager` integration, and the chess client builds and runs with the current rendering/layout stack.

Current responsibilities include:

```text
ScrollManager
├── viewport extent
├── content extent
├── offset
├── maximum offset
├── clamping
├── accumulated ancestor offsets
├── wheel routing
└── layout-derived content extent

UIManager
├── owns ScrollManager
├── routes wheel input to ScrollManager
├── applies accumulated scroll coordinate transform
└── synchronizes scroll state during frame preparation
```

The basic application/rendering integration is working. Dedicated behavioral validation of scrolling itself remains separate work.

## Coordinate model

Scroll must not rewrite the original layout positions of descendants.

The intended relationship is:

```text
layout position
      ↓
accumulated scroll offset
      ↓
effective render/input position
```

For nested scroll containers, ancestor offsets accumulate.

Stored layout geometry remains stable while scrolling changes the effective coordinate used by rendering and input.

## Range model

For a scroll container:

```text
maxOffsetX = max(0, content.width  - viewport.width)
maxOffsetY = max(0, content.height - viewport.height)
```

Offsets are clamped to:

```text
0 ≤ offsetX ≤ maxOffsetX
0 ≤ offsetY ≤ maxOffsetY
```

Viewport and content extent are derived from the framework's layout geometry during `ScrollManager::sync()`. They are not configured through a second public viewport/content-size API. The public scroll state that client code may change directly is the scroll offset.

## Border and padding

The scroll viewport uses the framework's existing layout box model.

The usable client size is derived by accounting for the node's border and padding. Do not introduce a separate Scroll-specific box model.

## Input

Wheel routing is framework-level.

Current flow:

```text
SDL mouse wheel
      ↓
UIManager
      ↓
ScrollManager::handleWheel()
      ↓
hit-test target
      ↓
nearest scrollable ancestor(s)
      ↓
apply available delta
      ↓
clamp
```

Remaining wheel delta may propagate to outer scroll containers when an inner container reaches its limit.

Pointer events are converted to renderer/logical coordinates before entering the framework input pipeline. Scroll presentation is applied separately through the framework coordinate transform.

## Rendering and clipping

Scroll reuses the existing NodeTree clipping semantics based on `Overflow::HIDDEN` rather than introducing a second clipping architecture.

`UIManager` applies the scroll coordinate transform during render traversal.

Dedicated validation still needs to confirm clipped content, nested scrolling and the exact interaction between transformed coordinates and hit testing.

## Hit testing

Input traversal runs under the same scroll coordinate transform used by the framework input path.

After a wheel operation changes scroll offset, `InputManager::refreshHover()` re-evaluates the node under the current pointer position using the same transformed coordinate space. This refresh generates only the required hover transitions; it does not synthesize a mouse-move event or alter pointer capture/drag state.

Validation target:

```text
pointer coordinates
      ↓
SDL render/logical coordinate conversion
      ↓
scroll transform
      ↓
existing NodeTree hit-test
      ↓
viewport clipping
      ↓
hover transition / event target
```

## Content extent

`ScrollManager::sync()` derives content extent from the actual geometry of visible descendants while respecting nested registered scroll containers.

This behavior should be covered by the dedicated scroll validation pass.

## Scrollbar presentation

Scrollbar visuals are intentionally not part of the current Scroll core.

A scrollbar should only be added after behavior is runtime-validated and a concrete reusable visual contract is established.

A scroll container must remain useful without a visible scrollbar.

## Public component decision

Do not create a standalone `Scroll` / `ScrollArea` component merely because `ScrollManager` exists.

Only introduce a public component if repeated application-level usage demonstrates that a component API provides a real reusable abstraction beyond the service/infrastructure already available.

## Remaining work

```text
runtime wheel tests
    ↓
render/clipping validation
    ↓
hit-test / hover validation
    ↓
nested scroll validation
    ↓
only then decide on scrollbar/public Scroll component
```
