/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_OSCOMPAT_H
#define HERMES_SUPPORT_OSCOMPAT_H

#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/ErrorOr.h"

#ifdef _WINDOWS
#include <io.h>
// Expose alloca to whoever includes OSCompat.h
// On Windows, malloc.h defines alloca (alloca is used in Runtime.cpp
// in order to achieve non-deterministic stack depth).
#include <malloc.h>
#else
#include <unistd.h>
#endif

#include <chrono>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

// This file defines cross-os APIs for functionality provided by our target
// operating systems.

namespace hermes {
namespace oscompat {

#ifndef NDEBUG
void set_test_page_size(size_t pageSz);
void reset_test_page_size();
#endif // !NDEBUG

/// Returns the current page size.
size_t page_size();

#ifndef NDEBUG
/// For testing purposes, we can limit the maximum net change in allocated
/// virtual address space from this point forward.  That is, if we track the sum
/// of future allocations, minus future frees, an allocation that would make
/// that sum exceed \p totSz fails.
void set_test_vm_allocate_limit(size_t totSz);

/// Return the test VM allocation lmit to "unlimited."
void unset_test_vm_allocate_limit();
#endif // !NDEBUG

// Allocates a virtual memory region of the given size (required to be
// a multiple of page_size()), and returns a pointer to the start.
// Optionally specify a page-aligned hint for where to place the mapping.
// Returns nullptr if the allocation is unsuccessful.  The pages
// will be zero-filled on demand.
llvh::ErrorOr<void *> vm_allocate(size_t sz, void *hint = nullptr);

// Allocates a virtual memory region of the given size and alignment (both
// must be multiples of page_size()), and returns a pointer to the start.
// Optionally specify a page-aligned hint for where to place the mapping.
// Returns nullptr if the allocation is unsuccessful.  The pages
// will be zero-filled on demand.
llvh::ErrorOr<void *>
vm_allocate_aligned(size_t sz, size_t alignment, void *hint = nullptr);

/// Free a virtual memory region allocated by \p vm_allocate.
/// \p p must point to the base address that was returned by \p vm_allocate.
/// Memory region returned by \p vm_allocate_aligned must be freed by
/// invoking \p vm_free_aligned, instead of this function.
/// \p size must match the value passed to the respective allocate functions.
/// In other words, partial free is not allowed.
void vm_free(void *p, size_t sz);

/// Similar to \p vm_free, but for memory regions returned by
/// \p vm_allocate_aligned.
void vm_free_aligned(void *p, size_t sz);

/// Similar to vm_allocate_aligned, but regions of memory must be explicitly
/// committed with \p vm_commit before they are used. This can be used to
/// reserve large contiguous address spaces without failing due to overcommit.
llvh::ErrorOr<void *>
vm_reserve_aligned(size_t sz, size_t alignment, void *hint = nullptr);

/// Similar to \p vm_free, but for memory regions returned by
/// \p vm_reserve_aligned.
void vm_release_aligned(void *p, size_t sz);

/// Commit a region of memory so that it can be used. \p sz must be page aligned
/// and \p p must be a page aligned pointer in a region returned by
/// \p vm_reserve_aligned. The pages will be zero-filled on demand.
llvh::ErrorOr<void *> vm_commit(void *p, size_t sz);

/// Uncommit a region of memory once it is no longer in use. \p sz must be page
/// aligned and \p p must be a page aligned pointer in a region returned by
/// \p vm_reserve_aligned.
void vm_uncommit(void *p, size_t sz);

/// Mark the \p sz byte region of memory starting at \p p as being a good
/// candidate for huge pages.
/// \pre sz must be a multiple of oscompat::page_size().
void vm_hugepage(void *p, size_t sz);

/// Mark the \p sz byte region of memory starting at \p p as not currently in
/// use, so that the OS may free it. \p p must be page-aligned.
void vm_unused(void *p, size_t sz);

/// Mark the \p sz byte region of memory starting at \p p as soon being needed,
/// so that the OS may prefetch it. \p p must be page-aligned.
void vm_prefetch(void *p, size_t sz);

/// Assign a \p name to the \p sz byte region of virtual memory starting at
/// pointer \p p.  The name is assigned only on supported platforms (currently
/// only Android).  This name appears when the OS is queried about the mapping
/// for a process (e.g. by /proc/<pid>/maps).
void vm_name(void *p, size_t sz, const char *name);

/// None indicates no access; ReadWrite allows reading and writing.
/// (We can add finer granularity, like read-only, if required.)
enum class ProtectMode { ReadWrite, None };

/// Set the \p sz byte region of memory starting at \p p to the specified
/// \p mode. \p p must be page-aligned. \return true if successful,
/// false on error.
bool vm_protect(void *p, size_t sz, ProtectMode mode);

enum class MAdvice { Random, Sequential };

/// Issue an madvise() call.
/// \return true on success, false on error.
bool vm_madvise(void *p, size_t sz, MAdvice advice);

/// Return the footprint of the memory-mapping starting at \p start (inclusive)
/// and ending at \p end (exclusive). The notions of "footprint" and "mapping"
/// are platform-specific, conforming to the following specification:
///
///  - "Mapping" refers to the abstraction from the kernel for contiguous chunks
///    of virtual memory (i.e. from `mmap` or `vm_allocate`).
///  - "Footprint" refers to the metric by which the platform measures the
///    impact of a region on memory pressure. I.e. if this metric goes up, the
///    likelihood that the process is killed due to memory pressure increases.
///
/// \return the footprint as a number of pages on success, and error on
/// failure.
llvh::ErrorOr<size_t> vm_footprint(char *start, char *end);

/// Return the number of pages in the given region that are currently in RAM.
/// If \p runs is provided, then populate it with the lengths of runs of
/// consecutive pages with the same resident/non-resident status, alternating
/// between the two statuses, and with the first element always denoting a
/// number of resident pages (0 if the first page is not resident).
///
/// Return -1 on failure (including not supported).
int pages_in_ram(
    const void *p,
    size_t sz,
    llvh::SmallVectorImpl<int> *runs = nullptr);

/// Return the protection modes of all memory mappings that overlap with
/// [p, p + sz). Each mode is a 4-character string of the form:
/// [r-][w-][x-][ps] where the last character indicates private/shared.
/// Returns an empty vector if this operation is not supported.
std::vector<std::string> get_vm_protect_modes(const void *p, size_t sz);

/// Resident set size (RSS), in bytes: the amount of RAM used by the process.
/// It excludes virtual memory that has been paged out or was never loaded.
/// \return Peak RSS usage throughout this process's history.
uint64_t peak_rss();

/// Resident set size (RSS), in bytes: the amount of RAM used by the process.
/// It excludes virtual memory that has been paged out or was never loaded.
/// \return Current RSS usage.
uint64_t current_rss();

/// Private dirty bytes for the process, which is a subset of the RSS. This
/// excludes pages that could be paged out (e.g. a mapped readonly file).
uint64_t current_private_dirty();

/// Get the number of \p voluntary and \p involuntary context switches the
/// process has made so far, or return false if unsupported.
bool num_context_switches(long &voluntary, long &involuntary);

/// \return OS thread id of current thread.
uint64_t thread_id();

/// Set the thread name for the current thread. This can be viewed in various
/// debuggers and profilers.
/// NOTE: Is a no-op on some platforms.
void set_thread_name(const char *name);

/// \return the duration in microseconds the CPU has spent executing this thread
/// upon success, or `std::chrono::microseconds::max()` on failure.
std::chrono::microseconds thread_cpu_time();

/// Get by reference the minor and major page fault counts for the current
/// thread. \return true if successful, false on error.
bool thread_page_fault_count(int64_t *outMinorFaults, int64_t *outMajorFaults);

/// \return name of current thread.
std::string thread_name();

/// \return mask of all CPU cores on which this thread may be scheduled,
/// or an empty vector on error.
std::vector<bool> sched_getaffinity();

/// \return the CPU core where this thread is currently scheduled,
/// or -1 on error.
int sched_getcpu();

/// \return a monotonically increasing count of time, where the unit depends on
/// the implementation.
uint64_t cpu_cycle_counter();

#ifdef _WINDOWS

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/// \return whether fd refers to a terminal / character device
inline int isatty(int fd) {
  return ::_isatty(fd);
}

inline bool should_color(int fd) {
  return false;
}

#else

/// \return whether fd refers to a terminal / character device
inline int isatty(int fd) {
  return ::isatty(fd);
}

inline bool should_color(int fd) {
  char *term = getenv("TERM");
  if (term == nullptr || strcmp("dumb", term) == 0) {
    return false;
  }
  return isatty(fd);
}

#endif

/// Set the env var \p name to \p value.
/// \p value must not be an empty string.
/// Setting an env var to empty is not supported because doing it
/// cross-platform is hard.
/// \return true if successful, false on error.
bool set_env(const char *name, const char *value);

/// Unset the env var \p name.
/// \return true if successful, false on error.
bool unset_env(const char *name);

/// LLVM sets up an alternate signal stack.  By default, the stack is
/// never deleted, and is reported as a leak.  The destructor of this
/// class makes a static variable point to the stack (if one exists).
/// ASAN does a limited reachability analysis: blocks directly
/// reachable from global variables (conservatively treating all
/// aligned bit patterns as possible pointers) are not considered
/// leaks.  The intended usages is that the main function of an
/// executable will create a local variable of this type in its main
/// function, so that the destructor will be executed before the
/// process completes.  The general requirement is that the destructor
/// of some instance be executed after the alternate signal stack gets
/// its last value.
class SigAltStackLeakSuppressor {
 public:
  SigAltStackLeakSuppressor() = default;
  ~SigAltStackLeakSuppressor();

 private:
  /// The static variable that will root the alternate stack.
  static void *stackRoot_;
};

} // namespace oscompat
} // namespace hermes

#endif // HERMES_VM_OSCOMPAT_H
