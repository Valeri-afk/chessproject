# Scroll Architecture

Scroll is a framework-level behavior/infrastructure concern. A standalone public `Scroll` / `ScrollArea` component is not currently required.

## Current implementation status

The source already contains `ScrollManager` and `UIManager` integration.

Current source-level responsibilities include:

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

The implementation is source-level only until full build and runtime validation are performed.

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

The stored layout geometry therefore remains stable while scrolling changes the effective coordinate used by rendering and input.

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

The scroll viewport must use the framework's existing layout box model.

The current implementation derives the usable client size by subtracting positive border and padding from the node's actual size.

Do not introduce a separate Scroll-specific box model.

Conceptually:

```text
actual border box
      ↓ subtract border
padding area
      ↓ account for padding/content origin
scroll viewport/content relationship
```

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

The implementation allows remaining wheel delta to propagate to outer scroll containers when an inner scroll container reaches its limit.

If no scroll container consumes the wheel event, normal input processing continues.

Pointer events are converted by SDL to the renderer/logical coordinate space before they enter the framework input pipeline. Scroll presentation is then applied separately through the framework coordinate transform.

## Rendering and clipping

Scroll should reuse the existing NodeTree clipping semantics based on `Overflow::HIDDEN` rather than creating a second clipping architecture.

The current `UIManager` applies the scroll coordinate transform during the render traversal.

The remaining validation work is to verify that the resulting transform and the existing clipping/hit-test traversal produce correct behavior for nested scroll containers and clipped content.

## Hit testing

Input traversal runs under the same scroll coordinate transform used by the framework input path.

After a wheel operation changes scroll offset, `InputManager::refreshHover()` re-evaluates the node under the current pointer position using the same transformed coordinate space. This refresh generates only the required hover transitions; it does not synthesize a mouse-move event or alter pointer capture/drag state.

The validation target is therefore:

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

`ScrollManager::sync()` currently derives content extent from the actual geometry of visible descendants while respecting nested registered scroll containers.

This should be validated against the framework's layout semantics before being considered final.

If a future component requires virtualization or a custom content extent, that requirement should be added deliberately rather than turning `ScrollManager` into an arbitrary measurement engine.

## Scrollbar presentation

Scrollbar visuals are intentionally not part of the current Scroll core.

A scrollbar should only be added after behavior is runtime-validated and a concrete reusable visual contract is established.

A scroll container must remain useful without a visible scrollbar.

## Public component decision

Do not create a standalone `Scroll` / `ScrollArea` component merely because `ScrollManager` exists.

First validate the framework behavior.

Only introduce a public component if repeated application-level usage demonstrates that a component API provides a real reusable abstraction beyond the service/infrastructure already available.

## Remaining work

```text
source integration review
    ↓
full build
    ↓
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
