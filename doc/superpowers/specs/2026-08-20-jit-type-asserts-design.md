# Design: runtime verification of FR type assumptions (`-Xjit-emit-type-asserts`)

## Motivation

The JIT elides tag checks and decides GC register residency based on FR type
facts (`FRState::globalType` / `localType`) that come from three unverified
sources: the bytecode header's number/non-pointer register counts (i.e. the
whole front-end pipeline), each emitter's hand-written result-type claims
(~150 `frUpdatedWithHW` sites), and the `*N` instruction contract
(`forceNumber`). A wrong fact produces silently wrong JS values (a mistyped
object NaN-boxes as NaN and flows through `fadd`) or delayed heap corruption
(a pointer in a NonPtr-class callee-saved register is invisible to the GC).
Nothing checks any of this at run time; `assertPostInstructionInvariants`
audits only compile-time bookkeeping.

This design adds emitted-code verification of those assumptions under the
new `emitTypeAsserts_` flag (`-Xjit-emit-type-asserts`, see Configuration), so a violation traps at
the first instruction that relies on it, with enough metadata to name the
site. Primary consumer: validating the arm64 backend before the x86-64 port,
then keeping x86-64 bring-up honest while every emitter is rewritten.

## Goals / non-goals

Goals:

- Trap at the **first consumption** of a value that violates the type fact
  the fast path relied on, and at the **end of the instruction** that
  leaves a global-register-class fact violated (see Class C).
- **Pure insertion**: the build with the flag on must make byte-identical
  allocation and code-shape decisions to the run with it off, plus
  inserted check sequences. No interaction with `TempRegAlloc`, no new
  spills, no changed LRU state.
- Actionable diagnostics: failure names the function, bytecode offset, FR,
  and violated predicate.

Non-goals:

- Not a general verifier. A wrong claim that is never consumed (and not
  covered by the write-side global-class checks) is not caught.
- Class C validates the front-end's number/non-pointer classification only
  for the FRs that actually received a global register (~16 per function):
  `enter()` records `globalType` only for those, so class claims for the
  overflow FRs are neither used by the JIT nor checkable here. (They are
  also harmless while unused — but a future backend that consumes them
  must extend the recording.)
- The tag-test helpers themselves (`emit_sh_ljs_is_*`) are trusted; they are
  already covered by static_asserts against the HV encoding.
- No attempt to keep flags/registers alive across a *failed* check — the
  failure path is noreturn.

## Design principles

1. **Assert the predicate the fast path relies on, not the declared FRType.**
   Consumption sites know exactly what they exploit: `arithBinOp` relies on
   "is a double", the strict-equality raw-bit tier relies on "not a pointer
   and not a double", global-reg residency relies on "not a pointer". The
   API therefore takes a predicate, not an `FRType`:

   ```cpp
   enum class TypePred : uint8_t {
     IsNumber,     // unsigned-below (HVTag_First << kHV_NumDataBits)
     IsBool,       // ETag == HVETag_Bool
     NotPointer,   // tag unsigned-below the pointer range (GC-safety)
     BitComparable, // NotPointer && !IsNumber: bits are the === identity
     IsObject,     // tag == HVTag_Object (typed-invariant sites, phase 2)
   };
   ```

2. **Reserved scratch only.** All check sequences use x16/x17 (already
   outside `TempRegAlloc`'s x0–x15 domain, already used as raw scratch by
   `callRuntime`/`callRuntimeWithSavedIP` and by the `_loadFrame`/
   `_storeFrame` encoding fallbacks). Never `allocTempGpX`.

3. **Flags-dead placement — a per-site obligation, not a given.** Checks
   clobber NZCV, so each insertion point must be verified flags-dead by the
   author; it is *not* a property emitters have in general. Counterexample:
   `selectObject` sets flags via `emit_sh_ljs_is_object`, then calls
   `getOrAllocFRInGpX`, then consumes the flags in `csel` — flags live
   across an allocation call. None of the Class A/B sites below have that
   shape (their operand materialization precedes their own compare), and
   the Class C hook runs at bytecode-instruction boundaries where NZCV is
   dead by construction, but any future site (Class D especially) must be
   checked, and `emitTypeAssert`'s doc comment must state the obligation.
   Checks are never inserted inside a window where x16 is live; all such
   windows (call-target/IP materialization, the frame-access encoding
   fallbacks) are contained within single helper emissions, so no
   insertion point in this design ever lands inside one.
   `readFRForAssert`'s own frame load may itself take the encoding
   fallback; that is fine — the fallback's x16 use is sequenced before x16
   receives the loaded value.

## The check primitive

