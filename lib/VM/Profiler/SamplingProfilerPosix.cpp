/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Profiler/SamplingProfiler.h"

#ifdef HERMESVM_SAMPLING_PROFILER_USE_POSIX

#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Profiler/ChromeTraceSerializerPosix.h"
#include "hermes/VM/StackFrame-inline.h"

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
#include <profilo/ExternalApi.h>
#endif

namespace hermes {
namespace vm {

/// Name of the semaphore.
const char *const kSamplingDoneSemaphoreName = "/samplingDoneSem";
/*static*/ std::atomic<Runtime *> SamplingProfiler::targetRuntime_;
volatile std::atomic<SamplingProfiler *> SamplingProfiler::sProfilerInstance_{
    nullptr};

void SamplingProfiler::registerRuntime(Runtime *runtime) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);

  // TODO: should we only register runtime when profiler is enabled?
  activeRuntimeThreads_[runtime] = pthread_self();
  targetRuntime_.store(runtime);
  threadLocalRuntime_.set(runtime);
  threadNames_[oscompat::thread_id()] = oscompat::thread_name();
}

void SamplingProfiler::unregisterRuntime(Runtime *runtime) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  bool succeed = activeRuntimeThreads_.erase(runtime);
  // TODO: should we allow recursive style
  // register/register -> unregister/unregister call?
  assert(succeed && "How can runtime not registered yet?");
  (void)succeed;

  targetRuntime_.store(nullptr);
  threadLocalRuntime_.set(nullptr);
}

int SamplingProfiler::invokeSignalAction(void (*handler)(int)) {
  struct sigaction actions;
  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  actions.sa_flags = 0;
  actions.sa_handler = handler;
  return sigaction(SIGPROF, &actions, nullptr);
}

bool SamplingProfiler::registerSignalHandlers() {
  if (isSigHandlerRegistered_) {
    return true;
  }
  if (invokeSignalAction(profilingSignalHandler) != 0) {
    perror("signal handler registration failed");
    return false;
  }
  isSigHandlerRegistered_ = true;
  return true;
}

bool SamplingProfiler::unregisterSignalHandler() {
  if (!isSigHandlerRegistered_) {
    return true;
  }
  // Restore to default.
  if (invokeSignalAction(SIG_DFL) != 0) {
    perror("signal handler unregistration failed");
    return false;
  }
  isSigHandlerRegistered_ = false;
  return true;
}

void SamplingProfiler::profilingSignalHandler(int signo) {
  // Fetch runtime used by this sampling thread.
  auto curThreadRuntime = targetRuntime_.load();
  if (curThreadRuntime == nullptr) {
    // Runtime may have unregistered itself before signal.
    return;
  }
  auto profilerInstance = sProfilerInstance_.load();
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  if (curThreadRuntime->getHeap().heapIsValid()) {
    assert(
        profilerInstance != nullptr &&
        "Why is sProfilerInstance_ not initialized yet?");
    profilerInstance->sampledStackDepth_ = profilerInstance->walkRuntimeStack(
        curThreadRuntime, profilerInstance->sampleStorage_);
  } else {
    // TODO: log "GC in process" meta event.
    profilerInstance->sampledStackDepth_ = 0;
  }
  if (!profilerInstance->samplingDoneSem_.notifyOne()) {
    abort(); // Something is wrong.
  }
}

bool SamplingProfiler::sampleStack() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  // Check profiling stopping request.
  if (!enabled_) {
    return false;
  }

  for (const auto &entry : activeRuntimeThreads_) {
    auto targetThreadId = entry.second;
    // Signal target runtime thread to sample stack.
    pthread_kill(targetThreadId, SIGPROF);

    // Threading: samplingDoneSem_ will synchronize with signal handler to
    // to make sure there will NOT be two SIGPROF signals sent to the same
    // runtime thread at the same time which prevents signal coalescing.
    if (!samplingDoneSem_.wait()) {
      return false;
    }

    if (sampledStackDepth_ > 0) {
      assert(
          sampledStackDepth_ <= sampleStorage_.stack.size() &&
          "How can we sample more frames than storage?");
      sampledStacks_.emplace_back(
          sampleStorage_.tid,
          sampleStorage_.timeStamp,
          sampleStorage_.stack.begin(),
          sampleStorage_.stack.begin() + sampledStackDepth_);
    }

    // Only sample the first thread for now.
    // TODO: support sampling more than all active threads.
    break;
  }
  return true;
}

