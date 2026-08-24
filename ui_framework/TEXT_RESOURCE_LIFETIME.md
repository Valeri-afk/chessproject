# Text resource lifetime contract

## Current ownership boundary

The framework currently does **not** own the source `TTF_Font` passed to text components.

```text
Client
  ├── TTF_Init()
  ├── TTF_OpenFont(...)
  │       ↓
  │    TTF_Font*
  │       ↓ non-owning
  Framework / TextContent / TextRenderer
  │
  └── TTF_CloseFont()
```

The framework owns resources that it derives from the source font:

```text
TTF_TextEngine
TTF_Text
copied raster TTF_Font
```

These are released by the internal `TextRenderer` before its containing `TextContent` is destroyed.

## Required lifetime rule

A client-owned `TTF_Font*` must remain alive for as long as any framework text component that references it may be measured, arranged, or drawn.

The framework does not retain ownership and does not currently add reference counting to `TTF_Font`.

Closing the source font while a component still references it is invalid client behavior and can result in use-after-free during Measure, Arrange, or Draw.

## Current chess client use case

The current chess client demonstrates a valid ordering:

```text
TTF_OpenFont()
    ↓
create/configure UI components
    ↓
run UIManager
    ↓
destroy UIManager and its TextContent/TextRenderer objects
    ↓
TTF_CloseFont()
    ↓
TTF_Quit()
```

This means the current client-owned source-resource model is sufficient for the existing use case.

## Why there is no framework ResourceManager for fonts yet

A general font `ResourceManager` is intentionally not introduced at this point.

The current framework does not yet have enough demonstrated resource-sharing/lifetime use cases to justify adding another ownership system merely for fonts.

The framework can safely own derived renderer resources while leaving source asset ownership to the client.

## Important distinction: renderer cache vs layout validity

`TextRenderer` tracks the SDL_ttf font generation when it creates a physical raster-font copy.

That generation tracking answers:

> Does the renderer's derived physical font need to be rebuilt?

It does **not** answer:

> Did the logical metrics change enough to require Measure/Arrange again?

If the client mutates a source `TTF_Font` in a way that may affect metrics, wrapping, line height, or desired size, the affected component must explicitly invalidate layout.

```text
font mutation
    ├── renderer sees generation change → refresh derived raster resource
    └── client/framework state change → invalidateLayout() when geometry may change
```

The two mechanisms intentionally remain separate.

## Known unresolved edge case

The current boundary becomes fragile when a source font is shared by many components and its lifetime is shorter than the UI tree lifetime.

Possible future solutions include:

```text
shared font handle
framework-owned font resource
reference-counted font wrapper
font registry/resource manager
```

None is selected yet because there is not currently a concrete use case requiring that additional ownership layer.

## Decision rule for future changes

Do not introduce a resource manager merely because `TTF_Font*` is non-owning.

Introduce stronger resource ownership only when a real use case requires one or more of:

```text
font lifetime independent from individual client scopes
shared ownership across unrelated UI trees
runtime font unloading
font replacement while components remain alive
resource hot-reload
central asset lifetime management
```

Until then, client-owned source fonts plus framework-owned derived rendering resources remain the intended contract.
