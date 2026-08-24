# Text Components Design

## Goal

Define a small set of framework-provided text components that cover the common standalone text use cases without turning typography into a large widget hierarchy.

The design separates:

```text
text semantics / component API
        ↓
shared text layout behavior
        ↓
logical → physical rendering preparation
        ↓
SDL_ttf rendering backend
```

`TextPrimitive` is not treated as the architectural center. It is considered legacy backend/rendering machinery from the previous closed layout model and is expected to be reshaped or removed after the new text contract is established.

## 1. Reference model

Established UI frameworks commonly provide a small simple-text component plus richer text/document abstractions rather than a separate widget class for every typographic style.

Examples:

- Flutter has `Text` for normal single-string text and `RichText` for multi-style text spans. `SelectableText` is a capability-oriented interaction variant rather than a separate typography hierarchy.
- Qt uses `QLabel` for ordinary text/image display and reserves document-oriented controls such as `QTextEdit`/`QTextBrowser` for richer or larger text content.
- WPF separates lightweight `TextBlock`/`Label` use from `FlowDocument`/`Paragraph` when document-style flow and rich content are required.

These frameworks are references rather than architectural authorities. The target is a smaller retained-mode C++/SDL framework.

## 2. Proposed minimal standard set

### `TextNode` → rename/reshape toward `Text`

Purpose:

```text
ordinary standalone text
short UI strings
labels
captions when no special semantic heading contract is needed
```

This is the general-purpose text component.

Core state:

```text
text
font
text color
horizontal alignment
vertical alignment
wrapping policy
```

It participates in the normal framework `Measure/Arrange/Draw` lifecycle.

There should be no requirement for a component such as Button/MenuItem/TabItem to contain a `Text` child merely to display text. Text-bearing components may own a shared text-layout state internally.

### `Heading`

Purpose:

```text
screen/page section titles
panel/card titles
modal titles
major UI hierarchy labels
```

`Heading` is a semantic typography component rather than merely a larger `Text`.

The initial design should use a small `HeadingLevel` enum, for example:

```text
H1
H2
H3
H4
```

Do not expose six or more levels until a real application demonstrates the need.

Heading level should map to framework typography defaults, while allowing the same normal text properties (font override, color, alignment, wrapping) where appropriate.

The framework should not require clients to reproduce the default heading sizes themselves.

### `Paragraph`

Purpose:

```text
multi-line body copy
help/rules text
settings descriptions
tooltips with longer text
documentation-like UI content
```

`Paragraph` is a convenience semantic component for body text whose normal expectation is wrapping and multi-line layout.

It should not introduce a second independent text-layout engine. It uses the same shared text-layout/backend contract as `Text` and differs primarily through defaults/policy:

```text
Text       → generic text defaults
Heading    → hierarchical heading defaults
Paragraph  → body-copy / wrapping defaults
```

The component should remain small.

## 3. What should NOT be a separate standard component yet

Do not create separate framework components for:

```text
Label
Caption
Title
Subtitle
SmallText
BodyText
DisplayText
Footnote
Overline
```

These are better represented by typography defaults/style or by `Text`/`Paragraph` until concrete application requirements establish a distinct semantic contract.

Do not create `RichText` in the first text migration stage unless the framework has an actual multi-style-inline requirement. Rich text introduces a larger model of spans/runs, selection and document semantics and should follow a concrete use case.

Do not create `TextField`/`InputField` as a simple text component. Editing, cursor, selection, composition, scrolling and input state are a separate subsystem and are already intentionally deferred.

## 4. Shared text contract

All three proposed components should use the same underlying text layout behavior.

Conceptually:

```text
Text / Heading / Paragraph
        │
        ▼
shared TextLayout state/operation
        │
        ├── text
        ├── font
        ├── wrap constraint
        ├── measured logical size
        ├── line/layout information
        └── final placement/alignment inputs
        │
        ▼
rendering preparation
        │
        ├── framework logical coordinates
        ├── renderer presentation scale
        └── physical raster font size
        │
        ▼
SDL_ttf backend
```

The exact class decomposition is intentionally not frozen yet. The key contract is that text layout works in framework logical coordinates and rendering converts logical font size to physical raster size before rasterization when a presentation scale requires it.

## 5. Standalone text vs text embedded in components

A standard component should not be forced to use `Text` as a child node merely because it displays text.

Preferred model:

```text
Button
    └── shared text state/layout operation

MenuItem
    └── shared text state/layout operation

TabItem
    └── shared text state/layout operation

Tooltip
    └── shared text state/layout operation

Text
    └── same shared text layout

Heading
    └── same shared text layout

Paragraph
    └── same shared text layout
```

This follows the framework's existing primitive/component boundary: a visual primitive or shared subsystem may be used internally without becoming a retained child `Node`. The component design guide explicitly allows text to remain an internal primitive rather than requiring a `TextNode` child.

## 6. Measure / Arrange contract

Text measurement is part of the open framework layout model.

For standalone text components:

```text
Measure
    available logical content width
        ↓
    text wrapping / line layout
        ↓
    desired logical content size

Arrange
    final logical content box
        ↓
    final text placement/alignment

Draw
    consume the prepared layout using renderer/backend scale
```

