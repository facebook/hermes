/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"
#include "hermes/hermes.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"

using namespace facebook;

static llvm::cl::opt<bool> CheckSyntax(
    "check",
    llvm::cl::desc("check script syntax without executing"),
    llvm::cl::init(false));

static llvm::cl::opt<std::string> EvalScript(
    "eval",
    llvm::cl::desc("evaluate script"),
    llvm::cl::value_desc("script"));

static llvm::cl::opt<bool> PrintResult(
    "print",
    llvm::cl::desc("print result of evaluated script"),
    llvm::cl::init(false));

static llvm::cl::opt<std::string> InputFilename(
    llvm::cl::Positional,
    llvm::cl::desc("<file>"),
    llvm::cl::init("-"));

class FileBuffer : public jsi::Buffer {
 public:
  static std::shared_ptr<jsi::Buffer> bufferFromFile(llvm::StringRef path) {
    auto fileBuffer = llvm::MemoryBuffer::getFileOrSTDIN(path);
    if (!fileBuffer)
      return nullptr;
    return std::make_shared<FileBuffer>(std::move(*fileBuffer));
  }

  FileBuffer(std::unique_ptr<llvm::MemoryBuffer> buffer)
      : buffer_(std::move(buffer)){};
  size_t size() const override {
    return buffer_->getBufferSize();
  }
  const uint8_t *data() const override {
    return reinterpret_cast<const uint8_t *>(buffer_->getBufferStart());
  }

 private:
  std::unique_ptr<llvm::MemoryBuffer> buffer_;
};

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvm::InitLLVM initLLVM(argc, argv);
  // Print a stack trace if we signal out.
  llvm::sys::PrintStackTraceOnErrorSignal("Hermes JSI");
  llvm::PrettyStackTraceProgram X(argc, argv);
  // Call llvm_shutdown() on exit to print stats and free memory.
  llvm::llvm_shutdown_obj Y;

  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes JSI\n");

  // Suppress any ASAN leak complaints for the alt signal stack on exit.
  ::hermes::oscompat::SigAltStackLeakSuppressor sigAltLeakSuppressor;

  if (!EvalScript.empty() && InputFilename != "-") {
    llvm::errs() << "Cannot use both --eval <script> and <file>" << '\n';
    return EXIT_FAILURE;
  }

  auto jsiBuffer = !EvalScript.empty()
      ? std::make_shared<jsi::StringBuffer>(EvalScript)
      : FileBuffer::bufferFromFile(InputFilename);

  if (!jsiBuffer) {
    llvm::errs() << "Error! Failed to open file: " << InputFilename << '\n';
    return EXIT_FAILURE;
  }

  auto srcPath = !EvalScript.empty()
      ? "<eval>"
      : InputFilename == "-" ? "<stdin>" : std::string(InputFilename);

  auto runtime = facebook::hermes::makeHermesRuntime();

  try {
    auto js = runtime->prepareJavaScript(jsiBuffer, srcPath);

    if (!CheckSyntax) {
      auto result = runtime->evaluatePreparedJavaScript(js);

      if (PrintResult) {
        llvm::outs() << result.toString(*runtime).utf8(*runtime) << '\n';
      }
    }
  } catch (const jsi::JSIException &e) {
    llvm::errs() << "JavaScript terminated via uncaught exception: " << e.what()
                 << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
