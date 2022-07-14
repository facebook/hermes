/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIMEFLAGS_H
#define HERMES_VM_RUNTIMEFLAGS_H

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/CommandLine.h"

#include "hermes/ConsoleHost/MemorySizeParser.h"
#include "hermes/ConsoleHost/RandomSeedParser.h"
#include "hermes/Public/RuntimeConfig.h"

#include <string>

/// This file defines flags that are specific to the Hermes runtime,
/// independent of the Hermes compiler.  Such flags must be defined in the
/// hermes and hvm binaries, so this file is included in all of those.

namespace cl {

using llvh::cl::cat;
using llvh::cl::desc;
using llvh::cl::Hidden;
using llvh::cl::init;
using llvh::cl::opt;
using llvh::cl::Option;
using llvh::cl::OptionCategory;
using llvh::cl::parser;

using hermes::vm::GCConfig;
using hermes::vm::RuntimeConfig;

static OptionCategory GCCategory(
    "Garbage Collector Options",
    "These control various parts of the GC.");

static OptionCategory RuntimeCategory(
    "Runtime Options",
    "These control aspects of the VM when it is being run.");

static opt<double> GCSanitizeRate(
    "gc-sanitize-handles",
    desc("A probability between 0 and 1 inclusive which indicates the chance "
         "that we do handle sanitization at a given allocation. "
         "Sanitization moves the heap to a new location. With ASAN "
         "enabled, this causes accesses via stale pointers into the heap to "
         "be sanitized."),
    cat(GCCategory),
#ifdef HERMESVM_SANITIZE_HANDLES
    init(0.01)
#else
    init(0.0),
    Hidden
#endif
);

static opt<int64_t, false, RandomSeedParser> GCSanitizeRandomSeed(
    "gc-sanitize-handles-random-seed",
    desc("A number used as a seed to the random engine for handle sanitization."
         "A negative value means to choose the seed at random"),
    cat(GCCategory),
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
    cat(GCCategory),
    init(false));

static opt<MemorySize, false, MemorySizeParser> MinHeapSize(
    "gc-min-heap",
    desc("Minimum heap size.  Format: <unsigned>{{K,M,G}{iB}"),
    cat(GCCategory),
    init(MemorySize{0}));

static opt<MemorySize, false, MemorySizeParser> InitHeapSize(
    "gc-init-heap",
    desc("Initial heap size.  Format: <unsigned>{{K,M,G}{iB}"),
    cat(GCCategory),
    init(MemorySize{1024 * 1024}));

static opt<double> OccupancyTarget(
    "occupancy-target",
    desc("Sizing heuristic: fraction of heap to be occupied by live data."),
    cat(GCCategory),
    init(GCConfig::getDefaultOccupancyTarget()));

static opt<bool> SampleProfiling(
    "sample-profiling",
    init(false),
    desc("Enable sampling profiler"),
    cat(RuntimeCategory));

static opt<MemorySize, false, MemorySizeParser> MaxHeapSize(
    "gc-max-heap",
    desc("Max heap size.  Format: <unsigned>{K,M,G}{iB}"),
    cat(GCCategory),
    init(MemorySize{1024 * 1024 * 1024}));

#ifdef HERMESVM_PROFILER_EXTERN
static opt<bool> PatchProfilerSymbols(
    "patch-profiler-symbols",
    desc("Patch profiler symbols in the executable at exit, "
         "instead of writing to symbol_dump.map file."),
    init(false),
    cat(RuntimeCategory));

static opt<std::string> ProfilerSymbolsFile(
    "profiler-symbols-file",
    desc("Dump profiler symbols in specified file at exit, "
         "instead of writing the symbol_dump.map file."),
    init("symbol_dump.map"),
    cat(RuntimeCategory));
#endif

static opt<bool> ES6Promise(
    "Xes6-promise",
    desc("Enable support for ES6 Promise"),
    init(RuntimeConfig::getDefaultES6Promise()),
    cat(RuntimeCategory));

static opt<bool> ES6Proxy(
    "Xes6-proxy",
    desc("Enable support for ES6 Proxy"),
    init(RuntimeConfig::getDefaultES6Proxy()),
    cat(RuntimeCategory));

static opt<bool> Intl(
    "Xintl",
    desc("Enable support for ECMA-402 Intl APIs"),
    init(RuntimeConfig::getDefaultIntl()),
    cat(RuntimeCategory));

static opt<bool> MicrotaskQueue(
    "Xmicrotask-queue",
    desc("Enable support for using microtasks"),
    init(RuntimeConfig::getDefaultMicrotaskQueue()),
    cat(RuntimeCategory));

static llvh::cl::opt<bool> StopAfterInit(
    "stop-after-module-init",
    llvh::cl::desc("Exit once module loading is finished. Useful "
                   "to measure module initialization time"),
    cat(RuntimeCategory));

static opt<bool> TrackBytecodeIO(
    "track-io",
    desc(
        "Track bytecode I/O when executing bytecode. Only works with bytecode mode"),
    cat(RuntimeCategory));

static opt<bool> StableInstructionCount(
    "Xstable-instruction-count",
    init(false),
    Hidden,
    desc("For CPU instructions debugging: fix random seed, silence logging"),
    cat(RuntimeCategory));

static opt<uint32_t> VMExperimentFlags(
    "Xvm-experiment-flags",
    llvh::cl::desc("VM experiment flags."),
    llvh::cl::init(RuntimeConfig::getDefaultVMExperimentFlags()),
    llvh::cl::Hidden,
    cat(RuntimeCategory));

static opt<bool> EnableHermesInternal(
    "enable-hermes-internal",
    llvh::cl::desc("Enable the HermesInternal object."),
    llvh::cl::init(RuntimeConfig::getDefaultEnableHermesInternal()));
static opt<bool> EnableHermesInternalTestMethods(
    "Xhermes-internal-test-methods",
    llvh::cl::desc("Enable the HermesInternal test methods."),
    llvh::cl::init(RuntimeConfig::getDefaultEnableHermesInternalTestMethods()),
    llvh::cl::Hidden);

static opt<bool> HeapTimeline(
    "Xheap-timeline",
    llvh::cl::desc(
        "Track heap allocation stacks and add them to the output of createHeapSnapshot()"),
    llvh::cl::init(false),
    cat(RuntimeCategory));

} // namespace cl

#endif // HERMES_VM_RUNTIMEFLAGS_H
