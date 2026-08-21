# UI Framework API Refactor Checklist

Status: API design is frozen. This document is the implementation checklist for `ui_framework` on `chessproject/main`.

## Rules

- Work only on `chessproject/main`.
- Preserve behavior and algorithms; this pass is API naming, visibility, file organization, and include dependency cleanup.
- Do not rewrite `docs/ARCHITECTURE.md` unless a concrete contradiction is found later; then make only the minimal correction.
- `UIManager` is the public framework facade.
- `InputSystem`, `LayoutSystem`, `ModalSystem`, and `ScrollSystem` are private services.
- `Primitives` are public API.
- SDL/SDL_ttf types remain public where already part of the framework/client contract; do not wrap `SDL_Renderer*`, `TTF_Font*`, etc.
- Scroll remains framework behavior; no public `Scroll`/`ScrollArea` component.
- Modal remains framework service; no public `Modal` component.

## Final classes

### Public

- `UIManager`
- `Node`
- `PanelNode`
- `StackPanelNode`
- `TextNode`
- `Button`
- `Checkbox`
- `Dropdown`
- `Menu`
- `MenuItem`
- `RadioButton`
- `Slider`
- `TabControl`
- `TabItem`
- `ToggleButton`
- public event/value types

### Private

- `NodeTree`
- `InputSystem` (from `InputManager`)
- `LayoutSystem` (from `LayoutManager`)
- `ModalSystem` (from `ModalManager`)
- `ScrollSystem` (from `ScrollManager`)
- `EventDispatcher`
- `EventHandlerStorage`
- `RenderingState`
- `LayoutConstraints`
- `LinearLayout`
- `ScrollState`
- modal backdrop implementation
- mutation/traversal machinery

`Manager` is reserved for the public `UIManager`; internal services use `System`.

## Final file layout

### Public

```text
ui_framework/include/ui_framework/
    ui_framework.hpp
    types.hpp
    events.hpp
    primitives.hpp
    node.hpp
    panel_node.hpp
    stack_panel_node.hpp
    text_node.hpp
    ui_manager.hpp
    components/
        button.hpp
        checkbox.hpp
        dropdown.hpp
        menu.hpp
        menu_item.hpp
        radio_button.hpp
        slider.hpp
        tab_control.hpp
        tab_item.hpp
        toggle_button.hpp
```

### Private

```text
ui_framework/src/detail/
    node_tree.hpp / node_tree.cpp
    input_system.hpp / input_system.cpp
    layout_system.hpp / layout_system.cpp
    modal_system.hpp / modal_system.cpp
    modal_system_backdrop.cpp
    scroll_system.hpp / scroll_system.cpp
    event_dispatcher.hpp
    event_handler_storage.hpp
    event_handler_storage.inl
    rendering_state.hpp
    layout_constraints.hpp
    linear_layout.hpp
    ...
```

Remove the public `core/` include hierarchy after all includes are migrated.

## Public `Node` API

Final names:

```cpp
Id getId() const noexcept;
Node* getParent() const noexcept;

void setVisible(bool);
bool isVisible() const noexcept;
void setEnabled(bool) noexcept;
bool isEnabled() const noexcept;
void setFocusable(bool) noexcept;
bool isFocusable() const noexcept;
void setCapturable(bool) noexcept;
bool isCapturable() const noexcept;

void setPosition(const LayoutPosition&);
LayoutPosition getPosition() const noexcept;
void setPositionMode(PositionMode);
PositionMode getPositionMode() const noexcept;
void setSize(const LayoutSizeValue&);
LayoutSizeValue getSize() const noexcept;
LayoutSize getDesiredSize() const noexcept;
LayoutPosition getActualPosition() const noexcept;
LayoutSize getActualSize() const noexcept;

void setMinSize(const LayoutSize&);
void setMaxSize(const LayoutSize&);
void setMinWidth(float);
void setMinHeight(float);
void setMaxWidth(float);
void setMaxHeight(float);
LayoutSize getMinSize() const noexcept;
LayoutSize getMaxSize() const noexcept;
float getMinWidth() const noexcept;
float getMinHeight() const noexcept;
float getMaxWidth() const noexcept;
float getMaxHeight() const noexcept;

void setPadding(const Padding&);
Padding getPadding() const noexcept;
void setLeftPadding(float);
void setRightPadding(float);
void setTopPadding(float);
void setBottomPadding(float);
void setBorder(const Border&);
Border getBorder() const noexcept;
void setLeftBorder(float);
void setRightBorder(float);
void setTopBorder(float);
void setBottomBorder(float);
void setOverflow(Overflow);
Overflow getOverflow() const noexcept;
```

Event API:

```cpp
using EventHandlerId = std::uint64_t;

template<class Event>
EventHandlerId on(Callback);

template<class Event>
void removeEventHandler(EventHandlerId);

template<class Event>
void clearEventHandlers();
```

