/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCDECL_H
#define HERMES_VM_GCDECL_H

#ifdef HERMESVM_GC_HADES
#include <mutex>
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
  operator bool() const {
    // Check that this thread owns the mutex.
    // The mutex must be held in order to check this condition safely.
    return tid_ == std::this_thread::get_id();
  }

  void lock() {
    inner_.lock();
    tid_ = std::this_thread::get_id();
  }

  bool try_lock() {
    if (inner_.try_lock()) {
      tid_ = std::this_thread::get_id();
      return true;
    } else {
      return false;
    }
  }

  void unlock() {
    tid_ = std::thread::id{};
    inner_.unlock();
  }

 private:
  std::mutex inner_;
  std::thread::id tid_;
};

using WeakRefMutex = DebugMutex;
using WeakRefLock = std::lock_guard<DebugMutex>;
#else
using WeakRefMutex = std::mutex;
using WeakRefLock = std::lock_guard<std::mutex>;
#endif
#else
/// Non-concurrent GCs don't need any locks in order to have correct
/// semantics for WeakRef, so use a cheap type to pass around instead.

/// FakeLockGuard has the same API as lock_guard but does nothing. It exists
/// only to prevent unused variable warnings.
template <typename Mutex>
class FakeLockGuard {
 public:
  explicit FakeLockGuard(Mutex &m) {
    (void)m;
  }
  // This is not "default" here to ensure that the destructor is non-trivial.
  ~FakeLockGuard() {}
};

using WeakRefMutex = bool;
using WeakRefLock = FakeLockGuard<WeakRefMutex>;

static_assert(
    !std::is_trivially_destructible<WeakRefLock>::value,
    "FakeLockGuard must not be trivially destructible to avoid warnings about "
    "unused variables");

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
