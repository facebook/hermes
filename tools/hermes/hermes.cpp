/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/ConsoleHost/RuntimeFlags.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PageAccessTracker.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/Allocator.h"
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/SHA1.h"
#include "llvh/Support/Signals.h"

#include "repl.h"

using namespace hermes;

namespace cl {
using llvh::cl::opt;

static opt<unsigned> Repeat(
    "Xrepeat",
    llvh::cl::desc("Repeat execution N number of times"),
    llvh::cl::init(1),
    llvh::cl::Hidden);

static opt<bool> RandomizeMemoryLayout(
    "Xrandomize-memory-layout",
    llvh::cl::desc("Randomize stack placement etc."),
    llvh::cl::init(false),
    llvh::cl::Hidden);

static opt<bool> GCAllocYoung(
    "gc-alloc-young",
    desc("Determines whether to (initially) allocate in the young generation"),
    cat(GCCategory),
    init(true));

static opt<bool> GCRevertToYGAtTTI(
    "gc-revert-to-yg-at-tti",
    desc("Determines whether to revert to young generation, if necessary, at "
         "TTI notification"),
    cat(GCCategory),
    init(false));

static opt<bool> GCBeforeStats(
    "gc-before-stats",
    desc("Perform a full GC just before printing statistics at exit"),
    cat(GCCategory),
    init(false));

static opt<bool> GCPrintStats(
    "gc-print-stats",
    desc("Output summary garbage collection statistics at exit"),
    cat(GCCategory),
    init(false));

static opt<unsigned> ExecutionTimeLimit(
    "time-limit",
    llvh::cl::desc("Number of milliseconds after which to abort JS exeuction"),
    llvh::cl::init(0));
} // namespace cl

