# Benchmarks

Runs JS benchmarks and compares results across engines or builds. Full docs: `benchmarks/bench-runner/README.md`.

## PowerShell Scripts

All scripts live in `benchmarks/hermes-windows/` and use `$PSScriptRoot` for paths, so they work from any working directory. They expect `build/ninja-clang-release/bin/hermes.exe` to exist.

### bench-all.ps1

Runs everything (test suites + individual benchmarks) and merges into one JSON file.

```powershell
.\bench-all.ps1 -c 5 -l baseline -output results.json
```

- `-c` — number of iterations per benchmark
- `-l` — label (e.g. "before", "after") included in the JSON output
- `-output` — output JSON file path

### bench-test-suits.ps1

Runs only the bench-runner test suites (`v8`, `octane`, `micros` categories). Wraps `bench-runner.py`.

```powershell
.\bench-test-suits.ps1 -c 5 -l baseline -output test-suits.json
```

### bench-individual-samples.ps1

Runs only the standalone individual benchmarks (listed in `Individual.md`), multiple iterations.

```powershell
.\bench-individual-samples.ps1 -c 5 -output individual.json
```

### bench-individual.ps1

Single-run helper used by `bench-individual-samples.ps1`. Runs each individual benchmark once and outputs a flat JSON of `{ "path": ms }`.

```powershell
.\bench-individual.ps1 -output single-run.json
```

### format-json.ps1

