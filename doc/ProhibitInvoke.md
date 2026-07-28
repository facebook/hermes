---
id: prohibit-invoke
title: Valid-invocation enforcement (prohibitInvoke and the constructor rule)
---

### Introduction

Hermes enforces two kinds of "you cannot invoke this function that way" rules:

  1. **`prohibitInvoke`** — a per-JS-function flag that says the function is
     constructor-only or call-only (e.g. an ES6 class constructor cannot be
     called without `new`; an arrow function, method, generator, or async
     function cannot be called with `new`).
  2. **The "a `NativeFunction` cannot be used as a constructor" rule** — a
     `CellKind`-based rule that rejects `new`ing a plain C++ builtin that is not
     a `NativeConstructor`.

These two mechanisms are **orthogonal** and fire at different times: the
`prohibitInvoke` check runs at the **callee entry** and is keyed on a per-function
flag; the constructor rule runs **caller-side** at the `new` site (when `this` is
created) and is keyed on the callee's `CellKind`. This document describes both,
and how each is realized across the four execution paths: the bytecode
interpreter, the JIT, C++ builtins, and the Static Hermes (SH) native C backend.

---

## Part 1 — `prohibitInvoke` (the per-function flag)

### The two enums

There are two `ProhibitInvoke` enums, with **deliberately different value
orderings**, bridged by `computeProhibitInvoke()`.

IR-level (`include/hermes/IR/IR.h`):

```cpp
enum class ProhibitInvoke {
  ProhibitNone,       // = 0
  ProhibitConstruct,  // = 1
  ProhibitCall,       // = 2
};
```

Backend/runtime (`include/hermes/BCGen/FunctionInfo.h`):

```cpp
enum ProhibitInvoke {
  Call = 0,       // regular (non-new) calls prohibited  => constructor-only
  Construct = 1,  // construct (new) calls prohibited     => call-only
  None = 2,
};
```

> **Gotcha:** in the backend enum the names denote *which kind of invocation is
> prohibited*, and the numeric values are inverted relative to the IR enum.
> Always convert via `computeProhibitInvoke()` (`FunctionInfo.h`); never assume
> the numeric values of the two enums line up.

### How the restriction is decided

The restriction is derived from the function's definition kind in
`Function::getProhibitInvoke()` (`lib/IR/IR.cpp`):

  - ES6 base/derived constructors → `ProhibitCall` (must use `new`).
  - Generators, async functions, arrow functions, and object/class methods →
    `ProhibitConstruct` (cannot use `new`).
  - Everything else → `ProhibitNone`.

### Where it is stored

The value lives in one of two places, depending on the function kind:

  - **Bytecode functions:** a 2-bit field in the `FunctionHeaderFlag` byte of the
    code block header (`include/hermes/BCGen/HBC/BytecodeFileFormat.h`), written
    during bytecode generation (`lib/BCGen/HBC/BytecodeGenerator.cpp`). Bit
    layout within the flags byte (LSB first): bits 0-1 `ProhibitInvoke`, bit 2
    `StrictMode`, bit 3 `HasExceptionHandler`, bit 4 `HasDebugInfo`, bit 5
    `Overflowed`, bits 6-7 `Kind`.
  - **SH/native-compiled functions (`NativeJSFunction`):** a 2-bit
    `prohibit_invoke` field in `SHNativeFuncInfo` (`include/hermes/VM/static_h.h`),
    emitted into the generated C function-info table (`lib/BCGen/SH/SH.cpp`).
  - **C++ builtins (`NativeFunction` / `NativeConstructor`):** no flag at all —
    for these, invocation validity is handled by the constructor rule (Part 2)
    and by hand-written checks in the builtin body.

The central predicate for the flag (`BytecodeFileFormat.h`):

```cpp
bool isCallProhibited(bool construct) const {
  return getProhibitInvoke() == (uint8_t)construct;
}
```

This exploits the backend enum values: for a construct call
(`construct == true == 1`) it matches `Construct(1)`; for a plain call
(`construct == false == 0`) it matches `Call(0)`; `None(2)` never matches
either, so one comparison covers both directions.

### Enforcement — a callee-entry check

The `prohibitInvoke` flag is checked at the **callee entry**, keyed on the
*callee's* own flags. There is no central gate: `Callable::call` (the vtable
funnel), `Interpreter::handleCallSlowPath`, and the SH `doCall` / `_sh_ljs_call`
helpers all deliberately do *not* check the flag; they only dispatch. Being a
callee-entry check keyed on the callee is exactly what makes it caller-agnostic
for bytecode functions.

