# Phase 2 Constraint Semantics

> **Status:** current architectural contract
> **Date:** 2026-08-19

This document defines the Phase 2 distinction between measurement proposals and final size constraints. It is a current reference for the framework-owned layout system.

## 1. Canonical pipeline

```text
parent proposal
    ↓
measurement proposal resolution
    ↓
content measurement at effective bounds
    ↓
desired content size
    ↓
box composition (padding + border)
    ↓
parent aggregation / allocation
    ↓
final size resolution
    ↓
actual geometry
```

Measurement proposal and final size resolution are separate operations.

## 2. Measurement proposal

The measurement proposal answers:

> "Under what available bounds should content determine its desired size?"

It is driven by the parent proposal, explicit size, and constraints that bound available content space.

For width-sensitive content such as wrapped text, `maxWidth` must affect the proposal before measurement.

## 3. Final size constraints

Final geometry is resolved using:

```text
explicit size
min size
max size
parent allocation
```

Canonical semantics:

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

A minimum does not automatically become an intrinsic measurement proposal. A maximum may narrow the proposal because content cannot meaningfully require more space than the node is permitted to occupy.

## 4. Fixed explicit size

If a node has a fixed width or height, content is measured under the corresponding effective content-box size after padding/border conversion.

The fixed size therefore constrains both measurement and final geometry.

## 5. Maximum size

If:

```text
parent available width = 500
maxWidth = 300
```

the effective measurement width is `300` before width-sensitive content measurement.

This prevents wrapped content from being measured at a width that the node is not allowed to occupy.

## 6. Minimum size

If:

```text
parent available width = 500
minWidth = 300
```

intrinsic content may still be measured under `500` when no fixed/max width restricts the proposal.

The final node width must nevertheless be at least `300`.

A minimum therefore does not by itself force a second intrinsic measurement at the minimum size.

## 7. Combined min/max

If:

```text
minWidth = 300
maxWidth = 400
parent available = 500
```

the effective measurement width is `400`.

Final width is resolved within:

```text
300 … 400
```

according to the intrinsic result and parent allocation.

## 8. Unbounded proposal

An unbounded main-axis proposal asks the child how large it naturally wants to be in that axis.

For a vertical Linear container, child height may therefore be unbounded during measurement while width remains bounded by the parent's content width.

## 9. Padding and border

Content measurement uses content-box terms.

Conceptually:

```text
outer proposal
    ↓
subtract padding + border
    ↓
content measurement proposal
```

The desired content size is then composed back into the node's outer box:

```text
content desired size
    ↓
add padding + border
    ↓
node desired outer size
```

## 10. Auto size and stretch

`Auto` does not mean "fill parent". Parent layout determines how an auto-sized child participates.

Normal Linear flow uses intrinsic/desired size for auto children. Cross-axis stretch may allocate a larger final size subject to the child's constraints.

A fixed cross-axis size is not overridden by stretch.

## 11. Scope boundary

This contract intentionally does not define:

```text
flex-grow
flex-shrink
flex-basis
flex-wrap
order
margin
Grid track sizing
multi-pass intrinsic track resolution
```

Those belong to later requirements and must not be inferred as Phase 2 behavior.
