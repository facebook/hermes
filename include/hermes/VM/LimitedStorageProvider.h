/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_LIMITEDSTORAGEPROVIDER_H
#define HERMES_VM_LIMITEDSTORAGEPROVIDER_H

#include "hermes/VM/StorageProvider.h"

#include <cstddef>
#include <memory>

namespace hermes {
namespace vm {

/// A LimitedStorageProvider is an adapter that imposes an extra limit on
/// requests for new storage.
class LimitedStorageProvider final : public StorageProvider {
  std::unique_ptr<StorageProvider> delegate_;
  size_t limit_;

 public:
  LimitedStorageProvider(
      std::unique_ptr<StorageProvider> &&provider,
      size_t limit)
      : delegate_(std::move(provider)), limit_(limit) {}

 protected:
  llvh::ErrorOr<void *> newStorageImpl(const char *name) override;

  void deleteStorageImpl(void *storage) override;
};

} // namespace vm
} // namespace hermes

#endif
