/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include <hermes/ConsoleHost/RuntimeFlags.h>
#include <hermes/Support/MemoryBuffer.h>
#include <hermes/TraceInterpreter.h>
#include <hermes/hermes.h>

#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"

#include <iostream>
#include <tuple>

namespace cl {

using llvm::cl::desc;
using llvm::cl::init;
using llvm::cl::opt;
using llvm::cl::Positional;
using llvm::cl::Required;

/// @name Synth benchmark specific flags
/// @{

static opt<std::string>
    TraceFile(desc("input trace file"), Positional, Required);

static opt<std::string>
    BytecodeFile(desc("input bytecode file"), Positional, Required);

static opt<std::string> Marker("marker", desc("marker to stop at"), init(""));
static llvm::cl::alias
    MarkerA("m", desc("alias for -marker"), llvm::cl::aliasopt(Marker));

static opt<std::string> SnapshotMarker(
    "snapshot-at-marker",
    desc("Take a snapshot at the given marker"),
    init(""));

static opt<std::string> Trace(
    "trace",
    desc(
        "Take a trace of the synthetic benchmark running. Can be used to verify that the replay made the same trace again. Outputs to the file given"),
    init(""));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_STATS)
static opt<bool>
    PrintStats("print-stats", desc("Print statistics"), init(false));
#endif

static opt<unsigned> BytecodeWarmupPercent(
    "bytecode-warmup-percent",
    desc("Eagerly read some bytecode into page cache. May yield faster startup "
         "for large bytecode files."),
    init(0));

static opt<int> Reps(
    "reps",
    desc(
        "Number of repetitions of execution. Any GC stats printed are those for the "
        "rep with the median \"totalTime\"."),
    init(1));

/// @}

/// @name Common flags from Hermes VM
/// @{

static opt<bool> GCAllocYoung(
    "gc-alloc-young",
    desc("Determines whether to (initially) allocate in the young generation"),
    cat(GCCategory),
    init(false));

static opt<bool> GCRevertToYGAtTTI(
    "gc-revert-to-yg-at-tti",
    desc("Determines whether to revert to young generation, if necessary, at "
         "TTI notification"),
    cat(GCCategory),
    init(true));

static opt<bool> GCBeforeStats(
    "gc-before-stats",
    desc("Perform a full GC just before printing statistics at exit"),
    cat(GCCategory),
    init(false));

static opt<bool> GCPrintStats(
    "gc-print-stats",
    desc("Output summary garbage collection statistics at exit"),
    cat(GCCategory),
    init(true));

/// @}

} // namespace cl

int main(int argc, char **argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes synth trace driver\n");

  using namespace facebook::hermes::tracing;
  try {
    TraceInterpreter::ExecuteOptions options;
    options.marker = cl::Marker;
    std::string snapshotMarkerFileName;
    if (!cl::SnapshotMarker.empty()) {
      llvm::SmallVector<char, 16> tmpfile;
      llvm::sys::fs::createTemporaryFile(
          cl::SnapshotMarker, "heapsnapshot", tmpfile);
      snapshotMarkerFileName = std::string{tmpfile.begin(), tmpfile.end()};
      options.snapshotMarker = cl::SnapshotMarker;
      options.snapshotMarkerFileName = snapshotMarkerFileName;
    }
    options.reps = cl::Reps;
    options.minHeapSize = cl::MinHeapSize.bytes;
    options.maxHeapSize = cl::MaxHeapSize.bytes;
    options.occupancyTarget = cl::OccupancyTarget;
    options.allocInYoung = cl::GCAllocYoung;
    options.revertToYGAtTTI = cl::GCRevertToYGAtTTI;
    options.forceGCBeforeStats = cl::GCBeforeStats;
    options.shouldPrintGCStats = cl::GCPrintStats || cl::GCBeforeStats;
    options.shouldTrackIO = cl::TrackBytecodeIO;
    options.bytecodeWarmupPercent = cl::BytecodeWarmupPercent;
    options.sanitizeRate = cl::GCSanitizeRate;
    options.sanitizeRandomSeed = cl::GCSanitizeRandomSeed;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_STATS)
    if (cl::PrintStats)
      llvm::EnableStatistics();
#endif

    if (!cl::Trace.empty()) {
      // If this is tracing mode, get the trace instead of the stats.
      options.shouldPrintGCStats = false;
      options.shouldTrackIO = false;
      std::error_code ec;
      llvm::raw_fd_ostream os(cl::Trace.c_str(), ec, llvm::sys::fs::F_Text);
      if (ec) {
        throw std::system_error(ec);
      }
      TraceInterpreter::execAndTrace(
          cl::TraceFile, cl::BytecodeFile, options, os);
      llvm::outs() << "\nWrote output trace to: " << cl::Trace << "\n";
    } else {
      llvm::outs() << TraceInterpreter::execAndGetStats(
                          cl::TraceFile, cl::BytecodeFile, options)
                   << "\n";
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_STATS)
    if (cl::PrintStats)
      llvm::PrintStatistics(llvm::outs());
#endif
    if (!cl::SnapshotMarker.empty()) {
      llvm::outs() << "Wrote heap snapshot for marker \"" << cl::SnapshotMarker
                   << "\" to " << snapshotMarkerFileName << "\n";
    }
    return 0;
  } catch (const std::invalid_argument &e) {
    std::cerr << "Invalid argument: " << e.what() << std::endl;
  } catch (const std::system_error &e) {
    std::cerr << "System error: " << e.what() << std::endl;
  }
  return 1;
}
