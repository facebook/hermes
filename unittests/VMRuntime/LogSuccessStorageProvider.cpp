/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LogSuccessStorageProvider.h"

#include "llvm/Support/Compiler.h"

#include <cassert>

namespace hermes {
namespace vm {

LogSuccessStorageProvider::LogSuccessStorageProvider(
    std::unique_ptr<StorageProvider> delegate)
    : delegate_(std::move(delegate)) {}

llvm::ErrorOr<void *> LogSuccessStorageProvider::newStorage(const char *name) {
  auto res = delegate_->newStorage(name);

  if (LLVM_LIKELY(res)) {
    numAllocated_++;
  }

  return res;
}

void LogSuccessStorageProvider::deleteStorage(void *storage) {
  delegate_->deleteStorage(storage);
  if (!storage) {
    return;
  }

  numDeleted_++;
  assert(numAllocated_ >= numDeleted_);
}

size_t LogSuccessStorageProvider::numAllocated() const {
  return numAllocated_;
}

size_t LogSuccessStorageProvider::numDeleted() const {
  return numDeleted_;
}

size_t LogSuccessStorageProvider::numLive() const {
  return numAllocated_ - numDeleted_;
}

} // namespace vm
} // namespace hermes
