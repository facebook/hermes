# Static Hermes Agent Performance Infrastructure

Tools for measuring, analyzing, and comparing Static Hermes performance.
All tools output compact (single-line) structured JSON (`--json`) for
programmatic parsing and human-readable text by default. Compact JSON
minimizes token consumption when AI agents ingest tool output. Designed
for both human engineers and AI agents.

## Quick Start

```bash
# Build shermes with cmake
cmake -B cmake-build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --target shermes

# Run benchmarks
agent-perf/tools/run_benchmarks.sh <shermes> micro --json

# A/B performance comparison (statistical)
agent-perf/tools/benchmark_compare.sh <shermes_a> <shermes_b> --suite all

# A/B binary size comparison
agent-perf/tools/binary_size_compare.sh --baseline <shermes_a> --optimized <shermes_b> --suite octane

# Analyze generated C code
shermes -emit-c -o /tmp/out.c test.js
agent-perf/tools/generated_code_analyzer.py -i /tmp/out.c --json

# Quick performance health check
agent-perf/tools/perf_proxies.sh <shermes> benchmarks/octane/richards.js
```

## Directory Structure

```
agent-perf/
├── tools/              24 analysis and measurement tools
├── benchmarks/
│   ├── micro/          9 targeted subsystem benchmarks (<5s each)
│   │   ├── arithmetic.js
│   │   ├── array_operations.js
│   │   ├── closures_scopes.js
│   │   ├── control_flow.js
│   │   ├── function_calls.js
│   │   ├── object_creation.js
│   │   ├── property_access.js
│   │   ├── string_operations.js
│   │   └── typed_operations.js
│   └── macro/          4 app-representative workloads
│       ├── app_startup.js
│       ├── component_tree.js
│       ├── json_processing.js
│       └── module_system.js
├── schemas/
│   └── perf_schema.json    JSON output format documentation
└── tests/              13 unit test files (250+ tests)
```

Additionally, Octane benchmarks live at `benchmarks/octane/` in the SH root.

## CLI Conventions

All tools follow these conventions:

- `--help` / `-h` — prints usage, arguments, and examples
- `--json` — outputs compact (single-line) structured JSON, optimized for minimal token consumption by AI agents
- `--typed` — compiles with Static Hermes typed mode
- `--compare <shermes_b>` — A/B comparison mode (shell tools)
- Exit code 0 on success, non-zero on error or regression detection
- Shell tools accept positional arguments; Python tools use named flags

---

## Tool Reference

### Benchmarking and Comparison

#### `run_benchmarks.sh` — Benchmark Runner

Compiles and runs the standard benchmark suites, extracting results from
`RESULT:` lines (micro/macro) or Octane score lines.

**Questions answered:**
- How fast is this shermes build on the standard benchmarks?
- What are the ops/sec for each micro/macro benchmark?
- What is the Octane score?

```
Usage: run_benchmarks.sh <shermes_binary> [micro|macro|octane|all] [options]

Arguments:
  <shermes_binary>          Path to the shermes compiler binary (required)
  [micro|macro|octane|all]  Benchmark suite to run (default: all)

Options:
  --json                    Output structured JSON
  --typed                   Compile with --typed flag
  --iterations <N>          Number of iterations per benchmark (default: 5)
  --octane                  Also run benchmarks/octane/ suite
  --shermes-args "ARGS"     Extra arguments to pass to shermes
```

#### `benchmark_compare.sh` — Statistical A/B Comparison

Statistical A/B comparison of two shermes binaries. Uses interleaved execution
for bias reduction and Mann-Whitney U test for significance.

**Questions answered:**
- Is binary B faster or slower than binary A?
- Are performance differences statistically significant?
- What is the geometric mean delta across benchmarks?

