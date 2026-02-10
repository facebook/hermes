---
name: gc-safe-coding
description: >
  Rules for writing and reviewing GC-safe C++ code in the Hermes VM runtime.
  Use when writing, modifying, or reviewing C++ runtime VM code that uses
  internal Hermes VM APIs (as opposed to code using JSI). This includes working
  with GC-managed types (HermesValue, Handle, PinnedValue, JSObject,
  StringPrimitive, etc.), Locals, GCScope, PseudoHandle, CallResult, or any
  function with _RJS suffix. Typically in lib/VM/, include/hermes/VM/, or
  API/hermes/.
---

For the full explanation and rationale, see [doc/GCSafeCoding.md](doc/GCSafeCoding.md).

## GC safepoints

A GC safepoint is either a GC heap allocation or a function call that might
transitively reach one (regular C heap allocations like `malloc` are not
safepoints). Any function that takes `Runtime &` or `PointerBase &`
may trigger GC, unless documented otherwise or named with `_noalloc`/`_nogc`.
Functions with `_RJS` suffix invoke JavaScript recursively and always trigger
GC.

**All raw pointers to GC objects must be rooted before any GC safepoint.**

## Rooting local values: use Locals + PinnedValue (required for new code)

All new code must use `Locals` + `PinnedValue<T>`. Do not introduce new
`GCScope` instances or `makeHandle()` calls.

```cpp
struct : public Locals {
  PinnedValue<JSObject> obj;
  PinnedValue<StringPrimitive> str;
  PinnedValue<> genericValue;
} lv;
LocalsRAII lraii(runtime, &lv);
```

### Assignment patterns

- **From PseudoHandle:** `lv.obj = std::move(*callResult);`
- **From HermesValue with known type:** `lv.obj.castAndSetHermesValue<JSObject>(hv);`
- **From raw pointer:** `lv.obj = somePtr;`
- **Clear:** `lv.obj = nullptr;`
- **In template context:** `lv.obj.template castAndSetHermesValue<T>(hv);`

### Passing to functions

`PinnedValue<T>` implicitly converts to `Handle<T>`. Pass directly to functions
that accept `Handle<T>`.

## Error handling with CallResult

Always check for exceptions before using the value:

```cpp
auto result = someOperation_RJS(runtime, args);
if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
  return ExecutionStatus::EXCEPTION;
lv.obj = std::move(*result);
```

## When Handle usage is fine (do not flag)

Not every use of `Handle<>` needs to be converted to `PinnedValue`. The rule
"use Locals, not GCScope" applies to **creating new rooted values** — allocating
new `PinnedHermesValue` slots via `makeHandle()` or `Handle<>` constructors.

The following are **not** allocating new handles and do not need conversion:

- **`vmcast<>(handle)`** — casts an existing handle to a different type. It does
  not take `Runtime &` and does not allocate a GCScope slot. The result points
  to the same `PinnedHermesValue` as the input.
- **`args.getArgHandle(n)`** — returns a handle pointing into the register
  stack, which is already a root. No new allocation.
- **Passing or receiving a `Handle<>` parameter** — the handle was allocated by
  the caller; the callee is just using it.

Only flag handle usage when a **new** `PinnedHermesValue` slot is being
allocated (via `makeHandle()`, `makeMutableHandle()`, or `Handle<>`/
`MutableHandle<>` constructors that take `Runtime &`).

## Checklist for writing / reviewing GC-safe code

1. **No raw pointers across GC safepoints.** Every pointer to a GC object must
   be stored in a `PinnedValue` before any call that takes `Runtime &` or is `_RJS`.
2. **Use Locals, not GCScope.** New code must not introduce `GCScope` or
   `makeHandle()`. Declare a `struct : public Locals` with `PinnedValue` fields
   and a `LocalsRAII`.
3. **Check every CallResult.** Never dereference a `CallResult` without first
   checking `== ExecutionStatus::EXCEPTION`.
4. **Never return Handle from local roots.** Do not return `Handle<T>` pointing
   into a `PinnedValue` or `GCScope` that is about to be destroyed. Return
   `CallResult<PseudoHandle<T>>` or `CallResult<HermesValue>` instead.
5. **Null prototype checks.** When traversing prototype chains, check for null
   before calling `castAndSetHermesValue`.
6. **Loops are safe with Locals.** `PinnedValue` fields are reused each
   iteration — no unbounded growth. If a `GCScope` is still needed for legacy
   APIs that return `Handle`, use `GCScopeMarkerRAII` or `flushToMarker`.
7. **Handles allocate in the topmost GCScope.** `makeHandle()`,
   `makeMutableHandle()`, `Handle<>` and `MutableHandle<>` constructors, and
   calls to functions that take `Runtime &`/`PointerBase &` and return
   `Handle<>`, all allocate a slot in the topmost `GCScope`. Functions that
   create or receive handles without returning them need their own `GCScope` or
   `GCScopeMarkerRAII` (preferred for one or two handles). Functions like
   `vmcast<>` that do not take `Runtime &` just cast existing handles without
   allocating.