/// Execute Hermes bytecode \p bytecode, respecting command line arguments.
/// \return an exit status.
static int executeHBCBytecodeFromCL(
    std::unique_ptr<hbc::BCProvider> bytecode,
    const driver::BytecodeBufferInfo &info) {
  auto recStats =
      (cl::GCPrintStats || cl::GCBeforeStats) && !cl::StableInstructionCount;
  ExecuteOptions options;
  options.runtimeConfig =
      vm::RuntimeConfig::Builder()
          .withGCConfig(vm::GCConfig::Builder()
                            .withMinHeapSize(cl::MinHeapSize.bytes)
                            .withInitHeapSize(cl::InitHeapSize.bytes)
                            .withMaxHeapSize(cl::MaxHeapSize.bytes)
                            .withOccupancyTarget(cl::OccupancyTarget)
                            .withSanitizeConfig(
                                vm::GCSanitizeConfig::Builder()
                                    .withSanitizeRate(cl::GCSanitizeRate)
                                    .withRandomSeed(cl::GCSanitizeRandomSeed)
                                    .build())
                            .withShouldRecordStats(recStats)
                            .withShouldReleaseUnused(vm::kReleaseUnusedNone)
                            .withAllocInYoung(cl::GCAllocYoung)
                            .withRevertToYGAtTTI(cl::GCRevertToYGAtTTI)
                            .build())
          .withEnableEval(cl::EnableEval)
          .withVerifyEvalIR(cl::VerifyIR)
          .withOptimizedEval(cl::OptimizedEval)
          .withAsyncBreakCheckInEval(cl::EmitAsyncBreakCheck)
          .withVMExperimentFlags(cl::VMExperimentFlags)
          .withES6Promise(cl::ES6Promise)
          .withES6Proxy(cl::ES6Proxy)
          .withIntl(cl::Intl)
          .withMicrotaskQueue(cl::MicrotaskQueue)
          .withEnableSampleProfiling(cl::SampleProfiling)
          .withRandomizeMemoryLayout(cl::RandomizeMemoryLayout)
          .withTrackIO(cl::TrackBytecodeIO)
          .withEnableHermesInternal(cl::EnableHermesInternal)
          .withEnableHermesInternalTestMethods(
              cl::EnableHermesInternalTestMethods)
          .build();

  options.basicBlockProfiling = cl::BasicBlockProfiling;

  options.stopAfterInit = false;
  options.timeLimit = cl::ExecutionTimeLimit;
  options.stopAfterInit = cl::StopAfterInit;
  options.forceGCBeforeStats = cl::GCBeforeStats;
  options.stabilizeInstructionCount = cl::StableInstructionCount;
  options.sampleProfiling = cl::SampleProfiling;
  options.heapTimeline = cl::HeapTimeline;

  bool success;
  if (cl::Repeat <= 1) {
    success = executeHBCBytecode(std::move(bytecode), options, &info.filename);
  } else {
    // The runtime is supposed to own the bytecode exclusively, but we
    // want to keep it around in this special case, so we can reuse it
    // between iterations.
    std::shared_ptr<hbc::BCProvider> sharedBytecode = std::move(bytecode);

    success = true;
    for (unsigned i = 0; i < cl::Repeat; ++i) {
      success &= executeHBCBytecode(
          std::shared_ptr<hbc::BCProvider>{sharedBytecode},
          options,
          &info.filename);
    }
  }
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

static vm::RuntimeConfig getReplRuntimeConfig() {
  return vm::RuntimeConfig::Builder()
      .withGCConfig(
          vm::GCConfig::Builder()
              .withInitHeapSize(cl::InitHeapSize.bytes)
              .withMaxHeapSize(cl::MaxHeapSize.bytes)
              .withSanitizeConfig(vm::GCSanitizeConfig::Builder()
                                      .withSanitizeRate(cl::GCSanitizeRate)
                                      .withRandomSeed(cl::GCSanitizeRandomSeed)
                                      .build())
              .withShouldRecordStats(cl::GCPrintStats)
              .build())
      .withVMExperimentFlags(cl::VMExperimentFlags)
      .withES6Promise(cl::ES6Promise)
      .withES6Proxy(cl::ES6Proxy)
      .withIntl(cl::Intl)
      .withMicrotaskQueue(cl::MicrotaskQueue)
      .withEnableHermesInternal(cl::EnableHermesInternal)
      .withEnableHermesInternalTestMethods(cl::EnableHermesInternalTestMethods)
      .build();
}

int main(int argc, char **argv) {
#ifndef HERMES_FBCODE_BUILD
  // Normalize the arg vector.
  llvh::InitLLVM initLLVM(argc, argv);
#else
  // When both HERMES_FBCODE_BUILD and sanitizers are enabled, InitLLVM may have
  // been already created and destroyed before main() is invoked. This presents
  // a problem because InitLLVM can't be instantiated more than once in the same
  // process. The most important functionality InitLLVM provides is shutting
  // down LLVM in its destructor. We can use "llvm_shutdown_obj" to do the same.
  llvh::llvm_shutdown_obj Y;
#endif

  llvh::cl::AddExtraVersionPrinter(driver::printHermesCompilerVMVersion);
  llvh::cl::ParseCommandLineOptions(argc, argv, "Hermes driver\n");

  if (cl::InputFilenames.size() == 0) {
    return repl(getReplRuntimeConfig());
  }

  // Tell compiler to emit async break check if time-limit feature is enabled
  // so that user can turn on this feature with single ExecutionTimeLimit
  // option.
  if (cl::ExecutionTimeLimit > 0) {
    cl::EmitAsyncBreakCheck = true;
  }

  // Make sure any allocated alt signal stack is not considered a leak
  // by ASAN.
  oscompat::SigAltStackLeakSuppressor sigAltLeakSuppressor;
  driver::CompileResult res = driver::compileFromCommandLineOptions();
  if (res.bytecodeProvider) {
    auto ret = executeHBCBytecodeFromCL(
        std::move(res.bytecodeProvider), res.bytecodeBufferInfo);
    return ret;
  } else {
    return res.status;
  }
}
