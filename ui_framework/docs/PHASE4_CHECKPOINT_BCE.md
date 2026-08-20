# Phase 4 Checkpoint — A / B / C / D / E / F / G

## Status

This checkpoint records the agreed source-level architecture after reviewing all Phase 4 rendering/backend discussion points A through G. It is intended to restore context without repeating the Phase 1–3 audit.

Current branch:

```text
phase4-rendering
```

`main` must remain unchanged until Phase 4 is complete.

Compilation, automated tests and runtime validation remain deferred until Phase 6.

---

## A — Geometry → Rendering Boundary

**Source-level architecture accepted; no code change required at this checkpoint.**

```text
position_ / requested layout state
        ↓
measurement
        ↓
desiredSize_
        ↓
arrange
        ↓
actualPosition_ / actualSize_
        ↓
rendering
        ↓
toSDLRect()
        ↓
SDL coordinates
```

`position_` is layout input, `desiredSize_` is the measurement result, and `actualPosition_` / `actualSize_` are final arranged geometry consumed by rendering.

`PositionMode::Absolute` remains parent-content-relative and is not screen-space positioning. This is existing Phase 2 semantics and is not changed for rendering convenience.

No transform matrices, coordinate-space wrapper types, or layout redesign are introduced by A.

---

## B — Render Traversal

**Source-level architecture accepted / no implementation expansion required at this checkpoint.**

Top-level priority:

```text
roots        → forward order
overlays     → forward order, excluding top modal
top modal    → last
```

`roots_` and `overlays_` contain the same `Node` type; they are separate top-level priority containers, not separate component classes.

Within a subtree, children render in structural forward order. Hit-testing follows the reverse effective paint order. Update follows `roots → overlays`.

No explicit `zIndex`, global paint priority, or stacking-context abstraction is introduced. Structural order plus top-level priority layers are sufficient for current scope.

---

## C — Renderer State / Clipping

**Source-level complete.**

`RendererStateScope` isolates/restores render target, viewport, scale, clip state, draw color, blend mode and color scale. `NodeTree::drawSubtree()` scopes node subtrees with RAII.

`Overflow::HIDDEN` creates a clip from final node geometry and intersects it with the active parent clip. Zero-area intersections skip the subtree. Renderer state is restored on scope exit.

C does not expand into dirty regions, partial redraw, generic backend abstraction, animation infrastructure or transform-aware clipping without a concrete requirement.

Runtime/compile validation remains deferred to Phase 6.

---

## D — Node State / Geometry / Visual Semantics

**Architecture accepted at source level. No new visual-property implementation is required merely to close D.**

Node properties are classified by impact:

```text
Node property
   ├── Layout / geometry
   ├── Hit-test / interaction
   └── Rendering / visual output
```

A framework-defined property must have one canonical framework interpretation. `virtual Node::hitTest()` is an extension point for genuinely custom geometry, not an invitation to reimplement standard framework semantics differently.

### Property classes

Layout-affecting:

```text
position
size
auto/fixed sizing inputs
padding
borderWidth
```

Interaction + rendering/clipping:

```text
visibility
overflow
```

Visual-only:

```text
opacity
colors / appearance values
```

`opacity` does not change layout or hit-testing by itself. An opacity-zero node may remain geometrically present and interactive; `visible`/`enabled` and other input semantics control interaction separately.

Visual-geometry:

```text
borderRadius
future transform / translation / scale / rotation
```

These do not change the layout box but alter effective visual shape/geometry and therefore belong to the same canonical-geometry problem class:

```text
layout geometry
    ↓
visual geometry
    ├── rendering shape
    ├── hit-test shape
    └── clip shape where applicable
```

`borderRadius` keeps the rectangular layout box but, once defined by the framework, should use canonical rounded geometry for standard hit-testing rather than requiring each component author to duplicate the math.

Parent clipping is framework-owned subtree semantics. A child does not need to know its parent’s clip. Future rounded clipping and transforms must be handled centrally.

Future transform handling should follow the same model as `borderRadius`: transform the effective geometry for rendering, hit-testing and clipping, with inverse transformation on input as appropriate.

Scroll is not treated as a trivial `Overflow::SCROLL` enum. It is a future cross-subsystem viewport/content feature involving content extent, viewport geometry, scroll offset, clipping, hit-test coordinate conversion and input behavior.

---

## E — Render Backend

**Architecture accepted; no backend abstraction expansion required at this checkpoint.**

The framework is intentionally **SDL3-only**.

Current dependency delivery:

```text
ui_framework/
    external/
        SDL3/
        SDL3_image/
        SDL3_ttf/
```

The framework supplies those dependencies. The application owns runtime lifecycle:

```text
Application
    → SDL_Init / SDL_Quit
    → SDL_Window lifetime
    → SDL_Renderer lifetime
    → renderer creation/destruction

UI Framework
    → uses SDL3 APIs
    → receives required SDL handles/context
    → performs rendering orchestration
    → does not own externally-created SDL runtime objects
```

Current SDL-facing hooks remain valid:

```cpp
Node::draw(SDL_Renderer*)
UIManager::runFrame(dt, SDL_Renderer*)
NodeTree::draw(renderer, ...)
```

No second backend, `IRenderBackend`, `BackendFactory`, generic renderer interface or premature `RenderContext` is introduced.

---

## F — Resource Boundary

**Source-level architecture accepted / F closed for the current framework scope.**

F is defined around ownership and backend representation, not around a generic resource-management subsystem.

### Ownership categories

Borrowed/external resources:

