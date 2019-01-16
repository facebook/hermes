#ifndef HERMES_VM_GCPOINTER_INL_H
#define HERMES_VM_GCPOINTER_INL_H

#include "hermes/VM/GCPointer.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

template <typename T>
template <typename NeedsBarriers>
GCPointer<T>::GCPointer(T *ptr, GC *gc, NeedsBarriers needsBarrierUnused)
    : GCPointerBase(ptr) {
  assert(
      (!ptr || gc->validPointer(ptr)) &&
      "Cannot construct a GCPointer from an invalid pointer");
  if (NeedsBarriers::value) {
    gc->writeBarrier(&ptr_, ptr);
  } else {
    assert(!gc->needsWriteBarrier(&ptr_, ptr));
  }
}

inline void GCPointerBase::set(void *ptr, GC *gc) {
  assert(
      (!ptr || gc->validPointer(ptr)) &&
      "Cannot set a GCPointer to an invalid pointer");
  ptr_ = ptr;
  gc->writeBarrier(&ptr_, ptr);
}

inline void *&GCPointerBase::getLoc(GC *gc) {
  assert(gc->inGC() && "Can only use GCPointer::getLoc within GC.");
  return ptr_;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCPOINTER_INL_H
