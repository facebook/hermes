/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_FILLERCELL_H
#define HERMES_VM_FILLERCELL_H

#include "hermes/VM/Runtime.h"
#include "hermes/VM/VTable.h"

namespace hermes {
namespace vm {

/// This class exists for cases where the GC wants to fill some heap region
/// with a non-object, just to allow a contiguous heap to "parse" correctly.
class FillerCell : public VariableSizeRuntimeCell {
  friend void FillerCellBuildMeta(const GCCell *, Metadata::Builder &);
  static const VTable vt;

 public:
  using size_type = uint32_t;

  static const VTable *vtable() {
    return &vt;
  };

  static constexpr CellKind getCellKind() {
    return CellKind::FillerCellKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::FillerCellKind;
  }

  static FillerCell *create(Runtime &runtime, size_type size) {
    assert(
        size >= sizeof(FillerCell) &&
        "Cannot make a FillerCell smaller than the baseline for a FillerCell");
    assert(
        isSizeHeapAligned(size) &&
        "A FillerCell must have a heap aligned size");
    return runtime.makeAVariable<FillerCell>(size);
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_FILLERCELL_H