```
Usage: benchmark_compare.sh <shermes_a> <shermes_b> [options]

Arguments:
  <shermes_a>               Path to baseline shermes binary (required)
  <shermes_b>               Path to candidate shermes binary (required)

Options:
  --suite <name>            Benchmark suite: micro, macro, all (default: all)
  --json                    Output structured JSON
  --typed                   Compile with --typed flag
  --iterations <N>          Iterations per benchmark (default: 11)
  --threshold <PCT>         Regression threshold percentage (default: 2.0)
```

Exit code 1 if any benchmark regresses beyond threshold.

#### `count_instructions.sh` — Deterministic Instruction Counts

Measures retired CPU instruction counts using `perf stat -e instructions`.
Instructions are the most deterministic performance metric — not affected by
system load or thermal throttling.

**Questions answered:**
- How many instructions does this workload execute?
- Did a compiler change increase or decrease instruction count?
- How many instructions does compilation itself take?

```
Usage: count_instructions.sh <js_file> [options]

Arguments:
  <js_file>                 JavaScript file to measure (required)

Options:
  --shermes <path>          Path to shermes binary (auto-detected if not set)
  --compare <a> <b>         A/B comparison mode with two shermes binaries
  --iterations <N>          Number of iterations (default: 11)
  --typed                   Compile with --typed flag
  --compile-only            Measure compilation instructions only (no execution)
  --json                    Output structured JSON
```

---

### Binary Size Analysis

#### `binary_size_compare.sh` — A/B Binary Size Comparison

End-to-end binary size comparison for Static Hermes compiled JS programs.
Compiles benchmarks with two shermes builds and compares `.text` section sizes,
call site counts, and per-call-site overhead. Optionally uses `bloaty` for
deeper section/symbol/compilation-unit analysis when available.

**Questions answered:**
- How much does a compiler/header change affect generated binary size?
- How many `_sh_ljs_get_by_val_rjs` (or other) call sites exist?
- What is the per-call-site overhead of an inlined fast path?
- (with `--bloaty`) Which sections, symbols, and compilation units grew most?

```
Usage: binary_size_compare.sh --baseline <shermes_a> --optimized <shermes_b> [options]

Options:
  --baseline <path>         Path to baseline shermes binary (required)
  --optimized <path>        Path to optimized shermes binary (required)
  --js <file.js>            JS file to compile (can specify multiple times)
  --suite <name>            Benchmark suite: micro, macro, octane, all
  --typed                   Compile with --typed flag
  --strip                   Compare stripped binaries (removes debug info)
  --bloaty                  Use bloaty for deeper symbol/section analysis (if installed)
  --json                    Output structured JSON
```

When `--bloaty` is specified and bloaty is installed, each benchmark's output
includes additional section-level, symbol-level (top 20), and compilation-unit
(top 10) breakdowns from bloaty's DWARF-aware analysis. Falls back to the
default nm/objdump analysis if bloaty is not available.

#### `binary_size_analyzer.py` — Symbol-Level Size Analysis

Parses `nm`/`objdump` output from two binaries to produce a detailed
symbol-level size comparison. Categorizes symbols and identifies the
largest contributors to size growth.

**Questions answered:**
- How much did the `.text` section grow/shrink?
- Which individual symbols changed size the most?
- Are there new inlined functions present in B but not A?
- What is the `get_by_val`-related symbol growth?

```
Usage: binary_size_analyzer.py [options]

Options:
  --baseline <path>         Path to baseline binary
  --optimized <path>        Path to optimized binary
  --nm-a <path>             Path to pre-captured nm output for baseline
  --nm-b <path>             Path to pre-captured nm output for optimized
  --json                    Output JSON

Either --baseline/--optimized or --nm-a/--nm-b is required.
```

Symbol categories: `get_by_val`, `get_by_id`, `property_put`, `calls`,
`arithmetic`, `generated_js`, `runtime`, `other`.

---

### Generated Code Analysis

#### `generated_code_analyzer.py` — Runtime Call Analysis

The primary diagnostic tool for understanding generated C code. Categorizes
every `_sh_*` runtime call, reports per-function statistics, and detects
anti-patterns.

