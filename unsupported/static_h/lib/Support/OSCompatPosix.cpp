/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)

#include "hermes/Support/Compiler.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"

#include <cassert>
#include <fstream>
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

#if defined(__linux__) || defined(__ANDROID__)
#include <sys/prctl.h>
#endif

#ifdef __ANDROID__
#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif

#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif // __ANDROID__

#include "llvh/Support/raw_ostream.h"

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

static llvh::ErrorOr<void *>
vm_mmap(void *addr, size_t sz, int prot, int flags, bool checkDebugLimit) {
  assert(sz % page_size_real() == 0);
#ifndef NDEBUG
  if (checkDebugLimit) {
    if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
      return make_error_code(OOMError::TestVMLimitReached);
    } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
      totalVMAllocLimit -= sz;
    }
  }
#endif // !NDEBUG
  void *result = mmap(addr, sz, prot, flags, -1, 0);
  if (result == MAP_FAILED) {
    // Since mmap is a POSIX API, even on MacOS, errno should use the POSIX
    // generic_category.
    return std::error_code(errno, std::generic_category());
  }
  return result;
}

static void vm_munmap(void *addr, size_t sz) {
  auto ret = munmap(addr, sz);
  assert(!ret && "Failed to free memory region.");
  (void)ret;

#ifndef NDEBUG
  if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit) && addr) {
    totalVMAllocLimit += sz;
  }
#endif
}

static char *alignAlloc(void *p, size_t alignment) {
  return reinterpret_cast<char *>(
      llvh::alignTo(reinterpret_cast<uintptr_t>(p), alignment));
}

static llvh::ErrorOr<void *>
vm_mmap_aligned(void *addr, size_t sz, size_t alignment, int prot, int flags) {
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);

  // Allocate a larger section to ensure that it contains  a subsection that
  // satisfies the request. Use *real* page size here since that's what vm_mmap
  // guarantees.
  const size_t excessSize = sz + alignment - page_size_real();
  auto result = vm_mmap(addr, excessSize, prot, flags, true);
  if (!result)
    return result;

  void *raw = *result;
  char *aligned = alignAlloc(raw, alignment);
  size_t excessAtFront = aligned - static_cast<char *>(raw);
  size_t excessAtBack = excessSize - excessAtFront - sz;

  if (excessAtFront)
    vm_munmap(raw, excessAtFront);
  if (excessAtBack)
    vm_munmap(aligned + sz, excessAtBack);

  return aligned;
}

static constexpr int kVMAllocateFlags = MAP_PRIVATE | MAP_ANONYMOUS;
static constexpr int kVMAllocateProt = PROT_READ | PROT_WRITE;

llvh::ErrorOr<void *> vm_allocate(size_t sz, void *hint) {
  assert(sz % page_size() == 0);
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
#endif // !NDEBUG
  return vm_mmap(hint, sz, kVMAllocateProt, kVMAllocateFlags, true);
}

llvh::ErrorOr<void *>
vm_allocate_aligned(size_t sz, size_t alignment, void *hint) {
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);

  // Opportunistically allocate without alignment constraint,
  // and see if the memory happens to be aligned.
  // While this may be unlikely on the first allocation request,
  // subsequent allocation requests have a good chance.
  auto result = vm_mmap(hint, sz, kVMAllocateProt, kVMAllocateFlags, true);
  if (!result) {
    return result;
  }
  void *mem = *result;
  if (mem == alignAlloc(mem, alignment)) {
    return mem;
  }

  // Free the opportunistic allocation.
  vm_munmap(mem, sz);

  return vm_mmap_aligned(
      hint, sz, alignment, kVMAllocateProt, kVMAllocateFlags);
}

void vm_free(void *p, size_t sz) {
  vm_munmap(p, sz);
}

void vm_free_aligned(void *p, size_t sz) {
  vm_free(p, sz);
}

static constexpr int kVMReserveProt = PROT_NONE;
static constexpr int kVMReserveFlags =
    MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;

llvh::ErrorOr<void *>
vm_reserve_aligned(size_t sz, size_t alignment, void *hint) {
  return vm_mmap_aligned(hint, sz, alignment, kVMReserveProt, kVMReserveFlags);
}

void vm_release_aligned(void *p, size_t sz) {
  vm_munmap(p, sz);
}

llvh::ErrorOr<void *> vm_commit(void *p, size_t sz) {
  return vm_mmap(p, sz, kVMAllocateProt, kVMAllocateFlags | MAP_FIXED, false);
}

void vm_uncommit(void *p, size_t sz) {
  auto res = vm_mmap(p, sz, kVMReserveProt, kVMReserveFlags | MAP_FIXED, false);
  (void)res;
  assert(res && "uncommit failed");
}

