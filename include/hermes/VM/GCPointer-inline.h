/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GCPOINTER_INL_H
#define HERMES_VM_GCPOINTER_INL_H

#include "hermes/VM/GCPointer.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/PointerBase-inline.h"
#include "hermes/VM/PointerBase.h"

namespace hermes {
namespace vm {

inline GCPointerBase::GCPointerBase(PointerBase *base, void *ptr)
    : ptr_(
#ifdef HERMESVM_COMPRESSED_POINTERS
          base->pointerToBased(ptr)
#else
          ptr
#endif
      ) {
  // In some build configurations this parameter is unused.
  (void)base;
}

inline void *GCPointerBase::get(PointerBase *base) const {
#ifdef HERMESVM_COMPRESSED_POINTERS
  return base->basedToPointer(ptr_);
#else
  (void)base;
  return ptr_;
#endif
}

inline void *GCPointerBase::getNonNull(PointerBase *base) const {
#ifdef HERMESVM_COMPRESSED_POINTERS
  return base->basedToPointerNonNull(ptr_);
#else
  (void)base;
  return ptr_;
#endif
}

template <typename T>
template <typename NeedsBarriers>
GCPointer<T>::GCPointer(
    PointerBase *base,
    T *ptr,
    GC *gc,
    NeedsBarriers needsBarrierUnused)
    : GCPointerBase(base, ptr) {
  assert(
      (!ptr || gc->validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  if (NeedsBarriers::value) {
    gc->writeBarrier(&ptr_, ptr);
  } else {
    assert(!gc->needsWriteBarrier(&ptr_, ptr));
  }
}

inline void GCPointerBase::set(PointerBase *base, void *ptr, GC *gc) {
  assert(
      (!ptr || gc->validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
#ifdef HERMESVM_COMPRESSED_POINTERS
  ptr_ = base->pointerToBased(ptr);
#else
  (void)base;
  ptr_ = ptr;
#endif
  gc->writeBarrier(&ptr_, ptr);
}

inline GCPointerBase::StorageType GCPointerBase::getStorageType() const {
  return ptr_;
}

inline GCPointerBase::StorageType &GCPointerBase::getLoc(GC *gc) {
  assert(gc->inGC() && "Can only use GCPointer::getLoc within GC.");
  return ptr_;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_INL_H
