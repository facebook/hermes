# Shared JIT Compiler Driver Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extract the arch-independent Compiler decode driver out of
`lib/VM/JIT/arm64/JIT.cpp` into a shared file compiled once per arch, with
byte-identical arm64 emission as the acceptance gate.

**Architecture:** The `JITContext::Compiler` nested class (bytecode decode,
BB walk, label management, error paths — no assembly) moves verbatim into
`lib/VM/JIT/JitCompiler.cpp`, wrapped in the arch namespace via a macro
supplied by a new dispatch header `lib/VM/JIT/JitCurArch.h`. Only the
`JITContext` housekeeping methods stay in `arm64/JIT.cpp`. This is
milestone 0 of the x86-64 backend spec: after it, the emitter method
surface is the compiler-enforced porting contract.

**Tech Stack:** C++17, CMake/Ninja, aarch64 cross build under qemu-user
(`aarch64/README.md`), `utils/jit/jit-dump.sh` / `jit-diff.sh`.

**Spec:** `doc/superpowers/specs/2026-08-23-x86-64-jit-design.md`

## Global Constraints

- Work in the worktree `/home/tmikov/work/hermes-x86-jit`, branch
  `x86-jit`. NEVER `cd`; pass paths to commands (project rule).
- The build to verify against is `cmake-build-arm64` (aarch64 cross build,
  already configured; see `aarch64/README.md`). It runs via qemu binfmt:
  prefix runs with `QEMU_LD_PREFIX=/usr/aarch64-linux-gnu` when invoking
  the binary directly.
- 80-column limit, 2-space indent, copyright header on every new file
  (exact header shown in Task 2).
- Acceptance gate for the whole plan: `jit-diff.sh` reports the dumps
  identical (exit 0, no `--comments-ok`), `aarch64/qemu-sanity.sh` passes
  9/9, and `LIT_FILTER="jit/"` check-hermes shows 46 passes + 1
  unsupported.
- Commit messages end with:
  `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>`

---

### Task 1: Capture the pre-refactor baseline dump

**Files:** none modified. Produces
`cmake-build-arm64/jit-baseline.dump` (untracked build artifact).

- [ ] **Step 1: Ensure the current tree builds clean**

Run:
```bash
cmake --build /home/tmikov/work/hermes-x86-jit/cmake-build-arm64 --target hermes -j "$(nproc)"
```
Expected: successful link of `bin/hermes`.

- [ ] **Step 2: Capture the baseline**

Run:
```bash
QEMU_LD_PREFIX=/usr/aarch64-linux-gnu \
  /home/tmikov/work/hermes-x86-jit/utils/jit/jit-dump.sh \
  -o /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-baseline.dump \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/bin/hermes
```
Expected: a summary line on stderr; the dump file is non-empty (hundreds
of KB). If the script errors, do NOT improvise a replacement — stop and
report.

- [ ] **Step 3: Sanity-check the capture is comparable to itself**

Run the capture a second time to a different file and compare:
```bash
QEMU_LD_PREFIX=/usr/aarch64-linux-gnu \
  /home/tmikov/work/hermes-x86-jit/utils/jit/jit-dump.sh -q \
  -o /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-baseline2.dump \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/bin/hermes
/home/tmikov/work/hermes-x86-jit/utils/jit/jit-diff.sh --dumps \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-baseline.dump \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-baseline2.dump
```
Expected: exit 0, dumps identical. (This validates the canonicalization
before we rely on it.) Then delete `jit-baseline2.dump`.

No commit for this task (no repo changes).

---

### Task 2: Move the Compiler driver to `lib/VM/JIT/JitCompiler.cpp`

**Files:**
- Create: `lib/VM/JIT/JitCurArch.h`
- Create: `lib/VM/JIT/JitCompiler.cpp`
- Modify: `lib/VM/JIT/arm64/JIT.cpp` (shrinks from 1379 lines to ~60)
- Modify: `lib/VM/CMakeLists.txt` (JIT source list, currently lines
  159–177)

**Interfaces:**
- Consumes: the existing `arm64::Emitter` and `arm64::JITContext` —
  unchanged.
- Produces: `HERMESVM_JIT_ARCH_NS` (macro naming the current arch
  namespace, defined by `JitCurArch.h`) and the shared
  `JitCompiler.cpp`, which later milestones extend with an x86-64 branch
  in `JitCurArch.h`. No signature changes anywhere.