**Questions answered:**
- How many runtime calls are in the generated code?
- Which category dominates (property access, GC, arithmetic, etc.)?
- Which functions have the highest call density?
- Are there redundant type checks or excessive GC scoping?

```
Usage: generated_code_analyzer.py -i <generated.c> [options]

Options:
  -i, --input <path>        Path to .c file from shermes -emit-c (required)
  --json                    Output JSON
```

Categories tracked: `property_access`, `type_check`, `gc`, `arithmetic`,
`comparison`, `call`, `object`, `string`, `other`.

Detected patterns: `redundant_type_check`, `excessive_gc_scoping`,
`inline_opportunity`, `high_call_density`.

#### `generated_code_antipatterns.py` — C Compiler Optimization Inhibitors

Scans generated `.c` files for patterns that prevent the C compiler from
optimizing well — artifacts of code generation that indicate improvement
opportunities in shermes.

**Questions answered:**
- Does the generated C code have patterns that prevent optimization?
- Are there unnecessary GC root operations? Redundant casts?
- Is there excessive pointer aliasing?

```
Usage: generated_code_antipatterns.py -i <generated.c> [options]

Options:
  -i, --input <path>        Path to generated .c file (required)
  --json                    Output JSON
```

Detected patterns (by severity): `excessive_pointer_aliasing`,
`redundant_null_check`, `excessive_function_calls`, `unnecessary_gc_root`,
`redundant_cast`, `large_stack_frame`, `unreachable_after_throw`.

#### `generated_code_report.sh` — Per-Function Code Metrics

Generates C code and reports per-function metrics: lines of C, runtime call
count, and complexity indicators (branch count).

**Questions answered:**
- How many functions did the compiler generate?
- How large is each function? What is its runtime call count?
- How does generated code differ between two shermes builds?

```
Usage: generated_code_report.sh <shermes_binary> <workload.js> [options]

Options:
  --typed                   Compile with --typed flag
  --json                    Output structured JSON
  --compare <shermes_b>     Compare generated code from two shermes binaries
```

#### `sh_source_mapper.py` — JS-to-C Line Mapping

Maps between JS source, generated C, and native code by parsing `#line`
directives in C code generated by `shermes -emit-c`. Essential for correlating
native code hotspots with original JS source.

**Questions answered:**
- Which JS source line does C line 150 correspond to?
- Which C lines were generated from JS line 25?
- What is the full source map between JS and generated C?

```
Usage: sh_source_mapper.py -i <generated.c> [options]

Options:
  -i, --input <path>        Path to .c file from shermes -emit-c (required)
  --c-line <N>              Query: find JS source line for this C line
  --js-line <N>             Query: find all C lines for this JS line
  --js-file <name>          Filter --js-line results to a specific JS file
  --json                    Output JSON
```

#### `type_coverage_analyzer.py` — Typed Mode Effectiveness

Compares two generated `.c` files (typed vs. untyped) to measure how
effectively typed mode eliminates or reduces runtime calls.

**Questions answered:**
- How effective is typed mode at eliminating runtime calls?
- Which `_sh_*` calls are fully eliminated by type information?
- What is the per-function type coverage percentage?
- Are there calls that increased in typed mode (regressions)?

```
Usage: type_coverage_analyzer.py -t <typed.c> -u <untyped.c> [options]

Options:
  -t, --typed <path>        Path to typed-mode generated .c file (required)
  -u, --untyped <path>      Path to untyped-mode generated .c file (required)
  --json                    Output JSON
```

---

### Profiling and Hotspot Analysis

#### `collect_pmu.sh` — Hardware PMU Counter Collection

Collects hardware Performance Monitoring Unit counters using `perf stat`.
Computes derived ratios (IPC, miss rates). Requires Linux `perf`.

**Questions answered:**
- What is the IPC for this workload?
- What are the cache, branch, and TLB miss rates?
- How do two shermes builds compare on hardware counters?

