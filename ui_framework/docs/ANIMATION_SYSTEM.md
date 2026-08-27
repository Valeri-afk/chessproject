# Animation System

The UI framework provides a small internal animation runtime for framework-owned visual transitions. It is intentionally not a general-purpose animation authoring system.

## Runtime model

Animations advance through the normal framework time phase:

```text
UIManager::advanceTime(dt)
        ↓
NodeTree::advanceTime(dt)
        ↓
AnimationSystem::advance(dt)
```

`render()` only presents the current state. It does not advance animations.

## Ownership

The `AnimationSystem` is owned by `NodeTree`. Components do not own `Animation` objects and client code does not construct animation runtime objects.

A component requests an animation for one of its presentation values. The runtime stores the transition internally and applies the resulting value through the component's setter/callback.

The target Node is not owned by the animation. Animation lifetime is tied to the live Node lifetime through the framework's lifetime mechanism, so destroying a Node cannot leave the runtime with a usable target pointer.

## Property model

The current primitive animates a `float` presentation property:

```text
current value → target value
```

A property is identified internally by its address/key. Only one active animation is kept for a given property on a Node. Starting another animation for the same property replaces the previous transition and starts from the value currently represented by the Node.

Different properties on the same Node may animate concurrently.

This gives the following behavior:

```text
Button
  presentationScale ── animation A

Some other presentation value ── animation B
```

There is no single universal animation value per component.

## Completion and cancellation

An animation with zero duration applies its target immediately.

Cancelling an animation removes the transition without changing the current property value. A later animation can therefore start from that current value.

When a transition reaches its target, the final target value is applied and the transition is removed.

## Easing

The current runtime supports:

```text
Linear
EaseIn
EaseOut
EaseInOut
```

Easing is selected by framework code when a component starts its default visual transition. It is not exposed as a universal animation configuration on `Node`.

## Component contract

A component should expose animation configuration only when that configuration is a meaningful part of the component's public behavior.

The component remains responsible for deciding:

- whether a default visual behavior should animate;
- which presentation value is animated;
- when the transition starts or stops;
- whether the client can disable that default animation.

The animation runtime is responsible for:

- advancing time;
- interpolation;
- easing;
- replacement of the same-property transition;
- cancellation;
- safe handling of Node destruction.

## Current consumers

### Button

`Button` uses a private presentation scale value for its press visual behavior. The default press animation can be disabled through the component API. The semantic button state and layout size are not themselves animated.

Text is deliberately not part of this press-scale implementation. Text rasterization/layout is a separate concern and must not be approximated by changing font size after rasterization.

### Modal backdrop

`ModalSystem` uses the animation runtime for backdrop opacity. The backdrop is an internal framework Node and its opacity is a presentation value; modal session state remains separate from the animation.

## What is intentionally not part of the current system

The framework does not currently provide:

```text
public Animation objects
client-owned Animation instances
UIManager::animate(...) facade methods
animation callbacks such as onStart/onEnd
universal animated-property registration
universal animation configuration on Node
animation of text font size/rasterized glyphs
```

These should only be introduced when a concrete requirement demonstrates that the current property-based runtime is insufficient.