This is pure code motion. The moved block must not be edited in any way —
byte-identical emission is the gate, and the smaller the diff, the easier
the review.

- [ ] **Step 1: Create `lib/VM/JIT/JitCurArch.h`**

Exact content:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_JITCURARCH_H
#define HERMES_VM_JIT_JITCURARCH_H

#include "hermes/VM/JIT/Config.h"

#if HERMESVM_JIT

#if defined(__aarch64__) || defined(_M_ARM64)

#include "hermes/VM/JIT/arm64/JIT.h"

#include "arm64/JitEmitter.h"
#include "arm64/JitImpl.h"

/// The namespace containing the current architecture's JIT backend. The
/// shared compiler driver (JitCompiler.cpp) is compiled inside this
/// namespace, so its unqualified references to Emitter, JITContext, FR,
/// FRType, etc. resolve to the current backend's types.
#define HERMESVM_JIT_ARCH_NS arm64

#else
#error "JitCurArch.h: unsupported JIT architecture"
#endif

#endif // HERMESVM_JIT
#endif // HERMES_VM_JIT_JITCURARCH_H
```

Rationale recorded in the spec: the Compiler is a *nested private class*
of each arch's `JITContext`, so a `using CurEmitter = ...` alias cannot
express it; compiling the shared file inside the arch namespace moves the
code with zero identifier changes.

- [ ] **Step 2: Create `lib/VM/JIT/JitCompiler.cpp`**

Open `lib/VM/JIT/arm64/JIT.cpp` and locate the boundaries of the moved
block:
- START: the comment line `// Calculate the address of the next
  instruction given the name of the` (currently line 57, immediately
  after the closing brace of `JITContext::markRoots`).
- END: the closing brace of `Compiler::emitCreateRegExp` (currently line
  1374, immediately before `} // namespace arm64`).

Create `JitCompiler.cpp` with this frame, pasting the block **verbatim**
(no reflowing, no comment edits) where indicated:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// \file
/// The arch-independent JIT compiler driver: bytecode decode, basic-block
/// walk, label management and error handling. It contains no assembly; it
/// forwards to the current arch's Emitter, whose namespace is selected by
/// JitCurArch.h. Compiled once per binary, for exactly one arch.

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "JitCurArch.h"

#include "hermes/Inst/InstDecode.h"
#include "hermes/VM/JIT/DiscoverBB.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/StringPrimitiveValueDenseMapInfo-inline.h"

#define DEBUG_TYPE "jit"

namespace hermes {
namespace vm {
namespace HERMESVM_JIT_ARCH_NS {

// <<< PASTE the moved block (old JIT.cpp lines 57-1374) here, verbatim >>>

} // namespace HERMESVM_JIT_ARCH_NS
} // namespace vm
} // namespace hermes
#endif // HERMESVM_JIT
```

The moved block starts with the `NEXTINST`/`IPADD`/`ID`/`JIT_INLINE`
macro definitions and contains, in order: the `class JITContext::Compiler`
definition, `JITContext::compileImpl`, `Compiler::compileCodeBlock`,
`Compiler::compileCodeBlockImpl`, and every `Compiler::emit*` method
through `emitCreateRegExp`.

- [ ] **Step 3: Shrink `lib/VM/JIT/arm64/JIT.cpp`**

Delete the moved block from `JIT.cpp`. The remaining file is exactly:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "hermes/VM/JIT/arm64/JIT.h"

#include "JitImpl.h"

#define DEBUG_TYPE "jit"

namespace hermes {
namespace vm {
namespace arm64 {

JITContext::JITContext(bool enable) : enabled_(enable) {
  if (!enable)
    return;
  impl_ = std::make_unique<Impl>();
}

JITContext::~JITContext() = default;

void JITContext::setHCIdLimit(uint32_t hcIdLimit) {
  if (impl_)
    impl_->hcIdLimit = std::min<uint32_t>(hcIdLimit, Impl::kHCIdOverflow);
}

void JITContext::dumpCounters(llvh::raw_ostream &os) {
  static constexpr const char *kCounterNames[] = {
#define COUNTER_NAME(name) #name,
      JIT_COUNTERS(COUNTER_NAME)
#undef COUNTER_NAME
  };
  for (unsigned i = 0; i < (unsigned)JitCounter::_Last; ++i)
    os << kCounterNames[i] << ": " << counters_[i] << "\n";
}

void JITContext::markRoots(
    RootAcceptorWithNames &acceptor,
    bool markLongLived) {
  if (!impl_)
    return;
  acceptor.accept(impl_->usedHCs);
}

} // namespace arm64
} // namespace vm
} // namespace hermes
#endif // HERMESVM_JIT
```

