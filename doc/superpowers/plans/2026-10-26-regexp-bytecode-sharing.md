# Shared RegExp Bytecode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> superpowers:subagent-driven-development (recommended) or
> superpowers:executing-plans to implement this plan task-by-task. Steps
> use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Materialize each regex literal's bytecode at most once per
`RuntimeModule` or `SHUnit` and share it among every `JSRegExp` created
from it. Make Static Hermes compile regex literals ahead of time.

**Architecture:** A new immutable, intrusively and atomically
refcounted `vm::RegExpBytecode` holds one regexp's bytecode in a single
malloc. It carries an ownership kind:
- `Exclusive`: held by exactly one `JSRegExp`, which reports its memory;
- `Cached`: held by a literal cache, which reports it.

Each `RuntimeModule` and each `SHUnitExt` keeps a lazily filled vector
of `Cached` entries indexed by literal ID. SH codegen emits the
literals' bytecode into the generated C, and `_sh_ljs_create_regexp`
reads it from the unit instead of parsing.

**Tech Stack:** C++17, C (SH-generated code), lit/FileCheck, GoogleTest,
ASan/TSan.

**Spec:** `doc/superpowers/specs/2026-10-26-regexp-bytecode-sharing-design.md`
(Codex-reviewed over 3 rounds; binding). Read it in full before Task 1,
and read `DESIGN-PRINCIPLES.md` (repo root). Where this plan resolves an
ambiguity in the spec, it says so in the task ("Spec resolution"). The
last section lists them all.

## Global Constraints

- **Repository.** The repository is the worktree
  `/home/tmikov/work/hermes-regex-jit` (branch `regexp-bytecode-sharing`).
  The shell's working directory is a DIFFERENT checkout
  (`/home/tmikov/work/hermes`): every path, `git` and `cmake` command uses
  `R=/home/tmikov/work/hermes-regex-jit` explicitly (`git -C $R ...`).
  Never bare `cd`; use `( cd dir; cmd; )` if unavoidable.
- **Scope.**
  - Regex *literals* only: `CreateRegExp` in HBC, `CreateRegExpInst` in
    SH.
  - Non-goals, from the spec:
    - caching `new RegExp(str)` or `new RegExp(re)`;
    - weak references or eviction;
    - borrowing module storage;
    - matcher costs;
    - an SH heap-snapshot node (filed as a dz issue in Task 6).
- **Names** (spec-fixed):
  - `vm::RegExpBytecode` in `include/hermes/VM/RegExpBytecode.h`, with
    `enum class Ownership : uint8_t { Exclusive, Cached }`;
  - `RuntimeModule::getRegExpBytecode(uint32_t)` and
    `RuntimeModule::regExpCacheMemorySize()`;
  - `JSRegExp::getBytecode()`;
  - SH ABI: `SHRegExpTableEntry {offset, length}` and `SHUnit` fields
    `regexp_storage`, `regexp_table`, `num_regexps`;
  - `_sh_ljs_create_regexp(shr, unit, pattern, flags, regexp_id)`;
  - generated C arrays `s_regexp_storage` and `s_regexp_table`.
- **GC-safe coding.** Every C++ change under `lib/VM/`,
  `include/hermes/VM/` or `unittests/VMRuntime/` is written after
  invoking the `gc-safe-coding` skill.
  - Hermes throws by longjmp, so no `LocalsRAII` or `GCScope` may span a
    call that can throw.
  - Never compare raw pointers across a GC point. Compare bytecode
    pointers only between objects that are both rooted.
- **Style.**
  - 80 columns, 2-space indent.
  - A doc comment on every declaration.
  - Only trivial one-line methods inside a class body.
  - New files start with the Meta copyright header (CLAUDE.md).
- **Build trees** (configured: clang, Debug, `-O1`):
  - `$R/cmake-build-asan`: ASan, `HERMESVM_ALLOW_JIT=2`; the reference
    tree;
  - `$R/cmake-build-tsan`: TSan;
  - `$R/cmake-build-hsan`: ASan, MallocGC, `HERMESVM_SANITIZE_HANDLES`;
  - `$R/cmake-build-hv32`: ASan, `HERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32`.

  Rebuild a tree before running its tests and after every mutation and
  restore:

  ```bash
  cmake --build $R/cmake-build-<t> --target hermes hermesc shermes \
    HermesVMRuntimeTests
  ```
- **Tests.**
  - Unit tests:
    `$R/cmake-build-<t>/unittests/VMRuntime/HermesVMRuntimeTests
    --gtest_filter='<pattern>'`.
  - Full suite: `cmake --build $R/cmake-build-<t> --target check-hermes`.
    Baseline on `cmake-build-asan` at the spec commit: 4473 expected
    passes, 6 expected failures, 162 unsupported, 0 failures.
  - Filtered lit: prefix with `LIT_FILTER='<regex>'`.
- **Prove-can-fail.** Show every new test FAILING with the code it pins
  disabled, using the mutation named in the task. Then restore the code,
  REBUILD the tree, and re-run the test green.
  - A test labelled "regression guard" passes before the change. It is
    still shown to fail under its named mutation.
- **Scratch files.** `S` below is your session scratchpad directory (the
  harness gives its path). Put every temporary file there, never in the
  repo.
- **Commits.**
  - One commit per task. The subject is given in the task. Every message
    ends with exactly:
    ```
    Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_019iPixSEVBGc2qFmM3Mq1Js
    ```
  - Stage by explicit path only: never `git add -A`, never
    `git add dz/issues`.
  - Never push.
- **Reporting.** Each task's report lists:
  - the commit hash;
  - the trees built;
  - every suite run, with its pass/unsupported/fail counts;
  - each mutation, with the failure it produced.

  Task 6 copies these into the spec's "Delivered" section.
- **BASE.** The spec is commit `fb22f7c1f`, and this plan is committed on
  top of it. BASE for the final code review is the plan commit; Task 1's
  report records it (`git -C $R rev-parse HEAD` before Task 1).

## Review Focus

These are inputs the spec implies but its test list does not name, with
the behavior a person would expect. Each has a test in the owning task.

1. **Per-object state on shared bytecode.** Two live regexps from the
   same literal keep independent `lastIndex`, `exec` progress and named
   groups, and a regexp used as a `split` separator keeps its own
   `lastIndex` and `sticky`. Tested on the interpreter, `-O0`, lazy and
   the JIT by `test/hermes/regexp-literal-shared.js` and
   `test/jit/regexp-literal-shared.js` (Task 2).
2. **Literals in different modules with the same ID.** Two modules whose
   only literals are `/one/` and `/two/` both use regexp ID 0. Each must
   evaluate its own pattern (Task 2, `LiteralCachesArePerModule`).
3. **A moving GC.** On the MallocGC `cmake-build-hsan` tree with
   sanitize rate 1.0, every allocation moves the heap, and with it the
   `JSRegExp` cell holding the refcounted pointer. Matches must still be
   right, with no double-release and no ASan report. The default rate is
   only 0.01 (`include/hermes/VM/RuntimeFlags.h:44-56`,
   `unittests/VMRuntime/VMRuntimeTestHelpers.h:41`), so:
   - `RegExpBytecodeTest` uses a fixture with rate 1.0;
   - `regexp-literal-shared.js` has a `-gc-sanitize-handles=1` RUN line.
     The option is accepted, and inert, in trees without handle
     sanitization.

   Both run on `cmake-build-hsan` in Task 2.
4. **Compressed pointers (HV32).** The `JSRegExp` layout changes, which
   affects the 8-byte alignment comment at `JSRegExp.h:145-148`. The full
   suite must pass on `cmake-build-hv32` (Task 6).
5. **A non-ASCII, `u`-flag literal in SH AOT.** Its pattern symbol is
   UTF-16, and its bytecode was compiled from the compiler's UTF-8 form.
   `source` and matching must agree with the interpreter (Task 4,
   `test/shermes/regexp-aot.js`).

---

## File Structure

- `include/hermes/VM/RegExpBytecode.h`, `lib/VM/RegExpBytecode.cpp`
  (new), `lib/VM/CMakeLists.txt`: the shared bytecode object.
- `include/hermes/VM/JSRegExp.h`, `lib/VM/JSRegExp.cpp`: hold an
  `IntrusiveRefCntPtr`, the ownership-aware fast copy and memory
  reporting.
- `lib/VM/Interpreter-slowpaths.cpp`: `Interpreter::createRegExp`.
- `include/hermes/VM/RuntimeModule.h`, `lib/VM/RuntimeModule.cpp`,
  `lib/VM/Domain.cpp`: the module literal cache, its snapshot nodes and
  its malloc size.
- `include/hermes/VM/static_h.h`, `lib/VM/StaticH.cpp`,
  `lib/VM/SHUnitExt.h`, `lib/VM/StaticHUnit.cpp`: the SH ABI and the unit
  cache.
- `lib/BCGen/SH/SH.cpp`: SH emission.
- Tests:
  - `unittests/VMRuntime/RegExpBytecodeTest.cpp` (new),
    `unittests/VMRuntime/CMakeLists.txt`,
    `unittests/VMRuntime/HeapSnapshotTest.cpp`;
  - `test/hermes/regexp-literal-shared.js`,
    `test/jit/regexp-literal-shared.js`, `test/shermes/regexp-aot.js`
    (new).
- The spec's "Delivered" section, and one dz issue.

---

### Task 1: `RegExpBytecode` and `JSRegExp` on top of it

A refactor with no sharing yet. Every `JSRegExp` gets an `Exclusive`
entry. The interpreter copies its literal's bytes into a fresh entry,
exactly as it copies them into a malloc today.

**Files:**
- Create: `include/hermes/VM/RegExpBytecode.h`, `lib/VM/RegExpBytecode.cpp`
- Modify: `lib/VM/CMakeLists.txt` (source list; `JSRegExp.cpp` is at
  line 39)
- Modify: `include/hermes/VM/JSRegExp.h:11-18`, `:50-58`, `:133-156`
- Modify: `lib/VM/JSRegExp.cpp:101-166`, `:192-203`, `:264-275`,
  `:324-392`, `:400-425`
- Modify: `lib/VM/Interpreter-slowpaths.cpp:618-619`
- Create: `unittests/VMRuntime/RegExpBytecodeTest.cpp`
- Modify: `unittests/VMRuntime/CMakeLists.txt:11-79`

**Interfaces:**
- **Produces:**
  - `RegExpBytecode::create(llvh::ArrayRef<uint8_t>, Ownership)`,
    returning `llvh::IntrusiveRefCntPtr<RegExpBytecode>`;
  - `bytecode()`, `header()`, `ownership()` and `mallocSize()`;
  - `JSRegExp::initialize(self, runtime, pattern, flags,
    llvh::IntrusiveRefCntPtr<RegExpBytecode>)`, which replaces the
    `ArrayRef` overload;
  - `llvh::ArrayRef<uint8_t> JSRegExp::getBytecode() const`;
  - `const RegExpBytecode *JSRegExp::getBytecodeEntry() const`.
- **Spec resolution:** the spec names only `getBytecode()`.
  `getBytecodeEntry()` is added because the tests in Tasks 2-4 must read
  the entry's ownership and `mallocSize()`.
- **Spec resolution:** an `Exclusive` `JSRegExp` now reports
  `mallocSize()`, which includes the entry's header, instead of
  `bytecodeSize_`. This is the entry's real malloc size.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Write the tests**

