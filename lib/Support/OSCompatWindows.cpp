/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef _WINDOWS

#include "hermes/Support/OSCompat.h"

#include <cassert>

// Include windows.h first because other includes from windows API need it.
// The blank line after the include is necessary to avoid lint error.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // do not define min/max macros
#include <windows.h>

#include <io.h>
#include <psapi.h>

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
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwPageSize;
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

static char *alignAlloc(void *p, size_t alignment) {
  return reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(p), alignment));
}

void *vm_allocate(size_t sz) {
#ifndef NDEBUG
  assert(sz % page_size() == 0);
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
  if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
    return nullptr;
  } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
    totalVMAllocLimit -= sz;
  }
#endif // !NDEBUG

  // TODO(T40416012) introduce explicit "commit" in OSCompat abstraction of
  // virtual memory

  // In POSIX, a mem page implicitly transitions from "reserved" state to
  // "committed" state on access. However, on Windows, accessing
  // "reserved" but not "committed" page results in an access violation.
  // There is no explicit call to transition to "committed" state
  // in Hermes' virtual memory abstraction.
  // As a result, even though Windows allows one to "reserve" a page without
  // "commit"ting it, we have to do both here.
  void *result =
      VirtualAlloc(nullptr, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  return result; // result == nullptr if VirtualAlloc failed
}

void *vm_allocate_aligned(size_t sz, size_t alignment) {
  /// A value of 3 means vm_allocate_aligned will:
  /// 1. Opportunistic: allocate and see if it happens to be aligned
  /// 2. Regular: Try aligned allocation 3 times (see below for details)
  /// 3. Fallback: Allocate more than needed, and waste the excess
  constexpr int aligned_allocation_attempts = 3;

#ifndef NDEBUG
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);
  if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
    return nullptr;
  } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
    totalVMAllocLimit -= sz;
  }
#endif // !NDEBUG

  // Opportunistically allocate without alignment constraint,
  // and see if the memory happens to be aligned.
  // While this may be unlikely on the first allocation request,
  // subsequent allocation requests have a good chance.
  void *addr =
      VirtualAlloc(nullptr, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (addr == nullptr) {
    // Don't attempt to do anything further if the allocation failed.
    return nullptr;
  }
  if (LLVM_LIKELY(addr == alignAlloc(addr, alignment))) {
    return addr;
  }
  // Free the oppotunistic allocation.
  BOOL free_ret = VirtualFree(addr, 0, MEM_RELEASE);
  assert(free_ret && "Failed to free memory region in vm_allocate_aligned");
  (void)free_ret;

  for (int attempts = 0; attempts < aligned_allocation_attempts; attempts++) {
    // Allocate a larger section to ensure that it contains
    // a subsection that satisfies the request.
    addr = VirtualAlloc(
        nullptr,
        sz + alignment - page_size_real(),
        MEM_RESERVE,
        PAGE_READWRITE);
    if (addr == nullptr) {
      return nullptr;
    }
    // Find the desired subsection
    char *aligned = alignAlloc(addr, alignment);

    // Free the larger allocation (including the desired subsection)
    free_ret = VirtualFree(addr, 0, MEM_RELEASE);
    assert(free_ret && "Failed to free memory region in vm_allocate_aligned");
    (void)free_ret;

    // Request allocation at the desired subsection
    void *alloc_at_aligned =
        VirtualAlloc(aligned, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (alloc_at_aligned != nullptr) {
      assert(alloc_at_aligned == aligned);
      return aligned;
    }
  }

  // Similar to the regular mechanism, but simply return the desired
  // subsection (instead of free and re-allocate). This has two downsides:
  // 1. Wasted virtual address space.
  // 2. vm_free_aligned is now required to call VirtualQuery, which has
  //    a non-trivial cost.
  addr = VirtualAlloc(
      nullptr, sz + alignment - page_size_real(), MEM_RESERVE, PAGE_READWRITE);
  if (addr == nullptr) {
    return nullptr;
  }
  addr = alignAlloc(addr, alignment);
  addr = VirtualAlloc(addr, alignment, MEM_COMMIT, PAGE_READWRITE);
  assert(
      addr &&
      "Failed to commit subsection of reserved memory in vm_allocate_aligned");
  return addr;
}

void vm_free(void *p, size_t sz) {
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    vm_free_aligned(p, sz);
    return;
  }
#endif // !NDEBUG

  BOOL ret = VirtualFree(p, 0, MEM_RELEASE);

  assert(ret && "Failed to free memory region.");
  (void)ret;

#ifndef NDEBUG
  if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit) && p) {
    totalVMAllocLimit += sz;
  }