| Callee kind | Flag enforced at entry? | Where |
|---|---|---|
| Bytecode (interpreted) | Yes | `interpretFunction` entry preamble and inline Call fast-path (`lib/VM/Interpreter.cpp`) |
| Bytecode (JIT-compiled) | Yes | Inline in the compiled prologue (`lib/VM/JIT/arm64/JitEmitter.cpp`) → shared slow-path helpers |
| C++ builtin | N/A (no flag) | Constructor rule (Part 2) + manual body checks |
| SH-compiled (`NativeJSFunction`) | No | Flag is stored but never checked at call time |

#### Interpreter (bytecode callees)

Two sites, both raising the same two errors:

  - Entry preamble of `Interpreter::interpretFunction` (`lib/VM/Interpreter.cpp`):
    checks `curCodeBlock->getHeaderFlags().isCallProhibited(newFrame.isConstructorCall())`.
    Done before the callee frame is fully set up, so the exception appears to
    come from the call site.
  - Inline Call fast-path (`lib/VM/Interpreter.cpp`): checks
    `calleeBlock->getHeaderFlags().isCallProhibited(isCtorCall)`, where
    `isCtorCall` is (`new.target` is not undefined).
  - Messages: `"Function is not a constructor"` /
    `"Class constructor invoked without new"`.

The inline fast path is taken only for `JSFunction` callees *without* a JIT
pointer; when the callee is JIT-compiled, the interpreter skips its own check
because the JIT prologue self-checks.

#### JIT (bytecode callees; arm64 only)

Emitted **inline at the compiled function's own prologue**
(`lib/VM/JIT/arm64/JitEmitter.cpp`): reads
`getHeaderFlags().getProhibitInvoke()`, loads `new.target`, compares to
undefined, and branches to an out-of-line slow path that calls the shared
helpers `_sh_throw_invalid_call` / `_sh_throw_invalid_construct`
(`lib/VM/JIT/arm64/JitHandlers.cpp`) — same two error messages. Complete and
caller-agnostic. (arm64 is the only JIT backend present.)

#### SH native backend (`NativeJSFunction` callees) — flag enforcement gap

The SH C backend **records** `prohibit_invoke` in `SHNativeFuncInfo` and uses it
for `isConstructor()` (`lib/VM/Operations.cpp`) and `.prototype` suppression
(`lib/VM/Callable.cpp`), but **emits no callee-entry guard** for it:

  - The generated function prologue (`lib/BCGen/SH/SH.cpp`) emits
    stack-overflow / frame / try setup but no `new.target`/prohibit check.
  - Call lowering → `_sh_ljs_call` → `doCall` →
    `NativeJSFunction::_legacyCall` (just `functionPtr_(shr)`) — no flag check.
  - The code flags this as unfinished:
    `TODO(T168592126) standardize on where we perform function call validation
    for the native backend.`
  - Consistent with the gap, there is no SH-backend test for `prohibitInvoke`;
    the existing `test/hermes/prohibit-invoke.js` runs under the interpreter.

Note this gap is specifically about the *flag*. The caller-side constructor rule
(Part 2) *is* present in the SH backend via `_sh_ljs_create_this`.

---

## Part 2 — The constructor rule (`NativeFunction` cannot be `new`ed)

This is a separate, `CellKind`-based rule enforced **caller-side** at the `new`
site, when `this` is created — before the callee frame is set up. It rejects
`new`ing a plain C++ builtin (`NativeFunction`) that is not a `NativeConstructor`.
It does **not** consult the `prohibitInvoke` flag.

### Interpreter: `CreateThisForNew` / `CreateThisForSuper` → `createThisImpl`

