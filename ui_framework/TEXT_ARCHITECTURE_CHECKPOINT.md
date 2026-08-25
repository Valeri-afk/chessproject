# Text architecture checkpoint

> Branch: `recovery-before-node-tree-break`
> Status: **build-validated; runtime validation remains**

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

## What is completed

The following text-architecture work is complete and has reached build validation:

- `Typography` is the single public typography component; separate `Paragraph` / `Heading` components are not used.
- `Button`, `MenuItem`, and `TabItem` use internal `TextContent` directly.
- `TextLayout` owns logical measurement, wrapping, desired size, and line metrics.
- `TextContent::arrange()` resolves the text against the actual allocated logical size and alignment.
- `TextRenderer` owns SDL_ttf rendering and derived physical resources.
- `TextPrimitive`, `TextNode`, and `TextRuntime` are removed from the active architecture/build graph.
- Legacy `TextRenderState` and unused internal layout state have been removed.
- Component/public APIs do not expose renderer/backend state.

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
8 logical px
    |
    | presentation scale = 2
    v
16 px raster font
    |
    v
SDL_ttf rasterization
    |
    v
physical text rendering
```

The renderer must not rasterize an 8px font and subsequently enlarge its bitmap to 16px.

For integer logical presentation, the renderer temporarily operates in a physical presentation scope and restores logical presentation, viewport, clip and render scale afterward.

## Resource ownership

The source `TTF_Font*` remains client-owned and non-owning from the framework perspective.

The framework owns derived resources created by `TextRenderer`:

```text
TTF_TextEngine
TTF_Text
copied raster TTF_Font
```

The source font must outlive every text user. The current client already destroys UI users before closing the font, so no ResourceManager is justified yet.

The complete provisional ownership contract is documented in `TEXT_RESOURCE_LIFETIME.md`.

## Component policy

Text-bearing controls use `TextContent` directly instead of creating a child Typography node solely for a label.

```text
Button
  own button policy + TextContent

MenuItem
  own menu policy + TextContent

TabItem
  own tab policy + TextContent
```

## TextPrimitive decision

`TextPrimitive` is no longer an architectural concept. The remaining implementation responsibility is renderer/backend work and is named `TextRenderer` internally.

`TextRenderer` must not become public API. If the final implementation can be simplified further without a persistent renderer object, it may be folded into the text subsystem later.

## Build validation checkpoint

The framework and client now compile successfully after the current source/include cleanup and the manual restoration of the full `node_tree.cpp` implementation.

Important recovery note:

- `node_tree.cpp` is intentionally **not modified by automation** and must remain the manually restored full implementation.
- `input_system.cpp` is intentionally **not modified by automation** and requires the small manual include fix documented during recovery (`node_tree.hpp` and `event_dispatcher.hpp`).
- These manual edits are part of the intended working state even if they are not yet represented by this documentation commit.

## Next work

The next stage is **runtime validation / stabilization**, not another text architecture redesign.

Required checks:

1. Typography render smoke test.
2. Button/MenuItem/TabItem text rendering and alignment.
3. Wrapping and logical-size behavior.
4. Font replacement and text replacement resource refresh.
5. SDL renderer-state restoration.
6. Source-font lifetime boundary: client owns `TTF_Font*`; framework owns derived resources.
7. NodeTree/input/layout/render integration smoke tests.
8. Modal and scroll interaction tests.
9. Update this checkpoint after runtime validation with the actual tested scenarios.

## Explicit non-goals

- no general font ResourceManager
- no public TextRenderer API
- no generic text-specific fields on `Node`
- no separate Paragraph/Heading component hierarchy
- no separate paint invalidation queue
- no premature theme/typography token system
