/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef _WINDOWS

#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"

#include <cassert>
#include <vector>

#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>

#if defined(__linux__)
#if !defined(RUSAGE_THREAD)
#define RUSAGE_THREAD 1
#endif
#endif // __linux__

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
static size_t testPgSz = 0;

void set_test_page_size(size_t pageSz) {
  testPgSz = pageSz;
}

void reset_test_page_size() {
  testPgSz = 0;
}
#endif

static inline size_t page_size_real() {
  return getpagesize();
}

size_t page_size() {
#ifndef NDEBUG
  if (testPgSz != 0) {
    return testPgSz;
  }
#endif
  return page_size_real();
}

#ifndef NDEBUG
static constexpr size_t unsetVMAllocLimit = std::numeric_limits<size_t>::max();
static size_t totalVMAllocLimit = unsetVMAllocLimit;

void set_test_vm_allocate_limit(size_t totSz) {
  totalVMAllocLimit = totSz;
}

void unset_test_vm_allocate_limit() {
  totalVMAllocLimit = unsetVMAllocLimit;
}
#endif // !NDEBUG

static llvm::ErrorOr<void *> vm_allocate_impl(size_t sz) {
#ifndef NDEBUG
  if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
    return make_error_code(OOMError::TestVMLimitReached);
  } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
    totalVMAllocLimit -= sz;
  }
#endif // !NDEBUG

  void *result = mmap(
      nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (result == MAP_FAILED) {
    // Since mmap is a POSIX API, even on MacOS, errno should use the POSIX
    // generic_category.
    return std::error_code(errno, std::generic_category());
  }
  return result;
}

static char *alignAlloc(void *p, size_t alignment) {
  return reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(p), alignment));
}

llvm::ErrorOr<void *> vm_allocate(size_t sz) {
  assert(sz % page_size() == 0);
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
#endif // !NDEBUG
  return vm_allocate_impl(sz);
}

llvm::ErrorOr<void *> vm_allocate_aligned(size_t sz, size_t alignment) {
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);

  // Opportunistically allocate without alignment constraint,
  // and see if the memory happens to be aligned.
  // While this may be unlikely on the first allocation request,
  // subsequent allocation requests have a good chance.
  auto result = vm_allocate_impl(sz);
  if (!result) {
    return result;
  }
  void *mem = *result;
  if (mem == alignAlloc(mem, alignment)) {
    return mem;
  }

  // Free the oppotunistic allocation.
  oscompat::vm_free(mem, sz);

  // This time, allocate a larger section to ensure that it contains
  // a subsection that satisfies the request.
  // Use *real* page size here since that's what vm_allocate_impl guarantees.
  const size_t excessSize = sz + alignment - page_size_real();
  result = vm_allocate_impl(excessSize);
  if (!result)
    return result;

  void *raw = *result;
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

void vm_free_aligned(void *p, size_t sz) {
  vm_free(p, sz);
}

void vm_unused(void *p, size_t sz) {
#ifndef NDEBUG
  const size_t PS = page_size();
  assert(
      reinterpret_cast<intptr_t>(p) % PS == 0 &&
      "Precondition: pointer is page-aligned.");
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

bool vm_protect(void *p, size_t sz, ProtectMode mode) {
  auto prot = PROT_NONE;
  if (mode == ProtectMode::ReadWrite) {
    prot = PROT_WRITE | PROT_READ;
  }
  int err = mprotect(p, sz, prot);
  return err != -1;
}

bool vm_madvise(void *p, size_t sz, MAdvice advice) {
#ifndef NDEBUG
  const size_t PS = page_size();
  assert(
      reinterpret_cast<intptr_t>(p) % PS == 0 &&
      "Precondition: pointer is page-aligned.");
#endif

  int param = MADV_NORMAL;
  switch (advice) {
    case MAdvice::Random:
      param = MADV_RANDOM;
      break;
    case MAdvice::Sequential:
      param = MADV_SEQUENTIAL;
      break;
  }
  return madvise(p, sz, param) == 0;
}

int pages_in_ram(const void *p, size_t sz, llvm::SmallVectorImpl<int> *runs) {
  const auto PS = page_size();
  {
    // Align region start down to page boundary.
    const uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    const size_t adjust = addr % PS;
    p = reinterpret_cast<const void *>(addr - adjust);
    sz += adjust;
  }
  // Total number of pages that the region overlaps.
  const size_t mapSize = (sz + PS - 1) / PS;
#ifdef __linux__
  using MapElm = unsigned char;
#else
  using MapElm = char;
#endif
  std::vector<MapElm> bitMap(mapSize);
  if (mincore(const_cast<void *>(p), sz, bitMap.data())) {
    return -1;
  }
  // Total pages in RAM.
  int totalIn = 0;
  bool currentRunStatus = true;
  if (runs)
    runs->push_back(0);
  for (auto elm : bitMap) {
    // Lowest bit tells whether in RAM.
    bool thisStatus = (elm & 1);
    totalIn += thisStatus;
    if (runs) {
      if (thisStatus != currentRunStatus)
        runs->push_back(0);
      currentRunStatus = thisStatus;
      ++runs->back();
    }
  }
  return totalIn;
}

uint64_t peak_rss() {
  rusage ru;
  if (getrusage(RUSAGE_SELF, &ru)) {
    // failed
    return 0;
  }
  uint64_t rss = ru.ru_maxrss;
#if !defined(__APPLE__) || !defined(__MACH__)
  // Linux maxrss is in kilobytes, expand into bytes.
  rss *= 1024;
#endif
  return rss;
}

uint64_t current_rss() {
#if defined(__APPLE__) && defined(__MACH__)
  struct mach_task_basic_info info;
  mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(
          mach_task_self(),
          MACH_TASK_BASIC_INFO,
          (task_info_t)&info,
          &infoCount) != KERN_SUCCESS)
    return 0;
  return info.resident_size * page_size_real();
#else
  FILE *fp = fopen("/proc/self/statm", "r");
  if (!fp) {
    return 0;
  }
  long rss = 0;
  // The first field is total program size, second field is resident set size.
  if (fscanf(fp, "%*ld %ld", &rss) != 1) {
    fclose(fp);
    return 0;
  }
  fclose(fp);
  // The RSS number from from statm is in number of pages. Multiply by the real
  // page size to get the number in bytes.
  return rss * page_size_real();
#endif
}

