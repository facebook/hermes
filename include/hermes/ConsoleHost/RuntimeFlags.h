/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_RUNTIMEFLAGS_H
#define HERMES_VM_RUNTIMEFLAGS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"

#include "hermes/ConsoleHost/MemorySizeParser.h"
#include "hermes/ConsoleHost/RandomSeedParser.h"

#include <string>

/// This file defines flags that are specific to the Hermes runtime,
/// independent of the Hermes compiler.  Such flags must be defined in the
/// hermes, hvm, and repl binaries, so this file is included in all of those.

namespace cl {

using llvm::cl::desc;
using llvm::cl::Hidden;
using llvm::cl::init;
using llvm::cl::opt;
using llvm::cl::Option;
using llvm::cl::parser;

static opt<double> GCSanitizeRate(
    "gc-sanitize-handles",
    desc("A probability between 0 and 1 inclusive which indicates the chance "
         "that we do handle sanitization at a given allocation. "
         "Sanitization moves the heap to a new location. With ASAN "
         "enabled, this causes accesses via stale pointers into the heap to "
         "be sanitized."),
#ifdef HERMESVM_SANITIZE_HANDLES
    init(0.001)
#else
    init(0.0),
    Hidden
#endif
);

static opt<int64_t, false, RandomSeedParser> GCSanitizeRandomSeed(
    "gc-sanitize-handles-random-seed",
    desc("A number used as a seed to the random engine for handle sanitization."
         "A negative value means to choose the seed at random"),
    init(-1)
#ifndef HERMESVM_SANITIZE_HANDLES
        ,
    Hidden
#endif
);

static opt<bool> GCRandomizeAllocSpace(
    "gc-randomize-alloc-space",
    desc(
        "For GC's, like GenGC, that can allocate in different spaces, randomize "
        "the choice of space."),
    init(false));

static opt<bool> GCPrintStats(
    "gc-print-stats",
    desc("Output summary garbage collection statistics at exit"),
    init(false));

static opt<bool> GCAllocYoung(
    "gc-alloc-young",
    desc("Determines whether to (initially) allocate in the young generation"),
    init(true));

static opt<bool> GCRevertToYGAtTTI(
    "gc-revert-to-yg-at-tti",
    desc("Determines whether to revert to young generation, if necessary, at "
         "TTI notification"),
    init(false));

static opt<MemorySize, false, MemorySizeParser> MinHeapSize(
    "gc-min-heap",
    desc("Minimum heap size.  Format: <unsigned>{{K,M,G}{iB}"),
    init(MemorySize{0}));

static opt<MemorySize, false, MemorySizeParser> InitHeapSize(
    "gc-init-heap",
    desc("Initial heap size.  Format: <unsigned>{{K,M,G}{iB}"),
    init(MemorySize{1024 * 1024}));

static opt<bool> SampleProfiling(
    "sample-profiling",
    init(false),
    desc("Enable sampling profiler"));

static opt<MemorySize, false, MemorySizeParser> MaxHeapSize(
    "gc-max-heap",
    desc("Max heap size.  Format: <unsigned>{K,M,G}{iB}"),
    init(MemorySize{1024 * 1024 * 1024}));

#ifdef HERMESVM_PROFILER_EXTERN
static opt<bool> PatchProfilerSymbols(
    "patch-profiler-symbols",
    desc("Patch profiler symbols in the executable at exit, "
         "instead of writing to symbol_dump.map file."),
    init(false));

static opt<std::string> ProfilerSymbolsFile(
    "profiler-symbols-file",
    desc("Dump profiler symbols in specified file at exit, "
         "instead of writing the symbol_dump.map file."),
    init("symbol_dump.map"));
#endif

static opt<bool>
    ES6Symbol("Xes6-symbol", desc("Enable support for ES6 Symbol"), init(true));

static llvm::cl::opt<bool> StopAfterInit(
    "stop-after-module-init",
    llvm::cl::desc("Exit once module loading is finished. Useful "
                   "to measure module initialization time"));

} // namespace cl

#endif // HERMES_VM_RUNTIMEFLAGS_H