Note the include set: `JitEmitter.h`, `InstDecode.h`, `DiscoverBB.h`,
`RuntimeModule.h`, and `StringPrimitiveValueDenseMapInfo-inline.h` are no
longer needed here and are dropped; `JitImpl.h` stays (`Impl` must be
complete for the ctor/dtor and `markRoots`). If the compiler reports a
missing type in this file, add back the *single* header that provides it
rather than restoring the whole include list.

- [ ] **Step 4: Add the new files to the build**

In `lib/VM/CMakeLists.txt`, inside the `if (HERMESVM_ALLOW_JIT)` source
list, change the line

```
          JIT/JitHandlers.cpp JIT/JitHandlers.h
```

to

```
          JIT/JitHandlers.cpp JIT/JitHandlers.h
          JIT/JitCompiler.cpp JIT/JitCurArch.h
```

- [ ] **Step 5: Build**

Run:
```bash
cmake --build /home/tmikov/work/hermes-x86-jit/cmake-build-arm64 --target hermes -j "$(nproc)"
```
Expected: clean build. Likely first-attempt errors and their fixes:
- "member access into incomplete type" in `JitCompiler.cpp` → a header
  that `arm64/JIT.cpp` previously included transitively is missing; add
  it to `JitCompiler.cpp`'s include list.
- Duplicate symbol at link → part of the moved block was left behind in
  `arm64/JIT.cpp`; delete it there, never by editing the copy in
  `JitCompiler.cpp`.

- [ ] **Step 6: Byte-identical gate**

Run:
```bash
QEMU_LD_PREFIX=/usr/aarch64-linux-gnu \
  /home/tmikov/work/hermes-x86-jit/utils/jit/jit-dump.sh -q \
  -o /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-after.dump \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/bin/hermes
/home/tmikov/work/hermes-x86-jit/utils/jit/jit-diff.sh --dumps \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-baseline.dump \
  /home/tmikov/work/hermes-x86-jit/cmake-build-arm64/jit-after.dump
```
Expected: exit 0, dumps identical — including comments (do NOT pass
`--comments-ok`; the moved code contains the comment-emitting paths, so
comment drift would also indicate a real change). Any difference at all
means the move was not verbatim: diff the moved block against git history
(`git diff HEAD -- lib/VM/JIT/arm64/JIT.cpp lib/VM/JIT/JitCompiler.cpp`)
and fix the discrepancy; do not rationalize differences away.

- [ ] **Step 7: Prove the gate can fail**

Temporarily change, in `JitCompiler.cpp`, the body of `emitLoadConstZero`
from

```cpp
  em_.loadConstDouble(FR(inst->op1), 0, "Zero");
```

to

```cpp
  em_.loadConstDouble(FR(inst->op1), 1, "Zero");
```

Rebuild, re-run Step 6's capture+diff (to a throwaway output file).
Expected: `jit-diff.sh` exits 1 and reports instruction differences.
Then **revert the change**, rebuild, and re-run Step 6 verbatim.
Expected: exit 0 again. This proves the pipeline is actually comparing
emitted code.

- [ ] **Step 8: Functional gates**

Run:
```bash
/home/tmikov/work/hermes-x86-jit/aarch64/qemu-sanity.sh
LIT_FILTER="jit/" cmake --build /home/tmikov/work/hermes-x86-jit/cmake-build-arm64 --target check-hermes -j "$(nproc)"
```
Expected: sanity 9/9 passed; lit shows `Expected Passes: 46`,
`Unsupported Tests: 1`.

- [ ] **Step 9: Confirm non-JIT builds are unaffected**

Run:
```bash
cmake --build /home/tmikov/work/hermes-x86-jit/cmake-build-host --target hermes -j "$(nproc)"
```
Expected: clean build (this x86-64 host build has the JIT off; the new
files must compile to nothing under `#if HERMESVM_JIT` being 0 — note
`HERMESVM_ALLOW_JIT` is not set for this tree, so they are not even
listed; the point is the CMake edit broke nothing).

- [ ] **Step 10: Commit**