Legacy `addHandler/removeHandler/clearHandlers` are migration-only and must disappear from the final public API.

## `PanelNode`

Final public names:

```cpp
Node* addChild(std::unique_ptr<Node> child, std::size_t index);
void removeChild(Node& child);
std::size_t getChildCount() const noexcept;
bool hasChildren() const noexcept;
Node* getChild(std::size_t index) noexcept;
const Node* getChild(std::size_t index) const noexcept;
void forEachChild(const ChildCallback&);
void forEachChildReverse(const ChildCallback&);
```

Rename:
- `add` -> `addChild`
- `remove` -> `removeChild`
- `childCount` -> `getChildCount`
- `getChildAt` -> `getChild`
- `rForEachChild` -> `forEachChildReverse`

`getVisibleChild`, `visibleChildCount`, `visibleChildIndexAt`, `canAttach`, and `isAncestorOf` are not public APIs.

## `StackPanelNode`

Keep:

```cpp
setOrientation / getOrientation
setGap / getGap
setMainAlignment / getMainAlignment
setCrossAlignment / getCrossAlignment
```

## `TextNode`

Keep:

```cpp
getText / setText
getFont / setFont
getHorizontalAlignment / setHorizontalAlignment
getVerticalAlignment / setVerticalAlignment
getColor / setColor
```

`TTF_Font*` stays public; `TextPrimitive` remains implementation detail.

## `UIManager`

Final public API:

```cpp
void runFrame(float dt, SDL_Renderer* renderer);
void processEvent(const SDL_Event& event, SDL_Renderer* renderer);

Node* addRoot(std::unique_ptr<Node> node);
Node* addOverlay(std::unique_ptr<Node> node);
void removeRoot(Node* node);
void removeOverlay(Node* node);

bool enableScrolling(Node& node);
bool disableScrolling(Node& node);
bool isScrollingEnabled(const Node& node) const noexcept;
bool setScrollOffset(Node& node, const ScrollOffset& offset);
ScrollOffset getScrollOffset(const Node& node) const noexcept;
ScrollOffset getMaximumScrollOffset(const Node& node) const noexcept;

bool showModal(Node& node);
bool showModal(Node& node, BackdropClickBehavior behavior);
bool closeModal();
bool isModal(const Node& node) const noexcept;
Node* getActiveModal() const noexcept;

void setBackdropColor(const Color&) noexcept;
Color getBackdropColor() const noexcept;
void setBackdropFadeDuration(float) noexcept;
float getBackdropFadeDuration() const noexcept;
```

Not public: `attachRoot`, `attachOverlay`, `registerScrollNode`, `unregisterScrollNode`, `isScrollNodeRegistered`, `topModalNode`, or client-facing `Node::Id` scroll parameters.

## Internal systems

### `InputSystem`

Rename class/files:

```text
InputManager -> InputSystem
inputmanager.* -> input_system.*
```

Final internal vocabulary:

```cpp
processEvent
synchronizeState
resetState
updateHover
setFocus
clearFocus
capturePointer
releasePointerCapture
cancelPointerInteraction
setModalRoot
getFocusedNode
getFocusedNodeId
getCapturedNode
getPressedNode
isDragging
```

### `LayoutSystem`

Rename class/files:

```text
LayoutManager -> LayoutSystem
layoutmanager.* -> layout_system.*
```

Final internal vocabulary:

```cpp
setViewportSize
getViewportSize
updateViewportFromRenderer
requestFullLayout
processLayoutQueue
```

Measure/arrange helpers remain private.

### `ModalSystem`

Rename class/files:

```text
ModalManager -> ModalSystem
modalmanager.* -> modal_system.*
```

Keep service operations readable:

```cpp
showModal
closeModal
handleKeyDown
handlePointerDown
update
setViewportSize
setBackdropColor
getBackdropColor
setBackdropFadeDuration
getBackdropFadeDuration
clear
isModal
sync
```

Modal stack/backdrop lookup stays private.

### `ScrollSystem`

Rename class/files:

```text
ScrollManager -> ScrollSystem
scrollmanager.* -> scroll_system.*
```

Final internal vocabulary:

```cpp
enableScrolling
disableScrolling
isScrollingEnabled
setScrollOffset
scrollBy
getScrollState
getScrollOffset
getMaximumScrollOffset
getAccumulatedScrollOffset
findScrollableAncestor
processWheel
synchronizeState
clearState
```

`ScrollOffset` is public. `ScrollState` is private.

## Components

### Button

Keep all current state/style names (`setText/getText`, `setFont/getFont`, colors, variant, radius, press scale/animation, `isPressed`, `isHovered`, `activate`).

