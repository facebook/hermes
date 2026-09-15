# JitEmitter.cpp Split Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Split `lib/VM/JIT/arm64/JitEmitter.cpp` into eleven translation
units grouped by what they emit, moving the anonymous-namespace helpers
into a shared helper layer that every emitter can still reach.

**Architecture:** Every task is code motion. Definitions are cut from
`JitEmitter.cpp` and pasted into a new `.cpp` with a copyright header, the
`HERMESVM_JIT` guard, and the includes it needs; nothing is reworded,
renamed, or rewritten. The ~40 free `emit_*` helpers lose their anonymous
namespace and move to `JitEmitter-internal.h` (small hot ones, `inline`)
and `JitEmitter-internal.cpp` (bulky cold ones, external linkage). The
three `Emitter` member templates move to `JitEmitter.h` first, because a
template definition left in a `.cpp` stops being instantiable once its
callers live in other translation units.

**Tech Stack:** C++17 (no exceptions, no RTTI), asmjit, buck2 (primary),
CMake + Ninja (must stay green), LLVM lit + FileCheck, Sapling.

## As executed

This plan was carried out in full. Five things diverged from it; the spec
carries the detail, this is the index.

1. **The verification criterion changed before Task 2.** Byte-identical
   emitted code turned out to be unachievable: `isCheapConst` picks
   instruction forms from pointer values, so ASLR makes the dump differ
   run to run on an unchanged binary. `capture.sh` below shows the
   original narrow address normalization; what was actually used also
   canonicalizes constant materialization to a `CONST` token and drops
   the RO_DATA table, leaving ~91% of the dump compared byte for byte.
2. **Task 4 grew and was retitled** to "Make the shared `Emitter`
   definitions visible in `JitEmitter.h`". Beyond the three member
   templates it also moves `isCheapConst` (a circular-include problem
   found during the task) and the `Emitter::comment` logger check (a
   code-size problem found by measurement afterwards).
3. **Task 2 gained an always-inline attribute** on
   `emit_load_from_base_offset`, release-only, after measurement showed
   it was the one helper that measurably lost inlining.
4. **Task 15 produced no commit.** Both size fixes it found were folded
   into the commits that caused them, so no commit in the stack carries
   a regression a later one cleans up.
5. **Fix rounds amended the task's own commit** rather than appending,
   by the user's decision, so the stack stays one commit per task.

## This refactor inverts the usual TDD shape

There are no new tests to write, and writing any would be wrong. The
correctness criterion is a canonicalized `-Xdump-jitcode` comparison:
runtime addresses are normalized and constant materialization collapses
to a `CONST` token, because `isCheapConst` picks instruction forms from
pointer values that ASLR changes every run, leaving about 91% of the
dump compared byte for byte. So instead of "write a failing test, make
it pass", each task is:

1. capture the emitted code before the move (done once, in Task 1),
2. move the code,
3. prove the emitted code did not change.

A task that changes emitted output has a bug in the move, not a test that
needs updating. Do not "fix" a diff by accepting it.

This applies to Tasks 2-14. Task 15 measures and changes no code.

## Global Constraints

- **Comments are preserved verbatim.** Every comment, doc comment,
  `NOTE:`, and `FIXME` moves with its code unchanged. Do not reword,
  reflow, condense, or improve any prose. The only permitted edits: the
  copyright header on a new file; a helper doc comment that the move from
  an anonymous namespace into a header makes factually false; `\return` or
  parameter docs naming a type or linkage the move changes. A reviewer
  should be able to diff the concatenation of the new files against the
  old one and see only motion.
- Copyright header (Meta MIT, from `CLAUDE.md`) at the top of every new
  file, followed by a trailing newline at end of file.
- Every new `.cpp` opens with `#include "hermes/VM/JIT/Config.h"` then
  `#if HERMESVM_JIT`, and closes with `#endif // HERMESVM_JIT`. BUCK globs
  `lib/VM/**/*.cpp` unconditionally, so a file without the guard breaks
  non-JIT builds.
- Every new `.cpp` and `.h` is added to `lib/VM/CMakeLists.txt` in the
  `if (HERMESVM_ALLOW_JIT)` block. BUCK needs no edit; CMake does.
- C++ style: 80-column lines, 2-space indent.
- Run `arc f` after editing any `.cpp`/`.h`, before committing.
- Every `sl` invocation passes `--reason "<intent> - sl help <cmd>"`.
  Never pass `--reason` to `jf`, `arc`, or `buck2`.
- Commit messages use the `[SH]` format with the body hard-wrapped at 72
  columns. Title names the component, e.g.
  `[SH] JIT: Move register allocation to JitEmitter-regalloc.cpp`.
- Do not amend or submit anything unless explicitly asked.
- File naming follows `JitEmitter-<part>.cpp`, lowercase suffix,
  hyphenated for multiple words, matching `FlowChecker-expr.cpp` and
  `ESTreeIRGen-stmt.cpp`.
- Sizes are not tracked. The bar is that no file lands over ~2,000 lines.
  Do not add line or definition counts to this plan or the spec.

## Two hazards that only appear when the file is split

Both are invisible today because there is exactly one emitter translation
unit, and both bite silently.

**Templates defined in the `.cpp`.** A function template is instantiated
only where its definition is visible. `JitEmitter.h` declares three member
templates and `JitEmitter.cpp` defines them; with one TU that is
indistinguishable from an ordinary method. Once callers live elsewhere,
those TUs see only the declaration and emit undefined references. Whether
the link actually breaks depends on whether the *defining* TU happens to
instantiate the same specialization, so some cases fail outright and
others link by accident and break on a later refactor. Task 4 removes the
hazard before any emitter moves.

**Non-function entities in the anonymous namespace.** The namespace holds
a `static_assert` and `constexpr uint32_t kMaxInlineBaseOffset`, which
`emit_load_from_base_offset` depends on. Deleting the namespace without
moving them leaves a helper referring to a constant that is gone. Task 2
moves them explicitly.

