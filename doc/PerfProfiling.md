---
id: perf-profiling
title: perf Profiling
---

This document describes how to profile the performance of Static Hermes.

## Profiling JIT

Currently, we rely on Linux perf tool for profiling JIT'ed code, and we only
enable JIT on ARM64 platform. So you need to have a Linux machine with ARM64.

### How To Profile

1. Recording the trace. `-Xperf-prof` is to enable perf profiling, `-Xperf-prof-dir`
is to specify the output directory (by default it's `/tmp` so if you don't want
to look into those generated files, you can skip this flag). `-k mono` means we
use CLOCK_MONOTONIC for clock id, as it's used in jitdump. Here we only sample
user space events, but you could also sample kernel events and also specify
different flags (e.g., -g for call graph).
2. Injecting the jitdump into the perf.data. This will create an ELF binary for
each JIT'ed function.
3. Reporting the trace.
```bash
perf record -g -k mono -e cycles:u ./bin/hermes ~/js/test.js -Xjit=force \
  -Xperf-prof -Xperf-prof-dir ./
perf inject -j -i perf.data -o perf.data.jitted
perf report -i perf.data.jitted --call-graph=fractal,callee --children
```
