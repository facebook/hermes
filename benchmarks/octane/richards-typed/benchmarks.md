# Richards Benchmark — Engine Comparison

400 iterations of the Richards task scheduler benchmark.
Measured on Apple Silicon (macOS), single run, 2026-04-24.

## Results

| Engine                          | Time (ms) | Relative |
|---------------------------------|----------:|:--------:|
| Node v24.4.1 (V8 JIT)          |        19 |   1.0x   |
| **Static Hermes typed native**  |        65 |   3.4x   |
| **Static Hermes typed JIT**     |        86 |   4.5x   |
| **Static Hermes typed interp**  |       189 |   9.9x   |
| Static Hermes untyped JIT       |       208 |  10.9x   |
| Static Hermes untyped native    |       295 |  15.5x   |
| Static Hermes untyped interp    |       433 |  22.8x   |
| Node v24.4.1 `--jitless`        |       518 |  27.3x   |
| Porffor 0.61.13 (AOT→Wasm)     |       549 |  28.9x   |
| QuickJS 2025-04-26              |       848 |  44.6x   |

## Key Takeaways

- Static Hermes typed native is **3.4x** slower than V8 JIT, but **13x** faster than QuickJS.
- Sound typing gives a **3–4x** speedup across all Static Hermes modes (interp, JIT, native).
- Node `--jitless` (interpreter-only V8) is comparable to QuickJS and Porffor.