For `new X()`, the compiler emits a `CreateThisForNew` opcode (and
`CreateThisForSuper` for `super()` in derived constructors) *before* the actual
`Construct`. The interpreter handlers (`lib/VM/Interpreter.cpp`) call
`Interpreter::createThisImpl` (`lib/VM/Interpreter-slowpaths.cpp`), which
classifies the callee by `CellKind`:

  - `>= CallableExpectsThisKind_first` (`JSFunction`, `NativeJSFunction`): the
    callee wants a pre-made `this` → allocate the object.
  - `>= CallableMakesThisKind_first` (`NativeConstructor`,
    `FinalizableNativeFunction`, `JSCallableProxy`, `NativeJSClass`, `JSClass`):
    the callee makes its own `this` → return `undefined`.
  - `>= CallableUnknownMakesThisKind_first` (`BoundFunction`, `NativeFunction`):
    walk the bound-function target chain, then re-check. If after unwrapping the
    target is still a plain `NativeFunction` (not promoted into the "makes this"
    range where `NativeConstructor` lives), throw
    `"This function cannot be used as a constructor."`
  - else (not a `Callable` / not an object): throw via `raiseTypeErrorForValue`
    with `" cannot be used as a constructor."`.

The ordered `CellKind` ranges that make the `>=` comparisons work are defined in
`include/hermes/VM/CellKinds.def` (`CallableUnknownMakesThis = {BoundFunction,
NativeFunction}`, `CallableMakesThis` starts at `NativeConstructor`,
`CallableExpectsThis = {JSFunction, NativeJSFunction}`), with contiguity enforced
by static_asserts in `include/hermes/VM/CellKind.h`.

### SH backend: `_sh_ljs_create_this`

`_sh_ljs_create_this` (`lib/VM/StaticH.cpp`) is the SH analog with identical
`CellKind` logic and the same `"This function cannot be used as a constructor."`
throw. It additionally validates up front that `new.target` is a `Callable`
(`" invalid new.target."`). It is emitted by `generateCreateThisInst`
(`lib/BCGen/SH/SH.cpp`), and the JIT emits it too
(`lib/VM/JIT/arm64/JitEmitter.cpp`). So the constructor rule is present in all
backends — this is the caller-side construct gate the SH `prohibitInvoke` gap
does *not* affect.

### C++ builtins also self-check

Beyond the `CellKind` rule, dual/opposite-mode builtins enforce their own
contract in their body by inspecting `args.isConstructorCall()`, e.g.:

```cpp
if (args.isConstructorCall())
  return runtime.raiseTypeError("BigInt is not a constructor");        // BigInt.cpp
if (!args.isConstructorCall())
  return runtime.raiseTypeError("ArrayBuffer() called ...");            // ArrayBuffer.cpp
```

Dual-mode builtins (Array, Date, Error, Boolean) branch on
`args.isConstructorCall()` internally. `NativeConstructor` itself is largely a
marker subclass (its `_callImpl` only *asserts* invariants in debug builds).

---

## How the two mechanisms combine

During `new X()` the two checks fire in order:

1. **Caller-side (constructor rule).** `CreateThisForNew` / `createThisImpl`
   (interpreter) or `_sh_ljs_create_this` (SH/JIT) runs first, classifies the
   callee by `CellKind`, and either allocates `this`, returns `undefined`, or
   throws `"This function cannot be used as a constructor."` for a plain
   `NativeFunction`. No call frame exists yet.
2. **Callee-entry (`prohibitInvoke` flag).** The subsequent `Construct` performs
   the call; on entry the callee's `prohibitInvoke` flag is checked via
   `isCallProhibited(isCtorCall)`, throwing `"Function is not a constructor"` /
   `"Class constructor invoked without new"`.

They catch different things and are complementary rather than redundant:

| | Caller-side constructor rule | Callee-entry `prohibitInvoke` |
|---|---|---|
| Mechanism | `CellKind` range check | `prohibit_invoke` / header-flag bit |
| Catches | plain `NativeFunction` used with `new` | JS/native function whose flag forbids that invocation (arrow/method/generator/async/class ctor) |
| Timing | at the `new` site, before the frame exists | when the callee frame is set up |
| Message | `"This function cannot be used as a constructor."` | `"Function is not a constructor"` / `"Class constructor invoked without new"` |
| Sites | `createThisImpl` (interp), `_sh_ljs_create_this` (SH/JIT) | `Interpreter.cpp` entry/fast-path, JIT prologue |

### `isConstructor` — the unified predicate

`isConstructor` (`lib/VM/Operations.cpp`) is the one place that layers both
mechanisms together. It walks the `JSCallableProxy` and `BoundFunction` target
chains to the eventual target, then:

  - for a `JSFunction` (bytecode), consults the flag via
    `!isCallProhibited(/*construct=*/true)`;
  - for a `NativeJSFunction`, consults `prohibit_invoke != ProhibitInvoke::Construct`;
  - otherwise applies the `CellKind` rule: a plain `NativeFunction` is not a
    constructor unless it is a `FinalizableNativeFunction` or `NativeConstructor`.

