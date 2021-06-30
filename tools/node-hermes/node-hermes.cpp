/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

#include <fcntl.h>

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

// Defines the constants needed by fs.js.
static void defineConstants(jsi::Runtime &runtime, jsi::Object &target) {
#define DEFINE_CONSTANT(runtime, name)        \
  do {                                        \
    target.setProperty(runtime, #name, name); \
  } while (0)
#ifdef S_IFIFO
  DEFINE_CONSTANT(runtime, S_IFIFO);
#endif
#ifdef S_IFLNK
  DEFINE_CONSTANT(runtime, S_IFLNK);
#endif
#ifdef S_IFSOCK
  DEFINE_CONSTANT(runtime, S_IFSOCK);
#endif
#ifdef F_OK
  DEFINE_CONSTANT(runtime, F_OK);
#endif
#ifdef R_OK
  DEFINE_CONSTANT(runtime, R_OK);
#endif
#ifdef W_OK
  DEFINE_CONSTANT(runtime, W_OK);
#endif
#ifdef X_OK
  DEFINE_CONSTANT(runtime, X_OK);
#endif
#ifdef O_SYMLINK
  DEFINE_CONSTANT(runtime, O_SYMLINK);
#endif
  DEFINE_CONSTANT(runtime, S_IFMT);
  DEFINE_CONSTANT(runtime, S_IFREG);
  DEFINE_CONSTANT(runtime, O_WRONLY);
#undef DEFINE_CONSTANT
}

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

// Adds the 'constants' object as a property of internalBinding. Currently
// this object only has the fs property defined on it.
static jsi::Value constantsBinding(jsi::Runtime &rt, RuntimeState &rs) {
  jsi::Object fsProp{rt};
  defineConstants(rt, fsProp);
  jsi::Object constants{rt};
  constants.setProperty(rt, jsi::String::createFromAscii(rt, "fs"), fsProp);
  jsi::String constantsLabel = jsi::String::createFromAscii(rt, "constants");
  rs.setInternalBindingProp(constantsLabel, std::move(constants));
  return rs.getInternalBindingProp(constantsLabel);
}

// Implements the internal binding JS functionality. Currently only includes
// constants that are relevant to the execution of fs.js.
static jsi::Value internalBinding(
    const jsi::String &propName,
    jsi::Runtime &rt,
    RuntimeState &rs) {
  if (rs.internalBindingPropExists(propName)) {
    return rs.getInternalBindingProp(propName);
  }
  std::string propNameUTF8 = propName.utf8(rt);
  if (propNameUTF8 == "constants") {
    return constantsBinding(rt, rs);
  } else if (propNameUTF8 == "fs") { // Next to be implemented.
    return jsi::Value::undefined();
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
    const std::string &dirname,
    jsi::Runtime &rt,
    jsi::Object &builtinModules) {
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
    llvh::SmallString<32> fullFileName{dirname};
    canonicalizePath(fullFileName, filenameUTF8);

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
    const std::string &dirname,
    jsi::Runtime &rt,
    RuntimeState &rs,
    jsi::Object &builtinModules) {
  std::string filenameUTF8 = filename.utf8(rt);
  llvh::Optional<jsi::Object> existingMod = rs.findRequiredModule(filenameUTF8);
  if (existingMod.hasValue()) {
    return std::move(*existingMod);
  }
  jsi::Function result =
      resolveRequireCall(filename, dirname, rt, builtinModules);
  jsi::Object mod{rt};
  jsi::Object exports{rt};
  mod.setProperty(rt, "exports", exports);

  jsi::Object &mapModule = rs.addRequiredModule(filenameUTF8, std::move(mod));

  jsi::Value out = result.call(
      rt,
      exports,
      rt.global().getProperty(rt, "require"),
      mapModule,
      filename,
      dirname,
      5);
  return mapModule.getProperty(rt, "exports");
}

// Initializes all the builtin modules as well as several of
// the functions/properties needed to execute the modules.
static void initialize(
    facebook::hermes::HermesRuntime &runtime,
    const std::string &dirname,
    RuntimeState &rs,
    jsi::Object &builtinModules) {
  // Creates require JS function and links it to the c++ version.
  jsi::Function req = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "require"),
      1,
      [&rs, &dirname, &builtinModules](
          jsi::Runtime &rt,
          const jsi::Value &,
          const jsi::Value *args,
          size_t count) -> jsi::Value {
        if (count == 0) {
          throw jsi::JSError(rt, "Not enough arguments passed in");
        }
        return require(args[0].toString(rt), dirname, rt, rs, builtinModules);
      });
  runtime.global().setProperty(runtime, "require", req);

  // Creates internalBinding JS function and links it to the c++ version.
  jsi::Function intBinding = jsi::Function::createFromHostFunction(
      runtime,
      jsi::PropNameID::forAscii(runtime, "internalBinding"),
      1,
      [&rs](
          jsi::Runtime &rt,
          const jsi::Value &,
          const jsi::Value *args,
          size_t count) -> jsi::Value {
        if (count == 0) {
          throw jsi::JSError(rt, "Not enough arguments passed in");
        }
        return internalBinding(args[0].toString(rt), rt, rs);
      });
  runtime.global().setProperty(runtime, "internalBinding", intBinding);

  // Will add more properties as they are required.
  jsi::Object process{runtime};
  runtime.global().setProperty(runtime, "process", process);

  runtime.global().setProperty(runtime, "global", runtime.global());

  jsi::Object primordials{runtime};
  runtime.global().setProperty(runtime, "primordials", primordials);
  require(
      jsi::String::createFromAscii(runtime, "internal/per_context/primordials"),
      dirname,
      runtime,
      rs,
      builtinModules);
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
  RuntimeState rs{*runtime};

  llvh::SmallString<32> dirName{InputFilename};
  llvh::sys::path::remove_filename(dirName, llvh::sys::path::Style::posix);
  const std::string strDirname{dirName.data(), dirName.size()};

  llvh::ArrayRef<uint8_t> moduleObj = getNodeBytecode();
  assert(
      facebook::hermes::HermesRuntime::isHermesBytecode(
          moduleObj.data(), moduleObj.size()) &&
      "internal module bytecode invalid");
  std::shared_ptr<jsi::Buffer> buf =
      std::make_shared<ArrayRefBuffer>(moduleObj);
  jsi::Object builtinModules =
      runtime->evaluateJavaScript(buf, "NodeBytecode.js").asObject(*runtime);

  try {
    initialize(*runtime, strDirname, rs, builtinModules);
    auto result = runtime->evaluateJavaScript(jsiBuffer, srcPath);

  } catch (const jsi::JSIException &e) {
    llvh::errs() << "JavaScript terminated via uncaught exception: " << e.what()
                 << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
