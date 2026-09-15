# Optimization Notes for richards-typed.js

Analysis of the LIR (`shermes --typed -dump-lir`) and assembly
(`shermes --typed -S`) to identify performance bottlenecks.

## Constants: `var` vs `const`

Changed module-level constants from `var` to `const` for correctness and style.
Testing shows `var`, `let`, and `const` produce identical IR, LIR, and
performance — the keyword choice does not affect optimization.

### Which constants get inlined

Regardless of `var`/`let`/`const`, constants initialized with **plain numeric
literals** are inlined at use sites as `LIRLoadConstInst`. They do not appear
in the scope and have zero overhead:

```js
const STATE_HELD: number = 4;       // inlined
const ID_DEVICE_A: number = 4;      // inlined
const COUNT: number = 1000;         // inlined
```

Constants whose initializers **reference other declarations** are NOT inlined.
They are stored in scope `%VS1` and read via `LoadFrameInst` at every use:

```js
const STATE_SUSPENDED_RUNNABLE: number = STATE_SUSPENDED | STATE_RUNNABLE;  // NOT inlined
const STATE_NOT_HELD: number = ~STATE_HELD;                                 // NOT inlined
```

The values ARE folded at initialization time (e.g., `STATE_SUSPENDED_RUNNABLE`
is stored as literal `3`), but they are still read from the scope rather than
substituted as immediates at use sites.

`DATA_SIZE = 4` is a plain literal but is also not inlined — possibly because
it is declared after the two expression-initialized constants and ends up in
the same scope region.

### Scope `%VS1` contents

```
scope %VS1 [
  STATE_SUSPENDED_RUNNABLE: number,
  STATE_NOT_HELD: number,
  DATA_SIZE: number,
  Task: undefined|object,
  TaskControlBlock: undefined|object,
  IdleTask: undefined|object,
  DeviceTask: undefined|object,
  WorkerTask: undefined|object,
  HandlerTask: undefined|object,
  ...prototype objects...
]
```

The class constructors appear as `undefined|object` because of TDZ — they start
as `undefined` and are set when the class definition executes.

## Hot path observations

### `schedule()` — main loop

Uses a local variable `tcb` for the loop, which narrows `null|object` to `object`
via `CheckedTypeCastInst`. The loop body inlines `isHeldOrSuspended()` and
partially inlines `TCB.run()`.

### Virtual dispatch: `this.task.run(packet)`

In `TCB.run()` (LIR function `"run 1#"`), the call to `this.task.run(packet)` goes
through:
1. `PrLoadInst` to load `this.task` — typed as `uninit|object`
2. `ThrowIfInst` to check for uninitialized field
3. `TypedLoadParentInst` + `PrLoadInst` to load the `run` method from the prototype
4. `CallInst [njsf] (:any)` — return type is `any`
5. `CheckedTypeCastInst` to narrow `any` back to `null|object`

The `uninit|object` type on `task` and the `any` return from `CallInst` are
sources of unnecessary checks.

### `ThrowIfInst` for `uninit|object` fields

The `scheduler` field in task subclasses is typed `uninit|object`. Every access
generates a `ThrowIfInst` to guard against use-before-initialization. This
appears repeatedly in all task `run` methods (e.g., IdleTask.run has 3 such
checks for `this.scheduler`).

### Redundant `CheckedTypeCastInst`

Nullable fields (`null|object`) that are checked for null in one branch still
require `CheckedTypeCastInst` when accessed later, because the compiler does
not propagate narrowing across `this.field` accesses. This is especially
visible in `HandlerTask.run` ("run 5#") which has ~20 such casts.

### `LoadFrameInst` for non-inlined constants

`STATE_SUSPENDED_RUNNABLE`, `STATE_NOT_HELD`, and `DATA_SIZE` are loaded from
the closure scope on every use, requiring `GetParentScopeInst` or
`GetClosureScopeInst` followed by `LoadFrameInst`. These appear in nearly
every hot function.

## Assembly observations

### Numbers are NaN-boxed doubles

All `number`-typed values are stored as NaN-boxed IEEE 754 doubles. Bitwise
operations (like `state & STATE_HELD`) require conversion round-trips:

```asm
fjcvtzs  w8, d0       ; double → int32
fjcvtzs  w9, d1       ; double → int32
and      w8, w9, w8   ; actual AND
scvtf    d0, w8       ; int32 → double
```

Plus `fcmp d0, d0` NaN guards before each conversion. Fields like `state`, `id`,
`priority` are integers by nature but pay this overhead on every access.

Static Hermes does not yet support native integer types.

### Property access: direct slots vs indirect

Objects have a fixed number of **direct property slots** (5 slots, offsets 0–4).
Properties at these offsets compile to inline memory loads:

```asm
ldr  x10, [x10, #40]   ; direct load of offset 0 (byte offset 40 in object)
```

Properties at offset 5+ overflow into indirect storage and require a **runtime
function call**:

```asm
mov  w2, #0             ; slot index within indirect area
bl   __sh_prload_indirect
```

This is significant for `TaskControlBlock` which has 6 fields:

| Offset | Field      | Access type |
|--------|-----------|-------------|
| 0      | link      | direct      |
| 1      | id        | direct      |
| 2      | priority  | direct      |
| 3      | queue     | direct      |
| 4      | task      | direct      |
| 5      | state     | **indirect** |

`state` is the most frequently accessed field (every `isHeldOrSuspended`,
`markAsHeld`, `markAsRunnable`, etc.) but it's the only one that overflows
into indirect storage.

### Applied: Reorder TCB fields to put `state` in a direct slot

Swapped `state` (offset 4) and `task` (offset 5) in `TaskControlBlock`.
`state` is accessed on every loop iteration; `task` is accessed once per
dispatch.

Before (offset 5 → indirect, function call):
```asm
mov  w2, #0
bl   __sh_prload_indirect
```

After (offset 4 → direct, inline load):
```asm
ldr  x9, [x9, #72]     ; direct memory load
```

**Result: 72ms → 66ms (8% improvement).**
