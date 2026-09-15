# Split `JitEmitter.cpp` into logically separated translation units

Date: 2026-08-18
Status: Implemented

## Summary

Split `lib/VM/JIT/arm64/JitEmitter.cpp`, which is well over eight thousand
lines, into eleven translation units grouped by what they emit: register
allocation, object construction, the property protocol, arithmetic,
control flow, calls, environments, arrays, allocation and constants, with
the emitter machinery left behind in `JitEmitter.cpp`.

The ~40 free `emit_*` helpers currently in an anonymous namespace move
into a shared helper layer visible to all eleven TUs: the small hot ones
become `inline` in `JitEmitter-internal.h`, the bulky cold ones get
external linkage in `JitEmitter-internal.cpp`. Every helper stays reachable
from every emitter, as today.

The work lands as thirteen commits of code motion, each verified to emit
identical machine code, with measurement done at the end rather than as a
commit of its own.

## Motivation

`JitEmitter.cpp` holds two distinct things: a layer of ~40 free `emit_*`
helpers in an anonymous namespace, and the `Emitter::` definitions that
implement every JIT-emitted instruction, which are the bulk of the file.

The file is hard to navigate and slow to compile as a single translation
unit. The obstacle to splitting it is the helper layer: the helpers are
file-local today, so every method body can call any of them, and usage is
dense and scattered — from a couple of call sites for the rarest to a
couple of dozen for `emit_sh_ljs_get_pointer`. Splitting the methods
across translation units forces those helpers to become visible across TU
boundaries, and with that comes the risk of losing the inlining they get
today for free.

## Goals

- Group the emitters so that finding a given instruction is obvious from
  the file name.
- Keep every file comfortably under ~2,000 lines.
- Every helper remains callable from every emitter, as today.
- The emitted machine code is unchanged, to the precision the harness can
  establish (see Verification).
- No measurable regression in JIT compilation time or binary size.

### A note on sizes

This split is about logical grouping, not about hitting line targets.
Sizes in this document are deliberately qualitative: the bar is roughly
2,000 lines per file, and every grouping below clears it with room to
spare. Exact line and definition counts are not tracked here and should
not be added to this spec or to the implementation plan — they go stale
on every commit and distract from the only question that matters, which
is whether a file holds one coherent topic.

## Out of scope

- Splitting `JitEmitter.h`. It is a coherent set of declarations and is
  not the pain point. It does gain four definitions below the class —
  three member templates and the `comment` logger check — because those
  have to be visible to every TU; see the commit stack.
- Any behavioural change. Every commit is pure code motion.
- Preparing for an x86-64 port. There is no `lib/VM/JIT/x86*` yet; that
  work should drive its own restructuring when it arrives.

## Hard constraint: comments are preserved verbatim

Every comment, doc comment, `NOTE:`, and `FIXME` moves with its code
**unchanged**. This refactor does not reword, reflow, condense, or
"improve" any prose. The only permitted edits are:

- the file-header copyright block on each new file;
- a doc comment on a helper that moves from an anonymous namespace into a
  header, if and only if it previously said something that the move makes
  false;
- `\return` / parameter docs that name a type or linkage that the move
  changes.

Anything else is out of scope. A reviewer should be able to diff the
concatenation of the new files against the old one and see only motion.

## Final layout

| File | Contents |
| --- | --- |
| `JitEmitter.cpp` | prologue/epilogue and emitter machinery |
| `JitEmitter-regalloc.cpp` | register allocation and frame sync |
| `JitEmitter-alloc.cpp` | young-gen allocation, GC cell init |
| `JitEmitter-const.cpp` | constant materialization |
| `JitEmitter-object.cpp` | object construction and prototype |
| `JitEmitter-property.cpp` | property access protocol |
| `JitEmitter-arith.cpp` | conversions, arithmetic, comparison |
| `JitEmitter-control.cpp` | branches, switches, throw/catch |
| `JitEmitter-array.cpp` | arrays, iterators, arguments |
| `JitEmitter-call.cpp` | calls and `this` handling |
| `JitEmitter-env.cpp` | environments, closures, generators |

