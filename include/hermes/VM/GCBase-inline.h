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
#include "hermes/VM/GCCell-inline.h"

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
      cellSize<T>() >= GC::minAllocationSize() &&
          cellSize<T>() <= GC::maxNormalAllocationSize(),
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
    CanBeLarge canBeLarge,
    MayFail mayFail,
    class... Args>
T *GCBase::makeAVariable(uint32_t size, Args &&...args) {
  // For now, when mayFail == MayFail::Yes, we must have canBeLarge ==
  // CanBeLarge::Yes.
  static_assert(
      (mayFail == MayFail::No) || (canBeLarge == CanBeLarge::Yes),
      "Only large allocation can actually fail");
  assert(
      ((canBeLarge == CanBeLarge::No) ||
       VTable::getVTable(T::getCellKind())->allowLargeAlloc) &&
      "T must support large allocation when canBeLarge is Yes");
  // If size is greater than the max, we should OOM.
  assert(
      size >= GC::minAllocationSize() && "Cell size is smaller than minimum");
  assert(
      !VTable::getVTable(T::getCellKind())->size &&
      "Cell is not variable size.");
  return makeA<
      T,
      false /* fixedSize */,
      hasFinalizer,
      longLived,
      canBeLarge,
      mayFail>(heapAlignSize(size), std::forward<Args>(args)...);
}

template <
    typename T,
    bool fixedSize,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    CanBeLarge canBeLarge,
    MayFail mayFail,
    class... Args>
T *GCBase::makeA(uint32_t size, Args &&...args) {
  // For now, when mayFail == MayFail::Yes, we must have canBeLarge ==
  // CanBeLarge::Yes.
  static_assert(
      (mayFail == MayFail::No) || (canBeLarge == CanBeLarge::Yes),
      "Only large allocation can actually fail");
  assert(
      ((canBeLarge == CanBeLarge::No) ||
       VTable::getVTable(T::getCellKind())->allowLargeAlloc) &&
      "T must support large allocation when canBeLarge is Yes");
  assert(
      isSizeHeapAligned(size) && "Size must be aligned before reaching here");
  assert(
      !!VTable::getVTable(T::getCellKind())->finalize_ ==
          (hasFinalizer == HasFinalizer::Yes) &&
      "hasFinalizer should be set iff the cell has a finalizer.");

  T *ptr = static_cast<GC *>(this)
               ->makeAImpl<
                   T,
                   fixedSize,
                   hasFinalizer,
                   longLived,
                   canBeLarge,
                   mayFail>(size, std::forward<Args>(args)...);
#if !defined(NDEBUG) || defined(HERMES_MEMORY_INSTRUMENTATION)
  if constexpr (mayFail == MayFail::Yes) {
    // If it fails, a nullptr is allowed and simply return it to the caller.
    if (LLVM_UNLIKELY(!ptr))
      return nullptr;
  }
#endif
#ifndef NDEBUG
  ptr->setDebugAllocationIdInGC(nextObjectID());
#endif
#ifdef HERMES_MEMORY_INSTRUMENTATION
  newAlloc(ptr, ptr->getAllocatedSizeSlow());
#endif
  return ptr;
}

/// Allocate two young gen objects of the size \p size1 + \p size2.
/// Calls the constructors of types T1 and T2 with the arguments \p t1Args and
/// \p t2Args respectively.
/// \pre the TOTAL size must be able to fit in the young gen
///  (can be checked with canAllocateInYoungGen(size)).
/// \post both result pointers are in the young gen.
/// \return a pointer to size1 size T1, and a pointer to size2 size T2.
template <typename T1, typename T2, typename... T1Args, typename... T2Args>
inline std::pair<T1 *, T2 *> GCBase::make2YoungGenUnsafe(
    uint32_t size1,
    std::tuple<T1Args...> t1Args,
    uint32_t size2,
    std::tuple<T2Args...> t2Args) {
  assert(
      isSizeHeapAligned(size1) &&
      "Call to alloc must use a size aligned to HeapAlign");
  assert(
      isSizeHeapAligned(size2) &&
      "Call to alloc must use a size aligned to HeapAlign");

  assert(
      !VTable::getVTable(T1::getCellKind())->finalize_ &&
      "must not have a finalizer");
  assert(
      !VTable::getVTable(T2::getCellKind())->finalize_ &&
      "must not have a finalizer");
#ifdef HERMESVM_GC_RUNTIME
  T *ptr = runtimeGCDispatch([&](auto *gc) {
    return gc->template make2YoungGenUnsafeImpl<T1, T2>(
        size1, t1Args, size2, t2Args);
  });
#else
  auto [t1, t2] = static_cast<GC *>(this)->make2YoungGenUnsafeImpl<T1, T2>(
      size1, t1Args, size2, t2Args);
#endif
#ifndef NDEBUG
  t1->setDebugAllocationIdInGC(nextObjectID());
  t2->setDebugAllocationIdInGC(nextObjectID());
#endif
#ifdef HERMES_MEMORY_INSTRUMENTATION
  newAlloc(t1, t1->getAllocatedSize());
  newAlloc(t2, t2->getAllocatedSize());
#endif

  assert(inYoungGen(t1) && "must be in young gen");
  assert(inYoungGen(t2) && "must be in young gen");

  return {t1, t2};
}

#ifdef HERMESVM_GC_RUNTIME
constexpr uint32_t GCBase::maxNormalAllocationSizeImpl() {
  // Return the lesser of the two GC options' max allowed sizes.
  return std::min({
#define GC_KIND(kind) kind::maxNormalAllocationSizeImpl(),
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

template <typename Acceptor>
inline void GCBase::markCell(Acceptor &acceptor, GCCell *cell) {
  SlotVisitor::visit(
      acceptor,
      cell,
      Metadata::metadataTable[static_cast<size_t>(cell->getKind())].offsets);
}

template <typename Acceptor>
inline void GCBase::markCellWithinRange(
    Acceptor &acceptor,
    GCCell *cell,
    const char *begin,
    const char *end) {
  SlotVisitor::visitWithinRange(
      acceptor,
      cell,
      Metadata::metadataTable[static_cast<size_t>(cell->getKind())].offsets,
      begin,
      end);
}

template <typename Acceptor>
inline void GCBase::markCellWithNames(Acceptor &acceptor, GCCell *cell) {
  const CellKind kind = cell->getKind();
  SlotVisitor::visitWithNames(
      acceptor,
      cell,
      Metadata::metadataTable[static_cast<size_t>(kind)].offsets,
      Metadata::metadataTable[static_cast<size_t>(kind)].names);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_INLINE_H