```text
SDL_Renderer*
TTF_Font*
```

The framework uses these but does not own their runtime lifecycle. `TextNode::setFont()` accepts a borrowed `TTF_Font*` and `TextNode` does not destroy it.

Framework-owned renderer-bound cache:

```text
TTF_TextEngine*
TTF_Text*
```

`TextNode` creates and destroys these objects itself. They are bound to the current `SDL_Renderer*` and are recreated when the renderer changes.

### Resource representation

No generic `ResourceManager`, `TextureManager`, `FontManager`, `AssetManager`, `IResource` hierarchy or public opaque resource-handle system is introduced at this stage.

The useful abstraction is semantic rather than managerial:

```text
semantic resource
    ↓
backend-bound representation
    ↓
SDL3
```

For text, the canonical current path is:

```text
TextNode semantic state
    ↓
TTF_Text
    ↓
renderer-bound SDL_ttf text engine
    ↓
SDL_Renderer
```

The framework does not need to support every SDL3/SDL3_ttf rendering path merely because SDL exposes them. Other text engines are not part of the current framework contract.

Direct drawing and rendering into an SDL texture that is the current renderer target can use the same renderer text path; this does not require a second framework text abstraction.

A future persistent `SDL_Texture` or offscreen-render target pipeline is a separate renderer-bound resource problem and requires a concrete requirement before being introduced.

### Core rule

```text
Dependency delivery ≠ resource ownership
semantic resource ≠ backend representation
```

---

## G — Animation Boundary

### Status

**Architecture accepted; no animation subsystem is implemented in Phase 4.**

G establishes the boundary between temporal state change and the systems that consume the resulting state.

### Core principle

Animation is a **state transition mechanism**, not a rendering mechanism.

```text
Animation
    ↓
changes framework/component state
    ↓
existing property semantics
    ├── Layout
    ├── Hit-test
    └── Rendering
```

An animation must not directly own or bypass rendering semantics:

```text
Animation
    ✕ SDL rendering calls
    ✕ renderer state
    ✕ clipping policy
```

### Visual-only animation

Properties such as future `opacity`, appearance values, `borderRadius` and future visual transforms may be animated without implying layout changes.

Example:

```text
opacity 1.0 → 0.0
```

changes rendering state only. It does not implicitly disable hit-testing.

### Layout-affecting animation

If an animation changes a layout-affecting property such as position, size, padding or border width, the animation does not implement a second layout system. It changes the relevant property and the property's normal layout/invalidation semantics apply:

```text
Animation
    ↓
layout-affecting property
    ↓
existing layout invalidation
    ↓
measure / arrange
    ↓
actual geometry
    ↓
render
```

### Animation ownership

No `AnimationManager`, timeline manager or mandatory animation list inside `Node` is introduced.

The absence of a central animation subsystem does **not** mean that every animation must live inside `Node`. Ownership remains with the layer that owns the behavior/resource, according to the future concrete requirement.

A component may implement local temporal behavior where appropriate. A future framework-wide animation subsystem may also be introduced if multiple concrete requirements justify one. Phase 4 does not choose or implement that subsystem.

The important invariant is that animation changes state through the framework's established property semantics instead of bypassing them.

### Runtime position

`UIManager::runFrame(dt, renderer)` already has `update(dt)` before rendering, giving future animation a natural update point. The exact ordering between animation updates and component `Node::update()` is intentionally not fixed until a concrete animation subsystem exists.

### Explicitly not implemented by G

- AnimationManager;
- generic timeline system;
- easing library/API;
- keyframe system;
- automatic tweening API;
- animation-specific renderer API.

---

## Final Phase 4 architectural result: A–G

```text
                        APPLICATION
                             │
                  owns SDL runtime lifecycle
                             │
                             ▼
                           SDL3
                             │
                      SDL_Renderer*
                             │
                             ▼
                     ┌───────────────┐
                     │ UI Framework  │
                     ├───────────────┤
                     │ Geometry      │
                     │ Traversal     │
                     │ Hit-test      │
                     │ Clipping      │
                     │ Visual state  │
                     │ Resources     │
                     │ Rendering     │
                     └───────┬───────┘
                             │
                 backend-bound representations
                             │
                             ▼
                           SDL3
```

Top-level processing priority:

```text
roots → overlays → top modal for final presentation
```

State semantics:

```text
Layout state
    ↓
actual geometry
    ↓
visual/geometry semantics
    ├── render
    ├── hit-test
    └── clip
```

Animation semantics:

```text
Animation
    ↓
state change
    ↓
normal framework semantics
```

Resource semantics:

```text
borrowed external resources
        +
framework-owned renderer-bound caches
        ↓
canonical SDL3 rendering path
```

### What Phase 4 intentionally does NOT introduce

```text
ResourceManager
TextureManager
FontManager
AnimationManager
IRenderBackend
BackendFactory
second rendering backend
premature RenderContext
persistent text texture cache
full offscreen resource system
zIndex / stacking contexts
scroll subsystem
transform subsystem
```

These are not rejected as universally bad designs. They are deferred because the current framework has no concrete requirement that justifies their added semantics and ownership complexity.

### Next step

The architectural review A–G is complete.

The next task is **implementation reconciliation**:

1. Compare the agreed A–G contract against the actual `phase4-rendering` source.
2. Mark each contract as `implemented`, `partially implemented`, `contract-only`, or `missing`.
3. Identify the minimal implementation scope required to bring source into alignment.
4. Only then modify code.
5. Do not run compilation/tests/runtime validation until Phase 6.
6. Do not modify `main`.
