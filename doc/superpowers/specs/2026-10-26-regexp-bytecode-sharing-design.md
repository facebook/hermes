# Shared RegExp bytecode for literals

Status: implemented (see Delivered). Written against x86-jit `58f7ba649`,
then rebased onto `static_h` `393a539e6`; nothing here depends on the JIT.

## Problem

Every evaluation of a regex literal produces a new `JSRegExp` (required by
the spec) and gives it a private copy of the regex bytecode:

- Interpreter and both JITs: `CreateRegExp` →
  `Interpreter::createRegExp` (`lib/VM/Interpreter-slowpaths.cpp:597-621`;
  the JIT reaches it through `lib/VM/JIT/JitHandlers.cpp:245`) fetches the
  bytes from the module (`RuntimeModule::getRegExpBytecodeFromRegExpID`,
  `lib/VM/RuntimeModule.cpp:290-295`) and `JSRegExp::initializeBytecode`
  mallocs and memcpys them (`lib/VM/JSRegExp.cpp:264-275`). The finalizer
  frees them (`lib/VM/JSRegExp.cpp:377-387`).
- Static Hermes: `_sh_ljs_create_regexp` (`lib/VM/StaticH.cpp:1789-1810`)
  calls `JSRegExp::initialize(re, runtime, pattern, flags)`, which
  **re-parses and re-compiles the pattern on every evaluation**
  (`lib/VM/JSRegExp.cpp:169-204`). SH has no ahead-of-time regex bytecode
  (`lib/BCGen/SH/SH.cpp:1805-1825`, TODO T132343328).

The interpreter cost is a malloc + memcpy + free per evaluation; the SH
cost is a full parse and compile per evaluation. Either way, nothing
per-pattern survives between evaluations, so a future regex JIT would have
nowhere to keep compiled code.

## Goals

1. Each regex literal's bytecode is materialized at most once per
   `RuntimeModule` (interpreter/JIT) or per `SHUnit` (SH), and shared by
   every `JSRegExp` created from it.
2. SH compiles regex literals ahead of time, as HBC already does, so it
   never parses a literal at runtime.
3. A single shared, immutable, reference-counted object that a later regex
   JIT can hang compiled code off.

## Non-goals

- Caching `new RegExp(str)`. Recreating a dynamic regex repeatedly is the
  program's choice; dynamic regexes keep exclusively owned bytecode, as
  today. (A regex JIT would compile only after a per-entry use count, which
  a regex rebuilt per call never reaches.)
- Weak references, eviction, LRU. Literal caches are bounded by the number
  of distinct literals and hold strong references.
- Borrowing the module's storage instead of copying once. The module is not
  guaranteed to outlive the regex (see Lifetime below), and with lazy
  compilation the storage is replaced as the module grows
  (`lib/BCGen/HBC/BytecodeGenerator.cpp:848-851` →
  `lib/BCGen/HBC/BCProviderFromSrc.cpp:72-73`).
- Matcher, glue, and per-search allocation costs (`std::vector` in
  `performSearch`, `lib/VM/JSRegExp.cpp:291`).

## Design

### `vm::RegExpBytecode`

New native class (`include/hermes/VM/RegExpBytecode.h`; the name
`CompiledRegExp` is taken by the compiler-side class in
`include/hermes/Regex/RegexSerialization.h:39`):

```cpp
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

  static llvh::IntrusiveRefCntPtr<RegExpBytecode> create(
      llvh::ArrayRef<uint8_t> bytecode, Ownership ownership);

  llvh::ArrayRef<uint8_t> bytecode() const;
  const regex::RegexBytecodeHeader &header() const;
  Ownership ownership() const;
  /// Bytes of malloc'd memory, including this header.
  size_t mallocSize() const;

  static void operator delete(void *p) { free(p); }
 private:
  uint32_t size_;
  Ownership ownership_;
  // uint8_t bytes_[size_] follow, suitably aligned.
};
```

- One `checkedMalloc` holds the object and the trailing bytes; a
  class-specific `operator delete` frees it, which is what
  `ThreadSafeRefCountedBase::Release` calls
  (`external/llvh/include/llvh/ADT/IntrusiveRefCntPtr.h:97-102`).
