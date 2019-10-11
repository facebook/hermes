/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_POINTERBASE_INLINE_H
#define HERMES_VM_POINTERBASE_INLINE_H

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/AlignedStorage.h"

#include <cassert>
#include <cstdint>

#if defined(HERMESVM_ALLOW_COMPRESSED_POINTERS) && LLVM_PTR_SIZE == 8 && \
    defined(HERMESVM_GC_NONCONTIG_GENERATIONAL)
/// \macro HERMESVM_COMPRESSED_POINTERS
/// \brief If defined, store pointers as 32 bits in GC-managed Hermes objects.
#define HERMESVM_COMPRESSED_POINTERS
#endif

#ifdef HERMESVM_COMPRESSED_POINTERS

namespace hermes {
namespace vm {

/// @name Inline implementations
/// @{

inline BasedPointer::BasedPointer(std::nullptr_t) : segAndOffset_(0) {}

inline BasedPointer::BasedPointer(void *heapAddr)
    : segAndOffset_(computeSegmentAndOffset(heapAddr)) {}

/*static*/
inline uint32_t BasedPointer::computeSegmentAndOffset(const void *heapAddr) {
  assert(heapAddr != nullptr);
  uintptr_t addrAsInt = reinterpret_cast<uintptr_t>(heapAddr);
  void *segStart = AlignedStorage::start(heapAddr);
  /// segStart is the start of the AlignedStorage containing
  /// heapAddr, so its low AlignedStorage::kLogSize are zeros.
  /// Thus, offset, below, will be the offset of heapAddr within the
  /// segment; bits above AlignedStorage::kLogSize will be zero.
  uintptr_t offset = addrAsInt - reinterpret_cast<uintptr_t>(segStart);
  /// Now get the segment index, and shift it so that it's bits do not
  /// overlap with those of offset.
  return (AlignedHeapSegment::segmentIndexFromStart(segStart)
          << AlignedStorage::kLogSize) |
      offset;
}

inline uint32_t BasedPointer::getSegmentIndex() const {
  return segAndOffset_ >> AlignedStorage::kLogSize;
}

inline uint32_t BasedPointer::getOffset() const {
  return segAndOffset_ & ((1 << AlignedStorage::kLogSize) - 1);
}

inline uint32_t BasedPointer::getRawValue() const {
  return segAndOffset_;
}

inline void *PointerBase::basedToPointer(BasedPointer ptr) const {
  // The value
  char *segBase = reinterpret_cast<char *>(segmentMap_[ptr.getSegmentIndex()]);
  return segBase + ptr.getRawValue();
}

inline BasedPointer PointerBase::pointerToBased(void *ptr) const {
  if (!ptr) {
    // Null pointers are special and can't be "compressed" in the same
    // way as non-null pointers.  So we make a special case for null.
    return BasedPointer{};
  }
  return pointerToBasedNonNull(ptr);
}

inline BasedPointer PointerBase::pointerToBasedNonNull(void *ptr) const {
  assert(ptr && "Null pointer given to pointerToBasedNonNull");
  return BasedPointer{ptr};
}

inline void PointerBase::setSegment(unsigned idx, void *segStart) {
  assert(segStart == AlignedStorage::start(segStart) && "Precondition");
  // See the explanation for the "bias" above the declaration of
  // segmentMap_ in PointerBase.h.
  char *bias =
      reinterpret_cast<char *>(segStart) - (idx << AlignedStorage::kLogSize);
  segmentMap_[idx] = bias;
  AlignedHeapSegment::setSegmentIndexFromStart(segStart, idx);
}

/// @}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_COMPRESSED_POINTERS

#endif
