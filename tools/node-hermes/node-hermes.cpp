/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings/InternalBindings.h"
#include "RuntimeState.h"
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
#include "uv.h"

#include <math.h> /* floor */

using namespace facebook;

// -help options.
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

/// Adds a JS function wrapper around the StringRef buffer passed in.
static const std::shared_ptr<jsi::Buffer> addjsWrapper(
    llvh::StringRef strBuffer) {
  std::string wrappedBuffer =
      "(function(exports, require, module, __filename, __dirname) {";
  wrappedBuffer += strBuffer;
  wrappedBuffer += "});";
  return std::make_unique<jsi::StringBuffer>(std::move(wrappedBuffer));
}

/// Implements the internal binding JS functionality. Currently only includes
/// constants that are relevant to the execution of fs.js.
static jsi::Value internalBinding(
    const std::string &propNameUTF8,
    RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  if (rs.internalBindingPropExists(propNameUTF8)) {
    return rs.getInternalBindingProp(propNameUTF8);
  }
  if (propNameUTF8 == "constants") {
    return constantsBinding(rs);
  } else if (propNameUTF8 == "fs") {
    return fsBinding(rs);
  } else if (propNameUTF8 == "buffer") {
    return bufferBinding(rs);
  } else if (propNameUTF8 == "util") {
    return utilBinding(rs);
  } else if (propNameUTF8 == "tty_wrap") {
    return ttyBinding(rs);
  } else if (propNameUTF8 == "pipe_wrap") {
    return pipeBinding(rs);
  }
  // Will not go into this case but keeping it in case some other internal
  // binding functionality is going to be added in the future.
  throw jsi::JSError(
      rt,
      "This functionality of internalBinding has not been implemented yet: " +
          propNameUTF8);
}

/// Given the requested module name/file name, which is either a builtin
/// module that has already been compiled to byte code or a JS file which
/// needs to be directly read from disk,
/// returns the wrapper function which is called to `require` it.
static jsi::Function resolveRequireCall(
    const jsi::String &filename,
    RuntimeState &rs,
    jsi::Object &builtinModules) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Value result;
  if (builtinModules.hasProperty(rt, filename)) {
    result = builtinModules.getProperty(rt, filename);
  } else {
    std::string filenameUTF8 = filename.utf8(rt);
    if (filenameUTF8.empty()) {
      throw jsi::JSError(
          rt, "A valid file name must be inputted to require call");
    }
    if (filenameUTF8[0] != '.') {
      throw jsi::JSError(
          rt, "The following module is not supported yet: " + filenameUTF8);
    }
    llvh::SmallString<32> fullFileName = rs.resolvePath(filenameUTF8, "./");

    auto memBuffer = llvh::MemoryBuffer::getFileOrSTDIN(fullFileName.str());
    if (!memBuffer) {
      std::string fullErrorMessage{"Failed to open file: "};
      fullErrorMessage += fullFileName;
      throw jsi::JSError(rt, std::move(fullErrorMessage));
    }
    std::shared_ptr<jsi::Buffer> buf =
        addjsWrapper(memBuffer.get()->getBuffer());
    result = rt.evaluateJavaScript(buf, filenameUTF8);
  }
  return result.asObject(rt).asFunction(rt);
}

/// Reads and exports the values from a given file.
static jsi::Value require(
    const jsi::String &filename,
    RuntimeState &rs,
    jsi::Object &builtinModules,
    jsi::Function &intBinding) {
  jsi::Runtime &rt = rs.getRuntime();

  std::string filenameUTF8 = filename.utf8(rt);
  llvh::Optional<jsi::Object> existingMod = rs.findRequiredModule(filenameUTF8);
  if (existingMod.hasValue()) {
    return std::move(*existingMod);
  }
  jsi::Function result = resolveRequireCall(filename, rs, builtinModules);
  jsi::Object mod{rt};
  jsi::Object exports{rt};
  mod.setProperty(rt, "exports", exports);

  jsi::Object &mapModule = rs.addRequiredModule(filenameUTF8, std::move(mod));

  if (builtinModules.hasProperty(rt, filename)) {
    result.call(
        rt,
        exports,
        rt.global().getProperty(rt, "require"),
        mapModule,
        intBinding,
        filename,
        std::string{rs.getDirname()},
        6);
  } else {
    result.call(
        rt,
        exports,
        rt.global().getProperty(rt, "require"),
        mapModule,
        filename,
        std::string{rs.getDirname()},
        5);
  }

  return mapModule.getProperty(rt, "exports");
}