void SamplingProfiler::timerLoop() {
  while (true) {
    if (!sampleStack()) {
      return;
    }

    // TODO: make sampling rate configurable.
    // TODO: add random fluctuation to interval value.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

uint32_t SamplingProfiler::walkRuntimeStack(
    const Runtime *runtime,
    StackTrace &sampleStorage) {
  unsigned count = 0;

  // TODO: capture leaf frame IP.
  const Inst *ip = nullptr;
  // Whether we successfully captured a stack frame or not.
  bool capturedFrame = true;
  for (ConstStackFramePtr frame : runtime->getStackFrames()) {
    capturedFrame = true;
    auto &frameStorage = sampleStorage.stack[count];
    // Check if it is pure JS frame.
    auto *calleeCodeBlock = frame.getCalleeCodeBlock();
    if (calleeCodeBlock != nullptr) {
      frameStorage.kind = StackFrame::FrameKind::JSFunction;
      frameStorage.jsFrame.functionId = calleeCodeBlock->getFunctionID();
      frameStorage.jsFrame.offset =
          (ip == nullptr ? 0 : calleeCodeBlock->getOffsetOf(ip));
      auto *module = calleeCodeBlock->getRuntimeModule();
      assert(module != nullptr && "Cannot fetch runtimeModule for code block");
      frameStorage.jsFrame.runtimeModuleId = moduleIdManager_.findOrAdd(module);
    } else {
      if (auto *nativeFunction =
              dyn_vmcast_or_null<NativeFunction>(frame.getCalleeClosure())) {
        frameStorage.kind = StackFrame::FrameKind::NativeFunction;
        frameStorage.nativeFrame =
            reinterpret_cast<uintptr_t>(nativeFunction->getFunctionPtr());
      } else {
        // TODO: handle BoundFunction.
        capturedFrame = false;
      }
    }
    // Update ip to caller for next iteration.
    ip = frame.getSavedIP();
    if (capturedFrame) {
      ++count;
      if (count >= sampleStorage.stack.size()) {
        break;
      }
    }
  }
  sampleStorage.tid = oscompat::thread_id();
  sampleStorage.timeStamp = std::chrono::steady_clock::now();
  return count;
}

/*static*/ const std::shared_ptr<SamplingProfiler>
    &SamplingProfiler::getInstance() {
  // Do not use make_shared here because that requires
  // making constructor public.
  static std::shared_ptr<SamplingProfiler> instance(new SamplingProfiler());
  return instance;
}

/*static*/ bool SamplingProfiler::collectStackForLoom(
    ucontext_t *ucontext,
    int64_t *frames,
    uint8_t *depth,
    uint8_t max_depth) {
  auto profilerInstance = sProfilerInstance_.load();
  Runtime *curThreadRuntime = profilerInstance->threadLocalRuntime_.get();
  if (curThreadRuntime == nullptr) {
    // No runtime in this thread.
    return false;
  }
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  uint32_t sampledStackDepth = 0;
  if (curThreadRuntime->getHeap().heapIsValid()) {
    assert(
        profilerInstance != nullptr &&
        "Why is sProfilerInstance_ not initialized yet?");
    sampledStackDepth = profilerInstance->walkRuntimeStack(
        curThreadRuntime, profilerInstance->sampleStorage_);
  } else {
    // TODO: log "GC in process" meta event.
    sampledStackDepth = 0;
  }

  // Loom stack frames are encoded like this:
  //   1. Most significant bit of 64bits address is set for native frame.
  //   2. JS frame: module id in high 32 bits, address virtual offset in lower
  //   32 bits, with MSB unset.
  //   3. Native frame: address returned with MSB set.
  // TODO: enhance this when supporting more frame types.
  sampledStackDepth = std::min(sampledStackDepth, (uint32_t)max_depth);
  for (uint32_t i = 0; i < sampledStackDepth; ++i) {
    constexpr uint64_t kNativeFrameMask = ((uint64_t)1 << 63);
    const StackFrame &stackFrame = profilerInstance->sampleStorage_.stack[i];
    if (stackFrame.kind == StackFrame::FrameKind::JSFunction) {
      ModuleIdManager::ModuleId moduleId = stackFrame.jsFrame.runtimeModuleId;
      OptValue<RuntimeModule *> moduleOpt =
          profilerInstance->moduleIdManager_.getModule(moduleId);
      assert(moduleOpt.hasValue() && "Unable to find module from id.");
      auto *bcProvider = moduleOpt.getValue()->getBytecode();
      uint32_t virtualOffset = bcProvider->getVirtualOffsetForFunction(
                                   stackFrame.jsFrame.functionId) +
          stackFrame.jsFrame.offset;
      uint64_t frameAddress = ((uint64_t)moduleId << 32) + virtualOffset;
      assert(
          (frameAddress & kNativeFrameMask) == 0 &&
          "Module id should take less than 32 bits");
      frames[i] = frameAddress;
    } else if (stackFrame.kind == StackFrame::FrameKind::NativeFunction) {
      frames[i] = ((uint64_t)stackFrame.nativeFrame | kNativeFrameMask);
    } else {
      llvm_unreachable("Unknown frame kind");
    }
  }
  *depth = sampledStackDepth;
  return true;
}

SamplingProfiler::SamplingProfiler() : sampleStorage_(kMaxStackDepth) {
#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  profilo_api()->register_external_tracer_callback(
      TRACER_TYPE_JAVASCRIPT, collectStackForLoom);
#endif
  sProfilerInstance_.store(this);
}

void SamplingProfiler::dumpSampledStack(llvm::raw_ostream &OS) {
  // TODO: serialize to visualizable trace format.
  std::lock_guard<std::mutex> lockGuard(profilerLock_);

  OS << "dumpSamples called from runtime\n";
  OS << "Total " << sampledStacks_.size() << " samples\n";
  for (unsigned i = 0; i < sampledStacks_.size(); ++i) {
    auto &sample = sampledStacks_[i];
    uint64_t timeStamp = sample.timeStamp.time_since_epoch().count();
    OS << "[" << i << "]: tid[" << sample.tid << "], ts[" << timeStamp << "] ";

    // Leaf frame is in sample[0] so dump it backward to
    // get root => leaf represenation.
    for (auto iter = sample.stack.rbegin(); iter != sample.stack.rend();
         ++iter) {
      const StackFrame &frame = *iter;
      if (frame.kind == StackFrame::FrameKind::JSFunction) {
        OS << "[JS]" << frame.jsFrame.functionId << ":" << frame.jsFrame.offset;
      } else if (frame.kind == StackFrame::FrameKind::NativeFunction) {
        OS << "[Native]" << frame.nativeFrame;
      } else {
        llvm_unreachable("Unknown frame kind");
      }
      OS << " => ";
    }
    OS << "\n";
  }
}

void SamplingProfiler::dumpChromeTrace(llvm::raw_ostream &OS) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  auto pid = getpid();
  ChromeTraceSerializer serializer(
      ChromeTraceFormat::create(pid, threadNames_, sampledStacks_));
  serializer.serialize(moduleIdManager_, OS);
  clear();
}

