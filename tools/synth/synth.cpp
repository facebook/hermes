#ifdef HERMESVM_SYNTH_REPLAY
// Only run this file with replay mode on

#include <hermes/Support/MemoryBuffer.h>
#include <hermes/TraceInterpreter.h>
#include <hermes/hermes.h>

#include "hermes/ConsoleHost/MemorySizeParser.h"
#include "llvm/Support/CommandLine.h"

#include <iostream>
#include <tuple>

namespace {

using llvm::cl::desc;
using llvm::cl::init;
using llvm::cl::opt;
using llvm::cl::Positional;
using llvm::cl::Required;

static opt<std::string>
    TraceFile(desc("input trace file"), Positional, Required);

static opt<std::string>
    BytecodeFile(desc("input bytecode file"), Positional, Required);

static opt<std::string> Marker("marker", desc("marker to stop at"), init(""));
static llvm::cl::alias
    MarkerA("m", desc("alias for -marker"), llvm::cl::aliasopt(Marker));

static opt<cl::MemorySize, false, cl::MemorySizeParser> MinHeap(
    "gc-min-heap",
    desc("Minimum heap size.  Format: <unsigned>{{K,M,G}{iB}"),
    init(cl::MemorySize{0}));

static opt<cl::MemorySize, false, cl::MemorySizeParser> MaxHeap(
    "gc-max-heap",
    desc("Maximum heap size.  Format: <unsigned>{{K,M,G}{iB}"),
    init(cl::MemorySize{0}));

static opt<bool> GCPrintStats(
    "gc-print-stats",
    desc("Print GC stats. On by default."),
    init(true));

static opt<bool> TrackBytecodeIO(
    "track-io",
    desc("Track bytecode I/O during execution, and print stats."),
    init(false));

static opt<bool> GCAllocYoung(
    "gc-alloc-young",
    desc("Determines whether to (initially) allocate in the young generation"),
    init(true));

static opt<bool> GCRevertToYGAtTTI(
    "gc-revert-to-yg-at-tti",
    desc("Determines whether to revert to young generation, if necessary, at "
         "TTI notification"),
    init(false));

static opt<unsigned> BytecodeWarmupPercent(
    "bytecode-warmup-percent",
    desc("Eagerly read some bytecode into page cache. May yield faster startup "
         "for large bytecode files."),
    init(0));

} // namespace

int main(int argc, char **argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv, "Hermes synth trace driver\n");

  using namespace facebook::hermes;
  try {
    TraceInterpreter::ExecuteOptions options;
    options.marker = Marker;
    options.minHeapSize = MinHeap.bytes;
    options.maxHeapSize = MaxHeap.bytes;
    options.allocInYoung = GCAllocYoung;
    options.revertToYGAtTTI = GCRevertToYGAtTTI;
    options.shouldPrintGCStats = GCPrintStats;
    options.shouldTrackIO = TrackBytecodeIO;
    options.bytecodeWarmupPercent = BytecodeWarmupPercent;
#ifdef HERMESVM_API_TRACE
    // If this is tracing mode, get the trace instead of the stats.
    if (!options.marker.empty()) {
      throw std::invalid_argument(
          "Shouldn't be given a marker for trace comparisons");
    }
    options.shouldPrintGCStats = false;
    options.shouldTrackIO = false;
    TraceInterpreter::execAndTrace(
        TraceFile, BytecodeFile, options, llvm::outs());
    llvm::outs() << "\n";
#else
    llvm::outs() << TraceInterpreter::execAndGetStats(
                        TraceFile, BytecodeFile, options)
                 << "\n";
#endif
    return 0;
  } catch (const std::invalid_argument &e) {
    std::cerr << "Invalid argument: " << e.what() << std::endl;
  } catch (const std::system_error &e) {
    std::cerr << "System error: " << e.what() << std::endl;
  }
  return 1;
}

#else

#include <iostream>

int main() {
  std::cerr << "Build this with @fbsource//xplat/mode/hermes/synth instead"
            << std::endl;
  return 1;
}
#endif