```
Usage: collect_pmu.sh <shermes_binary> <workload.js> [options]

Options:
  --typed                   Compile with --typed flag
  --json                    Output structured JSON
  --compare <shermes_b>     A/B comparison with second shermes binary
  --shermes-args "ARGS"     Extra arguments to pass to shermes
```

Counters: `instructions`, `cycles`, `L1-dcache-load-misses`,
`L1-icache-load-misses`, `branch-misses`, `dTLB-load-misses`.

#### `pmu_anomalies.py` — PMU Anomaly Detection

Accepts PMU counter data as JSON and flags values that exceed known-good
thresholds. Pairs with `collect_pmu.sh --json` output.

**Questions answered:**
- Are there microarchitectural performance anomalies?
- Is IPC suspiciously low?
- Are cache or branch misprediction rates abnormal?

```
Usage: pmu_anomalies.py [-i <pmu_data.json>] [options]

Options:
  -i, --input <path>        Path to PMU JSON data (default: stdin)
  --json                    Output JSON
```

Thresholds: IPC < 1.0, branch miss > 10%, L1-icache miss > 5%,
L1-dcache miss > 10%, dTLB miss > 3%.

#### `profile_to_json.py` — Perf Report Parser

Converts `perf report --stdio` output to structured JSON, extracting
per-function profiling data.

**Questions answered:**
- What are the hottest functions in a profiled binary?
- How are samples distributed across modules?

```
Usage: profile_to_json.py [-i <perf_report.txt>] [options]

Options:
  -i, --input <path>        Path to perf report output (default: stdin)
  -n, --top <N>             Show only top N entries
  -f, --filter <substr>     Filter to entries whose module contains substring
  --json                    Output JSON
```

#### `annotate_disasm.py` — Instruction-Level Hotspot Annotation

Produces hotness-annotated disassembly from `perf annotate` output, marking
hot instructions (>=1.0% samples) with `>>>`.

**Questions answered:**
- Which assembly instructions are the hottest?
- Where is the CPU spending the most time at the instruction level?

```
Usage: annotate_disasm.py [-i <perf_annotate.txt>] [options]

Options:
  -i, --input <path>        Path to perf annotate --stdio output (default: stdin)
  -f, --function <name>     Filter to a specific function (substring match)
  --json                    Output JSON
```

#### `annotate_source.py` — Source-Level Hotspot Annotation

Overlays line-level profile data onto source code, mapping `perf annotate`
sample counts back to source lines and marking hot lines with `[HOT X.X%]`.

**Questions answered:**
- Which source code lines consume the most CPU time?
- Where are the hotspots in the source (not assembly) view?

```
Usage: annotate_source.py [-i <perf_annotate.txt>] [options]

Options:
  -i, --input <path>        Path to perf annotate --stdio output (default: stdin)
  -f, --function <name>     Filter to a specific function (substring match)
  --json                    Output JSON
```

#### `runtime_call_profiler.py` — Runtime Function Profiling

Profiles `_sh_*` runtime function usage from `perf report` output. Ranks by
sample count, groups by category, and identifies top optimization candidates.

**Questions answered:**
- Which `_sh_*` runtime functions consume the most CPU?
- Which category (property access, GC, arithmetic) dominates?
- What are the top optimization candidates?

```
Usage: runtime_call_profiler.py [-i <perf_report.txt>] [options]

Options:
  -i, --input <path>        Path to perf report --stdio output (default: stdin)
  -n, --top <N>             Number of top candidates to show (default: 20)
  --json                    Output JSON
```

---

### Anti-Pattern Scanners

#### `js_antipattern_scan.py` — JavaScript Anti-Patterns

Scans JavaScript source for patterns that exercise slow engine paths in
Hermes/Static Hermes, preventing effective AOT compilation.

**Questions answered:**
- Does this JS code use patterns that are slow in Hermes?
- Are there uses of `eval`, `with`, `arguments`, `delete`?
- Are there sparse arrays or megamorphic property accesses?