Create `unittests/VMRuntime/RegExpBytecodeTest.cpp`:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers1.h"
#include "VMRuntimeTestHelpers.h"

#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSRegExp.h"
#include "hermes/VM/RegExpBytecode.h"
#include "hermes/VM/StringPrimitive.h"

#include "gtest/gtest.h"

#include <cstddef>

using namespace hermes::vm;

namespace hermes {
namespace unittest {
namespace regexpbytecodetest {
namespace {

/// Runs every test with handle sanitization at every allocation. On a
/// HERMESVM_SANITIZE_HANDLES build with MallocGC, each allocation then
/// moves the heap (Hades instead collects the young generation); without
/// HERMESVM_SANITIZE_HANDLES the rate has no effect.
class RegExpBytecodeTest : public RuntimeTestFixtureBase {
 public:
  RegExpBytecodeTest()
      : RuntimeTestFixtureBase(
            RuntimeConfig::Builder()
                .withGCConfig(
                    GCConfig::Builder(kTestGCConfigBuilder)
                        .withSanitizeConfig(vm::GCSanitizeConfig::Builder()
                                                .withSanitizeRate(1.0)
                                                .build())
                        .build())
                .build()) {}
};

/// \return element \p index of \p arr, which must be a JSRegExp.
JSRegExp *regExpAt(Runtime &runtime, Handle<JSArray> arr, uint32_t index) {
  return vmcast<JSRegExp>(arr->at(runtime, index).unboxToHV(runtime));
}

/// Run \p code in a new module and \return its completion value, which
/// must be an array.
Handle<JSArray> runForArray(Runtime &runtime, const char *code) {
  CallResult<HermesValue> res =
      runtime.run(code, "test.js", hbc::CompileFlags{});
  EXPECT_NE(ExecutionStatus::EXCEPTION, res.getStatus());
  return runtime.makeHandle(vmcast<JSArray>(*res));
}

TEST(RegExpBytecodeObjectTest, CreateCopiesBytes) {
  auto compiled = CompiledRegExp::tryCompile("a(b)c", "gy");
  ASSERT_TRUE(compiled.hasValue());
  llvh::ArrayRef<uint8_t> src = compiled->getBytecode();

  llvh::IntrusiveRefCntPtr<RegExpBytecode> entry =
      RegExpBytecode::create(src, RegExpBytecode::Ownership::Exclusive);
  EXPECT_NE(src.data(), entry->bytecode().data());
  EXPECT_TRUE(src.equals(entry->bytecode()));
  EXPECT_TRUE(
      regex::SyntaxFlags::fromByte(entry->header().syntaxFlags).sticky);
  EXPECT_EQ(RegExpBytecode::Ownership::Exclusive, entry->ownership());
  // The bytes follow the object, aligned like a malloc result.
  EXPECT_EQ(
      0u,
      reinterpret_cast<uintptr_t>(entry->bytecode().data()) %
          alignof(std::max_align_t));
  EXPECT_GE(entry->mallocSize(), sizeof(RegExpBytecode) + src.size());
  EXPECT_LT(
      entry->mallocSize(),
      sizeof(RegExpBytecode) + alignof(std::max_align_t) + src.size());

  // A second reference keeps the bytes alive after the first is dropped
  // (ASan checks the read).
  llvh::IntrusiveRefCntPtr<RegExpBytecode> second = entry;
  entry.reset();
  EXPECT_TRUE(src.equals(second->bytecode()));
}

TEST_F(RegExpBytecodeTest, DynamicRegExpsAreExclusive) {
  Handle<JSArray> arr = runForArray(
      runtime, "[new RegExp('abc', 'g'), new RegExp('abc', 'g')]");
  JSRegExp *r0 = regExpAt(runtime, arr, 0);
  JSRegExp *r1 = regExpAt(runtime, arr, 1);
  EXPECT_NE(r0->getBytecode().data(), r1->getBytecode().data());
  EXPECT_TRUE(r0->getBytecode().equals(r1->getBytecode()));
  EXPECT_EQ(
      RegExpBytecode::Ownership::Exclusive,
      r0->getBytecodeEntry()->ownership());
  // The sole holder reports the entry.
  EXPECT_EQ(
      r0->getBytecodeEntry()->mallocSize(), r0->getVT()->getMallocSize(r0));
}

TEST_F(RegExpBytecodeTest, SplitDoesNotClearStickyOfItsArgument) {
  // split() makes a sticky-cleared copy of its separator
  // (lib/VM/JSLib/RegExp.cpp:2436-2439); the separator itself must stay
  // sticky.
  CallResult<HermesValue> res = runtime.run(
      "var r = /a/y; 'bab'.split(r); r.lastIndex = 0;"
      "r.exec('ba') === null && r.sticky",
      "test.js",
      hbc::CompileFlags{});
  ASSERT_FALSE(isException(res));
  EXPECT_TRUE(res->getBool());
}

} // namespace
} // namespace regexpbytecodetest
} // namespace unittest
} // namespace hermes
```

In `unittests/VMRuntime/CMakeLists.txt`, add `RegExpBytecodeTest.cpp` to
`RTSources` on its own line, immediately before `RuntimeConfigTest.cpp`
(line 60).

- [ ] **Step 3: See it fail**

```bash
cmake --build $R/cmake-build-asan --target HermesVMRuntimeTests
```

Expected: a compile error, `hermes/VM/RegExpBytecode.h` not found.

- [ ] **Step 4: Write `RegExpBytecode`**

`include/hermes/VM/RegExpBytecode.h`:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_REGEXPBYTECODE_H
#define HERMES_VM_REGEXPBYTECODE_H

#include "hermes/Regex/RegexBytecode.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/IntrusiveRefCntPtr.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace hermes {
namespace vm {

/// The bytecode of one compiled regexp. It is immutable after create() and
/// shared through llvh::IntrusiveRefCntPtr. The count is atomic because the
/// last reference may be dropped by a JSRegExp finalizer on the Hades
/// background thread. The object and its bytes are one malloc.
class RegExpBytecode final
    : public llvh::ThreadSafeRefCountedBase<RegExpBytecode> {
 public:
  /// Who is responsible for reporting this object's memory.
  enum class Ownership : uint8_t {
    /// Owned by exactly one JSRegExp; that JSRegExp reports it.
    Exclusive,
    /// Held by a literal cache (RuntimeModule or SHUnitExt) and shared by
    /// any number of JSRegExps; the cache reports it.
    Cached,
  };

  /// \return a new object holding a copy of \p bytecode, which must start
  /// with a regex::RegexBytecodeHeader.
  static llvh::IntrusiveRefCntPtr<RegExpBytecode> create(
      llvh::ArrayRef<uint8_t> bytecode,
      Ownership ownership);

  /// \return the bytecode.
  llvh::ArrayRef<uint8_t> bytecode() const {
    return {bytes(), size_};
  }

  /// \return the header at the start of the bytecode.
  const regex::RegexBytecodeHeader &header() const {
    return *reinterpret_cast<const regex::RegexBytecodeHeader *>(bytes());
  }

  /// \return who reports this object's memory.
  Ownership ownership() const {
    return ownership_;
  }

  /// \return the bytes of malloc'd memory, including this header.
  size_t mallocSize() const;

  /// Free the single allocation made by create(); called through the
  /// delete in ThreadSafeRefCountedBase::Release().
  static void operator delete(void *p) {
    free(p);
  }

 private:
  /// Construct the header; create() copies the bytes after it.
  RegExpBytecode(uint32_t size, Ownership ownership)
      : size_(size), ownership_(ownership) {}

  /// \return the offset of the bytes from the start of the allocation:
  /// the object size rounded up to malloc alignment.
  static constexpr size_t bytesOffset();

  /// \return the start of the bytes.
  inline const uint8_t *bytes() const;

  /// \return the start of the bytes.
  inline uint8_t *bytes();

  /// Number of bytes of bytecode.
  uint32_t size_;

  /// Who reports this object's memory.
  Ownership ownership_;
};

constexpr size_t RegExpBytecode::bytesOffset() {
  return (sizeof(RegExpBytecode) + alignof(std::max_align_t) - 1) /
      alignof(std::max_align_t) * alignof(std::max_align_t);
}

inline const uint8_t *RegExpBytecode::bytes() const {
  return reinterpret_cast<const uint8_t *>(this) + bytesOffset();
}

inline uint8_t *RegExpBytecode::bytes() {
  return reinterpret_cast<uint8_t *>(this) + bytesOffset();
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_REGEXPBYTECODE_H
```

`lib/VM/RegExpBytecode.cpp`:

```cpp
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/RegExpBytecode.h"

#include "hermes/Support/CheckedMalloc.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <new>

namespace hermes {
namespace vm {

llvh::IntrusiveRefCntPtr<RegExpBytecode> RegExpBytecode::create(
    llvh::ArrayRef<uint8_t> bytecode,
    Ownership ownership) {
  assert(
      bytecode.size() >= sizeof(regex::RegexBytecodeHeader) &&
      "regexp bytecode must start with a header");
  assert(
      bytecode.size() <= std::numeric_limits<uint32_t>::max() &&
      "Bytecode size cannot exceed 32 bits");
  void *mem = checkedMalloc(bytesOffset() + bytecode.size());
  auto *self = new (mem) RegExpBytecode((uint32_t)bytecode.size(), ownership);
  memcpy(self->bytes(), bytecode.data(), bytecode.size());
  return llvh::IntrusiveRefCntPtr<RegExpBytecode>(self);
}

size_t RegExpBytecode::mallocSize() const {
  return bytesOffset() + size_;
}

} // namespace vm
} // namespace hermes
```

Add `RegExpBytecode.cpp` to the source list in `lib/VM/CMakeLists.txt`,
next to `JSRegExp.cpp`, keeping the list's order style. Check
`grep -n checkedMalloc $R/include/hermes/Support/CheckedMalloc.h` for
the function's namespace. If it is not reachable unqualified inside
`hermes::vm`, qualify it as `hermes::checkedMalloc`, the way
`JSRegExp.cpp:273` reaches it.

- [ ] **Step 5: Convert `JSRegExp`**

`include/hermes/VM/JSRegExp.h`:
- Add `#include "hermes/VM/RegExpBytecode.h"` after the
  `hermes/VM/JSObject.h` include (line 13).
- Replace the `ArrayRef` `initialize` (lines 50-58) with:

```cpp
  /// Initializes RegExp with existing bytecode. Populates fields for the
  /// pattern and flags, but performs no validation on them. It is assumed that
  /// the bytecode is correct and corresponds to the given pattern/flags.
  /// \p bytecode is shared, not copied. The regexp must not have been
  /// initialized before.
  static void initialize(
      Handle<JSRegExp> selfHandle,
      Runtime &runtime,
      Handle<StringPrimitive> pattern,
      Handle<StringPrimitive> flags,
      llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode);
```

- After `setSyntaxFlags` (line 101), add:

```cpp
  /// \return the bytecode. The regexp must have been initialized.
  llvh::ArrayRef<uint8_t> getBytecode() const {
    return bytecode_->bytecode();
  }

  /// \return the object holding the bytecode, or null before
  /// initialization.
  const RegExpBytecode *getBytecodeEntry() const {
    return bytecode_.get();
  }
```

- Replace `initializeBytecode` (lines 136-137) with:

