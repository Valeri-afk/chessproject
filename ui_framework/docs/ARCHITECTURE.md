# Architecture

This document describes the current architecture of the UI framework as implemented in the source code. The source code is authoritative.

## 1. Architectural Overview

The runtime is centralized around `NodeTree`, with `UIManager` as the public facade.

```text
                         UIManager
                             |
          +------------------+------------------+
          |                  |                  |
      NodeTree          InputSystem      LayoutSystem
          |                  |                  |
          |                  v                  v
          |            EventDispatcher    StackPanelNode
          |
          +------ Node
          +------ PanelNode
          +------ lifecycle / ownership / mutation
          +------ traversal / update / layout queue
          +------ hit-testing / rendering
          +------ roots / overlays
          |
      ModalSystem
          |
          +------ NodeTree
          +------ InputSystem
          |
      ScrollSystem
          |
          +------ NodeTree
```

This is not a strictly layered architecture. `NodeTree` is intentionally a central runtime authority, while `InputSystem`, `LayoutSystem`, `ModalSystem` and `ScrollSystem` coordinate through it.

## 2. Node

`Node` is the base runtime/component object. It exposes common framework state including:

```text
Node::Id
parent / tree ownership reference
visibility / enabled state
focusable / capturable state
position / PositionMode
requested size / min / max size
desired / actual geometry
padding / border
clipToBounds
event-handler registration
```

`Node` provides virtual hooks for update, draw, Measure/Arrange content behavior, mount/unmount and hit testing. Component-specific semantic state is not placed in `Node` merely for reuse.

`Node::parent_` and `Node::owner_` are non-owning references. Structural ownership is held by `std::unique_ptr` in `NodeTree` roots/overlays or `PanelNode` children.

Every node receives a process-wide generated `Node::Id`. `NodeTree` maintains a live-node registry and the core invariant is:

```cpp
findNode(node.getId()) == &node
```

for every live node.

## 3. PanelNode

`PanelNode` derives from `Node` and adds structural child ownership through `std::vector<std::unique_ptr<Node>>`.

It provides:

```text
addChild / removeChild
getChild / getChildCount
forward/reverse child traversal
```

Mounted structural changes route through `NodeTree`, which establishes ownership, live registration, mount/unmount lifecycle and layout consequences. A child cannot already have a parent or tree owner when attached.

## 4. NodeTree

`NodeTree` is the central runtime structure. It manages:

```text
roots and overlays
structural ownership
live-node registration / lookup
mount / unmount lifecycle
pre/post-order traversal
deferred structural mutation
layout invalidation queue
update traversal
render traversal
hit testing
```

The live registry maps `NodeId` to `Node*`. Detaching a subtree unregisters it recursively and clears its `owner_` references.

### Mutation safety

`NodeTree::ScopedMutationGuard` protects traversal/callback execution. Structural operations requested while a mutation scope is active are queued. `flushMutationQueue()` drains the queue using snapshot-swap semantics and is an internal runtime operation.

### Traversal

Internal traversal supports `Continue`, `SkipChildren` and `Stop`. Top-level roots/overlays can also be traversed in forward or reverse order.

## 5. Lifecycle and Ownership

The lifecycle is conceptually:

```text
Detached
   ↓ attach
Owned + registered
   ↓ mount
Mounted/live
   ↓ detach
Unmounted + unregistered
   ↓
Detached
```

Ownership and liveness are separate concepts but are established together for an attached subtree. Framework services that retain node IDs reconcile their state against the live registry.

## 6. Layout

`LayoutSystem` owns Measure/Arrange execution. `NodeTree` owns the layout queue.

The current layout model is retained and recursive:

```text
queued root
   ↓
Measure recursively
   ↓
commit desired sizes
   ↓
Arrange recursively
   ↓
commit actual positions/sizes
```

`MeasureContext` supplies available content-box size and a child-measure operation. `ArrangeContext` supplies content position/size and child placement.

The box model is border-box at the node level. Framework layout converts between border-box and content-box using padding and border.

The current size model is:

```text
LayoutValue::Auto
LayoutValue::Value (fixed)
Node min size
Node max size
```

A fixed value and maximum can constrain the measurement proposal; minimum size is applied as a final constraint. `Auto` has no universal fill-parent meaning.

`PositionMode::Absolute` removes a child from normal `StackPanelNode` flow. Absolute children are measured and arranged separately relative to the parent's content geometry.

### Invalidation

Invalidation is explicit. `Node::invalidateLayout()` routes through the owning tree, and `UIManager::invalidateLayout(node)` inserts the affected node into the tree's layout queue. The queue promotes invalidation to the containing top-level root/overlay and deduplicates it.