Plus the helper layer: `JitEmitter-internal.h` / `JitEmitter-internal.cpp`.

`JitEmitter-property.cpp` is the largest of these and
`JitEmitter-alloc.cpp` the smallest; both are fine.

### Naming

The `JitEmitter-<part>.cpp` form follows the convention already used for
split files elsewhere in the tree: `FlowChecker.cpp` alongside
`FlowChecker-expr.cpp` and `FlowChecker-scopetypes.cpp`, and
`ESTreeIRGen.cpp` alongside `ESTreeIRGen-expr.cpp`,
`ESTreeIRGen-stmt.cpp`, `ESTreeIRGen-func.cpp` and the rest. Suffixes are
lowercase, hyphenated where they need more than one word.

`JitEmitter-internal.h` follows the same convention, and there is a direct
precedent for a shared internal header in this exact shape:
`lib/VM/Interpreter.cpp` sits alongside `Interpreter-slowpaths.cpp` and
`Interpreter-internal.h`. The out-of-line helper bodies go in
`JitEmitter-internal.cpp`, so the whole helper layer is one base name and
one ordinary `.h`/`.cpp` pair. `Interpreter` has no `-internal.cpp`
because it has little out-of-line internal code; here there is enough that
it would sit oddly next to `frameSetup` and `emitROData` in the core file.

Whether a helper is hot or cold is then not a naming question at all: it
is just whether the header carries its body or only its declaration.

### Per-file contents

**`JitEmitter.cpp`** — `Emitter::Emitter`, `enter`, `commentV`,
`addToRuntime`, `assertPostInstructionInvariants`, `newBasicBlock`,
`getDebugFunctionName`, `frameSetup`, `leave`, `callThunk`,
`callThunkWithSavedIP`, `callWithoutThunk`, `emitIncrementCounter`,
`initHCLazyIDMayAlloc`, `loadFrameAddr`, `getBytecodeIP`, `unreachable`,
`profilePoint`, `directEval`, `mov`, `loadParam`, `getGlobalObject`,
`declareGlobalVar`, `debugger`, `createRegExp`, `newPrefLabel`,
`reserveData`, `uint64Const`, `registerThunk`, `emitCatchTable`,
`emitSlowPaths`, `emitThunks`, `emitROData`, and the `operator<<` at the
top of the file.

**`JitEmitter-regalloc.cpp`** — `_storeHWToFrame`,
`movHWFromFR`, `movHWFromMem`, `movFRFromHW`, `syncFrameOutParam`,
`freeReg`, `syncAndFreeTempReg`, `useReg`, `_spillTempForFR`,
`syncToFrame`, `syncAllFRTempExcept`, `freeAllFRTempExcept`, `freeFRTemp`,
`_assignAllocatedLocalHWReg`, `_isFRInRegister`, `getOrAllocFRInVecD`,
`getOrAllocFRInGpX`, `getOrAllocFRInAnyReg`, `frUpdatedWithHW`,
`frUpdateType`. Not `movHWFromHW` or `_allocTemp`: they are member
templates and their definitions live in `JitEmitter.h`.

**`JitEmitter-alloc.cpp`** — `bumpAllocAndUnpoison`, `initGCCell`,
`allocInYoung`, `alloc2InYoung`.

**`JitEmitter-const.cpp`** — `loadConstDouble`,
`loadSmallHermesValueInGpX`, `loadConstStringInGpX`, `loadConstBits64`,
`loadConstString`, `loadConstBigInt`. Not `loadBits64InGp`, which is a
member template defined in `JitEmitter.h`.

**`JitEmitter-object.cpp`** — `newObject`, `newObjectWithParent`,
`newObjectWithBuffer`, `newObjectWithBufferSlow`,
`newObjectWithBufferAndParent`, `newTypedObjectWithBuffer`,
`loadParentNoTraps`, `typedLoadParent`, `instanceOf`.

