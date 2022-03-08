/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSNATIVEFUNCTIONS
#define HERMES_VM_JSNATIVEFUNCTIONS

#include "hermes/VM/CallResult.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

#define NATIVE_FUNCTION(func) \
  CallResult<HermesValue> func(void *, Runtime &, NativeArgs);
#include "hermes/VM/NativeFunctions.def"

/// Get a human-readable name of a native function.
const char *getFunctionName(NativeFunctionPtr);
const char *getFunctionName(NativeConstructor::CreatorFunction *);

} // namespace vm
} // namespace hermes
#endif // HERMES_VM_JSNATIVEFUNCTIONS
