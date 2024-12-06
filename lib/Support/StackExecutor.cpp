/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/StackExecutor.h"

#if defined(HERMES_USE_BOOST_CONTEXT) && HERMES_USE_BOOST_CONTEXT
#include "llvh/Support/Debug.h"
#include "llvh/Support/raw_ostream.h"

#include <boost/context/fiber.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>

#include <cassert>

#define DEBUG_TYPE "StackExecutor"

namespace ctx = boost::context;

namespace hermes {

class StackExecutor {
  /// The argument to be passed to the callee.
  void *arg_ = nullptr;
  /// The callee to be invoked by the fiber each time it is resumed. If it is
  /// nullptr when the fiber is resumed, the fiber terminates.
  void (*fn_)(void *) = nullptr;
  /// The fiber with its own stack.
  ctx::fiber fiber_;

 public:
  explicit StackExecutor(size_t stackSize = 0)
      : fiber_(
            std::allocator_arg,
            ctx::protected_fixedsize_stack(
                stackSize ? stackSize : ctx::stack_traits::default_size()),
            [this](ctx::fiber &&next) {
              // Loop until the callee becomes nullptr.
              while (fn_) {
                LLVM_DEBUG(llvh::dbgs() << "StackExecutor running function\n");
                (*fn_)(arg_);
                LLVM_DEBUG(llvh::dbgs() << "StackExecutor function done\n");
                next = std::move(next).resume();
              }
              LLVM_DEBUG(llvh::dbgs() << "Ending StackExecutor fiber\n");
              // The std::move() here is required by older compilers.
              return std::move(next);
            }) {}

  /// Execute the specified function in the fiber.
  void exec(void *arg, void (*fn)(void *arg)) {
    assert(fn_ == nullptr && "StackExecutor recursive call??");
    arg_ = arg;
    fn_ = fn;
    fiber_ = std::move(fiber_).resume();
    arg_ = nullptr;
    fn_ = nullptr;
  }

  ~StackExecutor() {
    LLVM_DEBUG(llvh::dbgs() << "~StackExecutor");
    assert(fn_ == nullptr && "StackExecutor recursive call??");
    // Tell the fiber to terminate.
    fiber_ = std::move(fiber_).resume();
  }
};

std::shared_ptr<StackExecutor> newStackExecutor(
    size_t stackSize,
    std::chrono::milliseconds /*timeout*/) {
  return std::make_shared<StackExecutor>(stackSize);
}

void executeInStack(StackExecutor &exec, void *arg, void (*func)(void *arg)) {
  exec.exec(arg, func);
}

} // namespace hermes

#else

#include "hermes/Support/SerialExecutor.h"

#include <future>

namespace hermes {

class StackExecutor {
 public:
  SerialExecutor executor;

  explicit StackExecutor(size_t stackSize, std::chrono::milliseconds timeout)
      : executor(stackSize, timeout) {}
};

std::shared_ptr<StackExecutor> newStackExecutor(
    size_t stackSize,
    std::chrono::milliseconds timeout) {
  return std::make_shared<StackExecutor>(stackSize, timeout);
}

void executeInStack(StackExecutor &exec, void *arg, void (*func)(void *arg)) {
  std::packaged_task<void()> task([arg, func]() { func(arg); });
  auto future = task.get_future();
  exec.executor.add([&task]() { task(); });
  future.wait();
}

} // namespace hermes
#endif
