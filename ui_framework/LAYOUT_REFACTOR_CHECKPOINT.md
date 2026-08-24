# Measure / Arrange Refactor Checkpoint

> **Status:** active implementation checkpoint
> **Branch:** `fix/sharp-logical-text`
> **Purpose:** freeze the architectural decisions reached during the Measure/Arrange, imperative invalidation, and text-layout investigation; distinguish settled semantics from implementation work; and define the remaining completion criteria.

## 1. Goal

The refactor is intended to make component-specific layout behavior open and imperative without making the framework's runtime model open-ended.

The target is:

```text
Framework owns
    tree ownership and lifecycle
    traversal
    framework-known layout semantics
    measurement constraints
    layout scheduling
    geometry commit
    rendering traversal
    input/runtime machinery

Component owns
    component-specific state
    custom Measure policy
    custom Arrange policy
    custom Draw policy
    explicit semantic notifications
```

The concrete motivation is to stop requiring the framework to know and centrally register every component-specific property merely because that property can affect layout or paint.

## 2. Decisions already settled

### 2.1 Measure is universal

`Node` has Measure semantics regardless of whether it owns children.

```text
Leaf Node
    → measure own content

PanelNode
    → may recursively measure children
    → aggregates child desired sizes
    → returns own desired content size
```

`measureContent()` remains a useful default primitive for ordinary leaf components.

### 2.2 Arrange is universal

`Node` has Arrange semantics regardless of whether it owns children.

```text
Leaf Node
    → use assigned content geometry

PanelNode
    → choose child allocations
    → ask framework to arrange children
```

Only structural components need child-arrangement capability.

### 2.3 PanelNode is a structural capability

Plain `Node` does not acquire framework-visible children at this stage.

```text
Leaf : Node
Container : PanelNode
```

`PanelNode` primarily means framework-managed child ownership/structure. It is not required to impose one particular layout algorithm.

`StackPanelNode` is a specialized `PanelNode` with a predefined linear layout policy.

### 2.4 Custom layout policy belongs to the component

The framework owns execution and constraints; the component owns the policy.

```text
Custom Panel
    measure()
        → decide how children contribute to desired size

    arrange()
        → decide child allocations
```

The component does not receive direct access to:

```text
NodeTree internals
layout queue internals
mutation queue internals
phase flushing
raw geometry storage
```

### 2.5 Framework-known properties remain framework-owned semantics

The framework continues to interpret properties such as:

```text
size
min/max size
padding
border
position
position mode
overflow
visibility and input state
```

This does not require every framework-known property to be physically stored outside component classes; semantic ownership is what matters.

### 2.6 Component-specific properties remain component-owned

Examples include:

```text
text
font
icon
text/icon spacing
variant
custom sizing modes
colors
selection state
custom layout state
```

Their effects are expressed through the component phase contracts.

### 2.7 Explicit invalidation is the change notification model

The public layout notification is:

```cpp
uiManager.invalidateLayout(node);
```

Property setters do not need to silently schedule layout. Component/client code changes state and explicitly reports the consequence.

This applies to framework-known layout properties as well as custom layout-affecting state for the current design direction.

Exceptions already chosen:

```text
enabled
focusable
capturable
```

do not require layout invalidation merely because their values changed.

### 2.8 `invalidateLayout()` is root-based and coalesced

The existing `NodeTree` mechanism is already the intended implementation:

```text
changed node
    ↓
walk to top-level root / overlay
    ↓
queue root once
```

`layoutQueueSet_` deduplicates repeated invalidations.

Detached/non-live nodes do not become layout jobs.

`UIManager` is only the public facade; `NodeTree` remains the authoritative source of tree membership and queue semantics.

### 2.9 Invalidation is not synchronous

`invalidateLayout()` schedules future work. It does not run Measure/Arrange immediately and does not expose a flush operation.

### 2.10 Re-invalidation during layout is deferred

If `measure()` or `arrange()` causes an invalidation, the current pass is not recursively restarted.

The current queued roots are consumed first; a new invalidation queues the root again for a later framework-controlled pass.

A component that invalidates itself on every Measure/Arrange invocation can therefore cause repeated future passes. This is component behavior, not recursive scheduler execution.

### 2.11 Structural child mutation remains framework-managed

`PanelNode::addChild/removeChild` already use `NodeTree` mutation handling and already trigger the appropriate layout invalidation through the parent/root.

A separate public `treeStructureChanged()` notification is intentionally not part of this stage.

### 2.12 Structural mutations during framework phases are deferred

Current mutation scopes preserve traversal stability.

