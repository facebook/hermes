/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_RUNTIME_INLINE_H
#define HERMES_VM_RUNTIME_INLINE_H

#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

inline Callable *Runtime::getBuiltinCallable(unsigned builtinMethodID) {
  assert(
      builtinMethodID < BuiltinMethod::_count &&
      "out of bound builtinMethodID");
  return builtins_[builtinMethodID];
}

inline Handle<HiddenClass> Runtime::getHiddenClassForPrototype(
    JSObject *proto,
    unsigned reservedSlots) {
  assert(
      reservedSlots <= InternalProperty::NumInternalProperties &&
      "out of bounds");
  PinnedHermesValue *clazz = &rootClazzes_[reservedSlots];
  assert(!clazz->isUndefined() && "must initialize root classes before use");
  return Handle<HiddenClass>::vmcast(clazz);
}

inline HiddenClass *Runtime::getHiddenClassForPrototypeRaw(
    JSObject *proto,
    unsigned reservedSlots) {
  return *getHiddenClassForPrototype(proto, reservedSlots);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_RUNTIME_INLINE_H