```cpp
/// Emit (only when emitTypeAsserts_) a trap-on-violation check that the value
/// of \p fr, currently available in \p hwVal, satisfies \p pred.
/// Uses only x16/x17. Clobbers NZCV. Must be called when flags are dead.
void Emitter::emitTypeAssert(FR fr, HWReg hwVal, TypePred pred);
```

Sequences (input first normalized to a GpX: if `hwVal` is a VecD, emit
`fmov x16, dN` and check x16; helpers tolerate temp == input):

- `IsNumber`: `mov x17, #(HVTag_First << kHV_NumDataBits)` ; `cmp xIn, x17` ;
  `b.hs fail`. (Reuses the `emit_sh_ljs_is_double` shape; the only predicate
  needing both scratch registers when the input is already a GpX.)
- `IsBool`: `asr x16, xIn, #(kHV_NumDataBits - 1)` ; `cmn x16, #-HVETag_Bool`
  ; `b.ne fail` (existing `emit_sh_ljs_is_bool`).
- `NotPointer`: `asr x16, xIn, #kHV_NumDataBits` ; `cmn x16,
  #-HVTag_FirstPointer` ; `b.hs fail` (inverted `emit_sh_ljs_tag_is_pointer`).
- `BitComparable`: `IsNumber`-style bound check with inverted branch
  (`b.lo fail`) followed by `NotPointer`.
- `IsObject`: existing `emit_sh_ljs_is_object` with x16 as temp; `b.ne fail`.

For sites where no operand is materialized at all (see `jmpUndefined`
below), a small read helper loads the FR's current value into x16 directly
from its best location per `FRState` (local reg → global reg → frame slot),
again without touching the allocator:

```cpp
/// Read the current value of \p fr into x16 for assertion purposes,
/// without allocating or perturbing any state.
void Emitter::readFRForAssert(FR fr);
```

`readFRForAssert` must honor the FRState up-to-date invariants, not just
the location priority: read the local reg if one exists (locals always
hold the latest value), else the global reg **only if
`globalRegUpToDate`**, else the frame slot (asserting `frameUpToDate`),
and assert `!regIsDirty` on entry. Reading a stale location would make
the assert itself report phantom violations — the worst failure mode a
checking feature can have.

## Insertion sites

### Class A — `forceNumber` contract sites (highest value)

The `*N` bytecodes promise number operands; the emitters skip both checks
and slow paths. Assert `IsNumber` on every operand. This validates the
front-end's type inference, not merely the JIT's bookkeeping.

| Emitter | Condition | Operands asserted |
|---|---|---|
| `arithBinOp` | `forceNumber` (`AddN`, `SubN`, `MulN`, `DivN`) | left, right |
| `arithUnop` | `forceNumber`, **unreachable today** | input |
| `jCond` | `forceNumber` (`JLessN`, `JNotLessN`, `JLessEqualN`, `JNotLessEqualN`) | left, right |
| `mod` | `forceNumber`, **unreachable today** | left, right |
| `callWithNewTargetLong` | argc `fcvtzu` assumes number | frArgc |

The `*N` family is exactly those eight opcodes: there is no `NegateN` or
`ModN`, every `DECL_UNOP` hardcodes `forceNum = false`, and `mod`'s only
caller passes `false`. The `arithUnop` and `mod` rows are therefore
contracts the current bytecode never exercises; they cost nothing to
assert and cover a future `*N` opcode for free.

### Class B — `isFRKnown*` elisions

Wherever a fast path skips a check because `isFRKnownNumber/Bool/
OtherNonPtr` returned true, emit the skipped predicate anyway:

| Emitter | Elision | Predicate |
|---|---|---|
| `arithBinOp` / `arithUnop` | operand known Number → no NaN-box check, possibly no slow path | `IsNumber` per known operand |
| `compareImpl` / `jCond` | both known Number → no `b.vs` slow path | `IsNumber` per known operand |
| `mod` | both known Number → no `fcmp`/`b.vs` slow path | `IsNumber` per known operand |
| `jmpTrueFalse` | known Number → fcmp-only path; known Bool → tst-only path | `IsNumber` / `IsBool` |
| `jmpUndefined` | known Number/Bool → **emits nothing** | `IsNumber` or `IsBool`, whichever was relied on, via `readFRForAssert` (no operand is otherwise loaded) |
| `strictEqualImpl` | side known Bool/OtherNonPtr → raw-bit tier; side known Number → fcmp-only tier | `BitComparable` / `IsNumber` on the known side |
| `jStrictEqual` | separate emitter, same two tiers as `strictEqualImpl` | `BitComparable` / `IsNumber` on the known side |
| `toNumber` / `toNumeric` | known Number → early `return mov(...)`, no operand materialized | `IsNumber` via `readFRForAssert` |

