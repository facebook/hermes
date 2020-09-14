/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#ifdef HERMESVM_SAMPLING_PROFILER_POSIX

#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Profiler/ChromeTraceSerializerPosix.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <random>
#include <thread>

namespace hermes {
namespace vm {

/// Name of the semaphore.
const char *const kSamplingDoneSemaphoreName = "/samplingDoneSem";
/// Maximum allowed GC event extra info count.
constexpr uint32_t kMaxGCEventExtraInfoCount = 10;

volatile std::atomic<SamplingProfiler *> SamplingProfiler::sProfilerInstance_{
    nullptr};

void SamplingProfiler::registerRuntime(Runtime *runtime) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);

  // TODO: should we only register runtime when profiler is enabled?
  activeRuntimeThreads_[runtime] = pthread_self();
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

  threadLocalRuntime_.set(nullptr);
}

void SamplingProfiler::increaseDomainCount() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  // Reserve an empty slot. Use push_back to get exponential capacity expansion.
  domains_.push_back(nullptr);
}

void SamplingProfiler::decreaseDomainCount() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  // Shrink domains_ because a Domain has been destroyed.
  // The destroyed domain should not be held by SamplingProfiler so
  // there must have a corresponding reserved empty slot in domains_. Since
  // domains_ is used from front the last slot in domains_ should be null.
  assert(!domains_.empty() && "Why there is no domain?");
  assert(
      domains_.back() == nullptr &&
      "The destroyed domain should not be referenced.");
  domains_.pop_back();
}

void SamplingProfiler::registerDomain(Domain *domain) {
  // If domain is already registered do nothing, otherwise
  // store domain in the first unused/empty slot.
  // Invariant: domains_ are always filled from the front and the whole content
  // are destroyed at once so the registered/used domains are always at the
  // beginning of domains_.
  for (size_t i = 0, e = domains_.size(); i < e; ++i) {
    if (domains_[i] == domain) {
      // Already registered.
      return;
    } else if (domains_[i] == nullptr) {
      // Not registered before, fill in the first reserved empty slot.
      domains_[i] = domain;
      return;
    }
  }
  llvm_unreachable("Cannot find a reserved null domain slot.");
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
  // Avoid spoiling errno in a signal handler by storing the old version and
  // re-assigning it.
  auto oldErrno = errno;
  // Fetch runtime used by this sampling thread.
  auto profilerInstance = sProfilerInstance_.load();
  Runtime *curThreadRuntime = profilerInstance->threadLocalRuntime_.get();
  if (curThreadRuntime == nullptr) {
    // Runtime may have unregistered itself before signal.
    errno = oldErrno;
    return;
  }
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  if (LLVM_LIKELY(!curThreadRuntime->getHeap().inGC())) {
    assert(
        profilerInstance != nullptr &&
        "Why is sProfilerInstance_ not initialized yet?");
    profilerInstance->sampledStackDepth_ = profilerInstance->walkRuntimeStack(
        curThreadRuntime, profilerInstance->sampleStorage_);
  } else {
    // GC in process. Copy pre-captured stack instead.
    if (profilerInstance->preGCStackDepth_ > 0) {
      profilerInstance->sampleStorage_ = profilerInstance->preGCStackStorage_;
      profilerInstance->sampledStackDepth_ = profilerInstance->preGCStackDepth_;
    } else {
      // This GC (like mallocGC) did not record JS stack.
      // TODO: fix this for all GCs.
      profilerInstance->sampledStackDepth_ = 0;
    }
  }
  if (!profilerInstance->samplingDoneSem_.notifyOne()) {
    errno = oldErrno;
    abort(); // Something is wrong.
  }
  errno = oldErrno;
}

