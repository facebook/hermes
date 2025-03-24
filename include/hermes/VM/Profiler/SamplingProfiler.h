/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_SAMPLINGPROFILER_H_
#define HERMES_VM_PROFILER_SAMPLINGPROFILER_H_

#include "hermes/VM/Profiler/SamplingProfilerDefs.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/Public/SamplingProfiler.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/Runtime.h"

#include "llvh/ADT/DenseMap.h"

#include <chrono>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace hermes {
namespace vm {
namespace sampling_profiler {
struct Sampler;
} // namespace sampling_profiler

/// Singleton wall-time based JS sampling profiler that walks VM stack frames
/// in a configurable interval. The profiler can be enabled and disabled
/// on demand.
class SamplingProfiler {
 public:
  using ThreadId = uint64_t;
  using TimeStampType = std::chrono::steady_clock::time_point;
  using ThreadNamesMap =
      llvh::DenseMap<SamplingProfiler::ThreadId, std::string>;

  /// Captured JSFunction stack frame information for symbolication.
  /// TODO: consolidate the stack frame struct with other function/extern
  /// profilers.
  struct JSFunctionFrameInfo {
    // RuntimeModule this function is associated with.
    RuntimeModule *module;
    // Function id associated with current frame.
    uint32_t functionId;
    // IP offset within the function.
    uint32_t offset;
  };
  /// Captured NativeFunction frame information for symbolication.
  using LoomNativeFrameInfo = NativeFunctionPtr;
  /// Captured NativeFunction frame information for symbolication that hasn't
  /// been registered with the sampling profiler yet, and therefore can be moved
  /// by the GC.
  using NativeFunctionFrameInfo = size_t;

  /// Details about a suspend frame.
  struct SuspendFrameInfo {
    /// GC frame info. Pointing to string in gcEventExtraInfoSet_.
    using GCFrameInfo = const std::string *;

    /// Cause of the suspension.
    enum class Kind {
      /// Suspended to perform GC actions. The gcFrame field contains additional
      /// details specific to the GC.
      GC,
      /// Suspended to perform debugger actions.
      Debugger,
      /// Multiple suspensions have occurred.
      Multiple,
    } kind;
    /// Extra information about GC suspensions. Only valid when \c kind is
    /// \c GC. Pointing to string in gcEventExtraInfoSet_; it is optional
    /// and can be null to indicate no extra info.
    /// We can't directly use std::string here because it is
    /// inside a union.
    GCFrameInfo gcFrame;
  };

  // This will break with more than one RuntimeModule(like FB4a, eval() call or
  // lazy compilation etc...). It is simply a temporary thing to get started.
  // Will revisit after figuring out symbolication.
  struct StackFrame {
    /// Kind of frame.
    enum class FrameKind {
      JSFunction,
      NativeFunction,
      FinalizableNativeFunction,
      SuspendFrame,
    };

    // TODO: figure out how to store BoundFunction.
    // TODO: Should we do something special for NativeConstructor?
    union {
      /// Pure JS function frame info.
      JSFunctionFrameInfo jsFrame;
      /// Native function frame info storage used for loom profiling.
      LoomNativeFrameInfo nativeFunctionPtrForLoom;
      /// Native function frame info storage used for "regular" profiling.
      NativeFunctionFrameInfo nativeFrame;
      /// Suspend frame info.
      SuspendFrameInfo suspendFrame;
    };
    FrameKind kind;
  };

  /// Represent stack trace captured by one sampling.
  struct StackTrace {
    /// Id of the thread that this stack trace is taken from.
    ThreadId tid{0};
    /// Timestamp when the stack trace is taken.
    TimeStampType timeStamp;
    /// Captured stack frames, starting from the most recent frame.
    std::vector<StackFrame> stack;

    explicit StackTrace() = default;
    explicit StackTrace(
        ThreadId tid,
        TimeStampType ts,
        const std::vector<StackFrame>::iterator stackStart,
        const std::vector<StackFrame>::iterator stackEnd)
        : tid(tid), timeStamp(ts), stack(stackStart, stackEnd) {}
  };

