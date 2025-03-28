/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCPOINTER_INL_H
#define HERMES_VM_GCPOINTER_INL_H

#include "hermes/VM/GCPointer.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

template <typename T>
template <typename NeedsBarriers>
inline GCPointer<T>::GCPointer(PointerBase &base, T *ptr, GC &gc, NeedsBarriers)
    : GCPointerBase(CompressedPointer::encode(ptr, base)) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  if (NeedsBarriers::value) {
    gc.constructorWriteBarrier(this, ptr);
  } else {
    assert(!gc.needsWriteBarrier(this, ptr));
  }
}

template <typename T>
inline GCPointerInLargeObj<T>::GCPointerInLargeObj(
    PointerBase &base,
    GC &gc,
    T *ptr,
    const GCCell *owningObj)
    : GCPointerBase(CompressedPointer::encode(ptr, base)) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  // Constructing a GCPointer on an object that supports large allocation always
  // needs to perform write barrier. We may revisit this decision if we see a
  // case that needs optimization.
  gc.constructorWriteBarrierForLargeObj(owningObj, this, ptr);
}

template <typename T>
inline void GCPointer<T>::set(PointerBase &base, T *ptr, GC &gc) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrier(this, ptr);
  setNoBarrier(CompressedPointer::encode(ptr, base));
}

template <typename T>
inline void GCPointer<T>::setNonNull(PointerBase &base, T *ptr, GC &gc) {
  assert(
      gc.validPointer(ptr) && "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrier(this, ptr);
  setNoBarrier(CompressedPointer::encodeNonNull(ptr, base));
}

template <typename T>
inline void
GCPointer<T>::set(PointerBase &base, const GCPointer<T> &ptr, GC &gc) {
  assert(
      (!ptr || gc.validPointer(ptr.get(base))) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrier(this, ptr.get(base));
  setNoBarrier(ptr);
}

template <typename T>
inline void GCPointerInLargeObj<T>::set(
    PointerBase &base,
    GC &gc,
    T *ptr,
    const GCCell *owningObj) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrierForLargeObj(owningObj, this, ptr);
  setNoBarrier(CompressedPointer::encode(ptr, base));
}

template <typename T>
inline void GCPointerInLargeObj<T>::setNonNull(
    PointerBase &base,
    GC &gc,
    T *ptr,
    const GCCell *owningObj) {
  assert(
      gc.validPointer(ptr) && "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrierForLargeObj(owningObj, this, ptr);
  setNoBarrier(CompressedPointer::encodeNonNull(ptr, base));
}

inline void GCPointerBase::setNull(GC &gc) {
  gc.snapshotWriteBarrier(this);
  setNoBarrier(CompressedPointer(nullptr));
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_INL_H
