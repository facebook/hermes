/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_EMPTYCELL_H
#define HERMES_UNITTESTS_VMRUNTIME_EMPTYCELL_H

#include "TestHelpers.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/FillerCell.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/VTable.h"

namespace hermes {
namespace vm {

/// An uninitialized cell that is \p Size bytes wide, for use with \c
/// DummyRuntime, in tests. It is internally just a FillerCell but provides a
/// convenient template to create cells of a specific size (e.g. some fraction
/// of the segment size).
template <size_t Size>
struct EmptyCell final : public FillerCell {
  static constexpr size_t size() {
    return Size;
  }

  static EmptyCell *create(DummyRuntime &runtime) {
    return runtime.makeAVariable<EmptyCell>(size());
  }

  static EmptyCell *createLongLived(DummyRuntime &runtime) {
    return runtime.makeAVariable<EmptyCell, HasFinalizer::No, LongLived::Yes>(
        size());
  }

  /// Touch bytes in the cell from the end of its header until the end of its
  /// memory region, at page sized intervals.
  ///
  /// \return The number of pages touched.
  inline size_t touch();
};

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
