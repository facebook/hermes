/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/ConsoleHost/RuntimeFlags.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PageAccessTracker.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/SHA1.h"
#include "llvm/Support/Signals.h"

using namespace hermes;

namespace cl {
using llvm::cl::opt;

static opt<bool> EnableJIT(
    "jit",
    llvm::cl::desc("enable JIT compilation"),
    llvm::cl::init(false));

static opt<bool> DumpJITCode(
    "dump-jitcode",
    llvm::cl::desc("dump JIT'ed code"),
    llvm::cl::init(false));

static opt<bool> JITCrashOnError(
    "jit-crash-on-error",
    llvm::cl::desc("crash on any JIT compilation error"),
    llvm::cl::init(false));

static opt<unsigned> Repeat(
    "Xrepeat",
    llvm::cl::desc("Repeat execution N number of times"),
    llvm::cl::init(1),
    llvm::cl::Hidden);

static opt<bool> RandomizeMemoryLayout(
    "Xrandomize-memory-layout",
    llvm::cl::desc("Randomize stack placement etc."),
    llvm::cl::init(false),
    llvm::cl::Hidden);

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
    llvm::cl::desc("Number of milliseconds after which to abort JS exeuction"),
    llvm::cl::init(0));
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
          .withGCConfig(
              vm::GCConfig::Builder()
                  .withMinHeapSize(cl::MinHeapSize.bytes)
                  .withInitHeapSize(cl::InitHeapSize.bytes)
                  .withMaxHeapSize(cl::MaxHeapSize.bytes)
                  .withOccupancyTarget(cl::OccupancyTarget)
                  .withSanitizeConfig(
                      vm::GCSanitizeConfig::Builder()
                          .withSanitizeRate(cl::GCSanitizeRate)
                          .withRandomSeed(cl::GCSanitizeRandomSeed)
                          .build())
                  .withShouldRandomizeAllocSpace(cl::GCRandomizeAllocSpace)
                  .withShouldRecordStats(recStats)
                  .withShouldReleaseUnused(vm::kReleaseUnusedNone)
                  .withAllocInYoung(cl::GCAllocYoung)
                  .withRevertToYGAtTTI(cl::GCRevertToYGAtTTI)
                  .build())
          .withEnableJIT(cl::DumpJITCode || cl::EnableJIT)
          .withEnableEval(cl::EnableEval)
          .withVerifyEvalIR(cl::VerifyIR)
          .withVMExperimentFlags(cl::VMExperimentFlags)
          .withES6Symbol(cl::ES6Symbol)
          .withEnableSampleProfiling(cl::SampleProfiling)
          .withRandomizeMemoryLayout(cl::RandomizeMemoryLayout)
          .withTrackIO(cl::TrackBytecodeIO)
          .build();

  options.basicBlockProfiling = cl::BasicBlockProfiling;

  options.stopAfterInit = false;
#ifdef HERMESVM_PROFILER_EXTERN
  options.patchProfilerSymbols = cl::PatchProfilerSymbols;
  options.profilerSymbolsFile = cl::ProfilerSymbolsFile;
#endif
  options.timeLimit = cl::ExecutionTimeLimit;
  options.dumpJITCode = cl::DumpJITCode;
  options.jitCrashOnError = cl::JITCrashOnError;
  options.stopAfterInit = cl::StopAfterInit;
  options.forceGCBeforeStats = cl::GCBeforeStats;
  options.stabilizeInstructionCount = cl::StableInstructionCount;
#ifdef HERMESVM_SERIALIZE
  options.SerializeAfterInitFile = cl::SerializeAfterInitFile;
  options.DeserializeFile = cl::DeserializeFile;
  options.SerializeVMPath = cl::SerializeVMPath;
#endif

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

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvm::InitLLVM initLLVM(argc, argv);
  // Print a stack trace if we signal out.
  llvm::sys::PrintStackTraceOnErrorSignal("Hermes driver");
  llvm::PrettyStackTraceProgram X(argc, argv);
  // Call llvm_shutdown() on exit to print stats and free memory.
  llvm::llvm_shutdown_obj Y;

  llvm::cl::AddExtraVersionPrinter(driver::printHermesCompilerVMVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes driver\n");

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
