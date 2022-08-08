/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
  static_assert(
      cellSize<T>() >= minAllocationSize() &&
          cellSize<T>() <= maxAllocationSize(),
      "Cell size outside legal range.");
  assert(
      VTable::getVTable(T::getCellKind())->size && "Cell is not fixed size.");
  return makeA<T, true /* fixedSize */, hasFinalizer, longLived>(
      cellSize<T>(), std::forward<Args>(args)...);
}

template <
    typename T,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
T *GCBase::makeAVariable(uint32_t size, Args &&...args) {
  // If size is greater than the max, we should OOM.
  assert(
      size >= GC::minAllocationSize() && "Cell size is smaller than minimum");
  assert(
      !VTable::getVTable(T::getCellKind())->size &&
      "Cell is not variable size.");
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
  assert(
      !!VTable::getVTable(T::getCellKind())->finalize_ ==
          (hasFinalizer == HasFinalizer::Yes) &&
      "hasFinalizer should be set iff the cell has a finalizer.");
#ifdef HERMESVM_GC_RUNTIME
  T *ptr = runtimeGCDispatch([&](auto *gc) {
    return gc->template makeA<T, fixedSize, hasFinalizer, longLived>(
        size, std::forward<Args>(args)...);
  });
#else
  T *ptr =
      static_cast<GC *>(this)->makeA<T, fixedSize, hasFinalizer, longLived>(
          size, std::forward<Args>(args)...);
#endif
#ifndef NDEBUG
  ptr->setDebugAllocationIdInGC(nextObjectID());
#endif
#ifdef HERMES_MEMORY_INSTRUMENTATION
  newAlloc(ptr, size);
#endif
  return ptr;
}

#ifdef HERMESVM_GC_RUNTIME
constexpr uint32_t GCBase::maxAllocationSizeImpl() {
  // Return the lesser of the two GC options' max allowed sizes.
  return std::min({
#define GC_KIND(kind) kind::maxAllocationSizeImpl(),
      RUNTIME_GC_KINDS
#undef GC_KIND
  });
}

constexpr uint32_t GCBase::minAllocationSizeImpl() {
  // Return the greater of the two GC options' min allowed sizes.
  return std::max({
#define GC_KIND(kind) kind::minAllocationSizeImpl(),
      RUNTIME_GC_KINDS
#undef GC_KIND
  });
}
#endif

constexpr uint32_t GCBase::maxAllocationSize() {
  return std::min(GC::maxAllocationSizeImpl(), GCCell::maxSize());
}

constexpr uint32_t GCBase::minAllocationSize() {
  return GC::minAllocationSizeImpl();
}

template <typename Acceptor>
void GCBase::markWeakRefsIfNecessary(
    GCCell *cell,
    CellKind kind,
    Acceptor &acceptor) {
  markWeakRefsIfNecessary(
      cell,
      kind,
      acceptor,
      std::is_convertible<Acceptor &, WeakRefAcceptor &>{});
}

template <typename Acceptor>
inline void GCBase::markCell(GCCell *cell, Acceptor &acceptor) {
  markCell(cell, cell->getKind(), acceptor);
}

template <typename Acceptor>
inline void GCBase::markCell(GCCell *cell, CellKind kind, Acceptor &acceptor) {
  SlotVisitor<Acceptor> visitor(acceptor);
  markCell(visitor, cell, kind);
}

template <typename Acceptor>
inline void
GCBase::markCell(SlotVisitor<Acceptor> &visitor, GCCell *cell, CellKind kind) {
  visitor.visit(
      cell, Metadata::metadataTable[static_cast<size_t>(kind)].offsets);
  markWeakRefsIfNecessary(cell, kind, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithinRange(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    CellKind kind,
    const char *begin,
    const char *end) {
  visitor.visitWithinRange(
      cell,
      Metadata::metadataTable[static_cast<size_t>(kind)].offsets,
      begin,
      end);
  markWeakRefsIfNecessary(cell, kind, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithNames(
    SlotVisitorWithNames<Acceptor> &visitor,
    GCCell *cell) {
  const CellKind kind = cell->getKind();
  visitor.visit(
      cell,
      Metadata::metadataTable[static_cast<size_t>(kind)].offsets,
      Metadata::metadataTable[static_cast<size_t>(kind)].names);
  markWeakRefsIfNecessary(cell, kind, visitor.acceptor_);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_INLINE_H
