/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/OSCompat.h"

#include "gtest/gtest.h"

#include <thread>

using namespace hermes;

namespace {

/// Upper bound on the stack, nullptr if currently unknown.
const void *nativeStackHigh{nullptr};
/// This has already taken \c nativeStackGap into account,
/// so any stack outside [nativeStackHigh-nativeStackSize, nativeStackHigh]
/// is overflowing.
size_t nativeStackSize{0};
/// Native stack remaining before assuming overflow.
unsigned nativeStackGap = 16 * 1024;

/// Slow path for \c isOverflowing.
/// Sets \c stackLow_ \c stackHigh_.
/// \return true if the native stack is overflowing the bounds of the
///   current thread.
bool isStackOverflowingSlowPath() {
  auto [highPtr, size] = oscompat::thread_stack_bounds(nativeStackGap);
  nativeStackHigh = (const char *)highPtr;
  nativeStackSize = size;
#ifdef __GNUC__
  void *sp = __builtin_frame_address(0);
#else
  volatile char *var = 0;
  void *sp = (void *)&var;
#endif
  return (uintptr_t)nativeStackHigh - (uintptr_t)sp > nativeStackSize;
}

/// \return true if the native stack is overflowing the bounds of the
///   current thread. Updates the stack bounds if the thread which Runtime
///   is executing on changes.
inline bool isOverflowing() {
#ifdef __GNUC__
  void *sp = __builtin_frame_address(0);
#else
  volatile char *var = 0;
  void *sp = (void *)&var;
#endif
  // Check for overflow by subtracting the sp from the high pointer.
  // If the sp is outside the valid stack range, the difference will
  // be greater than the known stack size.
  // This is clearly true when 0 < sp < nativeStackHigh_ - size.
  // If nativeStackHigh_ < sp, then the subtraction will wrap around.
  // We know that nativeStackSize_ <= nativeStackHigh_
  // (because otherwise the stack wouldn't fit in the memory),
  // so the overflowed difference will be greater than nativeStackSize_.
  if (!((uintptr_t)nativeStackHigh - (uintptr_t)sp > nativeStackSize)) {
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

/// Helper variable for recursiveCall().
volatile unsigned recCount = 0;
volatile char *volatile dataPtr;

/// A recursive call that reliably stops when it is about to overflow the stack.
/// Prevent inlining to avoid allocating multiple frames at once.
LLVM_ATTRIBUTE_NOINLINE unsigned recursiveCall() {
  volatile char data[2000];
  dataPtr = data;

  ++recCount;
  if (isOverflowing()) {
    printf("Stack overflow, count=%u\n", recCount);
    return recCount;
  }

  // Prevent a tall call.
  return recursiveCall() + (--recCount);
}

void unboundedRecursion() {
  if (!oscompat::thread_stack_bounds(0).first) // Unsupported on this platform
    return;
  recursiveCall();
}

volatile unsigned sum = 0;

/// Ensure that we can read from the entire extent of the reported stack
void manualStackScan() {
  auto [high, size] = oscompat::thread_stack_bounds(0);
  if (!high) // Unsupported on this platform
    return;
  const char *low = (const char *)high - size;
  for (const char *p = (const char *)high - 16; p > low; p -= 2048) {
    volatile char x = *((volatile const char *)p);
    sum += x;
  }
}

TEST(StackBoundsTest, manualStackScan_mainThread) {
  manualStackScan();
}
TEST(StackBoundsTest, unboundRecursion_mainThread) {
  unboundedRecursion();
}
TEST(StackBoundsTest, manualStackScan_thread) {
  std::thread t(manualStackScan);
  t.join();
}
TEST(StackBoundsTest, unboundRecursion_thread) {
  std::thread t(unboundedRecursion);
  t.join();
}

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
// Windows/EMSCRIPTEN don't support pthreads well.

void runInThreadWith64KGuard(void *(threadFunc)(void *)) {
  pthread_t thread;
  pthread_attr_t attr;
  size_t guard_size = 64 * 1024; // 64KB

  // Initialize thread attributes object
  pthread_attr_init(&attr);

  // Set the guard size attribute
  if (pthread_attr_setguardsize(&attr, guard_size) != 0) {
    perror("Failed to set guard size");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&thread, &attr, threadFunc, nullptr) != 0) {
    perror("Failed to create thread");
    exit(EXIT_FAILURE);
  }

  // Clean up
  pthread_attr_destroy(&attr);
  pthread_join(thread, nullptr);
}

TEST(StackBoundsTest, manualStackScan_thread64KGuard) {
  runInThreadWith64KGuard([](void *) -> void * {
    manualStackScan();
    return nullptr;
  });
}
TEST(StackBoundsTest, unboundRecursion_thread64KGuard) {
  runInThreadWith64KGuard([](void *) -> void * {
    unboundedRecursion();
    return nullptr;
  });
}

TEST(StackBoundsTest, ThreadStackBounds) {
  auto [high, size] = oscompat::thread_stack_bounds();
  ASSERT_TRUE(size > 0);
#ifdef __GNUC__
  void *sp = __builtin_frame_address(0);
#else
  volatile char *var = 0;
  void *sp = (void *)&var;
#endif
  ASSERT_FALSE((uintptr_t)high - (uintptr_t)sp > size);
}

#endif

} // namespace
