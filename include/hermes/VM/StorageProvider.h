/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_STORAGEPROVIDER_H
#define HERMES_VM_STORAGEPROVIDER_H

#include <limits>
#include <memory>

namespace hermes {
namespace vm {

/// A StorageProvider creates and destroys memory space to be used for segments
/// by the GC.
class StorageProvider {
 public:
  StorageProvider() = default;
  virtual ~StorageProvider() = default;

  /// @name Factories
  /// @{

  /// Provide storage from a pre-allocated \p amount.
  /// \param excess An excess amount of bytes that should be allowed to be
  ///   allocated.
  /// \pre excess <= AlignedStorage::size().
  /// \post The returned StorageProvider will be able to allocate at most 1
  ///   extra storage for the excess amount specified.
  static std::unique_ptr<StorageProvider> preAllocatedProvider(
      size_t amount,
      size_t excess);

  /// Provide storage from mmap'ed separate regions.
  static std::unique_ptr<StorageProvider> mmapProvider();

  /// Provide storage via malloc.
  static std::unique_ptr<StorageProvider> mallocProvider();

  /// @}

  /// Create a new segment memory space.
  void *newStorage() {
    return newStorage(nullptr);
  }
  /// Create a new segment memory space and give this memory the name \p name.
  /// \return A pointer to a block of memory that has AlignedStorage::size()
  ///   bytes, and is aligned on AlignedStorage::size().
  virtual void *newStorage(const char *name) = 0;

  /// Delete the given segment's memory space, and make it available for re-use.
  /// \post Nothing in the range [storage, storage + AlignedStorage::size())
  ///   is valid memory to be read or written.
  virtual void deleteStorage(void *storage) = 0;
};

} // namespace vm
} // namespace hermes

#endif