void vm_hugepage(void *p, size_t sz) {
  assert(
      reinterpret_cast<uintptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");

#if defined(__linux__) || defined(__ANDROID__)
  // Since the alloc is aligned, it may benefit from huge pages.
  madvise(p, sz, MADV_HUGEPAGE);
#endif
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

llvh::ErrorOr<size_t> vm_footprint(char *start, char *end) {
#ifdef __MACH__
  const task_t self = mach_task_self();

  vm_address_t vAddr = reinterpret_cast<vm_address_t>(start);
  vm_size_t vSz = static_cast<vm_size_t>(end - start);
  vm_region_extended_info_data_t info;
  mach_msg_type_number_t fields = VM_REGION_EXTENDED_INFO_COUNT;
  mach_port_t unused;

  auto ret = vm_region_64(
      self,
      &vAddr,
      &vSz,
      VM_REGION_EXTENDED_INFO,
      // The expected contents, and requisite size of this struct depend on the
      // previous and next parameters to this function respectively. We cast it
      // to a "generic" info type to indicate this.
      reinterpret_cast<vm_region_info_t>(&info),
      &fields,
      &unused);

  if (ret != KERN_SUCCESS)
    return std::error_code(errno, std::generic_category());

  return info.pages_dirtied;
#else
  auto rStart = reinterpret_cast<uintptr_t>(start);
  auto rEnd = reinterpret_cast<uintptr_t>(end);

  char label[] = "Rss:";

  std::ifstream smaps("/proc/self/smaps");

  while (smaps) {
    std::string firstToken;
    smaps >> firstToken;

    // Ignore the rest of the line.
    smaps.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (firstToken.find_last_of(':') != std::string::npos) {
      // We are inside an entry, rather than at the start of one, so we should
      // ignore this line.
      continue;
    }

    // The first token should be the mapping's virtual address range if this is
    // the first line of a mapping's entry, so we extract it.
    std::stringstream ris(firstToken);
    uintptr_t mStart, mEnd;
    ris >> std::hex >> mStart;
    // Ignore '-'
    ris.ignore();
    ris >> mEnd;

    // The working assumption is that the kernel will not split a single memory
    // region allocated by \p mmap across multiple entries in the smaps output.
    if (mStart <= rStart && rEnd <= mEnd) {
      // Found the start of the section pertaining to our memory map
      break;
    }
  }

  while (smaps) {
    std::string line;
    std::getline(smaps, line);

    if (line.compare(0, sizeof(label) - 1, label) != 0) {
      continue;
    }

    std::stringstream lis(line);
    lis.ignore(line.length(), ' '); // Pop the label

    size_t rss;
    std::string unit;
    lis >> std::skipws >> rss >> unit;

    assert(unit == "kB");
    return rss * 1024 / page_size();
  }

  return std::error_code(errno, std::generic_category());
#endif
}

int pages_in_ram(const void *p, size_t sz, llvh::SmallVectorImpl<int> *runs) {
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
  if (fscanf(fp, "%*d %ld", &rss) != 1) {
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

#if defined(__linux__)
static bool overlap(uintptr_t a, size_t asize, uintptr_t b, size_t bsize) {
  // An empty interval has no overlap.
  if (asize == 0 || bsize == 0)
    return false;
  // Order by start address.
  if (a > b)
    return overlap(b, bsize, a, asize);
  // Overlap iff the first interval extends beyond the start of the second.
  return a + asize > b;
}
#endif

std::vector<std::string> get_vm_protect_modes(const void *p, size_t sz) {
  std::vector<std::string> modes;
#if defined(__linux__)
  unsigned long long begin;
  unsigned long long end;
  char mode[4 + 1];
  FILE *fp = fopen("/proc/self/maps", "r");
  if (!fp) {
    modes.emplace_back("unknown");
    return modes;
  }
  while (fscanf(fp, "%llx-%llx %4s", &begin, &end, mode) == 3) {
    if (overlap(
            reinterpret_cast<uintptr_t>(p),
            sz,
            static_cast<uintptr_t>(begin),
            static_cast<size_t>(end - begin))) {
      modes.push_back(mode);
    }
    // Discard remainder of the line.
    int result;
    do {
      result = fgetc(fp);
    } while (result != '\n' && result > 0);
  }
#endif
  return modes;
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

void set_thread_name(const char *name) {
  // Set the thread name for TSAN. It doesn't share the same name mapping as the
  // OS does. This macro expands to nothing if TSAN isn't on.
  TsanThreadName(name);
#if defined(__linux__) || defined(__ANDROID__)
  prctl(PR_SET_NAME, name);
#elif defined(__APPLE__)
  ::pthread_setname_np(name);
#endif
  // Do nothing if the platform doesn't support it.
}

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

#ifdef __linux__
std::vector<bool> sched_getaffinity() {
  std::vector<bool> v;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  int status = ::sched_getaffinity(0, sizeof(mask), &mask);
  if (status != 0) {
    return v;
  }
  int lastSet = -1;
  for (int cpu = 0; cpu < CPU_SETSIZE; ++cpu) {
    v.push_back(CPU_ISSET(cpu, &mask));
    if (v.back())
      lastSet = cpu;
  }
  // Trim trailing zeroes.
  v.resize(lastSet + 1);
  return v;
}

int sched_getcpu() {
  return ::sched_getcpu();
}
#else
std::vector<bool> sched_getaffinity() {
  // Not yet supported.
  return std::vector<bool>();
}

int sched_getcpu() {
  // Not yet supported.
  return -1;
}
#endif

uint64_t cpu_cycle_counter() {
#if defined(__aarch64__)
  // Clang's builtin causes SIGILL on some 64-bit ARM environments.
  uint64_t cnt;
  __asm __volatile("mrs %0, cntvct_el0" : "=&r"(cnt));
  return cnt;
#elif __has_builtin(__builtin_readcyclecounter)
  return __builtin_readcyclecounter();
#else
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * 1000LL * 1000LL * 1000LL + t.tv_nsec;
#endif
}

bool set_env(const char *name, const char *value) {
  // Enforce the contract of this function that value must not be empty
  assert(*value != '\0' && "value cannot be empty string");
  return setenv(name, value, 1) == 0;
}

bool unset_env(const char *name) {
  return unsetenv(name) == 0;
}

/*static*/
void *SigAltStackLeakSuppressor::stackRoot_{nullptr};

SigAltStackLeakSuppressor::~SigAltStackLeakSuppressor() {
  stack_t oldAltStack;
  if (sigaltstack(nullptr, &oldAltStack) == 0) {
    stackRoot_ = oldAltStack.ss_sp;
  }
}

} // namespace oscompat
} // namespace hermes

#endif // not _WINDOWS
