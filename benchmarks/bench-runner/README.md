# benchmark_runner

## Introduction

bench-runner is a tool to run benchmarks and compare results, between different engines, and between different versions of the same engine.  It is loosely based on gc_bench, but:

- Doesn't try to get detailed GC statistics -- it just uses elapsed time, as reported by the benchmarks themselves. (**NB**: any benchmark run in this framework must report it's time, printing a line of the form "Time: <ms>".  That is, the difference between a Date() created at the start and end of execution.)
- Does allow the benchmark to be run by multiple different engines.
- Does allow runs with the same engine, before/after some change, to be compared.

## Basic operation

To run the benchmarks:

```
python3 xplat/static_h/benchmarks/bench-runner/bench-runner.py --hermes -b ~/sworkspace/build_release/bin/hermes --cats v8
```

The result of this will be something like:

```
Ran each benchmark 1 times
Runtime:  hermes
General Stats:
                   Benchmark                       total
                                                  (time)
--------------------------------------------------------
                   v8-crypto           552.0  s +-  0.0%
                v8-deltablue           787.0  s +-  0.0%
                 v8-raytrace           163.0  s +-  0.0%
                   v8-regexp           329.0  s +-  0.0%
                 v8-richards           854.0  s +-  0.0%
                    v8-splay           139.0  s +-  0.0%
```

## Arguments

bench_runner shares many arguments with gc_bench.

You can specify the set of benchmarks to run, either individually:

- --benchmarks (or --bm) benchmark1 ... benchmarkN

or as a whole category:

- --categories (or --cats) cat1 ... catN

The benchmark and categories names are available via --help.

Other arguments:
- --count (-c) N: run each benchmark N times.
- --keep-tmp (-k): retain bytecode files for hermes.
- --verbose (-v): verbose output.
- --debug (-d): debug output.
- --hermes: use the hermes vm.
- --synth: use the synthetic benchmark interpreter.

Some arguments are new in bench-runner, or have added options:

- --binary (-b): path to the binary to run as the engine.  Before was just hermes or synthtrace; now can be an arbitrary engine, like v8 or jsc.

- --output-format (-f): format of the output.  Was "ascii" or "tab-separated".  To this we add "json": write the results as a json file, suitable as input for the *merger* program, described below.

- --out file: write the output to the specified file.  This is especially useful with "-f json".

- --v8, --v8jitless: as with --hermes, specify that you want to run with v8 (or v8 with the --jitless flag).  Note that this requires a --binary specification; it doesn't have a default for where v8 lives.

## Using bench-merge

Say you want to compare v8 and hermes at some point in time, or hermes before and after some modification.  Let's consider the latter case.  I built a release build of hermes in the "before" revision, and then ran, from the bench-runner directory:

```
python3 bench-runner.py --hermes -b ~/sworkspace/build_release/bin/hermes --cats v8 octane -l before -f json --out ~/tmp/before.json -c 5
```

Then I moved to the "after" revision, again built hermes, and ran:

```
python3 bench-runner.py --hermes -b ~/sworkspace/build_release/bin/hermes --cats v8 octane -l after -f json --out ~/tmp/after.json  -c 5

python3 bench-merge.py ~/tmp/before.json ~/tmp/after.json > ~/tmp/before-after.txt
```

The last output file, before-after.txt, has output like:

```
                      runner                   benchmark                  Total time                       ratio
================================================================================================================
             hermes (before)                       box2d          1677.4  s +-  0.8%                       1.000
              hermes (after)                       box2d          1653.4  s +-  0.3%                       0.986
----------------------------------------------------------------------------------------------------------------
             hermes (before)                   code-load          2376.0  s +-  1.5%                       1.000
              hermes (after)                   code-load          2433.0  s +-  4.8%                       1.024
...
             hermes (before)                    v8-splay           115.4  s +-  0.8%                       1.000
              hermes (after)                    v8-splay           113.8  s +-  1.7%                       0.986
----------------------------------------------------------------------------------------------------------------
             hermes (before)                     geomean                                                   1.000
              hermes (after)                     geomean                                                   0.988
```

## TBD:

- Benchmarks must be modified to measure and print their own elapsed time, in a known engine-independent way.  Will add more benchmarks.
- Add jsc.
- We should probably make the heap-size configs apply in some reasonable way to non-Hermes engines.
- Add labels for comparisons between different versions of the same runtime.  (The output above is from a later version that already has this.)
- Might want a way to run, say, JSC vs Hermes in one command.  (That might be a composite script of the commands in this diff.)
