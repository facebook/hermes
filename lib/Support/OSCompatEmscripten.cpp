/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef __EMSCRIPTEN__

#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"

#include <cassert>
#include <vector>

#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <unistd.h>

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

static llvh::ErrorOr<void *> vm_allocate_impl(size_t sz) {
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

llvh::ErrorOr<void *> vm_allocate(size_t sz, void * /* hint */) {
  assert(sz % page_size() == 0);
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
#endif // !NDEBUG
  return vm_allocate_impl(sz);
}

llvh::ErrorOr<void *>
vm_allocate_aligned(size_t sz, size_t alignment, void * /* hint */) {
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);
  // Ensure the alignment is a power of two as this is required by
  // aligned_alloc.
  assert(llvh::isPowerOf2_64(alignment));

  // Emscripten does not support partial munmap, so use aligned_alloc to obtain
  // an aligned region and then memset it to zero.
  auto *p = aligned_alloc(alignment, sz);
  if (!p)
    return std::error_code(errno, std::generic_category());
  memset(p, 0, sz);
  return p;
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
  free(p);
}

/// Define a no-op implementation of the reserve/commit APIs that just call
/// through to regular allocations.
llvh::ErrorOr<void *>
vm_reserve_aligned(size_t sz, size_t alignment, void *hint) {
  return vm_allocate_aligned(sz, alignment, hint);
}
void vm_release_aligned(void *p, size_t sz) {
  vm_free_aligned(p, sz);
}
llvh::ErrorOr<void *> vm_commit(void *p, size_t sz) {
  return p;
}
void vm_uncommit(void *p, size_t sz) {}

void vm_hugepage(void *p, size_t sz) {
  assert(
      reinterpret_cast<uintptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");
}

void vm_unused(void *p, size_t sz) {
#ifndef NDEBUG
  const size_t PS = page_size();
  assert(
      reinterpret_cast<intptr_t>(p) % PS == 0 &&
      "Precondition: pointer is page-aligned.");
#endif
}

void vm_prefetch(void *p, size_t sz) {
  assert(
      reinterpret_cast<intptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");
}

void vm_name(void *p, size_t sz, const char *name) {}

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
  return true;
}

llvh::ErrorOr<size_t> vm_footprint(char *start, char *end) {
  return std::error_code(errno, std::generic_category());
}

int pages_in_ram(const void *p, size_t sz, llvh::SmallVectorImpl<int> *runs) {
  return -1;
}

uint64_t peak_rss() {
  return 0;
}

uint64_t current_rss() {
  return 0;
}

uint64_t current_private_dirty() {
  return 0;
}

std::vector<std::string> get_vm_protect_modes(const void *p, size_t sz) {
  return std::vector<std::string>{};
}

bool num_context_switches(long &voluntary, long &involuntary) {
  voluntary = involuntary = -1;
  return false;
}

uint64_t thread_id() {
  return 0;
}

void set_thread_name(const char *name) {
  // Intentionally does nothing
}

// Platform-specific implementations of thread_cpu_time

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

// Platform-specific implementations of thread_page_fault_count

bool thread_page_fault_count(int64_t *outMinorFaults, int64_t *outMajorFaults) {
  return false;
}

std::string thread_name() {
  return "";
}

std::vector<bool> sched_getaffinity() {
  // Not yet supported.
  return std::vector<bool>();
}

int sched_getcpu() {
  // Not yet supported.
  return -1;
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

SigAltStackLeakSuppressor::~SigAltStackLeakSuppressor() {}

} // namespace oscompat
} // namespace hermes

#endif // __EMSCRIPTEN__
