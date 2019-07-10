/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_POINTERBASE_H
#define HERMES_VM_POINTERBASE_H

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

class BasedPointer final {
 public:
  explicit operator bool() const;

  bool operator==(BasedPointer other) const;

  explicit BasedPointer() : offset_(0) {}

 private:
  uint32_t offset_;

  explicit BasedPointer(uint32_t offset) : offset_(offset) {}

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
  /// Convert a pointer to an offset from this.
  /// \param ptr A pointer to convert.
  /// \pre ptr must start after this, and must end before 2 ^ 32 after this.
  inline BasedPointer pointerToBased(void *ptr) const;
  /// Same as above, but has a precondition that the pointer is not null.
  /// \pre ptr is not null.
  inline BasedPointer pointerToBasedNonNull(void *ptr) const;

  /// Convert an offset from this PointerBase into a real pointer that can be
  /// de-referenced.
  /// \param ptr An offset (based pointer) to convert.
  inline void *basedToPointer(BasedPointer ptr) const;
  /// Same as above, but has a precondition that the pointer is not null.
  /// \pre ptr is not null.
  inline void *basedToPointerNonNull(BasedPointer ptr) const;
};

/// @name Inline implementations
/// @{

inline BasedPointer::operator bool() const {
  return offset_ != 0;
}

inline bool BasedPointer::operator==(BasedPointer other) const {
  return offset_ == other.offset_;
}

inline void *PointerBase::basedToPointer(BasedPointer ptr) const {
  if (!ptr) {
    // Null pointers are special and can't be added to the base.
    return nullptr;
  }
  return basedToPointerNonNull(ptr);
}

inline void *PointerBase::basedToPointerNonNull(BasedPointer ptr) const {
  assert(ptr && "Null pointer given to basedToPointerNonNull");
  return reinterpret_cast<void *>(
      reinterpret_cast<uintptr_t>(this) + ptr.offset_);
}

inline BasedPointer PointerBase::pointerToBased(void *ptr) const {
  if (!ptr) {
    // Null pointers are special and can't be subtracted from the base.
    // NOTE: This is based on a choice to allow normal usage of nullptr
    // throughout the codebase. Instead, a special value of nullptr could be
    // used that is the same as the PointerBase address; however, that would be
    // very un-intuitive. This relies on PointerBase being strictly less than
    // the lowest address of the actual heap.
    return BasedPointer{};
  }
  return pointerToBasedNonNull(ptr);
}

inline BasedPointer PointerBase::pointerToBasedNonNull(void *ptr) const {
  assert(ptr && "Null pointer given to pointerToBasedNonNull");
  if (sizeof(void *) != sizeof(uint32_t)) {
    // Make sure that this is a well-formed offset in 64-bit builds.
    // The pointer must be strictly greater than the base in order to allow a
    // null check by comparing a based pointer to zero.
    // If a null pointer is desired, it will be explicitly handled above.
    assert(
        reinterpret_cast<uintptr_t>(ptr) > reinterpret_cast<uintptr_t>(this) &&
        "ptr should be strictly greater than the base pointer");
  }
// Use constexpr if in C++17.
#if LLVM_PTR_SIZE == 4
  // If this is a 32-bit build, then encode the pointer directly.
  uint32_t offset = reinterpret_cast<uint32_t>(ptr);
#else
  // Else, subtract a base pointer to make it 32 bits.
  uint32_t offset = static_cast<uint32_t>(
      reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(this));
#endif

  return BasedPointer{offset};
}

/// @}

} // namespace vm
} // namespace hermes

#endif