  enum class MayAllocate { No, Yes };

  /// \return true if this SamplingProfiler belongs to the current running
  /// thread. The current thread can change (e.g. in the time between
  /// this function returning and the caller inspecting the value), so the
  /// usefulness of this method depends upon knowledge of when the runtime
  /// will switch threads.
  bool belongsToCurrentThread();

  /// \returns the NativeFunctionPtr for \p stackFrame. Caller must hold
  /// runtimeDataLock_.
  NativeFunctionPtr getNativeFunctionPtr(const StackFrame &stackFrame) const {
    assert(
        (stackFrame.kind == StackFrame::FrameKind::NativeFunction ||
         stackFrame.kind == StackFrame::FrameKind::FinalizableNativeFunction) &&
        "unexpected stack frame kind");
    return nativeFunctions_[stackFrame.nativeFrame]->getFunctionPtr();
  }

  /// \returns the name (if one exists) for \p stackFrame. Caller must hold
  /// runtimeDataLock_.
  std::string getNativeFunctionName(const StackFrame &stackFrame) const {
    if (stackFrame.kind == StackFrame::FrameKind::NativeFunction) {
      // FrameKing::NativeFunction frames may be JS functions that are internal
      // to hermes -- e.g., Array.prototype.sort. For those functions, return
      // the native function name as defined in NativeFunctions.def.
      const char *name =
          hermes::vm::getFunctionName(getNativeFunctionPtr(stackFrame));
      if (strcmp(name, "")) {
        return name;
      }
    }

    // For all other FrameKind::NativeFunction frames, as well as
    // FrameKing::FinalizableNativeFunction ones, return the native function
    // name attribute, if one is available.
    return nativeFunctions_[stackFrame.nativeFrame]->getNameIfExists(runtime_);
  }

 private:
  friend struct sampling_profiler::Sampler;

  /// Protect data specific to a runtime, such as the sampled stacks,
  /// domains, and thread associated with the runtime.
  std::mutex runtimeDataLock_;

 protected:
  /// Sampled stack traces overtime. Protected by runtimeDataLock_.
  std::vector<StackTrace> sampledStacks_;

 private:
  // Threading: the suspendCount/preSuspendStack are accessed by both the VM
  // thread as well as the sampling profiler timer thread, hence they are all
  // protected by runtimeDataLock_.
  /// The counter of how many suspend calls are pending -- i.e., need to be
  /// resume()d.
  uint32_t suspendCount_{0};
  /// JS stack captured at time of GC.
  StackTrace preSuspendStackStorage_{};

  /// Prellocated map that contains thread names mapping.
  ThreadNamesMap threadNames_;

  /// Thread ID of the currently registered thread.
  ThreadId threadID_;

  /// Unique GC event extra info strings container.
  std::unordered_set<std::string> gcEventExtraInfoSet_;

  /// Domains to be kept alive for sampled RuntimeModules. Protected by
  /// runtimeDataLock_.
  std::vector<Domain *> domains_;

  /// NativeFunctions to be kept alive for sampled NativeFunctionFrameInfo.
  /// Protected by runtimeDataLock_.
  std::vector<NativeFunction *> nativeFunctions_;

 protected:
  Runtime &runtime_;

  /// \return suspendCount_ != 0, meaning that an external agent (e.g., GC) has
  /// suspended stack walking on this sampling profiler.
  bool isSuspended() const {
    return suspendCount_ != 0;
  }

 private:
  /// Hold \p domain so that the RuntimeModule(s) used by profiler are not
  /// released during symbolication.
  /// Refer to Domain.h for relationship between Domain and RuntimeModule.
  void registerDomain(Domain *domain);

  /// Hold \p nativeFunction so native function names can be added to the stack
  /// traces.
  NativeFunctionFrameInfo registerNativeFunction(
      NativeFunction *nativeFunction);

 protected:
  enum class InLoom { No, Yes };

