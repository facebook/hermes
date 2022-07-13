/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <random>
#include <thread>

namespace hermes {
namespace vm {

/// Name of the semaphore.
const char *const kSamplingDoneSemaphoreName = "/samplingDoneSem";

std::atomic<SamplingProfiler::GlobalProfiler *>
    SamplingProfiler::GlobalProfiler::instance_{nullptr};

std::atomic<SamplingProfiler *>
    SamplingProfiler::GlobalProfiler::profilerForSig_{nullptr};

void SamplingProfiler::GlobalProfiler::registerRuntime(
    SamplingProfiler *profiler) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  profilers_.insert(profiler);

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  assert(
      threadLocalProfilerForLoom_.get() == nullptr &&
      "multiple hermes runtime in the same thread");
  threadLocalProfilerForLoom_.set(profiler);
#endif
}

void SamplingProfiler::GlobalProfiler::unregisterRuntime(
    SamplingProfiler *profiler) {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  bool succeed = profilers_.erase(profiler);
  // TODO: should we allow recursive style
  // register/register -> unregister/unregister call?
  assert(succeed && "How can runtime not registered yet?");
  (void)succeed;

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  // TODO(T125910634): re-introduce the requirement for unregistering the
  // runtime in the same thread it was registered.
  threadLocalProfilerForLoom_.set(nullptr);
#endif
}

void SamplingProfiler::registerDomain(Domain *domain) {
  // If domain is not already registered, add it to the list.
  auto it = std::find(domains_.begin(), domains_.end(), domain);
  if (it == domains_.end())
    domains_.push_back(domain);
}

void SamplingProfiler::markRoots(RootAcceptor &acceptor) {
  std::lock_guard<std::mutex> lockGuard(runtimeDataLock_);
  for (Domain *&domain : domains_) {
    acceptor.acceptPtr(domain);
  }
}

int SamplingProfiler::GlobalProfiler::invokeSignalAction(void (*handler)(int)) {
  struct sigaction actions;
  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  // Allows interrupted IO primitives to restart.
  actions.sa_flags = SA_RESTART;
  actions.sa_handler = handler;
  return sigaction(SIGPROF, &actions, nullptr);
}

bool SamplingProfiler::GlobalProfiler::registerSignalHandlers() {
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

bool SamplingProfiler::GlobalProfiler::unregisterSignalHandler() {
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

void SamplingProfiler::GlobalProfiler::profilingSignalHandler(int signo) {
  // Ensure that writes made on the timer thread before setting the current
  // profiler are correctly acquired.
  SamplingProfiler *localProfiler;
  while (!(localProfiler = profilerForSig_.load(std::memory_order_acquire))) {
  }

  assert(
      localProfiler->suspendCount_ == 0 &&
      "Shouldn't interrupt the VM thread when the sampling profiler is "
      "suspended.");

  // Avoid spoiling errno in a signal handler by storing the old version and
  // re-assigning it.
  auto oldErrno = errno;

  auto profilerInstance = instance_.load();
  assert(
      profilerInstance != nullptr &&
      "Why is GlobalProfiler::instance_ not initialized yet?");

  // Sampling stack will touch GC objects(like closure) so only do so if heap
  // is valid.
  auto &curThreadRuntime = localProfiler->runtime_;
  assert(
      !curThreadRuntime.getHeap().inGC() &&
      "sampling profiler should be suspended before GC");
  (void)curThreadRuntime;
  profilerInstance->sampledStackDepth_ = localProfiler->walkRuntimeStack(
      profilerInstance->sampleStorage_, SaveDomains::Yes);
  // Ensure that writes made in the handler are visible to the timer thread.
  profilerForSig_.store(nullptr);

  if (!instance_.load()->samplingDoneSem_.notifyOne()) {
    errno = oldErrno;
    abort(); // Something is wrong.
  }
  errno = oldErrno;
}

bool SamplingProfiler::GlobalProfiler::sampleStack() {
  for (SamplingProfiler *localProfiler : profilers_) {
    auto targetThreadId = localProfiler->currentThread_;
    std::lock_guard<std::mutex> lk(localProfiler->runtimeDataLock_);

    if (localProfiler->suspendCount_ > 0) {
      // Sampling profiler is suspended. Copy pre-captured stack instead without
      // interrupting the VM thread.
      if (localProfiler->preSuspendStackDepth_ > 0) {
        sampleStorage_ = localProfiler->preSuspendStackStorage_;
        sampledStackDepth_ = localProfiler->preSuspendStackDepth_;
      } else {
        // This suspension didn't record a stack trace. For example, a GC (like
        // mallocGC) did not record JS stack.
        // TODO: fix this for all cases.
        sampledStackDepth_ = 0;
      }
    } else {
      // Ensure there are no allocations in the signal handler by keeping ample
      // reserved space.
      localProfiler->domains_.reserve(
          localProfiler->domains_.size() + kMaxStackDepth);
      size_t domainCapacityBefore = localProfiler->domains_.capacity();
      (void)domainCapacityBefore;

      // Guarantee that the runtime thread will not proceed until it has
      // acquired the updates to domains_.
      profilerForSig_.store(localProfiler, std::memory_order_release);

      // Signal target runtime thread to sample stack.
      pthread_kill(targetThreadId, SIGPROF);

      // Threading: samplingDoneSem_ will synchronise this thread with the
      // signal handler, so that we only have one active signal at a time.
      if (!samplingDoneSem_.wait()) {
        return false;
      }

      // Guarantee that this thread will observe all changes made to data
      // structures in the signal handler.
      while (profilerForSig_.load(std::memory_order_acquire) != nullptr) {
      }

      assert(
          localProfiler->domains_.capacity() == domainCapacityBefore &&
          "Must not dynamically allocate in signal handler");
    }

    assert(
        sampledStackDepth_ <= sampleStorage_.stack.size() &&
        "How can we sample more frames than storage?");
    localProfiler->sampledStacks_.emplace_back(
        sampleStorage_.tid,
        sampleStorage_.timeStamp,
        sampleStorage_.stack.begin(),
        sampleStorage_.stack.begin() + sampledStackDepth_);
  }
  return true;
}

void SamplingProfiler::GlobalProfiler::timerLoop() {
  oscompat::set_thread_name("hermes-sampling-profiler");

  constexpr double kMeanMilliseconds = 10;
  constexpr double kStdDevMilliseconds = 5;
  std::random_device rd{};
  std::mt19937 gen{rd()};
  // The amount of time that is spent sleeping comes from a normal distribution,
  // to avoid the case where the timer thread samples a stack at a predictable
  // period.
  std::normal_distribution<> distribution{
      kMeanMilliseconds, kStdDevMilliseconds};
  std::unique_lock<std::mutex> uniqueLock(profilerLock_);

  while (enabled_) {
    if (!sampleStack()) {
      return;
    }

    const uint64_t millis = round(std::fabs(distribution(gen)));
    // TODO: make sampling rate configurable.
    enabledCondVar_.wait_for(
        uniqueLock, std::chrono::milliseconds(millis), [this]() {
          return !enabled_;
        });
  }
}

uint32_t SamplingProfiler::walkRuntimeStack(
    StackTrace &sampleStorage,
    SaveDomains saveDomains,
    uint32_t startIndex) {
  unsigned count = startIndex;

  // TODO: capture leaf frame IP.
  const Inst *ip = nullptr;
  for (ConstStackFramePtr frame : runtime_.getStackFrames()) {
    // Whether we successfully captured a stack frame or not.
    bool capturedFrame = true;
    auto &frameStorage = sampleStorage.stack[count];
    // Check if it is pure JS frame.
    auto *calleeCodeBlock = frame.getCalleeCodeBlock(runtime_);
    if (calleeCodeBlock != nullptr) {
      frameStorage.kind = StackFrame::FrameKind::JSFunction;
      frameStorage.jsFrame.functionId = calleeCodeBlock->getFunctionID();
      frameStorage.jsFrame.offset =
          (ip == nullptr ? 0 : calleeCodeBlock->getOffsetOf(ip));
      auto *module = calleeCodeBlock->getRuntimeModule();
      assert(module != nullptr && "Cannot fetch runtimeModule for code block");
      frameStorage.jsFrame.module = module;
      // Don't execute a read or write barrier here because this is a signal
      // handler.
      if (saveDomains == SaveDomains::Yes)
        registerDomain(module->getDomainForSamplingProfiler(runtime_));
    } else if (
        auto *nativeFunction =
            dyn_vmcast<NativeFunction>(frame.getCalleeClosureUnsafe())) {
      frameStorage.kind = vmisa<FinalizableNativeFunction>(nativeFunction)
          ? StackFrame::FrameKind::FinalizableNativeFunction
          : StackFrame::FrameKind::NativeFunction;
      frameStorage.nativeFrame = nativeFunction->getFunctionPtr();
    } else {
      // TODO: handle BoundFunction.
      capturedFrame = false;
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

/*static*/ SamplingProfiler::GlobalProfiler *
SamplingProfiler::GlobalProfiler::get() {
  // We intentionally leak this memory to avoid a case where instance is
  // accessed after it is destroyed during shutdown.
  static GlobalProfiler *instance = new GlobalProfiler{};
  return instance;
}

SamplingProfiler::GlobalProfiler::GlobalProfiler() {
  instance_.store(this);
#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  profilo_api()->register_external_tracer_callback(
      TRACER_TYPE_JAVASCRIPT, collectStackForLoom);
#endif
}

bool SamplingProfiler::GlobalProfiler::enabled() {
  std::lock_guard<std::mutex> lockGuard(profilerLock_);
  return enabled_;
}

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
/*static*/ StackCollectionRetcode SamplingProfiler::collectStackForLoom(
    ucontext_t *ucontext,
    int64_t *frames,
    uint16_t *depth,
    uint16_t max_depth) {
  auto profilerInstance = GlobalProfiler::instance_.load();
  SamplingProfiler *localProfiler =
      profilerInstance->threadLocalProfilerForLoom_.get();
  if (localProfiler == nullptr) {
    // No runtime in this thread.
    return StackCollectionRetcode::NO_STACK_FOR_THREAD;
  }

  uint32_t sampledStackDepth = 0;
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  if (LLVM_LIKELY(localProfiler->suspendCount_ == 0)) {
    Runtime &curThreadRuntime = localProfiler->runtime_;
    assert(
        !curThreadRuntime.getHeap().inGC() &&
        "sampling profiler should be suspended before GC");
    (void)curThreadRuntime;
    assert(
        profilerInstance != nullptr &&
        "Why is GlobalProfiler::instance_ not initialized yet?");
    // Do not register domains for Loom profiling, since we don't use them for
    // symbolication.
    sampledStackDepth = localProfiler->walkRuntimeStack(
        profilerInstance->sampleStorage_, SaveDomains::No);
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

        uint32_t segmentID = bcProvider->getSegmentID();
        uint64_t frameAddress = ((uint64_t)segmentID << 32) + virtualOffset;
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

SamplingProfiler::SamplingProfiler(Runtime &runtime)
    : currentThread_{pthread_self()}, runtime_{runtime} {
  threadNames_[oscompat::thread_id()] = oscompat::thread_name();
  GlobalProfiler::get()->registerRuntime(this);
}

SamplingProfiler::~SamplingProfiler() {
  // TODO(T125910634): re-introduce the requirement for destroying the sampling
  // profiler on the same thread in which it was created.
  GlobalProfiler::get()->unregisterRuntime(this);
}

void SamplingProfiler::dumpSampledStackGlobal(llvh::raw_ostream &OS) {
  auto globalProfiler = GlobalProfiler::get();
  std::lock_guard<std::mutex> lk(globalProfiler->profilerLock_);
  if (!globalProfiler->profilers_.empty()) {
    auto *localProfiler = *globalProfiler->profilers_.begin();
    localProfiler->dumpSampledStack(OS);
  }
}

void SamplingProfiler::dumpSampledStack(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
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

void SamplingProfiler::dumpChromeTraceGlobal(llvh::raw_ostream &OS) {
  auto globalProfiler = GlobalProfiler::get();
  std::lock_guard<std::mutex> lk(globalProfiler->profilerLock_);
  if (!globalProfiler->profilers_.empty()) {
    auto *localProfiler = *globalProfiler->profilers_.begin();
    localProfiler->dumpChromeTrace(OS);
  }
}

void SamplingProfiler::dumpChromeTrace(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  auto pid = getpid();
  ChromeTraceSerializer serializer(
      ChromeTraceFormat::create(pid, threadNames_, sampledStacks_));
  serializer.serialize(OS);
  clear();
}

void SamplingProfiler::serializeInDevToolsFormat(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  hermes::vm::serializeAsProfilerProfile(
      OS, ChromeTraceFormat::create(getpid(), threadNames_, sampledStacks_));
  clear();
}

bool SamplingProfiler::enable() {
  return GlobalProfiler::get()->enable();
}

bool SamplingProfiler::GlobalProfiler::enable() {
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
  timerThread_ = std::thread(&GlobalProfiler::timerLoop, this);
  return true;
}

bool SamplingProfiler::disable() {
  return GlobalProfiler::get()->disable();
}

bool SamplingProfiler::GlobalProfiler::disable() {
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
  domains_.clear();
  // TODO: keep thread names that are still in use.
  threadNames_.clear();
}

void SamplingProfiler::suspend(std::string_view extraInfo) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  if (++suspendCount_ > 1 || extraInfo.empty()) {
    // If there are multiple nested suspend calls use a default "suspended"
    // label for the suspend entry in the call stack. Also use the default
    // when no extra info is provided.
    extraInfo = "suspended";
  }

  // Only record the stack trace for the first suspend() call.
  if (LLVM_UNLIKELY(GlobalProfiler::get()->enabled() && suspendCount_ == 1)) {
    recordPreSuspendStack(extraInfo);
  }
}

void SamplingProfiler::resume() {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  assert(suspendCount_ > 0 && "resume() without suspend()");
  if (--suspendCount_ == 0) {
    preSuspendStackDepth_ = 0;
  }
}

void SamplingProfiler::recordPreSuspendStack(std::string_view extraInfo) {
  std::pair<std::unordered_set<std::string>::iterator, bool> retPair =
      suspendEventExtraInfoSet_.emplace(extraInfo);
  SuspendFrameInfo suspendExtraInfo = &(*(retPair.first));

  auto &leafFrame = preSuspendStackStorage_.stack[0];
  leafFrame.kind = StackFrame::FrameKind::SuspendFrame;
  leafFrame.suspendFrame = suspendExtraInfo;

  // Leaf frame slot has been used, filling from index 1.
  preSuspendStackDepth_ =
      walkRuntimeStack(preSuspendStackStorage_, SaveDomains::Yes, 1);
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

    case SamplingProfiler::StackFrame::FrameKind::SuspendFrame:
      return left.suspendFrame == right.suspendFrame;

    default:
      llvm_unreachable("Unknown frame kind");
  }
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_POSIX
