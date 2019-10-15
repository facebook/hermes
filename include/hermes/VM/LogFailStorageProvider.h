/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_LOGFAILSTORAGEPROVIDER_H
#define HERMES_VM_LOGFAILSTORAGEPROVIDER_H

#include "hermes/VM/StorageProvider.h"

#include <cstddef>
#include <memory>

namespace hermes {
namespace vm {

/// A LogFailStorageProvider is an adapter that keeps track of the number of
/// times allocation requests to its delegate have failed.  Note that this
/// adapter does not manage the lifetime of its delegate.
class LogFailStorageProvider final : public StorageProvider {
  StorageProvider *delegate_;
  size_t numFailedAllocs_{0};

 public:
  explicit LogFailStorageProvider(StorageProvider *provider)
      : delegate_(provider) {}

  inline size_t numFailedAllocs() const;

  using StorageProvider::newStorage;
  llvm::ErrorOr<void *> newStorage(const char *name) override;

  void deleteStorage(void *storage) override;
};

inline size_t LogFailStorageProvider::numFailedAllocs() const {
  return numFailedAllocs_;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_LOGFAILSTORAGEPROVIDER_H