/// Sets up the hrtime function that will be defined on the process
/// object. If there was an argument passed in, then it calculates
/// the difference between the old time and the current time. Otherwise
/// it just converts the ns representation that uv_hrtime outputs to
/// an array with the seconds as the first element and the leftover
/// nanoseconds as the second element.
static jsi::Value
hrtime(jsi::Runtime &rt, const jsi::Value *args, size_t count) {
  double outputInNs = (double)uv_hrtime();
  if (count == 1) {
    jsi::Array argArray = args[0].asObject(rt).asArray(rt);
    double oldTimeSeconds = argArray.getValueAtIndex(rt, 0).asNumber();
    double oldTimeNs = argArray.getValueAtIndex(rt, 1).asNumber();
    double diffInNs = outputInNs - (oldTimeSeconds * 1e9 + oldTimeNs);

    return jsi::Array::createWithElements(
        rt, floor(diffInNs / 1e9), (diffInNs - (floor(diffInNs / 1e9) * 1e9)));
  } else
    return jsi::Array::createWithElements(
        rt,
        floor(outputInNs / 1e9),
        (outputInNs - (floor(outputInNs / 1e9) * 1e9)));
}

// Initializes all the builtin modules as well as several of
// the functions/properties needed to execute the modules.
static void initialize(
    RuntimeState &rs,
    jsi::Object &builtinModules,
    jsi::Function &intBinding) {
  jsi::Runtime &runtime = rs.getRuntime();

  // Creates require JS function and links it to the c++ version.
  jsi::Function req = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "require"),
      1,
      [&rs, &builtinModules, &intBinding](
          jsi::Runtime &rt,
          const jsi::Value &,
          const jsi::Value *args,
          size_t count) -> jsi::Value {
        if (count == 0) {
          throw jsi::JSError(rt, "Not enough arguments passed in");
        }
        return require(args[0].toString(rt), rs, builtinModules, intBinding);
      });
  runtime.global().setProperty(runtime, "require", req);

  // Will add more properties as they are required.
  runtime.global().setProperty(runtime, "global", runtime.global());
  require(
      jsi::String::createFromAscii(runtime, "node-hermes-helpers"),
      rs,
      builtinModules,
      intBinding);

  jsi::Object primordials{runtime};
  runtime.global().setProperty(runtime, "primordials", primordials);
  require(
      jsi::String::createFromAscii(runtime, "internal/per_context/primordials"),
      rs,
      builtinModules,
      intBinding);

  jsi::Object process{runtime};
  runtime.global().setProperty(runtime, "process", process);
  require(
      jsi::String::createFromAscii(
          runtime, "internal/bootstrap/switches/is_main_thread"),
      rs,
      builtinModules,
      intBinding);

  // This is a partial implementation of emitWarning, implemented
  // just for the sake of printing error messages from the console
  // constructor to stderr.
  jsi::Function warning = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "emitWarning"),
      1,
      [](jsi::Runtime &rt,
         const jsi::Value &,
         const jsi::Value *args,
         size_t count) -> jsi::Value {
        if (count == 0) {
          throw jsi::JSError(
              rt, "Not enough arguments passed in to process.emitWarning");
        }
        llvh::errs() << args[0].toString(rt).utf8(rt) << '\n';
        return jsi::Value::undefined();
      });
  process.setProperty(runtime, "emitWarning", warning);

  jsi::Function hrt = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "hrtime"),
      1,
      [](jsi::Runtime &rt,
         const jsi::Value &,
         const jsi::Value *args,
         size_t count) -> jsi::Value { return hrtime(rt, args, count); });
  process.setProperty(runtime, "hrtime", hrt);

  jsi::Object console = require(
                            jsi::String::createFromAscii(runtime, "console"),
                            rs,
                            builtinModules,
                            intBinding)
                            .asObject(runtime);
  runtime.global().setProperty(runtime, "console", console);
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

  llvh::SmallString<32> dirName{InputFilename};
  llvh::sys::path::remove_filename(dirName, llvh::sys::path::Style::posix);

  RuntimeState rs{std::move(dirName), uv_default_loop()};
  jsi::Runtime &rt = rs.getRuntime();

  llvh::ArrayRef<uint8_t> moduleObj = getNodeBytecode();
  assert(
      facebook::hermes::HermesRuntime::isHermesBytecode(
          moduleObj.data(), moduleObj.size()) &&
      "internal module bytecode invalid");
  std::shared_ptr<jsi::Buffer> buf =
      std::make_shared<ArrayRefBuffer>(moduleObj);
  jsi::Object builtinModules =
      rt.evaluateJavaScript(buf, "NodeBytecode.js").asObject(rt);

  // Creates internalBinding JS function. Will be passed as a param to
  // builtinModules.
  jsi::Function intBinding = jsi::Function::createFromHostFunction(
      rt,
      jsi::PropNameID::forAscii(rt, "internalBinding"),
      1,
      [&rs](
          jsi::Runtime &rt,
          const jsi::Value &,
          const jsi::Value *args,
          size_t count) -> jsi::Value {
        if (count == 0) {
          throw jsi::JSError(rt, "Not enough arguments passed in");
        }
        return internalBinding(args[0].toString(rt).utf8(rt), rs);
      });

  try {
    initialize(rs, builtinModules, intBinding);
    auto result = rt.evaluateJavaScript(jsiBuffer, srcPath);
    uv_run(rs.getLoop(), UV_RUN_DEFAULT);

  } catch (const jsi::JSIException &e) {
    llvh::errs() << "JavaScript terminated via uncaught exception: " << e.what()
                 << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
