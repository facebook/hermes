# nbody benchmark

The Computer Language Benchmarks Game
https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
contributed by Isaac Gouy
modified by Andrey Filatkin

License is BSD 3-Clause, see LICENSE.md

Tests floating point numerical computation and reading properties from an object.

There are type-annotated versions in `fully-typed` directory.

- fully-typed/nbody.js: the original file was type-annotated with Flow types. `Body` was turned into ES6 class.
- fully-typed/nbody.ts: copied from the flow-annotated file. The type declaration for built-in functions (e.g. `print()`) are not yet available, so the use site was `ts-ignore`d.
