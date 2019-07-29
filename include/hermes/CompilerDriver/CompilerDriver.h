/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_COMPILERDRIVER_COMPILERDRIVER_H
#define HERMES_COMPILERDRIVER_COMPILERDRIVER_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace driver {

enum CompileStatus {
  /// Compilation succeeded.
  Success,
  /// The combination of flags was invalid.
  InvalidFlags,
  /// The input files were source code that could not be parsed.
  ParsingFailed,
  /// The IR could not be verified.
  VerificationFailed,
  /// The file containing the global environment could not be parsed.
  LoadGlobalsFailed,
  /// An input file could not be read.
  InputFileError,
  /// An output file could not be written.
  OutputFileError,
};

/// Information about a bytecode file that is loaded into a buffer.
struct BytecodeBufferInfo {
  /// True if the buffer is mmapped
  bool bufferIsMmapped{false};
  /// Pointer to the beginning of the bytecode buffer.
  char *bufferStart{nullptr};
  /// Size of the bytecode buffer.
  size_t bufferSize{0};
  /// Name of the bytecode file.
  std::string filename;

  BytecodeBufferInfo() = default;

  BytecodeBufferInfo(
      bool isMmapped,
      char *start,
      size_t size,
      std::string filename)
      : bufferIsMmapped(isMmapped),
        bufferStart(start),
        bufferSize(size),
        filename(std::move(filename)){};
};

/// A type wrapping up the possible outputs of compileFiles.
struct CompileResult {
  /// The result of compilation.
  CompileStatus status;

  /// If the target is hbc and execution was requested, the bytecode provider,
  /// otherwise nullptr.
  std::unique_ptr<hermes::hbc::BCProvider> bytecodeProvider{};

  /// Information about the hbc buffer. Only relevant if the bytecode is
  /// loaded from a file, that is, bytecodeProvider is a BCProviderFromBuffer.
  BytecodeBufferInfo bytecodeBufferInfo{};

  /// The Context associated with compilation, if lazy compilation is enabled.
  /// This field has no purpose except to extend the lifetime of Context.
  std::shared_ptr<Context> context{};

  /* implicit */ CompileResult(CompileStatus status) : status(status) {}
};

/// Drive the Hermes compiler according to the command line options.
/// \return an exit status.
CompileResult compileFromCommandLineOptions();

/// Print the Hermes version (with VM) to the given stream \p s.
void printHermesCompilerVMVersion(llvm::raw_ostream &s);

/// Print the Hermes version (without VM) to the given stream \p s.
void printHermesCompilerVersion(llvm::raw_ostream &s);

/// Print the Hermes version for the REPL to the given stream \p s.
void printHermesREPLVersion(llvm::raw_ostream &s);

} // namespace driver
} // namespace hermes

namespace cl {
/// The following flags are used by both the compiler and the VM driver.
extern llvm::cl::opt<bool> BasicBlockProfiling;
extern llvm::cl::opt<bool> EnableEval;
extern llvm::cl::opt<bool> VerifyIR;
extern llvm::cl::opt<bool> EmitAsyncBreakCheck;
} // namespace cl
#endif