Rename callback:
- `setOnActivate` -> `setOnActivated`
- protected `onActivate` -> `onActivated`

### Checkbox

Keep `setChecked/isChecked`, `setBoxSize/getBoxSize`, `toggle`, `activate`.

Rename `setOnToggle` -> `setOnToggled`.

### RadioButton

Keep `setSelected/isSelected`, `setRadius/getRadius`, `select`, `activate`.

Rename `setOnSelect` -> `setOnSelected`.

### ToggleButton

Keep `setSelected/isSelected`, `toggle`, `activate`.

Rename `setOnToggle` -> `setOnToggled` if present.

### Slider

Keep:

```cpp
setMinimum/getMinimum
setMaximum/getMaximum
setValue/getValue
setStep/getStep
setOnValueChanged
```

### Menu

Keep item/state methods; rename callback:
- `setOnItemActivate` -> `setOnItemActivated`

### MenuItem

Keep text/font/colors/highlight/selection/`activate`; rename:
- `setOnActivate` -> `setOnActivated`

### Dropdown

Keep current API: `addItem/removeItem`, `open/close/toggle/isOpen`, selection accessors, `clearSelection`, `getTrigger/getMenu`, placeholder API, `setOnSelectionChanged`.

### TabControl

Keep current API: `addTab/removeTab`, both `selectTab` overloads, `clearSelection`, selected-tab/index getters, `setOnSelectionChanged`.

### TabItem

Keep text/font/colors and `setActive/isActive`, `activate`; rename `setOnActivate` -> `setOnActivated`.

## Events

Rename public header:

```text
event_types.hpp -> events.hpp
```

Keep event type names such as `MouseMoveEvent`, `MouseDownEvent`, `MouseUpEvent`, `MouseClickEvent`, `MouseEnterEvent`, `MouseLeaveEvent`, `MouseWheelEvent`, `KeyDownEvent`, and `KeyUpEvent`.

`EventDispatcher` and `EventHandlerStorage` remain private.

## Public types

Keep `types.hpp` as one cohesive public header.

Public:

- `Overflow`
- `MainAxisAlignment`
- `CrossAxisAlignment`
- `TextAlignment`
- `PositionMode`
- `LayoutPosition`
- `LayoutValueType`
- `LayoutValue`
- `LayoutSizeValue`
- `LayoutSize`
- `Padding`
- `Border`
- `Color`
- `Colors`
- `StyleProps`
- `ScrollOffset`

Move `LayoutConstraints` to private layout implementation.

## Primitives

- Public API.
- Public header: `ui_framework/primitives.hpp`.
- Preserve primitive behavior.
- Keep existing primitive function semantics during this refactor; do not combine a cosmetic primitive rename with the structural API migration.
- `SDL_Renderer*` remains public where required.

## Include dependency cleanup

- Public/client code must include only `ui_framework/...` public headers.
- No public include may reference `ui_framework/core/...` after migration.
- No public header may include `NodeTree`, any `*System`, `EventHandlerStorage`, or `RenderingState`.
- Internal source may include public headers and `src/detail/...`.
- `node.hpp` must no longer depend on `EventHandlerStorage`.

## Implementation order

1. Verify existing `Node::getId/getParent` migration and public event API.
2. Apply `PanelNode` API renames.
3. Apply component callback renames.
4. Apply `UIManager` public API rename.
5. Rename `InputManager/LayoutManager/ModalManager/ScrollManager` to systems and rename their files.
6. Move private headers into `src/detail`.
7. Move public headers out of `include/ui_framework/core` and rename them to snake_case public paths.
8. Rename `event_types.hpp` -> `events.hpp` and `core/primitives.hpp` -> `primitives.hpp`.
9. Create `ui_framework/ui_framework.hpp` umbrella header.
10. Update all source includes and CMake/source lists.
11. Remove legacy wrappers/aliases after all call sites are migrated.
12. Update documentation to match the frozen API.
13. Do not rewrite `ARCHITECTURE.md`; only make minimal corrections for concrete contradictions.
14. Only after source/doc refactor is complete, perform the separate compile/readiness pass.

## Special large files

These files are expected to require manual editing when the available GitHub write path cannot safely replace them:

- `ui_framework/src/core/nodetree.cpp`
- `ui_framework/src/core/inputmanager.cpp`

For those files, make only exact mechanical renames/call-site updates; do not rewrite behavior.

## Completion criteria

The API refactor is complete when:

- every name in this checklist is implemented;
- no legacy public names remain;
- no private system/manager is exposed through public headers;
- no public/client include uses `ui_framework/core/...`;
- public headers contain no private storage/rendering/layout dependencies;
- primitives remain public;
- SDL/TTF types remain available where required;
- Scroll and Modal remain framework services, not public components;
- no algorithm/behavior change was introduced by the API migration;
- documentation matches the final API.