**A third, which dictates the task order.** The cold helpers call the hot
ones — `emit_stringprim_get_length_and_flags` calls
`emit_sh_ljs_get_pointer`, `emit_jsobject_init` calls
`emit_sh_cp_encode_non_null` and `emit_store_cp` — and nothing calls the
other way. So the hot helpers must reach the header (Task 2) before the
cold ones move to their own translation unit (Task 3); the reverse order
puts the cold helpers in a TU that cannot see what they call.

## Resolved before planning

The spec listed one open question: whether any helper reaches into
`Emitter` internals, which would force it to stay a member or become a
friend. **It does not.** Every free helper in the anonymous namespace
takes `a64::Assembler &` and register operands and touches no `Emitter`
state. `OurErrorHandler` holds `asmjit::Error &` and a `std::function`;
`OurLogger` holds `a64::Assembler &` and a `PerfJitDump *`. The only
`Emitter` mention in that region is asmjit's own `BaseEmitter` in an
override signature. No helper needs friendship.

---

### Task 1: Baseline capture and verification harness

Produces no commit. It creates the reference the other fourteen tasks are
checked against, so it must run **before any source change**, on a clean
checkout of the commit the split starts from.

**Files:**
- Create: `/tmp/jitsplit/config.sh`, `capture.sh`, `verify.sh` (scratch)
- Create: `/tmp/jitsplit/baseline.txt` (the reference dump)

**Interfaces:**
- Produces: `/tmp/jitsplit/verify.sh`, invoked by every later task as
  `bash /tmp/jitsplit/verify.sh`. Exits 0 only if the buck2 build, the
  CMake build, the normalized JIT dump comparison, and the lit suite all
  pass.
- Produces: `/tmp/jitsplit/config.sh`, defining `ROOT` and `CMAKE_DIR`,
  sourced by `verify.sh`.

- [ ] **Step 1: Record the starting commit**

```bash
cd ~/fbsource/xplat/static_h
mkdir -p /tmp/jitsplit
sl log -r . -T '{node}\n' --reason "record split baseline commit - sl help log" \
  > /tmp/jitsplit/baseline-commit.txt
cat /tmp/jitsplit/baseline-commit.txt
```

If `/tmp` is cleared before the stack is finished, re-run this whole task
against that commit to regenerate the baseline.

- [ ] **Step 2: Choose the CMake build directory and write the config**

Whether an in-tree CMake build directory works depends on the checkout.
On an EdenFS checkout (a path under `~/fbsource`) it does not; use an
out-of-tree directory. On an ordinary filesystem checkout, in-tree is
fine. **If it is not obvious which applies, ask rather than guessing** —
the wrong choice costs a full configure and a confusing failure.

Everything downstream reads the answer from one file, so nothing else in
this plan hardcodes a path. Write it:

```bash
cat > /tmp/jitsplit/config.sh <<'EOF'
# Edit these two to match your checkout before running anything else.
ROOT="$HOME/fbsource/xplat/static_h"
CMAKE_DIR="$HOME/work/hws/fbsource-sh-debug"
EOF
```

On the machine this plan was written on the checkout was EdenFS and
`~/work/hws/fbsource-sh-debug` already existed, configured out of tree
with `CMAKE_BUILD_TYPE=Debug`, Ninja and `HERMESVM_ALLOW_JIT=2`. Confirm
whatever you chose actually points at this checkout and compiles the JIT:

```bash
source /tmp/jitsplit/config.sh
grep -E "^CMAKE_HOME_DIRECTORY|^HERMESVM_ALLOW_JIT|^CMAKE_BUILD_TYPE" \
  "$CMAKE_DIR/CMakeCache.txt"
```

Expected: `CMAKE_HOME_DIRECTORY` is this checkout, `HERMESVM_ALLOW_JIT=2`,
`CMAKE_BUILD_TYPE=Debug`. If the directory does not exist, create it:

```bash
source /tmp/jitsplit/config.sh
cmake -B "$CMAKE_DIR" -S "$ROOT" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DHERMESVM_ALLOW_JIT=2
```

- [ ] **Step 3: Write the dump-capture script**

The corpus is every JIT lit test plus the typed tests that reach
`FastArrayLoad`. Scripts do not need to pass — several throw on purpose.
They need to be deterministic, and the capture needs to fail loudly rather
than quietly producing nothing, because a capture that degenerates to the
same error text every time would make all later comparisons pass
vacuously.

Normalization is deliberately narrow: only `0x` followed by exactly 16 hex
digits, the form the runtime addresses take. Anything shorter — immediates
and small constants — is left alone so a real change in one is visible.
The count of substitutions is emitted so a shift in it can be noticed.

```bash
cat > /tmp/jitsplit/capture.sh <<'EOF'
#!/bin/bash
# Capture a normalized dump of all JIT-emitted code over a fixed corpus.
# Usage: capture.sh <hermes-binary> <repo-root> <output-file>
set -u
HERMES="$1"; ROOT="$2"; OUT="$3"

[ -x "$HERMES" ] || { echo "capture: no such binary: $HERMES" >&2; exit 1; }

TYPED=(
  "$ROOT/test/shermes/array-typed.js"
  "$ROOT/test/hermes/flow/array-for-of.js"
  "$ROOT/test/hermes/flow/nbody.js"
)
JITTESTS=("$ROOT"/test/jit/*.js)
[ -e "${JITTESTS[0]}" ] || { echo "capture: no test/jit/*.js found" >&2; exit 1; }
for f in "${TYPED[@]}"; do
  [ -f "$f" ] || { echo "capture: missing corpus file: $f" >&2; exit 1; }
done

: > "$OUT"
run_one() { # <label> <file> <extra-flag...>
  local label="$1" file="$2"; shift 2
  local before after
  echo "===== $label =====" >> "$OUT"
  before=$(wc -l < "$OUT")
  "$HERMES" "$@" -Xjit=force -Xjit-threshold=1 -Xdump-jitcode=3 "$file" 2>&1 \
    | sed -e 's/0x[0-9A-Fa-f]\{16\}/ADDR/g' >> "$OUT"
  after=$(wc -l < "$OUT")
  if [ "$after" -le "$((before + 2))" ]; then
    echo "capture: $label produced almost no output; corpus is broken" >&2
    exit 1
  fi
}
for f in "${JITTESTS[@]}"; do run_one "$(basename "$f")" "$f"; done
for f in "${TYPED[@]}"; do run_one "typed $(basename "$f")" "$f" -typed; done

echo "lines: $(wc -l < "$OUT")  ADDR substitutions: $(grep -c ADDR "$OUT")"
EOF
chmod +x /tmp/jitsplit/capture.sh
```