Invalidation does not execute layout synchronously. A later `LayoutSystem::processLayoutQueue()` performs Measure/Arrange.

## 7. StackPanelNode

`StackPanelNode` is the current one-dimensional layout container. It supports:

```text
Vertical / Horizontal
positive gap
main alignment: START / CENTER / END / SPACE_BETWEEN
cross alignment: START / CENTER / END / STRETCH
```

Visible non-absolute children participate in normal flow. Absolute children do not contribute to flow aggregation. More advanced flex/grid behavior is not implemented.

## 8. InputSystem

`InputSystem` owns:

```text
hovered / focused / captured / pressed nodes
pointer position
drag state and threshold
focus transitions
pointer capture
SDL mouse/keyboard/text event conversion
modal filtering
```

Tracked nodes use both raw pointers and `NodeId` values so state can be reconciled against `NodeTree`.

The default drag threshold is `5.0f`.

`UIManager::processEvent()` first converts the SDL event to render coordinates when a renderer is supplied. Wheel events are offered to `ScrollSystem` before ordinary input processing.

## 9. Hit Testing

`NodeTree::hitTest()` selects the deepest valid target according to current effective geometry and paint order. Roots and overlays are considered in reverse priority order; the supplied active modal root restricts the search to that subtree.

`clipToBounds` is the current Node-level clipping/interaction boundary. There is no public `Overflow` enum.

Scrolling affects effective coordinates through the internal `Node::ScopedCoordinateTransform`; it does not rewrite retained layout positions.

## 10. Focus and Pointer Capture

Focus is managed by `InputSystem`. A focus target must be live, visible, enabled and focusable and must satisfy the active modal boundary. Focus transitions emit `FocusLostEvent` and `FocusGainedEvent`.

Pointer capture is managed by `InputSystem`. Captured nodes remain primary pointer targets for movement/release until capture is released or invalidated. Capture state is reconciled against node liveness and modal restrictions.

## 11. Event System

The public registration API is:

```cpp
node->on<EventType>(callback)
```

`Node` stores handler records and creates a callback snapshot for each delivery. `EventDispatcher` performs propagation along the target ancestry when tunneling/bubbling are requested.

The dispatcher uses `NodeId` path entries and re-resolves nodes through `NodeTree`, so removed nodes are not dispatched to after removal.

The event model exposes:

```text
target
currentTarget
phase = TUNNELING / TARGET / BUBBLING
propagationStopped
```

`stopPropagation()` stops later propagation steps; it does not cancel other callbacks already present in the current target's handler snapshot.

Input/lifecycle events live in `events.hpp`. Components may define semantic events locally, for example button activation, checkbox toggling, slider value change, text change and text-input submission.

## 12. ModalSystem

`ModalSystem` is an internal service exposed through `UIManager`.

A modal must be a live, visible, enabled overlay node and cannot already be registered. Each session stores:

```text
modal NodeId
previous focus NodeId (optional)
previous modal NodeId (optional)
ModalOptions
```

The top session defines the active modal boundary. Opening a modal cancels incompatible pointer interaction, establishes the modal root and focuses the modal or its first valid focusable descendant.

`ModalOptions` controls:

```text
outsideClick = Consume / Close
closeOnEscape
showBackdrop
```

Escape is first offered to the focused node by `UIManager`; if not stopped, `ModalSystem` applies the active modal policy. Tab traversal is implemented within the active modal subtree.

Closing clears current focus, restores the previous modal/focus where valid and updates the active boundary. Invalid sessions are removed during synchronization.

The backdrop is an internal framework node/state; there is no public `Modal` component.

## 13. ScrollSystem

`ScrollSystem` is an internal service exposed through `UIManager`:

```cpp
enableScrolling(node)
disableScrolling(node)
isScrollingEnabled(node)
setScrollOffset(node, offset)
getScrollOffset(node)
getMaximumScrollOffset(node)
```

Only live `PanelNode` instances can be registered as scroll containers.

`ScrollState` stores content extent and offset. The viewport is derived from the scroll node's current actual border-box geometry after padding/border. Maximum offset is:

```text
max(0, content extent - viewport)
```

Offsets are clamped to valid ranges.

Content extent is derived from visible committed descendant geometry. A nested registered scroll container contributes its own viewport boundary rather than recursively expanding the outer extent with its internally scrolled content.

Wheel routing starts at the hit-test target, finds the nearest registered scroll ancestor and consumes as much delta as possible. Remaining delta may propagate to an outer registered scroll ancestor.

