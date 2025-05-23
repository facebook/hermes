/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STACKFRAME_INLINE_H
#define HERMES_VM_STACKFRAME_INLINE_H

#include "hermes/VM/StackFrame.h"

namespace hermes {
namespace vm {

template <bool isConst>
inline Callable *StackFramePtrT<isConst>::getCalleeClosureUnsafe() const {
  return vmcast<Callable>(getCalleeClosureOrCBRef());
}

template <bool isConst>
inline Handle<Callable> StackFramePtrT<isConst>::getCalleeClosureHandleUnsafe()
    const {
  return Handle<Callable>::vmcast(&getCalleeClosureOrCBRef());
}

template <bool isConst>
typename StackFramePtrT<isConst>::QualifiedCB *
StackFramePtrT<isConst>::getCalleeCodeBlock() const {
  auto &ref = getCalleeClosureOrCBRef();
  if (ref.isObject()) {
    if (auto *func = dyn_vmcast<JSFunction>(ref))
      return func->getCodeBlock();
    else
      return nullptr;
  } else {
    return ref.template getNativePointer<CodeBlock>();
  }
}

template <bool isConst>
inline Handle<Environment> StackFramePtrT<isConst>::getEnvironmentHandle()
    const {
  return getEnvironmentRef().isUndefined()
      ? HandleRootOwner::makeNullHandle<Environment>()
      : Handle<Environment>::vmcast_or_null(&getEnvironmentRef());
}

template <bool isConst>
inline Environment *StackFramePtrT<isConst>::getEnvironment() const {
  return getEnvironmentRef().isUndefined()
      ? nullptr
      : vmcast_or_null<Environment>(getEnvironmentRef());
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STACKFRAME_INLINE_H
