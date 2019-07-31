/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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
  CallResult<HermesValue> func(void *, Runtime *, NativeArgs);
#define NATIVE_FUNCTION_TYPED(func, type) \
  template <typename T>                   \
  CallResult<HermesValue> func(void *, Runtime *, NativeArgs);
#define NATIVE_FUNCTION_TYPED_2(func, type, type2) \
  template <typename T, CellKind C>                \
  CallResult<HermesValue> func(void *, Runtime *, NativeArgs);
#include "hermes/VM/NativeFunctions.def"

using CreatorFunction = CallResult<HermesValue>(Runtime *, Handle<JSObject>);
/// Get a human-readable name of a native function.
std::string getFunctionName(NativeFunctionPtr);
std::string getFunctionName(CreatorFunction *);

} // namespace vm
} // namespace hermes
#endif // HERMES_VM_JSNATIVEFUNCTIONS
