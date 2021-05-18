/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCPOINTER_INL_H
#define HERMES_VM_GCPOINTER_INL_H

#include "hermes/VM/GCPointer.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/PointerBase.h"

namespace hermes {
namespace vm {

template <typename T>
template <typename NeedsBarriers>
GCPointer<T>::GCPointer(PointerBase *base, T *ptr, GC *gc, NeedsBarriers)
    : GCPointerBase(base, ptr) {
  assert(
      (!ptr || gc->validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  if (NeedsBarriers::value) {
    gc->constructorWriteBarrier(this, ptr);
  } else {
    assert(!gc->needsWriteBarrier(this, ptr));
  }
}

inline void GCPointerBase::set(PointerBase *base, GCCell *ptr, GC *gc) {
  assert(
      (!ptr || gc->validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc->writeBarrier(this, ptr);
  setNoBarrier(CompressedPointer(base, ptr));
}

inline void GCPointerBase::setNull(GC *gc) {
  gc->snapshotWriteBarrier(this);
  setNoBarrier(CompressedPointer(nullptr));
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_INL_H
