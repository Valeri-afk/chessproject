# Text architecture checkpoint

> Branch: `fix/sharp-logical-text`
> Status: implementation checkpoint before build validation

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

There is one standalone public typography component rather than separate public Paragraph/Heading classes. Variants provide a compact baseline vocabulary; the component remains thin.

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

## Remaining work before build

1. Verify all text-bearing components use `TextContent`.
2. Remove stale `TextPrimitive` includes/files from the build graph.
3. Verify `text_renderer.cpp` and its header are consistent.
4. Verify no component exposes `TextLayoutResult` or `TextRenderState`.
5. Remove obsolete `TextNode` role only after repository-wide usage is confirmed absent.
6. Compile framework and client.
7. Fix compiler/API mismatches.
8. Run focused logical-scale and renderer-state tests.

## Explicit non-goals at this checkpoint

- no general font ResourceManager
- no public TextRenderer API
- no generic text-specific fields on `Node`
- no separate Paragraph/Heading component hierarchy
- no separate paint invalidation queue
- no premature theme/typography token system
