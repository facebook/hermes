/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"
#include "hermes/hermes.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/Signals.h"

using namespace facebook;

// -help options
static llvh::cl::opt<std::string> EvalScript(
    "eval",
    llvh::cl::desc("evaluate script"),
    llvh::cl::value_desc("script"));

static llvh::cl::opt<std::string> InputFilename(
    llvh::cl::Positional,
    llvh::cl::desc("<file>"),
    llvh::cl::init("-"));

class FileBuffer : public jsi::Buffer {
 public:
  static std::shared_ptr<jsi::Buffer> bufferFromFile(llvh::StringRef path) {
    auto fileBuffer = llvh::MemoryBuffer::getFileOrSTDIN(path);
    if (!fileBuffer)
      return nullptr;
    return std::make_shared<FileBuffer>(std::move(*fileBuffer));
  }

  FileBuffer(std::unique_ptr<llvh::MemoryBuffer> buffer)
      : buffer_(std::move(buffer)){};
  size_t size() const override {
    return buffer_->getBufferSize();
  }
  const uint8_t *data() const override {
    return reinterpret_cast<const uint8_t *>(buffer_->getBufferStart());
  }

 private:
  std::unique_ptr<llvh::MemoryBuffer> buffer_;
};

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvh::InitLLVM initLLVM(argc, argv);
  // Print a stack trace if we signal out.
  llvh::sys::PrintStackTraceOnErrorSignal("Node Hermes");
  llvh::PrettyStackTraceProgram X(argc, argv);
  // Call llvm_shutdown() on exit to print stats and free memory.
  llvh::llvm_shutdown_obj Y;

  llvh::cl::ParseCommandLineOptions(argc, argv, "Node Hermes \n");

  // Suppress any ASAN leak complaints for the alt signal stack on exit.
  ::hermes::oscompat::SigAltStackLeakSuppressor sigAltLeakSuppressor;

  if (!EvalScript.empty() && InputFilename != "-") {
    llvh::errs() << "Cannot use both --eval <script> and <file>" << '\n';
    return EXIT_FAILURE;
  }

  auto jsiBuffer = !EvalScript.empty()
      ? std::make_shared<jsi::StringBuffer>(EvalScript)
      : FileBuffer::bufferFromFile(InputFilename);

  if (!jsiBuffer) {
    llvh::errs() << "Error! Failed to open file: " << InputFilename << '\n';
    return EXIT_FAILURE;
  }

  std::string srcPath = !EvalScript.empty() ? "<eval>"
      : InputFilename == "-"                ? "<stdin>"
                                            : std::string(InputFilename);

  auto runtime = facebook::hermes::makeHermesRuntime();

  try {
    // add function headers for require, value of last expression becomes the
    // return value
    auto result = runtime->evaluateJavaScript(jsiBuffer, srcPath);

  } catch (const jsi::JSIException &e) {
    llvh::errs() << "JavaScript terminated via uncaught exception: " << e.what()
                 << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
