/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"
#include "hermes/VM/GC.h"

#include "hermes/Platform/Logging.h"
#include "hermes/Support/ErrorHandling.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/VTable.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <inttypes.h>
#include <system_error>

using llvm::dbgs;
using llvm::format;

namespace hermes {
namespace vm {

void GCBase::runtimeWillExecute() {
  if (recordGcStats_) {
    execStartTime_ = std::chrono::steady_clock::now();
    execStartCPUTime_ = oscompat::thread_cpu_time();
  }
}

bool GCBase::createSnapshotToFile(const std::string &fileName, bool compact) {
  std::error_code code;
  llvm::raw_fd_ostream os(fileName, code, llvm::sys::fs::OpenFlags::F_RW);
  if (code) {
    return false;
  }
  createSnapshot(os, compact);
  return true;
}

void GCBase::checkTripwire(
    size_t dataSize,
    std::chrono::time_point<std::chrono::steady_clock> now) {
  if (LLVM_LIKELY(!tripwireCallback_) ||
      LLVM_LIKELY(dataSize < tripwireLimit_) ||
      LLVM_LIKELY(now < nextTripwireMinTime_)) {
    return;
  }

  class Ctx : public GCTripwireContext {
   public:
    Ctx(GCBase *gc) : gc_(gc) {}

    bool createSnapshotToFile(const std::string &path, bool compact) override {
      return gc_->createSnapshotToFile(path, compact);
    }

   private:
    GCBase *gc_;
  } ctx(this);

  nextTripwireMinTime_ = std::chrono::steady_clock::time_point::max();
  tripwireCallback_(ctx);
  nextTripwireMinTime_ = now + tripwireCooldown_;
}

void GCBase::printAllCollectedStats(llvm::raw_ostream &os) {
  if (!recordGcStats_)
    return;

  dump(os);
  os << "GC stats:\n"
     << "{\n"
     << "\t\"type\": \"hermes\",\n"
     << "\t\"version\": 0,\n";
  printStats(os, false);
  os << "}\n";
}

void GCBase::getHeapInfo(HeapInfo &info) {
  info.numCollections = cumStats_.numCollections;
}

#ifndef NDEBUG
void GCBase::getDebugHeapInfo(DebugHeapInfo &info) {
  recordNumAllocatedObjects();
  info.numAllocatedObjects = numAllocatedObjects_;
  info.numReachableObjects = numReachableObjects_;
  info.numCollectedObjects = numCollectedObjects_;
  info.numFinalizedObjects = numFinalizedObjects_;
  info.numMarkedSymbols = numMarkedSymbols_;
  info.numHiddenClasses = numHiddenClasses_;
  info.numLeafHiddenClasses = numLeafHiddenClasses_;
}
#endif

#ifndef NDEBUG
void GCBase::DebugHeapInfo::assertInvariants() const {
  // The number of allocated objects at any time is at least the number
  // found reachable in the last collection.
  assert(numAllocatedObjects >= numReachableObjects);
  // The number of objects finalized in the last collection is at most the
  // number of objects collected.
  assert(numCollectedObjects >= numFinalizedObjects);
}
#endif

void GCBase::dump(llvm::raw_ostream &, bool) { /* nop */
}

void GCBase::printStats(llvm::raw_ostream &os, bool trailingComma) {
  std::chrono::duration<double> elapsedTime =
      std::chrono::steady_clock::now() - execStartTime_;
  auto elapsedCPUSeconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
          oscompat::thread_cpu_time())
          .count() -
      std::chrono::duration_cast<std::chrono::duration<double>>(
          execStartCPUTime_)
          .count();

  HeapInfo info;
  getHeapInfoWithMallocSize(info);
  getHeapInfo(info);
#ifndef NDEBUG
  DebugHeapInfo debugInfo;
  getDebugHeapInfo(debugInfo);
#endif

  os << "\t\"heapInfo\": {\n"
#ifndef NDEBUG
     << "\t\t\"Num allocated cells\": " << debugInfo.numAllocatedObjects
     << ",\n"
     << "\t\t\"Num reachable cells\": " << debugInfo.numReachableObjects
     << ",\n"
     << "\t\t\"Num collected cells\": " << debugInfo.numCollectedObjects
     << ",\n"
     << "\t\t\"Num finalized cells\": " << debugInfo.numFinalizedObjects
     << ",\n"
     << "\t\t\"Num marked symbols\": " << debugInfo.numMarkedSymbols << ",\n"
     << "\t\t\"Num hidden classes\": " << debugInfo.numHiddenClasses << ",\n"
     << "\t\t\"Num leaf classes\": " << debugInfo.numLeafHiddenClasses << ",\n"
     << "\t\t\"Num weak references\": " << ((GC *)this)->countUsedWeakRefs()
     << ",\n"
#endif
     << "\t\t\"Peak RSS\": " << oscompat::peak_rss() << ",\n"
     << "\t\t\"Heap size\": " << info.heapSize << ",\n"
     << "\t\t\"Allocated bytes\": " << info.allocatedBytes << ",\n"
     << "\t\t\"Num collections\": " << info.numCollections << ",\n"
     << "\t\t\"Malloc size\": " << info.mallocSizeEstimate << "\n"
     << "\t},\n";

