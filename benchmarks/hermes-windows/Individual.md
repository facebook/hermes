# Individual Benchmarks

Standalone JS files that can be run directly with `hermes` (not through bench-runner.py).

### Typed mode

Files with Flow type annotations need the `-typed` flag (or at minimum `-parse-flow`):
- `-typed` — parses Flow types **and** uses them for optimization (faster execution)
- `-parse-flow` — parses and strips Flow types without optimization (same speed as untyped)

```bash
hermes -typed benchmarks/many-subclasses/many-sh-1.js    # optimized (e.g. 8033ms)
hermes -parse-flow benchmarks/many-subclasses/many-sh-1.js  # unoptimized (e.g. 18397ms)
hermes benchmarks/many-subclasses/many.js                # plain JS (e.g. 18556ms)
```

Some files still fail even with `-typed` due to unsupported features (ES module imports, unimplemented type annotations). These are marked below.

## micros/

- micros/getNodeById.js: `Time: {ms}`
- micros/setInsert.js: `Time: {ms}`
- micros/stringify-number.js: `Time: {ms}`
- micros/typed-array-sort.js: `{TypeName}: {ms} ms` (one line per typed array type, e.g. `Int8Array       : 1239 ms`)

## jit-benches/

- jit-benches/idisp.js: `Time: {ms}`
- jit-benches/idispn.js: `Time: {ms}`

## many-subclasses/

- many-subclasses/many.js: `{result} M={num-subclasses}, N={iterations}, time={ms}`
- many-subclasses/many-sh-1.js: `{result} M={num-subclasses}, N={iterations}, time={ms}` (requires `-typed`)
- many-subclasses/many-sh-2.js: `{result} M={num-subclasses}, N={iterations}, time={ms}` (requires `-typed`)
- many-subclasses/many-sh-3.js: `{result} M={num-subclasses}, N={iterations}, time={ms}` (requires `-typed`)
- many-subclasses/many-sh-4.js: `{result} M={num-subclasses}, N={iterations}, time={ms}` (requires `-typed`)

## map-objects/

- map-objects/map-objects-untyped.js: `{ms} ms {N} iterations`
- map-objects/map-objects-typed.js: `{ms} ms {N} iterations` (requires `-typed`)

## map-strings/

- map-strings/map-strings-untyped.js: `{ms} ms {N} iterations`
- map-strings/map-strings-typed.js: `{ms} ms {N} iterations` (requires `-typed`)

## nbody/

- nbody/original/nbody.js: `{energy-value}` (correctness check only, no timing output)
- nbody/fully-typed/nbody.js: `{energy-value}` (requires `-typed`, correctness check only, no timing output)
- nbody/fully-typed/nbody.ts: `{energy-value}` (requires `-parse-ts` or `-transform-ts`, correctness check only, no timing output)

## string-switch/

- string-switch/plain/bench.js: `Switch {ms}`, `Object {ms}`, `Map    {ms}` (three lines comparing approaches)

## raytracer/

- raytracer/original/bench-raytracer.js: `exec time:  {ms} ms`
- raytracer/original/raytracer.ts: `exec time:  {ms} ms` (requires `-parse-ts`, parameter properties rewritten)
- raytracer/typed/: **requires shermes + CMake build** (compiled typed JS + C++ GUI, not available on Windows)

## MiniReact/

- MiniReact/no-objects/out/simple-stripped.js: `{ms} ms` (also renders HTML to stdout)
- MiniReact/no-objects/out/simple-lowered.js: `{ms} ms` (also renders HTML to stdout)
- MiniReact/no-objects/out/music-stripped.js: renders HTML only, no timing output
- MiniReact/no-objects/out/music-lowered.js: renders HTML only, no timing output
- MiniReact/no-deps/stripped/MiniReact.js: no output
- MiniReact/no-deps/MiniReact.js: **does not work** (unsupported type annotations even with `-typed`)
- MiniReact/original/MiniReact.js: **does not work** (ES module imports not supported)
- MiniReact/no-objects/out/simple.js: **does not work** (crashes with `-typed`)
- MiniReact/no-objects/out/music.js: **does not work** (unsupported type annotations even with `-typed`)

## widgets/

- widgets/simple-classes/widgets.js: `{ms} ms {N} iterations` (requires `-typed`)
- widgets/original/es5/widgets.js: `{ms} ms {N} iterations` (webpack-bundled ES5 variant)
- widgets/single-file/es5/widgets.js: `{ms} ms {N} iterations` (Babel-lowered ES5 variant)
- widgets/original/app_runner.js: **does not work** (ES module imports; use `es5/widgets.js` instead)
- widgets/single-file/widgets.js: **does not work** (unsupported type annotations; use `es5/widgets.js` instead)

## octane/ (Google Octane v2, BenchmarkSuite scoring)

- octane/box2d.js: `Box2D {score}`
- octane/crypto.js: `Crypto {score}`
- octane/deltablue.js: `DeltaBlue {score}`
- octane/earley-boyer.js: `EarleyBoyer {score}`
- octane/gbemu.js: `Gameboy {score}`
- octane/mandreel.js: `Mandreel {score}`
- octane/mandreel_latency.js: `MandreelLatency {score}`
- octane/navier-stokes.js: `NavierStokes {score}`
- octane/pdfjs.js: `PdfJS {score}`
- octane/raytrace.js: `RayTrace {score}`
- octane/regexp.js: `RegExp {score}`
- octane/richards.js: `Richards {score}`
- octane/splay.js: `Splay {score}`
- octane/splay_latency.js: `SplayLatency {score}`
- octane/typescript.js: `Typescript {score}`
- octane/zlib.js: `zlib {score}`

Note: Octane {score} is `reference / meanTime * 100` — higher = faster. The actual milliseconds are not printed. To get real time measurements, use the bench-runner versions in `bench-runner/resource/test-suites/octane/` instead. Some octane benchmarks emit compiler warnings to stderr but still run correctly.

### Overlap with bench-runner test suites

bench-runner `--cats v8 octane` covers most of these but not all:
- crypto, deltablue, raytrace, regexp, richards, splay — available in bench-runner under the `v8` category
- box2d, earley-boyer, navier-stokes, pdfjs, gbemu, code-load, typescript — available in bench-runner under the `octane` category
- mandreel, mandreel_latency, splay_latency, zlib — **only available here**, not in bench-runner