- [ ] **Step 4: Confirm the corpus is deterministic and the filter is not too wide**

Two captures from one binary must be identical after normalization. They
must also differ *before* normalization only in the places the filter
touches — otherwise something else is varying and the filter is hiding it.

```bash
source /tmp/jitsplit/config.sh
cd "$ROOT"
H="$HOME/fbsource/$(buck2 build //xplat/static_h:hermes --show-output 2>/dev/null | awk '{print $2}')"
bash /tmp/jitsplit/capture.sh "$H" "$ROOT" /tmp/jitsplit/a.txt
bash /tmp/jitsplit/capture.sh "$H" "$ROOT" /tmp/jitsplit/b.txt
diff -q /tmp/jitsplit/a.txt /tmp/jitsplit/b.txt && echo "DETERMINISTIC"
```

Expected: both captures report the same `ADDR substitutions` count, and
`DETERMINISTIC`. If they differ, inspect the diff. If the differing lines
are addresses the filter missed, widen it minimally; if they are anything
else, that thing needs excluding on its own terms rather than by
broadening the address pattern.

Known residual risk, worth stating rather than pretending away: a
JIT-materialized 64-bit constant printed as a full 16-digit hex value is
indistinguishable from an address and would be masked. The substitution
count printed by `capture.sh` is the tripwire — if it changes between the
baseline and a later capture, a value moved into or out of that form and
the diff must be inspected by hand.

- [ ] **Step 5: Save the baseline dump**

```bash
cp /tmp/jitsplit/a.txt /tmp/jitsplit/baseline.txt
rm -f /tmp/jitsplit/a.txt /tmp/jitsplit/b.txt
wc -l /tmp/jitsplit/baseline.txt
```

No baseline size is captured here. Code size is measured with the repo's
own tooling — the `facebook:bloaty` skill, which compares a commit against
its parent — at the two points where it matters: after the inline-helper
commit in Task 2, and across the whole stack in Task 15. Measuring
`libhermesvm*.a` byte counts by hand would be misleading: an archive's
size includes per-member headers and padding, so turning one object into
eleven moves it regardless of code size, which is precisely the signal we
are trying to read.

- [ ] **Step 6: Write the per-task verification script**

```bash
cat > /tmp/jitsplit/verify.sh <<'EOF'
#!/bin/bash
# Verify one code-motion commit. Exits non-zero on any failure.
set -u
source /tmp/jitsplit/config.sh
cd "$ROOT" || exit 1

echo "--- buck2 build"
buck2 build //xplat/static_h:hermes || { echo "FAIL: buck2 build"; exit 1; }

echo "--- cmake build ($CMAKE_DIR)"
cmake --build "$CMAKE_DIR" --target hermes > /tmp/jitsplit/cmake.log 2>&1 \
  || { echo "FAIL: cmake build"; tail -30 /tmp/jitsplit/cmake.log; exit 1; }

echo "--- jit dump comparison"
H="$HOME/fbsource/$(buck2 build //xplat/static_h:hermes --show-output 2>/dev/null | awk '{print $2}')"
bash /tmp/jitsplit/capture.sh "$H" "$ROOT" /tmp/jitsplit/now.txt \
  || { echo "FAIL: capture"; exit 1; }
if ! diff -u /tmp/jitsplit/baseline.txt /tmp/jitsplit/now.txt \
     > /tmp/jitsplit/dump.diff; then
  echo "FAIL: emitted code changed"
  head -60 /tmp/jitsplit/dump.diff
  exit 1
fi

echo "--- lit suite"
buck2 test //xplat/static_h:lit || { echo "FAIL: lit"; exit 1; }

echo "ALL CHECKS PASSED"
EOF
chmod +x /tmp/jitsplit/verify.sh
```

- [ ] **Step 7: Prove the harness passes on an unmodified tree**

```bash
bash /tmp/jitsplit/verify.sh
```

Expected: `ALL CHECKS PASSED`. A harness that cannot pass on an unchanged
tree is worthless as a regression check. Do not start Task 2 until this
prints that line.

---

### Task 2: Move the hot helpers, macros and pragma into a header

The hot helpers move **first**, before the cold ones. The dependency runs
one way: cold helpers call hot helpers and not the reverse —
`emit_stringprim_get_length_and_flags` calls `emit_sh_ljs_get_pointer`,
`emit_jsobject_init` calls `emit_sh_cp_encode_non_null` and
`emit_store_cp`. Moving the cold helpers into their own translation unit
first would leave them calling helpers still private to `JitEmitter.cpp`,
and the new TU would not compile.

Still one emitter TU after this task. If moving helpers into a header
costs anything in code size, it shows up here, where the header extraction
is the only variable.

