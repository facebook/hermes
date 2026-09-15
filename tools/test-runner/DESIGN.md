# test-runner Design Document

## 1. Motivation

The C++ test-runner replaces the Python-based `test_runner.py` for running the
test262 suite against Hermes. Key motivations:

- **8x faster**: ~20s vs ~153s for the full test262 suite.
- **Eliminates subprocess overhead**: The Python runner spawns 2 processes per
  test (compile + execute) × ~50K tests = ~100K subprocesses. The C++ runner
  does everything in-process.
- **Better resource management**: In-process execution enables crash isolation
  via signal handlers and direct control over memory and timeouts.

## 2. Architecture Overview

```
┌─────────────────────────────────────────────────┐
│                   test-runner                    │
│                                                 │
│  ┌───────────┐  ┌───────────┐  ┌─────────────┐ │
│  │ Discovery │→│ Skiplist  │→│ Thread Pool │ │
│  │           │  │ Filtering │  │  Execution  │ │
│  └───────────┘  └───────────┘  └──────┬──────┘ │
│                                       │        │
│                              ┌────────▼───────┐ │
│                              │   Per-Test:    │ │
│                              │ Source → BCPro-│ │
│                              │ viderFromSrc → │ │
│                              │ runBytecode    │ │
│                              └────────────────┘ │
└─────────────────────────────────────────────────┘
```

- Single C++ binary linked against Hermes libraries.
- In-process compilation and execution (no subprocess spawning).
- Thread pool for parallel test execution.
- Source → in-memory bytecode → execute directly (no `.hbc` serialization).

## 3. Key Design Decisions

### In-Process vs Subprocess

Python runner (2 processes per test):
```
source → hermes -emit-binary → .hbc file → hermes -b .hbc
```

C++ runner (single process, in-memory):
```
source → BCProviderFromSrc (in-memory) → Runtime::runBytecode
```

### Compilation Path

- Uses `hbc::BCProviderFromSrc::create()` directly.
- Skips `CompilerDriver` overhead (no CLI parsing, no file I/O for bytecode).
- Optional optimization passes via `-O` flag (default: off, matching Python runner).
- Optional lazy compilation via `--lazy` flag.

### Optimization Passes

- `-O` flag enables `hbc::fullOptimizationPipeline` callback.
- Default is no optimization (`-O0`), matching Python runner's default behavior.
- The callback is passed to `BCProviderFromSrc::create()`, which sets
  `opts.optimizationEnabled = !!runOptimizationPasses` internally.
- Full test262 suite passes with both `-O` and `-O0`.

### Lazy Compilation

- `--lazy` flag sets `CompileFlags.lazy = true` for `BCProviderFromSrc::create()`.
- Lazy mode is incompatible with persistent runtime modules. The runner sets
  `RuntimeModuleFlags.persistent = !lazy` to avoid the fatal error
  "Cannot enable persistent mode for lazy compilation."
- `lazy_skip_list` tests are only skipped when `--lazy` is active, matching the
  Python runner's conditional `if lazy: skip_categories.append(LAZY_SKIP_LIST)`.
- When `--lazy` is off (default), `lazy_skip_list` entries are loaded but ignored
  during filtering.

### JIT Compilation

- `--jit` flag accepts three modes: `off` (default), `on`, and `force`.
- Maps to `RuntimeConfig::EnableJIT` and `RuntimeConfig::ForceJIT`.
- JIT is a runtime-only setting — it does not affect compilation flags.
- `--jit=force` matches the Python runner's `--vm-args='-Xjit=force'` behavior.

### Shermes Compilation

- `--shermes` flag switches from in-process execution to subprocess-based
  compilation and execution using the Static Hermes (shermes) AOT compiler.
- Requires `--shermes-binary` to specify the path to the shermes executable.
- Two-step subprocess approach matching the Python runner:
  1. Compile: `shermes <source> -o <binary> <COMPILE_ARGS> [-strict] [-O|-O0]`
  2. Run: `<binary> -Xes6-proxy -Xhermes-internal-test-methods -Xmicrotask-queue`
- Uses LLVM's `ExecuteAndWait` for subprocess management with per-step timeouts.
- Timeout vs signal crash is distinguished via the error message string from
  `ExecuteAndWait` (returns -2 for both cases).
- Mutually exclusive with `--lazy` and `--jit` (which are in-process-only).
- Crash isolation is provided naturally by process isolation (no `sigsetjmp`
  needed).

### Crash Isolation (in-process execution)

Subprocess mode gets crash isolation for free; the in-process path doesn't —
a `SIGSEGV`/`SIGABRT`/`SIGBUS`/`SIGFPE`/`SIGILL` raised inside
`Runtime::runBytecode()` would otherwise kill the entire runner. A
`sigsetjmp`/`siglongjmp`-based guard converts those into per-test failures
and keeps the worker alive for subsequent tests.

