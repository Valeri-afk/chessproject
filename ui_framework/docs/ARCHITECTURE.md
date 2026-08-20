Architecture
This document describes the current architecture of the framework as implemented
in the source code.

The source code is the authoritative source of truth.

This document describes current responsibilities, ownership, lifecycle,
dependencies, invariants and existing architectural boundaries.

Planned architecture and future development order are defined separately in
ROADMAP.md.

1. Architectural Overview
The framework is currently organized around a central NodeTree.

NodeTree owns the live UI hierarchy and is responsible for node ownership,
lifetime, traversal, mutation, lifecycle, layout invalidation, update,
rendering and hit-testing.

UIManager acts as the public runtime facade and coordinates the major
subsystems.

The current architecture is not a strictly layered architecture. Several
subsystems have direct dependencies on NodeTree, and some responsibilities
are intentionally centralized there.

The current high-level structure is:

text
                         UIManager
                             |
        +--------------------+--------------------+
        |                    |                    |
    NodeTree            InputManager        LayoutManager
        |                    |                    |
        |              EventDispatcher      StackPanelNode
        |
        +------ Node
        +------ PanelNode
        +------ lifecycle
        +------ traversal
        +------ mutation
        +------ update
        +------ layout invalidation
        +------ hit-testing
        +------ rendering
        +------ overlays

    ModalManager
        |
        +------ NodeTree
        +------ InputManager


This diagram is intentionally high-level.

Actual source-level dependencies are more tightly coupled than this diagram
suggests.

In particular:

InputManager depends directly on NodeTree, EventDispatcher, SDL3 input
types and modal state.
LayoutManager operates directly on NodeTree and Node.
ModalManager coordinates directly with NodeTree and InputManager.
rendering is currently SDL3-specific;
NodeTree contains several responsibilities beyond pure tree ownership.

These are properties of the current implementation.

## 2. Repository Scope
## 2.1 Active Core

The active framework core is primarily located in:

include/ui_framework/
    types.hpp
    event_types.hpp
    core/
        node.hpp
        panelnode.hpp
        ui_manager.hpp
        stackpanelnode.hpp
        event_handler_storage.hpp
        event_handler_storage.inl


src/core/
    node.cpp
    nodetree.cpp
    nodetree.hpp
    panelnode.cpp
    ui_manager.cpp
    inputmanager.cpp
    inputmanager.hpp
    event_dispatcher.hpp
    layoutmanager.cpp
    layoutmanager.hpp
    modalmanager.cpp
    modalmanager.hpp
    stackpanelnode.cpp
    primitives.cpp

The exact public/private header placement is a repository organization detail;
the runtime implementation also uses internal headers from src/core.

## 2.2 Standard Components

The `components/` directories are an active framework layer.

They contain the current standard UI component set:

- Button
- ToggleButton
- Menu
- MenuItem
- TabControl
- TabItem
- Checkbox
- RadioButton
- Slider
- Dropdown

The component layer is intentionally small and does not imply a universal
component hierarchy or a requirement to reproduce a complete widget toolkit.

Legacy Widget-based implementations and removed experimental component files
are not part of the active component architecture.

## 2.3 Primitive Rendering Support

The following files are retained as low-level rendering support:

primitives.hpp
primitives.cpp

They provide primitive drawing functionality such as borders, rounded corners
and other low-level rendering operations.

They are not a central runtime subsystem.

They should therefore be considered part of the rendering/backend area rather
than part of:

- NodeTree ownership;
- lifecycle;
- traversal;
- input;
- event propagation;
- layout orchestration.

Their relevance becomes significant when rendering functionality is being
modified.

Primitives are the preferred mechanism for simple geometric presentation of
standard components. They are not a texture/resource ownership system and are
not intended to replace a future Image/resource layer.

## 2.4 SDL3 Backend

SDL3 is the current platform, input and rendering backend.

SDL3 itself is not vendored into this repository.

The framework currently depends directly on SDL3 in several areas, including:

- UIManager;
- InputManager;
- node rendering;
- viewport synchronization;
- panel rendering and clipping;
- primitive rendering.

There is currently no independent rendering/backend abstraction.

A future RenderContext or additional backend is therefore a future
architectural direction rather than a current architectural layer.

## 3. Node

Source:

include/ui_framework/core/node.hpp
src/core/node.cpp

