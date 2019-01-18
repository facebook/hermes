/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/OSCompat.h"

#include <cassert>

#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __MACH__
#include <mach/mach.h>

#ifdef __APPLE__
#include <pthread.h>
#endif // __APPLE__

#endif // __MACH__

#ifdef __linux__

#if !defined(_POSIX_TIMERS) || _POSIX_TIMERS <= 0
#error "Timers not supported on this Android platform."
#endif

#ifndef CLOCK_THREAD_CPUTIME_ID
#error "CLOCK_THREAD_CPUTIME_ID not supported by clock_gettime"
#endif

#include <sys/syscall.h>
#include <time.h>

#endif // __linux__

#ifdef __ANDROID__
#include <sys/prctl.h>

#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif

#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif // __ANDROID__

#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace oscompat {

#ifndef NDEBUG
namespace {
size_t testPgSz = 0;
} // namespace

void set_test_page_size(size_t pageSz) {
  testPgSz = pageSz;
}

void reset_test_page_size() {
  testPgSz = 0;
}
#endif

size_t page_size() {
#ifndef NDEBUG
  if (testPgSz != 0) {
    return testPgSz;
  }
#endif
  return getpagesize();
}

#ifndef NDEBUG
namespace {
constexpr size_t unsetVMAllocLimit = std::numeric_limits<size_t>::max();
size_t totalVMAllocLimit = unsetVMAllocLimit;
} // namespace

void set_test_vm_allocate_limit(size_t totSz) {
  totalVMAllocLimit = totSz;
}

void unset_test_vm_allocate_limit() {
  totalVMAllocLimit = unsetVMAllocLimit;
}
#endif // !NDEBUG

namespace {
void *vm_allocate_impl(size_t sz) {
#ifndef NDEBUG
  if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
    return nullptr;
  } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
    totalVMAllocLimit -= sz;
  }
#endif // !NDEBUG

  void *result = mmap(
      nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return result == MAP_FAILED ? nullptr : result;
}

char *alignAlloc(void *p, size_t alignment) {
  return reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(p), alignment));
}
} // namespace

void *vm_allocate(size_t sz) {
  assert(sz % page_size() == 0);
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(getpagesize())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
#endif // !NDEBUG
  return vm_allocate_impl(sz);
}

void *vm_allocate_aligned(size_t sz, size_t alignment) {
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);

  // Use *real* page size here since that's what vm_allocate_impl guarantees.
  const size_t excessSize = sz + alignment - getpagesize();
  void *raw = vm_allocate_impl(excessSize);
  if (raw == nullptr)
    return raw;

  char *aligned = alignAlloc(raw, alignment);
  size_t excessAtFront = aligned - static_cast<char *>(raw);
  size_t excessAtBack = excessSize - excessAtFront - sz;

  if (excessAtFront)
    oscompat::vm_free(raw, excessAtFront);
  if (excessAtBack)
    oscompat::vm_free(aligned + sz, excessAtBack);

  return aligned;
}

void vm_free(void *p, size_t sz) {
  auto ret = munmap(p, sz);

  assert(!ret && "Failed to free memory region.");
  (void)ret;

#ifndef NDEBUG
  if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit) && p) {
    totalVMAllocLimit += sz;
  }
#endif
}

void vm_unused(void *p, size_t sz) {
#ifndef NDEBUG
  const size_t PS = page_size();
  assert(
      reinterpret_cast<intptr_t>(p) % PS == 0 &&
      "Precondition: pointer is page-aligned.");
  assert(sz % PS == 0 && "Precondition: size is page-aligned.");
#endif

/// Change the flag we pass to \p madvise based on the platform, so that we are
/// always acting to reduce memory pressure, as perceived by that platform.
#if defined(__MACH__)

/// On the mach kernel, \p MADV_FREE causes the OS to deduct this memory from
/// the process's physical footprint.
#define MADV_UNUSED MADV_FREE

#elif defined(__linux__)

/// On linux, telling the OS that we \p MADV_DONTNEED some pages will cause it
/// to immediately deduct their size from the process's resident set.
#define MADV_UNUSED MADV_DONTNEED

#else
#error "Don't know how to return memory to the OS on this platform."
#endif // __MACH__, __linux__

  madvise(p, sz, MADV_UNUSED);

#undef MADV_UNUSED
}

void vm_prefetch(void *p, size_t sz) {
  assert(
      reinterpret_cast<intptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");
  madvise(p, sz, MADV_WILLNEED);
}

void vm_name(void *p, size_t sz, const char *name) {
#ifdef __ANDROID__
  prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, p, sz, name);
#else
  (void)p;
  (void)sz;
  (void)name;
#endif // __ANDROID__
}

uint64_t peak_rss() {
  rusage ru;
  if (getrusage(RUSAGE_SELF, &ru)) {
    // failed
    return 0;
  }
  uint64_t rss = ru.ru_maxrss;
#if !defined(__APPLE__) || !defined(__MACH__)
  rss *= 1024;
#endif
  return rss;
}

// Platform-specific implementations of thread_id
#if defined(__APPLE__) && defined(__MACH__)

uint64_t thread_id() {
  uint64_t tid = 0;
  auto ret = pthread_threadid_np(nullptr, &tid);
  assert(ret == 0 && "pthread_threadid_np shouldn't fail for current thread");
  (void)ret;
  return tid;
}

#elif defined(__ANDROID__)

uint64_t thread_id() {
  return gettid();
}

#elif defined(__linux__)

uint64_t thread_id() {
  return syscall(__NR_gettid);
}

#else
#error "Thread ID not supported on this platform"
#endif

// Platform-specific implementations of thread_cpu_time
#if defined(__APPLE__) && defined(__MACH__)

std::chrono::microseconds thread_cpu_time() {
  using namespace std::chrono;

  struct thread_basic_info tbi;
  mach_port_t self = pthread_mach_thread_np(pthread_self());
  mach_msg_type_number_t fields = THREAD_BASIC_INFO_COUNT;

  if (thread_info(self, THREAD_BASIC_INFO, (thread_info_t)&tbi, &fields) !=
      KERN_SUCCESS) {
    return microseconds::max();
  }

  microseconds::rep total = 0;
  total += tbi.user_time.microseconds;
  total += tbi.user_time.seconds * 1000000;

  total += tbi.system_time.microseconds;
  total += tbi.system_time.seconds * 1000000;

  return microseconds(total);
}

#elif defined(__linux__) // !(__APPLE__ && __MACH__)

std::chrono::microseconds thread_cpu_time() {
  using namespace std::chrono;

  struct timespec ts;

  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0) {
    return microseconds::max();
  }

  microseconds::rep total = 0;
  total += ts.tv_nsec / 1000;
  total += ts.tv_sec * 1000000;
  return microseconds(total);
}

#else // !(__APPLE__ && __MACH__), !__linux__
#error "Thread CPU Time not supported on this platform"
#endif // thread_cpu_time: (__APPLE__ && __MACH__), __linux__

std::string thread_name() {
  constexpr int kMaxThreadNameSize = 100;
  int ret = 0;
  char threadName[kMaxThreadNameSize];
#ifdef __ANDROID__
  ret = prctl(PR_GET_NAME, threadName);
#else
  ret = pthread_getname_np(pthread_self(), threadName, sizeof(threadName));
#endif
  if (ret != 0) {
    // thread name error should be non-fatal, simply return empty thread name.
    perror("thread_name failed");
    return "";
  }
  return threadName;
}

} // namespace oscompat
} // namespace hermes
