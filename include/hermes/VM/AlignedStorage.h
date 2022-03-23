/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ALIGNED_STORAGE_H
#define HERMES_VM_ALIGNED_STORAGE_H

#include "hermes/VM/AllocSource.h"
#include "llvh/Support/ErrorOr.h"
#include "llvh/Support/PointerLikeTypeTraits.h"

#include <cstddef>
#include <cstdint>

namespace hermes {
namespace vm {

class StorageProvider;

#ifndef HERMESVM_HEAP_SEGMENT_SIZE_KB
#error Heap segment size must be defined.
#endif

/// A memory allocation, whose size and alignment is constant and guaranteed to
/// be \c AlignedStorage::size().
struct AlignedStorage {
  /// The size and the alignment of the storage, in bytes.
  static constexpr unsigned kLogSize{
      llvh::detail::ConstantLog2<HERMESVM_HEAP_SEGMENT_SIZE_KB * 1024>::value};
  static constexpr size_t kSize{1 << kLogSize};
  static_assert(
      kSize == HERMESVM_HEAP_SEGMENT_SIZE_KB * 1024,
      "Heap segment size must be a power of 2.");

  /// Returns the pointer to the beginning of the storage containing \p ptr
  /// (inclusive). Assuming such a storage exists. Note that
  ///
  ///     AlignedStorage::start(storage.hiLim()) != storage.lowLim()
  ///
  /// as \c storage.hiLim() is not contained in the bounds of \c storage -- it
  /// is the first address not in the bounds.
  inline static void *start(const void *ptr);

  /// Returns the pointer to the end of the storage containing \p ptr
  /// (exclusive). Assuming such a storage exists. Note that
  ///
  ///     AlignedStorage::end(storage.hiLim()) != storage.hiLim()
  ///
  /// as \c storage.hiLim() is not contained in the bounds of \c storage -- it
  /// is the first address not in the bounds.
  inline static const void *end(const void *ptr);

  /// Returns whether \p a and \p b are contained in the same storage.
  inline static bool containedInSame(const void *a, const void *b);

  /// Returns the offset in bytes to \p ptr from the start of its containing
  /// storage. Assuming such a storage exists. Note that
  ///
  ///     AlignedStorage::offset(storage.hiLim()) != storage.size()
  ///
  /// as \c storage.hiLim() is not contained in the bounds of \c storage -- it
  /// is the first address not in the bounds.
  inline static size_t offset(const char *ptr);

  /// Returns the size, in bytes, of an \c AlignedStorage.
  inline static constexpr size_t size();

  friend void swap(AlignedStorage &, AlignedStorage &);

  static llvh::ErrorOr<AlignedStorage> create(StorageProvider *provider);
  static llvh::ErrorOr<AlignedStorage> create(
      StorageProvider *provider,
      const char *name);

  /// Allocates a 'null' instance (one that does not own a memory region)
  AlignedStorage() = default;

  /// \c AlignedStorage is moveable and assignable, but non-copyable.
  AlignedStorage(AlignedStorage &&);
  AlignedStorage &operator=(AlignedStorage);
  AlignedStorage(const AlignedStorage &) = delete;

  ~AlignedStorage();

  /// \return true if and only if this aligned storage owns memory.
  explicit inline operator bool() const;

  /// Tell the OS that this region of memory is no longer actively in use.
  ///
  /// \param from The beginning of the memory region (inclusive).
  /// \param to The end of the memory region (exclusive).
  ///
  /// \pre \p from is less than or equal to \p to.
  /// \pre \p from and \p to are page-aligned.
  /// \pre \p from and \p to are within the bounds of the storage.
  ///
  /// \post The memory within the region no longer counts towards the process's
  ///     footprint, as measured by the OS.
  void markUnused(char *from, char *to);

  /// \return The address of the lowest byte in the storage. The low boundary of
  ///     the storage (inclusive).
  inline char *lowLim() const;

  /// \return The address of the first byte following the storage. The high
  ///     boundary of the storage (non-inclusive).
  inline char *hiLim() const;

  /// \return \c true if and only if \p ptr is within the memory range owned by
  ///     this \c AlignedStorage.
  inline bool contains(const void *ptr) const;

 private:
  /// Manages a region of memory.
  /// \p provider The allocator of this storage. It will be used to delete this
  ///   storage.
  /// \p name The name to give to this storage.
  AlignedStorage(StorageProvider *provider, void *lowLim);

  /// Mask for isolating the offset into a storage for a pointer.
  static constexpr size_t kLowMask{kSize - 1};

  /// Mask for isolating the storage being pointed into by a pointer.
  static constexpr size_t kHighMask{~kLowMask};

  /// The start of the aligned storage.
  char *lowLim_{nullptr};

  /// The provider that created this storage. It will be used to properly
  /// destroy this.
  StorageProvider *provider_{nullptr};
};

/* static */ void *AlignedStorage::start(const void *ptr) {
  return reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(ptr) & kHighMask);
}

/* static */ const void *AlignedStorage::end(const void *ptr) {
  return reinterpret_cast<char *>(start(ptr)) + kSize;
}

/* static */ bool AlignedStorage::containedInSame(
    const void *a,
    const void *b) {
  return (reinterpret_cast<uintptr_t>(a) ^ reinterpret_cast<uintptr_t>(b)) <
      kSize;
}

/* static */ size_t AlignedStorage::offset(const char *ptr) {
  return reinterpret_cast<size_t>(ptr) & kLowMask;
}

/* static */ constexpr size_t AlignedStorage::size() {
  return kSize;
}

AlignedStorage::operator bool() const {
  return lowLim_;
}

char *AlignedStorage::lowLim() const {
  return lowLim_;
}

char *AlignedStorage::hiLim() const {
  return lowLim_ + kSize;
}

bool AlignedStorage::contains(const void *ptr) const {
  return AlignedStorage::start(ptr) == lowLim();
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_ALIGNED_STORAGE_H
