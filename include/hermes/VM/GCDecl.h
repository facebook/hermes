/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCDECL_H
#define HERMES_VM_GCDECL_H

#include <atomic>
#include <mutex>

#ifdef HERMESVM_GC_HADES
#include <thread>
#endif

namespace hermes {
namespace vm {

#ifdef HERMESVM_GC_HADES
#ifndef NDEBUG

/// A DebugMutex wraps a std::mutex and also tracks which thread currently has
/// the mutex locked. Only available in debug modes.
class DebugMutex {
 public:
  DebugMutex() : tid_() {}
  ~DebugMutex() = default;

  operator bool() const {
    // Check that this thread owns the mutex.
    // The mutex must be held in order to check this condition safely.
    return tid_ == std::this_thread::get_id();
  }

  void lock() {
    inner_.lock();
    tid_ = std::this_thread::get_id();
  }

  void unlock() {
    tid_ = std::thread::id{};
    inner_.unlock();
  }

  /// Allow bypassing the guards for special cases, like
  /// std::condition_variable.
  std::mutex &inner() {
    return inner_;
  }

  /// If inner() is used to handle locking, use assignThread to get the
  /// DebugMutex back in a consistent state.
  void assignThread(std::thread::id tid) {
    tid_ = tid;
  }

 private:
  std::mutex inner_;
  std::thread::id tid_;
};

using Mutex = DebugMutex;
#else
using Mutex = std::mutex;
#endif

using WeakRefMutex = Mutex;
using WeakRefLock = std::lock_guard<Mutex>;

/// HadesGC requires some fields to be atomic, such as array lengths.
template <typename T>
using AtomicIfConcurrentGC = std::atomic<T>;
#else
/// Non-concurrent GCs don't need any locks in order to have correct
/// semantics for WeakRef, so use a cheap type to pass around instead.

/// A FakeMutex has the same API as a std::mutex but does nothing.
/// It pretends to always be locked for convenience of asserts that need to work
/// in both concurrent code and non-concurrent code.
class FakeMutex {
 public:
  explicit FakeMutex() = default;

  operator bool() const {
    return true;
  }

  void lock() {}
  bool try_lock() {
    return true;
  }
  void unlock() {}
};

using Mutex = FakeMutex;
using WeakRefMutex = FakeMutex;
using WeakRefLock = std::lock_guard<WeakRefMutex>;

/// FakeAtomic has the same API as std::atomic, but ignores the memory order
/// argument and always accesses data non-atomically.
/// Used when the GC doesn't require atomicity.
/// In the JS VM, there is currently only one mutator thread and at most one GC
/// thread. The GC thread will not do any modifications to these atomics, and
/// will only read them. Therefore it is typically safe for the mutator to use
/// relaxed reads. Writes will typically require std::memory_order_release or
/// stricter to make sure the GC sees the writes which occur before the atomic
/// write.
/// NOTE: This differs from std::atomic where it doesn't have default memory
/// orders, since we want all atomic operations to be very explicit with their
/// requirements. Also don't define operator T for the same reason.
template <typename T>
class FakeAtomic final {
 public:
  constexpr explicit FakeAtomic() : data_{} {}
  constexpr explicit FakeAtomic(T desired) : data_{desired} {}

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

/// No need for atomics with non-Hades GCs.
template <typename T>
using AtomicIfConcurrentGC = FakeAtomic<T>;
#endif

#if defined(HERMESVM_GC_MALLOC)
class MallocGC;
using GC = MallocGC;
#elif defined(HERMESVM_GC_NONCONTIG_GENERATIONAL)
class GenGC;
using GC = GenGC;
#elif defined(HERMESVM_GC_HADES)
class HadesGC;
using GC = HadesGC;
#else
#error "Unsupported HermesVM GCKIND" #HERMESVM_GCKIND
#endif

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCDECL_H
