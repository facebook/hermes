/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/ConsoleHost/RuntimeFlags.h"
#include "hermes/VM/instrumentation/PageAccessTracker.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
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

static opt<bool> TrackBytecodeIO(
    "track-io",
    desc(
        "Track bytecode I/O when executing bytecode. Only works with bytecode mode"));

enum BytecodeIOStatsFormatKind { HUMAN, JSON };

static opt<BytecodeIOStatsFormatKind> BytecodeIOStatsFormat(
    "io-stats-format",
    llvm::cl::init(HUMAN),
    llvm::cl::desc("Options for the format for printing bytecode I/O stats"),
    llvm::cl::values(
        clEnumVal(HUMAN, "Output in human readable format"),
        clEnumVal(JSON, "Output in JSON format (default)")));

} // namespace cl

/// Execute Hermes bytecode \p bytecode, respecting command line arguments.
/// \return an exit status.
static int executeHBCBytecodeFromCL(std::unique_ptr<hbc::BCProvider> bytecode) {
  ExecuteOptions options;
  options.runtimeConfig =
      vm::RuntimeConfig::Builder()
          .withGCConfig(
              vm::GCConfig::Builder()
                  .withMinHeapSize(cl::MinHeapSize.bytes)
                  .withInitHeapSize(cl::InitHeapSize.bytes)
                  .withMaxHeapSize(cl::MaxHeapSize.bytes)
                  .withSanitizeRate(cl::GCSanitizeRate)
                  .withShouldRandomizeAllocSpace(cl::GCRandomizeAllocSpace)
                  .withShouldRecordStats(cl::GCPrintStats)
                  .withShouldReleaseUnused(false)
                  .withAllocInYoung(cl::GCAllocYoung)
                  .withRevertToYGAtTTI(cl::GCRevertToYGAtTTI)
                  .build())
          .withEnableJIT(cl::DumpJITCode || cl::EnableJIT)
          .withEnableEval(cl::EnableEval)
          .withES6Symbol(cl::ES6Symbol)
          .withEnableSampleProfiling(cl::SampleProfiling)
          .withRandomizeMemoryLayout(cl::RandomizeMemoryLayout)
          .build();

  options.basicBlockProfiling = cl::BasicBlockProfiling;

  options.stopAfterInit = false;
#ifdef HERMESVM_PROFILER_EXTERN
  options.patchProfilerSymbols = cl::PatchProfilerSymbols;
  options.profilerSymbolsFile = cl::ProfilerSymbolsFile;
#endif
  options.dumpJITCode = cl::DumpJITCode;
  options.jitCrashOnError = cl::JITCrashOnError;

  bool success;
  if (cl::Repeat <= 1) {
    success = executeHBCBytecode(std::move(bytecode), options);
  } else {
    // The runtime is supposed to own the bytecode exclusively, but we
    // want to keep it around in this special case, so we can reuse it
    // between iterations.
    std::shared_ptr<hbc::BCProvider> sharedBytecode = std::move(bytecode);

    success = true;
    for (unsigned i = 0; i < cl::Repeat; ++i) {
      success &= executeHBCBytecode(
          std::shared_ptr<hbc::BCProvider>{sharedBytecode}, options);
    }
  }
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv_) {
  // Print a stack trace if we signal out.
  llvm::sys::PrintStackTraceOnErrorSignal("Hermes driver");
  llvm::PrettyStackTraceProgram X(argc, argv_);
  // Fix argc and argv (necessary on some platforms)
  llvm::SmallVector<const char *, 256> args;
  llvm::SpecificBumpPtrAllocator<char> ArgAllocator;
  if (llvm::sys::Process::GetArgumentVector(
          args, llvm::makeArrayRef(argv_, argc), ArgAllocator)) {
    llvm::errs() << "Failed to get argc and argv.\n";
    return EXIT_FAILURE;
  }
  argc = args.size();
  const char **argv = args.data();
  // Call llvm_shutdown() on exit to print stats and free memory.
  llvm::llvm_shutdown_obj Y;

  llvm::cl::AddExtraVersionPrinter(driver::printHermesCompilerVMVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes driver\n");
  driver::CompileResult res = driver::compileFromCommandLineOptions();
  if (res.bytecodeProvider) {
    if (cl::TrackBytecodeIO) {
      if (!res.bytecodeBufferInfo.bufferIsMmapped) {
        // We use llvm::MemoryBuffer which does not use mmap if the file size
        // is less than a page.
        llvm::errs()
            << "Cannot use PageAccessTracker because bytecode is not mmapped.\n";
        return EXIT_FAILURE;
      }
      if (!PageAccessTracker::initialize(
              res.bytecodeBufferInfo.bufferStart,
              res.bytecodeBufferInfo.bufferSize)) {
        return EXIT_FAILURE;
      }
    }
    auto ret = executeHBCBytecodeFromCL(std::move(res.bytecodeProvider));
    if (ret == EXIT_SUCCESS && cl::TrackBytecodeIO) {
      if (!PageAccessTracker::printStats(
              llvm::outs(),
              cl::BytecodeIOStatsFormat ==
                  cl::BytecodeIOStatsFormatKind::JSON)) {
        return EXIT_FAILURE;
      }
      if (!PageAccessTracker::shutdown()) {
        return EXIT_FAILURE;
      }
    }
    return ret;
  } else {
    return res.status;
  }
}