  /// Walk runtime stack frames and store in \p sampleStorage.
  /// When this function is being invoked from signal handler, it should obey
  /// all rules of signal handler(no lock, no memory allocation etc...)
  /// \param sampleStorage references the buffer where stack frames will be
  /// stored.
  /// \param inLoom specifies this function is being invoked in a Loom callback.
  /// \param mayAllocate specifies whether this function may allocate memory.
  /// \return the number of frames that aren't stored in \p sampleStorage
  /// due to insufficient space.
  uint32_t walkRuntimeStack(
      StackTrace &sampleStorage,
      InLoom inLoom,
      MayAllocate mayAllocate);

 private:
  /// Record JS stack at time of suspension, caller must hold
  /// runtimeDataLock_.
  void recordPreSuspendStack(
      SuspendFrameInfo::Kind reason,
      llvh::StringRef gcFrame);

 protected:
  /// Clear previous stored samples.
  /// Note: caller should take the lock before calling.
  void clear();

 public:
  static std::unique_ptr<SamplingProfiler> create(Runtime &rt);

  virtual ~SamplingProfiler() = default;

  /// See documentation on \c GCBase::GCCallbacks.
  void markRootsForCompleteMarking(RootAcceptor &acceptor);

  /// Mark roots that are kept alive by the SamplingProfiler.
  void markRoots(RootAcceptor &acceptor);

  /// Dump sampled stack to \p OS.
  /// NOTE: this is for manual testing purpose.
  void dumpSampledStack(llvh::raw_ostream &OS);

  /// Dump sampled stack to \p OS in tracery trace format.
  void dumpTraceryTrace(llvh::raw_ostream &OS);

  /// Dump sampled stack trace to data structure that can be used by third
  /// parties.
  facebook::hermes::sampling_profiler::Profile dumpAsProfile();

  /// Dump the sampled stack to \p OS in the format expected by the
  /// Profiler.stop return type. See
  ///
  /// https://chromedevtools.github.io/devtools-protocol/tot/Profiler/#type-Profile
  ///
  /// for a description.
  void dumpChromeTrace(llvh::raw_ostream &OS);

  /// Static wrapper for dumpSampledStack.
  static void dumpSampledStackGlobal(llvh::raw_ostream &OS);

  /// Static wrapper for dumpTraceryTrace.
  static void dumpTraceryTraceGlobal(llvh::raw_ostream &OS);

  /// Enable and start profiling.
  static bool enable(double meanHzFreq = 100);

  /// Disable and stop profiling.
  static bool disable();

  /// Suspends the sample profiling. Every call to suspend must be matched by a
  /// call to resume. \c reason indicates the cause of suspension. If \c reason
  /// is GC, \c gcFrame can provide extra details, otherwise it is ignored.
  void suspend(
      SuspendFrameInfo::Kind reason,
      llvh::StringRef gcSuspendDetails = "");

  /// Resumes the sample profiling. There must have been a previous call to
  /// suspend() that hansn't been resume()d yet.
  void resume();

  /// Inform the sampling profiler that the runtime will now be executing
  /// bytecode on the current thread.
  void setRuntimeThread();

 protected:
  explicit SamplingProfiler(Runtime &runtime);
};

/// An RAII class for temporarily suspending (and auto-resuming) the sampling
/// profiler.
class SuspendSamplingProfilerRAII {
 public:
  explicit SuspendSamplingProfilerRAII(
      Runtime &runtime,
      SamplingProfiler::SuspendFrameInfo::Kind reason)
      : profiler_(runtime.samplingProfiler.get()) {
    if (profiler_) {
      profiler_->suspend(reason);
    }
  }

  ~SuspendSamplingProfilerRAII() {
    if (profiler_) {
      profiler_->resume();
    }
  }

 private:
  SamplingProfiler *profiler_;
};

bool operator==(
    const SamplingProfiler::StackFrame &left,
    const SamplingProfiler::StackFrame &right);

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

#endif // HERMES_VM_PROFILER_SAMPLINGPROFILER_H_
