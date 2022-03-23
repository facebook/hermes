/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCCELL_H
#define HERMES_VM_GCCELL_H

#include "hermes/Support/Algorithms.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/VTable.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace hermes {
namespace vm {

template <typename T>
class ExternalStringPrimitive;
class VariableSizeRuntimeCell;

/// Return the allocation size for a fixed-size cell corresponding to class C.
/// This must be used instead of the builtin sizeof operator, since classes
/// further down the GCCell hierarchy may add (fixed-size) trailing objects and
/// redefine this method.
template <class C>
static constexpr uint32_t cellSize() {
  static_assert(HeapAlign % alignof(C) == 0, "insufficient heap alignment");
  return heapAlignSize(C::template cellSizeImpl<C>());
}

/// This class stores a CellKind and the size of a cell in 32 bits.
class KindAndSize {
 public:
  size_t getSize() const {
    return size_;
  }
  CellKind getKind() const {
    return static_cast<CellKind>(kind_);
  }
  const VTable *getVT() const {
    return VTable::vtableArray[kind_];
  }
  KindAndSize() = default;
  KindAndSize(CellKind kind, size_t sz)
      : size_(sz), kind_(static_cast<uint8_t>(kind)) {
    assert((sz & 1) == 0 && "LSB of size must always be zero.");
  }

  static constexpr uint32_t maxSize() {
    return (1ULL << kNumSizeBits) - 1;
  }

 private:
  using RawType = CompressedPointer::RawType;
  static constexpr size_t kNumBits = sizeof(RawType) * 8;
  static constexpr size_t kNumKindBits = 8;
  // On 64 bit platforms without compressed pointers, just make the size 32 bits
  // so that it can be accessed without any masking or shifting.
  static constexpr size_t kNumSizeBits =
      std::min<size_t>(kNumBits - kNumKindBits, 32);
  static_assert(
      kNumCellKinds < 256,
      "More cell kinds than available kind bits.");

  /// The size of the cell. Due to heap alignment, we are guaranteed that the
  /// least significant bit will always be zero, so it can be used for the
  /// mark bit. In order for that to work, this has to come first.
  RawType size_ : kNumSizeBits;
  /// The CellKind of the cell.
  RawType kind_ : kNumKindBits;
};

static_assert(
    std::is_trivial<KindAndSize>::value,
    "KindAndSize must be trivial");

/// This include file defines a GCCell that allows forward heap
/// traversal in a contiguous space: given a pointer to the head, you
/// can get the size, and thus get to the head of the next cell.
class GCCell {
  /// Either contains the CellKind and size of this cell, or a forwarding
  /// pointer.
  union {
    KindAndSize kindAndSize_;
    AssignableCompressedPointer forwardingPointer_;
  };

#ifndef NDEBUG
  static constexpr uint16_t kMagic{0xce11};
  // This is used to ensure a cell is valid and is not some other value.
  const uint16_t magic_{kMagic};
  /// Semi-unique (until the global counter overflows) id associated with every
  /// allocation.
  uint32_t _debugAllocationId_;
#endif

 public:
  GCCell() = default;

  // GCCell-s are not copyable (in the C++ sense).
  GCCell(const GCCell &) = delete;
  void operator=(const GCCell &) = delete;

  /// Return the allocated size of the object in bytes.
  uint32_t getAllocatedSize() const {
    return kindAndSize_.getSize();
  }

  /// Implementation of cellSize. Do not use this directly.
  template <class C>
  static constexpr uint32_t cellSizeImpl() {
    static_assert(std::is_convertible<C *, GCCell *>::value, "must be a cell");
    // ExternalStringPrimitive<T> is special: it extends VariableSizeRuntimeCell
    // but it's actually fixed-size.
    static_assert(
        !std::is_convertible<C *, VariableSizeRuntimeCell *>::value,
        "must be fixed-size");
    return sizeof(C);
  }

  /// Is true if the cell has a variable size, false if it has a fixed size.
  bool isVariableSize() const {
    return isVariableSize(getVT());
  }

  /// Is true if the cell has a variable size, false if it has a fixed size,
  /// assuming \p vtp is the proper VTable for this cell.
  static bool isVariableSize(const VTable *vtp) {
    return vtp->size == 0;
  }

  /// \return the kind of this cell.
  CellKind getKind() const {
    return kindAndSize_.getKind();
  }

  /// \return the vtable for this cell.
  /// \pre The cell must be valid.
  const VTable *getVT() const {
    assert(isValid() && "Called getVT() on an invalid cell");
    return kindAndSize_.getVT();
  }

  /// \return the KindAndSize for this cell.
  /// NOTE: this should only be used by the GC.
  KindAndSize getKindAndSize() const {
    return kindAndSize_;
  }

  /// Set the KindAndSize of this GCCell to \p kindAndSize.
  /// NOTE: this should only be used by the GC.
  void setKindAndSize(KindAndSize kindAndSize) {
    kindAndSize_ = kindAndSize;
  }

