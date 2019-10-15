/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_LOGSUCCESSSTORAGEPROVIDER_H
#define HERMES_UNITTESTS_VMRUNTIME_LOGSUCCESSSTORAGEPROVIDER_H

#include "hermes/VM/StorageProvider.h"

#include <memory>

namespace hermes {
namespace vm {

/// Test StorageProvider adapter which keeps track of the counts of storages it
/// has allocated and deleted.
struct LogSuccessStorageProvider final : public StorageProvider {
  LogSuccessStorageProvider(std::unique_ptr<StorageProvider> delegate);

  llvm::ErrorOr<void *> newStorage(const char *name) override;
  void deleteStorage(void *storage) override;

  /// The number of storages this provider has allocated in its lifetime.
  size_t numAllocated() const;

  /// The number of storages this provider has deleted its lifetime.
  size_t numDeleted() const;

  /// The number of storages allocated by this provider that have not been
  /// deleted yet.
  size_t numLive() const;

 private:
  std::unique_ptr<StorageProvider> delegate_;

  size_t numAllocated_{0};
  size_t numDeleted_{0};
};

} // namespace vm
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_LOGSUCCESSSTORAGEPROVIDER_H
