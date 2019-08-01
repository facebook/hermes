/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_CONSOLEHOST_CONSOLEHOST_H
#define HERMES_CONSOLEHOST_CONSOLEHOST_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/CompilerDriver/CompilerDriver.h"
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
/// \p statSampler A pointer to a thread managing the collection of process-wide
///     statistics.  If this is non-null, and gcPrintStats is true, the thread
///     will be stopped, and the process stats printed if quit is called.  If it
///     is null, this does not happen.
/// \p filename If non-null, the filename of the BC buffer being loaded.
///    Used to find the other segments to be loaded at runtime.
void installConsoleBindings(
    vm::Runtime *runtime,
    vm::StatSamplingThread *statSampler = nullptr,
    const std::string *filename = nullptr);

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

  /// Exectuion time limit.
  uint32_t timeLimit{0};

  /// Dump JIT'ed code.
  bool dumpJITCode{false};

  /// Fatally crash on any JIT compilation error.
  bool jitCrashOnError{false};

  /// Perform a full GC just before printing any statistics.
  bool forceGCBeforeStats{false};
};

/// Executes the HBC bytecode provided in HermesVM.
/// \param filename if non-null the file name for the bytecode file.
/// \return true if the execution completed successfully, false on JS exception.
bool executeHBCBytecode(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    const ExecuteOptions &options,
    const std::string *filename);

} // namespace hermes

#endif