```cpp
  /// Take the reference \p bytecode and set syntaxFlags_ from its header.
  void setBytecode(llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode);

  /// \return the bytecode object if this regexp owns it exclusively and so
  /// reports its memory, otherwise null.
  const RegExpBytecode *exclusiveBytecode() const;
```

- Replace lines 151-152 (`bytecode_`, `bytecodeSize_`) with the
  following. Keep the comment at lines 145-148, which still applies: the
  pointer is 8 bytes.

```cpp
  /// The bytecode, owned exclusively or shared with a literal cache. One
  /// pointer, so the GC may relocate the cell by copying it.
  llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode_{};
```

`lib/VM/JSRegExp.cpp`:
- In the bytecode `initialize` (lines 101-128), change the parameter to
  `llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode`. Change the last
  line to `selfHandle->setBytecode(std::move(bytecode));`.
- Fast copy (lines 150-163). Replace the `initialize(...)` call at
  152-158 with:

```cpp
    // A Cached entry is shared. An Exclusive entry has exactly one holder,
    // which reports its memory, so it is copied.
    const RegExpBytecode *other = otherHandle->bytecode_.get();
    llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode =
        other->ownership() == RegExpBytecode::Ownership::Cached
        ? otherHandle->bytecode_
        : RegExpBytecode::create(
              other->bytecode(), RegExpBytecode::Ownership::Exclusive);
    initialize(selfHandle, runtime, lv.pattern, flags, std::move(bytecode));
```

  The entry pointer is read before `initialize`, which may allocate, and
  is not used after it. In this task every entry is `Exclusive`, so the
  shared branch is dead until Task 2.
- Dynamic path (line 202): replace the call with

```cpp
  initialize(
      selfHandle,
      runtime,
      pattern,
      flags,
      RegExpBytecode::create(bytecode, RegExpBytecode::Ownership::Exclusive));
```

- Replace `initializeBytecode` (lines 264-275) with:

```cpp
void JSRegExp::setBytecode(llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode) {
  assert(!bytecode_ && "a JSRegExp's bytecode is set only once");
  syntaxFlags_ = regex::SyntaxFlags::fromByte(bytecode->header().syntaxFlags);
  bytecode_ = std::move(bytecode);
}

const RegExpBytecode *JSRegExp::exclusiveBytecode() const {
  const RegExpBytecode *bytecode = bytecode_.get();
  return bytecode &&
          bytecode->ownership() == RegExpBytecode::Ownership::Exclusive
      ? bytecode
      : nullptr;
}
```

- `search`:
  - Line 329: keep the assert (`selfHandle->bytecode_` converts to bool).
  - Lines 351 and 359: `llvh::makeArrayRef(selfHandle->bytecode_,
    selfHandle->bytecodeSize_)` becomes `selfHandle->getBytecode()`. The
    entry is kept alive by the rooted `selfHandle` for the whole search.
- `~JSRegExp` (377-379) becomes `JSRegExp::~JSRegExp() = default;`.
  Releasing the pointer drops the reference.
- `_finalizeImpl` (381-387):

```cpp
void JSRegExp::_finalizeImpl(GCCell *cell, GC &gc) {
  JSRegExp *self = vmcast<JSRegExp>(cell);
  // A Cached entry's native ID belongs to its cache.
  if (const RegExpBytecode *bytecode = self->exclusiveBytecode())
    gc.getIDTracker().untrackNative(bytecode);
  self->~JSRegExp();
}
```

- `_mallocSizeImpl` (389-392):

```cpp
size_t JSRegExp::_mallocSizeImpl(GCCell *cell) {
  // A Cached entry is reported by its cache.
  const RegExpBytecode *bytecode = vmcast<JSRegExp>(cell)->exclusiveBytecode();
  return bytecode ? bytecode->mallocSize() : 0;
}
```

- `_snapshotAddEdgesImpl` (404-409) and `_snapshotAddNodesImpl`
  (414-424). Guard on `const RegExpBytecode *bytecode =
  self->exclusiveBytecode()` instead of `self->bytecode_`. Use
  `gc.getNativeID(bytecode)` and size `bytecode->mallocSize()` in place
  of `self->bytecode_` / `self->bytecodeSize_`.

`lib/VM/Interpreter-slowpaths.cpp` lines 618-619 become:

```cpp
  JSRegExp::initialize(
      lv.re,
      runtime,
      lv.pattern,
      lv.flags,
      RegExpBytecode::create(
          runtimeModule->getRegExpBytecodeFromRegExpID(regexpID),
          RegExpBytecode::Ownership::Exclusive));
```

Finally, grep the whole tree, excluding `cmake-build-*` and
`API/hermes_sandbox`, for `bytecodeSize_`, `initializeBytecode` and the
old `initialize` overload taking an `ArrayRef`. Callers per the spec:
- `Interpreter-slowpaths.cpp:613-619`;
- `StaticH.cpp:1795-1801`;
- `JSLib/RegExp.cpp:484` and `:667-669`;
- `JSRegExp.cpp`.

Every hit must be converted.

- [ ] **Step 6: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes \
  HermesVMRuntimeTests
$R/cmake-build-asan/unittests/VMRuntime/HermesVMRuntimeTests \
  --gtest_filter='RegExpBytecode*'
```

Expected: 4 tests pass.

- [ ] **Step 7: Prove each test can fail** (rebuild after each mutation
  and after each restore)
  - `CreateCopiesBytes`: make `mallocSize()` return `size_`. The
    `EXPECT_GE` fails.
  - `DynamicRegExpsAreExclusive` (regression guard for T3): create the
    dynamic path's entry with `Ownership::Cached`. The ownership and
    malloc-size expectations fail.
  - `SplitDoesNotClearStickyOfItsArgument` (regression guard for T5):
    1. In `search`, read sticky from
       `regex::SyntaxFlags::fromByte(selfHandle->bytecode_->header()
       .syntaxFlags)`.
    2. In `setSyntaxFlags`, also write `flags.toByte()` into the header
       through a `const_cast` (shared-state leakage).
    3. Turn the fast copy's condition into `true` for every entry, so the
       splitter shares.

    The test FAILS (`false`). This combination is what the test exists
    to catch.

- [ ] **Step 8: Full suite**

```bash
cmake --build $R/cmake-build-asan --target check-hermes
```

Expected: the baseline numbers, 0 failures.

- [ ] **Step 9: Commit**

```bash
git -C $R add include/hermes/VM/RegExpBytecode.h lib/VM/RegExpBytecode.cpp \
  lib/VM/CMakeLists.txt include/hermes/VM/JSRegExp.h lib/VM/JSRegExp.cpp \
  lib/VM/Interpreter-slowpaths.cpp unittests/VMRuntime/RegExpBytecodeTest.cpp \
  unittests/VMRuntime/CMakeLists.txt
git -C $R commit   # "VM: hold JSRegExp bytecode in a refcounted RegExpBytecode"
```

---

### Task 2: Per-module literal cache and fast-copy sharing

**Files:**
- Modify: `include/hermes/VM/RuntimeModule.h:278-280` and the private
  members (`:104-182`)
- Modify: `lib/VM/RuntimeModule.cpp:290-296`
- Modify: `lib/VM/Interpreter-slowpaths.cpp` (the Task 1 call)
- Modify: `unittests/VMRuntime/RegExpBytecodeTest.cpp`
- Create: `test/hermes/regexp-literal-shared.js`,
  `test/jit/regexp-literal-shared.js`

**Interfaces:**
- **Consumes:** Task 1's `RegExpBytecode` and `JSRegExp` API.
- **Produces:**
  `llvh::IntrusiveRefCntPtr<RegExpBytecode>
  RuntimeModule::getRegExpBytecode(uint32_t regExpID)`, and the private
  member
  `std::vector<llvh::IntrusiveRefCntPtr<RegExpBytecode>> regExpCache_`.
  Task 3 reads that member.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Write the tests**

Add these includes to `RegExpBytecodeTest.cpp`: `hermes/BCGen/HBC/BCProvider.h`,
`hermes/Support/MemoryBuffer.h`, `hermes/VM/RuntimeModule.h`,
`llvh/Support/MemoryBuffer.h` and `<memory>`. In the anonymous namespace,
after `runForArray`, add:

```cpp
/// Run \p code in a new module, compiled with \p flags, and \return its
/// completion value, which must be a function.
Handle<JSFunction> runForFunction(
    Runtime &runtime,
    const char *code,
    const hbc::CompileFlags &flags = hbc::CompileFlags{}) {
  CallResult<HermesValue> res = runtime.run(code, "test.js", flags);
  EXPECT_NE(ExecutionStatus::EXCEPTION, res.getStatus());
  return runtime.makeHandle(vmcast<JSFunction>(*res));
}

/// Call \p fn with no arguments and \return its result, which must be a
/// JSRegExp.
Handle<JSRegExp> callForRegExp(Runtime &runtime, Handle<JSFunction> fn) {
  CallResult<PseudoHandle<>> res =
      Callable::executeCall0(fn, runtime, Runtime::getUndefinedValue());
  EXPECT_NE(ExecutionStatus::EXCEPTION, res.getStatus());
  return runtime.makeHandle(vmcast<JSRegExp>(res->get()));
}

/// \return whether \p re matches the ASCII string \p str, searching from 0.
bool matches(Runtime &runtime, Handle<JSRegExp> re, const char *str) {
  CallResult<RegExpMatch> m = JSRegExp::search(
      re, runtime, StringPrimitive::createNoThrow(runtime, str), 0);
  EXPECT_NE(ExecutionStatus::EXCEPTION, m.getStatus());
  return !m->empty();
}
```

Then add the tests:

```cpp
TEST_F(RegExpBytecodeTest, LiteralIsSharedAcrossEvaluations) {
  Handle<JSArray> arr = runForArray(
      runtime,
      "(function () { function f() { return /a(b)c/g; }"
      " return [f(), f()]; })()");
  JSRegExp *r0 = regExpAt(runtime, arr, 0);
  JSRegExp *r1 = regExpAt(runtime, arr, 1);
  EXPECT_NE(r0, r1);
  EXPECT_EQ(r0->getBytecode().data(), r1->getBytecode().data());
  EXPECT_EQ(
      RegExpBytecode::Ownership::Cached, r0->getBytecodeEntry()->ownership());
  // The cache, not the JSRegExp, reports a Cached entry.
  EXPECT_EQ(0u, r0->getVT()->getMallocSize(r0));
}

TEST_F(RegExpBytecodeTest, LiteralCachesArePerModule) {
  // Each module's only literal has regexp ID 0.
  Handle<JSFunction> one =
      runForFunction(runtime, "(function () { return /one/; })");
  Handle<JSFunction> two =
      runForFunction(runtime, "(function () { return /two/; })");
  Handle<JSRegExp> r1 = callForRegExp(runtime, one);
  Handle<JSRegExp> r2 = callForRegExp(runtime, two);
  EXPECT_NE(r1->getBytecode().data(), r2->getBytecode().data());
  EXPECT_TRUE(matches(runtime, r1, "one"));
  EXPECT_FALSE(matches(runtime, r1, "two"));
  EXPECT_TRUE(matches(runtime, r2, "two"));
  EXPECT_FALSE(matches(runtime, r2, "one"));
}

