/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_POINTERBASE_H
#define HERMES_VM_POINTERBASE_H

#include "hermes/VM/AlignedStorage.h"
#include "llvm/Support/Compiler.h"

#include <cassert>
#include <cstdint>

#if defined(HERMESVM_ALLOW_COMPRESSED_POINTERS) && LLVM_PTR_SIZE == 8 && \
    defined(HERMESVM_GC_NONCONTIG_GENERATIONAL)
/// \macro HERMESVM_COMPRESSED_POINTERS
/// \brief If defined, store pointers as 32 bits in GC-managed Hermes objects.
#define HERMESVM_COMPRESSED_POINTERS
#endif

namespace hermes {
namespace vm {

#ifdef HERMESVM_COMPRESSED_POINTERS

class BasedPointer final {
 public:
  explicit operator bool() const;

  bool operator==(BasedPointer other) const;

  bool operator!=(BasedPointer other) const;

  BasedPointer() = default;

  /// Efficiently yields the representation of a null pointer.
  explicit BasedPointer(std::nullptr_t);

  inline uint32_t getSegmentIndex() const;

  inline uint32_t getOffset() const;

  inline uint32_t getRawValue() const;

 private:
  static inline uint32_t computeSegmentAndOffset(const void *heapAddr);

  // The low AlignedStorage::kLogSize bits are the offset, and the
  // remaining upper bits are the segment index.
  uint32_t segAndOffset_{0};

  inline explicit BasedPointer(void *heapAddr);

  // Only PointerBase needs to access this field and the constructor. To every
  // other part of the system this is an opaque type that PointerBase handles
  // translations for.
  friend class PointerBase;
};

/// PointerBase is an opaque type meant to be used as a base pointer.
/// This is used to implement a mechanism where 32-bit offsets are used as
/// pointers instead of a full pointer.
class PointerBase {
 public:
  /// Initialize the PointerBase.
  inline PointerBase();

  /// Convert a pointer to an offset from this.
  /// \param ptr A pointer to convert.
  /// \pre ptr must start after this, and must end before 2 ^ 32 after this.
  inline BasedPointer pointerToBased(void *ptr) const;
  /// Same as above, but has a precondition that the pointer is not null.
  /// \pre ptr is not null.
  BasedPointer pointerToBasedNonNull(void *ptr) const;

  /// Convert an offset from this PointerBase into a real pointer that can be
  /// de-referenced.
  /// \param ptr An offset (based pointer) to convert.
  void *basedToPointer(BasedPointer ptr) const;

  /// Same as above, but has a precondition that the pointer is not null.
  /// \pre ptr is not null.
  inline void *basedToPointerNonNull(BasedPointer ptr) const;

  /// Record \p segStart as the start address for the given segment index.
  inline void setSegment(unsigned idx, void *segStart);

  static constexpr unsigned kNullPtrSegmentIndex = 0;
  static constexpr unsigned kYGSegmentIndex = 1;
  static constexpr unsigned kFirstOGSegmentIndex = 2;

 private:
  /// Returns the segment at the given index.
  inline void *segmentForIndex(unsigned idx) const;

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
  static constexpr unsigned kMaxSegments = 1 << (32 - AlignedStorage::kLogSize);
  /// We add one entry so that segmentMap_[0] can contain the null pointer.
  static constexpr unsigned kSegmentMapSize = kMaxSegments + 1;
  SegmentPtr segmentMap_[kSegmentMapSize];
};

/// @name Inline implementations that don't depend on other include files.
/// Implementations that do are in PointerBase-inline.h.
/// @{

inline BasedPointer::operator bool() const {
  return segAndOffset_ != 0;
}

inline bool BasedPointer::operator==(BasedPointer other) const {
  return segAndOffset_ == other.segAndOffset_;
}

inline bool BasedPointer::operator!=(BasedPointer other) const {
  return !(segAndOffset_ == other.segAndOffset_);
}

inline PointerBase::PointerBase() {
  segmentMap_[kNullPtrSegmentIndex] = nullptr;
}

inline void *PointerBase::basedToPointerNonNull(BasedPointer ptr) const {
  assert(ptr && "Null pointer given to basedToPointerNonNull");
  // This implementation is the same for null and non-null pointers.
  return basedToPointer(ptr);
}

/// @}

#else // ! HERMESVM_COMPRESSED_POINTERS

/// If we're not using compressed pointers, we still need a null implementation
/// of PointerBase.
class PointerBase {};

#endif // HERMESVM_COMPRESSED_POINTERS

} // namespace vm
} // namespace hermes

#endif
