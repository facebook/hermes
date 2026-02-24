/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKROOT_INLINE_H
#define HERMES_VM_WEAKROOT_INLINE_H

#include "hermes/VM/GC.h"
#include "hermes/VM/SmallHermesValue-inline.h"
#include "hermes/VM/WeakRoot.h"

namespace hermes {
namespace vm {

GCCell *WeakRootBase::get(PointerBase &base, GC &gc) const {
  if (!*this)
    return nullptr;
  return getNonNull(base, gc);
}

GCCell *WeakRootBase::getNonNull(PointerBase &base, GC &gc) const {
  GCCell *ptr = CompressedPointer::getNonNull(base);
  gc.weakRefReadBarrier(ptr);
  return ptr;
}

template <typename T>
T *WeakRoot<T>::getNonNull(PointerBase &base, GC &gc) const {
  return static_cast<T *>(WeakRootBase::getNonNull(base, gc));
}

template <typename T>
T *WeakRoot<T>::get(PointerBase &base, GC &gc) const {
  return static_cast<T *>(WeakRootBase::get(base, gc));
}

SymbolID WeakRootSymbolID::get(GC &gc) {
  SymbolID id = (SymbolID)(*this);
  gc.weakRefReadBarrier(id);
  return id;
}

void WeakSmallHermesValue::setObject(CompressedPointer newVal) {
  assert(newVal && "Object pointer must not be null");
  setNoBarrier(encodeObjectValue(newVal));
}

void WeakSmallHermesValue::setObject(PointerBase &base, GCCell *ptr) {
  assert(ptr && "Object pointer must not be null");
  setNoBarrier(encodeObjectValue(ptr, base));
}

SymbolID WeakSmallHermesValue::getSymbol(GC &gc) const {
  SymbolID id = SmallHermesValue::getSymbol();
  gc.weakRefReadBarrier(id);
  return id;
}

GCCell *WeakSmallHermesValue::getPointer(PointerBase &base, GC &gc) const {
  GCCell *ptr = SmallHermesValue::getPointer(base);
  gc.weakRefReadBarrier(ptr);
  return ptr;
}

GCCell *WeakSmallHermesValue::getObject(PointerBase &base, GC &gc) const {
  assert(isObject());
  return getPointer(base, gc);
}

} // namespace vm
} // namespace hermes

#endif