**`JitEmitter-property.cpp`** — the `GetByIdImpl` class, `getByIdImpl`,
`getByIdWithReceiver`, `getByVal`, `getByValWithReceiver`, `getByIndex`,
`putByIdImpl`, `putByValImpl`, `putByValWithReceiver`, `delByVal`,
`defineOwnById`, `defineOwnInDenseArray`, `defineOwnByIndex`,
`defineOwnByVal`, `defineOwnGetterSetterByVal`, `getOwnBySlotIdx`,
`putOwnBySlotIdx`, `addOwnPrivateBySym`, `getOwnPrivateBySym`,
`putOwnPrivateBySym`, `createPrivateName`, `isIn`, `privateIsIn`. Also the
`JITNumGetByIdSpec` statistic, whose only uses are here.

**`JitEmitter-arith.cpp`** — `toNumber`, `toNumeric`, `toInt32`,
`addEmptyString`, `arithUnop`, `booleanNot`, `bitNot`, `typeOf`, `addS`,
`mod`, `arithBinOp`, `bitBinOp`, `strictEqualImpl`, `compareImpl`.

**`JitEmitter-control.cpp`** — `jmpTypeOfIs`, `typeOfIs`, `uintSwitchImm`,
`stringSwitchImm`, `jmpTrueFalse`, `jmp`, `jmpUndefined`, `jmpBuiltinIs`,
`jCond`, `jStrictEqual`, `throwInst`, `throwIfEmptyUndefinedImpl`,
`throwIfThisInitialized`, `catchInst`, `ret`.

**`JitEmitter-array.cpp`** — `newArray`, `newArrayWithBuffer`, `newFastArray`,
`fastArrayLength`, `fastArrayLoad`, `fastArrayStore`, `fastArrayPush`,
`fastArrayAppend`, `iteratorBegin`, `iteratorNext`, `iteratorClose`,
`getPNameList`, `getNextPName`, `toPropertyKey`, `getArgumentsLength`,
`getArgumentsPropByValImpl`, `reifyArgumentsImpl`.

**`JitEmitter-call.cpp`** — `callImpl`, `call`, `callN`, `callBuiltin`,
`callWithNewTarget`, `callWithNewTargetLong`, `callRequire`,
`getBuiltinClosure`, `createThis`, `selectObject`, `loadThisNS`,
`coerceThisNS`, `getNewTarget`.

**`JitEmitter-env.cpp`** — `createTopLevelEnvironment`,
`createFunctionEnvironment`, `createEnvironment`, `getParentEnvironment`,
`getEnvironment`, `getClosureEnvironment`, `loadFromEnvironment`,
`storeToEnvironment`, `createClosure`, `createBaseClass`,
`createDerivedClass`, `createGenerator`.

### Two judgement calls

`isIn` goes to property rather than object: `isIn` and `privateIsIn` are
the same operator over public and private keys, and `privateIsIn` belongs
with the other private-name operations. `instanceOf` goes to object,
because it walks the prototype chain like `loadParentNoTraps` and
`typedLoadParent`.

## Helper layer

### Placement rule

Helpers are placed by kind, not by call site. Every free `emit_*` function
and every helper class lives in the helper layer and is visible to all
eleven TUs, regardless of how many callers it has today. The property that
any emitter can reach for any helper is preserved; only the linkage
changes.

Filing a helper into whichever TU happens to use it today is what erodes a
shared helper layer, and it removes the option of a future emitter picking
it up. Do not do it.

The one thing that stays TU-local is `Emitter::GetByIdImpl`, and it is not
a helper: it is a nested class of `Emitter` that *is* the GetById
implementation, already forward-declared in `JitEmitter.h` and marked
`HERMES_ATTRIBUTE_INTERNAL_LINKAGE`. It has no second consumer and no
reuse story. If it ever grows one it moves into the helper layer under the
rule above.

### `JitEmitter-internal.h`, everything `inline`