Where an emitter computes one known-type boolean per operand
(`arithBinOp`, `mod`, `compareImpl`, `jCond`, `strictEqualImpl`,
`jStrictEqual`), guard each operand's check on that operand's own
boolean, never on the emitter's combined fast-path condition (`!slow`).
Per-operand asserts every fact the JIT holds, at its first appearance,
rather than only the facts that happen to be load-bearing for the shape
the emitter chose; with `leftIsNum && !rightIsNum`, `!slow` would assert
nothing at all.

`jmpUndefined` must **not** use `BitComparable` (`NotPointer && !IsNumber`):
it would trap on every legitimate number reaching the elided path, and it
accepts `undefined`. Assert the fact the emitter actually tested.

`toInt32` is deliberately absent: it has no `isFRKnownNumber` elision, and
always emits its own check and slow path.

### Class C — producer-side checks for global-class FRs (GC direction)

Consumption checks cannot cover the GC, whose "consumption" is implicit in
register residency. Instead, verify on the **producer side**: whenever a
value is stored into an FR whose `globalType` is `Number` or
`UnknownNonPtr` (equivalently: any FR that is *eligible* for a
callee-saved global register), assert the value matches the class:

- `globalType == Number` → `IsNumber`
- `globalType == UnknownNonPtr` → `NotPointer`

"Producer side", not "write time": the check is emitted at the *next
bytecode instruction boundary*, for the reasons argued below, so what it
validates is the value the FR holds once the writing instruction has
finished. An instruction that stores a non-conforming value, makes a
runtime call (a GC safepoint), and then overwrites it with a conforming
one before it ends passes the check. That gap is not a bug in the
recording; it follows from the boundary placement, which is the only
sound hook available.

Insertion point: a **post-instruction hook**, not a tail hook in
`frUpdatedWithHW`. A tail hook there is unsound: many emitters call
`frUpdatedWithHW` *before* emitting the write into the register (the
declare-then-write pattern that `FRState::regIsDirty` exists to model —
e.g. `catchInst` declares the result FR updated and only then emits the
`ldr` from `thrownValue_`; `callImpl`'s SavedCodeBlock reg, `bitNot`,
`booleanNot` and `loadThisNS` have the same shape), so the hook would
check a register that does not yet hold the value — and since
`frUpdatedWithHW` clears `regIsDirty`, the hook cannot even detect that it
is in that window. A third write path, `syncFrameOutParam` (the runtime
wrote the frame slot directly; seven callers), reaches neither
`frUpdatedWithHW` nor `movFRFromHW` at all.

Instead: `frUpdatedWithHW`/`movFRFromHW`/`syncFrameOutParam` merely
*record* the FR in a small per-instruction written-set (compile-time
bookkeeping, done only when the flag is on), and a hook invoked at each
bytecode instruction boundary — alongside the existing compile-time
`assertPostInstructionInvariants` call in `compileBB` — emits the checks
for the recorded FRs whose class requires one, then clears the set. At an
instruction boundary the value is definitionally materialized (that is
the post-instruction invariant), NZCV is dead by construction, x16/x17
are free, and all three write paths are covered by a single call site.

Guard the check on the *class claim* (`globalType == Number` /
`UnknownNonPtr`), and separately assert the expected equivalence
`(globalType != UnknownPtr) == globalReg.isValid()` — today the two
coincide only because `enter()`'s allocation loops set both together and
break together; that is a load-bearing coincidence worth pinning with an
assert rather than silently relying on.

### Class D (phase 2, optional) — typed-mode invariants

`typedLoadParent`, `fastArray*` element assumptions, and
`AddS`'s both-strings contract rely on typed-bytecode guarantees. Assert
`IsObject` / string tag on the inputs. Lower priority: these validate the
typed front-end rather than the JIT, and typed mode has its own checking
story, but the hooks are cheap once the primitive exists.

## Failure path and diagnostics

Each `emitTypeAssert` call registers a compile-time record and embeds its
index:

```cpp
struct TypeAssertSite {   // one entry per emitted check
  CodeBlock *codeBlock;
  uint32_t bytecodeOfs;   // codeBlock_->getOffsetOf(emittingIP)
  uint16_t frIndex;
  TypePred pred;
};
```

Emitted failure sequence per site (placed with the slow paths, out of
line — the inline cost is one conditional branch per predicate leg):

```
fail_N:  mov  w0, #N
         b    typeAssertFailLab
```