- Immutable after `create`. Nothing in the VM writes to bytecode after
  initialization today; the one per-object flag mutation (split clearing
  `sticky`, `lib/VM/JSLib/RegExp.cpp:2436-2439`) writes the `JSRegExp`'s
  own `syntaxFlags_`, which stays per object (see below).
- The count is atomic because the last release can happen on the Hades
  background thread: old-generation sweep runs finalizers
  (`lib/VM/gcs/HadesGC.cpp:1063`, `:1112`, `:1168`), and `~JSRegExp` is
  called from `JSRegExp::_finalizeImpl` (`lib/VM/JSRegExp.cpp:381-387`).
  Deletion is a plain `free`, safe on any thread. Nothing ever looks up an
  entry whose count may be dropping to zero, because every lookup path
  goes through a cache that itself holds a strong reference.

### `JSRegExp`

Replace `uint8_t *bytecode_` and `uint32_t bytecodeSize_`
(`include/hermes/VM/JSRegExp.h:151-152`) with
`llvh::IntrusiveRefCntPtr<RegExpBytecode> bytecode_`, keeping it right
after `pattern_` for the alignment reason in the comment at
`include/hermes/VM/JSRegExp.h:145-148`. A one-pointer intrusive handle is
safe to relocate when the GC moves the cell.

- `syntaxFlags_` stays a per-object field, still initialized from the
  header (`lib/VM/JSRegExp.cpp:269-271`). `JSRegExp::search` reads it for
  `sticky` (`lib/VM/JSRegExp.cpp:342`), and split's `setSyntaxFlags` must
  keep affecting only the splitter.
- `initialize(..., ArrayRef<uint8_t>)` (`include/hermes/VM/JSRegExp.h:53-58`)
  becomes `initialize(..., IntrusiveRefCntPtr<RegExpBytecode>)`. It asserts
  `!bytecode_`: every caller initializes a freshly created object (the
  callers are `Interpreter-slowpaths.cpp:613-619`, `StaticH.cpp:1795-1801`,
  `JSLib/RegExp.cpp:484`, `:667-669`, and the two internal ones in
  `JSRegExp.cpp`; Hermes has no `RegExp.prototype.compile`).
- `initialize(self, pattern, flags)` (`lib/VM/JSRegExp.cpp:169-204`, the
  dynamic path) wraps `regex.compile()` in an `Exclusive` entry.
- The fast copy `initialize(self, other, flags)`
  (`lib/VM/JSRegExp.cpp:130-166`) is used only by
  `regExpConstructorFastCopy` (`lib/VM/JSLib/RegExp.cpp:661-675`), whose
  callers are matchAll (`:1679`) and split (`:2428`). The public
  constructor `new RegExp(re)` does not use it: it goes through
  `regExpConstructorInternal` → `regExpInitialize` and recompiles
  (`lib/VM/JSLib/RegExp.cpp:566-630`, `:484`). That path is a dynamic
  regex and stays out of scope. With matching flags, the fast copy does:
  - `Cached` → share the pointer;
  - `Exclusive` → copy into a new `Exclusive` entry, exactly as today.
  This keeps the invariant "an `Exclusive` entry has exactly one
  `JSRegExp` holder", which the memory accounting below relies on.
- New accessor `llvh::ArrayRef<uint8_t> getBytecode() const` for the two
  `search` call sites (`lib/VM/JSRegExp.cpp:351`, `:359`) and for tests.
- `~JSRegExp` drops the reference instead of calling `free`.

### Interpreter/JIT literal cache: per `RuntimeModule`

- New member `std::vector<llvh::IntrusiveRefCntPtr<RegExpBytecode>>
  regExpCache_` and accessor `getRegExpBytecode(uint32_t regExpID)`, which
  replaces the `ArrayRef` accessor in `Interpreter::createRegExp`.
- Lazily sized. On a miss, grow to
  `bcProvider_->getRegExpTable().size()` if needed, then create a `Cached`
  entry from `getRegExpBytecodeFromRegExpID`. The table grows under lazy
  compilation (`BytecodeGenerator.cpp:848-851`), so the size is never
  fixed at module creation.
- Mutator-thread only: `Interpreter::createRegExp` runs on the mutator.
- Literal IDs are already deduplicated by pattern+flags at compile time
  (`UniquingRegExpTable`, `include/hermes/Regex/RegexSerialization.h:90`),
  so two literals with the same text in one module share one entry.