The small, hot helpers:

- HermesValue tag and encoding: `emit_sh_ljs_get_tag`,
  `emit_sh_ljs_tag_is_pointer`, `emit_sh_ljs_tag_is_object`,
  `emit_sh_ljs_tag_is_string`, `emit_sh_ljs_get_pointer`,
  `emit_sh_ljs_object`, `emit_sh_ljs_object2`, `emit_sh_ljs_is_double`,
  `emit_sh_ljs_string`, `emit_shv_string`, `emit_sh_ljs_is_object`,
  `emit_sh_ljs_is_string`, `emit_sh_ljs_is_bigint`, `emit_sh_ljs_is_empty`,
  `emit_sh_ljs_is_null`, `emit_sh_ljs_is_bool`, `emit_sh_ljs_is_undefined`,
  `emit_sh_ljs_is_symbol`, `emit_sh_ljs_bool`, `emit_sh_ljs_bool_const`
- compressed pointers and memory: `emit_sh_cp_decode`,
  `emit_sh_cp_decode_non_null`, `emit_sh_cp_encode_non_null`,
  `emit_load_cp`, `emit_store_cp`, `emit_load_shv`, `emit_store_shv`,
  `emit_load_from_base_offset`, `emit_load_slot16`, `maxNaturalBaseOffset`
- numeric and cell predicates: `emit_double_is_int`,
  `emit_double_is_uint32`, `emit_gccell_get_kind`, `emit_cellkind_in_range`
- immediates: `emit_cmp_imm32`, `emit_add_imm_u24`, `isCheapConst`,
  `isStpGpXImm`
- `class Emit_sh_shv_decode`: members, the inline constructor, and the
  inline `emitAll`
- the free `emit_sh_shv_decode` wrapper
- the macros `EXPECT_ERROR`, `EMIT_RUNTIME_CALL`,
  `EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP`,
  `EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP`
- the `-Wmissing-designated-field-initializers` pragma

That last item is easy to miss. It currently sits at the top of
`JitEmitter.cpp` and is required by every TU that constructs a `SlowPath`
with designated initializers, which is most of them. Putting it in the
shared header keeps it in one place instead of copied into every TU.

### `JitEmitter-internal.h` / `JitEmitter-internal.cpp`

The bulky, rarely called helpers get external linkage:
`emit_jsobject_init`, `emit_environment_init`,
`emit_stringprim_get_length_and_flags`, `emit_load_builtin_closure`,
`Emit_sh_shv_decode::emitFirstCase`, `Emit_sh_shv_decode::emitRestCases`,
and the `OurErrorHandler` and `OurLogger` classes.

`Emit_sh_shv_decode` is already shaped for this split: the constructor and
`emitAll` are small and defined in-class today, while `emitFirstCase` and
`emitRestCases` are already defined out of line.

### Linkage change

The anonymous namespace disappears. Helpers move into
`hermes::vm::arm64` and rely on `inline` (header) or external linkage
(support). Names are unchanged.

## Build system

`BUCK` globs `lib/VM/**/*.cpp`, so **new TUs need no BUCK change**. The
glob is unconditional, which means every new `.cpp` must open with

```cpp
#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
```

and close with `#endif // HERMESVM_JIT`, exactly as `JitEmitter.cpp` does
today. Without the guard, non-JIT builds break.

`lib/VM/CMakeLists.txt` lists JIT sources explicitly and must be updated
with each new file.

These two facts pull in opposite directions and are the main way a commit
in this stack can go wrong. BUCK picks up a new `.cpp` on its own, so a
commit that forgets the CMake entry still builds and tests green under
`buck2` while being broken for everyone on CMake. The CMake build
therefore has to be checked per commit, not once at the end.

Where the CMake build directory lives depends on the checkout, and this
spec does not assume either case. On an EdenFS checkout, such as one under
`~/fbsource`, in-tree CMake build directories do not work, so the build
directory has to sit outside the repository. On an ordinary filesystem
checkout the usual in-tree `cmake -B cmake-build-...` is fine and is what
`CLAUDE.md` describes.

