/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCBASE_INLINE_H
#define HERMES_VM_GCBASE_INLINE_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase.h"

#include "hermes/Support/Algorithms.h"

namespace hermes {
namespace vm {

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *GCBase::makeAFixed(Args &&...args) {
  return makeA<T, true /* fixedSize */, hasFinalizer, longLived>(
      cellSize<T>(), std::forward<Args>(args)...);
}

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *GCBase::makeAVariable(uint32_t size, Args &&...args) {
  return makeA<T, false /* fixedSize */, hasFinalizer, longLived>(
      heapAlignSize(size), std::forward<Args>(args)...);
}

template <
    typename T,
    bool fixedSize,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *GCBase::makeA(uint32_t size, Args &&...args) {
  assert(
      isSizeHeapAligned(size) && "Size must be aligned before reaching here");
#ifdef HERMESVM_GC_RUNTIME
  T *ptr;
  // Use static_cast below because we know the actual type of the heap.
  switch (getKind()) {
    case GCBase::HeapKind::HADES:
      ptr = llvh::cast<HadesGC>(this)
                ->makeA<T, fixedSize, hasFinalizer, longLived>(
                    size, std::forward<Args>(args)...);
      break;
    case GCBase::HeapKind::NCGEN:
      ptr =
          llvh::cast<GenGC>(this)->makeA<T, fixedSize, hasFinalizer, longLived>(
              size, std::forward<Args>(args)...);
      break;
    case GCBase::HeapKind::MALLOC:
      llvm_unreachable(
          "MallocGC should not be used with the RuntimeGC build config");
      break;
  }
#else
  T *ptr =
      static_cast<GC *>(this)->makeA<T, fixedSize, hasFinalizer, longLived>(
          size, std::forward<Args>(args)...);
#endif
#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
  getAllocationLocationTracker().newAlloc(ptr, size);
#endif
  return ptr;
}

#ifdef HERMESVM_GC_RUNTIME
constexpr uint32_t GCBase::maxAllocationSize() {
  // Return the lesser of the two GC options' max allowed sizes.
  return min(HadesGC::maxAllocationSize(), GenGC::maxAllocationSize());
}

constexpr uint32_t GCBase::minAllocationSize() {
  // Return the greater of the two GC options' min allowed sizes.
  return max(HadesGC::minAllocationSize(), GenGC::minAllocationSize());
}
#endif

template <typename Acceptor>
void GCBase::markWeakRefsIfNecessary(
    GCCell *cell,
    const VTable *vt,
    Acceptor &acceptor) {
  markWeakRefsIfNecessary(
      cell, vt, acceptor, std::is_convertible<Acceptor &, WeakRefAcceptor &>{});
}

template <typename Acceptor>
inline void GCBase::markCell(GCCell *cell, Acceptor &acceptor) {
  markCell(cell, cell->getVT(), acceptor);
}

template <typename Acceptor>
inline void
GCBase::markCell(GCCell *cell, const VTable *vt, Acceptor &acceptor) {
  SlotVisitor<Acceptor> visitor(acceptor);
  markCell(visitor, cell, vt);
}

template <typename Acceptor>
inline void GCBase::markCell(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    const VTable *vt) {
  visitor.visit(cell, metaTable_[static_cast<size_t>(vt->kind)]);
  markWeakRefsIfNecessary(cell, vt, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithinRange(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    const VTable *vt,
    const char *begin,
    const char *end) {
  visitor.visitWithinRange(
      cell, metaTable_[static_cast<size_t>(vt->kind)], begin, end);
  markWeakRefsIfNecessary(cell, vt, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithNames(
    SlotVisitorWithNames<Acceptor> &visitor,
    GCCell *cell) {
  const VTable *vt = cell->getVT();
  visitor.visit(cell, metaTable_[static_cast<size_t>(vt->kind)]);
  markWeakRefsIfNecessary(cell, vt, visitor.acceptor_);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_INLINE_H