Scrolling applies accumulated ancestor offsets through the internal coordinate-transform hook. Retained layout positions remain unchanged. After a handled wheel operation, `UIManager` refreshes hover at the same pointer coordinates.

There is no public `Scroll`/`ScrollArea` component and no `Overflow::SCROLL` property.

## 14. Rendering

SDL3 is the current concrete backend. The application owns SDL runtime lifetime and supplies the renderer to `UIManager`.

`NodeTree` owns rendering traversal. The framework applies clipping and scroll coordinate transforms during traversal and uses renderer-state preservation for temporary state.

The chess client uses SDL logical presentation at `1920 × 1080` with `SDL_LOGICAL_PRESENTATION_LETTERBOX`. `LayoutSystem` reads the current logical presentation from the renderer during `runFrame()` and requests full layout when it changes.

## 15. Text

The active text path is:

```text
Typography / text-bearing components / TextInput
        ↓
    TextContent
      ↙     ↘
 TextLayout  TextRenderer
```

`TextLayout` is a logical measurement/wrapping layer backed by SDL_ttf metrics. `TextContent` bridges text state to layout and rendering. `TextRenderer` is internal and backend-oriented.

The source `TTF_Font*` is non-owning from the framework perspective and must outlive its consumers. Derived renderer resources are owned internally.

## 16. TextInput

`TextInput` is a public single-line, game-oriented editing component. It is `Node`-based and internally owns `TextEditState` and `TextContent`.

Its current public editing API covers:

```text
committed text / placeholder / font
caret and selection queries/control
insertText
backspace / deleteForward
left/right/home/end
selectAll / clearSelection
```

It handles focused keyboard editing, committed SDL text input, temporary SDL text-editing/IME composition, mouse caret positioning and drag selection. It emits `TextChangedEvent` after committed text changes and `TextInputSubmittedEvent` on Enter.

IME composition is private temporary state and does not mutate committed text until committed text input arrives.

The current component intentionally does not provide clipboard, undo/redo, word-wise navigation, multiline editing, text viewport scrolling or framework-owned SDL text-input lifecycle/window-specific IME APIs.

## 17. Standard Components

The active component set includes:

```text
Button
ToggleButton
Menu / MenuItem
TabControl / TabItem
Checkbox
RadioButton
Slider
Dropdown
Typography
TextInput
Image
StackPanelNode / PanelNode
```

Components own semantic state and presentation. They use the common Node event registration mechanism and do not reimplement NodeTree, global input routing, layout orchestration, rendering traversal, modality or scrolling.

`Image` references an externally owned `SDL_Texture*`; it does not own texture lifetime or asset loading. It supports intrinsic size, tint and `STRETCH` / `CONTAIN` / `COVER` fit modes.

## 18. UIManager and Runtime Frame Flow

`UIManager` owns `NodeTree`, `InputSystem`, `ModalSystem`, `LayoutSystem` and `ScrollSystem` through `std::unique_ptr`.

The current `runFrame()` order is:

```text
sync renderer-derived viewport
    ↓
request full layout if viewport changed
    ↓
sync input state
    ↓
flush pending tree mutations
    ↓
process layout queue
    ↓
sync scrolling
    ↓
sync/update modality
    ↓
NodeTree update
    ↓
draw
```

`UIManager::processEvent()` is the public event-entry path. It handles scroll wheel and modal-special-key policy before delegating ordinary events to `InputSystem`.

## 19. Resource Boundary

The framework currently has no general resource manager contract. Client-owned source resources such as `TTF_Font*` and `SDL_Texture*` remain non-owning inputs to components/framework services. Backend-derived resources are owned by the internal subsystem that creates them.

## 20. Core Invariants

```text
Every live Node is registered in NodeTree.
findNode(id) returns the live object for that id.
Attached nodes have the correct NodeTree owner.
Panel children have the correct parent.
Structural mutation during guarded traversal is deferred.
Detached subtrees are unregistered and no longer tree-owned.
Layout invalidation is explicit and asynchronous.
Input state is reconciled against NodeTree liveness.
Active modal input remains inside the top modal boundary.
Scrolling changes effective coordinates, not retained layout positions.
```

## 21. Deferred / Not Stabilized

The following are intentionally outside the current stabilized framework contract:

```text
advanced flex/grid layout
non-rectangular hit-testing/clipping as a general geometry system
framework-wide scale/rotation transform stack
reparenting
scrollbar presentation
standalone Scroll / ScrollArea component
standalone Modal component
clipboard / rich TextInput editing
undo/redo / word-wise text navigation
multiline TextInput
backend-independent typography abstraction
framework-wide resource manager
```

Further separation or abstraction should be driven by concrete reusable requirements rather than architectural symmetry.