Check which one applies before configuring anything, and if it is not
obvious, ask rather than guessing. The two cases need different commands,
and a wrong guess costs a full configure and a confusing failure.

## Commit stack

Thirteen commits of code motion. Each keeps the build and the lit suite
green. Measurement happens after the last one and produces no commit,
because the two size fixes it turned up were folded back into the
commits that caused them rather than appended as cleanup.

| # | Commit | Effect |
| ---: | --- | --- |
| 1 | Extract hot helpers, macros and pragma to `JitEmitter-internal.h` | still one emitter TU |
| 2 | Extract cold helpers to `JitEmitter-internal.cpp` | still one emitter TU |
| 3 | Make the shared `Emitter` definitions visible in `JitEmitter.h` | still one emitter TU |
| 4 | Move register allocation to `JitEmitter-regalloc.cpp` | first TU split |
| 5 | Move allocation primitives to `JitEmitter-alloc.cpp` | |
| 6 | Move constant materialization to `JitEmitter-const.cpp` | |
| 7 | Move object construction to `JitEmitter-object.cpp` | |
| 8 | Move the property protocol to `JitEmitter-property.cpp` | |
| 9 | Move calls to `JitEmitter-call.cpp` | |
| 10 | Move environments and closures to `JitEmitter-env.cpp` | |
| 11 | Move arrays, iterators and arguments to `JitEmitter-array.cpp` | |
| 12 | Move arithmetic and comparison to `JitEmitter-arith.cpp` | |
| 13 | Move control flow to `JitEmitter-control.cpp` | |

Commits 1 and 2 are the load-bearing ones and land while there is still a
single emitter TU. If moving helpers into a header costs anything, it
shows up at commit 1, where the only variable is the header extraction and
nothing has been split yet. That is the cheapest possible place to find
out.

The hot helpers go first and the cold ones second, not the other way
round. The dependency runs one way: the cold helpers call the hot ones —
`emit_stringprim_get_length_and_flags` calls `emit_sh_ljs_get_pointer`,
`emit_jsobject_init` calls `emit_sh_cp_encode_non_null` and
`emit_store_cp` — and nothing calls back. Moving the cold helpers into
their own translation unit first would leave them calling helpers still
private to `JitEmitter.cpp`, and that unit would not compile.

Commit 3 exists because `JitEmitter.h` declares three member templates —
`movHWFromHW`, `_allocTemp` and `loadBits64InGp` — whose definitions live
in `JitEmitter.cpp`. A template is instantiated only where its definition
is visible. That is invisible while there is one translation unit and
becomes undefined references the moment callers live elsewhere. The
failure is not even uniform: `_allocTemp` is reached only through the
inline `allocTempGpX()`/`allocTempVecD()` wrappers in the header and is
instantiated nowhere in the register-allocation group, so it would be
undefined everywhere, while `loadBits64InGp<a64::GpX>` would happen to be
instantiated next to its own definition and link by accident, leaving only
`loadBits64InGp<a64::GpW>` broken. Moving the definitions into the header,
directly below the class that declares them, is the ordinary fix and needs
no instantiation list to maintain. It must land before commit 4.

Commit 3 also moves the logger check in `Emitter::comment` into the
header, for an unrelated reason discovered by measurement rather than
foreseen here: leaving the body in `JitEmitter.cpp` stopped release
builds from dead-stripping the debug-comment calls once their callers
moved to other TUs. See "Measured result".

Commits 4 through 13 are mechanical and independent of each other; their
order is not significant beyond doing the small ones first to shake out
the pattern.

## Verification

### Per commit

