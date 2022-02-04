/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef NODE_HERMES_RUNTIMESTATE_H
#define NODE_HERMES_RUNTIMESTATE_H

#include "hermes/hermes.h"

#include "jsi/jsi.h"
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/Signals.h"

#include "uv.h"

namespace facebook {

// Manages the module objects relevant to the 'require' and
// 'internalBinding' function calls.
class RuntimeState {
 public:
  // Parametrized constructor, copy constructor, assignment constructor
  // respectively.
  RuntimeState(llvh::SmallString<32> dirname, uv_loop_t *loop)
      : rt_(hermes::makeHermesRuntime()),
        dirname_(std::move(dirname)),
        loop_(loop),
        ttyStreamPropId_(jsi::PropNameID::forAscii(*rt_, "%ttyStreamObject%")),
        pipeStreamPropId_(
            jsi::PropNameID::forAscii(*rt_, "%pipeStreamObject%")){};

  RuntimeState(const RuntimeState &) = delete;
  RuntimeState &operator=(const RuntimeState &) = delete;

  // Given a module name, returns the jsi::Object if it has already
  // been created/exists in the map or llvh::None if it has yet to be.
  llvh::Optional<jsi::Object> findRequiredModule(std::string modName) {
    auto iter = requireModules_.find(modName);
    if (iter == requireModules_.end()) {
      return llvh::None;
    } else {
      return iter->second.getProperty(*rt_, "exports").asObject(*rt_);
    }
  }
  // Given the name of the module and the respective jsi::Object,
  // adds the object as a member to the map. Returns a reference to
  // the object in the map.
  jsi::Object &addRequiredModule(std::string modName, jsi::Object module) {
    auto iterAndSuccess =
        requireModules_.emplace(std::move(modName), std::move(module));
    assert(iterAndSuccess.second && "Insertion must succeed.");
    return iterAndSuccess.first->second;
  }

  // Checks to see if the internal binding property has already been
  // initialized.
  bool internalBindingPropExists(const std::string &propName) const {
    return internalBindingProps_.find(propName) != internalBindingProps_.end();
  }
  // Adds a new property to internal binding, given the name of the property
  // and the respective jsi::Object.
  void setInternalBindingProp(std::string propName, jsi::Object prop) {
    internalBindingProps_.emplace(std::move(propName), std::move(prop));
  }
  // Returns the jsi::Object corresponding to the internalBinding
  // property asked for.
  jsi::Value getInternalBindingProp(const std::string &propName) const {
    auto iter = internalBindingProps_.find(propName);
    return jsi::Value(*rt_, iter->second).getObject(*rt_);
  }

  /// Given a path \p target to some file that is either expressed relative to
  /// dirname_ or an as absolute path from \p rootDir, compute a path to the
  /// target file from the module root.
  llvh::SmallString<32> resolvePath(
      llvh::StringRef target,
      llvh::StringRef rootDir);

  using InternalFunction = jsi::Value(
      RuntimeState &,
      const jsi::Value &,
      const jsi::Value *,
      size_t);
  /// Initializes a new JS function given a function pointer to the c++
  /// function.
  void defineJSFunction(
      InternalFunction *functionPtr,
      llvh::StringRef functionName,
      size_t numArgs,
      jsi::Object &bindingProp);

  jsi::Runtime &getRuntime() {
    return *rt_;
  }

  llvh::StringRef getDirname() const {
    return dirname_;
  }

  uv_loop_t *getLoop() {
    return loop_;
  }

  jsi::PropNameID &getTTYStreamPropId() {
    return ttyStreamPropId_;
  }

  jsi::PropNameID &getPipeStreamPropId() {
    return pipeStreamPropId_;
  }

  /// Pipe Socket types (for use in pipe_wrap.cpp)
  enum PipeSocketType { SOCKET, SERVER, IPC };

 private:
  // Runtime used to access internal binding properties.
  std::unique_ptr<jsi::Runtime> rt_;
  // Keeps track of all the 'require' modules already initialized.
  std::unordered_map<std::string, jsi::Object> requireModules_;
  // Stores all of the internal binding properties seen so far.
  std::unordered_map<std::string, jsi::Object> internalBindingProps_;
  // Stores the name of the directory where the file being run lives.
  llvh::SmallString<32> dirname_;
  // Event loop used for libuv.
  uv_loop_t *loop_;
  // Cached PropNameID corresponding to the tty_wrap stream object.
  jsi::PropNameID ttyStreamPropId_;
  // Cached PropNameID corresponding to the pipe_wrap stream object.
  jsi::PropNameID pipeStreamPropId_;
};
} // namespace facebook
#endif
