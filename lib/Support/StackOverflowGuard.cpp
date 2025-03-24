/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/StackOverflowGuard.h"
#include "hermes/Support/OSCompat.h"

namespace hermes {

#ifdef HERMES_CHECK_NATIVE_STACK

bool StackOverflowGuard::isStackOverflowingSlowPath() {
  auto [highPtr, size] = oscompat::thread_stack_bounds(nativeStackGap);
  nativeStackHigh = (const char *)highPtr;
  nativeStackSize = size;
  return LLVM_UNLIKELY(
      (uintptr_t)nativeStackHigh - (uintptr_t)__builtin_frame_address(0) >
      nativeStackSize);
}

#endif

} // namespace hermes
