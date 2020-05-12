/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/CompilerDriver/CompilerDriver.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"

using namespace hermes;

int main(int argc, char **argv) {
#ifndef HERMES_FBCODE_BUILD
  // Normalize the arg vector.
  llvm::InitLLVM initLLVM(argc, argv);
#else
  // When both HERMES_FBCODE_BUILD and sanitizers are enabled, InitLLVM may have
  // been already created and destroyed before main() is invoked. This presents
  // a problem because InitLLVM can't be instantiated more than once in the same
  // process. The most important functionality InitLLVM provides is shutting
  // down LLVM in its destructor. We can use "llvm_shutdown_obj" to do the same.
  llvm::llvm_shutdown_obj Y;
#endif
  llvm::cl::AddExtraVersionPrinter(driver::printHermesCompilerVersion);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes driver\n");

  if (driver::outputFormatFromCommandLineOptions() ==
      OutputFormatKind::Execute) {
    // --help says "Choose output:" so mimic the wording here
    llvm::errs()
        << "Please choose output, e.g. -emit-binary. hermesc does not support -exec.\n";
    llvm::errs() << "Example: hermesc -emit-binary -out myfile.hbc myfile.js\n";
    return EXIT_FAILURE;
  }

  driver::CompileResult res = driver::compileFromCommandLineOptions();
  if (res.bytecodeProvider) {
    llvm::errs() << "Execution not supported with hermesc\n";
    assert(
        false &&
        "Execution mode was not checked prior to compileFromCommandLineOptions");
    return EXIT_FAILURE;
  }
  return res.status;
}
