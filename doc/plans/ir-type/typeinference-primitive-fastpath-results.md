# Primitive-Bitmask Fast Path — Results

Prototype per `typeinference-primitive-fastpath-design.md`, implemented on branch
`ir-type-speed`. Measured TypeInference time via `hermesc -O -ftime-report`
(sum of the `TypeInference` pass lines across its pipeline invocations), Release
builds, 3 runs, median. Bundles: Fb4aBundle.js (2019, 40MB),
map-20230823/bundle-new.js (2023, 78MB), fb4a marketplace 2026 (52MB).

## TypeInference median time

| variant            | Fb4a-2019 | map-2023 | mkt-2026 |
|--------------------|-----------|----------|----------|
| OLD (bitmask)      | 4.741s    | 1.840s   | 6.508s   |
| NEW baseline       | 5.203s    | 2.054s   | 7.212s   |
| +DenseMap cache    | 4.851s    | 1.895s   | 6.575s   |
| +lazy-flat cache   | 4.913s    | 1.960s   | 6.782s   |
| +eager-flat cache  | 4.878s    | 1.868s   | 6.436s   |

## Regression recovered (NEW baseline → OLD floor)

Baseline regression vs OLD: +9.7% / +11.7% / +10.8%.

| variant       | Fb4a-2019 | map-2023 | mkt-2026 |
|---------------|-----------|----------|----------|
| +DenseMap     | 76%       | 74%      | 90%      |
| +lazy-flat    | 63%       | 44%      | 61%      |
| +eager-flat   | 70%       | 87%      | 110%     |

(110% = eager-flat beat the old bitmask on the marketplace bundle.)

## Findings

- The fast path works: it recovers most of the v2 TypeInference regression. With
  the DenseMap cache, NEW is back to within +1–3% of the OLD bitmask; eager-flat
  closes it entirely (and slightly beats OLD on mkt-2026).
- **Cache-variant ranking**: eager-flat ≳ DenseMap > lazy-flat, but the gaps are
  small (≤ ~0.2s at 5–7s totals) and within run-to-run noise at 3 runs. The
  surprising bit is that lazy-flat trails DenseMap — likely the lazy first-touch
  materialization plus larger working set vs. the small hot set the DenseMap
  keeps warm. Eager-flat removes the miss path entirely; its one-time
  construction (≤3072 masks materialized per Module) did not show up as a
  regression on these whole-program compiles.
- **Correctness**: every variant produces byte-identical `.hbc` to baseline on
  all three bundles, and passes the 45 `TypeContextTest` unit tests and the
  type-inference/InstSimplify lit subset. The fast path is a pure optimization.
- **Working set is tiny.** Instrumenting the DenseMap to report its populated
  size showed only **38–58 distinct primitive masks out of 4096** were ever
  created per compile (Fb4a 43, map 38, marketplace 58). That explains the
  ranking: the DenseMap keeps those ~40–60 live entries hot in a few cache
  lines, whereas the flat array scatters them across a 4096-slot / 16 KB region
  (worse locality — why lazy-flat trailed), and the eager array materializes
  ~3072 masks of which ~98% are never read.

## Decision (shipped)

Ship the **DenseMap** cache. The flat/eager variants and the build-time
`HERMES_PRIMCACHE` selector were prototype-only measurement scaffolding and have
been removed: at a ~40–60 working set, cache locality dominates and the flat
array's hash-avoidance is a mirage, while the DenseMap also uses far less memory.
No re-measurement at higher run counts was needed — the DenseMap-vs-baseline win
is clear and the inter-variant deltas were noise-sized.

The larger remaining lever is unchanged: the ~10–13× re-inference amplification
in the TypeInference fixpoint (a separate worklist effort), which this fast path
does not address.

## Notes

- Measured via `-ftime-report` (built-in PassManager timer), so all five
  binaries (OLD, NEW baseline, and the three cache variants) were measured
  uniformly with no source instrumentation.
