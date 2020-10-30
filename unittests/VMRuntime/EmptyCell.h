/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_EMPTYCELL_H
#define HERMES_UNITTESTS_VMRUNTIME_EMPTYCELL_H

#include "TestHelpers.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/VTable.h"

namespace hermes {
namespace vm {

/// An uninitialized cell that is \p Size bytes wide, for use with \c
/// DummyRuntime, in tests.  \p FixedSize = false is passed into the allocation
/// functions to simulate allocating a variabled size cell, even though this
/// cell is not a subclass of \c VariableSizeRuntimeCell and so each template
/// instantiation has a statically determined size.
template <size_t Size>
struct EmptyCell final : public GCCell {
  static const VTable vt;
  static constexpr size_t size() {
    return Size;
  }

  static EmptyCell *create(DummyRuntime &runtime) {
    return runtime.makeAFixed<EmptyCell>(&runtime.getHeap());
  }

  static EmptyCell *createLongLived(DummyRuntime &runtime) {
    return runtime
        .makeAFixed<EmptyCell<Size>, HasFinalizer::No, LongLived::Yes>(
            &runtime.getHeap());
  }

  template <class C>
  static constexpr uint32_t cellSizeImpl() {
    static_assert(
        std::is_convertible<C *, EmptyCell *>::value, "must be an EmptyCell");
    return C::size();
  }

  EmptyCell(GC *gc) : GCCell(gc, &vt) {}

  /// Touch bytes in the cell from the end of its header until the end of its
  /// memory region, at page sized intervals.
  ///
  /// \return The number of pages touched.
  inline size_t touch();
};

template <size_t Size>
const VTable EmptyCell<Size>::vt{CellKind::UninitializedKind, Size};

template <size_t Size>
struct VarSizedEmptyCell final : public VariableSizeRuntimeCell {
  static const VTable vt;
  static constexpr size_t size() {
    return Size;
  }

  static VarSizedEmptyCell *create(DummyRuntime &runtime) {
    return runtime.makeAVariable<VarSizedEmptyCell>(size(), &runtime.getHeap());
  }

  VarSizedEmptyCell(GC *gc) : VariableSizeRuntimeCell(gc, &vt, Size) {}

  /// Touch bytes in the cell from the end of its header until the end of its
  /// memory region, at page sized intervals.
  ///
  /// \return The number of pages touched.
  inline size_t touch();
};

template <size_t Size>
const VTable VarSizedEmptyCell<Size>::vt{CellKind::UninitializedKind, 0};

template <size_t Size>
size_t EmptyCell<Size>::touch() {
  const auto PS = hermes::oscompat::page_size();

  volatile char *begin = reinterpret_cast<char *>(this);
  volatile char *extra = begin + sizeof(EmptyCell);
  volatile char *end = begin + size();

  size_t n = 0;
  for (auto p = extra; p < end; p += PS, ++n)
    *p = 1;

  return n;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_EMPTYCELL_H