- Named groups are unaffected. For literals, the compiler emits a
  `HermesBuiltin.initRegexNamedGroups` call after `CreateRegExp`
  (`lib/IRGen/ESTreeIRGen-expr.cpp:2911-2946`) that sets the per-object
  mapping (`lib/VM/JSLib/HermesBuiltin.cpp:983-991`).

### SH: ahead-of-time regex bytecode and a per-unit cache

Today SH emits `_sh_ljs_create_regexp(shr, pattern_sym, flags_sym)`
(`lib/BCGen/SH/SH.cpp:1798-1825`), and the runtime parses and compiles on
every call. The comment there says threading a compiled regex through to
SH codegen was "too awkward". That no longer holds: the compiler
`Context` already caches every literal's `CompiledRegExp` by pattern and
flags (`include/hermes/AST/Context.h:351-357`). HBC's ISel uses it
(`lib/BCGen/HBC/ISel.cpp:1514-1527`), and so does IRGen for every backend
(`lib/IRGen/ESTreeIRGen-expr.cpp:2920-2921`).

**Compiler (`lib/BCGen/SH/SH.cpp`):**
- `ModuleGen` (`:429-446`) gains a `UniquingRegExpTable`
  (`include/hermes/Regex/RegexSerialization.h:90`), the same deduplicating
  table HBC uses (`include/hermes/BCGen/HBC/Bytecode.h:219`, `:388-398`).
- `generateCreateRegExpInst` looks up
  `ctx.getCompiledRegExp(pattern, flags)`, adds it to the table, and emits
  `_sh_ljs_create_regexp(shr, shUnit, pattern_sym, flags_sym, id)`. It
  deletes the commented-out code and the TODO.
- Unit emission (`:3175-3217`) writes `s_regexp_storage`, built from the
  table's bytecode buffer, and `s_regexp_table`, an array of
  `{offset, length}`. It fills the new `SHUnit` fields. The buffer writer
  `generateBuffer` is currently a private member of `SHLiteralBuffers`
  (`:235`), so it moves to a free function.
- Every `CreateRegExpInst` that reaches SH codegen has a compiled entry
  in the `Context`:
  - `-O0` and typed mode both run semantic resolution, which compiles
    regex literals into the `Context` (`tools/shermes/shermes.cpp:886`,
    `lib/Sema/SemResolve.cpp:174`, `lib/Sema/SemanticResolver.cpp:821`).
  - Runtime `eval` in SH compiles to HBC, not SH
    (`lib/VM/StaticH.cpp:2441`, `lib/VM/JSLib/eval.cpp:101`), so its
    literals take the `RuntimeModule` path.

**ABI (`include/hermes/VM/static_h.h`):**
- New `SHRegExpTableEntry { uint32_t offset; uint32_t length; }`.
- New `SHUnit` fields: `regexp_storage`, `regexp_table`, `num_regexps`.
- `_sh_ljs_create_regexp` gains `SHUnit *unit` and `uint32_t regexp_id`
  parameters.
- This is an incompatible change to the unit layout and to the function's
  signature. Generated C is always compiled against the `static_h.h` of
  the runtime that loads it. The repository has no other caller: the two
  hits in `lib/VM/JIT/JitHandlers.{h,cpp}` are comments.

**Runtime:**
- `SHUnitExt` (`lib/VM/SHUnitExt.h:19-32`) gains
  `std::vector<IntrusiveRefCntPtr<RegExpBytecode>> regExpCache`, sized to
  `num_regexps` when the unit is attached (`lib/VM/StaticHUnit.cpp:117`).
  It is freed with the `SHUnitExt` in `sh_unit_done`
  (`lib/VM/StaticHUnit.cpp:201-204`, called from `~Runtime`,
  `lib/VM/Runtime.cpp:540-542`). SH has no lazy compilation, so the size
  is fixed.
- `_sh_ljs_create_regexp` creates the `JSRegExp` and takes the bytecode
  from the cache, creating a `Cached` entry on first use. It never parses.
  This mirrors `Interpreter::createRegExp`, and both call the same
  `JSRegExp::initialize(…, IntrusiveRefCntPtr)`.
