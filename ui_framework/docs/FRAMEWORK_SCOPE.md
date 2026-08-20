# Framework Scope and Purpose

## 1. Why This Framework Exists

This project originated from the development of a chess application, but the framework is not a chess framework and is not intended to become a complete widget library.

The framework exists to provide a small retained-mode C++/SDL3 runtime in which independently implemented UI objects can participate in one coherent system with shared ownership, lifecycle, traversal, layout, input, events and rendering.

The chess application is the immediate validation target. It is not the source of application-specific framework components.

The framework should be minimal: provide the generic infrastructure and standard UI concepts that a real application can reasonably reuse, while leaving application-specific composition and semantics to the client.

---

## 2. What This Project Is

This project is a **lightweight retained-mode C++ UI framework/runtime** for interactive graphical applications.

The framework provides infrastructure for:

- hierarchical UI object ownership and lifetime;
- lifecycle callbacks;
- traversal;
- safe structural mutation;
- update scheduling;
- rendering traversal;
- input coordination;
- event propagation;
- layout;
- reusable framework primitives;
- a small set of standard UI components;
- user-defined Nodes and PanelNodes built on the runtime.

The framework is intentionally narrower than large general-purpose application frameworks such as Qt or WPF.

It should be designed around the supported application class rather than around feature parity with larger toolkits.

---

## 3. Target Application

The immediate target application is a chess application using a separate chess engine/domain layer and the UI framework as its presentation/runtime layer.

The application may contain:

```text
Application
├── Main Menu
│   ├── New Game
│   ├── Settings
│   ├── Rules
│   ├── Openings
│   └── Exit
│
├── Game Screen
│   ├── Chess Board
│   ├── Chess Pieces
│   ├── Move Highlights
│   ├── Player Clocks
│   ├── Captured Pieces
│   ├── Move History
│   └── Auxiliary Controls
│
└── Auxiliary UI
    ├── Settings
    ├── Rules / Help
    ├── Opening Information
    ├── Dialogs / Modals
    └── Other application screens
```

These are examples of application composition, not a framework component catalog.

The framework does not implement chess rules or chess-domain behavior.

The boundary is:

```text
Chess Engine / Domain
        |
        v
Chess Client / Application
        |
        v
UI Framework
```

The chess engine owns chess state and rules. The client owns application-specific meaning and behavior. The framework owns reusable UI runtime mechanisms and standard UI concepts.

---

## 4. Framework Responsibilities

The framework should own mechanisms required to make many UI objects operate as one runtime.

### Runtime structure

- hierarchical node ownership;
- live-node registration;
- node identity;
- lifecycle;
- traversal;
- structural mutation;
- safe mutation during callbacks.

### UI execution

- update traversal;
- rendering traversal;
- ordering guarantees;
- callback execution boundaries.

### Interaction infrastructure

- input routing;
- focus/capture where required;
- hit testing;
- event propagation;
- event dispatch ordering.

### Layout infrastructure

- measurement;
- arrangement;
- layout invalidation/scheduling;
- reusable layout primitives.

### Component/runtime support

- framework-recognized Node state and generic execution semantics;
- reusable low-level primitives where centralization is justified;
- a small set of standard UI components;
- extension points for custom Nodes and PanelNodes.

The framework should centralize generic mechanics so custom component authors do not reimplement ownership, lifecycle, traversal, layout, input, hit-testing, events or rendering coordination.

---

## 5. Client Responsibilities

The framework client is responsible for application-specific state and meaning.

Examples:

- chess rules;
- chess engine integration;
- game state;
- move semantics;
- clock/game-time rules;
- opening data;
- PGN/FEN/domain data;
- navigation intent;
- what a button or menu item means to the application;
- application-specific component behavior.

The framework provides mechanisms, not application meaning.

For example:

```text
Framework:
    detect button click
    dispatch event
    invoke client callback

Client:
    interpret "New Game"
    reset chess session
    choose the next screen
```

A custom component may own its own semantic state and presentation, but it should use framework infrastructure instead of implementing a competing runtime.

---

## 6. Supported Application Class

The intended class of application is larger than a trivial one-screen game but smaller than a universal application platform.

The framework should be able to support applications with:

- multiple screens or views;
- nested panels and containers;
- menus;
- settings interfaces;
- lists and scrollable content when the corresponding framework capability is implemented;
- interactive controls;
- overlays and modal UI through the framework's modality service;
- continuously updated information such as clocks;
- dynamic content;
- custom drawing;
- application-specific node types;
- callback-driven interaction.

The chess application is the immediate concrete target, but the runtime should remain useful for other interactive C++ applications with similar infrastructure needs.

---

## 7. Capability Principles

The framework should not implement a capability merely because another UI toolkit implements it.

Preferred rule:

> A capability belongs in the framework when it represents a real responsibility of the UI runtime or is repeatedly required by the supported application class, and the resulting developer contract remains appropriately small.

### Current framework capabilities

The current active standard component layer contains:

```text
Button
ToggleButton
Menu / MenuItem
TabControl / TabItem
Checkbox
RadioButton
Slider
Dropdown
```

The active framework foundation contains:

```text
Node
PanelNode
StackPanelNode
TextNode / TextPrimitive
NodeTree
InputManager
EventDispatcher
LayoutManager
RenderingState
ScrollManager
ModalManager
primitives
```

Framework-level scrolling and modality are services/infrastructure, not mandatory standalone `Scroll`, `ScrollArea` or `Modal` components.

### Deferred capabilities

These remain deliberately unresolved or deferred:

```text
List
TextField / Input
Image / resource ownership
IconButton
Scrollbar presentation
Standalone Scroll / ScrollArea component
Standalone Modal component
```