A structural mutation during Measure/Arrange/Draw is not made visible in the middle of the current traversal. The stable post-mutation tree is observed by later work.

### 2.13 Measure proposal and final allocation are different concepts

This is one of the central contracts:

```text
Measure proposal
    ≠
Arrange allocation
```

A Measure constraint is an upper bound / available bound. A component may report a desired size larger than it.

Arrange receives a size allocation selected by the parent layout policy.

### 2.14 `Auto` is not `fill parent`

Historical Phase 2 documentation and the real linear layout implementation agree:

```text
Auto
    → intrinsic measurement / parent allocation
```

The raw `Auto/Value` representation is not part of generic `MeasureContext`.

`Auto` is interpreted by the surrounding framework/layout policy.

Top-level root sizing may have its own viewport semantics; child `Auto` is parent-layout-dependent.

### 2.15 Constraint ownership is fixed

The framework owns framework-known constraint resolution.

Canonical current semantics:

```text
Fixed size
    → measurement proposal + final size

Max size
    → measurement proposal + final size

Min size
    → final size only

Auto
    → intrinsic measurement / parent allocation
```

In particular, `min` does not automatically become a Measure proposal.

`max` may narrow Measure before width-sensitive content is measured.

### 2.16 Border-box model is retained

The node's outer geometry is a border box.

Measure/Arrange hooks work with content-space geometry while the framework converts between outer and content boxes using padding and border.

Conceptually:

```text
outer proposal
    ↓
subtract padding + border
    ↓
component Measure
    ↓
desired content size
    ↓
add padding + border
    ↓
desired outer size
```

### 2.17 Final constraints happen after parent allocation

Custom layout may choose stretch, centering, spacing, or another allocation policy.

The framework then applies framework-owned final constraints:

```text
parent allocation
    ↓
fixed/min/max resolution
    ↓
actual child size
```

A custom component should not duplicate `resolveFinalSize()` semantics.

Historical stretch behavior confirms this ordering.

### 2.18 `Overflow` is render traversal state

`Overflow::HIDDEN` does not participate in Measure/Arrange.

The current `NodeTree::drawSubtree()` applies nested clipping through renderer-state RAII.

`PanelNode` does not own child render traversal. `NodeTree` owns clipping, traversal ordering, mutation safety, root/overlay ordering, and recursive draw traversal.

### 2.19 No separate `invalidatePaint()` at this stage

Rendering currently runs every frame. There is no demonstrated need for a separate paint-dirty queue yet.

A separate paint invalidation API is deferred until the render pipeline actually requires one.

### 2.20 Derived geometry is cached state, not live computation

`getDesiredSize()` and `getActualSize()` expose the latest committed framework geometry.

After invalidation and before the next layout pass, the previous value remains readable but may be stale.

No explicit geometry-validity flag is required at this stage.

### 2.21 Text is now a separate typography/layout layer

The old standalone `TextNode` role is being replaced by public `Typography`.

The current direction is one typography component with a compact variant model rather than separate `Heading` and `Paragraph` components.

Current semantic variants:

```text
INHERIT
H1 / H2 / H3 / H4 / H5 / H6
SUBTITLE1 / SUBTITLE2
BODY1 / BODY2
BUTTON
CAPTION
OVERLINE
```

The variants currently resolve to small default logical font-size presets. There is intentionally no full Material-style theme/resource system yet.

Current `Typography` responsibilities:

```text
text
font (non-owning client resource)
variant
logical font size
logical line height
wrap mode
horizontal / vertical alignment
color
Measure
Arrange
Draw delegation
```

### 2.22 Logical text size is resolved before rasterization

The required text pipeline is:

```text
logical font size
    ↓
presentation scale
    ↓
physical raster font size
    ↓
SDL_ttf rasterization
```

An 8 logical-pixel font must therefore be rasterized at the required physical size rather than rasterized at 8 physical pixels and then scaled as a bitmap.

The current backend already contains the necessary raster-font copy/generation logic and that behavior must be preserved through the cleanup.

### 2.23 SDL_ttf owns the basic font metrics

The framework should not invent a separate font-metrics engine for basic Typography needs.

The current text layout uses SDL_ttf facilities for:

```text
font size
font height
font ascent/descent when needed later
font line skip
wrapped text size
```

`lineHeight == 0` means native SDL_ttf line skip for the current font.

A public baseline API is intentionally deferred until a concrete inline/rich-text/caret use case requires it.

### 2.24 Text resource ownership is intentionally provisional

The current source-resource boundary is:

```text
Client creates TTF_Font*
    ↓
Client owns its lifetime
    ↓
Framework consumes non-owning TTF_Font*
```

