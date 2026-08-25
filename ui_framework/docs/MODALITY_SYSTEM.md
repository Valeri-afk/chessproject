# Modality System

## Role

Modality is framework infrastructure, not a public `Modal` component.

The current service is `ModalManager`/the corresponding internal modality system behind `UIManager`.

## Responsibilities

The modality subsystem owns:

```text
modal registration / stack
active modal
modal-root filtering
backdrop state
backdrop rendering
Escape handling
pointer/backdrop handling
viewport synchronization
modal cleanup
```

The client-facing surface is semantic:

```cpp
uiManager.showModal(node);
uiManager.closeModal();
uiManager.isModal(node);
uiManager.getActiveModal();
```

Backdrop configuration is also exposed through the framework facade where required.

## Modal stack

Modality is stack-oriented. The active/top modal defines the modal input boundary.

Underlying application content remains in the tree but must not receive input that is restricted by the active modal root.

## Input filtering

Conceptually:

```text
SDL input
   ↓
UIManager / InputSystem
   ↓
active modal root exists?
   ↓ yes
restrict target traversal to modal subtree / allowed backdrop behavior
```

Escape and backdrop clicks are handled according to the configured modal/backdrop policy.

## Rendering

The modal backdrop is framework rendering infrastructure. It is not a child component that the client must construct to obtain modality.

Modal visual validation has already confirmed the basic backdrop/modal presentation in the chess client. Input/modal sequencing remains a separate validation concern.

## Lifecycle

Showing and closing a modal must remain synchronized with NodeTree live-node membership. A stale/detached modal node must not remain an active framework modal.

## Viewport

Modal/backdrop state follows the framework viewport. The service is synchronized with the current renderer/logical presentation dimensions rather than requiring an independent client source of truth.

## Public component decision

Do not introduce a public `Modal` component merely to represent the service. A component should only be added if repeated real use demonstrates a reusable semantic abstraction that cannot be expressed through the existing modality service and ordinary Nodes.

## Deferred validation

Later targeted validation should cover:

```text
MouseDown / MouseUp / MouseClick sequencing
modal input restriction
overlay hit testing
backdrop click behavior
Escape behavior
capture/focus interaction with modality
nested/modal stack behavior
```

Visual modality validation is not equivalent to input/modal correctness validation.
