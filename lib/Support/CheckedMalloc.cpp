/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/CheckedMalloc.h"

#include "hermes/Support/ErrorHandling.h"

#include "llvm/Support/Compiler.h"

namespace hermes {

void *checkedMalloc(size_t sz) {
  void *res = ::malloc(sz);
  if (LLVM_UNLIKELY(!res)) {
    hermes_fatal("malloc failure");
  }
  return res;
}

} // namespace hermes
