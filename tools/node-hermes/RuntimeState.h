/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef NODE_HERMES_RUNTIMESTATE_H
#define NODE_HERMES_RUNTIMESTATE_H

#include "hermes/hermes.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Program.h"
#include "llvh/Support/Signals.h"

namespace facebook {

// Manages the module objects relevant to the 'require' and
// 'internalBinding' function calls.
class RuntimeState {
 public:
  // Parametrized constructor, copy constructor, assignment constructor
  // respectively.
  RuntimeState(jsi::Runtime &rt)
      : rt_(rt),
        internalBindingPropNameID(
            jsi::PropNameID::forAscii(rt, "internalBinding")){};
  RuntimeState(const RuntimeState &) = delete;
  RuntimeState &operator=(const RuntimeState &) = delete;

  // Given a module name, returns the jsi::Object if it has already
  // been created/exists in the map or llvh::None if it has yet to be.
  llvh::Optional<jsi::Object> findRequiredModule(std::string modName) {
    auto iter = requireModules_.find(modName);
    if (iter == requireModules_.end()) {
      return llvh::None;
    } else {
      return iter->second.getProperty(rt_, "exports").asObject(rt_);
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
  bool internalBindingPropExists(const jsi::String &propName) {
    return rt_.global()
        .getProperty(rt_, internalBindingPropNameID)
        .asObject(rt_)
        .hasProperty(rt_, propName);
  }
  // Adds a new property to internal binding, given the name of the property
  // and the respective jsi::Object.
  void setInternalBindingProp(const jsi::String &propName, jsi::Object prop) {
    rt_.global()
        .getProperty(rt_, internalBindingPropNameID)
        .asObject(rt_)
        .setProperty(rt_, propName, prop);
  }
  // Returns the jsi::Value corresponding to the internalBinding
  // property asked for.
  jsi::Value getInternalBindingProp(const jsi::String &propName) {
    return rt_.global()
        .getProperty(rt_, internalBindingPropNameID)
        .asObject(rt_)
        .getProperty(rt_, propName);
  }

 private:
  // Keeps track of all the 'require' modules already initialized.
  std::unordered_map<std::string, jsi::Object> requireModules_;
  // Runtime used to access internal binding properties.
  jsi::Runtime &rt_;
  // Cached PropNameID corresponding to "internalBinding" for faster
  // accesses/lookup.
  jsi::PropNameID internalBindingPropNameID;
};
} // namespace facebook
#endif
