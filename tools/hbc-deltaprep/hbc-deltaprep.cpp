/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeFormConverter.h"
#include "hermes/BCGen/HBC/HBC.h"

using namespace hermes::hbc;

static llvm::cl::opt<BytecodeForm> Form(
    "form",
    llvm::cl::values(
        clEnumValN(BytecodeForm::Delta, "delta", "form suitable for diffing"),
        clEnumValN(
            BytecodeForm::Execution,
            "execution",
            "form suitable for execution")),
    llvm::cl::Required,
    llvm::cl::desc("Form to convert to (execution or delta)"));

static llvm::cl::opt<std::string> InputFilename(
    llvm::cl::desc("input file"),
    llvm::cl::Required,
    llvm::cl::Positional);

static llvm::cl::opt<std::string> OutputFilename(
    "out",
    llvm::cl::Required,
    llvm::cl::desc("Output file name"));

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvm::InitLLVM initLLVM(argc, argv);
  llvm::sys::PrintStackTraceOnErrorSignal("hbc-deltaprep");
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm::llvm_shutdown_obj Y;
  llvm::cl::ParseCommandLineOptions(
      argc, argv, "Hermes bytecode deltaprep tool\n");

  // Read the file and then copy the bytes into a mutable buffer.
  // TODO: switch to WritableMemoryBuffer after updating LLVM. Or use mmap()
  // with a private mapping.
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileBufOrErr =
      llvm::MemoryBuffer::getFile(
          InputFilename, -1, false /* RequiresNullTerminator */);
  if (!fileBufOrErr) {
    llvm::errs() << "Error: fail to open file: " << InputFilename << ": "
                 << fileBufOrErr.getError().message() << "\n";
    return -1;
  }
  auto bytes = (*fileBufOrErr)->getBuffer();
  std::vector<uint8_t> mutableBytes(bytes.begin(), bytes.end());

  std::string error;
  if (!convertBytecodeToForm(mutableBytes, Form, &error)) {
    llvm::errs() << "Error: failed to prepare file " << InputFilename << ": "
                 << error << '\n';
    return -1;
  }

  std::error_code EC;
  llvm::raw_fd_ostream fileOS(OutputFilename.data(), EC, llvm::sys::fs::F_None);
  if (EC) {
    llvm::errs() << "Error: fail to open file " << OutputFilename << ": "
                 << EC.message() << '\n';
    return -1;
  }
  fileOS.write(
      reinterpret_cast<const char *>(mutableBytes.data()), mutableBytes.size());

  return 0;
}