bool SamplingProfiler::sampleStack(std::unique_lock<std::mutex> &uniqueLock) {
  // Check profiling stopping request.
  if (!enabled_) {
    return false;
  }

  // Make a copy of activeRuntimeThreads_ because it may be modified by
  // runtime threads during enumeration.
  auto activeRuntimeThreadsCopy = activeRuntimeThreads_;
  for (const auto &entry : activeRuntimeThreadsCopy) {
    auto targetThreadId = entry.second;
    // Signal target runtime thread to sample stack.
    pthread_kill(targetThreadId, SIGPROF);

    // Threading: samplingDoneSem_ will synchronize with signal handler to
    // to make sure there will NOT be two SIGPROF signals sent to the same
    // runtime thread at the same time which prevents signal coalescing.
    // Also, release profilerLock_ before waiting because the runtime thread
    // we try to signal may be trying to hold profilerLock_. Failing to unlock
    // profilerLock_ may cause deadlock: timer thread waiting on
    // samplingDoneSem_ while target runtime thread is waiting on timer thread
    // to release profilerLock_.
    uniqueLock.unlock();
    if (!samplingDoneSem_.wait()) {
      return false;
    }
    // Reacquire profilerLock_ to protect the access to fields of
    // SamplingProfiler.
    // sampledStacks_/sampledStacks_/sampleStorage_ fields may be
    // modified during the unlock peroid which is fine because,
    // unlike activeRuntimeThreads_, their read/write operations
    // do not overlap each other across unlock/lock boundary.
    uniqueLock.lock();
    // Enabled may have changed since the lock was released, and we should exit
    // as early as possible before sending more signals to threads that no
    // longer have signal handlers attached.
    if (!enabled_) {
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
  oscompat::set_thread_name("hermes-sampling-profiler");

  constexpr double kMeanMilliseconds = 10;
  constexpr double kStdDevMilliseconds = 5;
  std::random_device rd{};
  std::mt19937 gen{rd()};
  // The amount of time that is spent sleeping comes from a normal distribution,
  // to avoid the case where the timer thread samples a stack at a predictable
  // period.
  std::normal_distribution<> distribution{kMeanMilliseconds,
                                          kStdDevMilliseconds};
  std::unique_lock<std::mutex> uniqueLock(profilerLock_);

  while (true) {
    if (!sampleStack(uniqueLock)) {
      return;
    }

    const uint64_t millis = round(std::fabs(distribution(gen)));
    // TODO: make sampling rate configurable.
    bool disabled = enabledCondVar_.wait_for(
        uniqueLock, std::chrono::milliseconds(millis), [this]() {
          return !enabled_;
        });
    if (disabled) {
      // The sampling profiler was disabled: don't continue sampling.
      return;
    }
    // Else there was a timeout, which means enabled_ wasn't changed while this
    // function was waiting. Continue to sample the stack.
  }
}

uint32_t SamplingProfiler::walkRuntimeStack(
    Runtime *runtime,
    StackTrace &sampleStorage,
    uint32_t startIndex) {
  unsigned count = startIndex;

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
      frameStorage.jsFrame.module = module;
      registerDomain(module->getDomainUnsafe(runtime));
    } else {
      if (auto *nativeFunction =
              dyn_vmcast_or_null<NativeFunction>(frame.getCalleeClosure())) {
        frameStorage.kind = vmisa<FinalizableNativeFunction>(nativeFunction)
            ? StackFrame::FrameKind::FinalizableNativeFunction
            : StackFrame::FrameKind::NativeFunction;
        frameStorage.nativeFrame = nativeFunction->getFunctionPtr();
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

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
/*static*/ StackCollectionRetcode SamplingProfiler::collectStackForLoom(
    ucontext_t *ucontext,
    int64_t *frames,
    uint16_t *depth,
    uint16_t max_depth) {
  auto profilerInstance = sProfilerInstance_.load();
  Runtime *curThreadRuntime = profilerInstance->threadLocalRuntime_.get();
  if (curThreadRuntime == nullptr) {
    // No runtime in this thread.
    return StackCollectionRetcode::NO_STACK_FOR_THREAD;
  }
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  uint32_t sampledStackDepth = 0;
  if (!curThreadRuntime->getHeap().inGC()) {
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
  //   3. Native/Finalizable frame: address returned with MSB set.
  // TODO: enhance this when supporting more frame types.
  sampledStackDepth = std::min(sampledStackDepth, (uint32_t)max_depth);
  for (uint32_t i = 0; i < sampledStackDepth; ++i) {
    constexpr uint64_t kNativeFrameMask = ((uint64_t)1 << 63);
    const StackFrame &stackFrame = profilerInstance->sampleStorage_.stack[i];
    switch (stackFrame.kind) {
      case StackFrame::FrameKind::JSFunction: {
        auto *bcProvider = stackFrame.jsFrame.module->getBytecode();
        uint32_t virtualOffset = bcProvider->getVirtualOffsetForFunction(
                                     stackFrame.jsFrame.functionId) +
            stackFrame.jsFrame.offset;

        uint32_t moduleId = bcProvider->getCJSModuleOffset();
        uint64_t frameAddress = ((uint64_t)moduleId << 32) + virtualOffset;
        assert(
            (frameAddress & kNativeFrameMask) == 0 &&
            "Module id should take less than 32 bits");
        frames[i] = frameAddress;
        break;
      }

      case StackFrame::FrameKind::NativeFunction:
        frames[i] = ((uint64_t)stackFrame.nativeFrame | kNativeFrameMask);
        break;

      case StackFrame::FrameKind::FinalizableNativeFunction:
        frames[i] =
            ((uint64_t)stackFrame.finalizableNativeFrame | kNativeFrameMask);
        break;

      default:
        llvm_unreachable("Loom: unknown frame kind");
    }
  }
  *depth = sampledStackDepth;
  if (*depth == 0) {
    return StackCollectionRetcode::EMPTY_STACK;
  }
  return StackCollectionRetcode::SUCCESS;
}
#endif

SamplingProfiler::SamplingProfiler() : sampleStorage_(kMaxStackDepth) {
#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  profilo_api()->register_external_tracer_callback(
      TRACER_TYPE_JAVASCRIPT, collectStackForLoom);
#endif
  sProfilerInstance_.store(this);
  // Reserve max possible unique GC event extra info count to
  // avoid rehashing.
  gcEventExtraInfoSet_.reserve(kMaxGCEventExtraInfoCount);
}

void SamplingProfiler::dumpSampledStack(llvh::raw_ostream &OS) {
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
      switch (frame.kind) {
        case StackFrame::FrameKind::JSFunction:
          OS << "[JS] " << frame.jsFrame.functionId << ":"
             << frame.jsFrame.offset;
          break;

        case StackFrame::FrameKind::NativeFunction:
          OS << "[Native] " << reinterpret_cast<uintptr_t>(frame.nativeFrame);
          break;

        case StackFrame::FrameKind::FinalizableNativeFunction:
          OS << "[HostFunction]";
          break;

        default:
          llvm_unreachable("Unknown frame kind");
      }
      OS << " => ";
    }
    OS << "\n";
  }
}

void SamplingProfiler::dumpChromeTrace(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  auto pid = getpid();
  ChromeTraceSerializer serializer(
      ChromeTraceFormat::create(pid, threadNames_, sampledStacks_));
  serializer.serialize(OS);
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
  timerThread_ = std::thread(&SamplingProfiler::timerLoop, this);
  return true;
}

bool SamplingProfiler::disable() {
  {
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
  }
  // Notify the timer thread that it has been disabled.
  enabledCondVar_.notify_all();
  // Wait for timer thread to exit. This avoids the timer thread reading from
  // memory that is freed after a main thread exits. This is outside the lock
  // on profilerLock_ since the timer thread needs to acquire that lock.
  timerThread_.join();
  return true;
}

void SamplingProfiler::clear() {
  sampledStacks_.clear();
  // Release all strong roots to domains.
  // Note: we can't clear domains_ because we have to maintain the storage size.
  for (Domain *&domain : domains_) {
    domain = nullptr;
  }
  // TODO: keep thread names that are still in use.
  threadNames_.clear();
}

void SamplingProfiler::onGCEvent(
    Runtime *runtime,
    GCEventKind kind,
    const std::string &extraInfo) {
  switch (kind) {
    case GCEventKind::CollectionStart: {
      assert(
          preGCStackDepth_ == 0 && "preGCStackDepth_ is not reset after GC?");
      std::lock_guard<std::mutex> lockGuard(profilerLock_);
      if (LLVM_LIKELY(!enabled_)) {
        return;
      }
      recordPreGCStack(runtime, extraInfo);
      break;
    }

    case GCEventKind::CollectionEnd:
      preGCStackDepth_ = 0;
      break;

    default:
      llvm_unreachable("Unknown GC event");
  }
}

void SamplingProfiler::recordPreGCStack(
    Runtime *runtime,
    const std::string &extraInfo) {
  GCFrameInfo gcExtraInfo = nullptr;
  // Only record extra info if not exceeding max allowed count to prevent
  // rehash.
  assert(
      gcEventExtraInfoSet_.size() < kMaxGCEventExtraInfoCount &&
      "Need to increase kMaxGCEventExtraInfoCount.");
  if (!extraInfo.empty() &&
      gcEventExtraInfoSet_.size() < kMaxGCEventExtraInfoCount) {
    std::pair<std::unordered_set<std::string>::iterator, bool> retPair =
        gcEventExtraInfoSet_.insert(extraInfo);
    gcExtraInfo = &(*(retPair.first));
  }

  auto &leafFrame = preGCStackStorage_.stack[0];
  leafFrame.kind = StackFrame::FrameKind::GCFrame;
  leafFrame.gcFrame = gcExtraInfo;

  // Leaf frame slot has been used, filling from index 1.
  preGCStackDepth_ = walkRuntimeStack(runtime, preGCStackStorage_, 1);
}

bool operator==(
    const SamplingProfiler::StackFrame &left,
    const SamplingProfiler::StackFrame &right) {
  if (left.kind != right.kind) {
    return false;
  }
  switch (left.kind) {
    case SamplingProfiler::StackFrame::FrameKind::JSFunction:
      return left.jsFrame.functionId == right.jsFrame.functionId &&
          left.jsFrame.offset == right.jsFrame.offset;

    case SamplingProfiler::StackFrame::FrameKind::NativeFunction:
      return left.nativeFrame == right.nativeFrame;

    case SamplingProfiler::StackFrame::FrameKind::FinalizableNativeFunction:
      return left.finalizableNativeFrame == right.finalizableNativeFrame;

    case SamplingProfiler::StackFrame::FrameKind::GCFrame:
      return left.gcFrame == right.gcFrame;

    default:
      llvm_unreachable("Unknown frame kind");
  }
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_POSIX