- **Signal handlers** are installed once globally in `runAllTests()` before
  workers spawn (POSIX-only, gated on `#ifndef _WIN32`).
- **Per-thread state** lives in `thread_local CrashGuardState tCrashGuard`
  (`sigjmp_buf`, an `active` flag, the captured signal). Worker code wraps
  each test's `executeCompiledTest()` call with `sigsetjmp` + setting
  `active = 1`; on `siglongjmp`, the guard reports
  `FAIL: Execution crashed (<signal name>)` and the worker continues.
- **Crashes outside the guarded region** (e.g., during compilation) fall
  through to `SIG_DFL` and re-raise — only execution is recoverable.
- **Intentional leaks on crash**: the Hermes runtime, its `TimeLimitMonitor`
  watch entry, and any allocations on the test's stack frame are leaked.
  C++ destructors cannot run safely after `siglongjmp`, and forcing them
  risks corrupting GC state. The leaked monitor entry is harmless because
  the runtime memory is leaked (not freed), so any later background-thread
  write lands on still-valid memory. If a crash corrupts global heap
  metadata, a subsequent crash on the same worker will not re-enter the
  guard (`active` is 0) and will terminate the process via `SIG_DFL`.

#### Alternate signal stack

Stack-overflow `SIGSEGV` cannot be handled on the regular thread stack, so
each worker is associated with a `sigaltstack`-registered alternate stack
(handlers use `SA_ONSTACK`). Two subtleties on macOS + sanitizers:

- **Reuse, don't replace, an existing alt stack.** Sanitizer runtimes
  (`sanitizer_common::SetAlternateSignalStack`, used by ASan/TSan/MSan)
  install a per-thread alt stack at thread init (~`SIGSTKSZ * 4`).
  `setupThreadAltStack()` queries the kernel and skips its own setup if
  one is already in place — only allocating when none exists.
- **`mmap`, not `malloc`, when we do allocate.** Sanitizer runtimes call
  `UnsetAlternateSignalStack` at thread exit, which queries the kernel for
  `ss_sp` and unconditionally `munmap`s it. macOS retains the kernel's
  `ss_sp` even after `sigaltstack(SS_DISABLE)`, so the sanitizer sees
  whatever pointer we installed — `munmap` of `mmap` memory is a benign
  no-op (works even after we've already unmapped it), but `munmap` of
  `malloc` memory fails with `EINVAL`, tripping the sanitizer's internal
  `CHECK` and aborting the process via `SIGTRAP`.
- `teardownThreadAltStack()` only acts when we own the alt stack
  (`tCrashGuard.altStackMem != nullptr`); for reused stacks we leave the
  owner's cleanup alone.

### CompileFlags (matching Python's COMPILE_ARGS)

```cpp
compileFlags.test262 = true;
compileFlags.enableES6BlockScoping = true;
compileFlags.enableTDZ = true;
compileFlags.enableAsyncGenerators = true;
compileFlags.emitAsyncBreakCheck = true; // for timeout support
```

### RuntimeConfig (matching Python's run flags)

```cpp
ES6Proxy = true
MicrotaskQueue = true
EnableHermesInternalTestMethods = true
Test262 = true
```

### Handle Sanitizer Support

- Tests in `handlesan_skip_list` run with `GCSanitizeConfig::SanitizeRate = 0.0`.
- Matches Python runner's `-gc-sanitize-handles=0` behavior.

### Feature Detection

- Uses compile-time `#ifdef HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES`.
- Mirrors Python's runtime `hermes --version` feature detection.

## 4. Differences from Python Runner

| Aspect              | Python runner              | C++ runner                 |
|---------------------|----------------------------|----------------------------|
| Lazy compilation    | `--lazy` flag              | `--lazy` flag              |
| JIT compilation     | `--vm-args='-Xjit=force'`  | `--jit={off,on,force}`     |
| staticBuiltins      | Explicitly disabled        | Default (off)              |
| Bytecode path       | Serialized to `.hbc` file  | In-memory `BCProvider`     |
| Shermes compilation | `shermes` subprocess       | `--shermes` subprocess     |
| stdout handling     | Normal (inherited)         | Suppressed during tests    |
| Crash isolation     | Process boundary           | `sigsetjmp` guard (in-proc)|

## 5. File Structure

| File              | Purpose                                            |
|-------------------|----------------------------------------------------|
| `main.cpp`        | CLI, test discovery orchestration, result reporting |
| `Executor.cpp/h`  | Compilation, execution, timeout                     |
| `Skiplist.h`      | JSON skiplist loading, feature/path-based skip logic|
| `TestDiscovery.h` | File enumeration, frontmatter parsing              |
| `CMakeLists.txt`  | Build configuration                                |

## 6. Testing

- Full test262 suite: 38,418 passes, 0 failures.
- Exact match with Python runner on pass/fail counts.
- 8x wall-time speedup.
