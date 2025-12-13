# Static Hermes Update, December 2024

*December 19, 2024 · tmikov*

Throughout much of 2024, the Hermes team has been focused on improving both the performance and ECMAScript compliance of untyped JavaScript. While the primary goal of the Static Hermes project is to achieve native-like performance with typed JavaScript, we recognize that a significant amount of untyped code must also execute correctly and efficiently.

This post highlights some of the key improvements we've made in the static_h branch of Hermes. The static_h branch supports both typed and untyped compilation and execution. We are working toward making the static_h branch opt-in initially, and later the default React Native engine next year.

## Major Advances in ECMAScript Compliance

**Full spec-compliant implementation of ES6 classes.** This does not include private fields and static blocks, but support is already under way.

Our classes implementation is ~2.3x faster at constructing instances of a class, and ~4.5x faster in super invocations compared to Babel's strict transform. Additionally, we are ~1.35x and ~1.15x, respectively, faster than Babel's loose transform, all while remaining fully spec-compliant.

**Full block scoping support for `let`, `const` and Temporal Dead Zone (TDZ)**, with an option to disable TDZ for increased performance.

These features have been a long time coming and represent a significant step forward in closing the gap in Hermes' functionality—though this gap has largely been mitigated by React Native's Babel pipeline. Now, we are finally here.

## Untyped JS Performance Wins

The table below shows the "times x" performance improvements in Octane benchmarks of the static_h branch compared to the main Hermes branch. The first column represents performance with the regular interpreter, while the second column shows results with JIT enabled.

The last three benchmarks are custom additions by our team. These benchmarks highlight, among other things, the dramatic performance improvements of the JIT when more type information is available.

In the InterpDispatch2 and Mandelbrot benchmarks, our compiler was able to statically infer many types, resulting in significant speed-ups. With typed JavaScript, these numbers are expected to climb even higher.

| Benchmark        | Normal | JIT  |
|------------------|--------|------|
| Richards         | 1.24   | 1.78 |
| DeltaBlue        | 1.29   | 1.86 |
| Crypto           | 1.34   | 1.91 |
| RayTrace         | 1.72   | 2.07 |
| EarleyBoyer      | 1.49   | 1.71 |
| RegExp           | 1.20   | 1.20 |
| Splay            | 1.22   | 1.33 |
| SplayLatency     | 1.09   | 1.15 |
| NavierStokes     | 1.50   | 1.87 |
| PdfJS            | 1.68   | 1.78 |
| Mandreel         | 1.38   | 1.69 |
| MandreelLatency  | 1.36   | 1.42 |
| Gameboy          | 1.36   | 1.68 |
| Box2D            | 1.41   | 1.93 |
| zlib             | 1.65   | 2.04 |
| Typescript       | 1.28   | 1.46 |
| InterpDispatch   | 1.66   | 2.97 |
| InterpDispatch2  | 1.58   | 7.90 |
| Mandelbrot       | 1.25   | 3.29 |
