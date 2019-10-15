/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/LogFailStorageProvider.h"

#include "llvm/Support/Compiler.h"

namespace hermes {
namespace vm {

llvm::ErrorOr<void *> LogFailStorageProvider::newStorage(const char *name) {
  auto res = delegate_->newStorage(name);

  if (LLVM_UNLIKELY(!res)) {
    numFailedAllocs_++;
  }

  return res;
}

void LogFailStorageProvider::deleteStorage(void *storage) {
  delegate_->deleteStorage(storage);
}

} // namespace vm
} // namespace hermes