**Files:**
- Create: `lib/VM/JIT/arm64/JitEmitter-internal.h`
- Create: `lib/VM/JIT/arm64/JitEmitter-internal.cpp` (empty skeleton; Task 3
  fills it)
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp`
- Modify: `lib/VM/CMakeLists.txt`

**Interfaces:**
- Produces, all `inline`, in `namespace hermes::vm::arm64` in
  `JitEmitter-internal.h`: `emit_sh_ljs_get_tag`,
  `emit_sh_ljs_tag_is_pointer`, `emit_sh_ljs_tag_is_object`,
  `emit_sh_ljs_tag_is_string`, `emit_sh_ljs_get_pointer`,
  `emit_sh_ljs_object`, `emit_sh_ljs_object2`, `emit_sh_ljs_is_double`,
  `emit_sh_ljs_string`, `emit_shv_string`, `emit_sh_ljs_is_object`,
  `emit_sh_ljs_is_string`, `emit_sh_ljs_is_bigint`, `emit_sh_ljs_is_empty`,
  `emit_sh_ljs_is_null`, `emit_sh_ljs_is_bool`, `emit_sh_ljs_is_undefined`,
  `emit_sh_ljs_is_symbol`, `emit_sh_ljs_bool`, `emit_sh_ljs_bool_const`,
  `emit_sh_cp_decode`, `emit_sh_cp_decode_non_null`,
  `emit_sh_cp_encode_non_null`, `emit_load_cp`, `emit_store_cp`,
  `emit_load_shv`, `emit_store_shv`, `emit_load_from_base_offset`,
  `emit_load_slot16`, `maxNaturalBaseOffset`, `emit_double_is_int`,
  `emit_double_is_uint32`, `emit_gccell_get_kind`, `emit_cellkind_in_range`,
  `emit_cmp_imm32`, `emit_add_imm_u24`, `isCheapConst`, `isStpGpXImm`,
  `emit_sh_cp_decode_non_null_preserve_input`,
  `emit_sh_shv_decode`; plus `class Emit_sh_shv_decode` with its inline
  constructor and inline `emitAll`; plus `constexpr uint32_t
  kMaxInlineBaseOffset`; plus the `static_assert` from the top of the
  anonymous namespace; plus the macros `EXPECT_ERROR`,
  `EMIT_RUNTIME_CALL`, `EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP`,
  `EMIT_RUNTIME_CALL_WITHOUT_THUNK_AND_SAVED_IP`.
- Produces in `JitEmitter-internal.cpp`:
  `Emit_sh_shv_decode::emitFirstCase`, `Emit_sh_shv_decode::emitRestCases`.
- Consumes: nothing from earlier tasks.

- [ ] **Step 1: Create the two file skeletons**

`JitEmitter-internal.h`:

```cpp
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT

#include "JitEmitter.h"

namespace hermes::vm::arm64 {

// Declarations and inline definitions move here.

} // namespace hermes::vm::arm64

#endif // HERMESVM_JIT
```

`JitEmitter-internal.cpp` gets the copyright header,
`#include "JitEmitter-internal.h"`, the guard, and an empty
`namespace hermes::vm::arm64 { }`. It stays almost empty until Task 3.

Add `#include "JitEmitter-internal.h"` to `JitEmitter.cpp` next to its
existing `#include "JitImpl.h"`, and add both new files to
`lib/VM/CMakeLists.txt` inside the `if (HERMESVM_ALLOW_JIT)` block:

```cmake
          JIT/arm64/JitEmitter-internal.cpp JIT/arm64/JitEmitter-internal.h
```

- [ ] **Step 2: Move the hot free helpers into the header**

Cut each helper named in the Produces list above from `JitEmitter.cpp`'s
anonymous namespace and paste it into `JitEmitter-internal.h`, in the same
order, doc comments unchanged, each marked `inline`. `isCheapConst`
currently sits just below the anonymous namespace as a file-scope
`static`; it moves too, losing `static` and gaining `inline`.

Leave the four cold helpers and the two classes where they are — Task 3
moves them. The anonymous namespace therefore still exists after this
task, holding only those.

- [ ] **Step 3: Move the non-function entities**

The anonymous namespace is not only functions. Move these too, or the
helpers that depend on them will not compile:

- the `static_assert(HERMESVALUE_VERSION == 2, ...)` at the top of the
  block
- `constexpr uint32_t kMaxInlineBaseOffset = maxNaturalBaseOffset(8);`,
  which `emit_load_from_base_offset` reads

`kMaxInlineBaseOffset` must appear after `maxNaturalBaseOffset` in the
header, since it calls it at compile time.

- [ ] **Step 4: Move `Emit_sh_shv_decode`**

Put the class definition, its inline constructor, and its inline `emitAll`
in the header. Move `emitFirstCase` and `emitRestCases` — already defined
out of line today — into `JitEmitter-internal.cpp`.

- [ ] **Step 5: Move the macros and the pragma**

Move `EXPECT_ERROR` and the three `EMIT_RUNTIME_CALL` macros into the
header. Move the `-Wmissing-designated-field-initializers` pragma block
there too: every TU that builds a `SlowPath` with designated initializers
needs it, which is most of them.

```cpp
// Disable warnings about missing designated initializers since they occur often
// when we construct SlowPaths.
#ifdef __clang__
#if __has_warning("-Wmissing-designated-field-initializers")
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
#endif
#endif
```

Note the tradeoff in the commit message rather than silently widening a
suppression: today this pragma affects one translation unit; in the header
it affects every TU that includes it, and everything after the include in
those TUs, not only `SlowPath` construction. It is not wrapped in
`push`/`pop` because the suppression has to stay active through the whole
of each emitter TU — that is where the designated initializers are. The
blast radius is bounded by the include list: only emitter TUs include this
header.

- [ ] **Step 6: Format**

```bash
cd ~/fbsource/xplat/static_h && arc f
```

- [ ] **Step 7: Verify**

```bash
bash /tmp/jitsplit/verify.sh
```

Expected: `ALL CHECKS PASSED`.

- [ ] **Step 8: Commit**

