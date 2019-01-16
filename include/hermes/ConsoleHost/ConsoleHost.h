#ifndef HERMES_CONSOLEHOST_CONSOLEHOST_H
#define HERMES_CONSOLEHOST_CONSOLEHOST_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/ConsoleHost/MemorySizeParser.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/instrumentation/StatSamplingThread.h"

#include <memory>

namespace hermes {

namespace vm {
class Runtime;
}

/// Installs console host functions in Runtime \p runtime.
/// Host functions installed:
///   - quit()
///   - createHeapSnapshot(filename, compact = true)
///
/// \p gcPrintStats Indicates whether or not a call to quit() should result in
///     the GC stats being output.
/// \p statSampler A pointer to a thread managing the collection of process-wide
///     statistics.  If this is non-null, and gcPrintStats is true, the thread
///     will be stopped, and the process stats printed if quit is called.  If it
///     is null, this does not happen.
void installConsoleBindings(
    vm::Runtime *runtime,
    bool gcPrintStats,
    vm::StatSamplingThread *statSampler = nullptr);

/// Options for executing an HBC bundle.
struct ExecuteOptions {
  // Configuration options for the Garbage Collector.
  vm::RuntimeConfig runtimeConfig;

  /// Enable basic block profiling.
  bool basicBlockProfiling{false};

  /// Stop after creating the RuntimeModule.
  bool stopAfterInit{false};

#ifdef HERMESVM_PROFILER_EXTERN
  /// Patch the symbols so that the external profiler can be used.
  bool patchProfilerSymbols{false};

  /// Dump the symbols in given file name.
  std::string profilerSymbolsFile;
#endif

  /// Dump JIT'ed code.
  bool dumpJITCode{false};

  /// Fatally crash on any JIT compilation error.
  bool jitCrashOnError{false};
};

/// Executes the HBC bytecode provided in HermesVM.
/// \return true if the execution completed successfully, false on JS exception.
bool executeHBCBytecode(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    const ExecuteOptions &options);

} // namespace hermes

#endif