bool SamplingProfiler::enable() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  if (enabled_) {
    return true;
  }
  if (!samplingDoneSem_.open(kSamplingDoneSemaphoreName)) {
    return false;
  }
  if (!registerSignalHandlers()) {
    return false;
  }
  enabled_ = true;
  // Start timer thread.
  std::thread(&SamplingProfiler::timerLoop, this).detach();
  return true;
}

bool SamplingProfiler::disable() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  if (!enabled_) {
    // Already disabled.
    return true;
  }
  if (!samplingDoneSem_.close()) {
    return false;
  }
  // Unregister handlers before shutdown.
  if (!unregisterSignalHandler()) {
    return false;
  }
  // Telling timer thread to exit.
  enabled_ = false;
  return true;
}

bool operator==(
    const SamplingProfiler::StackFrame &left,
    const SamplingProfiler::StackFrame &right) {
  if (left.kind != right.kind) {
    return false;
  }
  if (left.kind == SamplingProfiler::StackFrame::FrameKind::JSFunction) {
    return left.jsFrame.functionId == right.jsFrame.functionId &&
        left.jsFrame.offset == right.jsFrame.offset;
  } else if (
      left.kind == SamplingProfiler::StackFrame::FrameKind::NativeFunction) {
    return left.nativeFrame == right.nativeFrame;
  } else {
    llvm_unreachable("Unknown frame kind");
  }
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_USE_POSIX