1. `buck2 build //xplat/static_h:hermes`
2. A CMake build of the `hermes` target, to catch a missing
   `lib/VM/CMakeLists.txt` entry that `buck2` cannot see. Which build
   directory to use depends on the checkout — see "Build system" above.
   On the EdenFS checkout this design was written against it was
   `cmake --build ~/work/hws/fbsource-sh-debug --target hermes`, an
   out-of-tree directory configured against `xplat/static_h` with
   `CMAKE_BUILD_TYPE=Debug`, Ninja and `HERMESVM_ALLOW_JIT=2` so that the
   JIT is compiled; verified working on 2026-08-18. On a normal
   filesystem checkout an in-tree build directory does the same job.
3. `buck2 test //xplat/static_h:lit` — on macOS arm64 this runs the suite
   twice, plain and with `-Xjit=force`, because `jit_enabled` resolves to
   `2` in `BUCK`.
4. `-Xdump-jitcode=3` output identical to the pre-split baseline over a
   fixed corpus, after canonicalization (see below).

**The emitted code is not reproducible run to run, so "byte-identical"
needs qualifying.** `isCheapConst` in `JitEmitter.cpp` decides between
materializing a 64-bit constant inline with `mov`/`movk` and spilling it
to RO data by counting its non-zero 16-bit halfwords. JIT code bakes in
runtime pointers, so ASLR changes those values every run, the emitter
genuinely selects different instructions, and every subsequent RO_DATA
offset shifts with it. Two captures from an *unchanged* binary differed
in 1,902 lines out of 105,319.

Running under `lldb`, which disables ASLR, cuts that to 58 lines but
does not close it — the contiguous heap's base comes from a separate
randomized mmap and there is no flag to pin it. It also costs about four
times the wall clock and needs a macOS permission grant that hangs
unattended runs, so it was rejected.

What the harness therefore compares is the dump with constant
materialization canonicalized: `mov xN, 0x…` and `ldr xN, [RO_DATA…]`
collapse to a `CONST xN` token and the RO_DATA table is dropped. That
leaves **102,221 of 105,319 lines, about 91%, compared byte for byte** —
every instruction, register, branch, label, ordering and comment. What
is masked is whether a constant went inline or to RO data, and its
value. Deterministic across three consecutive captures.

Commits 1 through 13 are code motion, so for those *any* difference in
the canonicalized dump is a bug, not a judgement call.

### Measured result

Measured on `libhermesvm.dylib` built MinSizeRel, `HEAP_HV_PREFER32`,
`HERMES_IS_MOBILE_BUILD`, `HERMESVM_ALLOW_JIT=2` and
`HERMESVM_ALLOW_CONTIGUOUS_HEAP=ON`. Both of the last two matter:
without `ALLOW_JIT` none of this code is compiled into the library at
all and the delta is a meaningless zero, and HV32 without a contiguous
heap makes the JIT refuse to build.

| | shipping sections | vs pre-split |
| --- | ---: | ---: |
| pre-split | 2,470,898 | — |
| split, as first written | 2,483,737 | +12,839 (+0.520%) |
| after both fixes below | 2,474,572 | **+3,674 (+0.149%)** |

Two thirds of the initial regression came from something the design did
not anticipate, and it was not the helpers.

**`Emitter::comment` was the dominant cost, and it never moved.** It
opens with `if (!hasLogger()) return;`, and `hasLogger()` folds to a
constant false when `ASMJIT_NO_LOGGING` is defined, as it is in release
builds. While every caller shared a translation unit with the body, the
compiler deleted the call and dead-stripped the format string. Once the
callers moved away the body was no longer visible, so each had to
materialize and pass its string: 4.1KB of `__cstring` plus 4.4KB of
`__text` in call setup. Fixed by putting the logger check inline in
`JitEmitter.h` while leaving `vsnprintf` out of line in `commentV`, so
enabling logging does not duplicate the formatting per TU.

**`emit_load_from_base_offset` was the only helper that measurably lost
inlining**, about 760 bytes. Fixed with `JIT_EMIT_ALWAYS_INLINE`, which
applies `LLVM_ATTRIBUTE_ALWAYS_INLINE` only under `NDEBUG`.

Both fixes are folded into the commits that introduced the respective
regressions, so no commit in the stack carries one.

