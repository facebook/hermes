/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_POINTERBASE_H
#define HERMES_VM_POINTERBASE_H

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/SegmentInfo.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/MathExtras.h"

#include <cassert>
#include <cstdint>

namespace hermes {
namespace vm {

#ifdef HERMESVM_COMPRESSED_POINTERS
class BasedPointer final {
 public:
  using StorageType = uint32_t;
  explicit operator bool() const;

  inline bool operator==(BasedPointer other) const;

  inline bool operator!=(BasedPointer other) const;

  BasedPointer() = default;

  /// Efficiently yields the representation of a null pointer.
  explicit BasedPointer(std::nullptr_t);
  explicit BasedPointer(StorageType raw);

  BasedPointer &operator=(std::nullptr_t);

  inline StorageType getRawValue() const;

 private:
#ifndef HERMESVM_CONTIGUOUS_HEAP
  // Only PointerBase needs these functions. To every other part of the system
  // this is an opaque type that PointerBase handles translations for.
  friend class PointerBase;
  inline uint32_t getSegmentIndex() const;
  inline uint32_t getOffset() const;
#endif

  StorageType raw_;
};
#endif

/// PointerBase is an opaque type meant to be used as a base pointer.
/// This is used to implement a mechanism where 32-bit offsets are used as
/// pointers instead of a full pointer.
class PointerBase {
 public:
  /// Initialize the PointerBase.
  inline PointerBase();

  /// Record \p segStart as the start address for the given segment index.
  inline void setSegment(unsigned idx, void *segStart);

#ifdef HERMESVM_COMPRESSED_POINTERS
 private:
  friend class CompressedPointer;

  /// Convert a pointer to an offset from this.
  /// \param ptr A pointer to convert.
  /// \pre ptr must start after this, and must end before 2 ^ 32 after this.
  inline BasedPointer pointerToBased(const void *ptr) const;
  /// Same as above, but has a precondition that the pointer is not null.
  /// \pre ptr is not null.
  inline BasedPointer pointerToBasedNonNull(const void *ptr) const;

  /// Convert an offset from this PointerBase into a real pointer that can be
  /// de-referenced.
  /// \param ptr An offset (based pointer) to convert.
  inline void *basedToPointer(BasedPointer ptr) const;

