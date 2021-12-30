/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_BACKING_STORAGE_H
#define HERMES_VM_BACKING_STORAGE_H

#include "hermes/Public/GCConfig.h"
#include "hermes/VM/AllocSource.h"

#include <tuple>

namespace hermes {
namespace vm {
namespace detail {

/// An abstraction over the region of memory the garbage collector uses to store
/// the heap.
struct BackingStorage {
  friend void swap(BackingStorage &, BackingStorage &);

  /// (1) Allocate a region of memory.  May allocate a region smaller than the
  /// request.
  ///
  /// \param sz The desired size of the region to allocate, in bytes.
  /// \param minSz The minimum size that it is permissible to allocate, if the
  ///     desired size is not available.
  /// \param src Where to allocate the memory from.
  BackingStorage(
      gcheapsize_t sz,
      gcheapsize_t minSz,
      AllocSource src = AllocSource::VMAllocate);

  /// (2) Variant of the above in which \p minSz == \p sz; the requested
  /// \p sz must be allocated, or allocation fails.
  BackingStorage(gcheapsize_t sz, AllocSource src = AllocSource::VMAllocate);

  /// (3) Variant of (1) where the source is \p AllocSource::VMAllocate and the
  /// resulting region is annotated with \p name.
  BackingStorage(gcheapsize_t sz, gcheapsize_t minSz, const char *name);

  /// (4) Variant of (2) where the source is \p AllocSource::VMAllocate and the
  /// resulting region is annotated with \p name.
  BackingStorage(gcheapsize_t sz, const char *name);

  BackingStorage() = default;
  BackingStorage(const BackingStorage &) = delete;
  BackingStorage(BackingStorage &&);

  BackingStorage &operator=(BackingStorage);

  ~BackingStorage();

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

  /// Returns pointer to the beginning of the memory region.
  inline char *lowLim() const {
    return lowLim_;
  }

  /// Returns pointer one past the end of the memory region.
  inline char *hiLim() const {
    return hiLim_;
  }

  inline gcheapsize_t size() const {
    return hiLim_ - lowLim_;
  }

  /// Returns true if and only if \p ptr lies within the memory region.
  bool contains(void *ptr) const;

 private:
  BackingStorage(
      const std::tuple<char *, void *, size_t> &alloc,
      AllocSource src);

  char *lowLim_;
  char *hiLim_;
  /// The original pointer returned by the allocator.
  /// This is used for the malloc backing store in case it does not give back a
  /// page-aligned pointer.
  void *origPointer_;
  AllocSource src_;
};

} // namespace detail
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_BACKING_STORAGE_H