uint64_t current_private_dirty() {
#if defined(__linux__)
  uint64_t sum = 0;
  FILE *fp = fopen("/proc/self/smaps", "r");
  static const char kPrefix[] = "Private_Dirty:";
  constexpr size_t kPrefixLen = sizeof(kPrefix) - 1;
  char buf[128]; // Just needs to fit the lines we care about.
  while (fgets(buf, sizeof(buf), fp))
    if (strncmp(buf, kPrefix, kPrefixLen) == 0)
      sum += atoll(buf + kPrefixLen);
  fclose(fp);
  return sum * 1024;
#else
  return 0;
#endif
}

bool num_context_switches(long &voluntary, long &involuntary) {
  voluntary = involuntary = -1;
  rusage ru;
  // Only Linux is known to have RUSAGE_THREAD.
#if defined(__linux__)
  const int who = RUSAGE_THREAD;
#else
  const int who = RUSAGE_SELF;
#endif
  if (getrusage(who, &ru)) {
    // failed
    return false;
  }
  voluntary = ru.ru_nvcsw;
  involuntary = ru.ru_nivcsw;
  return true;
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

// Platform-specific implementations of thread_page_fault_count

#if defined(__APPLE__) && defined(__MACH__)

bool thread_page_fault_count(int64_t *outMinorFaults, int64_t *outMajorFaults) {
  task_events_info eventsInfo;
  mach_msg_type_number_t count = TASK_EVENTS_INFO_COUNT;
  kern_return_t kr = task_info(
      mach_task_self(), TASK_EVENTS_INFO, (task_info_t)&eventsInfo, &count);
  if (kr == KERN_SUCCESS) {
    *outMinorFaults = eventsInfo.faults;
    *outMajorFaults = eventsInfo.pageins;
  }
  return kr == KERN_SUCCESS;
}

#elif defined(__linux__) // !(__APPLE__ && __MACH__)

bool thread_page_fault_count(int64_t *outMinorFaults, int64_t *outMajorFaults) {
  struct rusage stats = {};
  int ret = getrusage(RUSAGE_THREAD, &stats);
  if (ret == 0) {
    *outMinorFaults = stats.ru_minflt;
    *outMajorFaults = stats.ru_majflt;
  }
  return ret == 0;
}

#else // !(__APPLE__ && __MACH__), !__linux__
#error "Thread page fault count not supported on this platform"
#endif // thread_page_fault_count: (__APPLE__ && __MACH__), __linux__

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

bool set_env(const char *name, const char *value) {
  // Enforce the contract of this function that value must not be empty
  assert(*value != '\0' && "value cannot be empty string");
  return setenv(name, value, 1) == 0;
}

bool unset_env(const char *name) {
  return unsetenv(name) == 0;
}

SigAltStackDeleter::SigAltStackDeleter() {
#ifndef __APPLE__
  stack_t oldAltStack;
  if (sigaltstack(nullptr, &oldAltStack) == 0) {
    origStack_ = oldAltStack.ss_sp;
  }
#endif
}

SigAltStackDeleter::~SigAltStackDeleter() {
#ifndef __APPLE__
  stack_t oldAltStack;
  if (sigaltstack(nullptr, &oldAltStack) == 0 && oldAltStack.ss_sp != nullptr &&
      oldAltStack.ss_sp == origStack_) {
    free(oldAltStack.ss_sp);
  }
#endif
}

} // namespace oscompat
} // namespace hermes

#endif // not _WINDOWS
