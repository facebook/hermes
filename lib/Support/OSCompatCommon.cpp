/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"
#include "hermes/Support/SlowAssert.h"

#include "llvh/Support/raw_ostream.h"

#ifdef __linux__
#include <sys/resource.h>
#endif

namespace hermes {
namespace oscompat {

std::pair<const void *, size_t> thread_stack_bounds(unsigned gap) {
  // Avoid calling into the platform-specific code if we can cache the result
  // on a per-thread basis.

  // The cached result of thread_stack_bounds().
  static thread_local std::pair<const void *, size_t> cachedBounds{nullptr, 0};
  static thread_local uint64_t stackSize{0};

  std::pair<const void *, size_t> bounds = cachedBounds;

  if (LLVM_UNLIKELY(!bounds.first)) {
    cachedBounds = bounds = detail::thread_stack_bounds_impl();
    rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    stackSize = limit.rlim_cur;
  }

  // Check that the cached bounds don't change. This is useful for catching
  // potential bugs in thread_stack_bounds_impl.
  if (bounds != detail::thread_stack_bounds_impl()) {
    llvh::errs() << "thread_stack_bounds: " << bounds.first << " "
                 << bounds.second << "\n";
    llvh::errs() << "thread_stack_bounds_impl: "
                 << detail::thread_stack_bounds_impl().first << " "
                 << detail::thread_stack_bounds_impl().second << "\n";
    rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    auto newStackSize = limit.rlim_cur;
    llvh::errs() << "Cached stack size: " << stackSize << "\n";
    llvh::errs() << "Computed stack size: " << newStackSize << "\n";
  }
  HERMES_SLOW_ASSERT(bounds == detail::thread_stack_bounds_impl());

  // Adjust for the gap here to allow caching with multiple gaps.
  auto [high, size] = bounds;
  return {high, gap < size ? size - gap : 0};
}

} // namespace oscompat
} // namespace hermes