Framework/backend owns derived resources such as copied raster fonts, `TTF_TextEngine`, and `TTF_Text` objects.

The lifetime relationship between client-owned source fonts and framework-owned derived caches is explicitly documented as an unresolved edge case. No general `ResourceManager` is introduced at this stage.

### 2.25 Font mutation and layout invalidation are separate

The backend may inspect `TTF_GetFontGeneration()` to refresh derived raster resources.

That backend cache check does **not** invalidate framework layout.

If the client mutates a font in a way that may change metrics or wrapping, the client/component must explicitly call `invalidateLayout()` on affected nodes.

### 2.26 TextLayoutResult is local to text components

The current text layer introduces:

```text
TextLayoutResult
    desiredSize
    lineHeight
    lineCount
```

`TextLayout::measureLayout()` produces the prepared logical result; the legacy `measure()` remains a convenience wrapper returning only `desiredSize`.

The result is not added to base `Node` or `ArrangeContext` yet because generic Node layout does not require text-specific metadata.

For `Typography`, the intended lifecycle is:

```text
Measure
    → result under Measure constraint

Arrange
    → re-resolve wrapping under actual allocated width

Draw
    → consume the arranged text state
```

This avoids treating the Measure result as final when wrapping width can change during parent allocation.

### 2.27 TextPrimitive is not a layout owner

`TextPrimitive` is explicitly not part of the framework layout contract.

Its current code still contains historical layout-related behavior such as wrap-width setup, rendered-size queries, alignment calculation, and low-level rendering. This is the remaining cleanup target.

The desired endpoint is:

```text
Typography / text component
    ↓
TextLayoutResult + TextRenderState
    ↓
low-level text renderer/backend
    ↓
SDL_ttf + renderer
```

### 2.28 TextPrimitive likely should be renamed after semantic cleanup

The current name `TextPrimitive` reflects the old layout-engine representation and is misleading once it no longer owns layout semantics.

The preferred name after the API cleanup is:

```text
TextRenderer
```

because the surviving responsibility is expected to be:

```text
SDL_ttf text object/cache management
physical raster font preparation
logical → physical render conversion
renderer state handling
actual glyph/text draw
```

`TextPrimitive` should not be renamed prematurely while old callers still use the compatibility overload. Rename is a final mechanical cleanup after migration to the new render-state contract.

### 2.29 TextPrimitive should not necessarily survive as a public concept

Even after the rename, `TextRenderer` is expected to be an internal/backend class rather than part of the client-facing framework API.

The final decision is therefore:

```text
Do not expose TextPrimitive/TextRenderer as a client extension point.

Keep an internal renderer object only if it simplifies:
    renderer cache lifetime
    SDL_ttf text object ownership
    raster font caching
    logical → physical conversion

If those responsibilities can be expressed as a smaller internal implementation
without a persistent object, the class may disappear entirely at the final
cleanup stage.
```

This is intentionally different from the old `TextPrimitive` concept: it is no longer a framework layout primitive.

## 3. Current implementation status

### Architecture / semantics

```text
Imperative invalidation                DONE / settled
Measure on Node                       DONE / settled
Arrange on Node                       DONE / settled
Measure on PanelNode                  DONE / settled
Arrange on PanelNode                  DONE / settled
PanelNode as structural capability    DONE / settled
Auto/min/max semantics                DONE / validated
Border-box semantics                  DONE / validated
Overflow clipping model               DONE / validated
Root-based layout queue               DONE / validated
Re-invalidation semantics             DONE / validated
Client-owned source font boundary     DONE / provisional
Typography component direction        DONE / settled
TextLayoutResult direction             DONE / settled
TextPrimitive final role               IN PROGRESS
TextRenderer rename                    DEFERRED until migration complete
```

### Implementation / migration

```text
Node public Measure/Arrange hooks
    PRESENT

NodeTree root/coalesced invalidation
    PRESENT

TextLayout logical font sizing
    PRESENT

TextLayout logical line height
    PRESENT

Typography component
    PRESENT / migration in progress

TextLayoutResult
    PRESENT

TextRenderState
    PRESENT

New TextPrimitive render-state overload
    PRESENT

Typography → Arrange-specific text resolution
    PRESENT

TextPrimitive old layout-heavy draw path
    STILL PRESENT / cleanup target

Embedded text components fully migrated
    NOT DONE

Old TextNode compatibility cleanup
    NOT DONE

Framework-wide explicit invalidation audit
    NOT DONE

Typography policy/theme layer
    NOT PLANNED YET

Full framework build/runtime validation
    NOT DONE

Real chess client validation
    NOT DONE
```