  /// We distinguish between two styles of forwarding pointer:
  /// "marked" and "unmarked".  When marked forwarding pointers are
  /// used, we can make efficient queries on a GCCell to determine
  /// whether a forwarding pointer has been installed.  (For example,
  /// we might use a low order "mark" bit in the vtp_ field to
  /// indicate that that holds a forwarding pointer.)  With unmarked
  /// forwarding pointers, some external data (e.g., external
  /// mark bits in mark/sweep/compact) must indicate whether the
  /// GCCell currently holds a forwarding pointer; there is no
  /// efficient query for this.

  /// The next two functions define unmarked forwarding pointers.

  /// Sets this cell to contain a forwarding pointer to another cell.
  /// NOTE: this should only be used by the GC.
  void setForwardingPointer(CompressedPointer cell) {
    forwardingPointer_ = cell;
  }

  /// \return a forwarding pointer to another object, if one exists. If one
  /// has not yet been set by \c setForwardingPointer(), this function is
  /// guaranteed to not return a pointer into the GC heap.
  /// NOTE: this should only be used by the GC.
  CompressedPointer getForwardingPointer() const {
    return forwardingPointer_;
  }

  /// These three functions implement marked forwarding pointers.

  /// Sets this cell to contain a forwarding pointer to another cell. Note that
  /// the heap is not well-formed in phases that set mark bits in live objects;
  /// the mark bits must be removed, or the objects declared dead, for
  /// well-formedness to be restored.
  /// NOTE: this should only be used by the GC.
  void setMarkedForwardingPointer(CompressedPointer cell) {
    forwardingPointer_ = CompressedPointer::fromRaw(cell.getRaw() | 0x1);
  }

  /// Assumes (and asserts) that a forwarding pointer has been set in
  /// this cell via setMarkedForwardingPointer.
  /// \return the forwarding pointer that was set.
  /// NOTE: this should only be used by the GC.
  CompressedPointer getMarkedForwardingPointer() const {
    assert(isMarked());
    return CompressedPointer::fromRaw(forwardingPointer_.getRaw() - 0x1);
  }

  /// \return whether a forwarding pointer has been set in this cell via
  /// setMarkedForwardingPointer.
  bool hasMarkedForwardingPointer() const {
    return isMarked();
  }

  const GCCell *nextCell() const {
    return reinterpret_cast<const GCCell *>(
        reinterpret_cast<const char *>(this) + getAllocatedSize());
  }
  GCCell *nextCell() {
    return reinterpret_cast<GCCell *>(
        reinterpret_cast<char *>(this) + getAllocatedSize());
  }

  /// \return true iff the cell is valid.
  ///
  /// Validity is defined by:
  ///   * The cell has a correct magic header
  ///   * The cell has a non-null vtable pointer that points to a VTable.
  bool isValid() const {
#ifndef NDEBUG
    // We only do the cell magic number check in debug builds.
    return magic_ == kMagic && kindAndSize_.getVT()->isValid();
#else
    return kindAndSize_.getVT()->isValid();
#endif
  }

  /// \return true iff the cell is valid (i.e., its vtable is valid)
  // and has the given \p expectedKind.
  bool isValid(CellKind expectedKind) const {
    return kindAndSize_.getVT()->isValid(expectedKind);
  }

  /// Placement new
  static void *operator new(size_t, void *p) {
    return p;
  }

  // Make new/delete illegal for GC cells.
  static void *operator new(size_t) = delete;
  static void operator delete(void *) = delete;

  /// \return a semi-unique (until the global counter overflows) id of this
  /// memory allocation.
  uint64_t getDebugAllocationId() const {
#ifndef NDEBUG
    return _debugAllocationId_;
#else
    return 0;
#endif
  }

  /// Returns whether the cell's mark bit is set.
  bool isMarked() const {
    return forwardingPointer_.getRaw() & 0x1;
  }

  static constexpr uint32_t maxSize() {
    return KindAndSize::maxSize();
  }
};

/// A VariableSizeRuntimeCell is a GCCell with a variable size only known
/// at runtime, whereas GCCell is for fixed-size objects.
/// \see ArrayStorage for how to inherit from this class correctly.
class VariableSizeRuntimeCell : public GCCell {
 public:
  uint32_t getSize() const {
    return getAllocatedSize();
  }

  /// Sets the size of the current cell to be \p sz.
  /// NOTE: This should only be used by the GC, and only to shrink objects.
  /// \pre sz is already heap-aligned.
  void setSizeFromGC(uint32_t sz) {
    assert(
        isVariableSize() &&
        "Cannot call setSizeFromGC on a non-variable size cell");
    assert(
        sz >= sizeof(VariableSizeRuntimeCell) &&
        "Should not allocate a VariableSizeRuntimeCell of size less than "
        "the size of a cell");
    setKindAndSize(KindAndSize{getKind(), sz});
  }
};

static_assert(
    alignof(GCCell) <= HeapAlign,
    "GCCell's alignment exceeds the alignment requirement of the heap");

#ifdef NDEBUG
static_assert(
    sizeof(GCCell) == sizeof(CompressedPointer) &&
        sizeof(VariableSizeRuntimeCell) == sizeof(CompressedPointer),
    "Cell metadata should only be the size of a single pointer.");
#endif

static const char kInvalidHeapValue = (char)0xcc;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCCELL_H
