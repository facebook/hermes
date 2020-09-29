/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_POINTERBASE_INLINE_H
#define HERMES_VM_POINTERBASE_INLINE_H

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SegmentInfo.h"

#include <cassert>
#include <cstdint>

namespace hermes {
namespace vm {

/// @name Inline implementations
/// @{

inline BasedPointer::BasedPointer(std::nullptr_t) : raw_(0) {}

inline BasedPointer &BasedPointer::operator=(std::nullptr_t) {
  raw_ = 0;
  return *this;
}

inline BasedPointer::StorageType BasedPointer::getRawValue() const {
  return raw_;
}

inline BasedPointer PointerBase::pointerToBasedNonNull(void *ptr) const {
  assert(ptr && "Null pointer given to pointerToBasedNonNull");
  return BasedPointer{ptr};
}

#ifdef HERMESVM_COMPRESSED_POINTERS
inline BasedPointer::BasedPointer(void *heapAddr)
    : raw_(computeSegmentAndOffset(heapAddr)) {}

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
  return (SegmentInfo::segmentIndexFromStart(segStart)
          << AlignedStorage::kLogSize) |
      offset;
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

inline BasedPointer PointerBase::pointerToBased(void *ptr) const {
  if (!ptr) {
    // Null pointers are special and can't be "compressed" in the same
    // way as non-null pointers.  So we make a special case for null.
    return BasedPointer{};
  }
  return pointerToBasedNonNull(ptr);
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

#else
inline BasedPointer::BasedPointer(void *heapAddr)
    : raw_(reinterpret_cast<uintptr_t>(heapAddr)) {}

inline void *PointerBase::basedToPointer(BasedPointer ptr) const {
  return reinterpret_cast<void *>(ptr.getRawValue());
}

inline BasedPointer PointerBase::pointerToBased(void *ptr) const {
  return BasedPointer{ptr};
}
#endif // HERMESVM_COMPRESSED_POINTERS

/// @}

} // namespace vm
} // namespace hermes

#endif
