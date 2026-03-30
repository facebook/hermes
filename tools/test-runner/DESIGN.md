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
- No optimization passes (test262 doesn't need them).
- No lazy compilation (test files are small).

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

### Crash Isolation

- `sigsetjmp`/`siglongjmp` crash guard catches `SIGABRT`/`SIGSEGV` per test.
- Converts crashes to test failures instead of killing the process.
- Trade-off: less safe than process isolation, but much faster.

### Handle Sanitizer Support

- Tests in `handlesan_skip_list` run with `GCSanitizeConfig::SanitizeRate = 0.0`.
- Matches Python runner's `-gc-sanitize-handles=0` behavior.

### Feature Detection

- Uses compile-time `#ifdef HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES`.
- Mirrors Python's runtime `hermes --version` feature detection.

## 4. Differences from Python Runner

| Aspect              | Python runner              | C++ runner                 |
|---------------------|----------------------------|----------------------------|
| Optimization level  | `-O` (enabled)             | None (default)             |
| staticBuiltins      | Explicitly disabled        | Default (off)              |
| Bytecode path       | Serialized to `.hbc` file  | In-memory `BCProvider`     |
| Crash recovery      | Process isolation          | Signal handler             |
| stdout handling     | Normal (inherited)         | Suppressed during tests    |

## 5. File Structure

| File              | Purpose                                            |
|-------------------|----------------------------------------------------|
| `main.cpp`        | CLI, test discovery orchestration, result reporting |
| `Executor.cpp/h`  | Compilation, execution, crash guard, timeout        |
| `Skiplist.h`      | JSON skiplist loading, feature/path-based skip logic|
| `TestDiscovery.h` | File enumeration, frontmatter parsing              |
| `CMakeLists.txt`  | Build configuration                                |

## 6. Testing

- Full test262 suite: 38,418 passes, 0 failures.
- Exact match with Python runner on pass/fail counts.
- 8x wall-time speedup.
