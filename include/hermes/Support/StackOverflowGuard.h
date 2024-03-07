/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// OverflowGuard is designed to catch imminent native stack overflow. It does
/// this using two different heuristics, depending on HERMES_CHECK_NATIVE_STACK.
///
/// If HERMES_CHECK_NATIVE_STACK is defined, it will use real stack checking. It
/// calls into platform-specific APIs to obtain the upper stack bound of the
/// currently executing thread. It will then check the current stack address
/// against the upper limit, along with some user-defined gap. Overflow is
/// reported if the current stack address is close enough to the upper bound,
/// accounting for the supplied gap value.
///
/// If HERMES_CHECK_NATIVE_STACK is not defined, a simple depth counter is used.
/// Every time a recursive call is made, the counter should be bumped. Overflow
/// is reported if the counter reaches some user-defined max.
///
//===----------------------------------------------------------------------===//

#ifndef HERMES_SUPPORT_STACKOVERFLOWGUARD_H
#define HERMES_SUPPORT_STACKOVERFLOWGUARD_H

#include <cstddef>
#include "hermes/Support/OSCompat.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {

#ifdef HERMES_CHECK_NATIVE_STACK

class StackOverflowGuard {
  explicit StackOverflowGuard(unsigned stackGap) : nativeStackGap(stackGap) {}

 public:
  /// Upper bound on the stack, nullptr if currently unknown.
  const void *nativeStackHigh{nullptr};
  /// This has already taken \c nativeStackGap into account,
  /// so any stack outside [nativeStackHigh-nativeStackSize, nativeStackHigh]
  /// is overflowing.
  size_t nativeStackSize{0};
  /// Native stack remaining before assuming overflow.
  unsigned nativeStackGap;

  static StackOverflowGuard nativeStackGuard(unsigned stackGap) {
    return StackOverflowGuard(stackGap);
  }

  /// \return true if the native stack is overflowing the bounds of the
  ///   current thread. Updates the stack bounds if the thread which Runtime
  ///   is executing on changes.
  inline bool isOverflowing() {
    // Check for overflow by subtracting the sp from the high pointer.
    // If the sp is outside the valid stack range, the difference will
    // be greater than the known stack size.
    // This is clearly true when 0 < sp < nativeStackHigh_ - size.
    // If nativeStackHigh_ < sp, then the subtraction will wrap around.
    // We know that nativeStackSize_ <= nativeStackHigh_
    // (because otherwise the stack wouldn't fit in the memory),
    // so the overflowed difference will be greater than nativeStackSize_.
    if (LLVM_LIKELY(!(
            (uintptr_t)nativeStackHigh - (uintptr_t)__builtin_frame_address(0) >
            nativeStackSize))) {
      // Fast path: quickly check the stored stack bounds.
      // NOTE: It is possible to have a false negative here (highly unlikely).
      // If the program creates many threads and destroys them, a new
      // thread's stack could overlap the saved stack so we'd be checking
      // against the wrong bounds.
      return false;
    }
    // Slow path: might be overflowing, but update the stack bounds first
    // in case execution has changed threads.
    return isStackOverflowingSlowPath();
  }

  /// Clear the native stack bounds and force recomputation.
  inline void clearStackBounds() {
    nativeStackHigh = nullptr;
    nativeStackSize = 0;
  }

 private:
  /// Slow path for \c isOverflowing.
  /// Sets \c stackLow_ \c stackHigh_.
  /// \return true if the native stack is overflowing the bounds of the
  ///   current thread.
  bool isStackOverflowingSlowPath() {
    auto [highPtr, size] = oscompat::thread_stack_bounds(nativeStackGap);
    nativeStackHigh = (const char *)highPtr;
    nativeStackSize = size;
    return LLVM_UNLIKELY(
        (uintptr_t)nativeStackHigh - (uintptr_t)__builtin_frame_address(0) >
        nativeStackSize);
  }
};

#else

class StackOverflowGuard {
  explicit StackOverflowGuard(size_t max) : maxCallDepth(max) {}

 public:
  /// This is how many recursive calls have already been made.
  /// This grows towards maxCallDepth.
  size_t callDepth{0};
  /// If callDepth exceeds this value, it is considered overflow.
  size_t maxCallDepth;

  static StackOverflowGuard depthCounterGuard(unsigned stackGap) {
    return StackOverflowGuard(stackGap);
  }

  /// \return true if \c callDepth has exceeded the budget set by \c
  /// maxCallDepth.
  inline bool isOverflowing() {
    return callDepth > maxCallDepth;
  }
};

#endif

} // namespace hermes

#endif // HERMES_SUPPORT_STACKOVERFLOWGUARD_H
