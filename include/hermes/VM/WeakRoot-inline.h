/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKROOT_INLINE_H
#define HERMES_VM_WEAKROOT_INLINE_H

#include "hermes/VM/GC.h"
#include "hermes/VM/WeakRoot.h"

namespace hermes {
namespace vm {

void *WeakRootBase::get(PointerBase &base, GC &gc) const {
  if (!*this)
    return nullptr;
  GCCell *ptr = CompressedPointer::getNonNull(base);
  gc.weakRefReadBarrier(ptr);
  return ptr;
}

template <typename T>
T *WeakRoot<T>::get(PointerBase &base, GC &gc) const {
  return static_cast<T *>(WeakRootBase::get(base, gc));
}

} // namespace vm
} // namespace hermes

#endif
