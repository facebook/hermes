/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCCELL_H
#define HERMES_VM_GCCELL_H

#include "hermes/VM/CellKind.h"
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
/// redefine this method. Example usage:
///
/// CallResult<HermesValue> MyCell::create(Runtime *runtime, ...) {
///   void *mem = runtime->alloc(cellSize<MyCell>());
///   ...
template <class C>
static constexpr uint32_t cellSize() {
  static_assert(HeapAlign % alignof(C) == 0, "insufficient heap alignment");
  return C::template cellSizeImpl<C>();
}

/// This include file defines a GCCell that allows forward heap
/// traversal in a contiguous space: given a pointer to the head, you
/// can get the size, and thus get to the head of the next cell.
class GCCell {
  /// Pointer to the virtual table which also serves as a forwarding pointer.
  const VTable *vtp_;

#ifndef NDEBUG
  static constexpr uint16_t kMagic{0xce11};
  // This is used to ensure a cell is valid and is not some other value.
  const uint16_t magic_{kMagic};
  /// Semi-unique (until the global counter overflows) id associated with every
  /// allocation.
  uint64_t _debugAllocationId_;
#endif

 public:
  explicit GCCell(GC *gc, const VTable *vtp);

  /// Makes a new GCCell with only a type and a size.
  /// NOTE: This bypasses some debugging checks in the GCCell constructor taking
  /// a GC parameter, so this should only be used in cases where the debug
  /// checks will be wrong.
  explicit GCCell(const VTable *vtp);

  // GCCell-s are not copyable (in the C++ sense).
  GCCell(const GCCell &) = delete;
  void operator=(const GCCell &) = delete;

  /// Return the allocated size of the object in bytes.
  uint32_t getAllocatedSize() const;

  /// Return the allocated size of the object in bytes, assuming \p vtp
  /// is the proper VTable for this cell.
  /// (Some GC's like MarkSweepCompact temporarily overwrite the vtp
  /// and store it elsewhere, but still need to find the object size
  /// when vtp is incorrect.)
  uint32_t getAllocatedSize(const VTable *vtp) const;

