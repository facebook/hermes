/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

#ifndef HERMESVM_LEAN
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#endif
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/ConsoleHost/RuntimeFlags.h"
#include "hermes/Public/Buffer.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/PageAccessTracker.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"

#define DEBUG_TYPE "hvm"

using namespace hermes;

static llvh::cl::opt<std::string> InputFilename(
    llvh::cl::desc("input file"),
    llvh::cl::init("-"),
    llvh::cl::Positional);

// Lean VM doesn't include the disassembler.
#ifndef HERMESVM_LEAN
static llvh::cl::opt<bool> Disassemble(
    "d",
    llvh::cl::desc("Disassemble bytecode"));

static llvh::cl::opt<bool> PrettyDisassemble(
    "pretty-disassemble",
    llvh::cl::init(true),
    llvh::cl::desc("Pretty print the disassembled bytecode"));
#endif

static llvh::cl::opt<unsigned> Repeat(
    "Xrepeat",
    llvh::cl::desc("Repeat execution N number of times"),
    llvh::cl::init(1),
    llvh::cl::Hidden);

static llvh::cl::opt<bool> GCPrintStats(
    "gc-print-stats",
    llvh::cl::desc("Output summary garbage collection statistics at exit"),
    llvh::cl::cat(cl::GCCategory),
    llvh::cl::init(false));

// This is the vm driver.
int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvh::InitLLVM initLLVM(argc, argv);
  llvh::sys::PrintStackTraceOnErrorSignal("hvm");
  llvh::PrettyStackTraceProgram X(argc, argv);
  llvh::llvm_shutdown_obj Y;
  llvh::cl::ParseCommandLineOptions(argc, argv, "Hermes VM driver\n");

  llvh::ErrorOr<std::unique_ptr<llvh::MemoryBuffer>> FileBufOrErr =
      llvh::MemoryBuffer::getFileOrSTDIN(InputFilename);

  std::string filename = FileBufOrErr.get()->getBufferIdentifier().str();

  if (!FileBufOrErr) {
    llvh::errs() << "Error! Failed to open file: " << InputFilename << "\n";
    return -1;
  }

  auto buffer = std::make_unique<MemoryBuffer>(FileBufOrErr.get().get());
  auto ret =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(std::move(buffer));

  if (!ret.first) {
    llvh::errs() << ret.second;
    return 1;
  }

  std::unique_ptr<hbc::BCProvider> bytecode = std::move(ret.first);

#ifndef HERMESVM_LEAN
  if (Disassemble) {
    hermes::hbc::BytecodeDisassembler disassembler(std::move(bytecode));
    disassembler.setOptions(
        PrettyDisassemble ? hermes::hbc::DisassemblyOptions::Pretty
                          : hermes::hbc::DisassemblyOptions::None);
    disassembler.disassemble(llvh::outs());
  }
#endif

  ExecuteOptions options;
  options.runtimeConfig =
      vm::RuntimeConfig::Builder()
          .withGCConfig(
              vm::GCConfig::Builder()
                  .withInitHeapSize(cl::InitHeapSize.bytes)
                  .withMaxHeapSize(cl::MaxHeapSize.bytes)
                  .withSanitizeConfig(
                      vm::GCSanitizeConfig::Builder()
                          .withSanitizeRate(cl::GCSanitizeRate)
                          .withRandomSeed(cl::GCSanitizeRandomSeed)
                          .build())
                  .withShouldRandomizeAllocSpace(cl::GCRandomizeAllocSpace)
                  .withShouldRecordStats(
                      GCPrintStats && !cl::StableInstructionCount)
                  .withShouldReleaseUnused(vm::kReleaseUnusedNone)
                  .withName("hvm")
                  .build())
          .withES6Promise(cl::ES6Promise)
          .withES6Proxy(cl::ES6Proxy)
          .withIntl(cl::Intl)
          .withMicrotaskQueue(cl::MicrotaskQueue)
          .withTrackIO(cl::TrackBytecodeIO)
          .withEnableHermesInternal(cl::EnableHermesInternal)
          .withEnableHermesInternalTestMethods(
              cl::EnableHermesInternalTestMethods)
          .build();

  options.stabilizeInstructionCount = cl::StableInstructionCount;
  options.stopAfterInit = cl::StopAfterInit;
#ifdef HERMESVM_PROFILER_EXTERN
  options.patchProfilerSymbols = cl::PatchProfilerSymbols;
  options.profilerSymbolsFile = cl::ProfilerSymbolsFile;
#endif

  bool success;
  if (Repeat <= 1) {
    success = executeHBCBytecode(std::move(bytecode), options, &filename);
  } else {
    // The runtime is supposed to own the bytecode exclusively, but we
    // want to keep it around in this special case, so we can reuse it
    // between iterations.
    std::shared_ptr<hbc::BCProvider> sharedBytecode = std::move(bytecode);

    success = true;
    for (unsigned i = 0; i < Repeat; ++i) {
      success &= executeHBCBytecode(
          std::shared_ptr<hbc::BCProvider>{sharedBytecode}, options, &filename);
    }
  }
  return success ? 0 : 1;
}
#undef DEBUG_TYPE