Node is the base object representing an element of the UI hierarchy.

It contains common state and behavior shared by node types.

### Responsibilities

Node currently owns or exposes:

- unique Node::Id;
- parent relationship;
- tree ownership reference;
- visibility;
- enabled state;
- focusability;
- capturability;
- logical position;
- position mode;
- requested size;
- minimum and maximum size;
- desired size;
- actual position;
- actual size;
- padding;
- border;
- overflow mode;
- event handlers.

It also provides virtual hooks for:

- update()
- draw()
- onMount()
- onUnmount()
- hitTest()

Layout measurement and arrangement are framework-owned by LayoutManager.
Node does not expose the legacy measure()/arrange() lifecycle to clients.

Therefore Node currently combines several responsibilities:

- runtime state
- layout state
- input-related state
- event handling
- lifecycle hooks
- rendering hook

This is the current implementation boundary.

Node is the base runtime/component object. Its framework-recognized state is
not intended to encode every component-specific semantic property.

Generic properties such as visibility, enabled state, geometry, padding,
border and overflow are interpreted by framework subsystems. Concrete
components keep their domain state in the component itself.

## 3.1 Node Ownership

A Node does not own itself.

Ownership is established through the tree/container hierarchy.

A node can be owned by:

- NodeTree as a root;
- NodeTree as an overlay;
- PanelNode as a child container.

Node stores:

```cpp
Node *parent_;
NodeTree *owner_;
These are non-owning references.

Actual ownership is represented by:

cpp
std::unique_ptr<Node>
stored by NodeTree or PanelNode.

3.2 Node Identity
Every node receives a globally generated Node::Id.

NodeTree maintains a registry:

cpp
NodeId -> Node*
for currently live nodes.

The important identity invariant is:

cpp
findNode(node.id()) == &node
for every live node.

4. PanelNode
Source:

include/ui_framework/core/panelnode.hpp
src/core/panelnode.cpp

PanelNode derives from Node and is the current generic child-container node.

It owns child nodes through:

cpp
std::vector<std::unique_ptr<Node>>
Responsibilities
PanelNode provides:

child ownership;

child insertion;

child removal;

child traversal;

reverse child traversal;

visible-child access;

visible-child indexing;

hierarchy validation.

PanelNode does not own the layout measurement or arrangement lifecycle.

Its current role is:

text
Node
  +
structural child ownership
  +
child/layout composition capability
PanelNode is selected when a concrete component genuinely requires
owned child Nodes, child geometry management, layout flow or structural
composition. The presence of text, icons or multiple visual primitives
alone does not require PanelNode.

4.1 Hierarchy Invariants
A child cannot be attached when:

it already has a parent;

it already belongs to a NodeTree;

the attachment would create a hierarchy cycle.

When a PanelNode belongs to a tree, structural child mutation is coordinated
with NodeTree.

5. NodeTree
Source:

src/core/nodetree.hpp
src/core/nodetree.cpp

NodeTree is the central runtime structure.

It is currently the most important architectural subsystem.

Responsibilities
NodeTree currently manages:

root nodes;

overlay nodes;

node ownership;

child attachment/detachment;

live-node registration;

node identity lookup;

lifecycle;

traversal;

deferred mutation;

layout invalidation;

update traversal;

rendering traversal;

hit-testing.

NodeTree therefore currently acts as more than a pure ownership container.

5.1 Root and Overlay Containers
The tree maintains two top-level collections:

roots_

overlays_

Both contain owning:

cpp
std::unique_ptr<Node>
Overlay nodes are rendered and hit-tested above normal roots.

5.2 Live Node Registry
NodeTree maintains:

cpp
std::unordered_map<NodeId, Node*> liveNodes_
A node becomes live when its subtree is attached to the tree.

The complete subtree is registered recursively.

When detached, the subtree is recursively unregistered.

The registry is used throughout the framework to validate pointer identity and
lifetime.

The central invariant is:

cpp
findNode(node.id()) == &node
for every live node.

6. Ownership and Lifecycle
The current lifecycle can be summarized as:

text
Detached Node
     |
     | attach
     v
Owned by NodeTree
     |
     | register subtree
     v
Live Node
     |
     | mount
     v
Mounted Node
     |
     | detach
     v
Unmount
     |
     | unregister
     v
Detached Node
The exact attachment path depends on whether the node is:

a root;

an overlay;

a child of a PanelNode.

When attached to a tree, ownership and live-node registration are established
recursively for the subtree.

Mount/unmount callbacks are executed through subtree traversal.

7. Mutation Model
The framework uses deferred structural mutation to protect traversal and
callback execution from changes to the hierarchy.

NodeTree maintains a mutation queue.

Mutations are executed through:

cpp
flushMutationQueue()
The queue uses snapshot-swap semantics.

If execution of one batch creates additional mutations, those mutations are
placed into the next batch and processed during the same flush operation.

A nested mutation scope is represented by:

cpp
ScopedMutationGuard
Structural operations performed while a mutation scope is active are deferred
instead of modifying the hierarchy immediately.

The mutation guard is used by traversal and by runtime operations that
may invoke callbacks capable of mutating the tree, including event
dispatch and update.

Rendering and other operations also use the guard where required by
their current implementation.

Deferred mutation is therefore one of the fundamental runtime invariants.

8. Traversal
NodeTree provides internal pre-order and post-order traversal.

Traversal supports:

Continue

SkipChildren

Stop

Traversal is used for:

mounting;

unmounting;

update;

ownership registration;

ownership removal;

other subtree operations.

Top-level roots and overlays can also be traversed in forward or reverse order.

Traversal operates together with mutation guards and live-node validation where
required by the operation.

9. Update
The normal update path is:

text
UIManager
    |
    v
NodeTree::update()
    |
    +-- roots
    |
    +-- overlays
           |
           v
      pre-order traversal
           |
           v
      Node::update(dt)
Invisible or disabled nodes are skipped together with their descendants.

Structural mutations generated during update are deferred and processed after
the traversal according to the mutation model.

10. Layout
Source:

src/core/layoutmanager.hpp
src/core/layoutmanager.cpp
include/ui_framework/types.hpp

LayoutManager currently owns the orchestration of the Measure / Arrange
process.

The current layout system is recursive.

text
NodeTree layout queue
        |
        v
LayoutManager
        |
        +-- measureRecursive()
        |
        +-- arrangeRecursive()
Layout is currently invalidated at the root level.

When a node requires layout, NodeTree walks from that node toward its root and
queues the root.

The queue is deduplicated using:

layoutQueue_

layoutQueueSet_

The current invalidation granularity is therefore:

text
affected node
      |
      v
nearest tree root
      |
      v
full root layout
LayoutManager is the framework-owned authority for measurement and arrangement.
Layout containers participate through the internal layout pipeline; clients do
not implement or invoke Measure / Arrange lifecycle methods.

11. Measure
MeasureContext provides:

availableSize

measureChild()

The available size represents the content-box size.

The current layout implementation converts between:

content-box

border-box

using padding and border values.

The recursive process is approximately:

text
measure parent
    |
    +-- measure child
    |      |
    |      +-- measure descendants
    |
    +-- calculate desired size
The resulting desired size is stored on Node.

Fixed sizes and min/max constraints are applied by the current layout
implementation.

12. Arrange
ArrangeContext provides:

contentPosition

contentSize

placeChild()

The arrange process recursively assigns:

actualPosition_

actualSize_

to child nodes.

Child sizes are clamped against their min/max constraints before being
committed.

13. StackPanelNode
Source:

include/ui_framework/core/stackpanelnode.hpp
src/core/stackpanelnode.cpp

StackPanelNode derives from:

PanelNode

and provides the concrete one-dimensional flow behavior used by LayoutManager.

It supports:

Vertical

Horizontal

orientation.

The current implementation:

measures children sequentially;

accumulates their sizes along the main axis;

uses the maximum child size on the cross axis;

arranges children sequentially;

stretches children across the cross axis.

StackPanelNode is therefore an existing framework-owned layout container.

Its current behavior is part of the completed Phase 2 one-dimensional layout
scope. More advanced flex/grid behavior remains outside Phase 2.

14. Positioning
Node currently exposes:

PositionMode::Layout

PositionMode::Absolute

PositionMode::Absolute is part of the completed Phase 2 positioning scope.

Absolute-positioned children are separated from normal one-dimensional flow
and receive framework-owned final geometry according to the current layout
pipeline.

Advanced positioning semantics beyond the Phase 2 scope are not implied.

15. InputManager
Source:

src/core/inputmanager.hpp
src/core/inputmanager.cpp

InputManager converts SDL input into framework-level interaction state and
events.

Responsibilities
It currently manages:

hovered node;

focused node;

captured node;

pressed node;

drag state;

pointer tracking;

focus transitions;

pointer capture;

mouse enter/leave;

mouse click;

mouse drag;

mouse wheel;

keyboard events;

modal input restriction.

Tracked nodes use both:

Node*

optional<Node::Id>

This allows cached pointers to be validated against the current NodeTree.

16. Hit Testing
Hit testing is currently initiated by NodeTree.

Hit-testing follows effective paint order.

Top-level roots and overlays are considered in reverse priority order, with the
top modal acting as the highest-priority interaction boundary when active.

Within a subtree, children are traversed in reverse structural paint order and
the deepest valid descendant is selected.

Overflow::HIDDEN acts as an ancestor clipping boundary during hit-testing.

17. Focus
Focus is managed by InputManager.

A node must satisfy the relevant state requirements before receiving focus.

Focus transitions dispatch:

FocusLostEvent

FocusGainedEvent

Tracked focus state uses both:

Node*

Node::Id

The NodeId is the authoritative identity used to resolve the current live node
through NodeTree.

InputManager separates stale-reference reconciliation from semantic focus
validation:

cpp
syncState()
    → repairs references to nodes that are no longer live.

validateInputState()
    → validates live focused nodes against visibility, enabled state,
      focusability and the active modal boundary.
When a live focused node becomes invalid, InputManager performs the semantic
focus transition through clearFocus(), which dispatches FocusLostEvent.

When the focused node has already disappeared from the live NodeTree,
InputManager only repairs the stale reference; no event is dispatched to a dead
node.

The active modal root is therefore a focus boundary. A live focused node
outside the active modal subtree is invalidated through the normal focus
transition rather than being silently removed by state synchronization.

Focus transitions are reentrancy-safe. A focus request or clear operation made
during FocusLostEvent or FocusGainedEvent is reconciled by InputManager before
the enclosing transition completes.

18. Pointer Capture
Pointer capture is managed by InputManager.

The captured node becomes the primary target for pointer movement and release
events even when the pointer is no longer over that node.

Capture is used by the drag system.

The current drag model contains:

pressed node

captured node

press position

drag threshold

dragging state

The current default drag threshold is:

cpp
5.0f
Capture state is tracked using both Node* and Node::Id and is reconciled
against the NodeTree live-node registry.

Capture is protected against reentrant callback mutation. An operation that
begins with an existing capture only releases that interaction. If a callback
establishes a different capture during MouseUp, Click or DragEnd processing,
the newly established capture is preserved.

A captured node that is no longer live or no longer satisfies the active input
state or modal boundary is invalidated by InputManager.

19. Event System
The event system consists of:

event_types.hpp

EventHandlerStorage

Node::dispatchEvent()

EventDispatcher

InputManager

The framework currently defines events including:

MouseMove

MouseDown

MouseUp

MouseClick

MouseWheel

MouseEnter

MouseLeave

MouseDragBegin

MouseDrag

MouseDragEnd

KeyDown

KeyUp

FocusGained

FocusLost

20. EventHandlerStorage
EventHandlerStorage stores handlers by event type.

Handlers receive:

Event&

Node&

Handlers receive a HandlerToken when registered.

Handlers can be:

added;

removed;

cleared.

The storage uses event-type-indexed tables.

During dispatch, EventHandlerStorage creates a snapshot of the current handler
entries and invokes handlers from that snapshot.

Therefore mutation of the live handler table during callback does not invalidate
the current iteration.

Handlers added during the current dispatch are not included in the existing
snapshot.

Handlers removed or cleared during the current dispatch remain part of the
current snapshot and therefore do not alter the already-started iteration.

21. EventDispatcher
Source:

src/core/event_dispatcher.hpp

EventDispatcher is responsible for event propagation along the node ancestry.

The propagation path is constructed from:

cpp
target -> parent -> ... -> root
The dispatcher supports:

TUNNELING

TARGET

BUBBLING

The caller independently selects whether tunneling and bubbling are enabled.

The propagation path stores NodeId values rather than relying exclusively on
raw pointers.

For each propagation step, the current Node is resolved again through
NodeTree::findNode(). A node that is no longer live is not dispatched to.

EventDispatcher performs propagation only. InputManager owns the surrounding
input-event orchestration and establishes the NodeTree mutation scope during
framework input dispatch.

Event propagation therefore operates together with the framework's deferred
mutation model.

The current event model uses:

cpp
event.target
    → original dispatch target

event.currentTarget
    → node currently receiving the event

event.phase
    → TUNNELING, TARGET or BUBBLING
Event propagation can be stopped with:

cpp
UIEvent::stopPropagation()
When propagation is stopped, dispatch does not continue to subsequent
propagation nodes or phases.

EventDispatcher does not own Node lifetime, mutation queues or input state.

22. ModalManager
Source:

src/core/modalmanager.hpp
src/core/modalmanager.cpp

ModalManager maintains a stack of active modal sessions.

A modal must:

be live;

belong to the overlay container;

be visible;

be enabled.

Each modal session stores:

modal NodeId

previous focused NodeId

Conceptually:

text
overlay nodes
      |
      v
ModalManager
      |
      v
modal stack
      |
      v
top modal
Only the top modal defines the current modal interaction boundary.

23. Modal Input Boundary
When a modal is active, InputManager receives the top modal as its modal
root.

Hit testing is then restricted to that modal subtree.

The resulting interaction boundary is:

text
Top Modal
   |
   +-- descendants can receive pointer input
   |
   +-- outside nodes cannot be targeted
The modal input boundary therefore depends directly on:

NodeTree hit testing;

InputManager state;

overlay ownership;

ModalManager.

24. Modal Focus
When a modal opens:

current focus is remembered;

pointer interaction is cancelled;

the first valid focusable descendant is searched;

focus is moved into the modal.

When a modal closes:

pointer interaction is cancelled;

current focus is cleared;

previous focus is restored if still valid;

otherwise another valid focusable node is selected.

Invalid modal sessions are removed during synchronization.

Modality is a framework-level service implemented through ModalManager.

ModalManager owns framework-level modal behavior, including modal session/stack
management, modal-root input restrictions, focus/capture interaction,
Escape routing, backdrop interaction and focus restoration.

The legacy Modal component was removed from the active source tree because its
implementation depended on the legacy Widget model.

A standalone public Modal component is intentionally not required at the
current stage. If a concrete reusable presentation contract later proves that
a component API is useful, it should be built on top of the existing modality
service rather than reimplementing modality behavior.

25. Rendering
SDL3 is the current and only concrete backend.
The application owns SDL runtime lifetime.
The framework consumes SDL3 types and the supplied renderer.
No generic RenderContext/backend interface exists as a current layer.

The primary rendering traversal is owned by NodeTree.

The current order is approximately:

text
roots
  |
  v
non-top overlays
  |
  v
top modal
This ensures that the top modal is rendered last among the overlay layer.

NodeTree::drawSubtree() currently:

checks visibility;

applies overflow clipping where required;

calls Node::draw();

recursively renders panel children.

Renderer state is temporarily saved/restored around subtree rendering.

25.1 Animation
Animation is not a current global rendering subsystem. It is a mechanism
for changing semantic/visual state through normal framework semantics.

26. Clipping
Overflow::HIDDEN defines a subtree clipping/interaction boundary.

Clipping is a framework-level mechanism implemented during rendering traversal.
Nested clipping rectangles are intersected.

Clipping and scrolling are related but distinct. Clipping determines which part
of a subtree may be rendered/interacted with. Scrolling changes the effective
coordinate space of descendant content. The two mechanisms are composed during
framework traversal.

Scrolling is a separate framework-level concern implemented through
ScrollManager. The framework does not model scrolling as a simple
Overflow::SCROLL property.

27. Scrolling
Scrolling is a framework-level behavior rather than a required standalone UI
component.

ScrollManager owns scroll state and coordinates with UIManager and
NodeTree.

The current source-level responsibilities include:

viewport/content extent

scroll offset

maximum scroll range

offset clamping

wheel input routing

nested scroll chaining

layout-derived content extent

coordinate transformation during input/render traversal

Scrolling does not rewrite the original layout positions of descendants.

The effective descendant coordinates are obtained by applying the accumulated
scroll offset during framework traversal.

Overflow::HIDDEN remains the existing clipping mechanism. It is a clipping
primitive and is not itself equivalent to a complete scrolling subsystem.

A standalone Scroll / ScrollArea component and scrollbar presentation are
not currently required.

28. Component System
Phase 5 established the active standard component layer on top of the existing
runtime, layout, input/event and rendering infrastructure.

The component system is intentionally responsibility-driven rather than based
on a universal inheritance hierarchy.

Node is the default component base.

PanelNode is used when structural child ownership and child/layout composition
are part of the component's semantics.

StackPanelNode may be reused when its existing child-flow semantics match the
component.

Components own semantic state and presentation. Generic framework mechanics such
as ownership, lifecycle, layout orchestration, hit-testing, event propagation,
render traversal, clipping, modality and scrolling remain framework-owned.

29. Runtime Frame Flow
The public runtime facade is UIManager.

The normal frame flow is approximately:

text
UIManager::runFrame()
        |
        +-- synchronize viewport
        |
        +-- prepare for tree operation
        |
        +-- NodeTree::update()
        |
        +-- LayoutManager::processLayoutQueue()
        |
        +-- synchronize input/modal state
        |
        +-- NodeTree::draw()
        |
        +-- synchronize state
Input events follow a separate path:

text
SDL_Event
    |
    v
UIManager::processEvent()
    |
    v
InputManager
    |
    +-- hit-test
    +-- focus/capture state
    +-- event generation
    |
    v
EventDispatcher
    |
    v
Node handlers
Tree mutations may be deferred during these operations and flushed before the
runtime proceeds to the next stable state.

30. UIManager
UIManager is the main public runtime facade.

It currently exposes operations including:

frame execution;

SDL event processing;

root attachment/detachment;

overlay attachment/detachment;

viewport configuration;

modal open/close;

modal queries.

It owns the major runtime managers through std::unique_ptr:

NodeTree

InputManager

ModalManager

LayoutManager

ScrollManager

It also synchronizes renderer-derived viewport state before frame processing.

The individual managers remain separate implementation subsystems.

31. Framework Viewport
The framework viewport is the current logical UI coordinate space.

When SDL logical presentation is configured, the framework obtains its viewport
size from the renderer's logical presentation rather than requiring the client
to push viewport dimensions manually.

The physical monitor size and physical render-output size are not the framework
UI coordinate system.

The current viewport therefore represents the logical presentation area
provided to the framework by the SDL renderer.

If logical presentation is unavailable, the renderer output size is used as the
compatibility fallback.

32. Dependency Structure
The current architecture can be summarized as:

text
                         UIManager
                             |
          +------------------+------------------+
          |                  |                  |
          v                  v                  v
      NodeTree          InputManager      LayoutManager
          |                  |                  |
          |                  v                  v
          |            EventDispatcher    StackPanelNode
          |
          +------ Node
          |
          +------ PanelNode
          |
          +------ lifecycle
          |
          +------ traversal
          |
          +------ mutation
          |
          +------ update
          |
          +------ layout invalidation
          |
          +------ hit-testing
          |
          +------ rendering
          |
          +------ overlays
                         |
                         v
                    ModalManager
                         |
                         v
                    ScrollManager
Important direct dependencies include:

cpp
UIManager
  -> NodeTree
  -> InputManager
  -> LayoutManager
  -> ModalManager
  -> ScrollManager

InputManager
  -> NodeTree
  -> EventDispatcher
  -> ModalManager state
  -> SDL3

LayoutManager
  -> NodeTree
  -> Node

ModalManager
  -> NodeTree
  -> InputManager

NodeTree
  -> Node
  -> PanelNode
  -> SDL3 rendering
This is a description of the current dependency structure, not a target
layered architecture.

33. Core Invariants
The following invariants are fundamental to the current implementation.

33.1 Node Lifetime
A live node must be registered in NodeTree.

For a live node:

cpp
findNode(node.id()) == &node
must hold.

33.2 Ownership
A node attached to a tree has:

cpp
owner_ == that NodeTree
for the complete owned subtree.

33.3 Parent Relationship
A child attached to a PanelNode has that panel as its parent.

33.4 Mutation Safety
Structural mutations performed during guarded traversal are deferred.

33.5 Detached State
A detached subtree must no longer belong to the previous NodeTree.

Its live-node registrations must also be removed from that tree.

33.6 Traversal Safety
Traversal must tolerate structural changes initiated by callbacks according to
the current deferred-mutation and live-node validation mechanisms.

33.7 Layout Invalidation
A layout-affecting mutation currently queues the containing root for layout
processing.

33.8 Input Safety
InputManager must not continue using invalid node references after the
corresponding node has been removed from the live tree.

33.9 Modal Boundary
When a modal is active, pointer targeting must remain inside the top modal
subtree.

34. Current Architectural Boundaries
The current implementation establishes the following responsibility
boundaries.

Runtime / Tree
NodeTree owns:

runtime hierarchy;

ownership;

lifecycle;

mutation;

traversal.

Layout
LayoutManager owns the framework-level measurement and arrangement pipeline.

Individual layout containers provide layout-specific behavior through that
pipeline. They do not own or expose the Measure / Arrange lifecycle.

Resource boundary
Renderer-bound resources are owned by the Node/component that needs them.
There is no generic ResourceManager in the current architecture.
Semantic resources and backend-bound representations remain conceptually
separate, but no additional generic resource subsystem is required yet.

Input
InputManager owns interaction state and converts SDL input into framework
input events.

Events
EventDispatcher owns event propagation mechanics.

EventHandlerStorage owns handler registration and storage.

Modal
ModalManager owns modal stack semantics and coordinates modal-related focus
behavior.

Scrolling
ScrollManager owns scroll state and coordinates with UIManager and NodeTree.
Scrolling is a framework-level behavior rather than a component property.

Rendering
Rendering is currently implemented directly through SDL3 and the
NodeTree rendering traversal.

Components
Components own:

semantic state

component-specific visual properties

presentation

component-specific semantic interaction

intentional structural composition

Framework infrastructure owns:

ownership/lifetime

NodeTree traversal

layout orchestration

hit-testing

event propagation

focus/capture

render traversal

clipping

modality

scroll mechanics

future text-input infrastructure

future resource ownership infrastructure

35. Areas Outside Current Runtime Stabilization
The following areas exist in the repository or are represented by current
types, but are not currently stabilized architectural layers:

advanced alignment;

Grid;

complete component hierarchy;

rendering abstraction;

backend abstraction;

second rendering backend;

resource abstraction.

ControlNode was a historical/experimental WPF-inspired abstraction.
It is no longer part of the active source tree and is not an accepted universal
base class for framework components.

The current component architecture uses concrete responsibilities to determine
the appropriate base:

Node

PanelNode

StackPanelNode

A component does not require ControlNode merely because it is interactive
or visually complex.

StackPanelNode is an existing and currently implemented one-dimensional layout
container. Its Phase 2 behavior is considered complete at source level.

These areas are not necessarily architectural problems.
They are simply not part of the currently stabilized runtime foundation.
Their development order is defined in ROADMAP.md.

36. Current Architecture Summary
The current framework should be understood as a centralized runtime architecture
built around NodeTree:

text
                         UIManager
                             |
          +------------------+------------------+
          |                  |                  |
      NodeTree          InputManager      LayoutManager
          |                  |                  |
          |                  v                  |
          |            EventDispatcher          |
          |                                     |
          +------ Node / PanelNode       StackPanelNode
          |
          +------ ownership
          |
          +------ lifecycle
          |
          +------ mutation
          |
          +------ traversal
          |
          +------ update
          |
          +------ layout invalidation
          |
          +------ hit-testing
          |
          +------ rendering
          |
          +------ overlays
                         |
                         v
                    ModalManager
                         |
                         v
                    ScrollManager
The important architectural characteristic is that NodeTree is currently
the central runtime authority rather than a narrowly scoped tree container.

The framework is therefore not yet a fully separated layered architecture.

Further separation of responsibilities should be based on concrete requirements
and the active development phase rather than introduced solely for architectural
symmetry.

The current source code remains the authoritative definition of all behavior.

Component architecture cross-reference

Phase 5 component architecture is documented separately in
docs/PHASE5_COMPONENT_ARCHITECTURE.md.

That document defines the current component-design rules, including the
Node/PanelNode boundary, semantic content vs structural children, component
state ownership, primitive-node criteria and the rule that new abstractions
must emerge from concrete requirements.

