# What's New in the Next Stable Release 260318099.0.0

*June 5, 2026 · zhaogang*

A roundup of the JavaScript features, performance wins, and fixes that have landed in the static_h branch and will be shipped in the next Hermes stable release 260318099.0.0.

## New Supported Features

- **Set operations:** `union`, `intersection`, `difference`, `symmetricDifference`, `isSubsetOf`, `isSupersetOf`, `isDisjointFrom`.
- **Grouping:** `Object.groupBy()` and `Map.groupBy()`.
- **Iterator Helpers:** `.map()`, `.filter()`, `.take()`, `.drop()`, `.flatMap()`, `.reduce()`, and more directly on iterators, plus `Iterator.from()`.
- **Async iteration:** , `for await...of` (with `Symbol.asyncIterator`) and async generator (not tested in production, don't use except for testing).
- **`Promise.withResolvers()`** — create a promise together with its `resolve`/`reject` functions, without the executor boilerplate.
- **`FinalizationRegistry`**, and `WeakRef` / `WeakMap` / `WeakSet` now accept symbols.
- **`TextDecoder`** now ships with the engine (no polyfill needed).
- **Strings:** `String.prototype.isWellFormed()` and `toWellFormed()`.
- **More:** `Error.isError()`, `Math.sumPrecise()`, `ArrayBuffer.prototype.detached`, and BigInt keys in object literals.

## TypeScript

- Strip type-only TypeScript syntax directly with the `--transform-ts` flag (type erasure, similar to Node.js's `--strip-types`) — no separate Babel pass required.

## Bug fixes

A batch of correctness and stability fixes also landed, including:

- TypedArray and DataView access on detached `ArrayBuffer`s
- `Array.prototype.sort` and `toSpliced` on very large arrays
- RegExp `/iu` Unicode case-folding
- `Proxy` invariant checks
- `Symbol.description` spec compliance

## Faster

- **`JSON.parse` / `JSON.stringify` are significantly faster** — see [JSON Parsing Performance: 2.7x-3.4x Faster](2025-11-24-json-parsing-performance.md).
- **`Map` and `Set` are faster** — the backing hash table (`OrderedHashMap`) was reworked around a contiguous data table, with the index moved off the GC heap, for quicker iteration and insertion and less GC overhead.
- Faster object/property access and array operations, plus expanded JIT coverage.

## Also

- **Unicode updated to 17.0** — newer emoji and scripts across string and RegExp APIs.
- **Larger heaps:** the GC can now manage more than 4 GB on 64-bit devices with HV64, lifting the old ~4 GB ceiling for memory-heavy workloads. (The 32-bit compressed-pointer configuration stays bounded by design.)

## JSI updates for native modules

- **Runtime data APIs** — `setRuntimeData()` / `getRuntimeData()` give native modules per-runtime storage without fragile `Runtime*`-to-data maps. See [Introducing JSI's New Runtime Data APIs](2025-06-09-jsi-runtime-data-apis.md).
- **`ISerialization`** — efficient binary encoding of JS values for passing between runtimes, including serialize-with-transfer and external (zero-copy) `ArrayBuffer` data. See [ISerialization: Efficient Binary Encoding for JS Values](2025-12-02-iserialization.md).

## Bytecode version changed (98 → 99)

We bumped the bytecode version to 99. So if you use prebuilt bytecode, you'll need to rebuild it with the new version of Hermes.