  /// Implementation of cellSize. Do not use this directly.
  template <class C>
  static constexpr uint32_t cellSizeImpl() {
    static_assert(std::is_convertible<C *, GCCell *>::value, "must be a cell");
    // ExternalStringPrimitive<T> is special: it extends VariableSizeRuntimeCell
    // but it's actually fixed-size.
    static_assert(
        !std::is_convertible<C *, VariableSizeRuntimeCell *>::value ||
            std::is_same<C, ExternalStringPrimitive<char>>::value ||
            std::is_same<C, ExternalStringPrimitive<char16_t>>::value,
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
    return getVT()->kind;
  }

  /// \return the vtable for this cell.
  /// \pre The cell must be valid.
  const VTable *getVT() const {
    assert(isValid() && "Called getVT() on an invalid cell");
    return vtp_;
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
  void setForwardingPointer(const GCCell *cell) {
    vtp_ = reinterpret_cast<const VTable *>(cell);
  }

  /// \return a forwarding pointer to another object, if one exists. If one
  /// has not yet been set by \c setForwardingPointer(), this function is
  /// guaranteed to not return a pointer into the GC heap.
  /// NOTE: this should only be used by the GC.
  GCCell *getForwardingPointer() const {
    return reinterpret_cast<GCCell *>(const_cast<VTable *>(vtp_));
  }

  /// These three functions implement marked forwarding pointers.

  /// Sets this cell to contain a forwarding pointer to another cell.
  /// NOTE: this should only be used by the GC.
  void setMarkedForwardingPointer(const GCCell *cell) {
    vtp_ = reinterpret_cast<const VTable *>(cell);
    setMarkBit();
  }

  /// Assumes (and asserts) that a forwarding pointer has been set in
  /// this cell via setMarkedForwardingPointer.
  /// \return the forwarding pointer that was set.
  /// NOTE: this should only be used by the GC.
  GCCell *getMarkedForwardingPointer() const {
    return reinterpret_cast<GCCell *>(
        const_cast<VTable *>(removeKnownMarkBit(vtp_)));
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
    return magic_ == kMagic && vtp_ && vtp_->isValid();
#else
    return vtp_ && vtp_->isValid();
#endif
  }

  /// \return true iff the cell is valid (i.e., its vtable is valid)
  // and has the given \p expectedKind.
  bool isValid(CellKind expectedKind) const {
    return vtp_ && vtp_->isValid(expectedKind);
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

  /// Sets the cell's mark bit.  May be used, for example, to indicate that
  /// the cell contains a forwarding pointer.  Note that the heap is not
  /// well-formed in phases that set mark bits in live objects; the mark bits
  /// must be removed, or the objects declared dead, for well-formedness to be
  /// restored.
  void setMarkBit() {
    vtp_ = reinterpret_cast<const VTable *>(
        reinterpret_cast<uintptr_t>(vtp_) | 0x1);
  }

  /// Returns whether the cell's mark bit is set.
  bool isMarked() const {
    return isMarked(vtp_);
  }

  static bool isMarked(const VTable *vtp) {
    return reinterpret_cast<uintptr_t>(vtp) & 0x1;
  }

  /// If the cell has any associated external memory, return the size (in bytes)
  /// of that external memory, else zero.
  inline gcheapsize_t externalMemorySize() const {
    return getVT()->externalMemorySize(this);
  }

 private:
  /// This version assumes that the bit is set, and that it can
  /// therefore subtract 1.
  template <typename T>
  static T *removeKnownMarkBit(T *vt) {
    assert(isMarked(vt));
    return reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(vt) - 0x1);
  }
};

/// A VariableSizeRuntimeCell is a GCCell with a variable size only known
/// at runtime, whereas GCCell is for fixed-size objects.
/// \see ArrayStorage for how to inherit from this class correctly.
class VariableSizeRuntimeCell : public GCCell {
 protected:
  /// variableSize_ represents the number of bytes used by this object at
  /// runtime. This field will never increase, but may decrease during a GC
  /// compaction.
  uint32_t variableSize_;
  /// A VariableSizeRuntimeCell must be constructed from a fixed size, and has
  /// that same size for its lifetime.
  /// To change the size, allocate a new object.
  VariableSizeRuntimeCell(GC *gc, const VTable *vtp, uint32_t size)
      : GCCell(gc, vtp), variableSize_(heapAlignSize(size)) {
    // Need to align to the GC here, since the GC doesn't know about this field.
    assert(
        size >= sizeof(VariableSizeRuntimeCell) &&
        "Should not allocate a VariableSizeRuntimeCell of size less than "
        "the size of a cell");
  }

  /// Makes a new VariableSizeRuntimeCell with only a type and a size.
  VariableSizeRuntimeCell(const VTable *vtp, uint32_t size)
      : GCCell(vtp), variableSize_(heapAlignSize(size)) {
    // Need to align to the GC here, since the GC doesn't know about this field.
    assert(
        size >= sizeof(VariableSizeRuntimeCell) &&
        "Should not allocate a VariableSizeRuntimeCell of size less than "
        "the size of a cell");
  }

 public:
  uint32_t getSize() const {
    return variableSize_;
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
    variableSize_ = sz;
  }
};

static_assert(
    alignof(GCCell) <= HeapAlign,
    "GCCell's alignment exceeds the alignment requirement of the heap");

#ifdef NDEBUG
inline GCCell::GCCell(GC *, const VTable *vtp) : vtp_(vtp) {}

inline GCCell::GCCell(const VTable *vtp) : vtp_(vtp) {}
#endif

inline uint32_t GCCell::getAllocatedSize(const VTable *vtp) const {
  // The size is either fixed in the VTable, or variable and stored as some
  // offset in that VTable
  assert(vtp && "Should not have a null vtable pointer");
  uint32_t size = 0;
  if (isVariableSize(vtp)) {
    // Variable size.
    size = static_cast<const VariableSizeRuntimeCell *>(this)->getSize();
  } else {
    // Fixed size
    size = vtp->size;
  }
  assert(
      size != 0 &&
      "Should not have zero size stored into a "
      "cell");
  return size;
}

inline uint32_t GCCell::getAllocatedSize() const {
  return getAllocatedSize(getVT());
}

static const char kInvalidHeapValue = (char)0xcc;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCCELL_H