- Named groups: SH literals get the `initRegexNamedGroups` call from the
  shared IRGen, as HBC does (`lib/IRGen/ESTreeIRGen-expr.cpp:2923-2944`).
  The runtime group-mapping build in `initialize(pattern, flags)` was
  redundant for SH literals and goes away with the parse. T7c checks that
  named-group behaviour is preserved.
- No symbol-keyed runtime map is needed. The unit's index identifies the
  literal, as `regExpID` does for `RuntimeModule`.

### Lifetime

- A `RuntimeModule` can be destroyed while regexes created from it are
  still alive. `Runtime::markDomainRefInRuntimeModules` deletes modules
  whose domain is dead during weak-root marking
  (`lib/VM/Runtime.cpp:803-826`), and a `JSRegExp` holds no reference to
  its domain. The module's cache then drops its references; surviving
  `JSRegExp`s keep the entries alive.
- `RuntimeModule::~RuntimeModule` may therefore run on whatever thread
  runs weak-root marking. The atomic count covers this; no other shared
  state is touched.

### Memory accounting and heap snapshots

Today each `JSRegExp` reports its bytecode as malloc size and as a native
`RegExpBytecode` snapshot node identified by
`gc.getNativeID(bytecode_)`, untracked in the finalizer
(`lib/VM/JSRegExp.cpp:381-425`). Sharing would emit duplicate native
nodes and multiply-counted sizes, so responsibility follows `Ownership`:

- `Exclusive`: unchanged. The `JSRegExp` reports `mallocSize()`, emits
  the native node and edge, and untracks the entry pointer in its
  finalizer.
- `Cached` in a `RuntimeModule`: this follows the `CodeBlock` precedent.
  Code blocks get their own snapshot nodes and are not part of
  `additionalMemorySize()` (`lib/VM/RuntimeModule.cpp:412-417`,
  `:455-498`).
  - **Snapshot.** `snapshotAddNodes` emits:
    - one native `RegExpBytecode` node per materialized entry, sized
      `mallocSize()`;
    - a node for the cache vector, identified by `&regExpCache_` (as
      `&functionMap_` is) and sized `capacity * sizeof(pointer)`, with
      indexed edges to the entry nodes.

    `snapshotAddEdges` adds a `regExpCache` edge from the module node.
    `additionalMemorySize()` is unchanged, so the `RuntimeModule` node's
    size, which is `sizeof(RuntimeModule) + additionalMemorySize()`
    (`lib/VM/Domain.cpp:85-94`), does not double-count.
  - **Malloc estimate.** A new `RuntimeModule::regExpCacheMemorySize()`
    returns the vector capacity in bytes plus the sum of the entry sizes.
    It is added in `Domain::_mallocSizeImpl` (`lib/VM/Domain.cpp:62-70`)
    and nowhere in the snapshot.
  - **ID tracking.** `~RuntimeModule` untracks the entry pointers and
    `&regExpCache_`, as it does for code blocks and `&functionMap_`
    (`lib/VM/RuntimeModule.cpp:72-75`). Otherwise a stale native ID
    survives and runtime teardown can assert (`lib/VM/Runtime.cpp:525`).
  - **`JSRegExp`.** It reports 0 and emits nothing for a `Cached` entry.
- `Cached` in an `SHUnitExt`: `sh_unit_additional_memory_size`
  (`lib/VM/StaticHUnit.cpp:206-210`) adds the cache vector's capacity
  plus the sizes of the materialized entries. SH units have no native
  snapshot node today, so these entries get no node either (follow-up dz
  issue).
- Known gap: after a module is destroyed, `Cached` entries still held by
  surviving `JSRegExp`s are reported by nobody. This is rare (dead domain,
  live regex) and preferable to dangling snapshot edges.

### Future regex JIT hook (not in this change)

Compiled code would live on `Cached` entries only, behind a use counter,
and be freed when the entry's last reference drops.

- **Release thread.** That release may run on the Hades background
  thread. asmjit permits this: `JitRuntime::_release` calls
  `JitAllocator::release`, which takes the allocator lock
  (`external/asmjit/asmjit/src/asmjit/core/jitruntime.cpp:73-75`,
  `jitallocator.cpp:904-912`).
