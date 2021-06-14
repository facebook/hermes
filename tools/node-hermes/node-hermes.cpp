/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"
#include "hermes/hermes.h"
#include "nodelib/NodeBytecode.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
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

/// Takes an StringRef to a file path and
/// creates a jsi::Buffer pointing to it without copying or taking ownership.
/// The StringRef must outlive the class.
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

/// Takes an ArrayRef to a series of bytes and
/// creates a jsi::Buffer pointing to it without copying or taking ownership.
/// The ArrayRef must outlive the class.
class ArrayRefBuffer : public jsi::Buffer {
 public:
  ArrayRefBuffer(llvh::ArrayRef<uint8_t> array) : array_(array){};
  const uint8_t *data() const override {
    return array_.data();
  }
  size_t size() const override {
    return array_.size();
  }

 private:
  llvh::ArrayRef<uint8_t> array_;
};

/// Given the directory that the original JS file runs from and the
/// relative path of the target, forms the absolute path
/// for the target.
static void canonicalizePath(
    llvh::SmallVectorImpl<char> &dirname,
    llvh::StringRef target) {
  if (!target.empty() && target[0] == '/') {
    // If the target is absolute (starts with a '/'), resolve from the module
    // root (disregard the dirname).
    dirname.clear();
    llvh::sys::path::append(dirname, target.drop_front(1));
    return;
  }
  llvh::sys::path::append(dirname, llvh::sys::path::Style::posix, target);

  // Remove all dots. This is done to get rid of ../ or anything like ././.
  llvh::sys::path::remove_dots(dirname, true, llvh::sys::path::Style::posix);
}

/// Adds a JS function wrapper around the StringRef buffer passed in.
static const std::shared_ptr<jsi::Buffer> addjsWrapper(
    llvh::StringRef strBuffer) {
  std::string wrappedBuffer =
      "(function(exports, require, module, __filename, __dirname) {";
  wrappedBuffer += strBuffer;
  wrappedBuffer += "});";
  return std::make_unique<jsi::StringBuffer>(std::move(wrappedBuffer));
}

/// Given the requested module name/file name, which is either a builtin
/// module that has already been compiled to byte code or a JS file which
/// needs to be directly read from disk,
/// returns the wrapper function which is called to `require` it.
static jsi::Function resolveRequireCall(
    const std::string &filename,
    const std::string &dirname,
    jsi::Runtime &rt) {
  std::shared_ptr<jsi::Buffer> buf;
  if (filename == "fs") {
    llvh::ArrayRef<uint8_t> module = getNodeBytecode();
    assert(facebook::hermes::HermesRuntime::isHermesBytecode(
        module.data(), module.size()));
    buf = std::make_shared<ArrayRefBuffer>(module);
  } else {
    if (filename.empty()) {
      throw jsi::JSError(
          rt, "A valid file name must be inputted to require call");
    }
    if (filename[0] != '.') {
      throw jsi::JSError(rt, "This module is not supported yet");
    }
    llvh::SmallString<32> fullFileName{dirname};
    canonicalizePath(fullFileName, filename);

    auto memBuffer = llvh::MemoryBuffer::getFileOrSTDIN(fullFileName.str());
    if (!memBuffer) {
      std::string fullErrorMessage{"Failed to open file: "};
      fullErrorMessage += fullFileName;
      throw jsi::JSError(rt, std::move(fullErrorMessage));
    }
    buf = addjsWrapper(memBuffer.get()->getBuffer());
  }
  jsi::Value result = rt.evaluateJavaScript(buf, filename);
  return result.asObject(rt).asFunction(rt);
}

/// Reads and exports the values from a given file.
static jsi::Value require(
    const std::string &filename,
    const std::string &dirname,
    jsi::Runtime &rt,
    std::unordered_map<std::string, jsi::Object> &fileTracker) {
  auto iterAlreadyExists = fileTracker.find(filename);
  if (iterAlreadyExists != fileTracker.end()) {
    return iterAlreadyExists->second.getProperty(rt, "exports");
  }

  jsi::Function result = resolveRequireCall(filename, dirname, rt);
  jsi::Object mod{rt};
  jsi::Object exports{rt};

  mod.setProperty(rt, "exports", exports);
  auto iterAndSuccess =
      fileTracker.emplace(std::make_pair(filename, std::move(mod)));
  assert(iterAndSuccess.second && "Insertion must succeed.");

  jsi::Value out = result.call(
      rt,
      exports,
      rt.global().getProperty(rt, "require"),
      iterAndSuccess.first->second,
      filename,
      dirname,
      5);

  return iterAndSuccess.first->second.getProperty(rt, "exports");
}

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

  // Maps from filename to JS module object.
  std::unordered_map<std::string, jsi::Object> fileTracker;

  llvh::SmallString<32> dirName{InputFilename};
  llvh::sys::path::remove_filename(dirName, llvh::sys::path::Style::posix);
  const std::string strDirname{dirName.data(), dirName.size()};
  try {
    // Creates require JS function and links it to the c++ version.
    jsi::Function req = jsi::Function::createFromHostFunction(
        *runtime,
        jsi::PropNameID::forAscii(*runtime, "require"),
        1,
        [&fileTracker, &strDirname](
            jsi::Runtime &rt,
            const jsi::Value &,
            const jsi::Value *args,
            size_t count) -> jsi::Value {
          if (count == 0) {
            throw jsi::JSError(rt, "Not enough arguments passed in");
          }
          return require(
              args[0].toString(rt).utf8(rt), strDirname, rt, fileTracker);
        });
    runtime->global().setProperty(*runtime, "require", req);

    auto result = runtime->evaluateJavaScript(jsiBuffer, srcPath);

  } catch (const jsi::JSIException &e) {
    llvh::errs() << "JavaScript terminated via uncaught exception: " << e.what()
                 << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