## 4. Remaining implementation work

### Step 1 — Complete the text renderer split

Finish the new `TextRenderState + TextLayoutResult` path in the backend.

Then remove from the renderer layer the semantics that belong to `TextLayout`/component policy:

```text
wrapping decisions
alignment policy
logical geometry calculation
layout measurement
```

The renderer should retain only physical/backend responsibilities.

### Step 2 — Migrate all text-bearing components

Audit and migrate at least:

```text
TextNode / legacy text component
Typography
Button
MenuItem
TabItem
Dropdown
other real text-bearing controls
```

Each should either:

```text
own a TextLayout/text state directly
```

or use the shared internal text contract.

Do not make every control a child `Typography` node merely to draw a string.

### Step 3 — Decide final TextRenderer existence

After migration, inspect the remaining backend responsibilities.

Keep an internal `TextRenderer` only if its object lifetime/cache state is useful. Otherwise fold the implementation into a smaller text-rendering subsystem and remove the class completely.

The old name `TextPrimitive` must not survive the final public API.

### Step 4 — Remove legacy TextNode/compatibility paths

After all real callers migrate:

```text
remove old TextNode public role if no longer required
remove old TextPrimitive draw overload
remove old measure compatibility helpers if unused
```

Do not perform this before the actual callers are migrated.

### Step 5 — Finish imperative invalidation audit

Audit framework-known and custom state that can affect Measure/Arrange.

Required rule:

```text
state mutation
    ↓
explicit invalidateLayout()
```

Do not restore hidden setter-driven layout scheduling merely to reduce client call sites.

Interactive callbacks may explicitly call `invalidateLayout()` when interaction changes geometry.

### Step 6 — Final layout/text acceptance coverage

Maintain focused acceptance cases combining:

```text
custom Measure/Arrange
min/max
fixed size
Auto
stretch
padding
border
text wrapping
font size
line height
logical presentation scaling
invalidateLayout()
```

The text cases should specifically verify that a logical font size is converted to physical raster size before rasterization.

### Step 7 — Runtime/build validation

After migration:

```text
build framework
build chess client
run existing tests
run custom layout acceptance tests
verify resize
verify text wrapping
verify line height
verify clipping
verify overlays/modals
verify structural mutation
verify repeated invalidation
verify text at presentation scales
```

Historical Phase 2 numerical cases remain semantic acceptance references.

### Step 8 — Final cleanup

Only after runtime validation:

```text
remove obsolete helpers
remove obsolete documentation about old property/layout models
remove compatibility overloads
rename TextPrimitive → TextRenderer if retained
or remove the class completely if its responsibilities collapse naturally
update public documentation
```

## 5. Definition of done

The refactor is complete when all of the following are true:

```text
1. A leaf Node can define custom Measure/Arrange without PanelNode.

2. A PanelNode can define a custom child layout policy without framework
   knowledge of its custom properties.

3. Framework-known size/min/max/padding/border semantics remain centralized
   in framework constraint resolution.

4. Custom components do not need to duplicate framework constraint math.

5. Layout-affecting state changes use the explicit invalidateLayout contract.

6. Old deferLayoutMutation-based component layout scheduling is removed.

7. Structural mutation remains owned by PanelNode/NodeTree and does not need
   a second notification path.

8. Overflow clipping remains framework-owned and stack-scoped during Draw.

9. Re-invalidation during Measure/Arrange remains deferred rather than
   recursively re-entering layout.

10. Typography is the single public standalone text component family;
    separate Heading/Paragraph components are not required.

11. Text layout works in logical coordinates and rasterizes at physical size
    before rendering.

12. Text rendering backend is not a layout owner.

13. TextPrimitive is either removed or reduced to an internal renderer with a
    correct name such as TextRenderer.

14. The real chess client builds and runs correctly on the new model.

15. Historical numerical acceptance cases remain semantically satisfied.
```

## 6. Working mode from this checkpoint onward

The architecture-definition phase is considered largely complete.

New questions should be investigated only when they block implementation or reveal a contradiction with the settled historical semantics.

Default workflow:

```text
inspect current code
    ↓
compare against settled contract / historical reference
    ↓
make the smallest implementation change
    ↓
validate
    ↓
update this checkpoint if the architectural contract changes
```

Current priority order:

```text
1. finish TextPrimitive → renderer split
2. migrate all real text-bearing components
3. remove legacy TextNode / compatibility paths
4. complete explicit invalidation audit
5. final TextRenderer existence/name decision
6. build + runtime + chess-client validation
7. final cleanup
```
