# Phase 6 — Core Development Status Checkpoint

This document records the current framework-core development point and is the continuation map for the next work.

## 1. Current development point

```text
Phase 6 — Framework Core / Subsystem Development
        |
        v
Validation / Stabilization boundary
```

Phase 5 is complete as a component-development phase.

The framework now has working core foundations for:

- layout
- input/focus/capture routing
- modality
- scrolling
- logical text layout/rendering
- the current standard component set

The latest framework/client build has been validated successfully after recovery of the full `NodeTree` implementation and small include/API corrections.

## 2. Phase 5 result

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

Deferred/not standard at this point:

```text
TextField / Input    → text-input infrastructure still required
Image                → minimal resource/texture infrastructure still required
Scroll               → behavior is core infrastructure; public Scroll / ScrollArea remains deferred
Modal                → service infrastructure exists; standalone public component not required yet
Paper                → not a framework component
Card                 → client-side composition/style pattern
```

The old `Widget` / `ControlNode` direction is not part of the active architecture.

## 3. Phase 6 subsystem status

### Modality — implemented core infrastructure

`ModalManager` provides:

```text
modal registration
modal stack/order
modal input boundary
exclusive hit-testing for the active modal
focus restriction
pointer capture restriction
Escape routing
outside-click interception
backdrop interaction policy
backdrop visual layer
backdrop fade state
backdrop lifecycle
nested modal focus restoration
modal invalidation/removal cleanup
deferred-mutation-safe backdrop ownership
```

Current backdrop policy:

```text
BackdropClickBehavior
    Consume
    Close
```

The backdrop is an internal framework overlay node, not a public standard component.

### Scrolling — implemented core behavior

`ScrollManager` provides:

```text
scroll state
viewport/content extent relationship
scroll offset/range and clamping
SDL wheel routing
nested scroll chaining with residual delta
presentation coordinate transform
hit-testing through transformed content
Overflow::HIDDEN clipping integration
layout-derived viewport/content extent
nested scroll container boundaries
scroll-state lifecycle cleanup
```

Ownership remains:

```text
ScrollManager → scroll state and input routing
Node / UIManager → presentation coordinate transform
NodeTree → traversal, clipping and hit-test integration
LayoutManager → layout geometry; scroll does not mutate layout positions
```

Layout positions remain unchanged by scrolling. Scroll offset is separate state.

Reference: `SCROLL_ARCHITECTURE.md`.

### Text — implementation complete; runtime validation next

The text architecture is now:

```text
Typography / text-bearing controls
              |
              v
         TextContent
          /       \
         v         v
    TextLayout   TextRenderer
         |            |
 TextLayoutResult   SDL_ttf
```

Important decisions:

- `TextPrimitive` is removed from the architecture.
- `TextNode` / `TextRuntime` are removed from the active text architecture/build graph.
- `TextLayout` owns logical measurement and wrapping.
- `TextRenderer` is internal and renderer-only.
- Text-bearing controls own `TextContent` directly.
- Components do not expose SDL_ttf or renderer-cache state.
- The source `TTF_Font*` remains client-owned; derived text resources are framework-owned.

Reference: `TEXT_ARCHITECTURE_CHECKPOINT.md` and `TEXT_RESOURCE_LIFETIME.md`.

### Text input / editing — not implemented

Still requires a proper reusable contract for:

```text
text input events
composition / IME
caret
selection
editing commands
clipboard
keyboard navigation
repeat/backspace/delete
focus integration
text-input lifecycle
```

### Image / resource infrastructure — not implemented

Still requires a minimal reusable texture/resource ownership contract. Do not create a broad ResourceManager without concrete requirements.

### Generic overlay / popup infrastructure — deferred

Do not add a second overlay architecture merely because `Dropdown` exists. Introduce one only if concrete requirements demonstrate the need for root-level placement, clipping escape, global ordering, outside-click coordination across unrelated subtrees, or collision policy.

## 4. Recovery / build-validation checkpoint

The current recovery branch is:

```text
recovery-before-node-tree-break
```

A successful framework/client build has been reached after restoring the full `node_tree.cpp` implementation and correcting the small `Node` constness / `MeasureContext` callback mismatch encountered during validation.

Recovery rule:

- Do not replace the full `node_tree.cpp` implementation with a simplified/truncated version.
- Do not automate changes to `node_tree.cpp` or `input_system.cpp` unless the exact intended diff is known.
- Small compiler fixes should be made directly when they are local and unambiguous.

## 5. What remains before declaring the current core fully validated

```text
1. Runtime smoke tests
2. Typography/text rendering tests
3. Component interaction tests
4. NodeTree/input/layout/render integration tests
5. Modal interaction tests
6. Scroll interaction tests
7. Memory/lifetime checks
8. Source/include consistency review
9. Final documentation checkpoint
```

The next implementation priority is **not** another architecture rewrite. It is validation and stabilization of what is already implemented.

## 6. After core validation

Once the current core is stable:

```text
Stop adding framework capabilities temporarily.
Document the validated core boundary.
Then implement only the next infrastructure that has a concrete reusable requirement:

1. Text input / editing
2. Minimal image/resource ownership
3. Overlay/popup infrastructure only if evidence requires it
```

A public Scroll / ScrollArea component and scrollbar visuals remain optional and should not be added before the core scroll behavior is validated in runtime use.

## 7. Working rule

```text
Prefer small local fixes over broad refactors.
If a compiler error is small and unambiguous, fix it locally.
If a proposed change touches NodeTree architecture, stop and verify first.
Do not expand the component catalog without a concrete reusable need.
Do not add application-specific chess functionality while framework core is being stabilized.
```

## 8. Immediate next step

```text
Runtime validation / stabilization of the currently working framework.
```

Start with a small smoke test that exercises:

```text
NodeTree
  -> layout
  -> text measurement/rendering
  -> input dispatch
  -> focus/capture
  -> scrolling
  -> modality
```

Then record the actual runtime results here before beginning the next infrastructure phase.