The residual +3,674 is +2,544 in `__text`, spread thin with no dominant
symbol, and +1,104 in `__const`. The `__const` share is asmjit
declaring its register constants as `static constexpr GpX x0 = GpX(0);`
at namespace scope in a header: `static` forces internal linkage, so
each of the nine TUs that use them gets a private 16-byte copy that the
linker may not merge. Nine registers times eight extra copies times 16
bytes is 1,152, against a measured 1,104. Fixing it means changing
vendored third-party code to `inline constexpr`.

**ThinLTO does not help; it makes both numbers worse.** Same
configuration plus `-flto=thin` on compile and link: the split costs
+7,488 bytes rather than +3,674, and the library grows about 39KB
(+1.58%) whether or not it is split. With twelve modules instead of one,
ThinLTO imports and inlines across more boundaries. This is a size
measurement only and says nothing about throughput, which is the usual
reason to enable it.

**On noise floors.** During this work a 64-byte "code size regression"
was chased down and turned out to be run-to-run variance — the unchanged
baseline produced the larger figure once in ten runs. The MinSizeRel
numbers above are from deterministic builds, but any timing comparison
must establish its spread on a single unchanged binary first.

## Risks

**Helpers reaching into `Emitter`.** RESOLVED before implementation: no
free helper touches `Emitter` state. They take `a64::Assembler &` and
register operands; `OurErrorHandler` holds an `asmjit::Error &` and a
`std::function`, `OurLogger` an `a64::Assembler &` and a `PerfJitDump *`.
None needed friendship.

**Rebase cost.** Thirteen commits over one large file conflict with any
concurrent JIT work. The stack should land promptly or be rebased often.
An auto-restack of C++ can merge cleanly into code that does not compile,
so build after every restack rather than trusting a conflict-free merge.

**A category may read badly as a unit.** The groupings above are drawn
from how the emitters relate, not from measurement, so one of them could
turn out to be a bag of unrelated things once it is a file of its own.
`JitEmitter-property.cpp` is the most likely candidate for a further split,
along `GetById` / `PutById` / `defineOwn` lines, if it reads that way.

**Two small TUs.** `JitEmitter-const.cpp` and `JitEmitter-alloc.cpp` are
small for standalone files. Both are single coherent concerns, which is
why they are split, but they are the first candidates to fold back into
`JitEmitter.cpp` if the file count becomes annoying.

## What the implementation actually taught

Every problem hit during this split came from the same root: something
that worked only because there was one translation unit. The design
foresaw two instances (templates defined in the `.cpp`, non-function
entities in the anonymous namespace). Three more surfaced only when code
started moving:

- **Ambient includes.** Each new TU exposed a different type that the
  original file received through somebody else's header chain, not its
  own include: `JSObject::maxYoungGenAllocationPropCount` from
  `JSObject-inline.h`, `VTable`/`CellKind` via `Callable.h` →
  `JSObject.h`, `SHRuntimeModule` from `JitHandlers.h`. The rule that
  emerged: include the header that *defines* what you use.
- **Interprocedural dead-code elimination.** `Emitter::comment` linked
  fine after the split but silently stopped being optimized away. A green
  build proves nothing about this class of regression.
- **Removing an orphaned include can strand a type.** Deleting
  `Callable.h` was correct, but it left `CellKind` — used four times in
  the same file — arriving only transitively. Grep for the symbols a
  header supplies, not for the header's name.

The general lesson: `verify.sh` catches behavioural change, and the
compiler catches missing declarations, but neither catches "this only
linked/optimized by accident". Those need reasoning about the code.

Known follow-ups, none blocking: `hermes/BCGen/SerializedLiteralParser.h`,
`hermes/VM/IdentifierTable.h`, `hermes/VM/Interpreter.h` and
`hermes/VM/StaticHUtils.h` are now unused in `JitEmitter.cpp` and can be
removed; `JitEmitter-object.cpp` uses `CellKind` without including
`CellKind.h`.
