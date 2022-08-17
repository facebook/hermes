/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/CheckedMalloc.h"

#include "hermes/Support/ErrorHandling.h"

#include "llvh/Support/Compiler.h"

namespace hermes {

void *checkedMalloc(size_t sz) {
  void *res = ::malloc(sz);
  if (LLVM_UNLIKELY(!res)) {
    hermes_fatal("malloc failure");
  }
  return res;
}

void *checkedCalloc(size_t count, size_t size) {
  void *res = ::calloc(count, size);
  if (LLVM_UNLIKELY(!res)) {
    hermes_fatal("malloc failure");
  }
  return res;
}

} // namespace hermes
