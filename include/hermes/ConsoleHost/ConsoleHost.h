/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CONSOLEHOST_CONSOLEHOST_H
#define HERMES_CONSOLEHOST_CONSOLEHOST_H

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/ConsoleHost/MemorySizeParser.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/instrumentation/StatSamplingThread.h"

#include "llvh/ADT/MapVector.h"

#include <memory>

namespace hermes {

/// Stores state used in certain NativeFunctions in the ConsoleHost.
/// `ConsoleHostContext *` can be passed as the ctx pointer to NativeFunctions.
/// In particular, it stores the job queue which powers setTimeout and friends,
/// which are only available by default in Hermes when using the ConsoleHost.
class ConsoleHostContext {
  /// Queue of jobs that have been added via setTimeout and clearTimeout.
  /// Values are marked by registering a custom roots function with the Runtime.
  llvh::MapVector<uint32_t, vm::Callable *> queuedJobs_{};

  /// Next job ID to be allocated by queueJob.
  uint32_t nextJobId_{1};

 public:
  /// Registers the ConsoleHostContext roots with \p runtime.
  ConsoleHostContext(vm::Runtime *runtime);

  /// \return true when there are no queued jobs remaining.
  bool jobsEmpty() const {
    return queuedJobs_.empty();
  }

  /// Enqueue a job for setTimeout.
  /// \return the ID of the job.
  uint32_t queueJob(vm::PseudoHandle<vm::Callable> job) {
    queuedJobs_.insert({nextJobId_, job.get()});
    job.invalidate();
    return nextJobId_++;
  }

  /// \param id the job to clear from the queue.
  void clearJob(uint32_t id) {
    queuedJobs_.remove_if([id](std::pair<uint32_t, vm::Callable *> it) {
      return it.first == id;
    });
  }

  /// Remove the first job from the queue.
  /// \return the Callable which represents the queued job, None if empty queue.
  llvh::Optional<vm::PseudoHandle<vm::Callable>> dequeueJob() {
    if (queuedJobs_.empty())
      return llvh::None;
    vm::PseudoHandle<vm::Callable> result =
        createPseudoHandle(queuedJobs_.front().second);
    queuedJobs_.erase(queuedJobs_.begin());
    return result;
  }
};

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
    ConsoleHostContext &ctx,
    vm::StatSamplingThread *statSampler = nullptr,
#ifdef HERMESVM_SERIALIZE
    const std::string *serializePath = nullptr,
#endif
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

  /// Execution time limit.
  uint32_t timeLimit{0};

  /// Dump JIT'ed code.
  bool dumpJITCode{false};

  /// Fatally crash on any JIT compilation error.
  bool jitCrashOnError{false};

  /// Perform a full GC just before printing any statistics.
  bool forceGCBeforeStats{false};

  /// Try to execute the same number of CPU instructions
  /// across repeated invocations of the same JS.
  bool stabilizeInstructionCount{false};

  /// Run the sampling profiler.
  bool sampleProfiling{false};

#ifdef HERMESVM_SERIALIZE
  /// Serialize VM state after global object initialization to file.
  std::string SerializeAfterInitFile;

  /// Deserialize VM state from file.
  std::string DeserializeFile;

  /// File to serialize VM state to when serializeVM is called.
  std::string SerializeVMPath;
#endif // HERMESVM_SERIALIZE

  /// Start tracking heap objects before executing bytecode.
  bool heapTimeline{false};
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