- **Rooting invariant the JIT must keep.** Code must not be freed while
  it runs. Holding a raw pointer to the code does not guarantee that.
  Every invocation must keep either the owning `JSRegExp` rooted or an
  explicit entry reference for its whole duration, including reentry.
  Today's matcher receives a `Handle<JSRegExp>`
  (`lib/VM/JSRegExp.cpp:324`).
- **Allocator lifetime.** The allocator must outlive every entry that
  releases code.

This change only has to keep `RegExpBytecode` as the single per-pattern
object.

## Tests

Each test states what fails before the change, or labels itself a
regression guard. They are VM unit tests in a new
`unittests/VMRuntime/RegExpBytecodeTest.cpp` (running JS with
`runtime.run`, as in `unittests/VMRuntime/StackTracesTreeTest.cpp:43-57`)
unless noted. "Same bytes" means equal `getBytecode().data()` on two
simultaneously live objects. Before the change, two live objects always
have distinct malloc'd buffers, so that equality cannot hold by accident.

- **T1, literal sharing.** `function f(){ return /a(b)c/g; } [f(), f()]`:
  both elements have the same bytes. Fails before: distinct copies.
- **T2, lazy compilation growth.** Run with lazy compilation forced,
  where a lazily compiled function contains a literal whose ID is past
  the table size at module load. The literal is shared across two calls
  and matches correctly. The test first asserts that precondition (the
  table size before and after the lazy compile, read from the
  `BCProvider`), so it cannot pass vacuously. Fails before: distinct
  copies. An implementation
  that sized the cache at module load would index out of bounds (ASan
  build).
- **T3, dynamic not shared (regression guard).** `new RegExp("abc")`
  twice gives different bytes. Passes before and after; pins the
  non-goal.
- **T4, fast copy (C++).** The test calls
  `JSRegExp::initialize(self, other, flags)` directly, keeping both
  objects rooted and alive during the comparison:
  - `other` comes from a literal (`Cached`): equal flags share the bytes;
    different flags give distinct, recompiled bytes.
  - `other` comes from `new RegExp(str)` (`Exclusive`): equal flags give
    distinct bytes.

  The first assertion fails before. The public `new RegExp(re)` is not
  tested for sharing because it is out of scope.
- **T5, per-object sticky (regression guard).**
  `var r = /a/y; "bab".split(r); r.lastIndex = 0; r.exec("ba")` is
  `null`, and `r.sticky` is `true`. Guards against `split`'s flag change
  leaking through shared state.
- **T6, module dies first (regression guard).** The default, non-lazy
  `Runtime::run` is unsuitable here. It loads source-compiled modules as
  persistent whenever lazy compilation is off
  (`include/hermes/BCGen/HBC/BCProviderFromSrc.h:164-168`), and
  `runBytecode` then keeps the provider in `persistentBCProviders_`
  (`lib/VM/Runtime.cpp:1123-1125`, `:1156`), so the storage would
  outlive the module. Instead:
  - Compile with `bytecodeForSource`.
  - Copy the bytes into an **owning** buffer (`OwnedMemoryBuffer` around
    `getMemBufferCopy`), because a plain `Buffer` does not free its data
    (`include/hermes/Support/Buffer.h:22-24`). Then drop the vector.
  - Build a `BCProviderFromBuffer` held only by the `shared_ptr` passed to
    `runBytecode` with `persistent = false`. `runBytecode` creates the
    fresh `Domain` itself (`lib/VM/Runtime.cpp:1193`).
  - The script returns a literal regex. The test keeps only that
    `JSRegExp` and a `std::weak_ptr<BCProvider>`.
  - Collect, and assert the precondition that the weak pointer has
    expired. Then `exec` must still match, with no ASan report.

  Passes before (private copy). Fails under ASan for any variant that
  borrows module storage.