TEST_F(RegExpBytecodeTest, LazilyCompiledLiteralGrowsTheCache) {
  hbc::CompileFlags flags;
  flags.lazy = true;
  flags.preemptiveFileCompilationThreshold = 0;
  flags.preemptiveFunctionCompilationThreshold = 0;
  Handle<JSFunction> f = runForFunction(
      runtime,
      "(function () { return function () { return /lazy(re)/g; }; })()",
      flags);
  RuntimeModule *rm = f->getRuntimeModule();
  // Precondition: the literal is not in the table until its function is
  // compiled, so its ID is past the table size at module load.
  ASSERT_EQ(0u, rm->getBytecode()->getRegExpTable().size());
  Handle<JSRegExp> r0 = callForRegExp(runtime, f);
  ASSERT_EQ(1u, rm->getBytecode()->getRegExpTable().size());
  Handle<JSRegExp> r1 = callForRegExp(runtime, f);
  EXPECT_EQ(r0->getBytecode().data(), r1->getBytecode().data());
  EXPECT_TRUE(matches(runtime, r1, "xlazyre"));
}

TEST_F(RegExpBytecodeTest, FastCopySharesOnlyCachedBytecode) {
  Handle<JSArray> arr =
      runForArray(runtime, "[/a(b)c/g, new RegExp('a(b)c', 'g')]");
  Handle<JSRegExp> lit = runtime.makeHandle(regExpAt(runtime, arr, 0));
  Handle<JSRegExp> dyn = runtime.makeHandle(regExpAt(runtime, arr, 1));
  ASSERT_EQ(
      RegExpBytecode::Ownership::Cached, lit->getBytecodeEntry()->ownership());
  ASSERT_EQ(
      RegExpBytecode::Ownership::Exclusive,
      dyn->getBytecodeEntry()->ownership());

  // The fast copy used by matchAll and split.
  auto copy = [&](Handle<JSRegExp> other, const char *flags) {
    Handle<JSRegExp> self = runtime.makeHandle(JSRegExp::create(runtime));
    EXPECT_EQ(
        ExecutionStatus::RETURNED,
        JSRegExp::initialize(
            self,
            runtime,
            other,
            StringPrimitive::createNoThrow(runtime, flags)));
    return self;
  };

  Handle<JSRegExp> litSame = copy(lit, "g");
  EXPECT_EQ(lit->getBytecode().data(), litSame->getBytecode().data());

  Handle<JSRegExp> litOther = copy(lit, "gy");
  EXPECT_NE(lit->getBytecode().data(), litOther->getBytecode().data());
  EXPECT_EQ(
      RegExpBytecode::Ownership::Exclusive,
      litOther->getBytecodeEntry()->ownership());
  EXPECT_TRUE(JSRegExp::getSyntaxFlags(*litOther).sticky);

  Handle<JSRegExp> dynSame = copy(dyn, "g");
  EXPECT_NE(dyn->getBytecode().data(), dynSame->getBytecode().data());
  EXPECT_TRUE(dyn->getBytecode().equals(dynSame->getBytecode()));
  EXPECT_EQ(
      RegExpBytecode::Ownership::Exclusive,
      dynSame->getBytecodeEntry()->ownership());
}

TEST_F(RegExpBytecodeTest, LiteralOutlivesItsModule) {
  // runtime.run() would load the module as persistent, and the runtime
  // would keep its storage. Instead, run from an owning buffer, held only
  // by the provider, with persistent = false.
  std::weak_ptr<hbc::BCProvider> weakProvider;
  Handle<JSRegExp> re;
  {
    std::vector<uint8_t> bc =
        bytecodeForSource("(function () { return /a(b)c/g; })()");
    auto buffer = std::make_unique<OwnedMemoryBuffer>(
        llvh::MemoryBuffer::getMemBufferCopy(
            llvh::StringRef((const char *)bc.data(), bc.size())));
    std::shared_ptr<hbc::BCProvider> provider =
        hbc::BCProviderFromBuffer::createBCProviderFromBuffer(std::move(buffer))
            .first;
    ASSERT_TRUE(provider);
    weakProvider = provider;
    RuntimeModuleFlags rmFlags;
    rmFlags.persistent = false;
    CallResult<HermesValue> res = runtime.runBytecode(
        std::move(provider),
        rmFlags,
        "test.js",
        Runtime::makeNullHandle<Environment>());
    ASSERT_FALSE(isException(res));
    re = runtime.makeHandle(vmcast<JSRegExp>(*res));
  }
  for (int i = 0; i < 3 && !weakProvider.expired(); ++i)
    runtime.collect("test");
  // Precondition: the module and its storage are gone.
  ASSERT_TRUE(weakProvider.expired());
  EXPECT_TRUE(matches(runtime, re, "xabcx"));
}
```

`re` is assigned inside the block. If `Handle<JSRegExp> re;` does not
compile (no default constructor), declare
`MutableHandle<JSRegExp> re{runtime};` and assign with `re = ...`. Pass
it to `matches` as `re` (it converts to `Handle<JSRegExp>`).

Create `test/hermes/regexp-literal-shared.js`:

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s

// Regexps from one literal share bytecode but nothing else: lastIndex,
// match progress and named groups stay per object.

function main() {
  function mk() { return /a(?<n>\d)/g; }
  var a = mk(), b = mk();
  print(a === b);
// CHECK: false
  print(a.exec("a1a2")[0], a.lastIndex, b.lastIndex);
// CHECK-NEXT: a1 2 0
  print(b.exec("xa3")[0], b.lastIndex, a.lastIndex);
// CHECK-NEXT: a3 3 2
  print(a.exec("a1a2").groups.n, a.lastIndex);
// CHECK-NEXT: 2 4
  print(mk().test("a5"), mk().lastIndex);
// CHECK-NEXT: true 0
  var s = /b/y;
  print("abc".split(s).length, s.sticky, s.lastIndex);
// CHECK-NEXT: 2 true 0
  // split() cleared sticky on its private copy only.
  print(s.exec("ab") === null);
// CHECK-NEXT: true
}
main();
```

Create `test/jit/regexp-literal-shared.js`:

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xjit-crash-on-error %S/../hermes/regexp-literal-shared.js | %FileCheck --match-full-lines %S/../hermes/regexp-literal-shared.js
// REQUIRES: jit

// The JIT reaches the same Interpreter::createRegExp; run the shared
// literal test through it.
```

If `-lazy` is not a `hermes` option (`$R/cmake-build-asan/bin/hermes
--help | grep -i lazy`), use the option the help text names for lazy
compilation, and say so in the report.

- [ ] **Step 3: See them fail**

Build and run `--gtest_filter='RegExpBytecode*'` on `cmake-build-asan`.

Expected:
- `LiteralIsSharedAcrossEvaluations`, `LazilyCompiledLiteralGrowsTheCache`
  and `FastCopySharesOnlyCachedBytecode` FAIL: there are distinct
  copies, and the ownership is `Exclusive`.
- `LiteralCachesArePerModule` and `LiteralOutlivesItsModule` pass. They
  are regression guards.
- The two lit tests pass, also guards:

  ```bash
  LIT_FILTER='regexp-literal-shared' cmake --build $R/cmake-build-asan \
    --target check-hermes
  ```

- [ ] **Step 4: Implement the cache**

`include/hermes/VM/RuntimeModule.h`:
- Add `#include "hermes/VM/RegExpBytecode.h"` with the other VM
  includes.
- After `moduleExports_` (line 181), add:

```cpp
  /// The shared bytecode of each regexp literal, indexed by regexp ID and
  /// created on first use. Grows with the regexp table under lazy
  /// compilation. Mutator thread only.
  std::vector<llvh::IntrusiveRefCntPtr<RegExpBytecode>> regExpCache_{};
```

- After `getRegExpBytecodeFromRegExpID` (line 280), add:

```cpp
  /// \return the shared bytecode of regexp literal \p regExpID, creating
  /// it from the module's bytecode on first use. Mutator thread only.
  llvh::IntrusiveRefCntPtr<RegExpBytecode> getRegExpBytecode(
      uint32_t regExpID);
```

`lib/VM/RuntimeModule.cpp`, after `getRegExpBytecodeFromRegExpID`
(line 296):

```cpp
llvh::IntrusiveRefCntPtr<RegExpBytecode> RuntimeModule::getRegExpBytecode(
    uint32_t regExpID) {
  if (LLVM_UNLIKELY(regExpID >= regExpCache_.size())) {
    // Lazy compilation adds regexps to the table as functions compile.
    size_t tableSize = bcProvider_->getRegExpTable().size();
    assert(regExpID < tableSize && "Invalid regexp id");
    regExpCache_.resize(tableSize);
  }
  llvh::IntrusiveRefCntPtr<RegExpBytecode> &entry = regExpCache_[regExpID];
  if (LLVM_UNLIKELY(!entry)) {
    entry = RegExpBytecode::create(
        getRegExpBytecodeFromRegExpID(regExpID),
        RegExpBytecode::Ownership::Cached);
  }
  return entry;
}
```

`lib/VM/Interpreter-slowpaths.cpp`: the `RegExpBytecode::create(...)`
argument from Task 1 becomes
`runtimeModule->getRegExpBytecode(regexpID)`.

- [ ] **Step 5: Build and run**

On `cmake-build-asan`, run:
- `--gtest_filter='RegExpBytecode*'`: all pass;
- `LIT_FILTER='regexp'`: all pass.

- [ ] **Step 6: Moving GC (Review Focus 3)**

```bash
cmake --build $R/cmake-build-hsan --target hermes hermesc shermes \
  HermesVMRuntimeTests
$R/cmake-build-hsan/unittests/VMRuntime/HermesVMRuntimeTests \
  --gtest_filter='RegExpBytecode*'
LIT_FILTER='regexp' cmake --build $R/cmake-build-hsan --target check-hermes
```

Expected: all pass, with no ASan report.