Note that `createThisImpl` / `_sh_ljs_create_this` do **not** call
`isConstructor` and do **not** check the flag — they only do the `CellKind`
check. `isConstructor` is used by RJS/spec operations (e.g. `Reflect.construct`,
`instanceof` helpers), not as the `new`-site gate.

---

## Cross-calling behavior

Because the `prohibitInvoke` check is at the callee entry and keyed on the
callee's own flags, and the constructor rule is at the `new` site keyed on the
callee's `CellKind`, both hold uniformly regardless of the caller:

  - Interpreter → bytecode: flag enforced (interpreter entry / fast path);
    constructor rule enforced at `CreateThisForNew`.
  - JIT → bytecode, and anyone → JIT-compiled bytecode: flag enforced (JIT
    prologue self-checks); constructor rule enforced via emitted
    `_sh_ljs_create_this`.
  - SH → bytecode: flag enforced — `doCall` routes `JSFunction` callees to their
    JIT pointer or `_interpret`, both of which carry the callee-entry check;
    constructor rule enforced via `_sh_ljs_create_this`.
  - Anyone → C++ builtin: the constructor rule (caller-side `CellKind` check)
    rejects `new`ing a non-constructor builtin; call/construct contracts beyond
    that rely on the builtin's own manual checks.
  - Anyone → SH-compiled `NativeJSFunction`: the constructor rule still applies
    caller-side, but the `prohibitInvoke` *flag* is **not** enforced at
    invocation time. This is the one real hole (`TODO(T168592126)`).

---

## Other consumers (not enforcement)

  - `.prototype` setup: `lib/VM/Callable.cpp` suppresses `.prototype` on
    call-only (`ProhibitInvoke::Construct`) non-generator functions.
  - Inliner: `lib/Optimizer/Scalar/Inlining.cpp` refuses to inline a construct
    call into a `ProhibitConstruct` function, and vice versa for `ProhibitCall`.
  - InstSimplify: `lib/Optimizer/Scalar/InstSimplify.cpp` can drop an unused
    `new.target` when `ProhibitNone`.
  - Disassembler: `lib/BCGen/HBC/BytecodeDisassembler.cpp` prints `Constructor`
    for `ProhibitInvoke::Call` and `NCFunction` for `ProhibitInvoke::Construct`.

---

## Quick reference

`prohibitInvoke` (per-function flag, callee-entry):

  - Enums: `include/hermes/IR/IR.h`, `include/hermes/BCGen/FunctionInfo.h`
  - Decision logic: `lib/IR/IR.cpp` (`Function::getProhibitInvoke()`)
  - Predicate: `include/hermes/BCGen/HBC/BytecodeFileFormat.h` (`isCallProhibited`)
  - Set: `lib/BCGen/HBC/BytecodeGenerator.cpp` (bytecode), `lib/BCGen/SH/SH.cpp` (SH)
  - Interpreter enforcement: `lib/VM/Interpreter.cpp`
  - JIT enforcement: `lib/VM/JIT/arm64/JitEmitter.cpp`, `lib/VM/JIT/arm64/JitHandlers.cpp`
  - SH flag gap: `lib/BCGen/SH/SH.cpp`, `lib/VM/StaticH.cpp` (`TODO(T168592126)`)

Constructor rule (`CellKind`-based, caller-side):

  - Interpreter: `CreateThisForNew`/`CreateThisForSuper` in `lib/VM/Interpreter.cpp`
    → `createThisImpl` in `lib/VM/Interpreter-slowpaths.cpp`
  - SH/JIT: `_sh_ljs_create_this` in `lib/VM/StaticH.cpp`, emitted by
    `lib/BCGen/SH/SH.cpp` and `lib/VM/JIT/arm64/JitEmitter.cpp`
  - CellKind ranges: `include/hermes/VM/CellKinds.def`, `include/hermes/VM/CellKind.h`
  - Manual builtin checks: `lib/VM/JSLib/{BigInt,ArrayBuffer,DataView,Boolean,Array,Date,Error}.cpp`

Unified predicate:

  - `isConstructor`: `lib/VM/Operations.cpp`
