/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#pragma once

#include <mutex>

#include <jsi/decorator.h>
#include <jsi/jsi.h>

namespace facebook {
namespace jsi {

class ThreadSafeRuntime : public Runtime {
 public:
  virtual void lock() const = 0;
  virtual void unlock() const = 0;
  virtual Runtime& getUnsafeRuntime() = 0;
};

namespace detail {

// The actual implementation of a given ThreadSafeRuntime. It's parameterized
// by:
//
// - R: The actual Runtime type that this wraps
// - L: A lock type that has two members:
//   - void lock(const R&)
//   - void unlock(const R&)
template <typename R, typename L>
class ThreadSafeRuntimeImpl final
    : public WithRuntimeDecorator<std::lock_guard<L>, L, R, ThreadSafeRuntime> {
 public:
  template <typename... Args>
  ThreadSafeRuntimeImpl(Args&&... args)
      : WithRuntimeDecorator<std::lock_guard<L>, L, R, ThreadSafeRuntime>(
            unsafe_,
            lock_),
        unsafe_(std::forward<Args>(args)...),
        lock_(unsafe_) {}

  R& getUnsafeRuntime() override {
    return WithRuntimeDecorator<std::lock_guard<L>, L, R, ThreadSafeRuntime>::
        plain();
  }

  void lock() const override {
    lock_.lock();
  }

  void unlock() const override {
    lock_.unlock();
  }

 private:
  R unsafe_;
  mutable L lock_;
};

} // namespace detail

} // namespace jsi
} // namespace facebook
