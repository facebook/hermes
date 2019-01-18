/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_VTABLE_H
#define HERMES_VM_VTABLE_H

#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/Metadata.h"

#include "llvm/Support/raw_ostream.h"

#include <cstdint>

namespace hermes {
namespace vm {

// Forward declarations.
class GCCell;

/// The "metadata" for an allocated GC cell: the kind of cell, and
/// methods to "mark" (really, to invoke a GC callback on JS values in
/// the block) and (optionally) finalize the cell.
struct VTable {
  /// The cell kind.
  const CellKind kind;
  /// `size` should be the size of the cell if it is fixed, or 0 if it is
  /// variable sized.
  /// If it is variable sized, it should inherit from \see
  /// VariableSizeRuntimeCell.
  const uint32_t size;
  /// Called during GC when an object becomes unreachable. Must not perform any
  /// allocations or access any garbage-collectable objects.  Unless an
  /// operation is documented to be safe to call from a finalizer, it probably
  /// isn't.
  using FinalizeCallback = void(GCCell *, GC *gc);
  FinalizeCallback *const finalize_;
  /// Call gc functions on weak-reference-holding objects.
  using MarkWeakCallback = void(GCCell *, GC *gc);
  MarkWeakCallback *const markWeak_;
  /// Report if there is any size contribution from an object beyond the GC.
  /// Used to report any externally allocated memory for metric gathering.
  using MallocSizeCallback = size_t(GCCell *);
  MallocSizeCallback *const mallocSize_;
  /// Ask the cell what its post-compaction size will be.
  /// This should not modify the cell.
  using CompactSizeCallback = gcheapsize_t(const GCCell *);
  CompactSizeCallback *const compactSize_;
  /// Compact the cell, decreasing any size-related fields inside the cell.
  using CompactCallback = void(GCCell *);
  CompactCallback *const compact_;

  constexpr explicit VTable(
      CellKind kind,
      uint32_t size,
      FinalizeCallback *finalize = nullptr,
      MarkWeakCallback *markWeak = nullptr,
      MallocSizeCallback *mallocSize = nullptr,
      CompactSizeCallback *compactSize = nullptr,
      CompactCallback *compact = nullptr)
      : kind(kind),
        size(heapAlignSize(size)),
        finalize_(finalize),
        markWeak_(markWeak),
        mallocSize_(mallocSize),
        compactSize_(compactSize),
        compact_(compact) {}

  bool isVariableSize() const {
    return size == 0;
  }

  void finalizeIfExists(GCCell *cell, GC *gc) const {
    if (finalize_) {
      finalize_(cell, gc);
    }
  }

  void finalize(GCCell *cell, GC *gc) const {
    assert(
        finalize_ &&
        "Cannot unconditionally finalize if it doesn't have a finalize pointer");
    finalize_(cell, gc);
  }

  void markWeakIfExists(GCCell *cell, GC *gc) const {
    if (markWeak_) {
      markWeak_(cell, gc);
    }
  }

  size_t getMallocSize(GCCell *cell) const {
    return mallocSize_ ? mallocSize_(cell) : 0;
  }

  bool canBeCompacted() const {
    return compact_;
  }

  /// Tell the \p cell to shrink itself, and return its new size. If the cell
  /// doesn't have any shrinking to do, return the \p origSize.
  gcheapsize_t getCompactedSize(GCCell *cell, gcheapsize_t origSize) const {
    return canBeCompacted() ? heapAlignSize(compactSize_(cell)) : origSize;
  }

  void compact(GCCell *cell) const {
    compact_(cell);
  }
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const VTable &vt);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_VTABLE_H
