# UI Framework

C++20 UI framework built on SDL3.

The repository contains the current framework source code and the focused architectural contracts used during development.

## Documentation

Start with:

- [Framework Scope](docs/FRAMEWORK_SCOPE.md)
- [Layout System](docs/LAYOUT_SYSTEM.md)
- [Lifetime and Mutations](docs/LIFETIME_AND_MUTATIONS.md)
- [Deferred Operations](docs/DEFERRED_OPERATIONS.md)
- [Component Design](docs/COMPONENT_DESIGN.md)
- [Input System](docs/INPUT_SYSTEM.md)
- [Event Dispatching](docs/EVENT_DISPATCHING.md)
- [Rendering System](docs/RENDERING_SYSTEM.md)
- [Modality System](docs/MODALITY_SYSTEM.md)
- [Scroll System](docs/SCROLL_SYSTEM.md)
- [Text System](docs/TEXT_SYSTEM.md)
- [Future Text Input System](docs/TEXT_INPUT_SYSTEM.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Development Instructions](docs/INSTRUCTIONS.md)

`ARCHITECTURE.md` is the large architecture document and is intentionally maintained separately from the focused subsystem contracts.

The source code is the authoritative source of truth for current behavior. The focused documents describe stable contracts, ownership boundaries, deliberate non-goals and future boundaries; they are not phase checkpoints.

## Current state

The core runtime, Measure → Arrange layout, explicit layout invalidation, rendering, event registration, modality and scrolling are implemented and have been validated in the chess client at the current development boundary.

Input/modal sequencing and future editable text input remain separate work. The text rendering/layout stack is implemented through `Typography`, `TextContent`, `TextLayout` and internal `TextRenderer`.
