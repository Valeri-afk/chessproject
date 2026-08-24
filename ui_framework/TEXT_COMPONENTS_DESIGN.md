# Typography Design

## Goal

Use one framework text component, `Typography`, instead of separate `Heading` and `Paragraph` components.

The model is inspired by Material UI's `Typography`: the component represents text plus a semantic typography `variant`, while typography presets are a separate policy from the text layout/rendering machinery.

The framework should copy the useful idea, not the web-specific API.

```text
Typography
    │
    ├── text content
    ├── typography variant
    ├── font resource
    ├── color
    ├── alignment
    └── layout/wrapping properties
         │
         ▼
    TextLayout
         │
         ▼
    Measure / Arrange
         │
         ▼
    physical rasterization
         │
         ▼
    SDL_ttf backend
```

`TextPrimitive` is not the architectural center. It remains legacy/backend rendering machinery until the text migration is complete.

## 1. Material UI reference

Material UI's `Typography` uses one component with a `variant` property instead of separate components for headings, body text, captions, etc. Its standard variants include heading levels, subtitles, body text, button text, caption, and overline. Typography configuration also includes properties such as font family, font size, font weight, line height, and letter spacing.

The important architectural idea for this framework is:

```text
one text component
        +
semantic/style variant
        +
shared layout/rendering
```

not a hierarchy of `Heading`, `Paragraph`, `Caption`, `Label`, `BodyText`, and similar classes.

MUI's web-specific properties such as `component`, `sx`, and HTML element mapping are intentionally not part of this framework.

## 2. `Typography`

`Typography` is the single standard standalone text component.

Its initial variant set is:

```text
INHERIT
H1
H2
H3
H4
H5
H6
SUBTITLE1
SUBTITLE2
BODY1
BODY2
BUTTON
CAPTION
OVERLINE
```

The exact presets are not yet tied to concrete font resources or numeric sizes.

This is deliberate: the framework currently has no typography/theme/resource policy that can safely manufacture and own a canonical set of `TTF_Font*` resources.

### Component properties

The initial component-level properties are:

```text
text
variant
font
color
horizontal alignment
vertical alignment
```

These come from the existing `TextNode`/`TextLayout` contract, with `variant` being the new typography-specific property.

Future typography-specific properties may include:

```text
font family / font resource selection
font size
font weight
font style
line height
letter spacing
```

but these should be introduced through a coherent typography/style policy rather than as ad-hoc properties on every text component.

### Wrapping / no-wrap

MUI exposes `noWrap`. The framework should eventually have an equivalent capability, but it should be introduced as part of the text layout contract rather than as a rendering-only boolean.

A future model could be:

```text
WrapMode
    WRAP
    NO_WRAP
```

possibly followed later by ellipsis/truncation policies.

For the current migration, do not add `noWrap` until `TextLayout` has an explicit wrapping policy. The existing text layout already performs width-constrained wrapping, so a public property needs to be propagated consistently through Measure, Arrange, and Draw.

## 3. Typography variants are policy, not layout

A variant should describe intended typography semantics:

```text
H1/H2/...       → heading hierarchy
SUBTITLE1/2     → supporting titles
BODY1/2         → normal body text
BUTTON          → action/control text
CAPTION         → secondary small text
OVERLINE        → auxiliary label text
INHERIT         → inherit/use supplied typography
```

The variant must not implement a second layout algorithm.

Conceptually:

```text
Typography::Variant
        ↓
TypographyPolicy   [future]
        ↓
font/style metrics
        ↓
TextLayout
```

This allows the framework to introduce a theme/material-like typography system later without coupling it to `Measure/Arrange` or SDL_ttf.

## 4. No `Heading` / `Paragraph` components

Separate `Heading` and `Paragraph` components are intentionally removed.

Their use cases map to `Typography` variants:

```text
Heading H1      → Typography(H1)
Heading H2      → Typography(H2)
...
Paragraph       → Typography(BODY1/BODY2)
Caption         → Typography(CAPTION)
Button text     → Typography(BUTTON)
```

There is no reason for a paragraph to own a separate layout engine. Multi-line body text is simply a typography variant combined with the normal text wrapping policy.

Likewise, a heading is not a different layout primitive. It is a semantic typography variant.

## 5. Text embedded in controls

Controls that display text should use the same text-layout contract as `Typography`, but they do not need a `Typography` child node.

