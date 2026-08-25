# Rendering System

## Role

Rendering is framework runtime infrastructure below the component layer.

```text
Node/component
   ↓
component visual state
   ↓
rendering primitives / specialized backend
   ↓
SDL renderer
```

Components draw their own visual content. `NodeTree` owns render traversal, ordering, clipping and traversal safety.

## SDL logical presentation

The chess client uses a fixed logical UI space:

```text
1920 × 1080
```

The renderer is configured with:

```cpp
SDL_SetRenderLogicalPresentation(
    renderer,
    1920,
    1080,
    SDL_LOGICAL_PRESENTATION_LETTERBOX);
```

The physical window/display may have a different size. SDL fits the logical content while preserving aspect ratio and letterboxes the remaining dimension using the clear color.

This logical coordinate system is the basis for component/layout sizing and makes UI dimensions independent of the physical display resolution.

## Renderer state isolation

Rendering code should preserve and restore renderer state when applying temporary state such as clipping. The current subtree clipping path uses RAII-style state scopes.

## Overflow and clipping

`Overflow::HIDDEN` is a rendering traversal concern, not a Measure/Arrange constraint.

Conceptually:

```text
current parent clip
    ∩
node clip
    ↓
draw node
    ↓
draw descendants
    ↓
restore previous renderer state
```

`PanelNode` does not own recursive rendering traversal. `NodeTree` does.

## Scroll transform

Scrolling does not rewrite retained layout geometry. The scroll system supplies an accumulated offset that is applied as an effective coordinate transform during rendering and input traversal.

```text
stored layout position
    ↓
scroll transform
    ↓
effective render position
```

## Rendering primitives

Low-level primitives remain deliberately narrow and below component semantics. They centralize reusable immediate SDL drawing algorithms such as rectangles, rounded rectangles, lines, circles and similar geometric operations.

A primitive must not know about Node lifecycle, component state, input, focus, modality or application semantics.

Do not turn the primitive module into a general resource/graphics system merely because SDL can support a feature.

## Text rendering

Text rendering is a dedicated internal backend:

```text
TextLayout
    ↓
TextRenderer
    ↓
SDL_ttf
```

`TextRenderer` owns physical rasterization, derived font resources, SDL text objects and renderer-state details. Logical wrapping, desired size and alignment policy remain above it.

## Rendering cadence

Rendering currently executes every frame. Therefore there is no separate public paint invalidation queue.

Render-only state may change without forcing Measure/Arrange. A dedicated `invalidatePaint()` API should only be introduced if the rendering architecture later requires explicit dirty scheduling.

## Validation

Current visual/runtime validation has covered:

```text
Typography variants
text alignment
wrapped text
text-bearing controls
button variants
menus/dropdowns
tabs
stack/scroll presentation
modal/backdrop presentation
1920×1080 logical presentation
resizable/fullscreen letterboxing
```

Dedicated input/modal correctness remains a separate validation pass.
