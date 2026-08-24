# Layout Refactor — Current Status

> Branch: `fix/sharp-logical-text`
>
> This file is a current implementation status companion to `LAYOUT_REFACTOR_CHECKPOINT.md`. It intentionally does not replace the architectural checkpoint or the deferred `TextPrimitive` review.

## Settled architecture

```text
Client
  ├─ mutates state
  └─ UIManager::invalidateLayout(node)

Component semantic/interactive method
  └─ protected Node::invalidateLayout() when its operation changes layout state

NodeTree
  ├─ tree ownership / lifecycle
  ├─ structural mutation queue
  └─ root-based coalesced layout queue

LayoutSystem
  ├─ Measure traversal
  ├─ Arrange traversal
  ├─ framework-known constraints
  ├─ border/content conversion
  ├─ Absolute positioning policy
  └─ final geometry resolution

Component / PanelNode
  ├─ custom Measure policy
  ├─ custom Arrange policy
  └─ custom Draw policy
```

## Current implementation status

### DONE / settled

- `Node::measure(const MeasureContext&)` and `Node::arrange(const ArrangeContext&)` are the production extension points.
- `measureContent()` remains the default leaf measurement primitive.
- `PanelNode` is a structural capability; it does not impose a layout algorithm.
- `StackPanelNode` delegates flow policy to `LinearLayout`.
- `LinearLayout` performs only linear measure/arrange policy; it does not own tree traversal or scheduling.
- `LayoutSystem` owns framework execution and framework-known constraint semantics.
- Absolute positioning remains framework-owned policy.
- Border-box/content-box conversion remains framework-owned.
- `Auto`, fixed, min/max, and final-size semantics are centralized in framework constraints/resolution.
- `Overflow` remains rendering/clipping state, not Measure/Arrange state.
- Layout invalidation is asynchronous, root-based, and coalesced.
- Re-invalidation during Measure/Arrange is deferred to a later batch rather than recursively re-entering layout.
- Structural mutation during framework phases is deferred and processed after the protected traversal scope.
- `UIManager::invalidateLayout(Node&)` is the public client contract.
- `Node::invalidateLayout()` is a protected component-side hook for semantic/interactive operations that change layout-relevant state.
- Base `Node` setters do not perform hidden layout scheduling.
- The legacy `deferLayoutMutation` path has been removed from the migrated code path; `TabItem` was the last confirmed production use found during this audit.
- `NodeTree` no longer owns `TextPrimitive`.
- The old monolithic layout-engine responsibilities are now assigned to explicit owners: `NodeTree`, `LayoutSystem`, `layout_constraints`, `LinearLayout`, and component `Measure/Arrange` policy.

## Component invalidation rule

The settled rule is based on layout consequences, not on whether an operation is interactive:

```text
pure setter
    → mutation only
    → client is responsible for explicit invalidation

semantic component operation
    → component may call protected invalidateLayout()
      when it changes layout-relevant state

presentation-only interaction
    → no layout invalidation

structural child mutation
    → NodeTree/framework owns invalidation
```

Examples:

```text
Dropdown::open/close/clearSelection  → component invalidates
Checkbox::toggle                      → no invalidation
Slider value changes                  → no invalidation
Button press/hover animation          → no invalidation
```

## Important recent correctness fixes

### Absolute Auto resolution

Absolute children now resolve width and height independently. A fixed width does not force an Auto height to use the measurement proposal, and vice versa.

```text
fixed + fixed → fixed allocation
fixed + auto  → fixed width + measured height
auto + fixed  → measured width + fixed height
auto + auto   → measured desired size
```

### Dropdown menu measurement

`Dropdown` no longer forces menu height from the cached `getDesiredSize()` of a hidden menu. Menu width is constrained from the trigger while height remains Auto so the next Measure pass computes intrinsic height.

## Legacy engine inventory

| Former responsibility | Current owner |
|---|---|
| Tree traversal | `NodeTree` |
| Layout queue | `NodeTree` |
| Mutation queue | `NodeTree` |
| Measure traversal | `LayoutSystem` |
| Arrange traversal | `LayoutSystem` |
| Proposal constraints | `layout_constraints` |
| Final constraints | `layout_constraints` / `LayoutSystem` |
| Border/content conversion | `LayoutSystem` |
| Root/viewport sizing | `LayoutSystem` |
| Absolute placement | `LayoutSystem` |
| Linear flow measure | `LinearLayout` |
| Linear flow arrange | `LinearLayout` |
| Custom leaf measurement | `Node::measureContent()` / component |
| Custom child layout policy | `Node::measure/arrange()` on component |
| Child ownership | `PanelNode` / `NodeTree` |
| Rendering traversal/clipping | `NodeTree` |
| Text measurement/rendering internals | `TextPrimitive` / `TextNode` — intentionally deferred |

## Remaining work

1. User-side build and runtime validation.
2. Focused custom layout acceptance coverage for Measure/Arrange + invalidation + constraints.
3. Final review of component behavior revealed by runtime validation.
4. Only after the main layout refactor is complete: focused `TextPrimitive` / `TextNode` semantic review.
5. Final cleanup of obsolete text/runtime plumbing after the text review, if still applicable.

## Explicitly deferred

`TextPrimitive` and `TextRuntime` are deliberately not redesigned or removed during the current layout migration. They may disappear or be reshaped after the framework is fully validated on the open Measure/Arrange model.

Build and runtime verification are intentionally external to this checkpoint work and should be performed on the developer's environment.
