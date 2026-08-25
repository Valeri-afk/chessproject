# Text System

## Current architecture

The active text path is:

```text
Typography / text-bearing controls
        ↓
    TextContent
      ↙     ↘
 TextLayout  TextRenderer
     ↓           ↓
logical      SDL_ttf
measurement
```

`TextContent` is an internal bridge owning logical text state and the Measure/Arrange/Draw connection. Components do not expose `TextLayoutResult`, renderer state, `TTF_Text`, or raster-font caches.

## Typography

`Typography` is the single public standalone text component. There is no separate `Heading` / `Paragraph` component hierarchy.

Variants include semantic styles such as:

```text
INHERIT
H1–H6
SUBTITLE1/2
BODY1/2
BUTTON
CAPTION
OVERLINE
```

Variants are typography policy/semantic metadata, not independent layout algorithms.

## Text-bearing controls

`Button`, `MenuItem` and `TabItem` use internal shared text state rather than creating a retained `Typography` child merely to display a string.

The same logical text contract should be reused by future text-bearing controls.

## TextLayout

`TextLayout` owns logical text measurement and wrapping:

```text
logical font size
logical line height
wrap mode
available logical width
alignment-independent desired size
line metrics needed by current Typography
```

Measure resolves text against the available logical content width. Arrange resolves it again against the actual allocated logical size because wrapping width may change after parent allocation.

The renderer is not the layout owner.

## TextLayoutResult

The internal layout result contains the currently required logical metadata such as:

```text
desiredSize
lineHeight
lineCount
```

It is intentionally not a generic `Node` field and does not expose glyph runs, baselines, caret data or SDL text objects until concrete features require them.

## TextRenderer

`TextRenderer` is internal/backend-only. It owns physical/rendering concerns:

```text
TTF_TextEngine
TTF_Text
derived physical raster fonts
font-generation cache refresh
logical → physical render conversion
SDL renderer-state isolation
actual SDL_ttf drawing
```

It does not own wrapping, desired-size calculation, Measure, Arrange or semantic alignment policy.

## Logical → physical rendering

The client uses a 1920×1080 logical UI space with SDL logical presentation and letterboxing.

Text size is expressed in logical coordinates and converted to physical raster size before SDL_ttf rasterization:

```text
logical font size
    × presentation scale
    ↓
physical raster font size
    ↓
SDL_ttf rasterization
    ↓
physical rendering
```

The renderer must not normally rasterize a tiny bitmap and then enlarge that bitmap.

## Font ownership

The source `TTF_Font*` is client-owned and non-owning from the framework perspective.

The framework owns derived renderer resources such as copied raster fonts, `TTF_TextEngine` and `TTF_Text`.

The source font must outlive every text user. No general font ResourceManager exists yet.

## Font mutation

SDL_ttf font generation tracking is only a backend cache-consistency mechanism. It tells the renderer whether a derived physical font needs refresh.

It does not invalidate layout.

If a source font mutation may change metrics, wrapping or desired size, the affected component/client must explicitly invalidate layout.

## Wrapping

Wrapping is a layout concern and must be consistently represented through Measure, Arrange and Draw. A future `WrapMode` may expose `WRAP` / `NO_WRAP` and later truncation/ellipsis policies, but these should only be added when the full contract is implemented.

## Deferred text input

The framework currently has no editable text/input-control subsystem. This document deliberately does not define implementation yet.

A future text-input system will need to integrate with:

```text
focus
keyboard input
text composition / IME if required
caret/selection
editing state
TextLayout
rendering
```

Do not add those fields to base `Node` merely in anticipation.

## Non-goals

```text
public TextRenderer API
font ResourceManager
rich text spans
text selection
editable text
ellipsis
advanced typography theme inheritance
font-weight/style system without a concrete policy
```
