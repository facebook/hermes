/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_FAKETHREADS_H
#define HERMES_SUPPORT_FAKETHREADS_H

#include "llvh/Support/ErrorHandling.h"

#include <atomic>
#include <chrono>
#include <mutex>

namespace hermes {

/// FakeAtomic has the same API as std::atomic, but ignores the memory order
/// argument and always accesses data non-atomically.
/// Used when the GC doesn't require atomicity.
/// NOTE: This differs from std::atomic where it doesn't have default memory
/// orders, since we want all atomic operations to be very explicit with their
/// requirements. Also don't define operator T for the same reason.
template <typename T>
class FakeAtomic final {
 public:
  constexpr FakeAtomic() : data_{} {}
  constexpr FakeAtomic(T desired) : data_{desired} {}

  T load(std::memory_order order) const {
    (void)order;
    return data_;
  }

  void store(T desired, std::memory_order order) {
    (void)order;
    data_ = desired;
  }

  T fetch_add(T arg, std::memory_order order) {
    (void)order;
    const T oldData = data_;
    data_ += arg;
    return oldData;
  }

  T exchange(T arg, std::memory_order order) {
    (void)order;
    const T oldData = data_;
    data_ = arg;
    return oldData;
  }

  T fetch_sub(T arg, std::memory_order order) {
    (void)order;
    const T oldData = data_;
    data_ -= arg;
    return oldData;
  }

  /// Use store explicitly instead.
  FakeAtomic &operator=(const FakeAtomic &) = delete;

 private:
  T data_;
};

/// A FakeMutex has the same API as a std::mutex but does nothing.
/// It pretends to always be locked for convenience of asserts that need to work
/// in both concurrent code and non-concurrent code.
class FakeMutex {
 public:
  explicit FakeMutex() = default;

  operator bool() const {
    return true;
  }

  uint32_t depth() const {
    return 1;
  }

  void lock() {}
  bool try_lock() {
    return true;
  }
  void unlock() {}
};

} // namespace hermes

#if defined(__wasi__) && !LLVM_ENABLE_THREADS
// Provide fake replacement for std::thread under Wasi to make compilation
// possible.

namespace std {

using recursive_mutex = hermes::FakeMutex;
using mutex = hermes::FakeMutex;

template <class mutex_type>
class unique_lock {
  mutex_type *m_;

 public:
  explicit unique_lock(mutex_type &m) : m_(&m) {}
  unique_lock(mutex_type &m, std::adopt_lock_t) : m_(&m) {}

  mutex_type *release() noexcept {
    mutex_type *res = m_;
    m_ = nullptr;
    return res;
  }

  void lock() {}
  void unlock() {}
};

enum class cv_status { no_timeout, timeout };

class condition_variable {
 public:
  void notify_all() noexcept {}
  void notify_one() noexcept {}

  template <class Clock, class Duration>
  std::cv_status wait_until(
      std::unique_lock<std::mutex> &,
      const std::chrono::time_point<Clock, Duration> &) {
    return cv_status::no_timeout;
  }

  template <class Rep, class Period>
  std::cv_status wait_for(
      std::unique_lock<std::mutex> &,
      const std::chrono::duration<Rep, Period> &) {
    return cv_status::no_timeout;
  }

  void wait(std::unique_lock<std::mutex> &) {}

  template <class Predicate>
  void wait(std::unique_lock<std::mutex> &, Predicate pred) {
    if (!pred()) {
      llvh::report_fatal_error(
          "waiting in std::condition_variable not supported under Wasi");
    }
  }
};

class condition_variable_any {
 public:
  void notify_all() noexcept {}
  void notify_one() noexcept {}

  template <class Lock, class Clock, class Duration>
  std::cv_status wait_until(
      Lock &,
      const std::chrono::time_point<Clock, Duration> &) {
    return cv_status::no_timeout;
  }

  template <class Lock, class Rep, class Period>
  std::cv_status wait_for(Lock &, const std::chrono::duration<Rep, Period> &) {
    return cv_status::no_timeout;
  }

  template <class Lock, class Predicate>
  void wait(Lock &, Predicate pred) {
    if (!pred()) {
      llvh::report_fatal_error(
          "waiting in std::condition_variable not supported under Wasi");
    }
  }
};

template <class T>
class future {
 public:
  T get() {
    llvh::report_fatal_error("std::future not supported under Wasi");
  }
};

template <class R>
class promise {
 public:
  future<R> get_future() {
    return future<R>();
  }
  void set_value(const R &value) {
    llvh::report_fatal_error("std::promise not supported under Wasi");
  }
};

template <>
class promise<void> {
 public:
  future<void> get_future() {
    return future<void>();
  }
  void set_value() {}
};

class thread {
 public:
  class id {
   public:
    bool operator==(const id &other) const noexcept {
      return true;
    }
  };

  thread() noexcept {}

  template <class F, class... Args>
  explicit thread(F &&f, Args &&...args) {
    llvh::report_fatal_error("std::thread not supported under Wasi");
  }

  id get_id() const noexcept {
    return id();
  }

  void join() {}
  bool joinable() const noexcept {
    return false;
  }
};

namespace this_thread {
inline thread::id get_id() noexcept {
  return thread::id();
}
} // namespace this_thread

} // namespace std

#endif

#endif
