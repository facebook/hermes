/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PageAccessTracker.h"
#include "hermes/VM/RuntimeFlags.h"

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

namespace {
struct Flags : public cli::VMOnlyRuntimeFlags {
  llvh::cl::opt<unsigned> Repeat{
      "Xrepeat",
      llvh::cl::desc("Repeat execution N number of times"),
      llvh::cl::init(1),
      llvh::cl::Hidden};

  llvh::cl::opt<bool> GCBeforeStats{
      "gc-before-stats",
      llvh::cl::desc(
          "Perform a full GC just before printing statistics at exit"),
      llvh::cl::cat(GCCategory),
      llvh::cl::init(false)};

  llvh::cl::opt<bool> GCPrintStats{
      "gc-print-stats",
      llvh::cl::desc("Output summary garbage collection statistics at exit"),
      llvh::cl::cat(GCCategory),
      llvh::cl::init(false)};

  llvh::cl::opt<unsigned> ExecutionTimeLimit{
      "time-limit",
      llvh::cl::desc(
          "Number of milliseconds after which to abort JS execution"),
      llvh::cl::init(0)};
};

Flags flags;

} // namespace

/// Execute Hermes bytecode \p bytecode, respecting command line arguments.
/// \return an exit status.
static int executeHBCBytecodeFromCL(
    std::unique_ptr<hbc::BCProvider> bytecode,
    const driver::BytecodeBufferInfo &info) {
#if !HERMESVM_JIT
  if (flags.DumpJITCode || flags.EnableJIT || flags.ForceJIT) {
    llvh::errs() << "JIT is not enabled in this build\n";
    return EXIT_FAILURE;
  }
#endif

  auto recStats = (flags.GCPrintStats || flags.GCBeforeStats);
  ExecuteOptions options;
  options.runtimeConfig =
      vm::RuntimeConfig::Builder()
          .withGCConfig(vm::GCConfig::Builder()
                            .withMinHeapSize(flags.MinHeapSize.bytes)
                            .withInitHeapSize(flags.InitHeapSize.bytes)
                            .withMaxHeapSize(flags.MaxHeapSize.bytes)
                            .withOccupancyTarget(flags.OccupancyTarget)
                            .withSanitizeConfig(
                                vm::GCSanitizeConfig::Builder()
                                    .withSanitizeRate(flags.GCSanitizeRate)
                                    .withRandomSeed(flags.GCSanitizeRandomSeed)
                                    .build())
                            .withShouldRecordStats(recStats)
                            .withShouldReleaseUnused(vm::kReleaseUnusedNone)
                            .withAllocInYoung(flags.GCAllocYoung)
                            .withRevertToYGAtTTI(flags.GCRevertToYGAtTTI)
                            .build())
          .withMaxNumRegisters(flags.MaxNumRegisters)
          .withEnableJIT(flags.DumpJITCode || flags.EnableJIT || flags.ForceJIT)
          .withEnableEval(cl::EnableEval)
          .withVerifyEvalIR(cl::VerifyIR)
          .withOptimizedEval(cl::OptimizedEval)
          .withAsyncBreakCheckInEval(cl::EmitAsyncBreakCheck)
          .withVMExperimentFlags(flags.VMExperimentFlags)
          .withES6Promise(flags.ES6Promise)
          .withES6Proxy(flags.ES6Proxy)
          .withES6Class(flags.EvalES6Class)
          .withIntl(flags.Intl)
          .withMicrotaskQueue(flags.MicrotaskQueue)
          .withEnableSampleProfiling(
              flags.SampleProfiling !=
              ExecuteOptions::SampleProfilingMode::None)
          .withRandomizeMemoryLayout(flags.RandomizeMemoryLayout)
          .withTrackIO(flags.TrackBytecodeIO)
          .withEnableHermesInternal(flags.EnableHermesInternal)
          .withEnableHermesInternalTestMethods(
              flags.EnableHermesInternalTestMethods)
          .build();

  options.basicBlockProfiling = cl::BasicBlockProfiling;
  options.profilingOutFile = cl::ProfilingOutFile;

  options.stopAfterInit = false;
  options.timeLimit = flags.ExecutionTimeLimit;
  options.forceJIT = flags.ForceJIT;
  options.dumpJITCode = flags.DumpJITCode;
  options.jitCrashOnError = flags.JITCrashOnError;
  options.jitEmitAsserts = flags.JITEmitAsserts;
  options.stopAfterInit = flags.StopAfterInit;
  options.forceGCBeforeStats = flags.GCBeforeStats;
  options.sampleProfiling = flags.SampleProfiling;
  options.heapTimeline = flags.HeapTimeline;

  bool success;
  if (flags.Repeat <= 1) {
    success = executeHBCBytecode(std::move(bytecode), options, &info.filename);
  } else {
    // The runtime is supposed to own the bytecode exclusively, but we
    // want to keep it around in this special case, so we can reuse it
    // between iterations.
    std::shared_ptr<hbc::BCProvider> sharedBytecode = std::move(bytecode);

    success = true;
    for (unsigned i = 0; i < flags.Repeat; ++i) {
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
      .withGCConfig(vm::GCConfig::Builder()
                        .withInitHeapSize(flags.InitHeapSize.bytes)
                        .withMaxHeapSize(flags.MaxHeapSize.bytes)
                        .withSanitizeConfig(
                            vm::GCSanitizeConfig::Builder()
                                .withSanitizeRate(flags.GCSanitizeRate)
                                .withRandomSeed(flags.GCSanitizeRandomSeed)
                                .build())
                        .withShouldRecordStats(flags.GCPrintStats)
                        .build())
      .withVMExperimentFlags(flags.VMExperimentFlags)
      .withES6Promise(flags.ES6Promise)
      .withES6Proxy(flags.ES6Proxy)
      .withES6Class(flags.EvalES6Class)
      .withIntl(flags.Intl)
      .withMicrotaskQueue(flags.MicrotaskQueue)
      .withEnableHermesInternal(flags.EnableHermesInternal)
      .withEnableHermesInternalTestMethods(
          flags.EnableHermesInternalTestMethods)
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

  // If -Xeval-es6-class is not specified, but -Xes6-class is, copy the latter
  // into the former.
  if (flags.EvalES6Class.getNumOccurrences() == 0 &&
      cl::ES6Class.getNumOccurrences()) {
    flags.EvalES6Class.setValue(cl::ES6Class.getValue());
  }

  if (cl::InputFilenames.size() == 0) {
    return repl(getReplRuntimeConfig());
  }

  // Tell compiler to emit async break check if time-limit feature is enabled
  // so that user can turn on this feature with single ExecutionTimeLimit
  // option.
  if (flags.ExecutionTimeLimit > 0) {
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
