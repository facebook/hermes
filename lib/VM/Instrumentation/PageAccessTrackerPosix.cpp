/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef _WINDOWS

#include "hermes/VM/instrumentation/PageAccessTracker.h"

#include "hermes/Support/JSONEmitter.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "llvm/ADT/STLExtras.h"

using namespace hermes;

PageAccessTracker *volatile PageAccessTracker::tracker = nullptr;

bool PageAccessTracker::initialize(void *bufStart, size_t bufSize) {
  if (tracker) {
    llvm::errs() << "PageAccessTracker can only be initialized once.\n";
    return false;
  }
  uint32_t pageSize = getpagesize();
  if (bufSize < pageSize) {
    llvm::errs()
        << "Nothing to track because the buffer is less than a page.\n";
    return false;
  }
  // If buffer start is not page aligned, choose the next page's start address.
  void *bufStartPage =
      (void *)(((uintptr_t)bufStart + pageSize - 1) / pageSize * pageSize);
  // If buffer end is not page aligned, choose the last page end address that
  // is inside the buffer.
  void *bufEndPage =
      (void *)(((uintptr_t)bufStart + bufSize) / pageSize * pageSize);
  assert(
      (uintptr_t)bufStartPage <= (uintptr_t)bufEndPage &&
      "Buffer start page should be before end page.");
  uint32_t totalPages =
      ((uintptr_t)bufEndPage - (uintptr_t)bufStartPage) / pageSize;

// On linux, failure to access the page triggers a sigsegv; on macOS, a sigbus.
#if defined(__linux__)
  const int signum = SIGSEGV;
#elif defined(__APPLE__) && defined(__MACH__)
  const int signum = SIGBUS;
#else
#error "OS type not supported."
#endif

  tracker = new PageAccessTracker(pageSize, bufStartPage, totalPages, signum);

  // Register custom signal handler for accessing unreadable pages.
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = signalHandler;
  if (sigaction(signum, &sa, NULL) != 0) {
    perror("sigaction failed");
    return false;
  }

  // Mark the whole bytecode file unreadable.
  if (mprotect(bufStartPage, totalPages * pageSize, PROT_NONE) != 0) {
    perror("mprotect failed");
    return false;
  }

  return true;
}

bool PageAccessTracker::shutdown() {
  if (!tracker) {
    llvm::errs()
        << "Cannot shutdown PageAccessTracker without first initializing it.\n";
    return false;
  }
  // Reset the signal handler to default.
  if (signal(tracker->signal_, SIG_DFL) == SIG_ERR) {
    perror("signal failed");
    return false;
  }
  delete tracker;
  tracker = nullptr;
  return true;
}

PageAccessTracker::PageAccessTracker(
    uint32_t pageSize,
    void *bufStartPage,
    uint32_t totalPages,
    int signal)
    : pageSize_(pageSize),
      bufStartPage_(bufStartPage),
      totalPages_(totalPages),
      signal_(signal) {
  accessedPageIds_ = llvm::make_unique<unsigned int[]>(totalPages);
}

void PageAccessTracker::recordPageAccess(void *accessedAddr) {
  assert(accessedPageCount_ < totalPages_ && "Unexpected page count overflow.");
  accessedPageIds_[accessedPageCount_++] =
      ((uintptr_t)accessedAddr - (uintptr_t)bufStartPage_) / pageSize_;
}

bool PageAccessTracker::isTracking(void *addr) {
  return (uintptr_t)addr >= (uintptr_t)bufStartPage_ &&
      (uintptr_t)addr < (uintptr_t)bufStartPage_ + pageSize_ * totalPages_;
}

void PageAccessTracker::signalHandler(
    int signum,
    siginfo_t *si,
    void * /* unused */) {
  if (!tracker) {
    // This is impossible because the signal handler would only be registered
    // if the PageAccessTracker is initialized.
    _exit(EXIT_FAILURE);
  }

  // Check if accessed address falls into the file buffer range.
  // If the address is not in range, set the default signal handler and
  // kill the process.
  if (!tracker->isTracking(si->si_addr)) {
    if (signal(signum, SIG_DFL) == SIG_ERR) {
      _exit(EXIT_FAILURE);
    }
    kill(getpid(), signum);
    return;
  }
  tracker->recordPageAccess(si->si_addr);
  // Mark the page readable so we can continue execution.
  void *accessedPage =
      (void
           *)((uintptr_t)si->si_addr / tracker->pageSize_ * tracker->pageSize_);
  if (mprotect(accessedPage, tracker->pageSize_, PROT_READ) != 0) {
    _exit(EXIT_FAILURE);
  }
}

void PageAccessTracker::printStats(llvm::raw_ostream &OS) {
  OS << "Bytecode I/O stats:\n";
  OS << "page size: " << pageSize_ << "\n";
  OS << "number of pages in bytecode file (rounded down): " << totalPages_
     << "\n";
  OS << "number of pages accessed during execution: " << accessedPageCount_
     << "\n";
  printPageAccessedOrder(OS);
}

void PageAccessTracker::printStatsJSON(llvm::raw_ostream &OS) {
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("page_size", pageSize_);
  json.emitKeyValue("total_pages", totalPages_);
  json.emitKeyValue("accessed_pages", accessedPageCount_);
  json.emitKey("page_ids");
  json.openArray();
  for (unsigned i = 0; i < accessedPageCount_; ++i) {
    json.emitValue(accessedPageIds_[i]);
  }
  json.closeArray();
  json.closeDict();
}

void PageAccessTracker::printPageAccessedOrder(llvm::raw_ostream &OS) {
  OS << "Page ids in accessed order:\n";
  for (unsigned i = 0; i < accessedPageCount_; ++i) {
    OS << accessedPageIds_[i] << "\n";
  }
}

void PageAccessTracker::printPageAccessedOrderJSON(llvm::raw_ostream &OS) {
  JSONEmitter json(OS);
  json.openDict();
  json.emitKey("page_ids");
  json.openArray();
  for (unsigned i = 0; i < accessedPageCount_; ++i) {
    json.emitValue(accessedPageIds_[i]);
  }
  json.closeArray();
  json.closeDict();
}

bool PageAccessTracker::printStats(llvm::raw_ostream &OS, bool json) {
  if (!tracker) {
    llvm::errs() << "PageAccessTracker is not initialized.\n";
    return false;
  }
  if (json) {
    tracker->printStatsJSON(OS);
  } else {
    tracker->printStats(OS);
  }
  return true;
}

bool PageAccessTracker::printPageAccessedOrder(
    llvm::raw_ostream &OS,
    bool json) {
  if (!tracker) {
    llvm::errs() << "PageAccessTracker is not initialized.\n";
    return false;
  }
  if (json) {
    tracker->printPageAccessedOrderJSON(OS);
  } else {
    tracker->printPageAccessedOrder(OS);
  }
  return true;
}

#endif // not _WINDOWS
