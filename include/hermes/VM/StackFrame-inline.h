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
StackFramePtrT<isConst>::getCalleeCodeBlock(Runtime &runtime) const {
  auto &ref = getCalleeClosureOrCBRef();
  if (ref.isObject()) {
    if (auto *func = dyn_vmcast<JSFunction>(ref))
      return func->getCodeBlock(runtime);
    else
      return nullptr;
  } else {
    return ref.template getNativePointer<CodeBlock>();
  }
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STACKFRAME_INLINE_H
