/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/LimitedStorageProvider.h"

#include "hermes/VM/AlignedStorage.h"

namespace hermes {
namespace vm {

void *LimitedStorageProvider::newStorage(const char *name) {
  if (limit_ < AlignedStorage::size()) {
    return nullptr;
  }
  limit_ -= AlignedStorage::size();
  return delegate_->newStorage(name);
}

void LimitedStorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }
  delegate_->deleteStorage(storage);
  limit_ += AlignedStorage::size();
}

} // namespace vm
} // namespace hermes
