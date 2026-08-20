# Handoff — Current Framework Core Validation Boundary

This document is a continuation checkpoint for a new chat. It is not a replacement for `ARCHITECTURE.md` and must not be used as a reason to rewrite the architecture document wholesale.

Repository:

```text
Valeri-afk/ui-framework
```

Current branch:

```text
main
```

## 1. Current development boundary

Phase 5 component development is complete as a focused component phase.

Phase 6 core infrastructure discovered during that work is implemented at source level and is now at the **validation/stabilization boundary**.

The next development step is not to add another large subsystem. It is to verify that the existing source, public APIs and documentation are internally consistent and then validate the framework through the real build/runtime environment.

The source tree remains the authoritative source of truth for current behavior.

## 2. Current standard components

The active standard component set is:

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

Deferred or non-component concepts remain:

```text
TextField / Input    → pending text-input/editing infrastructure
Image                → pending resource/texture ownership infrastructure
Scroll / ScrollArea  → framework behavior exists; standalone component deferred
Modal                → framework service exists; standalone component deferred
Paper                → composition/style pattern
Label                → covered by text primitives/text nodes
Card                 → client-side composition/style pattern
```

## 3. Modality status

`ModalManager` is the framework-level modality service.

The service owns the reusable modality contract, including modal stack/order, modal-root input restrictions, focus/capture policy, Escape routing, outside/backdrop interaction, backdrop lifecycle/presentation and restoration/cleanup behavior.

A standalone public `Modal` component is intentionally deferred.

## 4. Viewport model

The SDL renderer's logical presentation size is the framework UI coordinate space when logical presentation is configured.

The framework must obtain the current logical presentation from SDL rather than requiring a client-driven `UIManager::setViewportSize()` source of truth.

Fallback when logical presentation is unavailable:

```text
current render-output size
```

The corresponding viewport synchronization is owned internally by `LayoutManager`.

## 5. Scroll status

Scroll is implemented as framework-level behavior/infrastructure rather than a visual `Scroll`/`ScrollArea` widget.

Current core responsibilities:

```text
ScrollManager
├── scroll state
├── viewport/content relationship derived from layout
├── offset
├── maximum offset
├── clamping
├── accumulated ancestor offsets
├── wheel routing
├── nested residual-delta chaining
└── state cleanup

UIManager / Node
├── presentation coordinate transform
└── integration with render/input traversal

NodeTree
├── traversal
├── clipping through existing Overflow::HIDDEN
└── hit-testing

LayoutManager
└── layout geometry; scroll does not rewrite layout positions
```

The core invariant is:

```text
layout position
    !=
scrolled presentation position
```

Scroll offset is stored separately from layout geometry.

Nested scroll presentation uses the accumulated offsets of relevant scroll ancestors. A scroll node's own offset moves its content, not the node itself.

## 6. Scroll input and coordinate model

Pointer input is normalized into SDL renderer/logical coordinates before entering the framework input pipeline.

The intended flow is:

```text
SDL window/input coordinates
        ↓
SDL render/logical coordinate conversion
        ↓
UIManager / InputManager
        ↓
scroll presentation transform where required
        ↓
NodeTree hit-test / rendering
```

Wheel routing uses the hit-test target and walks the node ancestry. A nested scroll container gets first opportunity to consume the wheel delta. Any residual delta that cannot be consumed because the inner container is at a boundary continues to the next scroll ancestor.

## 7. Hover after scrolling

`InputManager` owns hover state.

After a handled wheel operation changes scroll offset, the framework:

```text
1. leaves the scroll transform used for wheel routing;
2. performs normal tree/state synchronization without a scroll transform;
3. enters a fresh scroll presentation transform;
4. calls InputManager::refreshHover();
5. leaves the transform again.
```

`refreshHover()` performs only the required hover transition:

```text
hit-test
    ↓
compare previous/new hovered node
    ↓
MouseLeave if needed
    ↓
MouseEnter if needed
    ↓
store hovered node
```

It must not synthesize a `MouseMoveEvent`, start drag processing, alter pointer capture or introduce a second hover subsystem.

## 8. Current source-level decisions

The following decisions are already implemented and should not be reverted without concrete evidence:

```text
UIManager::setViewportSize()      → removed as client-driven source of truth
Scroll viewport/content setters  → removed from current public Scroll API
Scroll offset                    → explicit mutable scroll state
Overflow::HIDDEN                 → existing clipping mechanism
layout positions                 → stable under scroll
Modal                             → service-level infrastructure
Scroll                            → framework-level behavior first
```

Do not reintroduce client-driven viewport synchronization or a standalone Scroll component merely for symmetry.

## 9. Current validation boundary

The repository should now be treated as:

```text
core implementation
        ↓
source/documentation consistency
        ↓
validation / stabilization
        ↓
full build + runtime verification in the real environment
```

The remaining validation concerns include:

```text
source/include consistency
public API consistency
render/input/layout integration
Scroll clipping and nested hit-testing
hover transitions after scroll
Modal interaction behavior
lifetime/mutation safety
```

Do not expand Scroll with scrollbar visuals or gesture physics before the existing behavior is validated.

Do not add another large infrastructure layer without a concrete framework-level responsibility.

## 10. Documentation set for continuation

Use these documents as current context:

```text
PHASE6_CORE_STATUS_CHECKPOINT.md
PHASE6_SCOPE_CANDIDATES.md
SCROLL_ARCHITECTURE.md
PHASE5_SOURCE_AUDIT.md
COMPONENT_DESIGN_GUIDE.md
PHASE6_MODALITY_REQUIREMENTS.md
PRIMITIVES_ROLE.md
ROADMAP.md
FRAMEWORK_SCOPE.md
ARCHITECTURE.md
```

`ARCHITECTURE.md` remains the large architectural document and should receive only deliberate, targeted updates after a concrete architectural decision has been established.

## 11. Final target for the next continuation

The current target is no longer "implement Scroll". It is:

```text
source truth aligned with documentation     ✓
Scroll framework behavior implemented       ✓
Modal framework behavior implemented        ✓
SDL logical viewport contract               ✓
SDL input coordinate conversion             ✓
scroll wheel chaining                       ✓
scroll presentation transform               ✓
Overflow::HIDDEN integration                ✓
hit-test integration                        ✓
hover refresh after scroll                  ✓

next:
final source/documentation consistency review
then real validation/build/runtime
```

The key rule for future continuation is:

> Treat `main` as the single current development line. Verify the actual repository state before modifying code, preserve the existing framework-level Scroll/Modal contracts, and prefer validation and evidence over adding new infrastructure.
