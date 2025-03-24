/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/LimitedStorageProvider.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/AlignedHeapSegment.h"

namespace hermes {
namespace vm {

llvh::ErrorOr<void *> LimitedStorageProvider::newStorageImpl(
    size_t sz,
    const char *name) {
  if (limit_ < FixedSizeHeapSegment::storageSize()) {
    return make_error_code(OOMError::TestVMLimitReached);
  }
  limit_ -= sz;
  return delegate_->newStorage(sz, name);
}

void LimitedStorageProvider::deleteStorageImpl(void *storage, size_t sz) {
  if (!storage) {
    return;
  }
  delegate_->deleteStorage(storage, sz);
  limit_ += sz;
}

} // namespace vm
} // namespace hermes
