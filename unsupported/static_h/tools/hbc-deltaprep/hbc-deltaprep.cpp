/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeFormConverter.h"
#include "hermes/BCGen/HBC/HBC.h"

using namespace hermes::hbc;

static llvh::cl::opt<BytecodeForm> Form(
    "form",
    llvh::cl::values(
        clEnumValN(BytecodeForm::Delta, "delta", "form suitable for diffing"),
        clEnumValN(
            BytecodeForm::Execution,
            "execution",
            "form suitable for execution")),
    llvh::cl::Required,
    llvh::cl::desc("Form to convert to (execution or delta)"));

static llvh::cl::opt<std::string> InputFilename(
    llvh::cl::desc("input file"),
    llvh::cl::Required,
    llvh::cl::Positional);

static llvh::cl::opt<std::string> OutputFilename(
    "out",
    llvh::cl::Required,
    llvh::cl::desc("Output file name"));

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
  llvh::cl::ParseCommandLineOptions(
      argc, argv, "Hermes bytecode deltaprep tool\n");

  // Read the file and then copy the bytes into a mutable buffer.
  // TODO: switch to WritableMemoryBuffer after updating LLVM. Or use mmap()
  // with a private mapping.
  llvh::ErrorOr<std::unique_ptr<llvh::MemoryBuffer>> fileBufOrErr =
      llvh::MemoryBuffer::getFile(
          InputFilename, -1, false /* RequiresNullTerminator */);
  if (!fileBufOrErr) {
    llvh::errs() << "Error: fail to open file: " << InputFilename << ": "
                 << fileBufOrErr.getError().message() << "\n";
    return -1;
  }
  auto bytes = (*fileBufOrErr)->getBuffer();
  std::vector<uint8_t> mutableBytes(bytes.begin(), bytes.end());

  std::string error;
  if (!convertBytecodeToForm(mutableBytes, Form, &error)) {
    llvh::errs() << "Error: failed to prepare file " << InputFilename << ": "
                 << error << '\n';
    return -1;
  }

  std::error_code EC;
  llvh::raw_fd_ostream fileOS(OutputFilename.data(), EC, llvh::sys::fs::F_None);
  if (EC) {
    llvh::errs() << "Error: fail to open file " << OutputFilename << ": "
                 << EC.message() << '\n';
    return -1;
  }
  fileOS.write(
      reinterpret_cast<const char *>(mutableBytes.data()), mutableBytes.size());

  return 0;
}
