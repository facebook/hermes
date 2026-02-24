/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace hermes {

/// An opaque utility class that enables executing a function in fresh stack.
class StackExecutor;

/// Allocate a new executor.
///
/// \param stackSize the stack size that the executor will provide. 0 means
///   default.
/// \param timeout if possible, depending on the implementation of the executor,
///   free the resources after the specified inactivity timeout. The resources
///   will be allocated again when needed.
std::shared_ptr<StackExecutor> newStackExecutor(
    size_t stackSize = 0,
    std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

/// Execute the specified function in the provided stack executor.
/// Note that this is not thread safe.
void executeInStack(StackExecutor &exec, void *arg, void (*func)(void *arg));

/// Execute the specified lambda in the provided stack executor.
template <typename F>
void executeInStack(StackExecutor &exec, const F &f) {
  executeInStack(
      exec, const_cast<void *>(static_cast<const void *>(&f)), [](void *arg) {
        (*static_cast<const F *>(arg))();
      });
}

} // namespace hermes
