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

namespace hermes {
namespace vm {

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *GCBase::makeAFixed(Args &&...args) {
  const uint32_t size = cellSize<T>();
  T *ptr = static_cast<GC *>(this)
               ->makeA<T, true /* fixedSize */, hasFinalizer, longLived>(
                   size, std::forward<Args>(args)...);
#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
  getAllocationLocationTracker().newAlloc(ptr, size);
#endif
  return ptr;
}

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *GCBase::makeAVariable(uint32_t size, Args &&...args) {
  size = heapAlignSize(size);
  T *ptr = static_cast<GC *>(this)
               ->makeA<T, false /* fixedSize */, hasFinalizer, longLived>(
                   size, std::forward<Args>(args)...);
#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
  getAllocationLocationTracker().newAlloc(ptr, size);
#endif
  return ptr;
}

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
