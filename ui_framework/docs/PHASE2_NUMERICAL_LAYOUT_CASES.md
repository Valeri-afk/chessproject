# Phase 2 Numerical Layout Acceptance Cases

> **Status:** Phase 2 acceptance reference
> **Date:** 2026-08-19

This document is the numerical acceptance reference for the minimal Phase 2 layout system. It records the semantics already established by the implementation and is no longer a pre-specification research document.

## 1. Canonical pipeline

Every case is interpreted through this ownership model:

```text
parent border-box proposal
        ↓
Node measurement constraints
(size + max; min does not narrow intrinsic proposal)
        ↓
content-box proposal
        ↓
content measurement
        ↓
desired content size
        ↓
box composition (padding + border)
        ↓
parent aggregation / child allocation
        ↓
final constraint resolution (size + min + max)
        ↓
actual geometry
```

Canonical constraint semantics:

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

For width-sensitive content such as wrapped text, `maxWidth` must affect the measurement proposal before measurement. A minimum does not automatically force intrinsic content to be measured at the minimum size.

## 2. Case A — Fixed-width Text

```text
Root width = 400
└── Vertical Panel
      └── Text
```

Text has no explicit width. Under width 400 it measures to `400 × 80`.

Expected panel desired size:

```text
400 × 80
```

Expected Text rect:

```text
(0, 0, 400, 80)
```

## 3. Case B — Text plus Button with gap

```text
Vertical Panel, width = 400, gap = 10
├── Text   = 400 × 80
└── Button = 120 × 40
```

Expected desired height:

```text
80 + 10 + 40 = 130
```

Expected placement:

```text
Text   = (0, 0, 400, 80)
Button = (0, 90, 120, 40)
```

## 4. Case C — Panel padding

```text
Panel width = 400
padding left/right = 20
top/bottom = 10
Text
```

Content width:

```text
400 - 20 - 20 = 360
```

If Text measures `360 × 80`, expected panel desired size is:

```text
400 × 100
```

## 5. Case D — Fixed Text width smaller than parent

Parent content width = `400`.

Text fixed width = `200`, measured at width 200.

If result is `200 × 140`, center alignment gives:

```text
x = (400 - 200) / 2 = 100
```

Expected rect:

```text
(100, 0, 200, 140)
```

## 6. Case E — Min/max on Text

Parent proposal width = `400`.

Text intrinsic result under that proposal = `400 × 80`.

Constraints:

```text
minHeight = 120
maxHeight = 200
```

Measurement remains intrinsically `400 × 80`.

Final resolved height is `120`.

```text
measurement desired = 400 × 80
final geometry      = 400 × 120
```

The minimum is therefore not fed back into the intrinsic measurement proposal.

## 7. Case F — Content plus padding

Text desired under proposal 200:

```text
160 × 24
```

Container padding:

```text
left/right = 10
top/bottom = 8
```

Expected outer desired size:

```text
180 × 40
```

## 8. Case G — Parent width changes text wrapping

Initial:

```text
Panel width = 400
Text = 400 × 80
```

After resize:

```text
Panel width = 250
Text proposal = 250
Text result = 250 × 128
```

The changed proposal alone causes a new measurement; no Text property mutation is required.

## 9. Case H — Vertical main-axis center

Container height = `300`.

Children:

```text
A = 50
B = 70
gap = 20
```

Occupied height:

```text
50 + 20 + 70 = 140
```

Free space:

```text
300 - 140 = 160
```

Center leading offset:

```text
80
```

Expected positions:

```text
A = y 80
B = y 150
```

## 10. Case I — Cross-axis center

Horizontal container height = `100`.

Child = `60 × 40`.

Expected cross-axis offset:

```text
y = (100 - 40) / 2 = 30
```

## 11. Case J — Stretch

Horizontal container height = `100`.

Child desired height = `40`.

Stretch allocates cross-axis height `100`, then final min/max constraints are resolved.

If `maxHeight = 70`, final height is `70`.

If `minHeight = 120`, final height is `120`, even though that exceeds the available cross-axis size. Phase 2 does not implement flex-shrink.

## 12. Case K — Absolute child

Container = `400 × 300`.

Absolute child:

```text
position = (50, 40)
size = 100 × 80
```

Expected child rect relative to the parent's content origin:

```text
(50, 40, 100, 80)
```

The child contributes nothing to normal-flow main-axis aggregation.

## 13. Case L — Nested panels

Inner horizontal panel:

```text
A = 100 × 40
B = 120 × 40
gap = 10
```

Expected inner desired size:

```text
230 × 40
```

Outer vertical panel:

```text
Text = 400 × 80
Inner = 230 × 40
gap = 10
```

Expected outer desired height:

```text
80 + 10 + 40 = 130
```

## 14. Case M — Visibility

Children:

```text
A = 50
B = hidden
C = 70
```

Only visible flow children participate in measurement/allocation.

The hidden child contributes neither size nor gap.

## 15. Case N — Fixed parent height with overflowing text

Parent height = `100` fixed.

Text intrinsic result = `300 × 140`.

The parent retains its constrained final height of `100`.

Phase 2 does not introduce automatic text shrinking. Overflow remains governed by the existing Node overflow model.

## 16. Case O — MaxWidth changes text measurement

Parent proposal width = `500`.

Text has:

```text
maxWidth = 300
```

The effective measurement proposal is width `300`, not `500` followed by a post-measure clamp.

This is mandatory for wrapping correctness because the measured height can depend on width.

## 17. Acceptance rule

The central acceptance invariant is:

```text
measurement proposal != final size
```

The framework must never implement width-sensitive measurement as:

```text
measure at unconstrained/oversized width
→ clamp width afterward
```

when `maxWidth` should have narrowed the proposal.

Likewise, `minWidth`/`minHeight` must not silently become intrinsic measurement proposals merely because they affect final geometry.

## 18. Deliberately deferred semantics

These cases are outside Phase 2:

```text
flex-grow
flex-shrink
flex-basis
flex-wrap
order
CSS-style left/right/top/bottom absolute constraints
margin
Grid track sizing
multi-pass intrinsic track resolution
content-dependent stretch remeasurement
```

They must not be introduced into the minimal Phase 2 implementation.

## 19. Current verdict

The implementation is considered consistent with these acceptance semantics for static source-level reasoning.

Runtime/build verification remains intentionally deferred until the project's final validation stage.