```text
Button
    └── shared text state/layout

MenuItem
    └── shared text state/layout

TabItem
    └── shared text state/layout

Tooltip
    └── shared text state/layout

Typography
    └── same shared text layout
```

A text-bearing control may choose an appropriate typography variant internally, for example `BUTTON`, without creating a retained child node.

## 6. Measure / Arrange contract

Typography participates in the open framework layout model:

```text
Measure
    available logical content width
        ↓
    wrapping / text measurement
        ↓
    desired logical size

Arrange
    final logical content box
        ↓
    final text placement

Draw
    renderer/backend scale
        ↓
    physical rasterization
```

The current public `TextLayout::measure()` should remain a logical measurement operation. Do not expose line boxes, glyph runs, baselines, hit-testing structures, or `TTF_Text` through the public layout API until concrete features require them.

There is currently some duplicated work between logical measurement and SDL_ttf rendering. This is an implementation concern to resolve when the backend is reshaped; it is not a reason to prematurely expand the public layout contract.

## 7. Logical → physical typography

Typography sizes are expressed in framework logical coordinates.

When the renderer has a presentation scale, the logical font size must be converted to physical raster size **before** rasterization.

```text
8 logical px
    × 2x presentation scale
        ↓
16 physical raster px
        ↓
SDL_ttf rasterization
```

This avoids rasterizing an 8px bitmap and then enlarging that bitmap to 16px.

The existing `TextPrimitive` already contains part of this backend behavior through its copied raster font and presentation scale. That implementation should be preserved semantically while its layout responsibilities are moved out of the legacy primitive.

## 8. Resource ownership

The current resource boundary remains:

```text
Client
    │
    ├── creates SDL / SDL_ttf resources
    └── owns source resource lifetime
            │
            ▼
Framework
    └── consumes TTF_Font*
```

`TTF_Font*` stored by `TextLayout` is non-owning.

Framework/backend may own derived resources such as:

```text
raster font copy
TTF_TextEngine
TTF_Text
```

The exact lifetime relationship between client-owned source resources and framework-owned derived caches remains intentionally unresolved. The current documentation must not imply that a general resource manager exists.

The client should keep a source `TTF_Font*` alive for as long as the framework can use it. Framework-owned derived resources are released by the framework/backend.

## 9. Font mutation and invalidation

`TTF_GetFontGeneration()` is a backend cache-consistency mechanism, not a layout invalidation mechanism.

```text
client mutates TTF_Font
        │
        ├── backend cache
        │      └── generation → refresh derived resource
        │
        └── framework layout
               └── client → invalidateLayout()
```

A font mutation may change glyph metrics and therefore affect measured size or wrapping. The framework must not silently invalidate layout as a side effect of the resource mutation.

If the client mutates a font in a way that can affect geometry, it is responsible for explicitly invalidating affected layout.

This preserves the framework rule that setters and resource mutation do not automatically introduce hidden invalidation side effects.

## 10. Initial typography policy

Do not introduce concrete Material-like numeric values yet.

The eventual typography policy should probably be a separate framework-level object/theme rather than hardcoded inside `Typography`:

```text
Typography
    variant = H2
        ↓
TypographyTheme / TypographyPolicy
        ↓
font family
font size
font weight
line height
letter spacing
        ↓
resolved text style
        ↓
TextLayout
```

This also gives us a clean place to solve the future question of who owns the default font resources.

Until that system exists, the variant remains semantic metadata and the client-supplied `TTF_Font*` remains the actual rendering resource.

## 11. Deliberately deferred features

Do not add these merely to imitate Material UI:

```text
HTML component mapping
sx/style-object system
rich text spans
text selection
editable text
ellipsis
advanced typography theme inheritance
font resource manager
```

Each should follow an actual framework use case.

The next relevant extension after the base migration is likely `WrapMode`, followed by a real typography policy/theme once resource ownership is understood.

## 12. Refactor order

1. Keep `TextNode`/shared text implementation stable while the new typography API is introduced.
2. Use `Typography` as the single public standalone text component.
3. Remove `Heading` and `Paragraph` as separate components.
4. Migrate text-bearing controls toward the shared typography/text-layout contract.
5. Separate all layout semantics from `TextPrimitive`.
6. Preserve rasterization/cache responsibilities in the backend.
7. Revisit whether `TextPrimitive` can disappear after the new text pipeline is stable.
8. Only then introduce a framework-wide typography policy/theme if real use cases justify it.
