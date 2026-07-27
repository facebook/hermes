---
id: prohibit-invoke
title: ProhibitInvoke — call/construct restrictions
---

### Introduction

`prohibitInvoke` encodes, per function, whether that function may be invoked
normally, invoked with `new`, or both. It is how Hermes implements restrictions
such as "an ES6 class constructor cannot be called without `new`" and "a method,
arrow function, generator, or async function cannot be called with `new`".

This document describes where the restriction is defined, how it is stored, how
it is computed by the compiler, and how it is enforced at runtime across the four
ways a function can be executed: the bytecode interpreter, the JIT, C++ builtins,
and the Static Hermes (SH) native C backend.

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
    the restriction lives in hand-written C++ code.

The central predicate everyone uses (`BytecodeFileFormat.h`):

```cpp
bool isCallProhibited(bool construct) const {
  return getProhibitInvoke() == (uint8_t)construct;
}
```

This exploits the backend enum values: for a construct call
(`construct == true == 1`) it matches `Construct(1)`; for a plain call
(`construct == false == 0`) it matches `Call(0)`; `None(2)` never matches
either, so one comparison covers both directions.

### Enforcement per function kind

There is **no central gate**. `Callable::call` (the vtable funnel),
`Interpreter::handleCallSlowPath`, and the SH `doCall` / `_sh_ljs_call` helpers
all deliberately do *not* check `prohibitInvoke`; they only dispatch. Instead,
for bytecode functions the check happens at the **callee entry**, keyed on the
*callee's* own flags. This is what makes it caller-agnostic.

| Callee kind | Enforced? | Where |
|---|---|---|
| Bytecode (interpreted) | Yes | `interpretFunction` entry preamble and inline Call fast-path (`lib/VM/Interpreter.cpp`) |
| Bytecode (JIT-compiled) | Yes | Inline in the compiled prologue (`lib/VM/JIT/arm64/JitEmitter.cpp`) → shared slow-path helpers |
| C++ builtin | Manual only | Each builtin inspects `args.isConstructorCall()` in its own body |
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

#### C++ builtins (`NativeFunction` / `NativeConstructor`)

`NativeConstructor` is essentially a marker subclass, used for the
`isConstructor` query, heap snapshots, and `parentForNewThis_RJS`. Its
`_callImpl` is compiled only in debug builds and merely *asserts* invariants
before delegating to `NativeFunction::_callImpl`. There is **no automatic
new/non-new rejection**. Each builtin enforces its own contract manually, e.g.:

```cpp
if (args.isConstructorCall())
  return runtime.raiseTypeError("BigInt is not a constructor");        // BigInt.cpp
if (!args.isConstructorCall())
  return runtime.raiseTypeError("ArrayBuffer() called ...");            // ArrayBuffer.cpp
```

Dual-mode builtins (Array, Date, Error, Boolean) branch on
`args.isConstructorCall()` internally.

#### SH native backend (`NativeJSFunction` callees) — enforcement gap

The SH C backend **records** `prohibit_invoke` in `SHNativeFuncInfo` and uses it
for `isConstructor()` (`lib/VM/Operations.cpp`) and `.prototype` suppression
(`lib/VM/Callable.cpp`), but **emits no callee-entry guard**:

  - The generated function prologue (`lib/BCGen/SH/SH.cpp`) emits
    stack-overflow / frame / try setup but no `new.target`/prohibit check.
  - Call lowering → `_sh_ljs_call` → `doCall` →
    `NativeJSFunction::_legacyCall` (just `functionPtr_(shr)`) — no check.
  - The only construct-side validation is in `_sh_ljs_create_this`
    (`lib/VM/StaticH.cpp`), which checks Callable/CellKind and throws
    `"This function cannot be used as a constructor."` when it reaches a
    `NativeFunction`, but does **not** consult the `prohibit_invoke` bit.
  - The code flags this as unfinished:
    `TODO(T168592126) standardize on where we perform function call validation
    for the native backend.`
  - Consistent with the gap, there is no SH-backend test for `prohibitInvoke`;
    the existing `test/hermes/prohibit-invoke.js` runs under the interpreter.

### Cross-calling behavior

Because the bytecode check is at the callee entry and keyed on the callee's own
flags, it holds uniformly regardless of the caller:

  - Interpreter → bytecode: enforced (interpreter entry / fast path).
  - JIT → bytecode, and anyone → JIT-compiled bytecode: enforced (JIT prologue
    self-checks).
  - SH → bytecode: enforced — `doCall` routes `JSFunction` callees to their JIT
    pointer or `_interpret`, both of which carry the callee-entry check.
  - Anyone → C++ builtin: relies entirely on the builtin's own manual check
    (works because the check is in the callee body).
  - Anyone → SH-compiled `NativeJSFunction`: **not enforced** — neither caller
    nor callee checks the flag. This is the one real hole.

The design is intentionally callee-side, not call-site-side. The only
call-site-like spots (`doCall`, `_sh_ljs_call`, `handleCallSlowPath`,
`Callable::call`) deliberately skip the check and just dispatch.

### Other consumers (not enforcement)

  - `isConstructor` query: `lib/VM/Operations.cpp` reads the header flag for
    `JSFunction` and `prohibit_invoke` for `NativeJSFunction`.
  - `.prototype` setup: `lib/VM/Callable.cpp` suppresses `.prototype` on
    call-only (`ProhibitInvoke::Construct`) non-generator functions.
  - Inliner: `lib/Optimizer/Scalar/Inlining.cpp` refuses to inline a construct
    call into a `ProhibitConstruct` function, and vice versa for `ProhibitCall`.
  - InstSimplify: `lib/Optimizer/Scalar/InstSimplify.cpp` can drop an unused
    `new.target` when `ProhibitNone`.
  - Disassembler: `lib/BCGen/HBC/BytecodeDisassembler.cpp` prints `Constructor`
    for `ProhibitInvoke::Call` and `NCFunction` for `ProhibitInvoke::Construct`.

### Quick reference

  - Enums: `include/hermes/IR/IR.h`, `include/hermes/BCGen/FunctionInfo.h`
  - Decision logic: `lib/IR/IR.cpp` (`Function::getProhibitInvoke()`)
  - Predicate: `include/hermes/BCGen/HBC/BytecodeFileFormat.h` (`isCallProhibited`)
  - Set: `lib/BCGen/HBC/BytecodeGenerator.cpp` (bytecode), `lib/BCGen/SH/SH.cpp` (SH)
  - Interpreter enforcement: `lib/VM/Interpreter.cpp`
  - JIT enforcement: `lib/VM/JIT/arm64/JitEmitter.cpp`, `lib/VM/JIT/arm64/JitHandlers.cpp`
  - SH gap: `lib/BCGen/SH/SH.cpp`, `lib/VM/StaticH.cpp` (`TODO(T168592126)`)
  - Flag readers: `lib/VM/Operations.cpp` (`isConstructor`), `lib/VM/Callable.cpp` (prototype)
  - Manual builtin checks: `lib/VM/JSLib/{BigInt,ArrayBuffer,DataView,Boolean,Array,Date,Error}.cpp`