```bash
cd ~/fbsource/xplat/static_h
sl commit -m "$(cat <<'EOF'
[SH] JIT: Move the inline emitter helpers to JitEmitter-internal.h

Summary:
Move the remaining free `emit_*` helpers, the `Emit_sh_shv_decode` class,
the `EXPECT_ERROR` and `EMIT_RUNTIME_CALL` macros, and the
missing-designated-initializer pragma out of `JitEmitter.cpp`'s anonymous
namespace and into `JitEmitter-internal.h`, marked `inline` so they can
still be inlined once the emitters are split across translation units.
`Emit_sh_shv_decode::emitFirstCase` and `emitRestCases` were already out
of line and go to `JitEmitter-internal.cpp`.

The anonymous namespace also held a `static_assert` and
`kMaxInlineBaseOffset`, which `emit_load_from_base_offset` reads; both
move with the helpers. The namespace still exists, holding only the four
bulky helpers and the two asmjit subclasses, which move next.

The hot helpers move before the cold ones because the dependency runs
that way: the cold helpers call `emit_sh_ljs_get_pointer`,
`emit_sh_cp_encode_non_null` and `emit_store_cp`, and nothing calls in
the other direction. Moving the cold ones out first would put them in a
translation unit that cannot see the helpers they use.

The pragma has to be shared because every translation unit that
constructs a `SlowPath` with designated initializers needs it. Moving it
to a header does widen the suppression from one translation unit to every
TU that includes the header, and to everything after the include rather
than only `SlowPath` construction. It is not wrapped in `push`/`pop`
because it has to stay active through the whole of each emitter TU; the
blast radius is bounded by the include list.

Nothing has been split yet, so this isolates the cost of the header move
itself. Pure code motion; the emitted machine code is unchanged.

Test Plan:
`buck2 build //xplat/static_h:hermes`, a CMake build of the same target,
and `buck2 test //xplat/static_h:lit`. `-Xdump-jitcode=3` output over the
JIT and typed lit tests is byte-identical to the pre-split baseline with
addresses normalized. Code size compared against the parent commit with
the `facebook:bloaty` skill.

Reviewers: Hermes

Subscribers:

Tasks:

Tags:
EOF
)" --reason "commit inline helper extraction - sl help commit"
```

- [ ] **Step 9: Measure code size against the parent commit**

This is the measurement that answers the inlining question, taken while
there is still a single emitter TU, so nothing else can be blamed for a
change. It runs after the commit because the tooling compares a commit
against its parent.

Use the repo's own size tooling rather than measuring artifacts by hand:

```
facebook:bloaty
```

That skill selects the right optimized `hermesvm` artifact and reports a
per-symbol breakdown. Do not substitute a byte count of `libhermesvm*.a`
from the `<buck output dir>`: an archive's size includes per-member
headers and alignment padding, so it moves when one object becomes eleven
regardless of how much code there is, and a bare `find` can pick up a
stale or differently-configured artifact.

Report the number. A large increase means helpers stopped being inlined,
and the fix is to reconsider which helpers are `inline`, not to carry on.

---

### Task 3: Move the cold helpers into JitEmitter-internal.cpp

These are the four bulky, rarely called helpers and the two asmjit
subclasses left behind by Task 2. They can move now because the hot
helpers they call already live in `JitEmitter-internal.h`.

**Files:**
- Modify: `lib/VM/JIT/arm64/JitEmitter-internal.h` (add declarations)
- Modify: `lib/VM/JIT/arm64/JitEmitter-internal.cpp` (add definitions)
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp` (remove the moved definitions
  and the now-empty anonymous namespace)

**Interfaces:**
- Produces, in `namespace hermes::vm::arm64`: the free functions
  `emit_jsobject_init`, `emit_environment_init`,
  `emit_stringprim_get_length_and_flags` and `emit_load_builtin_closure`,
  each **declared** in `JitEmitter-internal.h` and **defined** in
  `JitEmitter-internal.cpp`, with signatures copied verbatim from their
  current definitions; and the classes `OurErrorHandler` and `OurLogger`,
  whose **class bodies go in the header** with any out-of-line member
  definitions in the `.cpp`.
- Consumes: the two files and every hot helper from Task 2.

- [ ] **Step 1: Move the cold helpers**

Cut these from the anonymous namespace in `JitEmitter.cpp`, in the same
relative order, doc comments unchanged:

- `emit_stringprim_get_length_and_flags` — declaration to the header,
  body to the `.cpp`
- `emit_jsobject_init` — same
- `emit_environment_init` — same
- `emit_load_builtin_closure` — same
- `class OurErrorHandler` — class body to the header
- `class OurLogger` — class body to the header

- [ ] **Step 2: Delete the now-empty anonymous namespace**

`JitEmitter.cpp` should no longer have a `namespace { ... }` block. The
`STATISTIC(JITNumGetByIdSpec, ...)` declaration stays in `JitEmitter.cpp`
for now; Task 9 moves it.

- [ ] **Step 3: Format**

```bash
cd ~/fbsource/xplat/static_h && arc f
```

- [ ] **Step 4: Verify**

```bash
bash /tmp/jitsplit/verify.sh
```

Expected: `ALL CHECKS PASSED`. If the dump differs, the move changed
behaviour — revert and redo it rather than accepting the new output.

- [ ] **Step 5: Commit**

```bash
cd ~/fbsource/xplat/static_h
sl commit -m "$(cat <<'EOF'
[SH] JIT: Move the cold emitter helpers to JitEmitter-internal.cpp

Summary:
Move the bulky, rarely called free helpers out of `JitEmitter.cpp`'s
anonymous namespace and into the helper layer created by the previous
commit: `emit_jsobject_init`, `emit_environment_init`,
`emit_stringprim_get_length_and_flags`, `emit_load_builtin_closure`, and
the `OurErrorHandler` and `OurLogger` classes. They keep their bodies out
of the header and gain declarations in it, so they have external linkage
under the same names.

They move after the inline helpers rather than before, because they call
`emit_sh_ljs_get_pointer`, `emit_sh_cp_encode_non_null` and
`emit_store_cp`; those had to be visible outside `JitEmitter.cpp` first.

With this, `JitEmitter.cpp`'s anonymous namespace is gone. Pure code
motion, so the emitted machine code is unchanged.

Test Plan:
`buck2 build //xplat/static_h:hermes`, a CMake build of the same target,
and `buck2 test //xplat/static_h:lit`. `-Xdump-jitcode=3` output over the
JIT and typed lit tests is byte-identical to the pre-split baseline with
addresses normalized.

Reviewers: Hermes

Subscribers:

Tasks:

Tags:
EOF
)" --reason "commit cold helper extraction - sl help commit"
```

---

### Task 4: Move the Emitter member template definitions into JitEmitter.h

Must land before any emitter moves out of `JitEmitter.cpp`, because Task 5
is the first task that would break the link.

`JitEmitter.h` declares three member templates whose definitions live in
`JitEmitter.cpp`. A template is instantiated only where its definition is
visible, so once a caller lives in another TU it emits an undefined
reference instead. That reference resolves only if the *defining* TU
happens to instantiate the same specialization, which makes the failure
mode inconsistent:

- `_allocTemp<HWReg::GpX>` and `_allocTemp<HWReg::VecD>` are called only
  from `allocTempGpX()` and `allocTempVecD()`, which are inline in
  `JitEmitter.h` itself. Nothing in the register-allocation group calls
  them, so `JitEmitter-regalloc.cpp` would hold the definition and never
  instantiate it — undefined in every TU that allocates a temp.
- `loadBits64InGp<a64::GpX>` would be instantiated inside
  `JitEmitter-const.cpp` by its own callers and so would link **by
  accident**, while `loadBits64InGp<a64::GpW>`, used only by `loadParam`
  in the core TU, would not.

Putting the definitions in the header is the ordinary mechanism and needs
no instantiation list to maintain when someone later calls one with a new
register type. The bodies are small — 17, 14 and 11 lines.

**Files:**
- Modify: `lib/VM/JIT/arm64/JitEmitter.h`
- Modify: `lib/VM/JIT/arm64/JitEmitter.cpp`

**Interfaces:**
- Produces: definitions of `Emitter::movHWFromHW`, `Emitter::_allocTemp`
  and `Emitter::loadBits64InGp`, visible to every TU that includes
  `JitEmitter.h`.
- Consumes: nothing.

- [ ] **Step 1: Move the three definitions**

Cut these from `JitEmitter.cpp` and paste them into `JitEmitter.h`
**immediately after the closing `}; // class Emitter`** and before the
closing `} // namespace hermes::vm::arm64`, so the definition sits next to
the declaration and nobody has to hunt for it:

- `template <bool use> void Emitter::movHWFromHW(HWReg dst, HWReg src)`
- `template <class TAG> HWReg Emitter::_allocTemp(TempRegAlloc &ra, llvh::Optional<HWReg> preferred)`
- `template <typename REG> void Emitter::loadBits64InGp(...)`

Keep their doc comments with them. Leave the declarations inside the class
exactly as they are.

- [ ] **Step 2: Check nothing else is in the same position**

`JitEmitter.h` has four template sites. Confirm the other one,
`bitMask32`, is already defined in the header and needs nothing:

```bash
grep -nE "^\s*template" lib/VM/JIT/arm64/JitEmitter.h
```

Expected: four hits — `bitMask32` (already defined inline just below its
declaration) and the three now defined after the class. If a fifth appears
later, it needs the same treatment before its TU moves.

- [ ] **Step 3: Format**

```bash
cd ~/fbsource/xplat/static_h && arc f
```

- [ ] **Step 4: Verify**

```bash
bash /tmp/jitsplit/verify.sh
```

Expected: `ALL CHECKS PASSED`. This task cannot change emitted code — it
only changes where a definition is parsed.

- [ ] **Step 5: Commit**

```bash
cd ~/fbsource/xplat/static_h
sl commit -m "$(cat <<'EOF'
[SH] JIT: Define the Emitter member templates in JitEmitter.h

Summary:
Move the definitions of `Emitter::movHWFromHW`, `Emitter::_allocTemp` and
`Emitter::loadBits64InGp` from `JitEmitter.cpp` into `JitEmitter.h`,
directly after the class that declares them.

A function template is only instantiated where its definition is visible.
That is invisible while `JitEmitter.cpp` is the sole emitter translation
unit, but splitting it would leave callers in other units emitting
undefined references. The failure would not even be uniform:
`_allocTemp<GpX>` and `_allocTemp<VecD>` are reached only through the
inline `allocTempGpX()`/`allocTempVecD()` wrappers in the header and are
instantiated nowhere in the register-allocation group, so they would be
undefined everywhere, while `loadBits64InGp<GpX>` would happen to be
instantiated alongside its definition and link by accident, leaving only
`loadBits64InGp<GpW>` broken.

Defining them in the header is the ordinary mechanism and, unlike
explicit instantiation, needs no list to maintain when a new register
type is used. The three bodies are small.

Test Plan:
`buck2 build //xplat/static_h:hermes`, a CMake build of the same target,
and `buck2 test //xplat/static_h:lit`. `-Xdump-jitcode=3` output over the
JIT and typed lit tests is byte-identical to the pre-split baseline with
addresses normalized.

Reviewers: Hermes

Subscribers:

Tasks:

Tags:
EOF
)" --reason "commit template definition move - sl help commit"
```

---

## Tasks 5 through 14: one translation unit each

These ten tasks are mechanically identical and independent of each other.
Order is smallest first, to shake out the pattern before the big ones.

**The shared step sequence, applied in every one of Tasks 5-14:**

1. **Create the file.** Copyright header, then:

```cpp
#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitEmitter.h"
#include "JitEmitter-internal.h"
#include "JitImpl.h"