- **T7, SH.**
  - **T7a, emission:** `test/shermes/regexp-aot.js` with
    `%shermes -emit-c -o - %s | %FileCheck %s`, following
    `test/shermes/strict-equal-inline-fastpath.js:8`. It checks:
    - `s_regexp_storage` and `s_regexp_table` are emitted;
    - two textually equal literals produce one table entry;
    - the call passes `shUnit` and an index.

    Fails before: none of these is emitted.
  - **T7b, sharing (C++):** a unit test builds a zeroed `SHUnit` with
    only the regexp fields set, attaches the `SHUnitExt`, and calls
    `_sh_ljs_create_regexp` twice with the same index. Both results have
    the same bytes. Fails before: the old function re-compiles on every
    call.
  - **T7c, behaviour (regression guard):** the same file run with
    `%shermes -exec`. It evaluates a named-group literal in a loop and
    checks `groups`, `flags`, `lastIndex` and `sticky` on each result.
  - **Not covered by tests.** No test proves that the runtime parse is
    gone. A cache miss that compiled from the symbols and then shared the
    result would pass T7a–c. Code review verifies instead that cache
    misses read the emitted bytecode and never call runtime compilation.
    That avoids adding a production counter.
- **T8, memory accounting.** First assert that the cache is empty. Then,
  for T1's objects:
  - each `JSRegExp` has malloc size 0;
  - `regExpCacheMemorySize()` grows by the vector capacity in bytes plus
    the entry's `mallocSize()` after the first evaluation, and by 0
    after the second;
  - a heap snapshot contains exactly one `RegExpBytecode` node for the
    literal, sized `mallocSize()`, plus the cache-vector node with its
    edges;
  - the `RuntimeModule` node's size is unchanged by the evaluations.

  Fails before: each `JSRegExp` reports `bytecodeSize_` and emits its
  own node.
- **Existing coverage:** `test/hermes/regexp*.js`, the `check-hermes`
  suite, and `-Xjit=force` runs of the JIT lit tests (the JIT calls the
  same `Interpreter::createRegExp`).
- **TSan.** Regexes that die young are finalized on the mutator
  (`lib/VM/gcs/HadesGC.cpp:2738-2739`), so the scenario must promote them
  first. It uses the async-collection pattern in
  `unittests/VMRuntime/GCBasicsTest.cpp:722`.
  - **Case 1:** keep literal-created regexes in an array across enough
    young-gen collections to promote them, asserting `!inYoungGen` as a
    precondition. Then drop them and start
    `collectWithOptions({Major, Async})`. A plain `collect()` blocks and
    runs on the mutator (`HadesGC.cpp:1462`, `:1484`). Meanwhile the
    mutator keeps evaluating the same literal, acquiring the cached
    entry.
  - **Case 2:** promote the regexes the same way. Then stop evaluating
    the module's literal and release its factory function, since a
    callable function keeps its `Domain` alive. Collect until module
    deletion is verified, then drop the regexes and drive an async major
    collection with unrelated work. The last references are released by
    old-gen-swept finalizers after the module destructor.

## Risks

- **SH ABI:** the `SHUnit` layout and `_sh_ljs_create_regexp`'s signature
  change. Already-generated C must be regenerated.
- **`JSRegExp` cell size:** shrinks by the 4-byte `bytecodeSize_` (modulo
  padding).
- **Heap snapshot tooling** that expects a `RegExpBytecode` node per
  regex will see one per module entry instead.
- **SH output embeds regex bytecode:** generated SH C now carries the
  regex bytecode, so a change to the regex bytecode format requires
  regenerating SH output, as literal buffers already do. Nothing checks
  it.
- **Sandbox:** the generated `API/hermes_sandbox` sources are not
  regenerated by this change.

## Delivered (2026-10-26)

### Commits

- Task 1, `6a5bba9a2`: VM: hold JSRegExp bytecode in a refcounted
  RegExpBytecode
- Task 2, `abb995b2f`: VM: share regexp literal bytecode per RuntimeModule
- Task 3, `02f2e6495`: VM: report cached regexp bytecode from its
  RuntimeModule; `7a9243940`: VM: test the Domain's report of cached
  regexp bytecode
- Task 4, `cb32dd9cb`: SH: compile regexp literals ahead of time
- Task 5, `6d4aae072`: VM: TSan guards for cached regexp bytecode release

### Suites

Each tree was rebuilt (`hermes hermesc shermes HermesVMRuntimeTests`)
before its run. Counts are expected passes / expected failures /
unsupported / unexpected failures from `check-hermes`.