- [ ] **Step 7: Prove each test can fail** (on `cmake-build-asan`;
  rebuild after each change)
  - `LiteralIsSharedAcrossEvaluations`: in `getRegExpBytecode`, return a
    new `Cached` entry without storing it. FAILS (distinct data).
  - `LazilyCompiledLiteralGrowsTheCache`: delete the resize branch.
    FAILS: an ASan or assertion failure indexing past the vector.
  - `FastCopySharesOnlyCachedBytecode`: in the fast copy, always take the
    `create` branch. FAILS at `litSame`.
  - `LiteralCachesArePerModule` (guard): make `regExpCache_` a
    function-local `static` vector inside `getRegExpBytecode`. It is then
    shared by every module, and the test FAILS (`r2` matches `one`).
  - `LiteralOutlivesItsModule` (guard): temporarily build a borrowing
    variant.
    1. Add `const uint8_t *borrowed_ = nullptr;` to `RegExpBytecode`.
    2. Make `bytecode()` and `header()` read from `borrowed_` when it is
       set.
    3. Add a temporary factory `createBorrowed(llvh::ArrayRef<uint8_t>)`.
       It allocates only the header, keeps the real `size_`, sets
       `borrowed_ = bytecode.data()`, and uses `Ownership::Cached`. Do
       not route it through `create` with an empty array: `create`'s
       header-size assertion would fire first.
    4. Have `RuntimeModule::getRegExpBytecode` use
       `createBorrowed(getRegExpBytecodeFromRegExpID(regExpID))`.

    The weak-pointer precondition still passes, and `matches` then FAILS
    under ASan (heap-use-after-free on the provider's freed buffer).
    Restore, rebuild, green.
  - The lit tests (guards). Use Task 1's shared-state mutation:
    - `search` reads sticky from the shared header;
    - `setSyntaxFlags` writes the header through a `const_cast`.

    After the split, `s.exec("ab")` no longer returns `null`, so the last
    `CHECK-NEXT: true` FAILS on every RUN line, the JIT one included.
    `lastIndex` and `groups` are JS properties of each object and cannot
    leak through the bytecode. The test pins them only as behaviour.

- [ ] **Step 8: Full suite** on `cmake-build-asan`: baseline numbers plus
  the two new lit tests, 0 failures.

- [ ] **Step 9: Commit**

```bash
git -C $R add include/hermes/VM/RuntimeModule.h lib/VM/RuntimeModule.cpp \
  lib/VM/Interpreter-slowpaths.cpp unittests/VMRuntime/RegExpBytecodeTest.cpp \
  test/hermes/regexp-literal-shared.js test/jit/regexp-literal-shared.js
git -C $R commit   # "VM: share regexp literal bytecode per RuntimeModule"
```

---

### Task 3: Memory accounting and heap snapshots for cached entries

**Files:**
- Modify: `include/hermes/VM/RuntimeModule.h` (next to
  `additionalMemorySize`, `:411-413`)
- Modify: `lib/VM/RuntimeModule.cpp:66-76` (destructor), `:412-417`,
  `:454-499` (snapshot)
- Modify: `lib/VM/Domain.cpp:62-71`
- Modify: `unittests/VMRuntime/HeapSnapshotTest.cpp`

**Interfaces:**
- **Consumes:** `regExpCache_` from Task 2, and
  `JSRegExp::getBytecodeEntry()`.
- **Produces:** `size_t RuntimeModule::regExpCacheMemorySize() const`.
- **Spec resolution:** T8 lives in `HeapSnapshotTest.cpp`, to reuse its
  snapshot parsers (`Node`, `Edge`, `TAKE_SNAPSHOT`,
  `FIND_NODE_AND_EDGES_FOR_ID`), rather than in `RegExpBytecodeTest.cpp`.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Write the test**

In `unittests/VMRuntime/HeapSnapshotTest.cpp`:
- add the includes `hermes/VM/Callable.h`, `hermes/VM/JSRegExp.h`,
  `hermes/VM/RegExpBytecode.h` and `hermes/VM/RuntimeModule.h`, where
  they are missing;
- after `FIND_NODE_AND_EDGES_FOR_ID` (line 364), add:

```cpp
/// \return every node named \p name.
static std::vector<Node> findNodesNamed(
    const std::string &name,
    const JSONArray &nodes,
    const JSONArray &strings) {
  std::vector<Node> found;
  for (auto it = nodes.begin(), e = nodes.end(); it != e;
       it += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT) {
    Node node = Node::parse(it, strings);
    if (node.name == name)
      found.push_back(std::move(node));
  }
  return found;
}
```

After `TEST_F(HeapSnapshotRuntimeTest, FunctionLocationForLazyCode)`,
add:

```cpp
TEST_F(HeapSnapshotRuntimeTest, RegExpBytecodeReportedByItsModule) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};

  CallResult<HermesValue> res = runtime.run(
      "(function () { return function () { return /a(b)c/g; }; })()",
      "file:///fake.js",
      hbc::CompileFlags{});
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> f = runtime.makeHandle(vmcast<JSFunction>(*res));
  RuntimeModule *rm = f->getRuntimeModule();
  const size_t numRegExps = rm->getBytecode()->getRegExpTable().size();
  ASSERT_EQ(1u, numRegExps);
  // Precondition: nothing is cached yet.
  ASSERT_EQ(0u, rm->regExpCacheMemorySize());
  const auto rmID = runtime.getHeap().getNativeID(rm);

  size_t rmSizeBefore;
  {
    JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory, false);
    ASSERT_TRUE(root != nullptr);
    const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
    const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));
    // Precondition: no other regexp bytecode is reported.
    ASSERT_EQ(0u, findNodesNamed("RegExpBytecode", nodes, strings).size());
    rmSizeBefore = FIND_NODE_FOR_ID(rmID, nodes, strings).selfSize;
  }

  auto call = [&]() {
    CallResult<PseudoHandle<>> r =
        Callable::executeCall0(f, runtime, Runtime::getUndefinedValue());
    EXPECT_FALSE(isException(r));
    return runtime.makeHandle(vmcast<JSRegExp>(r->get()));
  };

  Handle<JSRegExp> r0 = call();
  const RegExpBytecode *entry = r0->getBytecodeEntry();
  ASSERT_EQ(RegExpBytecode::Ownership::Cached, entry->ownership());
  EXPECT_EQ(0u, r0->getVT()->getMallocSize(r0.get()));
  const size_t sizeAfterFirst = rm->regExpCacheMemorySize();

  Handle<JSRegExp> r1 = call();
  EXPECT_EQ(entry, r1->getBytecodeEntry());
  EXPECT_EQ(0u, r1->getVT()->getMallocSize(r1.get()));
  // The second evaluation adds nothing.
  EXPECT_EQ(sizeAfterFirst, rm->regExpCacheMemorySize());

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory, false);
  ASSERT_TRUE(root != nullptr);
  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // Exactly one node for the shared bytecode, sized by its allocation.
  const auto entryID = runtime.getHeap().getNativeID(entry);
  std::vector<Node> bytecodeNodes =
      findNodesNamed("RegExpBytecode", nodes, strings);
  ASSERT_EQ(1u, bytecodeNodes.size());
  EXPECT_EQ(entryID, bytecodeNodes[0].id);
  EXPECT_EQ(entry->mallocSize(), bytecodeNodes[0].selfSize);

  // The module node keeps its size and points at its cache vector. Every
  // module emits a vector node (the runtime's internal module too), so the
  // vector is found through this module's edge.
  auto rmNodeAndEdges =
      FIND_NODE_AND_EDGES_FOR_ID(rmID, nodes, edges, strings);
  EXPECT_EQ(rmSizeBefore, rmNodeAndEdges.first.selfSize);
  auto cacheEdgeIt = std::find_if(
      rmNodeAndEdges.second.begin(),
      rmNodeAndEdges.second.end(),
      [](const Edge &edge) {
        return edge.type == HeapSnapshot::EdgeType::Internal &&
            edge.name == "regExpCache";
      });
  ASSERT_NE(rmNodeAndEdges.second.end(), cacheEdgeIt);

  // The vector node: named, sized by its capacity, with an indexed edge
  // to the entry.
  auto vectorNodeAndEdges =
      FIND_NODE_AND_EDGES_FOR_ID(cacheEdgeIt->toNode, nodes, edges, strings);
  EXPECT_EQ("std::vector<RegExpBytecode *>", vectorNodeAndEdges.first.name);
  const size_t slotSize = sizeof(llvh::IntrusiveRefCntPtr<RegExpBytecode>);
  const size_t vectorBytes = vectorNodeAndEdges.first.selfSize;
  EXPECT_GE(vectorBytes, numRegExps * slotSize);
  EXPECT_EQ(0u, vectorBytes % slotSize);
  EXPECT_EQ(
      std::vector<Edge>{Edge(HeapSnapshot::EdgeType::Element, 0, entryID)},
      vectorNodeAndEdges.second);

  // The malloc estimate agrees with the snapshot: the vector plus the
  // entry.
  EXPECT_EQ(vectorBytes + entry->mallocSize(), sizeAfterFirst);
}
```

When the fixture tears the runtime down, it asserts that every native ID
was untracked (`lib/VM/Runtime.cpp:522-528`). The test's
`getNativeID(entry)` and the snapshot's calls make the module
responsible for untracking.

- [ ] **Step 3: See it fail**

Build and run `--gtest_filter='HeapSnapshotRuntimeTest.RegExp*'`.

Expected: a compile error, `regExpCacheMemorySize` is not a member.

- [ ] **Step 4: Implement**

`RuntimeModule.h`, after `additionalMemorySize()` (line 413):

```cpp
  /// \return the malloc'd bytes held by the regexp literal cache: its
  /// vector and the entries in it. Reported by the Domain separately from
  /// additionalMemorySize(), because the entries have their own snapshot
  /// nodes.
  size_t regExpCacheMemorySize() const;
```

`RuntimeModule.cpp`:
- After `additionalMemorySize()` (line 417):

```cpp
size_t RuntimeModule::regExpCacheMemorySize() const {
  size_t size = regExpCache_.capacity() * sizeof(regExpCache_[0]);
  for (const auto &entry : regExpCache_) {
    if (entry)
      size += entry->mallocSize();
  }
  return size;
}
```

- In `~RuntimeModule`, after line 75:

```cpp
  // Entries may outlive the module, held by JSRegExps, but only the module
  // reports them.
  for (const auto &entry : regExpCache_) {
    if (entry)
      runtime_.getHeap().getIDTracker().untrackNative(entry.get());
  }
  runtime_.getHeap().getIDTracker().untrackNative(&regExpCache_);
```

- In `snapshotAddNodes`, after the `functionMap_` node (line 490):

```cpp

  // Create a native node for each cached regexp bytecode entry.
  for (const auto &entry : regExpCache_) {
    if (entry) {
      snap.beginNode();
      snap.endNode(
          HeapSnapshot::NodeType::Native,
          "RegExpBytecode",
          gc.getNativeID(entry.get()),
          entry->mallocSize(),
          0);
    }
  }

  // Create a node for regExpCache_, with an edge to each entry.
  snap.beginNode();
  for (int i = 0, e = regExpCache_.size(); i < e; i++) {
    if (const RegExpBytecode *entry = regExpCache_[i].get()) {
      snap.addIndexedEdge(
          HeapSnapshot::EdgeType::Element, i, gc.getNativeID(entry));
    }
  }
  snap.endNode(
      HeapSnapshot::NodeType::Native,
      "std::vector<RegExpBytecode *>",
      gc.getNativeID(&regExpCache_),
      regExpCache_.capacity() * sizeof(regExpCache_[0]),
      0);
```

- In `snapshotAddEdges`, after the `functionMap` edge:

```cpp
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Internal,
      "regExpCache",
      gc.getNativeID(&regExpCache_));
```

`lib/VM/Domain.cpp:66` becomes:

```cpp
    rmSize += sizeof(RuntimeModule) + rm->additionalMemorySize() +
        rm->regExpCacheMemorySize();
```

`additionalMemorySize()` and the `RuntimeModule` node size
(`Domain.cpp:92`) stay unchanged.

- [ ] **Step 5: Build and run**

Run `--gtest_filter='HeapSnapshot*:RegExpBytecode*'` on `cmake-build-asan`.

Expected: all pass.

- [ ] **Step 6: Prove it can fail**
  - In `JSRegExp::_mallocSizeImpl`, return `mallocSize()` for every
    entry. FAILS (`getMallocSize` is not 0).
  - Remove the per-entry node loop. FAILS (no `RegExpBytecode` node).
  - Remove the per-entry `untrackNative` loop from `~RuntimeModule`. The
    test binary FAILS at fixture teardown with "A pointer is left in the
    ID tracker".
  - Remove the `regExpCache` edge. FAILS (`cacheEdge` not found).

- [ ] **Step 7: Full suite** on `cmake-build-asan`: 0 failures.

- [ ] **Step 8: Commit**

```bash
git -C $R add include/hermes/VM/RuntimeModule.h lib/VM/RuntimeModule.cpp \
  lib/VM/Domain.cpp unittests/VMRuntime/HeapSnapshotTest.cpp
git -C $R commit   # "VM: report cached regexp bytecode from its RuntimeModule"
```

---

### Task 4: Static Hermes ahead-of-time regexp bytecode

The ABI, the runtime and the compiler change together: generated C
compiled against the new header must use the new call.

**Files:**
- Modify: `include/hermes/VM/static_h.h:31-38` (new entry type), `:133-142`
  (`SHUnit` fields) and `:891-892` (declaration)
- Modify: `lib/VM/SHUnitExt.h`, `lib/VM/StaticHUnit.cpp:117`, `:206-210`
- Modify: `lib/VM/StaticH.cpp:1788-1810`
- Modify: `lib/BCGen/SH/SH.cpp:177-246`, `:429-447`, `:1798-1826`,
  `:3169-3216`
- Modify: `unittests/VMRuntime/RegExpBytecodeTest.cpp`
- Create: `test/shermes/regexp-aot.js`

**Interfaces:**
- **Consumes:** `RegExpBytecode` (Task 1);
  `CompiledRegExp &Context::getCompiledRegExp(UniqueString *, UniqueString *)`
  (`include/hermes/AST/Context.h:351-357`); and `UniquingRegExpTable`
  (`include/hermes/Regex/RegexSerialization.h:90-162`).
- **Produces:**
  - `explicit SHUnitExt::SHUnitExt(uint32_t numRegExps)`;
  - `SHUnitExt::regExpCache`;
  - `hermes::vm::sh_unit_get_regexp_bytecode(SHUnit *, uint32_t)`.
- **Spec resolution:** the spec sizes the cache "when the unit is
  attached". This is done through the `SHUnitExt` constructor argument,
  and lookup goes through `sh_unit_get_regexp_bytecode`, mirroring
  `RuntimeModule::getRegExpBytecode`.
- **Spec resolution:** the new `SHUnit` fields go right after
  `object_literal_class_cache`, with the other literal data. The
  generated initializer uses designated initializers, so the position
  does not matter to it.

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Write the tests**

Unit test (T7b). Add these includes to `RegExpBytecodeTest.cpp`:
- `"../../lib/VM/SHUnitExt.h"`, following the precedent of
  `unittests/VMRuntime/JitChainWalkTest.cpp:12`;
- `"hermes/VM/StaticHUtils.h"`;
- `"hermes/VM/static_h.h"`.

Then add the test:

```cpp
TEST_F(RegExpBytecodeTest, SHUnitSharesPrecompiledBytecode) {
  auto compiled = CompiledRegExp::tryCompile("a(b)c", "g");
  ASSERT_TRUE(compiled.hasValue());
  llvh::ArrayRef<uint8_t> bc = compiled->getBytecode();
  const SHRegExpTableEntry table[] = {{0, (uint32_t)bc.size()}};

  // Only the regexp fields matter to _sh_ljs_create_regexp.
  SHUnit unit{};
  unit.regexp_storage = bc.data();
  unit.regexp_table = table;
  unit.num_regexps = 1;
  unit.runtime_ext = new SHUnitExt{unit.num_regexps};

  auto symbol = [&](const char *str) {
    CallResult<Handle<SymbolID>> sym =
        runtime.getIdentifierTable().getSymbolHandle(
            runtime, createASCIIRef(str));
    EXPECT_NE(ExecutionStatus::EXCEPTION, sym.getStatus());
    return sym->get().unsafeGetRaw();
  };
  const SHSymbolID pattern = symbol("a(b)c");
  const SHSymbolID flags = symbol("g");

  const size_t sizeBefore = sh_unit_additional_memory_size(&unit);
  auto create = [&]() {
    SHLegacyValue v = _sh_ljs_create_regexp(
        getSHRuntime(runtime), &unit, pattern, flags, 0);
    return runtime.makeHandle(vmcast<JSRegExp>(HermesValue::fromRaw(v.raw)));
  };
  Handle<JSRegExp> r0 = create();
  Handle<JSRegExp> r1 = create();
  EXPECT_EQ(r0->getBytecode().data(), r1->getBytecode().data());
  EXPECT_TRUE(bc.equals(r0->getBytecode()));
  EXPECT_EQ(
      RegExpBytecode::Ownership::Cached, r0->getBytecodeEntry()->ownership());
  // The vector was sized when the unit was attached; only the entry is
  // new.
  EXPECT_EQ(
      sizeBefore + r0->getBytecodeEntry()->mallocSize(),
      sh_unit_additional_memory_size(&unit));
  EXPECT_TRUE(matches(runtime, r1, "xabc"));

  // The regexps keep the entry alive after the unit's cache is gone.
  delete unit.runtime_ext;
  EXPECT_TRUE(matches(runtime, r0, "abc"));
}
```

The pattern symbol and the regexp handles are rooted: the symbols by the
identifier table and the `Handle`s, the regexps by the fixture's
`GCScope`. `createASCIIRef` is in `include/hermes/VM/StringRefUtils.h:32`.
Include that header if it is not reached transitively.

Add `#include <algorithm>` to `HeapSnapshotTest.cpp` in Task 3 if it is
missing (`std::find`).

Lit test (T7a, T7c, Review Focus 5). Create `test/shermes/regexp-aot.js`:

```js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -emit-c -o - %s | %FileCheck %s
// RUN: %shermes -O -emit-c -o - %s | %FileCheck %s
// RUN: %shermes -exec %s | %FileCheck --check-prefix=EXEC --match-full-lines %s

// SH compiles regexp literals ahead of time: each distinct literal gets one
// entry in s_regexp_table, and _sh_ljs_create_regexp takes its index.

function main() {
  function mk() { return /(?<y>\d{4})-(?<m>\d\d)/gy; }
  function same() { return /(?<y>\d{4})-(?<m>\d\d)/gy; }
  function uni() { return /é|\u{1F600}/u; }
  var prev = null;
  for (var i = 0; i < 3; ++i) {
    var r = i == 1 ? same() : mk();
    print(r === prev, r.flags, r.sticky, r.lastIndex);
    var m = r.exec("2024-10 rest");
    print(m.groups.y, m.groups.m, r.lastIndex);
    prev = r;
  }
  var u = uni();
  print(u.source, u.test("\u{1F600}"), u.test("é"), u.test("e"));
}
main();

// CHECK: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 0);
// CHECK: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 0);
// CHECK: _sh_ljs_create_regexp(shr, shUnit, get_symbols(shUnit)[{{[0-9]+}}], get_symbols(shUnit)[{{[0-9]+}}], 1);
// CHECK: static unsigned char s_regexp_storage[{{[0-9]+}}] = {
// CHECK-NEXT: static const SHRegExpTableEntry s_regexp_table[] = {
// CHECK-NEXT:   { .offset = 0, .length = [[LEN:[0-9]+]] },
// CHECK-NEXT:   { .offset = [[LEN]], .length = {{[0-9]+}} },
// CHECK-NEXT: };
// CHECK: .regexp_storage = s_regexp_storage, .regexp_table = s_regexp_table, .num_regexps = 2,

// EXEC: false gy true 0
// EXEC-NEXT: 2024 10 7
// EXEC-NEXT: false gy true 0
// EXEC-NEXT: 2024 10 7
// EXEC-NEXT: false gy true 0
// EXEC-NEXT: 2024 10 7
// EXEC-NEXT: é|\u{1F600} true true false
```

The three CHECK call lines assume that `mk`, `same` and `uni` are emitted
in source order, and that the optimizer neither inlines nor reorders
them.
- If `-O` inlines or reorders them, keep the `-O` RUN line. Give it a
  separate `--check-prefix` whose call checks still require two calls
  with the same id and one with a different id. For example, use
  `CHECK-DAG` lines `, 0);` (twice) and `, 1);`.
- Keep the table checks identical for both RUN lines.

Report any such adjustment.

- [ ] **Step 3: See them fail**

```bash
cmake --build $R/cmake-build-asan --target HermesVMRuntimeTests shermes
LIT_FILTER='regexp-aot' cmake --build $R/cmake-build-asan --target check-hermes
```

Expected:
- The unit test does not compile: no `SHRegExpTableEntry`, and the
  `SHUnitExt` constructor takes no argument.
- The two `-emit-c` RUN lines FAIL: no `s_regexp_table`, and the old call
  has three arguments.
- The `-exec` part passes. It is a regression guard.

- [ ] **Step 4: ABI** (`include/hermes/VM/static_h.h`)

After `SHShapeTableEntry` (line 38):

```c

/// Locates the bytecode of one regexp literal in SHUnit::regexp_storage.
typedef struct SHRegExpTableEntry {
  /// Offset of the bytecode in regexp_storage, in bytes.
  uint32_t offset;
  /// Length of the bytecode, in bytes.
  uint32_t length;
} SHRegExpTableEntry;
```

In `SHUnit`, after `object_literal_class_cache` (line 142):

```c

  /// Concatenated bytecode of the unit's regexp literals, compiled ahead of
  /// time.
  const unsigned char *regexp_storage;
  /// One entry per regexp literal, indexed by the id passed to
  /// _sh_ljs_create_regexp.
  const SHRegExpTableEntry *regexp_table;
  /// Number of entries in regexp_table.
  uint32_t num_regexps;
```

Replace the declaration at lines 891-892 with:

```c
/// Create a regexp from literal \p regexp_id of \p unit, whose pattern and
/// flags are the symbols \p pattern and \p flags. The bytecode was compiled
/// ahead of time and is shared by every evaluation of the literal.
SHERMES_EXPORT SHLegacyValue _sh_ljs_create_regexp(
    SHRuntime *shr,
    SHUnit *unit,
    SHSymbolID pattern,
    SHSymbolID flags,
    uint32_t regexp_id);
```

- [ ] **Step 5: Runtime**

`lib/VM/SHUnitExt.h`:
- Add the includes `"hermes/VM/RegExpBytecode.h"` and `<vector>`.
- In `SHUnitExt`, after `addCacheEntries`, add the member below, and
  replace the constructor:

```cpp
  /// The shared bytecode of each regexp literal, indexed like
  /// SHUnit::regexp_table and created on first use. Mutator thread only.
  std::vector<llvh::IntrusiveRefCntPtr<hermes::vm::RegExpBytecode>>
      regExpCache;

  /// Create the data for a unit with \p numRegExps regexp literals.
  explicit SHUnitExt(uint32_t numRegExps) : regExpCache(numRegExps) {
    // Reserve entry 0.
    addCacheEntries.emplace_back();
  }
```

- After `sh_unit_get_add_cache_entry`, add:

```cpp
/// \return the shared bytecode of regexp literal \p regExpID of \p unit,
/// creating it from the unit's precompiled bytecode on first use. Mutator
/// thread only.
llvh::IntrusiveRefCntPtr<RegExpBytecode> sh_unit_get_regexp_bytecode(
    SHUnit *unit,
    uint32_t regExpID);
```

`lib/VM/StaticHUnit.cpp`:
- Line 117 becomes `unit->runtime_ext = new SHUnitExt{unit->num_regexps};`.
  Grep `lib/` and `API/` (excluding `API/hermes_sandbox`) for any other
  `new SHUnitExt` and convert it the same way.
- `sh_unit_additional_memory_size` (lines 206-210) becomes:

```cpp
size_t hermes::vm::sh_unit_additional_memory_size(const SHUnit *unit) {
  const SHUnitExt *ext = unit->runtime_ext;
  size_t regExpSize = ext->regExpCache.capacity() * sizeof(ext->regExpCache[0]);
  for (const auto &entry : ext->regExpCache) {
    if (entry)
      regExpSize += entry->mallocSize();
  }
  return sizeof(unit->runtime_ext) + ext->templateMap.getMemorySize() +
      ext->addCacheEntries.capacity_in_bytes() + regExpSize;
}
```

- Add, after it:

```cpp
llvh::IntrusiveRefCntPtr<RegExpBytecode> hermes::vm::sh_unit_get_regexp_bytecode(
    SHUnit *unit,
    uint32_t regExpID) {
  auto &cache = unit->runtime_ext->regExpCache;
  assert(regExpID < cache.size() && "Invalid regexp id");
  llvh::IntrusiveRefCntPtr<RegExpBytecode> &entry = cache[regExpID];
  if (LLVM_UNLIKELY(!entry)) {
    const SHRegExpTableEntry &te = unit->regexp_table[regExpID];
    entry = RegExpBytecode::create(
        {unit->regexp_storage + te.offset, te.length},
        RegExpBytecode::Ownership::Cached);
  }
  return entry;
}
```

`lib/VM/StaticH.cpp`: replace `_sh_ljs_create_regexp` (lines 1788-1810)
with the following. Nothing in it can throw, so the `LocalsRAII` is
safe. Its structure is `Interpreter::createRegExp`'s
(`Interpreter-slowpaths.cpp:597-622`).

```cpp
LLVM_ATTRIBUTE_NOINLINE
extern "C" SHLegacyValue _sh_ljs_create_regexp(
    SHRuntime *shr,
    SHUnit *unit,
    SHSymbolID pattern,
    SHSymbolID flags,
    uint32_t regexp_id) {
  Runtime &runtime = getRuntime(shr);
  GCScopeMarkerRAII marker{runtime};
  struct : public Locals {
    PinnedValue<JSRegExp> re;
    PinnedValue<StringPrimitive> pattern;
    PinnedValue<StringPrimitive> flags;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.re = JSRegExp::create(runtime);
  lv.pattern =
      runtime.getStringPrimFromSymbolID(SymbolID::unsafeCreate(pattern));
  lv.flags = runtime.getStringPrimFromSymbolID(SymbolID::unsafeCreate(flags));
  // The bytecode was compiled ahead of time; nothing is parsed here.
  JSRegExp::initialize(
      lv.re,
      runtime,
      lv.pattern,
      lv.flags,
      sh_unit_get_regexp_bytecode(unit, regexp_id));
  return lv.re.getHermesValue();
}
```

`StaticH.cpp` already includes `SHUnitExt.h` (line 8). If
`lv.re.getHermesValue()` does not convert to `SHLegacyValue`, write
`HermesValue res = lv.re.getHermesValue(); return res;`, which is what
the old code's `return *cr;` relied on.

- [ ] **Step 6: Compiler** (`lib/BCGen/SH/SH.cpp`)

- Add `#include "hermes/Regex/RegexSerialization.h"` to the includes.
- Move `generateBuffer` out of `SHLiteralBuffers` (lines 235-245). Put it
  before the class, as a free function, and delete the `private:`
  section:

```cpp
/// Emit \p buf as a C array of unsigned char named \p name.
static void generateBuffer(
    llvh::raw_ostream &os,
    llvh::StringRef name,
    llvh::ArrayRef<unsigned char> buf) {
  os << "static unsigned char " << name << "[" << buf.size() << "] = {";
  for (unsigned char c : buf) {
    os << (int)c << ",";
  }
  os << "};\n";
}

/// Emit the regexp bytecode storage and the table that locates each
/// regexp in it.
static void generateRegExpTable(
    llvh::raw_ostream &os,
    const UniquingRegExpTable &table) {
  generateBuffer(os, "s_regexp_storage", table.getBytecodeBuffer());
  os << "static const SHRegExpTableEntry s_regexp_table[] = {\n";
  for (const RegExpTableEntry &entry : table.getEntryList()) {
    os.indent(2);
    os << "{ .offset = " << entry.offset << ", .length = " << entry.length
       << " },\n";
  }
  os << "};\n";
}
```

  An empty table emits `[] = {\n};`, exactly as `s_obj_shape_table`
  already does for a module with no object literals (lines 226-232).
- In `ModuleGen` (lines 429-447), after `nativeFunctionTable`, add:

```cpp

  /// Uniqued table of the module's regexp literals and their bytecode.
  UniquingRegExpTable regExpTable{};
```

- Replace `generateCreateRegExpInst` (lines 1798-1826) with:

```cpp
  void generateCreateRegExpInst(CreateRegExpInst &inst) {
    os_.indent(2);

    uint32_t patternStrID =
        moduleGen_.stringTable.add(inst.getPattern()->getValue().str());
    uint32_t flagsStrID =
        moduleGen_.stringTable.add(inst.getFlags()->getValue().str());
    // Semantic resolution compiled every regexp literal into the Context.
    CompiledRegExp &regexp = F_.getContext().getCompiledRegExp(
        inst.getPattern()->getValue().getUnderlyingPointer(),
        inst.getFlags()->getValue().getUnderlyingPointer());
    uint32_t regExpID = moduleGen_.regExpTable.addRegExp(&regexp);

    generateValue(inst);
    os_ << " = ";
    os_ << "_sh_ljs_create_regexp(shr, shUnit, ";
    os_ << llvh::format("get_symbols(shUnit)[%u]", patternStrID);
    os_ << ", ";
    os_ << llvh::format("get_symbols(shUnit)[%u]", flagsStrID);
    os_ << ", " << regExpID << ");\n";
  }
```

- In the unit emission (line 3170), after
  `moduleGen.literalBuffers.generate(OS);`, add
  `generateRegExpTable(OS, moduleGen.regExpTable);`.
- In the `CREATE_THIS_UNIT` initializer, after line 3211
  (`.object_literal_class_cache = ...`), add:

```cpp
       << ".regexp_storage = s_regexp_storage, "
       << ".regexp_table = s_regexp_table, "
       << ".num_regexps = " << moduleGen.regExpTable.getEntryList().size()
       << ", "
```

- [ ] **Step 7: Build and run**

```bash
cmake --build $R/cmake-build-asan --target hermes hermesc shermes \
  HermesVMRuntimeTests
$R/cmake-build-asan/unittests/VMRuntime/HermesVMRuntimeTests \
  --gtest_filter='RegExpBytecode*'
LIT_FILTER='shermes' cmake --build $R/cmake-build-asan --target check-hermes
```

Expected:
- Everything passes.
- Every existing `test/shermes` test passes. They all have zero or more
  regexps, and the ones with zero exercise the empty table.

- [ ] **Step 8: Prove each test can fail**
  - `SHUnitSharesPrecompiledBytecode`: in `sh_unit_get_regexp_bytecode`,
    return a new entry without storing it. FAILS (distinct data, and the
    memory size is wrong).
  - `regexp-aot.js` CHECK lines:
    - Emit `0` instead of `regExpID` in the call. The third call CHECK
      (`, 1);`) fails.
    - Separately, to pin uniquing: temporarily delete the early-return
      lookup in `UniquingRegExpTable::addRegExp`
      (`include/hermes/Regex/RegexSerialization.h:122-124`). The keys
      still reference Context-owned `CompiledRegExp`s. The table gets
      three entries, and the second call CHECK (`, 0);`, now `1`) and
      `.num_regexps = 2` fail. Restore it, and rebuild `hermesc` and
      `shermes` too.
  - `regexp-aot.js` EXEC (guard): in `_sh_ljs_create_regexp`, pass
    `regexp_id ^ 1` to `sh_unit_get_regexp_bytecode`. `mk` gets `uni`'s
    bytecode, and EXEC FAILS.

- [ ] **Step 9: Full suite** on `cmake-build-asan`: 0 failures.

- [ ] **Step 10: Code-review note for the report**

The spec states that no test proves the runtime parse is gone. Quote,
in the report, the final `_sh_ljs_create_regexp` and
`sh_unit_get_regexp_bytecode`. Confirm that neither calls
`JSRegExp::initialize(self, runtime, pattern, flags)` (the compiling
overload) nor `regex::Regex`.

- [ ] **Step 11: Commit**

```bash
git -C $R add include/hermes/VM/static_h.h lib/VM/SHUnitExt.h \
  lib/VM/StaticHUnit.cpp lib/VM/StaticH.cpp lib/BCGen/SH/SH.cpp \
  unittests/VMRuntime/RegExpBytecodeTest.cpp test/shermes/regexp-aot.js
git -C $R commit   # "SH: compile regexp literals ahead of time"
```

---

### Task 5: Concurrency guards under TSan

Spec "TSan" cases 1 and 2. These are regression guards.
- **Case 1.** Old-gen-sweep finalizers on the Hades background thread
  decrement a `Cached` entry's count while the mutator increments it by
  evaluating the same literal. The module's cache keeps the count above
  zero.
  - The sweep runs under `gcMutex_` on the executor thread
    (`lib/VM/gcs/HadesGC.cpp:1641`, `:1717`, `:1168`).
  - The mutator's cache lookup and pointer copy hold no such lock
    (`include/hermes/VM/HadesGC.h:1684`, `lib/VM/gcs/HadesGC.cpp:2339`).

  So a non-atomic count is a real race, which TSan can observe subject
  to scheduling.
- **Case 2.** A lifetime guard. The module dies first, and the
  finalizers of the promoted regexps then release the entry's last
  references.

**Files:**
- Modify: `unittests/VMRuntime/RegExpBytecodeTest.cpp`

**Interfaces:**
- **Consumes:** everything above;
  `GCBase::collectWithOptions(std::string &&, const GCOptions &)`
  (`include/hermes/VM/GCBase.h:1092-1094`);
  `GCBase::inYoungGen(const GCCell *)` (`:1101`); and `kConcurrentGC`
  (`include/hermes/VM/GCConcurrency.h:25`).

- [ ] **Step 1: Invoke the gc-safe-coding skill.**

- [ ] **Step 2: Write the tests**

- Add the includes `"hermes/VM/GCConcurrency.h"` and `<thread>`.
- Look up `Callable::executeCall1`'s exact signature in
  `include/hermes/VM/Callable.h` before writing the calls below. Adapt
  the argument (`HermesValue` or `Handle<>`) to it.
- Add these helpers to the anonymous namespace:

```cpp
/// Run \p code, which evaluates to [mk, fill]: mk() returns a regexp
/// literal, fill(n) returns an array of n results of mk().
void runFactory(
    Runtime &runtime,
    std::shared_ptr<hbc::BCProvider> provider,
    MutableHandle<JSFunction> &mk,
    MutableHandle<JSFunction> &fill) {
  RuntimeModuleFlags rmFlags;
  rmFlags.persistent = false;
  CallResult<HermesValue> res = runtime.runBytecode(
      std::move(provider),
      rmFlags,
      "test.js",
      Runtime::makeNullHandle<Environment>());
  ASSERT_NE(ExecutionStatus::EXCEPTION, res.getStatus());
  Handle<JSArray> arr = runtime.makeHandle(vmcast<JSArray>(*res));
  mk = vmcast<JSFunction>(arr->at(runtime, 0).unboxToHV(runtime));
  fill = vmcast<JSFunction>(arr->at(runtime, 1).unboxToHV(runtime));
}

/// The source run by runFactory.
const char *const kFactorySource =
    "(function () {"
    "  function mk() { return /c(o)n/g; }"
    "  function fill(n) {"
    "    var a = new Array(n);"
    "    for (var i = 0; i < n; ++i) a[i] = mk();"
    "    return a;"
    "  }"
    "  return [mk, fill];"
    "})()";

/// \return a provider for kFactorySource that is owned only by the
/// returned pointer, and set \p weak to observe it.
std::shared_ptr<hbc::BCProvider> factoryProvider(
    std::weak_ptr<hbc::BCProvider> &weak) {
  std::vector<uint8_t> bc = bytecodeForSource(kFactorySource);
  std::shared_ptr<hbc::BCProvider> provider =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::make_unique<OwnedMemoryBuffer>(
              llvh::MemoryBuffer::getMemBufferCopy(
                  llvh::StringRef((const char *)bc.data(), bc.size()))))
          .first;
  weak = provider;
  return provider;
}

/// Collect young generations until every element of \p arr is in the old
/// generation, so that its finalizer runs in an old-gen sweep.
::testing::AssertionResult promoteAll(Runtime &runtime, Handle<JSArray> arr) {
  for (int round = 0; round < 4; ++round) {
    bool allOld = true;
    for (uint32_t i = 0, e = JSArray::getLength(*arr, runtime); i < e; ++i) {
      if (runtime.getHeap().inYoungGen(
              vmcast<GCCell>(arr->at(runtime, i).unboxToHV(runtime))))
        allOld = false;
    }
    if (allOld && runtime.getHeap().inYoungGen(*arr) == false)
      return ::testing::AssertionSuccess();
    runtime.getHeap().collectWithOptions("test", GCBase::GCOptions{});
  }
  return ::testing::AssertionFailure() << "regexps were not promoted";
}

/// Request asynchronous old-generation collections while running \p work
/// for \p iterations, then finish with a synchronous full collection.
template <typename Work>
void driveAsyncMajor(Runtime &runtime, int iterations, Work work) {
  const GCBase::GCOptions async{
      GCBase::GCType::Major, GCBase::GCExecutionPolicy::Async};
  for (int i = 0; i < iterations; ++i) {
    if (i % 64 == 0)
      runtime.getHeap().collectWithOptions("test", async);
    GCScopeMarkerRAII marker{runtime};
    work();
    std::this_thread::yield();
  }
  runtime.collect("test");
}
```

- Add the tests:

```cpp
TEST_F(RegExpBytecodeTest, ConcurrentReleaseWhileAcquiring) {
  if (!kConcurrentGC)
    GTEST_SKIP() << "needs the Hades background thread";
  std::weak_ptr<hbc::BCProvider> weak;
  MutableHandle<JSFunction> mk{runtime}, fill{runtime};
  runFactory(runtime, factoryProvider(weak), mk, fill);

  CallResult<PseudoHandle<>> filled = Callable::executeCall1(
      fill,
      runtime,
      Runtime::getUndefinedValue(),
      HermesValue::encodeTrustedNumberValue(256));
  ASSERT_NE(ExecutionStatus::EXCEPTION, filled.getStatus());
  MutableHandle<JSArray> arr{runtime, vmcast<JSArray>(filled->get())};
  ASSERT_TRUE(promoteAll(runtime, arr));

  // Drop the promoted regexps; their old-gen sweep releases the shared
  // entry while the mutator keeps acquiring it.
  arr = nullptr;
  driveAsyncMajor(runtime, 4096, [&]() { callForRegExp(runtime, mk); });
  EXPECT_TRUE(matches(runtime, callForRegExp(runtime, mk), "con"));
}

TEST_F(RegExpBytecodeTest, ConcurrentReleaseAfterModuleDeath) {
  if (!kConcurrentGC)
    GTEST_SKIP() << "needs the Hades background thread";
  std::weak_ptr<hbc::BCProvider> weak;
  MutableHandle<JSArray> arr{runtime};
  {
    // Handles live until their GCScope ends, not their C++ scope. This
    // scope frees the factory's handles; arr belongs to the fixture's.
    GCScope factoryScope{runtime};
    MutableHandle<JSFunction> mk{runtime}, fill{runtime};
    runFactory(runtime, factoryProvider(weak), mk, fill);
    CallResult<PseudoHandle<>> filled = Callable::executeCall1(
        fill,
        runtime,
        Runtime::getUndefinedValue(),
        HermesValue::encodeTrustedNumberValue(256));
    ASSERT_NE(ExecutionStatus::EXCEPTION, filled.getStatus());
    arr = vmcast<JSArray>(filled->get());
    // Release the factory: a callable function keeps its Domain alive.
  }
  ASSERT_TRUE(promoteAll(runtime, arr));
  for (int i = 0; i < 3 && !weak.expired(); ++i)
    runtime.collect("test");
  // Precondition: the module is gone; only the regexps hold the entry.
  ASSERT_TRUE(weak.expired());

  arr = nullptr;
  driveAsyncMajor(runtime, 4096, [&]() {
    // Unrelated allocation keeps the mutator busy.
    JSObject::create(runtime);
  });
}
```

`MutableHandle` assignment from a raw pointer and from `nullptr` exists
in `include/hermes/VM/Handle.h`. Check it, and use `.set(...)` if
`operator=` takes a different type. In `promoteAll`, pass `arr.get()`,
not `*arr`, to `inYoungGen`. The `weak.expired()` precondition fails if
anything still roots the factory: fix the rooting, never the
assertion.

- [ ] **Step 3: Run on the TSan tree**

```bash
cmake --build $R/cmake-build-tsan --target HermesVMRuntimeTests
TSAN_OPTIONS=halt_on_error=1 \
  $R/cmake-build-tsan/unittests/VMRuntime/HermesVMRuntimeTests \
  --gtest_filter='RegExpBytecode*'
```

Expected: everything passes with no TSan report. Both concurrency tests
RUN, not skip. If they skip, `kConcurrentGC` is false in this tree:
report `grep HERMESVM_ALLOW_CONCURRENT_GC
$R/cmake-build-tsan/CMakeCache.txt` and STOP.

Also run the same filter on `cmake-build-asan`, which has the concurrent
GC too.

- [ ] **Step 4: Prove the guards can fail**

- **Case 1** (`ConcurrentReleaseWhileAcquiring`), on
  `cmake-build-tsan`:
  1. Derive `RegExpBytecode` from `llvh::RefCountedBase<RegExpBytecode>`,
     whose count is not atomic.
  2. Rebuild, and run the test up to 10 times. Expected: TSan reports a
     data race on `RefCount`.
  3. Restore, rebuild, green.

  If no run reports a race, say so plainly in the report, with the
  iteration counts tried. Do not change the test to force an outcome. A
  TSan guard that cannot be shown to fire is reported as such.
- **Case 2** (`ConcurrentReleaseAfterModuleDeath`), on
  `cmake-build-asan`:
  1. In `~RuntimeModule`, call `entry->Release()` once more per non-null
     entry (an over-release).
  2. With 256 holders, the count starts at 257. After the extra release
     and the cache's own, 255 references remain for 256 holders, so the
     255th finalizer frees the entry and the 256th touches freed memory.
     Expected: ASan reports a heap-use-after-free.
  3. Restore, rebuild, green.

  This shows the guard catches a lifetime error. It does not by itself
  show which thread ran the finalizer.

- [ ] **Step 5: Commit**

```bash
git -C $R add unittests/VMRuntime/RegExpBytecodeTest.cpp
git -C $R commit   # "VM: TSan guards for cached regexp bytecode release"
```

---

### Task 6: Cross-configuration verification, Delivered section, follow-up issue

**Files:**
- Modify: `doc/superpowers/specs/2026-10-26-regexp-bytecode-sharing-design.md`
  (append "Delivered")
- Create: one `dz/issues/*.md` file, via `dz`

- [ ] **Step 1: Full suites**

```bash
for t in asan hsan hv32; do
  cmake --build $R/cmake-build-$t --target check-hermes
done
LIT_FILTER='jit/' cmake --build $R/cmake-build-asan --target check-hermes
TSAN_OPTIONS=halt_on_error=1 \
  $R/cmake-build-tsan/unittests/VMRuntime/HermesVMRuntimeTests \
  --gtest_filter='RegExpBytecode*:HeapSnapshot*'
```

Expected: 0 failures everywhere (Review Focus 3 and 4). Record each
tree's counts.

- [ ] **Step 2: File the follow-up**

Run `dz help agents` first. If `dz` is not on `PATH`, STOP and report;
do not install it. Then create an issue:
- **Title:** `SH units have no heap-snapshot node for their regexp cache`
- **Body:** `sh_unit_additional_memory_size` counts the
  `SHUnitExt::regExpCache` vector and its entries. Because SH units have
  no native snapshot node, the entries appear in no heap snapshot. A unit
  node, with child nodes like `RuntimeModule::snapshotAddNodes`, would
  fix this. Cite `lib/VM/StaticHUnit.cpp` (the memory size function) and
  the spec's accounting section.

Add a component only if `dz` requires one (`dz component add` per
CLAUDE.md).

- [ ] **Step 3: Delivered section**

Append `## Delivered (2026-10-26)` to the spec. It lists:
- the commits of Tasks 1-5;
- the per-tree suite counts;
- each mutation and the failure it produced, including any TSan guard
  that could not be shown to fire;
- every spec resolution taken (from the list below, plus any made during
  implementation);
- the dz issue id.

- [ ] **Step 4: Commit**

```bash
git -C $R add doc/superpowers/specs/2026-10-26-regexp-bytecode-sharing-design.md \
  dz/issues/<the new issue file>.md
git -C $R commit   # "Spec: record delivery of shared regexp bytecode"
```

---

## Spec resolutions (summary)

1. `JSRegExp::getBytecodeEntry()` is added beside the spec's
   `getBytecode()`. Tests need the entry's ownership and `mallocSize()`
   (Task 1).
2. An `Exclusive` `JSRegExp` reports `mallocSize()`, which includes the
   entry header, where it previously reported the bare bytecode size
   (Task 1).
3. The trailing bytes are aligned to `alignof(std::max_align_t)`, as a
   malloc'd buffer is today (Task 1).
4. The SH cache is sized by `SHUnitExt(uint32_t numRegExps)` and read
   through `sh_unit_get_regexp_bytecode` (Task 4).
5. The new `SHUnit` fields follow `object_literal_class_cache` (Task 4).
6. T8 lives in `HeapSnapshotTest.cpp` to reuse its snapshot parsers
   (Task 3).
7. T6's mutation is a temporary borrowing variant of `RegExpBytecode`.
   It fails under ASan after the provider has expired (Task 2).
8. T8 reads the cache vector's capacity from its snapshot node, reached
   through the module's `regExpCache` edge, and checks the malloc
   estimate against it rather than assuming an exact capacity (Task 3).
9. Task 5's case 1 races the mutator's increments against concurrent
   finalizer decrements, and case 2 guards the last release after the
   module destructor. Each has its own mutation (Task 5).