```
Usage: js_antipattern_scan.py -i <file_or_dir> [options]

Options:
  -i, --input <path>        JS file or directory to scan (required)
  --json                    Output JSON
```

Detected patterns: `eval_usage`, `with_statement`,
`arguments_materialization`, `megamorphic_access`, `dynamic_property_name`,
`prototype_pollution`, `delete_operator`, `try_catch_in_loop`,
`sparse_array`, `implicit_global`.

#### `antipattern_scan.py` — C++ Performance Anti-Patterns

Scans C++ source files for common performance anti-patterns. Optionally
cross-references with profiling data to focus on hot functions.

**Questions answered:**
- Does this C++ code contain known performance anti-patterns?
- Are large types being passed by value?
- Are there redundant container lookups?

```
Usage: antipattern_scan.py -i <file_or_dir> [options]

Options:
  -i, --input <path>        C++ source file or directory (required)
  -p, --profile <path>      Profile JSON for cross-referencing hot functions
  --json                    Output JSON
```

Detected patterns: `pass_by_value`, `repeated_lookup`,
`string_concat_loop`, `virtual_dispatch_hot`, `unnecessary_copy`,
`redundant_find_access`.

---

### Compiler Analysis

#### `compiler_stats.sh` — Pass Statistics

Captures compiler pass statistics from `shermes --print-stats`, including
LLVM `STATISTIC()` counters and pass timing.

**Questions answered:**
- How many inlining decisions were made?
- How much time does each pass take?
- What are the DCE/CSE/type inference counters?

```
Usage: compiler_stats.sh <shermes_binary> <workload.js> [options]

Options:
  --typed                   Compile with --typed flag
  --json                    Output structured JSON
```

#### `compilation_profiler.sh` — Pipeline Stage Timing

Measures time for each compilation stage independently: JS-to-C, C-to-object,
and full pipeline (including linking).

**Questions answered:**
- Which compilation stage is the bottleneck?
- How long does JS-to-C take vs. C compilation vs. linking?
- What are the file sizes at each stage?

```
Usage: compilation_profiler.sh <shermes_binary> <workload.js> [options]

Options:
  --typed                   Compile with --typed flag
  --json                    Output structured JSON
```

#### `pass_experiment.sh` — Optimization Pass Marginal Value

Measures the marginal value of a specific optimization pass by comparing the
full pipeline vs. pipeline with that pass excluded.

**Questions answered:**
- How much does a specific pass (inlining, DCE, CSE) contribute?
- What happens to code size and performance if we disable it?

```
Usage: pass_experiment.sh <shermes_binary> <workload.js> --disable <pass> [options]

Arguments:
  <shermes_binary>          Path to shermes (required)
  <workload.js>             JavaScript file (required)

Options:
  --disable <pass_name>     Pass to exclude (required)
  --typed                   Compile with --typed flag
  --json                    Output structured JSON
```

Available passes: `dce`, `cse`, `mem2reg`, `instsimplify`, `simplifycfg`,
`stackpromotion`, `typeinference`, `inlining`, `codemotion`, `funcsigopts`,
`tdzdedup`.

#### `regalloc_report.py` — Register Allocation Quality

Analyzes register allocation quality from `shermes -dump-ra` output. Reports
per-function register count, spill count, and max register pressure.

**Questions answered:**
- How many registers does each function use?
- Which functions have the most spills?
- Are there register allocation quality problems?

```
Usage: regalloc_report.py [-i <dump_ra.txt>] [options]

Options:
  -i, --input <path>        Path to shermes -dump-ra output (default: stdin)
  --json                    Output JSON
```

---

### GC Analysis

#### `gc_stats.sh` — Garbage Collection Statistics

Compiles JS, runs the native binary, and captures GC statistics from stderr.
Computes allocation rate, GC overhead %, and survival ratio.

**Questions answered:**
- What is the GC overhead for this workload?
- What is the allocation rate?
- How does GC behavior differ between two builds?

