# UI Framework Roadmap

## Status

This is a **global checklist**, not a strict sequence of implementation phases.
Items may be worked on independently when dependencies and current implementation state make that appropriate.

The source code remains authoritative. The focused subsystem documents describe the current contracts for each area.

## Core completion checklist

### Scroll system

- [x] Complete and validate the scroll system behavior.
- [x] Validate nested scrolling and remaining wheel-delta propagation.
- [x] Validate scroll transforms together with clipping and hit testing.
- [x] Decide whether the current service-oriented scroll model is sufficient or whether a reusable public scroll component is justified.

Reference: `SCROLL_SYSTEM.md`

### Text input

- [ ] Implement editable text fields/input controls.
- [ ] Integrate editing with the existing `InputSystem` focus and keyboard routing.
- [ ] Build editing on top of `TextLayout` and `TextRenderer` without moving editing semantics into `Node` or the renderer.
- [ ] Determine the minimum required caret, selection, clipboard and IME contracts from concrete use cases.

Reference: `TEXT_INPUT_SYSTEM.md`

### Modality

- [x] Complete remaining modal behavior validation.
- [x] Validate modal open/close sequencing.
- [x] Validate focus restoration and fallback focus selection.
- [x] Validate backdrop behavior and modal interaction boundaries.
- [x] Validate nested modal sessions where supported.
- [x] Reassess whether a standalone public `Modal` component is ever necessary; current architecture uses `ModalSystem` as framework infrastructure.

Reference: `MODALITY_SYSTEM.md`

### Text architecture boundary

- [ ] Consider whether `TextLayout` can be hidden further behind `TextContent` / public text components without weakening the current architecture.
- [ ] Keep logical text measurement/wrapping separate from backend SDL_ttf rendering.
- [ ] Do not expose implementation types merely for convenience; expose them only when a concrete reusable contract requires them.

Reference: `TEXT_SYSTEM.md`

### Base and standard components

- [ ] Finish and validate the base/standard component set required by the target application.
- [ ] Verify component event callback usage against the existing Node event-registration model.
- [ ] Avoid introducing component-specific callback mechanisms that bypass framework event registration unless a concrete requirement justifies them.
- [ ] Keep component-specific semantics in components while framework-wide lifecycle, traversal, layout, input and rendering remain framework-owned.

Reference: `COMPONENT_DESIGN.md`, `EVENT_DISPATCHING.md`

## Layout and geometry

### Border-box model

- [x] Complete and stabilize the border-box model across the framework.
- [x] Verify content-box ↔ border-box conversions for padding and border.
- [x] Verify desired size, arranged size and rendered geometry stay consistent.

Reference: `LAYOUT_SYSTEM.md`

### Clipping

- [x] Complete clipping semantics and validation.
- [x] Verify nested clipping intersections.
- [x] Verify clipping consistency between rendering and hit testing.
- [x] Verify clipping together with scrolling transforms.

Reference: `RENDERING_SYSTEM.md`, `INPUT_SYSTEM.md`, `SCROLL_SYSTEM.md`

### Absolute positioning

- [x] Verify `PositionMode::Absolute` behavior in all relevant container/layout scenarios.
- [x] Verify absolute children are excluded from normal linear flow where intended.
- [x] Verify their final geometry and interaction boundaries are correct.

Reference: `LAYOUT_SYSTEM.md`

### LayoutValue type

- [x] Review the current `LayoutValue` model and verify all supported value types have clear semantics.
- [x] Verify fixed / auto / min / max-related behavior against the final border-box and Measure/Arrange contracts.
- [x] Remove or simplify any value variants that no longer represent a meaningful framework contract.

Reference: `LAYOUT_SYSTEM.md`

## NodeTree and component structure

### NodeTree ownership reference

- [x] Reconsider whether `Node::owner_` is still necessary in the final architecture: keep it as the node's runtime membership and mutation-routing boundary.
- [x] Preserve ownership/liveness invariants with `owner_` rather than replacing the mechanism.

This is an architectural investigation, not a commitment to remove `owner_`.

Reference: `LIFETIME_AND_MUTATIONS.md`, `ARCHITECTURE.md`

### PanelNode / structural mutation boundary

- [x] Review the current `PanelNode` structural API and its relationship with `NodeTree`.
- [x] Use `PanelNode::addChild/removeChild` as the supported client/component mutation API; mounted nodes route mutations through `NodeTree` automatically.
- [x] No explicit `notifyStructureChanged()` mechanism is required because mounted structural mutation already enters the authoritative `NodeTree` path.
- [x] Custom components can attach/remove nested children through the existing `PanelNode` API without bypassing framework ownership and mutation safety.
- [x] Preserve ownership, registration, lifecycle and deferred-mutation invariants across these operations.

Reference: `COMPONENT_DESIGN.md`, `LIFETIME_AND_MUTATIONS.md`, `DEFERRED_OPERATIONS.md`, `ARCHITECTURE.md`

## Cross-system validation

- [x] Validate input/event behavior after structural changes.
- [x] Validate modal restrictions together with focus, capture and overlays.
- [x] Validate scroll behavior together with clipping and hit testing.
- [x] Validate layout invalidation followed by the next layout pass.
- [ ] Validate rendering after geometry changes and logical-presentation resize.

## Architectural guardrails

- [x] Keep `UIManager` as the public framework facade where that remains appropriate.
- [x] Keep `NodeTree` as the runtime authority for ownership, liveness, structural mutation and coordinated traversal.
- [x] Do not introduce a universal property/dependency system without a concrete requirement.
- [x] Do not introduce parallel input, event or rendering orchestration systems in components/client code.
- [x] Avoid unnecessary framework abstractions until a repeated reusable requirement proves their value.

## Large-file safety

`node_tree.cpp` and `input_system.cpp` are large implementation files and should not be partially rewritten by automated edits. Changes requiring edits to those files should be performed manually and verified against the current source before being committed.
