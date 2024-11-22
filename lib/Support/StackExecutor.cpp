/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/StackExecutor.h"

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