namespace hermes::vm::arm64 {

// Definitions move here.

} // namespace hermes::vm::arm64
#endif // HERMESVM_JIT
```

2. **Move the definitions** listed for that task out of `JitEmitter.cpp`,
   in the same relative order, comments unchanged.
3. **Fix includes.** Build, and add only the includes the compiler
   actually demands. Do not copy `JitEmitter.cpp`'s whole include list.
4. **Add the file to `lib/VM/CMakeLists.txt`** in the
   `if (HERMESVM_ALLOW_JIT)` block.
5. **`arc f`**
6. **`bash /tmp/jitsplit/verify.sh`** — expect `ALL CHECKS PASSED`.
7. **Commit** with title
   `[SH] JIT: Move <topic> to JitEmitter-<part>.cpp`, a summary saying it
   is pure code motion and listing what moved, and the Test Plan wording
   used in Tasks 2-4.

If a link error naming a template appears despite Task 4, a fourth member
template has been added since this plan was written; give it the Task 4
treatment in its own commit rather than working around it here.

---

### Task 5: `JitEmitter-regalloc.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-regalloc.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `_storeHWToFrame`, `movHWFromFR`,
      `movHWFromMem`, `movFRFromHW`, `syncFrameOutParam`, `freeReg`,
      `syncAndFreeTempReg`, `useReg`, `_spillTempForFR`, `syncToFrame`,
      `syncAllFRTempExcept`, `freeAllFRTempExcept`, `freeFRTemp`,
      `_assignAllocatedLocalHWReg`, `_isFRInRegister`,
      `getOrAllocFRInVecD`, `getOrAllocFRInGpX`, `getOrAllocFRInAnyReg`,
      `frUpdatedWithHW`, `frUpdateType`.
      `movHWFromHW` and `_allocTemp` are no longer in `JitEmitter.cpp` at
      all after Task 4; do not look for them.
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move register allocation to JitEmitter-regalloc.cpp`

### Task 6: `JitEmitter-alloc.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-alloc.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `bumpAllocAndUnpoison`, `initGCCell`, `allocInYoung`,
      `alloc2InYoung`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move young-gen allocation to JitEmitter-alloc.cpp`

### Task 7: `JitEmitter-const.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-const.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `loadConstDouble`, `loadSmallHermesValueInGpX`,
      `loadConstStringInGpX`, `loadConstBits64`, `loadConstString`,
      `loadConstBigInt`. `loadBits64InGp` is not in this list — Task 4
      moved its definition to `JitEmitter.h`.
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move constant materialization to JitEmitter-const.cpp`

### Task 8: `JitEmitter-object.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-object.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `newObject`, `newObjectWithParent`, `newObjectWithBuffer`,
      `newObjectWithBufferSlow`, `newObjectWithBufferAndParent`,
      `newTypedObjectWithBuffer`, `loadParentNoTraps`, `typedLoadParent`,
      `instanceOf`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move object construction to JitEmitter-object.cpp`

### Task 9: `JitEmitter-property.cpp`

The largest task. `Emitter::GetByIdImpl` is a nested class of `Emitter`,
forward-declared in `JitEmitter.h` and marked
`HERMES_ATTRIBUTE_INTERNAL_LINKAGE`; it must be defined in exactly one TU,
and that TU is this one. The `STATISTIC(JITNumGetByIdSpec, ...)`
declaration moves here too, along with `#include "llvh/ADT/Statistic.h"`
and the `#define DEBUG_TYPE "jit"` that `STATISTIC` needs.

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-property.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: the `GetByIdImpl` class, `getByIdImpl`, `getByIdWithReceiver`,
      `getByVal`, `getByValWithReceiver`, `getByIndex`, `putByIdImpl`,
      `putByValImpl`, `putByValWithReceiver`, `delByVal`, `defineOwnById`,
      `defineOwnInDenseArray`, `defineOwnByIndex`, `defineOwnByVal`,
      `defineOwnGetterSetterByVal`, `getOwnBySlotIdx`, `putOwnBySlotIdx`,
      `addOwnPrivateBySym`, `getOwnPrivateBySym`, `putOwnPrivateBySym`,
      `createPrivateName`, `isIn`, `privateIsIn`
- [ ] Move `STATISTIC(JITNumGetByIdSpec, ...)`, `#define DEBUG_TYPE "jit"`
      and the `llvh/ADT/Statistic.h` include; drop them from
      `JitEmitter.cpp` only if nothing else there still uses them
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move the property protocol to JitEmitter-property.cpp`

### Task 10: `JitEmitter-call.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-call.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `callImpl`, `call`, `callN`, `callBuiltin`,
      `callWithNewTarget`, `callWithNewTargetLong`, `callRequire`,
      `getBuiltinClosure`, `createThis`, `selectObject`, `loadThisNS`,
      `coerceThisNS`, `getNewTarget`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move call emission to JitEmitter-call.cpp`

### Task 11: `JitEmitter-env.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-env.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `createTopLevelEnvironment`, `createFunctionEnvironment`,
      `createEnvironment`, `getParentEnvironment`, `getEnvironment`,
      `getClosureEnvironment`, `loadFromEnvironment`, `storeToEnvironment`,
      `createClosure`, `createBaseClass`, `createDerivedClass`,
      `createGenerator`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move environments and closures to JitEmitter-env.cpp`

### Task 12: `JitEmitter-array.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-array.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `newArray`, `newArrayWithBuffer`, `newFastArray`,
      `fastArrayLength`, `fastArrayLoad`, `fastArrayStore`,
      `fastArrayPush`, `fastArrayAppend`, `iteratorBegin`, `iteratorNext`,
      `iteratorClose`, `getPNameList`, `getNextPName`, `toPropertyKey`,
      `getArgumentsLength`, `getArgumentsPropByValImpl`,
      `reifyArgumentsImpl`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move arrays, iterators and arguments to JitEmitter-array.cpp`

### Task 13: `JitEmitter-arith.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-arith.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `toNumber`, `toNumeric`, `toInt32`, `addEmptyString`,
      `arithUnop`, `booleanNot`, `bitNot`, `typeOf`, `addS`, `mod`,
      `arithBinOp`, `bitBinOp`, `strictEqualImpl`, `compareImpl`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move arithmetic and comparison to JitEmitter-arith.cpp`

### Task 14: `JitEmitter-control.cpp`

**Files:** Create `lib/VM/JIT/arm64/JitEmitter-control.cpp`; modify
`JitEmitter.cpp`, `lib/VM/CMakeLists.txt`.

- [ ] Move: `jmpTypeOfIs`, `typeOfIs`, `uintSwitchImm`, `stringSwitchImm`,
      `jmpTrueFalse`, `jmp`, `jmpUndefined`, `jmpBuiltinIs`, `jCond`,
      `jStrictEqual`, `throwInst`, `throwIfEmptyUndefinedImpl`,
      `throwIfThisInitialized`, `catchInst`, `ret`
- [ ] Add to CMake, `arc f`
- [ ] `bash /tmp/jitsplit/verify.sh` → `ALL CHECKS PASSED`
- [ ] Commit: `[SH] JIT: Move control flow to JitEmitter-control.cpp`

After Task 14, `JitEmitter.cpp` should hold only the machinery:
`Emitter::Emitter`, `enter`, `comment`, `addToRuntime`,
`assertPostInstructionInvariants`, `newBasicBlock`, `getDebugFunctionName`,
`frameSetup`, `leave`, `callThunk`, `callThunkWithSavedIP`,
`callWithoutThunk`, `emitIncrementCounter`, `initHCLazyIDMayAlloc`,
`loadFrameAddr`, `getBytecodeIP`, `unreachable`, `profilePoint`,
`directEval`, `mov`, `loadParam`, `getGlobalObject`, `declareGlobalVar`,
`debugger`, `createRegExp`, `newPrefLabel`, `reserveData`, `uint64Const`,
`registerThunk`, `emitCatchTable`, `emitSlowPaths`, `emitROData`,
`emitThunks`, and the `operator<<` at the top.

---

### Task 15: Measurement report

Changes no code. Its output is a decision: ship the stack as is, or
investigate a regression.

**Files:**
- Modify: `doc/superpowers/specs/2026-08-18-jitemitter-split-design.md`

- [ ] **Step 1: Establish the timing noise floor**

Before comparing two binaries, measure the spread on one. This is not a
formality: during the work that produced this plan, a code size
"regression" turned out to be run-to-run variance, with the unchanged
baseline producing the larger figure once in ten runs.

```bash
source /tmp/jitsplit/config.sh
H="$HOME/fbsource/$(buck2 build //xplat/static_h:hermes --show-output 2>/dev/null | awk '{print $2}')"
for i in $(seq 1 10); do
  /usr/bin/time -p "$H" -Xjit=force -Xjit-threshold=1 \
    "$ROOT/test/jit/many-property-caches.js" 2>&1 | awk '/^real/{print $2}'