| Tree | Result |
|---|---|
| `cmake-build-asan` (instrumented) | 4527 / 6 / 162 / 2 |
| `cmake-build-hsan` (instrumented) | 4231 / 6 / 329 / 3 |
| `cmake-build-hv32` (not instrumented) | 4317 / 6 / 328 / 1 |
| `cmake-build-asan`, `LIT_FILTER='jit/'` | 167 / 0 / 8 / 0 (plus the 2 known unit failures, which the filter does not exclude) |
| `cmake-build-tsan`, `TSAN_OPTIONS=halt_on_error=1`, `RegExpBytecode*:HeapSnapshot*` | 25 of 25 passed, no report |

Unexpected failures, none caused by this branch:

- `APITests` `WithOrWithoutAllocationTracker/HeapSnapshotAPITest.HeapTimeline/0`
  and `/1` fail on every instrumented tree (asan, hsan). They fail
  identically at `fb22f7c1f`, before this branch, once
  `HERMES_MEMORY_INSTRUMENTATION=ON`. Filed as a dz issue (below).
- `jit/loose-eq-null.js` fails on hsan and hv32 with "JIT is not enabled
  in this build". Neither tree enables the JIT, the test carries no
  `REQUIRES` for it, and the file is not touched by this branch. The test
  exists only on x86-jit.

`cmake-build-asan`, `hsan` and `tsan` were reconfigured with
`HERMES_MEMORY_INSTRUMENTATION=ON` (the default follows
`HERMES_ENABLE_DEBUGGER`, so `HeapSnapshotTest.cpp` was never compiled
there). `hv32` stays uninstrumented, so heap-snapshot coverage runs on
the other three trees only. The pass counts above are therefore not
comparable to the 4473 baseline.

### Suites after rebasing onto `static_h` (2026-10-27)

On `static_h` the JIT exists only for arm64, so `cmake-build-asan` was
reconfigured with `HERMESVM_ALLOW_JIT=0` and the JIT was exercised in an
arm64 cross build (`HERMESVM_ALLOW_JIT=2`, Debug -O1, no ASan) under
qemu-user, following `aarch64/README.md` from x86-jit.

| Tree | Result |
|---|---|
| `cmake-build-asan` (instrumented) | 4422 / 6 / 202 / 2 |
| `cmake-build-hsan` (instrumented) | 4296 / 6 / 203 / 2 |
| `cmake-build-hv32` (not instrumented) | 4382 / 6 / 202 / 0 |
| `cmake-build-arm64`, `LIT_FILTER='jit/'` | 47 / 0 / 1 / 0 |
| `cmake-build-tsan`, `TSAN_OPTIONS=halt_on_error=1`, `RegExpBytecode*:HeapSnapshot*` | 26 of 26 passed, no report |

The unexpected failures are the two `HeapTimeline` tests, which fail
identically on `static_h` `393a539e6` with instrumentation on. On arm64,
`LIT_FILTER='jit/|regexp'` also fails the seven tests that run
`shermes -exec`, all because the host linker cannot link the arm64
`libshermes_console`; that is a limit of the cross setup.

### Mutations and the failure each produced

Task 1:

- `mallocSize()` returns `size_`: `CreateCopiesBytes` (19 vs 31).
- Dynamic-path entry created `Cached`: `DynamicRegExpsAreExclusive`.
- `search` reads sticky from the shared header, `setSyntaxFlags` writes
  it, fast copy always shares: `SplitDoesNotClearStickyOfItsArgument`.

Task 2:

- Return a fresh `Cached` entry without storing it:
  `LiteralIsSharedAcrossEvaluations` and
  `LazilyCompiledLiteralGrowsTheCache`.
- Delete the resize branch: ASan SEGV in `getRegExpBytecode`. A variant
  that sized the cache once on first miss (resize only when the cache is
  empty) passed `LazilyCompiledLiteralGrowsTheCache`, because that test
  grows the table only once. `LazyCompilesKeepGrowingTheCache` now pins
  growth across lazy compiles: it compiles a second function after the
  first (table size 1 -> 2) and fails under that mutation (see the final
  review fix).
- Fast copy always creates: `FastCopySharesOnlyCachedBytecode`.
- Function-local static cache: `LiteralCachesArePerModule` and
  `SplitDoesNotClearStickyOfItsArgument`.
