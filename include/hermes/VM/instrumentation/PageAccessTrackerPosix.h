/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERPOSIX_H
#define HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERPOSIX_H

#include <signal.h>
#include <memory>

#include "llvm/Support/raw_ostream.h"

namespace hermes {

/// Track paging behavior over a mmapped address range.
/// This is only meant be used in single-thread applications.
/// Usage:
///   ... create a mmapped buffer ...
///   PageAccessTracker::initialize(...);
///   ... do your thing ...
///   PageAccessTracker::printStats(...);
///   PageAccessTracker::shutdown();
class PageAccessTracker {
 private:
  /// Single instance of PageAccessTracker. This can only be instantiated when
  /// calling initialize, and can only be modified in the signal handler.
  static PageAccessTracker *volatile tracker;
  uint32_t pageSize_{0};
  /// Page aligned address for the beginning of the buffer.
  void *bufStartPage_{nullptr};
  /// Total number of pages in the that we are tracking, starting from
  /// bufStartPage_.
  uint32_t totalPages_{0};
  /// Array of page ids in the order they are accessed.
  std::unique_ptr<uint32_t[]> accessedPageIds_{nullptr};
  /// Total number of pages accessed during executing bytecode.
  uint32_t accessedPageCount_{0};
  /// Signal number that we are tracking.
  int signal_{0};

  PageAccessTracker(
      uint32_t pageSize,
      void *bufStartPage,
      uint32_t totalPages,
      int signal);

  ~PageAccessTracker() = default;

  /// Given \p accessedAddr, the accessed address, record the access.
  /// Intended to only be called by the signal handler.
  void recordPageAccess(void *accessedPage);

  /// Given \p addr, determine if we are tracking this address.
  bool isTracking(void *addr);

  /// Print the tracked stats, including page size, total number of pages,
  /// number of pages accessed, and the page ids in the accessed order.
  void printStats(llvm::raw_ostream &OS);
  void printStatsJSON(llvm::raw_ostream &OS);

  /// Print only the page ids in the accessed order.
  void printPageAccessedOrder(llvm::raw_ostream &OS);
  void printPageAccessedOrderJSON(llvm::raw_ostream &OS);

  /// Handle the signal sent when an inaccesible page is touched. When the page
  /// touched is in the range we are tracking, mark it readable and continue
  /// execution; otherwise re-trigger the signal, handle it with default
  /// handler, and kill the process.
  static void signalHandler(int signum, siginfo_t *si, void * /* unused */);

 public:
  /// Initialize the PageAccessTracker, mark the whole address range unreadable.
  /// This function is intended to only be called once before shutdown.
  /// Note if the beginning or end of the buffer is not on the page boundary,
  /// the first page or last page the buffer actually covers won't be tracked.
  /// \param bufStart, pointer to the start of the buffer.
  /// \param bufSize, the size of the buffer.
  /// \return true if initialization is successful.
  static bool initialize(void *bufStart, size_t bufSize);

  /// Print the tracked stats, including page size, total number of pages,
  /// number of pages accessed, and the page ids in the accessed order.
  /// \param json If true, print in json format, otherwise in a more readable
  /// format.
  /// \return true if function completed successfully, false on failure.
  static bool printStats(llvm::raw_ostream &OS, bool json);

  /// Print only the page ids in the accessed order.
  /// \param json If true, print in json format, otherwise in a more readable
  /// format.
  /// \return true if function completed successfully, false on failure.
  static bool printPageAccessedOrder(llvm::raw_ostream &OS, bool json);

  /// Shut down the PageAccessTracker.
  /// \return true if shutdown is successful.
  static bool shutdown();
};

} // namespace hermes

#endif // HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERPOSIX_H