Reformats a JSON file in-place with consistent 4-space indentation (fixes PowerShell's default formatting).

```powershell
.\format-json.ps1 results.json
```

### Comparing before/after

```powershell
# 1. Build baseline, run all benchmarks
.\bench-all.ps1 -c 5 -l before -output before.json

# 2. Make changes, rebuild, run again
.\bench-all.ps1 -c 5 -l after -output after.json

# 3. Compare test-suite results (bench-runner's merge tool)
python3 benchmarks/bench-runner/bench-merge.py before.json after.json
```

Note: `bench-merge.py` only compares bench-runner test suite results (it ignores individual benchmark entries). For individual benchmarks, compare the JSON files directly.

## How It Works

The benchmark JS files live in `benchmarks/bench-runner/resource/test-suites/`, organized by category:
- `v8-v6-perf/` — V8 benchmark suite (crypto, deltablue, raytrace, regexp, richards, splay)
- `octane/` — Octane suite (box2d, earley-boyer, navier-stokes, pdfjs, gbemu, code-load, typescript)
- `tsc/` — TypeScript compiler benchmark
- `micros/` — ~60 micro-benchmarks targeting specific operations (array, string, regexp, etc.)

Every JS file follows the same contract: do work, then print `Time: <milliseconds>` to stdout. For example, `micros/simpleSum.js`:
```js
var start = Date.now();
print(doSumNTimes(10000));
var end = Date.now();
print("Time: " + (end - start));
```

The runner (`bench-runner.py`) does the following for each benchmark:
1. **Compiles** the JS file to Hermes bytecode (`.hbc`) using: `hermes -O -Wno-undefined-variable -emit-binary`
2. **Warm-up run** — runs once to warm disk caches (discards result)
3. **Timed runs** — runs N times (set by `-c`), parses `Time: <ms>` from each run's stdout
4. **Reports** mean and stddev across runs

Benchmark names (used with `--bm`) and their JS file mappings are defined in `benchmarks/bench-runner/categories.py`.

## Windows Fork Differences

- **Script path**: `benchmarks/bench-runner/bench-runner.py` (not `xplat/static_h/benchmarks/...`)
- **Binary path**: Use local build, e.g. `build/ninja-clang-release/bin/hermes.exe`
- **`-gc-min-heap` unsupported**: Commented out in `benchmarks/bench-runner/runner.py`

## Running Benchmarks

```bash
python3 benchmarks/bench-runner/bench-runner.py --hermes -b build/ninja-clang-release/bin/hermes.exe {PARAMETERS}
```

Parameters:
- `--bm v8-crypto simpleSum` — run specific benchmarks by name
- `--cats v8` / `--cats octane` / `--cats v8 octane` — run all benchmarks in a category
- `-c 5` — run each benchmark 5 times (default 1)
- `-f json` / `-f tsv` — output format (default: `ascii` table to stdout)
- `--out results.json` — write results to file
- `-l name` — label the run as `name` (used for comparisons)

## Comparing Before/After

```bash
# Save labeled JSON runs, then merge
python3 benchmarks/bench-runner/bench-runner.py --hermes -b {BINARY} --cats v8 octane -l before -f json --out before.json -c 5
python3 benchmarks/bench-runner/bench-runner.py --hermes -b {BINARY} --cats v8 octane -l after -f json --out after.json -c 5
python3 benchmarks/bench-runner/bench-merge.py before.json after.json
```

## Available Categories

- `v8` — 6 benchmarks (crypto, deltablue, raytrace, regexp, richards, splay)
- `octane` — 7 benchmarks (box2d, earley-boyer, navier-stokes, pdfjs, gbemu, code-load, typescript)
- `tsc` — TypeScript compiler benchmark
- `micros` — ~60 micro-benchmarks (not run by default)

## Standalone Benchmarks

There are additional JS benchmarks outside of `bench-runner/` that are run directly with `hermes` (not through bench-runner.py). Each is a self-contained JS file you run with:

```bash
build/ninja-clang-release/bin/hermes benchmarks/<path-to-file>.js
```

| Directory | What it tests | Example |
|-----------|--------------|---------|
| `octane/` | Google Octane v2 suite (standalone versions) | `hermes benchmarks/octane/box2d.js` |
| `micros/` | Focused micro-benchmarks (tree search, set insert, stringify, typed array sort) | `hermes benchmarks/micros/getNodeById.js` |
| `jit-benches/` | JIT-targeted workloads | `hermes benchmarks/jit-benches/idisp.js` |
| `many-subclasses/` | Property access across many subclasses | `hermes benchmarks/many-subclasses/many.js` |
| `map-objects/` | Map with object keys | `hermes benchmarks/map-objects/map-objects-untyped.js` |
| `map-strings/` | Map with string keys | `hermes benchmarks/map-strings/map-strings-untyped.js` |
| `nbody/` | N-body physics simulation | `hermes benchmarks/nbody/original/nbody.js` |
| `string-switch/` | Switch vs object vs Map for property setting | `hermes benchmarks/string-switch/plain/bench.js` |
| `widgets/` | Widget rendering (classes, inheritance, arrays, Maps) | `hermes benchmarks/widgets/original/app_runner.js` |
| `MiniReact/` | React-like virtual DOM rendering | `hermes benchmarks/MiniReact/no-deps/MiniReact.js` |

Some directories also have typed variants (`*-sh-*.js`, `*-typed.js`) meant for Static Hermes (`shermes`), and C++ comparison implementations (`raytracer/typed/`, `sqlite-read/`, `imgui-demo/`) built via CMake.

## Examples

Run a single bench-runner benchmark (quick smoke test):
```bash
python3 benchmarks/bench-runner/bench-runner.py --hermes -b build/ninja-clang-release/bin/hermes.exe --bm v8-crypto
```

Run a standalone benchmark directly:
```bash
build/ninja-clang-release/bin/hermes.exe benchmarks/micros/getNodeById.js
```

Run a category with 3 iterations:
```bash
python3 benchmarks/bench-runner/bench-runner.py --hermes -b build/ninja-clang-release/bin/hermes.exe --cats v8 -c 3
```

Compare before/after a code change:
```bash
# 1. Run benchmarks on the baseline, save as labeled JSON
python3 benchmarks/bench-runner/bench-runner.py --hermes -b build/ninja-clang-release/bin/hermes.exe --cats v8 -c 3 -l before -f json --out before.json

# 2. Make changes, rebuild, run again
python3 benchmarks/bench-runner/bench-runner.py --hermes -b build/ninja-clang-release/bin/hermes.exe --cats v8 -c 3 -l after -f json --out after.json

# 3. Merge and compare — ratio < 1.0 means faster, > 1.0 means slower
python3 benchmarks/bench-runner/bench-merge.py before.json after.json
```