```
Usage: gc_stats.sh <shermes_binary> <workload.js> [options]

Options:
  --typed                   Compile with --typed flag
  --json                    Output structured JSON
  --compare <shermes_b>     A/B comparison with second shermes binary
```

---

### Multi-Metric

#### `perf_proxies.sh` — Quick Performance Health Check

Fast multi-dimensional performance assessment without requiring hardware
counters. Measures code size, call density, and compilation latency.

**Questions answered:**
- What is the overall "performance health" of a shermes build?
- How do code size, call density, and compilation latency compare?

```
Usage: perf_proxies.sh <shermes> <js_file_or_dir> [options]

Options:
  --typed                   Compile with --typed flag
  --compare <shermes_b>     A/B comparison mode
  --json                    Output structured JSON
```

Metrics: generated C size (lines/bytes), native binary size, runtime call
density (`_sh_*` calls), compilation latency (ms per stage), function count
and average size.

---

## Recommended Workflows

### Workflow 1: Performance Regression Check

Quick check for performance regressions between two shermes builds.

```bash
# Statistical A/B comparison with significance testing
agent-perf/tools/benchmark_compare.sh <baseline> <candidate> --suite all --json

# Deterministic instruction count comparison (most stable metric)
agent-perf/tools/count_instructions.sh benchmarks/octane/richards.js \
  --compare <baseline> <candidate> --iterations 21
```

### Workflow 2: Binary Size Impact Assessment

Quantify code size cost of an optimization that inlines code into generated C.

```bash
# Step 1: A/B binary size across Octane
agent-perf/tools/binary_size_compare.sh \
  --baseline <shermes_a> --optimized <shermes_b> --suite octane

# Step 2: Symbol-level drill-down on a specific benchmark
agent-perf/tools/binary_size_analyzer.py \
  --baseline baseline_richards.o --optimized optimized_richards.o
```

### Workflow 3: Generated Code Quality Analysis

Understand what the compiler produces and find optimization opportunities.

```bash
# Generate C code
shermes -emit-c -o /tmp/out.c workload.js

# Analyze runtime calls by category
agent-perf/tools/generated_code_analyzer.py -i /tmp/out.c --json

# Find code patterns that inhibit C compiler optimization
agent-perf/tools/generated_code_antipatterns.py -i /tmp/out.c

# Map hotspots back to JS source
agent-perf/tools/sh_source_mapper.py -i /tmp/out.c --c-line 150
```

### Workflow 4: Typed Mode Effectiveness

Measure how much type information improves generated code.

```bash
# Generate both typed and untyped C code
shermes -emit-c -o /tmp/untyped.c workload.js
shermes -emit-c --typed -o /tmp/typed.c workload.js

# Compare runtime call elimination
agent-perf/tools/type_coverage_analyzer.py -t /tmp/typed.c -u /tmp/untyped.c --json
```

### Workflow 5: Deep Profiling Investigation

Full profiling workflow from PMU counters to source-level hotspots.

```bash
# Step 1: Collect PMU counters
agent-perf/tools/collect_pmu.sh <shermes> workload.js --json > /tmp/pmu.json

# Step 2: Check for anomalies
agent-perf/tools/pmu_anomalies.py -i /tmp/pmu.json

# Step 3: Profile with perf
perf record -g -- ./compiled_binary
perf report --stdio > /tmp/report.txt

# Step 4: Parse profile and find hot runtime functions
agent-perf/tools/profile_to_json.py -i /tmp/report.txt --json
agent-perf/tools/runtime_call_profiler.py -i /tmp/report.txt --json

# Step 5: Annotate source with hotspots
perf annotate --stdio > /tmp/annotate.txt
agent-perf/tools/annotate_source.py -i /tmp/annotate.txt
```

### Workflow 6: Compiler Pass Analysis

Evaluate the contribution of individual optimization passes.

```bash
# Measure marginal value of inlining
agent-perf/tools/pass_experiment.sh <shermes> workload.js --disable inlining --json

# Compare pass statistics between builds
agent-perf/tools/compiler_stats.sh <shermes> workload.js --json
```

