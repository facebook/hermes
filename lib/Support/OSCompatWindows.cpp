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

static void *vm_allocate_impl(size_t sz) {
#ifndef NDEBUG
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

static char *alignAlloc(void *p, size_t alignment) {
  return reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(p), alignment));
}

void *vm_allocate(size_t sz) {
  assert(sz % page_size() == 0);
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
#endif // !NDEBUG
  return vm_allocate_impl(sz);
}

void *vm_allocate_aligned(size_t sz, size_t alignment) {
  // TODO(T40415882) Add support for aligned allocation on Windows.
  assert(false && "Aligned allocation is not supported on Windows");
  return nullptr;
}

void vm_free(void *p, size_t sz) {
  BOOL ret = VirtualFree(p, 0, MEM_RELEASE);

  assert(ret && "Failed to free memory region.");
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

uint64_t peak_rss() {
  PROCESS_MEMORY_COUNTERS pmc;
  auto ret = GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
  if (ret != 0) {
    // failed
    return 0;
  }
  return pmc.PeakWorkingSetSize;
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

} // namespace oscompat
} // namespace hermes

#endif // _WINDOWS
