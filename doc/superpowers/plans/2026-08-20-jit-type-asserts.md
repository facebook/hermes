# JIT FR Type Assertions Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Emit runtime checks into arm64 JIT'ed code, under a new default-off
flag, that verify the FR type facts the emitters rely on when they elide tag
checks and when they place values in class-restricted callee-saved registers.

**Architecture:** One check primitive (`Emitter::emitTypeAssert`) that uses
only the two reserved scratch registers and never touches `TempRegAlloc`,
plus one out-of-line fail stub per site riding the existing `slowPaths_`
deque, plus one shared per-function tail that calls a noreturn C++ handler.
Consumption-side checks (Classes A and B) go where the fast path has already
materialized its operand and before that fast path's own compare. The
producer-side check (Class C) runs from a hook at the bytecode instruction
boundary in `compileBB`, not from inside the register-file helpers.

**Tech Stack:** C++17 (no exceptions, no RTTI), asmjit, buck2 (the build and
test path for all work), CMake + Ninja (must stay green, out-of-tree only),
LLVM lit + FileCheck, Sapling.

**Design:** `doc/superpowers/specs/2026-08-20-jit-type-asserts-design.md`.
Read it before Task 1; this plan does not restate its motivation or its
justification for any of the choices below.

## Global Constraints

- **Emitted code must be byte-identical with the flag off.** After every
  task, `utils/jit/jit-diff.sh <before> <after>` must report
  `dumps are identical`. This is the single most important constraint: a
  debugging feature that perturbs production codegen is worse than none.
- **The flag is a runtime flag, not a build-time one.** Gate everything on
  `emitTypeAsserts_`. Never gate emitted-code paths on `#ifndef NDEBUG` —
  a release build with `-Xjit-emit-type-asserts` must emit the checks.
  (`assert()` on compile-time bookkeeping is still fine and encouraged.)
- **Check sequences use `x16`/`x17` only.** Never call `allocTempGpX`,
  `allocTempVecD`, `getOrAllocFRIn*`, `syncToFrame`, or `freeReg` from
  check-emission code. No new spills, no LRU state changes.
- **Every insertion point must be verified flags-dead by the author.** This
  is not a property arm64 emitters have in general; `selectObject` holds
  NZCV across `getOrAllocFRInGpX` and consumes it in `csel`. State the
  verification for each site in the task report.
- 80-column lines, 2-space indent, doc comment on every declaration, Meta
  copyright header on every new file, trailing newline, `\n` line endings.
- **Build and test with buck2. Never create a CMake build directory inside
  `fbsource`.** This checkout is on EdenFS, which does not support CMake
  build trees (`facebook/CLAUDE-meta.md`). CMake must stay green, but it is
  verified from a build directory outside `fbsource` — the controller
  supplies the path in each dispatch — and never by configuring one here.
- `arc f` and `arc lint` clean.

## Design points this plan resolves

The design doc leaves four things implicit that turn into concrete decisions
here. They are listed once, up front, rather than argued inside each task.

1. **The Class C hook is a sibling of `assertPostInstructionInvariants`, not
   an addition to its body.** That function is `{}` under `NDEBUG`
   (`JitEmitter.h:473`), so putting emitted-code generation inside it would
   silently disable Class C in release builds where the flag is on.
2. **The leaked object is the `std::vector`, not its buffer.** The design
   says to leak a `TypeAssertSite[]` and store its address in RO data, but
   the array's address is not known until every site is registered, which
   is after the RO-data constant must be emitted. Leaking the vector
   *object* gives a stable address at first use and needs no back-patching.
3. **The handler does not take `SHRuntime *`.** It formats from the site
   record alone (`CodeBlock::getFunctionID`, `getNameString`), so the fail
   tail is one instruction shorter and the stubs pass the site index in
   `w0` rather than `w1`.
4. **`TypeAssertSite` stores a bytecode offset, not an `inst::Inst *`.**
   Computed at emit time with `codeBlock_->getOffsetOf(emittingIP)`, the
   same call `getBytecodeIP` makes.

## Verification harness

Everything here is buck2. `jit-diff.sh` and `jit-dump.sh` take **binaries**,
not a build directory, so they work with buck2 output unchanged — resolve
the binary once and reuse it:

```bash
HERMES=$(buck2 build //xplat/static_h:hermes --show-full-output | awk '{print $2}')
```

Capture the baseline binary once, in Task 1, from a pristine tree before any
edit. Both sides of every later comparison must come from the same build
system, so a baseline built any other way is not usable:

```bash
cp "$HERMES" /tmp/hermes-typeassert-base
```

Then after each task:

```bash
HERMES=$(buck2 build //xplat/static_h:hermes --show-full-output | awk '{print $2}')
utils/jit/jit-diff.sh /tmp/hermes-typeassert-base "$HERMES"
buck2 test //xplat/static_h:lit
arc lint
```

`jit-diff.sh` defaults to running without the new flag, so
`dumps are identical` is the expected result for every task including the
last one. To see what a task *added*, capture with the flag explicitly:

```bash
utils/jit/jit-dump.sh --raw -c test/jit/type-asserts.js "$HERMES" | less
```

Note that `jit-dump.sh` does not pass `-Xjit-emit-type-asserts`; add it to
the invocation by hand when inspecting, or run the binary directly.

**CMake must also stay green**, but from a build directory *outside*
`fbsource`; the controller supplies the path in each dispatch. Never
configure one inside the repository — EdenFS does not support CMake build
trees, and a 4 GB build directory in the source tree is a real cost even
though `.gitignore` hides it.

`buck2 test //xplat/static_h:testsuite_tests` (and `:testsuite_tests_jit`)
are the heavyweight suites. Do not run them per task; they are for the final
review, and only when the human asks.

---

### Task 1: Add the `-Xjit-emit-type-asserts` flag

Plumbing only. No emitted-code change, which makes this the task that
establishes the byte-identical baseline.

The chain mirrors `jitEmitAsserts` exactly, at five sites. Do not reuse
`emitAsserts_`: it defaults to **true** under `!NDEBUG`
(`include/hermes/VM/RuntimeFlags.h:286`), so the checks would be emitted
in every debug lit run.

