/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIME_INLINE_H
#define HERMES_VM_RUNTIME_INLINE_H

#include "hermes/Inst/Builtins.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

inline NativeFunction *Runtime::getBuiltinNativeFunction(
    unsigned builtinMethodID) {
  assert(
      builtinMethodID < inst::BuiltinMethod::_count &&
      "invalid builtinMethodID");
  return builtins_[builtinMethodID];
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_RUNTIME_INLINE_H