```bash
git -C /home/tmikov/work/hermes-x86-jit add lib/VM/JIT/JitCurArch.h \
  lib/VM/JIT/JitCompiler.cpp lib/VM/JIT/arm64/JIT.cpp lib/VM/CMakeLists.txt
git -C /home/tmikov/work/hermes-x86-jit commit -m "JIT: Move the compiler driver to a shared file

The Compiler nested class -- bytecode decode, BB walk, labels, error
paths -- contains no assembly and forwards to the Emitter, so it is
arch-independent by construction. Move it verbatim from arm64/JIT.cpp
to lib/VM/JIT/JitCompiler.cpp, compiled inside the arch namespace
selected by the new JitCurArch.h. arm64/JIT.cpp keeps only the
JITContext housekeeping methods.

This is milestone 0 of the x86-64 backend plan: the emitter method
surface becomes the compiler-enforced porting contract, and the
x86-64 backend will plug in as a second branch in JitCurArch.h.

Verified byte-identical emission over the jit-dump corpus, plus
qemu-sanity 9/9 and test/jit 46+1 under qemu.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>"
```

---

### Task 3: Update the docs to match

**Files:**
- Modify: `doc/JIT.md` (source-layout table row for `JIT.cpp`, ~line 37;
  porting-notes bullet about arch-independent pieces, ~line 498)
- Modify: `doc/superpowers/specs/2026-08-23-x86-64-jit-design.md`
  (the shared-by-construction bullet's mechanism description)

**Interfaces:** documentation only.

- [ ] **Step 1: Update the `doc/JIT.md` source-layout table**

Replace the row

```
| `lib/VM/JIT/arm64/JIT.cpp` | `JITContext::Compiler`: per-opcode `emitXXX` methods that decode operands and forward to the `Emitter`. Drives BB-by-BB compilation. |
```

with

```
| `lib/VM/JIT/JitCompiler.cpp` | `JITContext::Compiler`: per-opcode `emitXXX` methods that decode operands and forward to the `Emitter`. Drives BB-by-BB compilation. Arch-independent; compiled inside the arch namespace selected by `JitCurArch.h`. |
| `lib/VM/JIT/arm64/JIT.cpp` | `JITContext` housekeeping: construction, `setHCIdLimit`, `dumpCounters`, `markRoots`. |
```

- [ ] **Step 2: Update the porting-notes bullet in `doc/JIT.md`**

Replace

```
- **Arch-independent pieces** usable as-is: DiscoverBB, RuntimeOffsets,
  PerfJitDump, JitHandlers and JitCounters (both already moved out of
  `arm64/`), the Compiler driver in JIT.cpp (contains no assembly; it could
  move to a shared location or be duplicated with the namespace changed).
```

with

```
- **Arch-independent pieces** usable as-is: DiscoverBB, RuntimeOffsets,
  PerfJitDump, JitHandlers, JitCounters, and the Compiler driver
  (JitCompiler.cpp) -- all now outside `arm64/`. A new backend implements
  the Emitter method surface and adds its branch to `JitCurArch.h`; the
  driver then compiles against it unchanged.
```

- [ ] **Step 3: Correct the mechanism description in the spec**

In `doc/superpowers/specs/2026-08-23-x86-64-jit-design.md`, in the
"Shared by construction" bullet, replace the parenthetical

```
(`JitCurArch.h`: `using CurEmitter =
  arm64::Emitter;` / `x86_64::Emitter` under the same `#if`s as
  `JIT/JIT.h`). Not a template: only one arch is ever compiled per binary,
```

with

```
(`JitCurArch.h` defines `HERMESVM_JIT_ARCH_NS` and includes the arch's
  emitter headers under the same `#if`s as `JIT/JIT.h`; the driver is
  compiled inside that namespace, so the Compiler stays a nested private
  class of the arch's `JITContext` with zero identifier changes -- a
  `using` alias cannot express a nested class). Not a template: only one
  arch is ever compiled per binary,
```

- [ ] **Step 4: Commit**

```bash
git -C /home/tmikov/work/hermes-x86-jit add doc/JIT.md \
  doc/superpowers/specs/2026-08-23-x86-64-jit-design.md
git -C /home/tmikov/work/hermes-x86-jit commit -m "doc: JIT.md and x86-64 spec updates for the shared compiler driver

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>"
```

---

## Out of scope for this plan

Milestones 1-6 of the spec (the x86-64 tree itself) each get their own
plan, written when the preceding milestone completes. Nothing in this
plan creates any x86-64 code; `JitCurArch.h`'s `#error` branch is the
placeholder the milestone-1 plan replaces.
