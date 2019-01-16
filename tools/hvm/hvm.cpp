#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

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
#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"

#define DEBUG_TYPE "hvm"

using namespace hermes;

static llvm::cl::opt<std::string> InputFilename(
    llvm::cl::desc("input file"),
    llvm::cl::init("-"),
    llvm::cl::Positional);

// Lean VM doesn't include the disassembler.
#ifndef HERMESVM_LEAN
static llvm::cl::opt<bool> Disassemble(
    "d",
    llvm::cl::desc("Disassemble bytecode"));

static llvm::cl::opt<bool> PrettyDisassemble(
    "pretty-disassemble",
    llvm::cl::init(true),
    llvm::cl::desc("Pretty print the disassembled bytecode"));
#endif

static llvm::cl::opt<bool> StopAfterInit(
    "stop-after-module-init",
    llvm::cl::desc("Exit once module loading is finished. Useful "
                   "to measure module initialization time"));

static llvm::cl::opt<unsigned> Repeat(
    "Xrepeat",
    llvm::cl::desc("Repeat execution N number of times"),
    llvm::cl::init(1),
    llvm::cl::Hidden);

// This is the vm driver.
int main(int argc, char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal("hvm");
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm::llvm_shutdown_obj Y;
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes VM driver\n");

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileBufOrErr =
      llvm::MemoryBuffer::getFileOrSTDIN(InputFilename);

  if (!FileBufOrErr) {
    llvm::errs() << "Error! Failed to open file: " << InputFilename << "\n";
    return -1;
  }

  auto buffer = llvm::make_unique<MemoryBuffer>(FileBufOrErr.get().get());
  auto ret =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(std::move(buffer));

  if (!ret.first) {
    llvm::errs() << ret.second;
    return 1;
  }

  std::unique_ptr<hbc::BCProvider> bytecode = std::move(ret.first);

#ifndef HERMESVM_LEAN
  if (Disassemble) {
    hermes::hbc::BytecodeDisassembler disassembler(std::move(bytecode));
    disassembler.setOptions(
        PrettyDisassemble ? hermes::hbc::DisassemblyOptions::Pretty
                          : hermes::hbc::DisassemblyOptions::None);
    disassembler.disassemble(llvm::outs());
  }
#endif

  ExecuteOptions options;
  options.runtimeConfig =
      vm::RuntimeConfig::Builder()
          .withGCConfig(
              vm::GCConfig::Builder()
                  .withInitHeapSize(cl::InitHeapSize.bytes)
                  .withMaxHeapSize(cl::MaxHeapSize.bytes)
                  .withSanitizeRate(cl::GCSanitizeRate)
                  .withShouldRandomizeAllocSpace(cl::GCRandomizeAllocSpace)
                  .withShouldRecordStats(cl::GCPrintStats)
                  .withShouldReleaseUnused(false)
                  .withName("hvm")
                  .build())
          .withES6Symbol(cl::ES6Symbol)
          .build();

  options.stopAfterInit = StopAfterInit;
#ifdef HERMESVM_PROFILER_EXTERN
  options.patchProfilerSymbols = cl::PatchProfilerSymbols;
  options.profilerSymbolsFile = cl::ProfilerSymbolsFile;
#endif

  bool success;
  if (Repeat <= 1) {
    success = executeHBCBytecode(std::move(bytecode), options);
  } else {
    // The runtime is supposed to own the bytecode exclusively, but we
    // want to keep it around in this special case, so we can reuse it
    // between iterations.
    std::shared_ptr<hbc::BCProvider> sharedBytecode = std::move(bytecode);

    success = true;
    for (unsigned i = 0; i < Repeat; ++i) {
      success &= executeHBCBytecode(
          std::shared_ptr<hbc::BCProvider>{sharedBytecode}, options);
    }
  }

  return success ? 0 : 1;
}
