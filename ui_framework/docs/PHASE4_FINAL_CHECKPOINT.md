# Phase 4 — Final Source Checkpoint

## Position

Branch:

```text
phase4-rendering
```

`main` is unchanged by the Phase 4 work tracked here.

This is a **source-level checkpoint only**. Compilation, automated tests and runtime validation remain deferred until Phase 6.

---

## A — Geometry → Rendering

**Status: implemented / settled.**

Final arranged geometry is:

```text
actualPosition_
actualSize_
```

Rendering consumes this final geometry. `PositionMode::Absolute` remains parent-content-relative and is not changed in Phase 4.

---

## B — Render Traversal

**Status: implemented / settled.**

Canonical top-level paint order:

```text
roots → overlays → top modal
```

Children paint in structural forward order. Hit-testing uses reverse effective paint order and recursive subtree traversal. Update follows `roots → overlays`.

No global `zIndex` or stacking-context system is introduced.

The previously duplicated `NodeTree::hitTest()` implementation has been removed. Only the recursive `hitTestSubtree()`-based implementation remains.

`Overflow::HIDDEN` in hit-testing is now treated as an ancestor clipping boundary rather than as a requirement that the parent itself must be the hit target.

---

## C — Renderer State / Clipping

**Status: implemented / settled at current scope.**

`RendererStateScope` provides per-subtree renderer-state isolation. Current scope covers render target, viewport, scale, clip, draw color, blend mode and color scale.

`Overflow::HIDDEN` uses nested rectangular clip intersection and state restoration through RAII.

No dirty-region renderer, generic renderer abstraction or transform-aware clip system is added in Phase 4.

---

## D — Node State / Geometry Semantics

**Status: contract settled; broad property API expansion intentionally deferred.**

Canonical property semantics are classified across layout, hit-test and rendering.

Important decisions:

```text
opacity
    → rendering only

borderWidth
    → layout-affecting

borderRadius
    → visual geometry; future canonical hit-test/clip geometry

overflow
    → subtree clip / interaction boundary; not a layout-size change

future transform
    → same canonical visual-geometry class as borderRadius

future scroll
    → cross-subsystem viewport/content feature, not Overflow::SCROLL
```

`Node::hitTest()` remains virtual as an extension point for genuinely custom geometry. Standard framework-defined geometry must have one canonical interpretation rather than being reimplemented separately by component authors.

---

## E — SDL3 Render Backend

**Status: implemented / settled.**

SDL3 remains the sole concrete backend.

Application owns SDL runtime lifecycle; framework uses SDL types and rendering APIs. No second backend, `IRenderBackend`, `BackendFactory` or premature `RenderContext` is introduced.

`Node::draw(SDL_Renderer*)` remains valid.

---

## F — Resource Boundary

**Status: implemented / settled at current scope.**

Borrowed/external:

```text
SDL_Renderer*
TTF_Font*
```

Framework-owned renderer-bound text cache:

```text
TTF_TextEngine*
TTF_Text*
```

No generic resource manager is introduced.

Semantic resources are separated conceptually from backend-bound representations. The current text path uses the SDL3 renderer text engine. Alternative SDL_ttf engines are not part of the current framework backend contract.

Persistent texture caching/offscreen resource systems remain deferred until concrete requirements exist.

---

## G — Animation Boundary

**Status: contract settled; no animation system implemented.**

Animation is a state-transition mechanism:

```text
Animation
    ↓
property/state change
    ↓
normal framework semantics
```

Animation does not own rendering, clipping, hit-test or layout semantics. Layout-affecting animated properties use existing layout semantics; visual-only properties remain visual-only.

No `AnimationManager`, timeline system, easing API or mandatory animation subsystem inside `Node` is introduced.

---

## Final Phase 4 conclusion

The source-level Phase 4 rendering/backend architecture is now defined and reconciled against the current branch.

The concrete Phase 4 rendering implementation already present is primarily:

```text
NodeTree rendering traversal
    +
RendererStateScope
    +
nested clipping
    +
mutation-safe traversal
    +
SDL3 backend usage
```

No additional broad architecture is justified at this point.

### Explicitly deferred

```text
ResourceManager
AnimationManager
IRenderBackend / second backend
RenderContext without concrete pressure
zIndex / stacking contexts
borderRadius implementation as a new public property
transform subsystem
scroll subsystem
persistent text-to-texture cache
full offscreen rendering/resource pipeline
```

### Validation gate

Do **not** compile, run tests or perform runtime validation before Phase 6.

The next Phase 4 action is only needed if a new concrete source requirement or contradiction appears. Otherwise this checkpoint is the handoff point for the next phase/review.