  /// Same as above, but has a precondition that the pointer is not null.
  /// \pre ptr is not null.
  inline void *basedToPointerNonNull(BasedPointer ptr) const;

#ifndef HERMESVM_CONTIGUOUS_HEAP
  static constexpr unsigned kNullPtrSegmentIndex = 0;
  /// To support 32-bit GC pointers in segmentIdx:offset form,
  /// segmentMap_ maps segment indices to "biased segment starts."
  /// This "bias" speeds up the decoding of a BasedPointer.  If the segmentMap_
  /// contained the actual segment starts, decoding a segmentIdx:offset form
  /// would require a shift to extract the segmentIdx, a lookup of the
  /// segment start, a subtraction of the shifted index to obtain the
  /// offset, and an addition to get the final native pointer.
  /// That is,
  ///   segmentIdx = basedPtr >> kLogSize;
  ///   segmentStart = segmentMap_[segmentIdx]
  ///   offset = basedPtr - (segmentIdx << kLogSize)
  ///   native_ptr = segmentStart + offset
  ///
  /// The bias allows us to avoid the subtraction: the segment table entry
  /// for index i is <segment-start> - (i << kLogSize)
  /// So the calculation can be:
  ///   segmentIdx = based_ptr >> kLogSize;
  ///   biasedSegmentStart = segmentMap_[segmentIdx]
  ///       [== segmentStart - (segmentIdx << kLogSize)]
  ///   native_ptr = basedPtr + biasedSegmentStart
  ///       [== basedPtr + segmentStart - (segmentIdx << kLogSize)
  ///        == segmentStart + (basedPtr - (segmentIdx << kLogSize))
  ///        == segmentStart + offset]
  ///
  /// Because we use the same 8-byte alignment in the offsets as we do
  /// in object pointers, the max heap size for 32-bit pointers is
  /// 4GB.  With 4MB segments, that's 1024 segments, plus one for
  /// segmentMap_[0], which is used for the null pointer
  /// representation.
  using SegmentPtr = void *;
  static constexpr unsigned kSegmentMapSize = 1
      << (32 - AlignedStorage::kLogSize);
  /// We subtract one entry so that segmentMap_[0] can contain the null pointer.
  static constexpr unsigned kMaxSegments = kSegmentMapSize - 1;
  SegmentPtr segmentMap_[kSegmentMapSize];
#endif
#endif // HERMESVM_COMPRESSED_POINTERS
};

/// @name Inline implementations.
/// @{

#ifdef HERMESVM_COMPRESSED_POINTERS
inline BasedPointer::BasedPointer(std::nullptr_t) : raw_(0) {}
inline BasedPointer::BasedPointer(StorageType raw) : raw_(raw) {}

inline BasedPointer &BasedPointer::operator=(std::nullptr_t) {
  raw_ = 0;
  return *this;
}

inline BasedPointer::StorageType BasedPointer::getRawValue() const {
  return raw_;
}

inline BasedPointer::operator bool() const {
  return raw_ != 0;
}

inline bool BasedPointer::operator==(BasedPointer other) const {
  return raw_ == other.raw_;
}

inline bool BasedPointer::operator!=(BasedPointer other) const {
  return raw_ != other.raw_;
}

#ifdef HERMESVM_CONTIGUOUS_HEAP
inline void *PointerBase::basedToPointerNonNull(BasedPointer ptr) const {
  assert(ptr && "Null pointer given to basedToPointerNonNull");
  uintptr_t addr = reinterpret_cast<uintptr_t>(this) + ptr.getRawValue();
  return reinterpret_cast<void *>(addr);
}

inline PointerBase::PointerBase() {
  // The PointerBase must be segment aligned, so that the compressed pointer
  // corresponding to the start of a segment is also segment aligned.
  assert(llvh::alignmentAdjustment(this, AlignedStorage::size()) == 0);
}

inline void *PointerBase::basedToPointer(BasedPointer ptr) const {
  return ptr ? basedToPointerNonNull(ptr) : nullptr;
}

inline BasedPointer PointerBase::pointerToBasedNonNull(const void *ptr) const {
  assert(ptr && "Null pointer given to pointerToBasedNonNull");
  uintptr_t offset = (uintptr_t)ptr - (uintptr_t)this;
  assert(llvh::isUInt<32>(offset) && "Pointer out of range");
  return BasedPointer{static_cast<uint32_t>(offset)};
}

inline BasedPointer PointerBase::pointerToBased(const void *ptr) const {
  return ptr ? pointerToBasedNonNull(ptr) : BasedPointer{nullptr};
}

inline void PointerBase::setSegment(unsigned idx, void *segStart) {
  assert(segStart == AlignedStorage::start(segStart) && "Precondition");
  SegmentInfo::setSegmentIndexFromStart(segStart, idx);
}
#else
inline void *PointerBase::basedToPointerNonNull(BasedPointer ptr) const {
  assert(ptr && "Null pointer given to basedToPointerNonNull");
  // This implementation is the same for null and non-null pointers.
  return basedToPointer(ptr);
}

inline PointerBase::PointerBase() {
  segmentMap_[kNullPtrSegmentIndex] = nullptr;
}

inline uint32_t BasedPointer::getSegmentIndex() const {
  return raw_ >> AlignedStorage::kLogSize;
}

inline uint32_t BasedPointer::getOffset() const {
  return raw_ & ((1 << AlignedStorage::kLogSize) - 1);
}

inline void *PointerBase::basedToPointer(BasedPointer ptr) const {
  char *segBase = reinterpret_cast<char *>(segmentMap_[ptr.getSegmentIndex()]);
  return segBase + ptr.getRawValue();
}

inline BasedPointer PointerBase::pointerToBased(const void *ptr) const {
  if (!ptr) {
    // Null pointers are special and can't be "compressed" in the same
    // way as non-null pointers.  So we make a special case for null.
    return BasedPointer{};
  }
  return pointerToBasedNonNull(ptr);
}

inline BasedPointer PointerBase::pointerToBasedNonNull(const void *ptr) const {
  assert(ptr && "Null pointer given to pointerToBasedNonNull");
  uintptr_t addrAsInt = reinterpret_cast<uintptr_t>(ptr);
  void *segStart = AlignedStorage::start(ptr);
  /// segStart is the start of the AlignedStorage containing
  /// ptr, so its low AlignedStorage::kLogSize are zeros.
  /// Thus, offset, below, will be the offset of ptr within the
  /// segment; bits above AlignedStorage::kLogSize will be zero.
  uintptr_t offset = addrAsInt - reinterpret_cast<uintptr_t>(segStart);
  /// Now get the segment index, and shift it so that its bits do not
  /// overlap with those of offset.
  uint32_t raw = (SegmentInfo::segmentIndexFromStart(segStart)
                  << AlignedStorage::kLogSize) |
      offset;
  return BasedPointer{raw};
}

inline void PointerBase::setSegment(unsigned idx, void *segStart) {
  assert(segStart == AlignedStorage::start(segStart) && "Precondition");
  // See the explanation for the "bias" above the declaration of
  // segmentMap_ in PointerBase.h.
  char *bias =
      reinterpret_cast<char *>(segStart) - (idx << AlignedStorage::kLogSize);
  segmentMap_[idx] = bias;
  SegmentInfo::setSegmentIndexFromStart(segStart, idx);
}
#endif // HERMESVM_CONTIGUOUS_HEAP
#else
inline PointerBase::PointerBase() {}

inline void PointerBase::setSegment(unsigned idx, void *segStart) {
  assert(segStart == AlignedStorage::start(segStart) && "Precondition");
  SegmentInfo::setSegmentIndexFromStart(segStart, idx);
}
#endif // HERMESVM_COMPRESSED_POINTERS

/// @}

} // namespace vm
} // namespace hermes

#endif