#endif
}

void vm_free_aligned(void *p, size_t sz) {
  // VirtualQuery is necessary because p may not be the base location
  // of the allocation (due to possible fallback in vm_allocate_aligned).
  MEMORY_BASIC_INFORMATION mbi;
  SIZE_T query_ret = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
  assert(query_ret != 0 && "Failed to invoke VirtualQuery in vm_free_aligned");

  BOOL ret = VirtualFree(mbi.AllocationBase, 0, MEM_RELEASE);
  assert(ret && "Failed to invoke VirtualFree in vm_free_aligned.");
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

  // TODO(T40416012) introduce explicit "commit" in OSCompat abstraction of
  // virtual memory

  // Do nothing.

  // In POSIX, a mem page implicitly transitions from "reserved" state to
  // "committed" state on access. However, on Windows, accessing
  // "reserved" but not "committed" page results in an access violation.
  // There is no explicit call to transition to "committed" state
  // in Hermes' virtual memory abstraction.
  // As a result, even though Windows has an API to transition a page from
  // "committed" state back to "reserved" state, we can not invoke it here.
}

void vm_prefetch(void *p, size_t sz) {
  assert(
      reinterpret_cast<intptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");

  // TODO(T40415796) provide actual "prefetch" implementation

  // Do nothing
}

void vm_name(void *p, size_t sz, const char *name) {
  (void)p;
  (void)sz;
  (void)name;
}

bool vm_protect(void *p, size_t sz, ProtectMode) {
  DWORD oldProtect;
  BOOL err = VirtualProtect(p, sz, PAGE_READWRITE, &oldProtect);
  return err != 0;
}

int pages_in_ram(const void *p, size_t sz, llvm::SmallVectorImpl<int> *runs) {
  // Not yet supported.
  return -1;
}

uint64_t peak_rss() {
  PROCESS_MEMORY_COUNTERS pmc;
  auto ret = GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
  if (ret != 0) {
    // failed
    return 0;
  }
  return pmc.PeakWorkingSetSize;
}

bool num_context_switches(long &voluntary, long &involuntary) {
  // Not yet supported.
  voluntary = involuntary = -1;
  return false;
}

uint64_t thread_id() {
  return GetCurrentThreadId();
}

static std::chrono::microseconds::rep fromFileTimeToMicros(
    const FILETIME &fileTime) {
  ULARGE_INTEGER uli;
  uli.LowPart = fileTime.dwLowDateTime;
  uli.HighPart = fileTime.dwHighDateTime;
  // convert from 100-nanosecond to microsecond
  return uli.QuadPart / 10;
}

std::chrono::microseconds thread_cpu_time() {
  FILETIME creationTime;
  FILETIME exitTime;
  FILETIME kernelTime;
  FILETIME userTime;
  GetThreadTimes(
      GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime);
  return std::chrono::microseconds(
      fromFileTimeToMicros(kernelTime) + fromFileTimeToMicros(userTime));
}

bool thread_page_fault_count(int64_t *outMinorFaults, int64_t *outMajorFaults) {
  // Windows provides GetProcessMemoryInfo. There is no counterpart of this
  // API call for threads. In addition, it measures soft page faults.
  // It may be possible to get per-thread information by using ETW (Event
  // Tracing for Windows), which is probably overkill for the use case here.

  // not supported on Windows
  return false;
}

std::string thread_name() {
  // SetThreadDescription/GetThreadDescription is too new (since
  // Windows 10 version 1607).
  // Prior to that, the concept of thread names only exists when
  // a Visual Studio debugger is attached.
  return "";
}

bool set_env(const char *name, const char *value) {
  // Setting an env var to empty requires a lot of hacks on Windows
  assert(*value != '\0' && "value cannot be empty string");
  return _putenv_s(name, value) == 0;
}

bool unset_env(const char *name) {
  return _putenv_s(name, "") == 0;
}

} // namespace oscompat
} // namespace hermes

#endif // _WINDOWS