done | sort -n | tee /tmp/jitsplit/noise-timing.txt
```

Record min and max. Any later difference smaller than that spread is not a
result.

- [ ] **Step 2: Compare JIT compile time against the baseline commit**

Check out the commit in `/tmp/jitsplit/baseline-commit.txt`, build, run the
same ten timing iterations, compare medians, then return to the top of the
stack.

- [ ] **Step 3: Compare code size across the whole stack**

Use the repo's size tooling, which selects the right optimized artifact
and reports per-symbol deltas:

```
binary-size-analysis
```

Give it the range from the commit in `/tmp/jitsplit/baseline-commit.txt`
to the top of the stack. For a single commit, `facebook:bloaty` compares
against the parent instead.

Do not measure this by hand with `find` and `stat` over the
`<buck output dir>`. Two things make that wrong here: an archive's byte
count includes per-member headers and alignment padding, so splitting one
object into eleven changes it independently of how much code exists; and a
bare glob can pick a stale or differently-configured artifact. The number
that matters is the size of the text section in one explicitly chosen
optimized build, which is what the skills report.

- [ ] **Step 4: Record the outcome in the spec**

Replace the spec's "Risks" entry about inlining with the measured numbers
and the noise floors they are judged against. If there is a real
regression, say so and propose the fix — most likely moving a specific
helper back to `inline` in the header — rather than quietly shipping it.

- [ ] **Step 5: Commit if the spec changed**

```bash
cd ~/fbsource/xplat/static_h
sl commit doc/superpowers/specs/2026-08-18-jitemitter-split-design.md \
  -m "[SH]EASY: Record JitEmitter split size and timing results

Summary:
Record the measured binary size and JIT compile time for the completed
\`JitEmitter.cpp\` split, together with the noise floors they are judged
against. Documentation only.

Test Plan: Documentation only; no build or tests affected.

Reviewers: Hermes

Subscribers:

Tasks:

Tags:" --reason "record measurement results - sl help commit"
```

---

## Self-Review

**Spec coverage.** Layout: Tasks 5-14, one per file, every definition
named. Helper layer and placement rule: Tasks 2 and 3. Comment
preservation: the first Global Constraint. Build system: the
`HERMESVM_JIT` guard and CMake entry are constraints, and the CMake build
is step 2 of `verify.sh`. Verification: `verify.sh` implements the
per-commit list; Task 15 implements the closing measurements with a noise
floor for each metric. Naming: a Global Constraint, used by every file
name here. The spec's open risk about helpers touching `Emitter` is
resolved above.

**Placeholders.** None. The exact include list per new file is
deliberately not enumerated — step 3 of the shared sequence says to add
only what the compiler demands, which is more reliable than a guessed
list.

**Type consistency.** Helper names in Task 2's Produces block are copied
from the current source. `verify.sh`, `capture.sh` and `config.sh` are
created in Task 1 and referenced by those names throughout; `capture.sh`
takes `<hermes-binary> <repo-root> <output-file>` and is called with those
three arguments. `ROOT` and `CMAKE_DIR` are defined only in `config.sh`
and never hardcoded elsewhere. `baseline.txt` is written in Task 1 and
read by `verify.sh`.

**Known limitation, stated rather than hidden.** The dump comparison
cannot distinguish a JIT-materialized 64-bit constant printed as a full
16-digit hex value from an address, so such a constant changing would be
masked. `capture.sh` prints the substitution count as a tripwire. Closing
this properly would need the dump to tag addresses distinctly, which is a
change to `-Xdump-jitcode` and out of scope here.

**Scratch files in `/tmp`.** They do not survive a reboot; Step 1 records
the baseline commit so they can be regenerated. Somewhere durable would
mean picking a path outside the repo, which is environment-dependent —
exactly what the spec says to ask about rather than bake in.
