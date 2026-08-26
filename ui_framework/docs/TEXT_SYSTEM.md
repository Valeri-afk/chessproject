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
font-backed    SDL_ttf
measurement   rasterization/draw
```

`TextContent` is an internal bridge owning logical text presentation state and the Measure/Arrange/Draw connection. Components do not expose `TextLayoutResult`, renderer state, `TTF_Text`, or raster-font caches.

The important architectural distinction is:

```text
TextLayout
    = text state + logical measurement/wrapping
      using font metrics

TextContent
    = component-facing orchestration/presentation state

TextRenderer
    = physical rasterization + SDL renderer integration
```

`TextLayout` is therefore a logical measurement layer, but it is **not a backend-independent typography engine**. Its current implementation obtains font metrics through SDL_ttf. This is intentional for the current framework and should not be mistaken for a future-proof typography abstraction.

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

## TextLayout architecture

`TextLayout` owns the retained text measurement state:

```text
text
source font (non-owning)
logical font size
logical line height
wrap mode
```

Its public operation is measurement:

```text
available logical width
        ↓
SDL_ttf font metrics
        ↓
TextLayoutResult
```

The implementation may create a temporary copied font when the requested logical font size or line height differs from the source font. That copy belongs only to the measurement operation.

This keeps font mutation local and prevents measurement from mutating the caller-owned source font.

### Why TextLayout is more complex than LayoutSystem

LayoutSystem operates on framework geometry primitives that it owns directly. Text measurement cannot determine its result without a font-metrics provider.

The current implementation therefore has an intentional dependency:

```text
TextLayout → SDL_ttf metrics API
TextRenderer → SDL_ttf rendering API
```

while still maintaining the higher-level separation:

```text
TextLayout  ≠ rasterization
TextRenderer ≠ wrapping/desired-size policy
```

A future backend-independent typography layer would be a larger architectural change and should only be introduced when another text backend, richer text model, or a concrete testing/reusability requirement justifies it.

## TextLayoutResult

The internal layout result contains the currently required logical metadata such as:

```text
desiredSize
lineHeight
lineCount
wrapWidth
```

It is intentionally not a generic `Node` field and does not expose glyph runs, baselines, caret data or SDL text objects until concrete features require them.

The result is currently deliberately small. It should not become a catch-all text state object.

## TextRenderer

`TextRenderer` is internal/backend-oriented. It owns physical/rendering concerns:

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

## Measure → Arrange → Draw

The text pipeline follows the framework layout lifecycle:

```text
Measure(available width)
        ↓
desired logical size
        ↓
parent allocates content size
        ↓
Arrange(actual content position/size)
        ↓
remeasure using final width when wrapping requires it
        ↓
Draw(renderer)
```

This second measurement during Arrange is intentional: the parent's final allocation may differ from the width available during Measure.

## Logical → physical rendering

The client uses a logical UI space with SDL logical presentation.

Text size is expressed in logical coordinates and converted to physical raster size before SDL_ttf rasterization when integer presentation scaling is active:

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

Wrapping is a layout concern and must be consistently represented through Measure, Arrange and Draw.

The current supported policies are:

```text
WRAP
NO_WRAP
```

Truncation/ellipsis and richer line-breaking policies are intentionally deferred until a concrete use case requires them.

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
backend-independent typography abstraction without a concrete requirement
```