There is no per-function metadata mechanism to hang the table on —
`addToRuntime` returns a bare `JITCompiledFunctionPtr` into
`setJITCompiled` — and inventing a registry for a fatal-only debugging
feature is overkill. Instead: **leak** the per-function
`std::vector<TypeAssertSite>` *object* (the handler never returns, and
JIT code is never freed anyway), store its address as one RO-data 64-bit
constant, and have the shared per-function tail materialize it:

```
typeAssertFailLab:
         ldr  x1, [RO_DATA + siteTableOfs]   // w0 = site idx, set per stub
         bl   _jit_type_assert_failed        // noreturn
```

Leak the vector object, not its buffer: the buffer's address is not final
until the last site is registered, which is long after the RO-data
constant has to be emitted, whereas the object's address is stable from
first use and needs no back-patching.

`_jit_type_assert_failed(uint32_t siteIdx, const
std::vector<TypeAssertSite> *sites)` formats "JIT type assert failed:
function F, bytecode offset O, rN, expected <pred>", and `hermes_fatal`s.
It takes no `SHRuntime *`: the site record alone names the function, via
`CodeBlock::getFunctionID` and `getNameString`. Being noreturn, it needs
no register/frame preservation, and the call may safely use the normal
`callRuntime` shape; no change to `addToRuntime`.
(A bare `brk #N` is an acceptable fallback first implementation — the
site index in the brk immediate plus the dump-jitcode listing already
localizes the failure — but the C++ handler costs little and removes the
need for a debugger to triage.)

## Configuration and rollout

- **A separate flag, default off** (`-Xjit-emit-type-asserts`, its own
  `emitTypeAsserts_` bool), *not* a rider on `emitAsserts_`:
  `-Xjit-emit-asserts` defaults to **true in debug builds**
  (`RuntimeFlags.h`), so piggybacking would emit the checks in every debug
  lit run, in a feature whose whole premise is that it perturbs nothing
  until asked. A separate opt-in bit also lets a bring-up failure be
  bisected by check class. (Consider subdividing further — A/B vs. C — if
  that bisection proves useful in practice.)
- Rollout order: land the primitive + Class A/B/C on arm64; run `test/jit`
  (with `HERMESVM_ALLOW_JIT=2`), test262, and the benchmark suite under
  `-Xjit-emit-type-asserts` to certify the *existing* type plumbing; then treat
  a clean assert run as a gating check during x86-64 bring-up.

## Verifying the checks can fail

A green run proves only that the suite ran. Before trusting the feature,
mutate and observe the trap (throwaway build):

1. Flip one emitter's claim — e.g. make `loadConstString` declare
   `FRType::Number` — and confirm a Class C trap naming that FR.
2. Weaken one elision — e.g. force `isFRKnownNumber` to return true for one
   FR in `arithBinOp` — and confirm a Class B trap at the consumption site.
3. Run one `*N` test with the mutation in (2) applied to the `jLessN` path
   to confirm Class A coverage.

These three mutations cover the three insertion classes; record the
expected trap message for each in the commit message of the feature.

## Cost

- Production builds: zero (everything is behind `emitTypeAsserts_`, which is a
  compile-of-the-JS-function-time flag, not a build-time flag; the checks
  simply are not emitted).
- With the flag on (in any build): 2-5 instructions per elided check plus
  2 per out-of-line fail stub; site-table memory proportional to check
  count. JIT compile time impact negligible.
- One new invariant on emitter authors: `emitTypeAssert` must be placed
  while flags are dead. This is enforced socially (it is called from ~10
  central places, not per-emitter) and documented at the primitive.

## x86-64 notes

- The design requires one (occasionally two) non-allocated scratch GPRs.
  x86-64 has no ABI-reserved analogue of x16/x17, so the port's register
  convention must reserve a caller-saved scratch (r11, optionally r10)
  outside the temp allocator — for this, and for the call-target/IP
  materialization duties x16/x17 already serve on arm64. Decide this up
  front; retrofitting a reserved scratch after the allocator hands out all
  16 GPRs is painful.
- Predicate sequences translate directly: `sar $47/$48` + `cmp` for tag
  tests, `mov r11, imm64` + `cmp` for the double bound; `ucomisd`-based
  shapes are not needed (all checks are integer tag tests on the raw bits,
  via `movq r11←xmm` for vector-resident values).
- Class C's write recording (`frUpdatedWithHW`, `movFRFromHW`,
  `syncFrameOutParam`) lives in `JitEmitter-regalloc.cpp` — the
  register-file engine the file split isolated, and the piece proposed for
  cross-arch sharing (cleanup item A3). If that extraction lands first,
  the recording half of Class C is written once for both backends. The
  emission half is the instruction-boundary hook, which each backend calls
  from its own `compileBB` dispatch loop.