**Files:**
- Modify: `include/hermes/VM/RuntimeFlags.h:286` (add after `JITEmitAsserts`)
- Modify: `include/hermes/ConsoleHost/ConsoleHost.h:171`
- Modify: `tools/hermes/hermes.cpp:150`
- Modify: `lib/ConsoleHost/ConsoleHost.cpp:969`
- Modify: `include/hermes/VM/JIT/JIT.h:127` (no-op stubs for the non-JIT build)
- Modify: `include/hermes/VM/JIT/arm64/JIT.h:143,161,191`
- Modify: `lib/VM/JIT/arm64/JIT.cpp:117` (`Compiler` ctor init list)
- Modify: `lib/VM/JIT/arm64/JitEmitter.h:307,460` (member + ctor param)
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp:64` (ctor init list)

**Interfaces:**
- Consumes: nothing.
- Produces: `Emitter::emitTypeAsserts_` (`bool const`, private),
  `JITContext::setEmitTypeAsserts(bool)`,
  `JITContext::getEmitTypeAsserts()`, and the
  `-Xjit-emit-type-asserts` command-line flag.

- [ ] **Step 1: Capture the baseline binary**

```bash
HERMES=$(buck2 build //xplat/static_h:hermes --show-full-output | awk '{print $2}')
cp "$HERMES" /tmp/hermes-typeassert-base
```

- [ ] **Step 2: Add the command-line flag**

In `include/hermes/VM/RuntimeFlags.h`, immediately after the
`JITEmitAsserts` option. Unlike that one, it is unconditionally
`init(false)`:

```cpp
  llvh::cl::opt<bool> JITEmitTypeAsserts{
      "Xjit-emit-type-asserts",
      llvh::cl::Hidden,
      llvh::cl::cat(RuntimeCategory),
      llvh::cl::desc(
          "(default false) Whether to verify FR type assumptions in JIT "
          "compiled code"),
      llvh::cl::init(false)};
```

- [ ] **Step 3: Thread it to the JIT context**

`include/hermes/ConsoleHost/ConsoleHost.h`, after `jitEmitAsserts`:

```cpp
  /// Verify FR type assumptions in JIT'ed code.
  bool jitEmitTypeAsserts{false};
```

`tools/hermes/hermes.cpp`, after line 150:

```cpp
  options.jitEmitTypeAsserts = flags.JITEmitTypeAsserts;
```

`lib/ConsoleHost/ConsoleHost.cpp`, after line 969:

```cpp
  runtime->getJITContext().setEmitTypeAsserts(options.jitEmitTypeAsserts);
```

- [ ] **Step 4: Add the JITContext accessors**

`include/hermes/VM/JIT/arm64/JIT.h`, beside `setEmitAsserts`/`getEmitAsserts`
and the `emitAsserts_` member:

```cpp
  /// Set the flag to verify FR type assumptions in the JIT'ed code.
  void setEmitTypeAsserts(bool emitTypeAsserts) {
    emitTypeAsserts_ = emitTypeAsserts;
  }
```

```cpp
  /// \return true if we should verify FR type assumptions in JIT'ed code.
  bool getEmitTypeAsserts() {
    return emitTypeAsserts_;
  }
```

```cpp
  /// Whether to verify FR type assumptions in the JIT'ed code.
  bool emitTypeAsserts_{false};
```

`include/hermes/VM/JIT/JIT.h` gets the matching no-ops, beside the existing
ones at lines 127 and 136:

```cpp
  void setEmitTypeAsserts(bool emitTypeAsserts) {}
```

```cpp
  bool getEmitTypeAsserts() {
    return false;
  }
```

- [ ] **Step 5: Thread it into the Emitter**

`JitEmitter.h`, after the `emitAsserts_` member at line 307:

```cpp
  /// Whether to verify FR type assumptions in the JIT'ed code.
  bool const emitTypeAsserts_;
```

Add a `bool emitTypeAsserts` parameter to the `Emitter` constructor
declaration (line 460) directly after `emitAsserts`, mirror it in the
definition's init list (`JitEmitter.cpp:64`), and pass
`jc.getEmitTypeAsserts()` from the `Compiler` ctor (`JIT.cpp:117`).

- [ ] **Step 6: Verify the flag exists and changes nothing**

```bash
HERMES=$(buck2 build //xplat/static_h:hermes --show-full-output | awk '{print $2}')
"$HERMES" -Xjit=force -Xjit-emit-type-asserts test/jit/binops.js
utils/jit/jit-diff.sh /tmp/hermes-typeassert-base "$HERMES"
```

Expected: the script exits 0 with `jit-diff: dumps are identical`, and the
run with the flag produces identical program output to a run without it.

- [ ] **Step 7: Commit**

```bash
sl commit include/hermes/VM/RuntimeFlags.h \
  include/hermes/ConsoleHost/ConsoleHost.h tools/hermes/hermes.cpp \
  lib/ConsoleHost/ConsoleHost.cpp include/hermes/VM/JIT/JIT.h \
  include/hermes/VM/JIT/arm64/JIT.h lib/VM/JIT/arm64/JIT.cpp \
  lib/VM/JIT/arm64/JitEmitter.h lib/VM/JIT/arm64/JitEmitter.cpp \
  --reason "commit type assert flag - sl help commit" \
  -m "..."
```

---

### Task 2: The check primitive, the failure path, and the first site

The primitive is not independently testable, so this task also converts one
Class A site — `arithBinOp` under `forceNumber` — which makes the whole
chain observable end to end.

**Files:**
- Modify: `lib/VM/JIT/arm64/JitEmitter.h`
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp`
- Modify: `lib/VM/JIT/arm64/JitEmitter-arith.cpp`
- Create: `test/jit/type-asserts.js`

**Interfaces:**
- Consumes: `Emitter::emitTypeAsserts_` from Task 1.
- Produces, in `namespace hermes::vm::arm64`:
  - `enum class TypePred : uint8_t` with `IsNumber`, `IsBool`,
    `NotPointer`, `BitComparable`, `IsObject`
  - `const char *typePredName(TypePred)`
  - `struct TypeAssertSite { CodeBlock *codeBlock; uint32_t bytecodeOfs;
    uint16_t frIndex; TypePred pred; }`
  - `void Emitter::emitTypeAssert(FR fr, HWReg hwVal, TypePred pred)`
  - `void Emitter::emitTypeAssertGpX(FR fr, const a64::GpX &xVal,
    TypePred pred)` (private)
  - `void Emitter::emitTypeAssertFailTail()` (private)
  - `[[noreturn]] void _jit_type_assert_failed(uint32_t siteIdx,
    const std::vector<TypeAssertSite> *sites)`
  - `static constexpr auto xScratch2 = a64::x17;`

- [ ] **Step 1: Declare the second scratch register**

`JitEmitter.h`, at the `xScratch` definition (line 203). Replace the
existing comment and add the second register:

```cpp
/// Scratch registers. x16/x17 sit outside the register allocator and are
/// used as scratch (call targets, IP materialization, type assert check
/// sequences); nothing holds a value in them across an emitter call.
static constexpr auto xScratch = a64::x16;
static constexpr auto xScratch2 = a64::x17;
```

- [ ] **Step 2: Add the predicate enum and the site record**

`JitEmitter.h`, above `class Emitter`:

```cpp
/// A property of an FR's value that a fast path relies on. These are the
/// predicates the emitters actually exploit, which is narrower and more
/// useful than the declared FRType.
enum class TypePred : uint8_t {
  /// Unsigned-below (HVTag_First << kHV_NumDataBits).
  IsNumber,
  /// ETag == HVETag_Bool.
  IsBool,
  /// Tag unsigned-below the pointer range. This is the GC-safety predicate.
  NotPointer,
  /// NotPointer && !IsNumber: comparable by raw bits.
  BitComparable,
  /// Tag == HVTag_Object.
  IsObject,
};

/// \return a human-readable name for \p pred, for diagnostics.
const char *typePredName(TypePred pred);

/// One emitted type check, recorded so the failure handler can name it.
struct TypeAssertSite {
  CodeBlock *codeBlock;
  uint32_t bytecodeOfs;
  uint16_t frIndex;
  TypePred pred;
};

/// Report a failed JIT type assertion and abort. Called only from JIT'ed
/// code, and never returns, so it needs no register or frame preservation.
[[noreturn]] void _jit_type_assert_failed(
    uint32_t siteIdx,
    const std::vector<TypeAssertSite> *sites);
```

- [ ] **Step 3: Add the Emitter members**

`JitEmitter.h`, in the private data of `class Emitter`, near `slowPaths_`:

```cpp
  /// Records for every emitted type check, in site-index order. Allocated
  /// on first use and deliberately leaked: its address is baked into the
  /// emitted code, the handler that reads it never returns, and JIT'ed
  /// code is never freed.
  std::vector<TypeAssertSite> *typeAssertSites_ = nullptr;
  /// The shared failure tail, bound only if there is at least one site.
  asmjit::Label typeAssertFailLab_{};
```

And the public/private methods:

```cpp
  /// Emit, only when emitTypeAsserts_ is set, a trap-on-violation check
  /// that the value of \p fr, currently held in \p hwVal, satisfies
  /// \p pred.
  ///
  /// Uses only xScratch/xScratch2 and never touches the register
  /// allocator, so it is a pure insertion. It clobbers NZCV, so the caller
  /// must have verified that flags are dead at the insertion point. That
  /// is an obligation, not a property emitters have in general: see
  /// selectObject, which holds flags across getOrAllocFRInGpX.
  void emitTypeAssert(FR fr, HWReg hwVal, TypePred pred);
```

- [ ] **Step 4: Implement the primitive**

`JitEmitter.cpp`:

```cpp
const char *typePredName(TypePred pred) {
  switch (pred) {
    case TypePred::IsNumber:
      return "number";
    case TypePred::IsBool:
      return "bool";
    case TypePred::NotPointer:
      return "non-pointer";
    case TypePred::BitComparable:
      return "non-pointer non-number";
    case TypePred::IsObject:
      return "object";
  }
  return "<invalid>";
}

void Emitter::emitTypeAssert(FR fr, HWReg hwVal, TypePred pred) {
  if (LLVM_LIKELY(!emitTypeAsserts_))
    return;
  if (hwVal.isVecD()) {
    a.fmov(xScratch, hwVal.a64VecD());
    emitTypeAssertGpX(fr, xScratch, pred);
  } else {
    emitTypeAssertGpX(fr, hwVal.a64GpX(), pred);
  }
}

void Emitter::emitTypeAssertGpX(
    FR fr,
    const a64::GpX &xVal,
    TypePred pred) {
  assert(emitTypeAsserts_ && "caller must check emitTypeAsserts_");
  if (!typeAssertSites_) {
    typeAssertSites_ = new std::vector<TypeAssertSite>();
    typeAssertFailLab_ = newPrefLabel("TYPEASSERT_FAIL", 0);
  }

  uint32_t idx = (uint32_t)typeAssertSites_->size();
  typeAssertSites_->push_back(TypeAssertSite{
      codeBlock_,
      codeBlock_->getOffsetOf(emittingIP),
      (uint16_t)fr.index(),
      pred});

  comment("// type assert r%u is %s", fr.index(), typePredName(pred));
  asmjit::Label failLab = newPrefLabel("TYPEASSERT_", idx);

  // The helpers below tolerate xTemp == xVal; every such use is the last
  // read of xVal in the sequence.
  switch (pred) {
    case TypePred::IsNumber:
      emit_sh_ljs_is_double(a, xVal, xScratch2);
      a.b_hs(failLab);
      break;
    case TypePred::IsBool:
      emit_sh_ljs_is_bool(a, xScratch, xVal);
      a.b_ne(failLab);
      break;
    case TypePred::NotPointer:
      emit_sh_ljs_get_tag(a, xScratch, xVal);
      emit_sh_ljs_tag_is_pointer(a, xScratch);
      a.b_hs(failLab);
      break;
    case TypePred::BitComparable:
      emit_sh_ljs_is_double(a, xVal, xScratch2);
      a.b_lo(failLab);
      emit_sh_ljs_get_tag(a, xScratch, xVal);
      emit_sh_ljs_tag_is_pointer(a, xScratch);
      a.b_hs(failLab);
      break;
    case TypePred::IsObject:
      emit_sh_ljs_is_object(a, xScratch, xVal);
      a.b_ne(failLab);
      break;
  }

  slowPaths_.emplace_back(
      failLab, emittingIP, [idx](Emitter &em, SlowPath &sp) {
        em.comment("// Type assert failure %u", idx);
        em.a.bind(sp.slowPathLab);
        em.a.mov(a64::w0, idx);
        em.a.b(em.typeAssertFailLab_);
      });
}
```

Condition codes, for review: `emit_sh_ljs_is_double` leaves number ⟺ LO, so
not-a-number is HS. `emit_sh_ljs_tag_is_pointer` leaves pointer ⟺ HS.
`emit_sh_ljs_is_bool` and `emit_sh_ljs_is_object` leave the property ⟺ EQ.

- [ ] **Step 5: Implement the shared tail and the handler**

`JitEmitter.cpp`:

```cpp
void Emitter::emitTypeAssertFailTail() {
  if (!typeAssertSites_)
    return;
  comment("// Type assert failure tail");
  a.bind(typeAssertFailLab_);
  // w0 already holds the site index, set by the per-site stub.
  a.ldr(
      a64::x1,
      a64::Mem(
          roDataLabel_,
          uint64Const(
              (uint64_t)typeAssertSites_, "type assert site table")));
  // Not EMIT_RUNTIME_CALL: that saves the current IP, and emitSlowPaths()
  // has already cleared emittingIP by the time this runs. The handler does
  // not need the IP anyway — the site record carries the bytecode offset.
  EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(
      *this,
      void (*)(uint32_t, const std::vector<TypeAssertSite> *),
      _jit_type_assert_failed);
}

void _jit_type_assert_failed(
    uint32_t siteIdx,
    const std::vector<TypeAssertSite> *sites) {
  const TypeAssertSite &site = (*sites)[siteIdx];
  std::string message;
  llvh::raw_string_ostream os(message);
  os << "JIT type assert failed: function "
     << site.codeBlock->getFunctionID() << "("
     << site.codeBlock->getNameString() << "), bytecode offset "
     << site.bytecodeOfs << ", r" << site.frIndex << ", expected "
     << typePredName(site.pred);
  os.flush();
  hermes_fatal(message);
}
```

Call the tail between the two existing calls at the end of the function
emission (`JitEmitter.cpp:575`), because it allocates an RO-data constant
and must therefore precede `emitROData`, and it is a branch target of slow
paths so it must follow `emitSlowPaths`:

```cpp
  emitSlowPaths();
  emitTypeAssertFailTail();
  emitROData();
```

- [ ] **Step 6: Convert the first site — `arithBinOp` under `forceNumber`**

In `lib/VM/JIT/arm64/JitEmitter-arith.cpp`, in `arithBinOp`, after both
operands have been materialized by `getOrAllocFRInVecD` and before the
fast path's own `fcmp`:

```cpp
  if (forceNumber) {
    emitTypeAssert(frLeft, hwLeft, TypePred::IsNumber);
    emitTypeAssert(frRight, hwRight, TypePred::IsNumber);
  }
```

Flags-dead verification for this site: the two `getOrAllocFRInVecD` calls
are the first flag-relevant emissions in the fast path, and the first
consumer of NZCV (`fcmp`/`b.vs`) is emitted strictly after. Record this in
the task report.

- [ ] **Step 7: Write the lit test**

`test/jit/type-asserts.js`. It runs the same code twice, once with the flag
and once without, and checks that behaviour is unchanged. It deliberately
does **not** FileCheck emitted code: pinning the check sequences line by
line would turn every future emitter change into a test edit, and the
property under test is that behaviour and flag-off codegen do not move.

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
// RUN: %hermes -Xjit=force %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xjit=force -Xjit-emit-type-asserts %s | %FileCheck --match-full-lines %s

function addNumbers(a, b) {
  var sum = 0;
  for (var i = 0; i < 100; ++i)
    sum += a * b - i;
  return sum;
}

print(addNumbers(3, 4));
// CHECK: -3750
```

- [ ] **Step 8: Verify**

```bash
HERMES=$(buck2 build //xplat/static_h:hermes --show-full-output | awk '{print $2}')
utils/jit/jit-diff.sh /tmp/hermes-typeassert-base "$HERMES"
buck2 test //xplat/static_h:lit
```

Expected: `dumps are identical`, all jit tests pass. Then confirm the checks
really are emitted:

```bash
"$HERMES" -Xjit=force -Xjit-emit-type-asserts \
  -Xdump-jitcode=1 test/jit/type-asserts.js | grep -c "type assert"
```

Expected: a nonzero count.

- [ ] **Step 9: Commit**

---

### Task 3: The remaining Class A sites

`forceNumber` promises number operands and the emitters skip both the check
and the slow path, so these are the sites where a front-end type-inference
bug is invisible today.

**Files:**
- Modify: `lib/VM/JIT/arm64/JitEmitter-arith.cpp` (`arithUnop`, `mod`)
- Modify: `lib/VM/JIT/arm64/JitEmitter-control.cpp` (`jCond`)
- Modify: `lib/VM/JIT/arm64/JitEmitter-call.cpp` (`callWithNewTargetLong`)

**Interfaces:**
- Consumes: `emitTypeAssert`, `TypePred` from Task 2.
- Produces: nothing new.

- [ ] **Step 1: `arithUnop`**

After `hwInput = getOrAllocFRInVecD(frInput, true);` and before the
`if (!inputIsNum)` block's `fcmp`:

```cpp
  if (forceNumber)
    emitTypeAssert(frInput, hwInput, TypePred::IsNumber);
```

- [ ] **Step 2: `mod`, `jCond`, `callWithNewTargetLong`**

Same shape in each: immediately after the last operand is materialized,
before that emitter's first flag-setting instruction. For
`callWithNewTargetLong` the operand is `frArgc` and the reliance is the
`fcvtzu` that assumes a number, so the check goes immediately before it.

For each site, verify flags-dead and record the verification.

- [ ] **Step 3: Verify and commit**

Run the harness. `dumps are identical` is still required.

---

### Task 4: `readFRForAssert` and the Class B sites

Class B covers the fast paths that skip a check because `isFRKnownNumber`,
`isFRKnownBool` or `isFRKnownOtherNonPtr` returned true. Note that those
predicates consult `globalType` as well as `localType`
(`JitEmitter.h:1157`), so they test the bytecode header's classification,
not only the JIT's own bookkeeping.

`jmpUndefined` needs a read helper: when the operand is known Number or
Bool it emits *nothing*, so there is no materialized register to check.

**Files:**
- Modify: `lib/VM/JIT/arm64/JitEmitter.h`
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp`
- Modify: `lib/VM/JIT/arm64/JitEmitter-arith.cpp` (`arithBinOp`,
  `arithUnop`, `compareImpl`, `strictEqualImpl`, `toNumber`, `toNumeric`)
- Modify: `lib/VM/JIT/arm64/JitEmitter-control.cpp` (`jCond`,
  `jmpTrueFalse`, `jmpUndefined`, `jStrictEqual`)

**Interfaces:**
- Consumes: `emitTypeAssert`, `TypePred`.
- Produces: `void Emitter::readFRForAssert(FR fr)`.

- [ ] **Step 1: Implement `readFRForAssert`**

The location priority must honour the `FRState` up-to-date invariants, not
just the location order. Reading a stale location would make the assert
report phantom violations, which is the worst failure mode a checking
feature can have.

```cpp
void Emitter::readFRForAssert(FR fr) {
  assert(emitTypeAsserts_ && "caller must check emitTypeAsserts_");
  FRState &frState = frameRegs_[fr.index()];
  assert(!frState.regIsDirty && "reading an FR that is about to be written");

  // Locals always hold the latest value; a global reg holds it only if
  // globalRegUpToDate; otherwise the frame must be up to date.
  if (frState.localGpX) {
    a.mov(xScratch, frState.localGpX.a64GpX());
  } else if (frState.localVecD) {
    a.fmov(xScratch, frState.localVecD.a64VecD());
  } else if (frState.globalReg && frState.globalRegUpToDate) {
    if (frState.globalReg.isGpX())
      a.mov(xScratch, frState.globalReg.a64GpX());
    else
      a.fmov(xScratch, frState.globalReg.a64VecD());
  } else {
    assert(frState.frameUpToDate && "FR has no up-to-date location");
    // The encoding fallback in _loadFrame also uses xScratch, but sequences
    // that use before xScratch receives the loaded value.
    _loadFrame(HWReg(xScratch), fr);
  }
}
```

Declare it in `JitEmitter.h` beside `emitTypeAssert`, private, with the doc
comment from the design.

- [ ] **Step 2: The materialized-operand sites**

For each row of the design's Class B table except `jmpUndefined`, insert the
check right after the operand the elision applies to is materialized:

| Emitter | Guard | Check |
|---|---|---|
| `arithBinOp`, `arithUnop` | `isFRKnownNumber(fr)` | `IsNumber` on that operand |
| `compareImpl`, `jCond` | both known Number | `IsNumber` on both |
| `jmpTrueFalse` | known Number / known Bool | `IsNumber` / `IsBool` |
| `strictEqualImpl` (`arith.cpp:692`) | known Bool or OtherNonPtr / known Number | `BitComparable` / `IsNumber` on the known side |
| `jStrictEqual` (`control.cpp:858`) | same two elisions, separate emitter | `BitComparable` / `IsNumber` on the known side |

`jStrictEqual` is a distinct emitter from `strictEqualImpl`, not a caller
of it, and carries its own copy of both tiers. Convert both.

Note that `arithBinOp`'s `forceNumber` path from Task 2 already sets
`localType = FRType::Number`, so guard the Class B insertion on
`!forceNumber` to avoid emitting the same check twice.

- [ ] **Step 3: The early-return elisions**

Three sites elide by returning before anything is materialized, so there is
no operand register to check and `readFRForAssert` is required. Flags are
trivially dead at all three: the insertion point is an otherwise empty
emission.

`jmpUndefined` (`control.cpp:685`) returns early when the input is known
Number **or** known Bool. Assert the fact that was actually relied on, in
the same order the emitter tests it:

```cpp
  if (emitTypeAsserts_) {
    readFRForAssert(frInput);
    emitTypeAssertGpX(
        frInput,
        xScratch,
        isFRKnownType(frInput, FRType::Number) ? TypePred::IsNumber
                                               : TypePred::IsBool);
  }
```

Do **not** use `BitComparable` here. It is `NotPointer && !IsNumber`, so it
would trap on every legitimate number reaching this path, and it accepts
`undefined` — inverted in both directions.

`toNumber` (`arith.cpp:20`) and `toNumeric` (`arith.cpp:68`) elide with
`return mov(frRes, frInput, false)` when the input is known Number. The
check goes immediately before that early return, on `frInput`:

```cpp
  if (isFRKnownNumber(frInput)) {
    if (emitTypeAsserts_) {
      readFRForAssert(frInput);
      emitTypeAssertGpX(frInput, xScratch, TypePred::IsNumber);
    }
    return mov(frRes, frInput, false);
  }
```

`toInt32` is **not** a Class B site and must not be converted: it has no
`isFRKnownNumber` elision at all — it always emits its check and slow
path.

- [ ] **Step 4: Verify and commit**

---

### Task 5: Class C — producer-side global-class checks

A value stored into an FR that owns a callee-saved global register must
match that register's class, or the GC will either miss a pointer or
misread one. Consumption checks cannot cover this, because the GC's
"consumption" is register residency rather than an emitted instruction.

The hook runs at the bytecode instruction boundary. Do not hook the tail of
`frUpdatedWithHW`: many emitters call it *before* emitting the write
(`catchInst`, `callImpl`'s SavedCodeBlock, `bitNot`, `booleanNot`,
`loadThisNS`, `selectObject`), so it would check a register the value has
not reached — and because `frUpdatedWithHW` clears `regIsDirty`, it could
not even detect that it was in that window. A third write path,
`syncFrameOutParam`, reaches neither `frUpdatedWithHW` nor `movFRFromHW`.

**Files:**
- Modify: `lib/VM/JIT/arm64/JitEmitter.h`
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp`
- Modify: `lib/VM/JIT/arm64/JitEmitter-regalloc.cpp`
- Modify: `lib/VM/JIT/arm64/JIT.cpp:157`

**Interfaces:**
- Consumes: `emitTypeAssertGpX`, `readFRForAssert`, `TypePred`.
- Produces: `void Emitter::recordFRWriteForAssert(FR fr)` (private),
  `void Emitter::emitPendingTypeAsserts()` (public, called from
  `compileBB`).

- [ ] **Step 1: Add the per-instruction written set**

`JitEmitter.h`, in the private data:

```cpp
  /// FRs written by the bytecode instruction currently being emitted whose
  /// global register class requires a check. Drained at each instruction
  /// boundary by emitPendingTypeAsserts().
  llvh::SmallVector<FR, 4> typeAssertPendingWrites_{};
```

```cpp
  /// Record that \p fr was written, so that the instruction boundary can
  /// check the value against its global register class. Records nothing
  /// unless the FR owns a global register. The callers gate on
  /// emitTypeAsserts_.
  void recordFRWriteForAssert(FR fr);
```

- [ ] **Step 2: Record from all three write paths**

`JitEmitter-regalloc.cpp`. In `frUpdatedWithHW`, at the top (before it
clears `regIsDirty`, though the position does not matter — only the
recording does); in `movFRFromHW`'s direct-to-frame `else` branch; and in
`syncFrameOutParam`:

```cpp
  if (LLVM_UNLIKELY(emitTypeAsserts_))
    recordFRWriteForAssert(fr);
```

```cpp
void Emitter::recordFRWriteForAssert(FR fr) {
  FRState &frState = frameRegs_[fr.index()];
  // The two conditions coincide today only because enter()'s allocation
  // loops set globalReg and globalType together and stop together. Pin it,
  // because the check below relies on it.
  assert(
      (frState.globalType != FRType::UnknownPtr) ==
          frState.globalReg.isValid() &&
      "globalType and globalReg must agree");
  if (frState.globalType == FRType::UnknownPtr)
    return;
  if (llvh::is_contained(typeAssertPendingWrites_, fr))
    return;
  typeAssertPendingWrites_.push_back(fr);
}
```

- [ ] **Step 3: Emit at the instruction boundary**

`JitEmitter.cpp`:

```cpp
void Emitter::emitPendingTypeAsserts() {
  if (LLVM_LIKELY(typeAssertPendingWrites_.empty()))
    return;
  for (FR fr : typeAssertPendingWrites_) {
    FRState &frState = frameRegs_[fr.index()];
    TypePred pred = frState.globalType == FRType::Number
        ? TypePred::IsNumber
        : TypePred::NotPointer;
    readFRForAssert(fr);
    emitTypeAssertGpX(fr, xScratch, pred);
  }
  typeAssertPendingWrites_.clear();
}
```

`JIT.cpp`, in `compileBB`, as a sibling of the existing call — **not**
inside `assertPostInstructionInvariants`, whose body is `{}` under
`NDEBUG`:

```cpp
    while (ip != to) {
      em_.emittingIP = ip;
      ip = dispatch(ip);
      em_.assertPostInstructionInvariants();
      em_.emitPendingTypeAsserts();
    }
```

`emittingIP` still names the instruction just emitted, which is what the
site record should carry.

Add to `newBasicBlock`:

```cpp
  assert(
      typeAssertPendingWrites_.empty() &&
      "pending type asserts must be drained at each instruction");
```

- [ ] **Step 4: Verify and commit**

Beyond the standard harness, run a program that exercises number-classed
global registers under the flag and confirm no trap:

```bash
"$HERMES" -Xjit=force -Xjit-emit-type-asserts test/hermes/flow/nbody.js
```

---

### Task 6: Prove the checks can fail

A green run proves only that the suite ran. Each mutation below is applied
to a throwaway working copy, observed, and reverted; none of them is
committed.

**This task produces no commit.** Its deliverable is the three recorded
failure messages, which go into the task report and into the Test Plan of
Task 5's commit. There is no `doc/JIT.md` or any other user-facing document
describing the JIT's `-X` flags in this repository — the `cl::desc` string
added in Task 1 is the documentation. Do not create one as part of this
feature.

**Files:**
- None. Mutations are applied and reverted, never committed.

**Interfaces:**
- Consumes: everything from Tasks 1-5.
- Produces: nothing.

- [ ] **Step 1: Class C — a lying emitter**

Change `loadConstString` to declare `FRType::Number`, rebuild, and run any
test that stores a string into a number-classed FR under the flag. Expect a
fatal naming that FR and `expected number`. Record the exact message.

- [ ] **Step 2: Class B — a weakened elision**

Force `isFRKnownNumber` to return true for one FR in `arithBinOp`, rebuild,
run `test/jit/binops.js` under the flag. Expect a trap at the consumption
site. Record the message.

- [ ] **Step 3: Class A — the `*N` contract**

Apply the Step 2 mutation to the `jLessN` path and run a typed test that
reaches it. Record the message.

- [ ] **Step 4: Revert all mutations and confirm the tree is clean**

```bash
sl status --reason "confirm mutations reverted - sl help status"
```

- [ ] **Step 5: Final verification**

```bash
buck2 test //xplat/static_h:lit
utils/jit/jit-diff.sh /tmp/hermes-typeassert-base "$HERMES"
```

`buck2 test //xplat/static_h:testsuite_tests_jit` is the run that would
actually certify the existing type plumbing, and the design's rollout order
asks for it here. It is heavyweight, so ask the human before running it
rather than starting it unprompted.

---

## Out of scope

- **Class D (typed-mode invariants).** `typedLoadParent`, the `fastArray*`
  element assumptions, and `AddS`'s both-strings contract. Cheap once the
  primitive exists, but they validate the typed front-end rather than the
  JIT, and typed mode has its own checking story. Revisit after Class A/B/C
  have run clean against test262 and the benchmark suite.
- **The x86-64 side.** The design's register-convention requirement — a
  reserved caller-saved scratch (r11, and a second for the `IsNumber`
  bound, which genuinely cannot be done with one register) — belongs in the
  port plan's register-convention section, which must be written before the
  temp allocator's range is fixed.
- **Sharing Class C across backends.** It lives in
  `JitEmitter-regalloc.cpp`, the piece proposed for cross-arch extraction
  (cleanup item A3). If that extraction lands first, Class C is written
  once for both backends; this plan does not depend on it either way.
