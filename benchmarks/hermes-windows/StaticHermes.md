# Static Hermes (`--static`) Benchmark Support

## Status

All three fixes have been implemented. Both `--dynamic` (default) and `--static`
modes are fully functional for test suites and individual benchmarks.

## How It Works

### bench.ts

- `--dynamic` (default): uses `hermes.exe` to interpret JS / run bytecode
- `--static`: uses `shermes.exe` to compile JS to native code, then runs it

Individual benchmarks use **two-phase compile-then-run** in static mode:
1. **Compile** (not timed): `shermes -O [flags] -o <temp>.exe file.js`
2. **Run** (timed): `<temp>.exe`

This ensures wall-clock measurements reflect only execution time, not
compilation overhead.

### bench-runner.py

A new `--shermes` runtime flag selects `ShermesRunner` (in
`benchmarks/bench-runner/shermes_runner.py`), which:
1. Compiles each JS file to a native `.exe` via `shermes -O -o <temp>.exe`
2. Sets `PATH` to include the Hermes lib directories so the compiled exe can
   find `hermesvm.dll` and `shermes_console.dll`
3. Runs the native exe N times, parsing `Time: <ms>` from stdout

## Windows DLL Resolution

On Windows, shermes-compiled executables dynamically link `hermesvm.dll` and
`shermes_console.dll`.  Since Windows has no rpath, the runner adds these
directories to `PATH` before spawning the compiled exe:
- `build/ninja-clang-release/lib/` (hermesvm.dll)
- `build/ninja-clang-release/tools/shermes/` (shermes_console.dll)

## Files Changed

- `benchmarks/hermes-windows/bench.ts` — `--dynamic`/`--static` flags,
  two-phase compile-then-run for individual benchmarks
- `benchmarks/bench-runner/shermes_runner.py` — new `ShermesRunner` class
- `benchmarks/bench-runner/bench-runner.py` — `--shermes` runtime option
- `benchmarks/hermes-windows/README.md` — documentation
