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
/// In particular, it stores the task queue which powers setTimeout and friends,
/// which are only available by default in Hermes when using the ConsoleHost.
class ConsoleHostContext {
  /// The task queue to implement a minimalistic event loop. The only type of
  /// task at this moment is timer task added via setTimeout and friends.
  /// Values are marked by registering a custom roots function with the Runtime.
  llvh::MapVector<uint32_t, vm::Callable *> taskQueue_{};

  /// Next task ID to be allocated by queueTask.
  uint32_t nextTaskId_{1};

 public:
  /// Registers the ConsoleHostContext roots with \p runtime.
  ConsoleHostContext(vm::Runtime *runtime);

  /// \return true when there are no queued tasks remaining.
  bool tasksEmpty() const {
    return taskQueue_.empty();
  }

  /// Enqueue a task.
  /// \return the ID of the task.
  uint32_t queueTask(vm::PseudoHandle<vm::Callable> task) {
    taskQueue_.insert({nextTaskId_, task.get()});
    task.invalidate();
    return nextTaskId_++;
  }

  /// \param id the task to clear from the queue.
  void clearTask(uint32_t id) {
    taskQueue_.remove_if([id](std::pair<uint32_t, vm::Callable *> it) {
      return it.first == id;
    });
  }

  /// Remove the first task from the queue.
  /// \return the Callable that represents the queued task, None if empty queue.
  llvh::Optional<vm::PseudoHandle<vm::Callable>> dequeueTask() {
    if (taskQueue_.empty())
      return llvh::None;
    vm::PseudoHandle<vm::Callable> result =
        createPseudoHandle(taskQueue_.front().second);
    taskQueue_.erase(taskQueue_.begin());
    return result;
  }
};

/// Microtask checkpoint.
/// https://html.spec.whatwg.org/C#perform-a-microtask-checkpoint
///
/// The algorithm needs to exhaust the event loop's microtask queue. Since the
/// only type of microtask in ConsoleHost environments is ECMAScript Job, It
/// is implemented by draining the job queue of \p runtime.
/// It needs to be performed as steps of 8.1.4.4 Calling scripts and 8.1.6.3
/// Process model (event loop), notably when the JS call stack is emptied.
namespace microtask {

/// Complete a checkpoint by repetitively trying to drain the engine job queue
/// until there was no errors (implying queue exhaustiveness).
/// Note that exceptions are directly printed to stderr.
inline void performCheckpoint(vm::Runtime *runtime) {
  if (!runtime->useJobQueue())
    return;

  while (
      LLVM_UNLIKELY(runtime->drainJobs() == vm::ExecutionStatus::EXCEPTION)) {
    runtime->printException(
        llvh::errs(), runtime->makeHandle(runtime->getThrownValue()));
  };
}

} // namespace microtask

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
