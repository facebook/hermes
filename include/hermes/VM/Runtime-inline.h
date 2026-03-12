/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIME_INLINE_H
#define HERMES_VM_RUNTIME_INLINE_H

#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

inline Callable *Runtime::getBuiltinCallable(unsigned builtinMethodID) {
  assert(
      builtinMethodID < BuiltinMethod::_count &&
      "out of bound builtinMethodID");
  return builtins_[builtinMethodID];
}

void Runtime::registerBuiltin(
    BuiltinMethod::Enum builtinIndex,
    Callable *builtin) {
  assert(builtins_[(size_t)builtinIndex] == nullptr && "Builtin already set");
  builtins_[(size_t)builtinIndex] = builtin;
}

inline void Runtime::enqueueJob(Callable *job) {
  jobQueue_.push_back(job);
}

inline Handle<HiddenClass> Runtime::getHiddenClassForPrototype(
    JSObject *proto,
    Handle<HiddenClass> root) {
  assert(root && "root must be non-null");
  return root;
}

inline Handle<HiddenClass> Runtime::getLazyHiddenClassForPrototype(
    JSObject *proto) {
  const PinnedValue<HiddenClass> *clazz = &lazyObjectClass_;
  return Handle<HiddenClass>::vmcast(clazz);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_RUNTIME_INLINE_H