---

## Tool Summary Table

| Tool | Category | Questions Answered |
|------|----------|--------------------|
| `run_benchmarks.sh` | Benchmark | How fast is this build? What are the ops/sec? |
| `benchmark_compare.sh` | Benchmark | Is B faster/slower than A? Statistically significant? |
| `count_instructions.sh` | Benchmark | How many instructions? Deterministic delta? |
| `binary_size_compare.sh` | Binary Size | .text growth? Call sites? Per-site overhead? |
| `binary_size_analyzer.py` | Binary Size | Which symbols grew? What category? |
| `generated_code_analyzer.py` | Code Analysis | Runtime call categories? Call density? Patterns? |
| `generated_code_antipatterns.py` | Code Analysis | C optimization inhibitors? GC root waste? |
| `generated_code_report.sh` | Code Analysis | Per-function metrics? A/B code diff? |
| `sh_source_mapper.py` | Code Analysis | JS line for C line? C lines for JS line? |
| `type_coverage_analyzer.py` | Code Analysis | Type coverage %? Eliminated calls? |
| `collect_pmu.sh` | Profiling | IPC? Cache/branch/TLB miss rates? |
| `pmu_anomalies.py` | Profiling | Microarch anomalies? Abnormal rates? |
| `profile_to_json.py` | Profiling | Hottest functions? Sample distribution? |
| `annotate_disasm.py` | Profiling | Hot assembly instructions? |
| `annotate_source.py` | Profiling | Hot source lines? |
| `runtime_call_profiler.py` | Profiling | Hot `_sh_*` functions? Top optimization targets? |
| `js_antipattern_scan.py` | Anti-Pattern | Slow JS patterns? eval/with/arguments? |
| `antipattern_scan.py` | Anti-Pattern | C++ anti-patterns? Pass-by-value? |
| `compiler_stats.sh` | Compiler | Pass counters? DCE/CSE/inline stats? |
| `compilation_profiler.sh` | Compiler | Stage bottleneck? JS-to-C vs C vs link? |
| `pass_experiment.sh` | Compiler | Marginal value of a pass? |
| `regalloc_report.py` | Compiler | Register pressure? Spill count? |
| `gc_stats.sh` | GC | GC overhead %? Allocation rate? |
| `perf_proxies.sh` | Multi-Metric | Overall health? Code size + density + latency? |

## Related Files

- `.llms/rules/sh_perf_benchmarking.md` — Performance benchmarking rules (opt-mode, metrics, statistics)
- `.llms/rules/sh_backend.md` — SH code generation rules
- `.llms/rules/sh_runtime.md` — SH runtime library rules
- `.llms/rules/sh_type_system.md` — Typed mode rules
- `.llms/rules/sh_compiler_pipeline.md` — Compilation pipeline rules
- `.llms/skills/sh_perf_investigation.md` — Performance investigation workflow
- `schemas/perf_schema.json` — JSON output format documentation

## Testing

All Python tools have unit tests in `tests/` using Python's built-in `unittest`
module. Tests use synthetic input data — no shermes binary, perf counters, or
build system required.

```bash
# Run ALL tests (from xplat/static_h/)
python3 -m unittest discover -s agent-perf/tests -p 'test_*.py' -v

# Run a single test file
python3 -m unittest agent-perf/tests/test_generated_code_analyzer.py -v

# Run a specific test class
python3 -m unittest agent-perf.tests.test_generated_code_analyzer.TestAnalyzeGeneratedCode -v

# Run a specific test method
python3 -m unittest agent-perf.tests.test_generated_code_analyzer.TestAnalyzeGeneratedCode.test_basic_analysis -v

# Validate all shell scripts (syntax check only)
for f in agent-perf/tools/*.sh; do bash -n "$f"; done
```

**Important:** Always run from the `xplat/static_h/` directory. Tests import
tools via relative path (`sys.path.insert` to `agent-perf/tools/`).