A text setter remains pure mutation. Client code may explicitly invalidate layout after runtime changes according to the framework invalidation contract.

Semantic component operations may use the protected component invalidation hook when appropriate.

## 7. Logical vs physical text size

The framework's UI coordinate space is logical when SDL logical presentation is configured.

Therefore a logical font size must not be rasterized at the logical pixel size and then enlarged as a bitmap.

Conceptually:

```text
8 logical px font
        ×
2x renderer presentation scale
        ↓
16 physical/raster px font
        ↓
SDL_ttf rasterization
        ↓
physical rendering
```

The important rule is:

```text
convert logical font size → physical raster size BEFORE rasterization
```

This avoids the quality loss caused by rasterizing an 8px glyph and subsequently scaling the resulting bitmap to 16px.

The current `TextPrimitive` contains an implementation of this principle through a copied raster font and integer presentation scale. That implementation is considered legacy/backend machinery and should be preserved semantically while being separated from the new text layout ownership model.

## 8. Resource ownership

### Current boundary

At the current stage, the intended resource boundary is:

```text
Client
    │
    ├── creates SDL / SDL_ttf resources
    └── is responsible for their lifetime
            │
            ▼
Framework
    └── consumes resources through the public API
```

In particular, `TTF_Font*` may be created and owned by the client and passed into framework components such as `TextNode`, `Button`, `MenuItem`, and `TabItem`.

This is the **current working boundary**, not a final resource-management architecture.

### Current ambiguity / unresolved responsibility

There are not yet enough framework use cases to prove that client-owned resource lifetime remains sufficient in every scenario. In particular, derived and cached resources introduce a second lifetime domain:

```text
client-owned TTF_Font*
        │
        ▼
framework/backend caches
        ├── copied raster font
        ├── TTF_TextEngine
        └── TTF_Text
```

The framework may own these derived/cache objects, but the exact lifetime relationship between the source client resource and framework-owned derived objects is not yet a fully specified public contract.

Therefore the documentation must treat resource lifetime responsibility as **currently ambiguous at the edge between client-owned source resources and framework-owned derived/cache resources**.

The framework must not silently imply that destroying a client-owned `TTF_Font*` is always safe while a framework cache still depends on it, nor should the current implementation be interpreted as establishing a general resource manager.

### Font mutation and invalidation

`TTF_Font*` is also mutable state from the framework's perspective. The backend currently uses `TTF_GetFontGeneration()` to detect source-font changes when deciding whether its derived raster-font cache is still valid. This is a **backend cache-consistency mechanism**, not a layout invalidation mechanism.

The distinction is intentional:

```text
client mutates TTF_Font
        │
        ├── backend raster cache
        │      └── generation can trigger cache refresh
        │
        └── framework layout
               └── does NOT auto-invalidate
```

A font mutation can change glyph metrics and therefore change measured logical size or wrapping. Since `TextLayout` does not own the source resource and does not automatically invalidate the containing node, the client remains responsible for calling `invalidateLayout()` when a font mutation can affect geometry.

This preserves the framework rule that setters/resource mutation do not secretly introduce framework invalidation side effects. Backend cache refresh and layout invalidation are separate responsibilities.

### What is intentionally NOT being introduced now

Do not introduce a general `FontManager`, `ResourceManager`, or opaque resource-handle system solely to complete the text/layout refactor.

The present goal is to preserve the existing client-owned source-resource boundary while explicitly recording the unresolved lifecycle edge so it can be revisited when real use cases expose a limitation.

### Practical current rule

Until this boundary is revisited:

- the client should keep every source `TTF_Font*` alive for as long as any framework component/backend operation may use it;
- framework-owned derived objects must be released by the framework/backend according to their own implementation lifetime;
- no assumption should be made that framework caches extend the lifetime of a client-owned source resource;
- a client mutation of `TTF_Font` that may affect metrics/geometry should be followed by explicit `invalidateLayout()` on affected components;
- a backend generation check does not replace explicit layout invalidation;
- no assumption should be made that a future resource manager will necessarily replace this boundary.

## 9. Initial implementation order

1. Establish the shared logical text layout contract.
2. Separate text layout semantics from SDL_ttf rendering/backend concerns.
3. Rework standalone `TextNode` into the final `Text` role.
4. Add `Heading` using the same text layout contract.
5. Add `Paragraph` using the same text layout contract with body/wrapping defaults.
6. Migrate Button/MenuItem/TabItem/Dropdown/Tooltip text handling to the shared contract where useful.
7. Only then revisit `TextPrimitive`/`TextRuntime` removal or final reshaping.
8. Compile/runtime validation happens after the text migration is internally coherent.

## 10. Open decisions

The following are deliberately not frozen yet:

- exact name of the shared text layout type;
- whether `Text` replaces `TextNode` or `TextNode` remains the public class name;
- exact `HeadingLevel` set;
- whether `Paragraph` needs properties beyond wrapping and default typography;
- exact relationship between layout cache and SDL_ttf `TTF_Text` object;
- exact lifetime model for derived raster fonts and text engines;
- whether rich text belongs in this framework at all;
- how text selection should be introduced later for input/selection scenarios;
- whether the current client-owned resource boundary is sufficient once the framework has more long-lived/shared/cached resource use cases.
