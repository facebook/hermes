/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/CompilerDriver/CompilerDriver.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"

using namespace hermes;

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
  llvm::cl::AddExtraVersionPrinter(driver::printHermesCompilerVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes driver\n");
  driver::CompileResult res = driver::compileFromCommandLineOptions();
  if (res.bytecodeProvider) {
    llvm::errs() << "Execution not supported with hermesc\n";
    return EXIT_FAILURE;
  }
  return res.status;
}
