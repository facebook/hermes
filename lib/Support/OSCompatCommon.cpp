/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"

namespace hermes {
namespace oscompat {

std::pair<const void *, size_t> thread_stack_bounds(unsigned gap) {
  // Avoid calling into the platform-specific code if we can cache the result
  // on a per-thread basis.

  // The cached result of thread_stack_bounds().
  static thread_local std::pair<const void *, size_t> cachedBounds{nullptr, 0};

  std::pair<const void *, size_t> bounds = cachedBounds;

  if (LLVM_UNLIKELY(!bounds.first)) {
    cachedBounds = bounds = detail::thread_stack_bounds_impl();
  }

  // Adjust for the gap here to allow caching with multiple gaps.
  auto [high, size] = bounds;
  return {high, gap < size ? size - gap : 0};
}

} // namespace oscompat
} // namespace hermes
