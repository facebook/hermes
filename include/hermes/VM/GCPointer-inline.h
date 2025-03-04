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

template <typename NeedsBarriers>
GCPointerBase::GCPointerBase(
    PointerBase &base,
    GCCell *ptr,
    GC &gc,
    NeedsBarriers)
    : CompressedPointer(CompressedPointer::encode(ptr, base)) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  if (NeedsBarriers::value) {
    gc.constructorWriteBarrier(this, ptr);
  } else {
    assert(!gc.needsWriteBarrier(this, ptr));
  }
}

GCPointerBase::GCPointerBase(
    PointerBase &base,
    GCCell *ptr,
    GC &gc,
    const GCCell *owningObj)
    : CompressedPointer(CompressedPointer::encode(ptr, base)) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  // Constructing a GCPointer on an object that supports large allocation always
  // needs to perform write barrier. We may revisit this decision if we see a
  // case that needs optimization.
  gc.constructorWriteBarrierForLargeObj(owningObj, this, ptr);
}

inline void GCPointerBase::set(PointerBase &base, GCCell *ptr, GC &gc) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrier(this, ptr);
  setNoBarrier(CompressedPointer::encode(ptr, base));
}

inline void GCPointerBase::setNonNull(PointerBase &base, GCCell *ptr, GC &gc) {
  assert(
      gc.validPointer(ptr) && "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrier(this, ptr);
  setNoBarrier(CompressedPointer::encodeNonNull(ptr, base));
}

inline void
GCPointerBase::set(PointerBase &base, CompressedPointer ptr, GC &gc) {
  assert(
      (!ptr || gc.validPointer(ptr.get(base))) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrier(this, ptr.get(base));
  setNoBarrier(ptr);
}

inline void GCPointerBase::setInLargeObj(
    PointerBase &base,
    GCCell *ptr,
    GC &gc,
    const GCCell *owningObj) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrierForLargeObj(owningObj, this, ptr);
  setNoBarrier(CompressedPointer::encode(ptr, base));
}

inline void GCPointerBase::setNonNullInLargeObj(
    PointerBase &base,
    GCCell *ptr,
    GC &gc,
    const GCCell *owningObj) {
  assert(
      gc.validPointer(ptr) && "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrierForLargeObj(owningObj, this, ptr);
  setNoBarrier(CompressedPointer::encodeNonNull(ptr, base));
}

inline void GCPointerBase::setInLargeObj(
    PointerBase &base,
    CompressedPointer ptr,
    GC &gc,
    const GCCell *owningObj) {
  assert(
      (!ptr || gc.validPointer(ptr.get(base))) &&
      "Cannot set a GCPointer to an invalid pointer");
  // Write barrier must happen before the write.
  gc.writeBarrierForLargeObj(owningObj, this, ptr.get(base));
  setNoBarrier(ptr);
}

inline void GCPointerBase::setNull(GC &gc) {
  gc.snapshotWriteBarrier(this);
  setNoBarrier(CompressedPointer(nullptr));
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_INL_H
