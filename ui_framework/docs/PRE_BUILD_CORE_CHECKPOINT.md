# Pre-Build Core Checkpoint

This document freezes the source/documentation state immediately before the first real build/runtime validation pass.

It is a validation checkpoint, not a new architecture proposal. It does not replace `ARCHITECTURE.md` and must not be used to justify a wholesale rewrite of that document.

Repository:

```text
Valeri-afk/ui-framework
```

Current branch:

```text
main
```

## 1. Core status

The planned core/runtime work for the current development boundary is implemented at source level.

Current status:

```text
Framework runtime            ✓
Layout                       ✓
Input / events               ✓
Rendering / SDL3             ✓
Standard components           ✓
ModalManager                 ✓
ScrollManager                ✓
SDL logical viewport          ✓
SDL render/logical input     ✓
Scroll coordinate transform  ✓
Wheel routing/chaining       ✓
Overflow::HIDDEN clipping    ✓
Scroll hit-test integration  ✓
Hover refresh after scroll   ✓
```

This is a source-level completion statement. It is not a claim that the framework has already been successfully compiled or runtime-tested.

## 2. Current standard components

```text
Button
ToggleButton
Menu
MenuItem
TabControl
TabItem
Checkbox
RadioButton
Slider
Dropdown
```

Deferred framework capabilities remain:

```text
TextField / Input
Image / resource infrastructure
optional Overlay / popup infrastructure
```

Framework-level behavior that is intentionally not represented as standalone components:

```text
Modal      → ModalManager service
Scroll     → ScrollManager framework behavior
```

A standalone `Scroll` / `ScrollArea` component and scrollbar visuals remain deferred until the existing behavior proves its reusable contract.

## 3. Source contracts frozen for validation

### UIManager

The public event entry point is:

```cpp
void processEvent(
    const SDL_Event &sdlEvent,
    SDL_Renderer *renderer);
```

The renderer is explicit because input events must be normalized to SDL renderer/logical coordinates before entering the framework input pipeline.

`UIManager::setViewportSize()` is not part of the current public API.

### LayoutManager

The framework viewport is derived from SDL logical presentation when configured, with renderer output size as fallback.

Layout geometry remains independent from scrolling.

### ScrollManager

`ScrollManager` owns:

```text
scroll offset
range/clamping
ancestor offset accumulation
wheel routing
nested residual-delta chaining
layout-derived viewport/content extent
scroll state cleanup
```

The current public Scroll API changes offset/state only; viewport and content extent are derived from layout.

### Node / NodeTree

Scroll does not rewrite logical/layout positions.

Presentation transforms are scoped to render/hit-test operations. Layout/state synchronization must occur without an active scroll transform.

Existing `Overflow::HIDDEN` clipping remains the clipping mechanism for scroll content.

### InputManager

`InputManager` owns hover/focus/capture state.

`refreshHover()` exists specifically to reconcile hover after a scroll offset change without synthesizing a mouse-move event or starting drag/capture behavior.

## 4. Coordinate pipeline

The current input/render model is:

```text
SDL window/input coordinates
        ↓
SDL_ConvertEventToRenderCoordinates()
        ↓
framework logical/render coordinates
        ↓
scroll presentation transform where required
        ↓
NodeTree hit-test / render traversal
```

The layout model is:

```text
layout geometry
        ↓
stable actual position/size
        ↓
scroll transform only for presentation/input
```

## 5. Scroll hover ordering

When a wheel event is consumed by scrolling:

```text
1. enter scroll transform
2. ScrollManager::handleWheel()
3. leave scroll transform
4. prepareForTreeOperation()
5. enter a fresh scroll transform
6. InputManager::refreshHover()
7. leave scroll transform
```

This ordering is intentional. `ScrollManager::sync()` must never observe presentation-transformed positions as layout geometry.

## 6. Documentation state

The living documentation has been aligned to the current source boundary:

```text
ROADMAP.md                         ✓
FRAMEWORK_SCOPE.md                 ✓
SCROLL_ARCHITECTURE.md             ✓
PHASE6_SCOPE_CANDIDATES.md         ✓
PHASE5_SOURCE_AUDIT.md             ✓
PHASE5_COMPONENT_CATALOG.md       ✓
PHASE5_COMPONENT_ARCHITECTURE...  ✓
PHASE6_CORE_STATUS_CHECKPOINT.md   ✓
NEXT_CHAT_SCROLL_HANDOFF.md        ✓
```

`ARCHITECTURE.md` remains separately maintained. It has been reviewed for obvious source/history mismatches, but has intentionally not been mechanically rewritten. Any future edits there must be deliberate and targeted.

## 7. Validation work not yet claimed

The following are intentionally **not** marked complete in this checkpoint:

```text
actual full compilation
runtime smoke tests
automated interaction tests
modal runtime tests
scroll runtime tests
memory/lifetime runtime checks
```

These require the real build/runtime environment and are the next stage after this checkpoint.

## 8. Current development rule

Until runtime validation produces evidence to the contrary:

```text
Do not add another large core subsystem.
Do not add a public Scroll/ScrollArea component.
Do not add scrollbar visuals.
Do not reintroduce client-driven viewport synchronization.
Do not rewrite layout positions to implement scrolling.
Do not create a second clipping system.
Do not expand the component catalog speculatively.
```

The next phase is validation and defect correction based on actual build/runtime evidence.

## 9. Handoff

For continuation, read this checkpoint together with:

```text
NEXT_CHAT_SCROLL_HANDOFF.md
PHASE6_CORE_STATUS_CHECKPOINT.md
PHASE6_SCOPE_CANDIDATES.md
SCROLL_ARCHITECTURE.md
PHASE5_SOURCE_AUDIT.md
FRAMEWORK_SCOPE.md
ROADMAP.md
ARCHITECTURE.md
```

Treat `main` as the only current development line and verify repository state before making changes.