`List` is deferred until it has a distinct generic contract beyond a semantic alias of an existing layout node.

Scrolling is already implemented as framework-level infrastructure through `ScrollManager`. A standalone `Scroll` / `ScrollArea` component remains deferred until repeated application usage demonstrates a useful component-level contract beyond the service and existing Node/PanelNode composition.

Modality is already implemented through `ModalManager` as a framework service. A separate public Modal component is not currently required.

`TextField / Input` and `Image / resource ownership` remain dependent on framework infrastructure that is not yet complete.

`IconButton` is deferred until a stable graphics/icon primitive and resource contract exists.

Scrollbar visuals are deferred until scroll behavior is runtime-validated and a concrete reusable visual contract exists.

### Future / optional capabilities

Capabilities such as drag-and-drop, richer document/editor behavior, accessibility, docking, advanced animation, or other broad toolkit features should be introduced only after concrete requirements establish them.

### Non-goals

- chess rules;
- chess-engine evaluation;
- PGN/FEN semantics;
- opening databases;
- application data models;
- application-specific business logic;
- universal arbitrary-content composition;
- CSS-style declaration/interpreting of arbitrary component properties;
- mandatory generic `Control`, `Interactive`, `Selectable`, `Composite`, or `Action` base classes;
- mandatory service objects that every custom component author must create/configure;
- feature parity with Qt, WPF, Material UI, or other larger UI toolkits.

---

## 8. Reparenting Scope

Reparenting is a legitimate capability in richer retained-mode UI toolkits, but it is not automatically required for every application.

For the current chess application, ordinary UI composition usually creates a node directly under the parent that should own it. Typical screens such as menus, settings, rules, opening cards, move history, timers, and dialogs do not inherently require an existing node to move between unrelated parents.

Therefore reparenting remains a **future capability**, not a current requirement.

If drag-and-drop, tab transfer, docking, or another concrete use case establishes the need, the runtime semantics should be designed first and only then should the smallest necessary public API be added.

---

## 9. Ownership Scope

The framework uses `std::unique_ptr` as the fundamental ownership mechanism.

The runtime contract is centered on framework-owned attachment and removal:

```text
add(std::unique_ptr<Node>)
remove(Node&)
```

Public ownership-transfer `detach()` is not part of the target runtime contract.

Client-held `Node*` references are non-owning. `NodeId` provides identity/liveness resolution; it does not provide ownership.

---

## 10. Current Architectural Center

The central problem solved by the framework is coordination.

Without a runtime, independent components compete for control over:

```text
render
update
input
callbacks
lifetime
hierarchy
```

The framework centralizes coordination through:

```text
UIManager
    |
    v
NodeTree
    |
    +-- Node ownership/lifetime
    +-- traversal
    +-- mutation safety
    +-- live-node registry
    +-- lifecycle
```

Other subsystems such as layout, input, events, modal handling, scrolling and rendering build on that runtime instead of independently controlling component lifetime and traversal.

---

## 11. Design Philosophy

1. **Runtime correctness before component breadth.** A small set of reliable infrastructure primitives is more valuable than many loosely integrated controls.

2. **Client extensibility without loss of runtime control.** Clients can create custom Nodes and components while the framework retains responsibility for generic runtime mechanics.

3. **Minimal sufficient API.** Do not add a capability only because a larger toolkit has it.

4. **Concrete requirements before abstractions.** Do not create hierarchy or properties merely for symmetry.

5. **Domain/application logic stays outside the framework.**

6. **The source code remains authoritative.** Documentation must not describe hypothetical behavior as implemented.

7. **Keep custom component contracts small.** Generic mechanics should be centralized when coordinated correctness requires it, without introducing unnecessary public service machinery.

---

## 12. Relationship to Large UI Frameworks

Qt, WPF, Material UI and similar systems are useful references for established UI concepts, component states and composition patterns.

They are not the specification for this project.

The appropriate question is:

> Does the supported application class require this capability, is providing it a responsibility of the framework, and does the abstraction remain appropriately small?

Existing frameworks are references, not feature checklists or hierarchy templates.

---

## 13. Current Development Strategy

The current process is:

```text
1. establish architecture and runtime contracts;
2. stabilize the framework core;
3. implement the minimal standard component layer;
4. complete Phase 6 modality/scroll integration and deferred infrastructure;
5. verify the framework through the chess client;
6. extend the framework only when a future application provides a real generic requirement.
```

The framework's scope should evolve from actual reusable requirements rather than speculative completeness.

---

## 14. Document Roles

`docs/FRAMEWORK_SCOPE.md`

Defines why the framework exists, what application class it targets, and what belongs or does not belong in the framework.

`docs/ROADMAP.md`

Defines development phases, dependencies and high-level exit criteria.

`docs/ARCHITECTURE.md`

Describes the broader architecture and remains the large architecture document requiring manual review before major edits.

`docs/COMPONENT_DESIGN_GUIDE.md`

Defines the practical rules for designing and reviewing components.

`docs/PHASE5_COMPONENT_ARCHITECTURE_CHECKPOINT.md`

Defines the current Phase 5 component architecture and is the architectural checkpoint for new component work.

`docs/PHASE5_COMPONENT_CATALOG.md`

Defines the current minimal standard component scope.

`docs/PHASE5_SOURCE_AUDIT.md`

Tracks source cleanup and retained subsystem responsibilities.

`docs/PHASE6_MODALITY_REQUIREMENTS.md`

Records requirements extracted from the legacy Modal implementation for Phase 6.

`docs/PRIMITIVES_ROLE.md`

Defines the role and boundary of the low-level rendering primitives.

When a new development context is opened, `FRAMEWORK_SCOPE.md` should be read early so implementation decisions are evaluated against the project's actual purpose.