- Borrowed variant of `RegExpBytecode`: ASan heap-use-after-free in
  `searchWithBytecodeImpl` from `LiteralOutlivesItsModule`.
- Sticky read from the shared header: `regexp-literal-shared.js` fails
  under `-O`, `-O0`, `-lazy`, `-gc-sanitize-handles=1` and the JIT lit
  test.

Task 3 (instrumented tree):

- `JSRegExp::_mallocSizeImpl` uses the entry: `getMallocSize` expected 0,
  got 35.
- Per-entry node loop disabled: assertion in
  `HeapSnapshot::addIndexedEdge`.
- Per-entry `untrackNative` removed: assertion in `Runtime::~Runtime`.
- `regExpCache` edge removed: the edge expectation in `HeapSnapshotTest`.
- `Domain.cpp` drops `regExpCacheMemorySize()`: Domain size expected 379,
  got 336.

Task 4:

- `sh_unit_get_regexp_bytecode` returns without storing:
  `SHUnitSharesPrecompiledBytecode`.
- Emit ID 0 for every literal: `regexp-aot.js` CHECK failure under `-O0`
  and `-O`.
- `addRegExp` early return disabled: `regexp-aot.js` fails under `-O0`.
  Under `-O` the optimizer merges the first two literals, so only `-O0`
  pins uniquing.
- `regexp_id ^ 1` in `_sh_ljs_create_regexp`: `regexp-aot.js` EXEC
  failure.

Task 5:

- `ThreadSafeRefCountedBase` replaced by `RefCountedBase`: TSan data race
  in `RefCountedBase<RegExpBytecode>::Release`, from a Hades finalizer
  against `Retain` in `getRegExpBytecode`. It fired on run 1 of 10, so
  detection is probabilistic.
- Extra `Release()` in `~RuntimeModule`: ASan heap-use-after-free in
  `RegExpBytecode::ownership()` from `JSRegExp::_finalizeImpl`.

Every TSan guard was shown to fire.

### Spec resolutions taken

1. `JSRegExp::getBytecodeEntry()` is added beside `getBytecode()`; tests
   need the entry's ownership and `mallocSize()` (Task 1).
2. An `Exclusive` `JSRegExp` reports `mallocSize()`, which includes the
   entry header, where it reported the bare bytecode size (Task 1).
3. Trailing bytes are aligned to `alignof(std::max_align_t)` (Task 1).
4. The SH cache is sized by `SHUnitExt(uint32_t numRegExps)` and read
   through `sh_unit_get_regexp_bytecode` (Task 4).
5. The new `SHUnit` fields follow `object_literal_class_cache` (Task 4).
6. T8 lives in `HeapSnapshotTest.cpp` to reuse its snapshot parsers
   (Task 3).
7. T6's mutation is a temporary borrowing variant of `RegExpBytecode`;
   it fails under ASan after the provider has expired (Task 2).
8. T8 reads the cache vector's capacity from its snapshot node and
   checks the malloc estimate against it (Task 3).
9. Task 5's case 1 races the mutator's increments against concurrent
   finalizer decrements; case 2 guards the last release after the module
   destructor (Task 5).

Made during implementation:

- T8 counts regexp nodes against a recorded baseline plus one, because
  the internal JS modules already cache four entries (Task 3).
- `RegExpBytecodeTest.cpp` has no `GTEST_SKIP` (not in the vendored
  gtest); the TSan tests return early on non-Hades trees, so they pass
  vacuously there (Task 5).
- Under `-O` the `regexp-aot.js` check accepts either order for the two
  merged literals (`OPT-DAG`) (Task 4).
- Controller rulings: Task 1 expects 3 tests, not 4; the review base is
  `fb22f7c1f`; `HERMES_MEMORY_INSTRUMENTATION=ON` in asan/hsan/tsan.

### Follow-ups (dz)

- `01a17ba7-e5f4-73fd-a894-4fe229246c62`: SH units have no heap-snapshot
  node for their regexp cache.
- `01a17ba7-e648-707f-887a-26f2dd9e4f04`: HeapSnapshotAPITest.HeapTimeline
  fails with HERMES_MEMORY_INSTRUMENTATION=ON.
- test/jit/loose-eq-null.js lacks REQUIRES: jit. It belongs to x86-jit,
  where the test lives, and is not filed on this branch.
