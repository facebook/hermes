/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_CHECKEDMALLOC_H
#define HERMES_SUPPORT_CHECKEDMALLOC_H

#include "hermes/Support/ErrorHandling.h"

#include "llvh/Support/Compiler.h"

#include <cstdlib>

namespace hermes {

/// Like malloc, but calls hermes_fatal on a null return.
LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS void *
checkedMalloc(size_t size);

/// Like checkedMalloc, but computes the total size in an overflow safe way.
/// This is declared inline so that constants can be propagated into the 'size'
/// parameter, optimizing the division.
LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS inline void *
checkedMalloc2(size_t count, size_t size) {
  size_t totalSize = count * size;
  if (LLVM_UNLIKELY(size > 0 && totalSize / size != count)) {
    hermes_fatal("malloc failure");
  }
  return checkedMalloc(totalSize);
}

/// Like checkedMalloc2, but zero-initializes the allocation.
LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS void *
checkedCalloc(size_t count, size_t size);

} // namespace hermes

#endif // HERMES_SUPPORT_CHECKEDMALLOC_H
