# Layout System

## Purpose

This document is the current contract for framework layout execution. The framework uses an imperative, retained-mode Measure → Arrange pipeline with explicit invalidation.

## Ownership boundary

```text
Framework
  → Measure/Arrange execution
  → constraints
  → layout scheduling
  → geometry commit
  → traversal

Component
  → component-specific Measure/Arrange policy
  → component-specific layout state

Client
  → explicit notification when semantic state makes derived layout stale
```

`Node` has Measure and Arrange semantics regardless of whether it owns children. `PanelNode` is the structural capability that can own framework-visible children. `StackPanelNode` is a specialized `PanelNode` with a predefined linear policy.

## Measure

Measure answers what size the component desires under the effective content-space proposal supplied by the framework.

```text
parent border-box proposal
    ↓
framework size/min/max/padding/border resolution
    ↓
effective content-box proposal
    ↓
component Measure
    ↓
desired content size
    ↓
framework box composition
    ↓
desired border-box size
```

Measure constraints are available bounds, not promises of final allocation. A component may report a desired size larger than the supplied available size.

Leaf `Node` measures its own content. `PanelNode` may recursively measure children and aggregate their desired sizes.

## Arrange

Arrange uses the final content geometry selected by the parent layout policy.

```text
parent allocation
    ↓
framework final constraints
    ↓
component Arrange
    ↓
actual internal geometry
    ↓
framework committed actualPosition / actualSize
```

`Measure proposal != Arrange allocation` is a core invariant.

## Constraint semantics

Current semantics are:

```text
Fixed size
  → measurement proposal + final size

Max size
  → measurement proposal + final size

Min size
  → final size only

Auto
  → intrinsic measurement / parent allocation
```

A minimum does not automatically narrow intrinsic measurement. A maximum may narrow the Measure proposal before width-sensitive content is measured.

`Auto` is not "fill parent". Its meaning depends on the surrounding layout policy.

## Border-box model

Node outer geometry remains a border box. Measure/Arrange component hooks operate in content-space terms; padding and border are converted by framework layout code.

`Overflow::HIDDEN` is not a layout constraint. It is a rendering/input boundary.

## Linear layout

The current minimal flow model supports orientation, gap, main-axis alignment and cross-axis alignment, visibility filtering and absolute-child separation.

Absolute children do not contribute to normal-flow aggregation.

Stretch allocates available cross-axis space and final min/max constraints are then applied. There is no flex-shrink/flex-basis/flex-wrap/grid implementation in this contract.

## Invalidation

The current public invalidation contract is:

```cpp
uiManager.invalidateLayout(node);
```

Invalidation is explicit and asynchronous. It does not run Measure/Arrange immediately and there is no public flush operation.

The framework does not observe arbitrary component fields.

```text
semantic state mutation
    ↓
explicit invalidateLayout()
    ↓
root promotion + queue deduplication
    ↓
next framework layout phase
```

Repeated invalidations of the same root are coalesced. Detached/non-live nodes are not accepted as layout jobs.

Invalidation of a descendant is promoted to the containing top-level root/overlay because the whole affected subtree must be measured under the root's constraints.

## Re-invalidation during layout

If Measure or Arrange invalidates a node, the current pass is not restarted recursively. The currently queued roots are consumed first; a new invalidation queues work for a later framework-controlled pass.

A component that invalidates itself on every Measure/Arrange call can therefore cause repeated future passes. That is component behavior, not recursive scheduler behavior.

## Derived geometry validity

`getDesiredSize()` and `getActualSize()` expose the latest committed layout values. They are cached derived state, not live computations.

After invalidation and before the next layout pass, old geometry remains readable but may be stale.

No separate geometry-validity bit is required at the current stage.

## Structural interaction

`PanelNode::addChild/removeChild` already participate in framework mutation handling and layout consequences. A separate public `treeStructureChanged()` notification is not required for ordinary structural child operations.

## No paint invalidation yet

Rendering runs every frame, so there is currently no separate `invalidatePaint()` queue. A paint-dirty subsystem should only be introduced if the renderer later requires explicit scheduling.

## Acceptance cases

Important acceptance cases include:

- fixed width text;
- text + button + gap;
- padding/border conversion;
- fixed child width smaller than parent;
- min/max without feeding min into Measure;
- parent width changing wrapped text;
- main/cross-axis centering;
- cross-axis stretch with final constraints;
- absolute children excluded from flow;
- nested panels;
- hidden children excluded from layout;
- fixed parent height with overflowing content;
- maxWidth affecting measurement before wrapping.

## Intentionally deferred

```text
flex-grow
flex-shrink
flex-basis
flex-wrap
order
margin
Grid track sizing
multi-pass intrinsic track resolution
content-dependent stretch remeasurement
```
