# Text architecture checkpoint

> Branch: `recovery-before-node-tree-break`
> Status: **visual/render validation passed; component event model passed; input/modal validation deferred**

## Current boundary

```text
Typography / Button / MenuItem / TabItem
            |
            v
       TextContent
        /       \
       v         v
 TextLayout   TextRenderer
     |             |
 TextLayoutResult SDL_ttf
                   |
             raster font/cache
```

`TextContent` is an internal, lightweight text object. It owns logical text state and the Measure/Arrange/Draw bridge. Components do not receive `TextLayoutResult`, `TextRenderState`, SDL_ttf objects, or renderer cache state.

## Completed architecture work

- `Typography` is the single public typography component; separate `Paragraph` / `Heading` components are not used.
- `Button`, `MenuItem`, and `TabItem` use internal `TextContent` directly.
- `TextLayout` owns logical measurement, wrapping, desired size, and line metrics.
- `TextContent::arrange()` resolves the text against the actual allocated logical size and alignment.
- `TextRenderer` owns SDL_ttf rendering and derived physical resources.
- `TextPrimitive`, `TextNode`, and `TextRuntime` are removed from the active architecture/build graph.
- Legacy `TextRenderState` and unused internal layout state have been removed.
- Component/public APIs do not expose renderer/backend state.
- `events.hpp` is the single event-type header; legacy `event_types.hpp` has been removed.

## Layout responsibility

`TextLayout` owns logical text measurement:

- logical font size
- logical line height
- wrapping
- desired size
- line metrics used by current Typography needs

`TextContent::measure()` resolves the proposed logical width. `TextContent::arrange()` resolves the text again against the actual allocated width and stores the arranged logical result.

Alignment and component geometry remain above the low-level renderer.

## Rendering responsibility

`TextRenderer` is internal and renderer-only. It owns:

- `TTF_TextEngine`
- `TTF_Text`
- derived physical raster-font copies
- font-generation based derived-resource refresh
- physical render conversion
- SDL renderer-state save/restore
- actual SDL_ttf drawing

It does not own Measure, Arrange, wrapping policy, alignment policy, desired-size calculation, or layout invalidation.

## Logical → physical text

The intended path is:

```text
logical font size
      |
      | presentation scale
      v
physical raster font
      |
      v
SDL_ttf rasterization
      |
      v
physical text rendering
```

The renderer must not rasterize a low-resolution font and then enlarge the resulting bitmap as the normal path.

The client currently uses a fixed **1920×1080 logical UI space** with:

```cpp
SDL_SetRenderLogicalPresentation(
    renderer,
    1920,
    1080,
    SDL_LOGICAL_PRESENTATION_LETTERBOX);
```

The physical window/display size is allowed to differ from the logical UI size. SDL performs aspect-preserving letterbox presentation.

## Visual/render validation checkpoint

The current client smoke-test application has been run successfully and the visual result is considered correct for the current stage.

Validated visually:

- Typography variants and normal body text.
- Horizontal alignment including centered and end-aligned text.
- Wrapped long text inside a fixed logical width.
- Text-bearing `Button`, `MenuItem`, and `TabItem` rendering.
- `Button` variants: filled, outlined, and text.
- Dropdown/menu rendering.
- Tab control rendering and selection state.
- Stack/scroll layout presentation.
- Modal/backdrop presentation as a rendering smoke test.
- Logical 1920×1080 presentation in resizable/fullscreen mode using SDL letterboxing.

The client is currently suitable as a visual smoke-test harness rather than a final application UI.

## Component event model checkpoint

There is one framework event registration mechanism on `Node`:

```cpp
node->on<ConcreteEvent>(callback);
```

Component-internal handlers may be registered by the component itself through its protected registration path. This is considered correct because those handlers implement the component's own behavior.

Semantic component callbacks remain convenience/semantic APIs, for example:

```cpp
button->setOnActivate(...);
checkbox->setOnToggle(...);
slider->setOnValueChanged(...);
```

The intended separation is:

```text
framework event registration
        |
        +--> component internal behavior
        |
        +--> client custom behavior

component semantic operation
        |
        +--> semantic component callback
```

The client does not need to know which low-level events a component uses internally.

The event model is considered architecturally acceptable at the current stage. No additional event abstraction should be introduced until a concrete requirement appears.

## Resource ownership

The source `TTF_Font*` remains client-owned and non-owning from the framework perspective.

The framework owns derived resources created by `TextRenderer`:

```text
TTF_TextEngine
TTF_Text
copied raster TTF_Font
```

The source font must outlive every text user. The current client already destroys UI users before closing the font, so no ResourceManager is justified yet.

## TextPrimitive decision

`TextPrimitive` is no longer an architectural concept. The remaining implementation responsibility is renderer/backend work and is named `TextRenderer` internally.

`TextRenderer` must not become public API. If the final implementation can be simplified further without a persistent renderer object, it may be folded into the text subsystem later.

## Recovery state / protected files

`node_tree.cpp` and `input_system.cpp` remain protected recovery files. They were manually restored/adjusted during recovery and must not be automatically rewritten as part of incremental cleanup.

Small include/signature fixes in surrounding files are acceptable. For changes requiring replacement of either protected large file, the edit must be performed manually.

## Next phase

The next phase is **component API validation followed by input/modal stabilization**.

### Component API validation

Review all public components for:

- stale legacy API;
- duplicated semantic callbacks;
- correct Measure/Arrange/Draw responsibilities;
- redundant Node-level APIs;
- correct ownership of component-local state.

Components in scope:

```text
Button
Checkbox
RadioButton
Slider
ToggleButton
Menu
MenuItem
Dropdown
TabControl
TabItem
Typography
StackPanelNode
PanelNode
```

### Deferred input/modal work

Do not mix this into visual validation. Later validate separately:

- MouseDown / MouseUp / MouseClick sequencing;
- MouseEnter / MouseLeave;
- capture;
- focus;
- drag;
- modal input restriction;
- overlay hit testing;
- modal/backdrop behavior.

## Explicit non-goals

- no general font ResourceManager
- no public TextRenderer API
- no generic text-specific fields on `Node`
- no separate Paragraph/Heading component hierarchy
- no separate paint invalidation queue
- no premature theme/typography token system
- no second generic event system alongside `Node::on<Event>()`