  os << "\t\"general\": {\n"
     << "\t\t\"numCollections\": " << cumStats_.numCollections << ",\n"
     << "\t\t\"totalTime\": " << elapsedTime.count() << ",\n"
     << "\t\t\"totalCPUTime\": " << elapsedCPUSeconds << ",\n"
     << "\t\t\"totalGCTime\": " << formatSecs(cumStats_.gcWallTime.sum()).secs
     << ",\n"
     << "\t\t\"avgGCPause\": " << formatSecs(cumStats_.avgGCWallTime()).secs
     << ",\n"
     << "\t\t\"maxGCPause\": " << formatSecs(cumStats_.gcWallTime.max()).secs
     << ",\n"
     << "\t\t\"totalGCCPUTime\": " << formatSecs(cumStats_.gcCPUTime.sum()).secs
     << ",\n"
     << "\t\t\"avgGCCPUPause\": " << formatSecs(cumStats_.avgGCCPUTime()).secs
     << ",\n"
     << "\t\t\"maxGCCPUPause\": " << formatSecs(cumStats_.gcCPUTime.max()).secs
     << ",\n"
     << "\t\t\"finalHeapSize\": " << formatSize(cumStats_.finalHeapSize).bytes
     << ",\n"
     << "\t\t\"totalAllocatedBytes\": "
     << formatSize(info.totalAllocatedBytes).bytes << "\n"
     << "\t}";

  if (trailingComma) {
    os << ",";
  }
  os << "\n";
}

void GCBase::recordGCStats(
    double wallTime,
    double cpuTime,
    gcheapsize_t finalHeapSize,
    CumulativeHeapStats *stats) {
  stats->gcWallTime.record(wallTime);
  stats->gcCPUTime.record(cpuTime);
  stats->finalHeapSize = finalHeapSize;
  stats->numCollections++;
}

void GCBase::recordGCStats(
    double wallTime,
    double cpuTime,
    gcheapsize_t finalHeapSize) {
  recordGCStats(wallTime, cpuTime, finalHeapSize, &cumStats_);
}

#ifdef HERMESVM_GCCELL_ID
uint64_t GCBase::nextObjectID() {
#ifdef HERMESVM_API_TRACE
  // This must be unique in order for the synthetic benchmarks to work, check
  // for overflow and fail fast.
  if (LLVM_UNLIKELY(
          debugAllocationCounter_ == std::numeric_limits<uint64_t>::max())) {
    hermes_fatal(
        "Overflowed the allocation counter in a synthetic benchmark trace");
  }
#endif
  return debugAllocationCounter_++;
}
#endif

void GCBase::oom() {
  oomDetail();
  hermes_fatal("OOM");
}

void GCBase::oomDetail() {
  HeapInfo heapInfo;
  getHeapInfo(heapInfo);
  // Could use a stringstream here, but want to avoid dynamic allocation.
  char detailBuffer[200];
  snprintf(
      detailBuffer,
      sizeof(detailBuffer),
      "[%.20s] numCollections = %d, heapSize = %d, allocated = %d, va = %" PRIu64,
      name_.c_str(),
      heapInfo.numCollections,
      heapInfo.heapSize,
      heapInfo.allocatedBytes,
      heapInfo.va);
  hermesLog("HermesGC", "OOM: %s.", detailBuffer);
  // Record the OOM custom data with the crash manager.
  crashMgr_->setCustomData("HermesGCOOMDetailBasic", detailBuffer);
}

#ifdef HERMESVM_SANITIZE_HANDLES
bool GCBase::shouldSanitizeHandles() {
  static std::uniform_real_distribution<> dist(0.0, 1.0);
  return dist(randomEngine_) < sanitizeRate_;
}
#endif

/*static*/
double GCBase::clockDiffSeconds(TimePoint start, TimePoint end) {
  std::chrono::duration<double> elapsed = (end - start);
  return elapsed.count();
}

/*static*/
double GCBase::clockDiffSeconds(
    std::chrono::microseconds start,
    std::chrono::microseconds end) {
  std::chrono::duration<double> elapsed = (end - start);
  return elapsed.count();
}

llvm::raw_ostream &operator<<(
    llvm::raw_ostream &os,
    const DurationFormatObj &dfo) {
  if (dfo.secs >= 1.0) {
    os << format("%5.3f", dfo.secs) << " s";
  } else if (dfo.secs >= 0.001) {
    os << format("%5.3f", dfo.secs * 1000.0) << " ms";
  } else {
    os << format("%5.3f", dfo.secs * 1000000.0) << " us";
  }
  return os;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const SizeFormatObj &sfo) {
  double dblsize = static_cast<double>(sfo.bytes);
  if (sfo.bytes >= (1024 * 1024 * 1024)) {
    double gbs = dblsize / (1024.0 * 1024.0 * 1024.0);
    os << format("%0.3f GiB", gbs);
  } else if (sfo.bytes >= (1024 * 1024)) {
    double mbs = dblsize / (1024.0 * 1024.0);
    os << format("%0.3f MiB", mbs);
  } else if (sfo.bytes >= 1024) {
    double kbs = dblsize / 1024.0;
    os << format("%0.3f KiB", kbs);
  } else {
    os << sfo.bytes << " B";
  }
  return os;
}

GCBase::GCCallbacks::~GCCallbacks() {}

} // namespace vm
} // namespace hermes
