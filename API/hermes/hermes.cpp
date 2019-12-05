/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes.h"

#include "llvm/Support/Compiler.h"

#ifndef LLVM_PTR_SIZE
#error "LLVM_PTR_SIZE needs to be defined"
#endif

#if LLVM_PTR_SIZE != 8
// Only have JSI be on the stack for builds that are not 64-bit.
#define HERMESJSI_ON_STACK
#endif

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/DebuggerAPI.h"
#include "hermes/Instrumentation/PerfMarkers.h"
#include "hermes/Platform/Logging.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/UTF16Stream.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Debugger/Debugger.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/JSLib/RuntimeJSONUtils.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/TimeLimitMonitor.h"

#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SHA1.h"
#include "llvm/Support/raw_os_ostream.h"

#include <atomic>
#include <limits>
#include <list>
#include <mutex>
#include <system_error>

#ifdef HERMESJSI_ON_STACK
#include <future>
#include <thread>
#endif

#include <jsi/instrumentation.h>
#include <jsi/threadsafe.h>

#ifdef HERMESVM_LLVM_PROFILE_DUMP
extern "C" {
int __llvm_profile_dump(void);
}
#endif

// If a function body might throw C++ exceptions other than
// jsi::JSError from Hermes, it should be wrapped in this form:
//
//   return maybeRethrow([&] { body })
//
// This will execute body; if exceptions are enabled, this execution
// will be wrapped in a try/catch that catches those exceptions, and
// rethrows them as JSI exceptions.
// This function should be used to wrap any JSI APIs that may allocate
// memory(for OOM) or may enter interpreter(call a VM API with _RJS postfix).
// We convert the hermes exception into a JSINativeException; JSError represents
// exceptions mandated by the spec, and JSINativeException covers all
// other exceptions.
namespace {
template <typename F>
auto maybeRethrow(const F &f) -> decltype(f()) {
#ifdef HERMESVM_EXCEPTION_ON_OOM
  try {
    return f();
  } catch (const ::hermes::vm::JSOutOfMemoryError &ex) {
    // We surface this as a JSINativeException -- the out of memory
    // exception is not part of the spec.
    throw ::facebook::jsi::JSINativeException(ex.what());
  }
#else // HERMESVM_EXCEPTION_ON_OOM
  return f();
#endif
}
} // namespace

namespace vm = hermes::vm;
namespace hbc = hermes::hbc;
using ::hermes::hermesLog;

namespace facebook {
namespace hermes {
namespace detail {

#if !defined(NDEBUG) && !defined(ASSERT_ON_DANGLING_VM_REFS)
#define ASSERT_ON_DANGLING_VM_REFS
#endif

static void (*sApiFatalHandler)(const std::string &) = nullptr;
/// Handler called by HermesVM to report unrecoverable errors.
/// This is a forward declaration to prevent a compiler warning.
void hermesFatalErrorHandler(
    void *user_data,
    const std::string &reason,
    bool gen_crash_diag);

void hermesFatalErrorHandler(
    void * /*user_data*/,
    const std::string &reason,
    bool /*gen_crash_diag*/) {
  // Actually crash and let breakpad handle the reporting.
  if (sApiFatalHandler) {
    sApiFatalHandler(reason);
  } else {
    *((volatile int *)nullptr) = 42;
  }
}

} // namespace detail

namespace {

// Max size of the runtime's register stack.
// The runtime register stack needs to be small enough to be allocated on the
// native thread stack in Android (1MiB) and on MacOS's thread stack (512 KiB)
// Calculated by: (thread stack size - size of runtime -
// 8 memory pages for other stuff in the thread)
static constexpr unsigned kMaxNumRegisters =
    (512 * 1024 - sizeof(::hermes::vm::Runtime) - 4096 * 8) /
    sizeof(::hermes::vm::PinnedHermesValue);

void raw_ostream_append(llvm::raw_ostream &os) {}

template <typename Arg0, typename... Args>
void raw_ostream_append(llvm::raw_ostream &os, Arg0 &&arg0, Args &&... args) {
  os << arg0;
  raw_ostream_append(os, args...);
}

template <typename... Args>
jsi::JSError makeJSError(jsi::Runtime &rt, Args &&... args) {
  std::string s;
  llvm::raw_string_ostream os(s);
  raw_ostream_append(os, std::forward<Args>(args)...);
  return jsi::JSError(rt, os.str());
}

/// HermesVM uses the LLVM fatal error handle to report fatal errors. This
/// wrapper helps us install the handler at construction time, before any
/// HermesVM code has been invoked.
class InstallHermesFatalErrorHandler {
 public:
  InstallHermesFatalErrorHandler() {
    // The LLVM fatal error handler can only be installed once. Use a Meyer's
    // singleton to guarantee it - the static "dummy" is guaranteed by the
    // compiler to be initialized no more than once.
    static int dummy = ([]() {
      llvm::install_fatal_error_handler(detail::hermesFatalErrorHandler);
      return 0;
    })();
    (void)dummy;
  }
};

#ifdef HERMESJSI_ON_STACK
// Minidumps include stack memory, not heap memory.  If we want to be
// able to inspect the Runtime object in a minidump, we can do that by
// arranging for it to be allocated on a stack.  No existing stack is
// a good candidate, so we achieve this by creating a thread just to
// hold the Runtime.

class StackRuntime {
 public:
  StackRuntime(const vm::RuntimeConfig &runtimeConfig)
      : thread_(runtimeMemoryThread, this, runtimeConfig.getGCConfig()) {
    startup_.get_future().wait();
    runtime_->emplace(
        provider_.get(),
        runtimeConfig.rebuild()
            .withRegisterStack(nullptr)
            .withMaxNumRegisters(kMaxNumRegisters)
            .build());
  }

  ~StackRuntime() {
    // We can't shut down the Runtime on the captive thread, because
    // it might need to make JNI calls to clean up HostObjects.  So we
    // delete it from here, which is going to be on a thread
    // registered with the JVM.
    runtime_->reset();
    shutdown_.set_value();
    thread_.join();
    runtime_ = nullptr;
  }

  ::hermes::vm::Runtime &getRuntime() {
    return **runtime_;
  }

 private:
  static void runtimeMemoryThread(StackRuntime *stack, vm::GCConfig config) {
#if defined(__APPLE__)
    // Capture the thread name in case if something was already set, so that we
    // can restore it later when we're potentially returning the thread back
    // to some pool.
    char buf[256];
    int getNameSuccess = pthread_getname_np(pthread_self(), buf, sizeof(buf));

    pthread_setname_np("hermes-runtime-memorythread");
#endif

    llvm::Optional<::hermes::vm::StackRuntime> rt;

    stack->provider_ = vm::StorageProvider::mmapProvider();
    stack->runtime_ = &rt;
    stack->startup_.set_value();
    stack->shutdown_.get_future().wait();
    assert(!rt.hasValue() && "Runtime was not torn down before thread");

#if defined(__APPLE__)
    if (!getNameSuccess) {
      pthread_setname_np(buf);
    }
#endif
  }

  // The order here matters.
  // * Set up the promises
  // * Initialize various pointers to null
  // * Start the thread which uses them
  // * Initialize provider_ and runtime_ from that thread
  std::promise<void> startup_;
  std::promise<void> shutdown_;
  std::unique_ptr<::hermes::vm::StorageProvider> provider_;
  llvm::Optional<::hermes::vm::StackRuntime> *runtime_{nullptr};
  std::thread thread_;
};
#endif

} // namespace

class HermesRuntimeImpl final : public HermesRuntime,
                                private InstallHermesFatalErrorHandler,
                                private jsi::Instrumentation {
 public:
  static constexpr int64_t kSentinelNativeValue = 0x6ef71fe1;

  HermesRuntimeImpl(const vm::RuntimeConfig &runtimeConfig)
      :
#ifdef HERMESJSI_ON_STACK
        stackRuntime_(runtimeConfig),
        runtime_(stackRuntime_.getRuntime()),
#else
        rt_(::hermes::vm::Runtime::create(
            runtimeConfig.rebuild()
                .withRegisterStack(nullptr)
                .withMaxNumRegisters(kMaxNumRegisters)
                .build())),
        runtime_(*rt_),
#endif
        crashMgr_(runtimeConfig.getCrashMgr()) {
    compileFlags_.optimize = false;
#ifdef HERMES_ENABLE_DEBUGGER
    compileFlags_.debug = true;
#endif

#ifndef HERMESJSI_ON_STACK
    // Register the memory for the runtime if it isn't stored on the stack.
    crashMgr_->registerMemory(&runtime_, sizeof(vm::Runtime));
#endif
    runtime_.addCustomRootsFunction(
        [this](vm::GC *, vm::SlotAcceptor &acceptor) {
          for (auto it = hermesValues_->begin(); it != hermesValues_->end();) {
            if (it->get() == 0) {
              it = hermesValues_->erase(it);
            } else {
              acceptor.accept(const_cast<vm::PinnedHermesValue &>(it->phv));
              ++it;
            }
          }
        });
    runtime_.addCustomWeakRootsFunction(
        [this](vm::GC *, vm::WeakRefAcceptor &acceptor) {
          for (auto it = weakHermesValues_->begin();
               it != weakHermesValues_->end();) {
            if (it->get() == 0) {
              it = weakHermesValues_->erase(it);
            } else {
              acceptor.accept(
                  const_cast<vm::WeakRef<vm::HermesValue> &>(it->wr));
              ++it;
            }
          }
        });
  }

 public:
  ~HermesRuntimeImpl() {
#ifdef HERMES_ENABLE_DEBUGGER
    // Deallocate the debugger so it frees any HermesPointerValues it may hold.
    // This must be done before we check hermesValues_ below.
    debugger_.reset();
#endif
#ifndef HERMESJSI_ON_STACK
    // Unregister the memory for the runtime if it isn't stored on the stack.
    crashMgr_->unregisterMemory(&runtime_);
#endif
  }

#ifdef HERMES_ENABLE_DEBUGGER
  // This should only be called once by the factory.
  void setDebugger(std::unique_ptr<debugger::Debugger> d) {
    debugger_ = std::move(d);
  }
#endif

  struct CountedPointerValue : PointerValue {
    CountedPointerValue() : refCount(1) {}

    void invalidate() override {
#ifdef ASSERT_ON_DANGLING_VM_REFS
      assert(
          ((1 << 31) & refCount) == 0 &&
          "This PointerValue was left dangling after the Runtime was destroyed.");
#endif
      dec();
    }

    void inc() {
      auto oldCount = refCount.fetch_add(1, std::memory_order_relaxed);
      assert(oldCount + 1 != 0 && "Ref count overflow");
      (void)oldCount;
    }

    void dec() {
      auto oldCount = refCount.fetch_sub(1, std::memory_order_relaxed);
      assert(oldCount > 0 && "Ref count underflow");
      (void)oldCount;
    }

    uint32_t get() const {
      return refCount.load(std::memory_order_relaxed);
    }

#ifdef ASSERT_ON_DANGLING_VM_REFS
    void markDangling() {
      // Mark this PointerValue as dangling by setting the top bit AND the
      // second-top bit. The top bit is used to determine if the pointer is
      // dangling. Setting the second-top bit ensures that accidental
      // over-calling the dec() function doesn't clear the top bit without
      // complicating the implementation of dec().
      refCount |= 0b11 << 30;
    }
#endif

   private:
    std::atomic<uint32_t> refCount;
  };

  struct HermesPointerValue final : CountedPointerValue {
    HermesPointerValue(vm::HermesValue hv) : phv(hv) {}

    // This should only ever be modified by the GC.  We const_cast the
    // reference before passing it to the GC.
    const vm::PinnedHermesValue phv;
  };

  struct WeakRefPointerValue final : CountedPointerValue {
    WeakRefPointerValue(vm::WeakRef<vm::HermesValue> _wr) : wr(_wr) {}

    // This should only ever be modified by the GC.  We const_cast the
    // reference before passing it to the GC.
    const vm::WeakRef<vm::HermesValue> wr;
  };

  HermesPointerValue *clone(const Runtime::PointerValue *pv) {
    if (!pv) {
      return nullptr;
    }
    // These are only ever allocated by us, so we can remove their constness
    auto result = static_cast<HermesPointerValue *>(
        const_cast<Runtime::PointerValue *>(pv));
    result->inc();
    return result;
  }

  template <typename T>
  T add(::hermes::vm::HermesValue hv) {
    static_assert(
        std::is_base_of<jsi::Pointer, T>::value, "this type cannot be added");
    hermesValues_->emplace_front(hv);
    return make<T>(&(hermesValues_->front()));
  }

  jsi::WeakObject addWeak(::hermes::vm::WeakRef<vm::HermesValue> wr) {
    weakHermesValues_->emplace_front(wr);
    return make<jsi::WeakObject>(&(weakHermesValues_->front()));
  }

  // overriden from jsi::Instrumentation
  std::string getRecordedGCStats() override {
    std::string s;
    llvm::raw_string_ostream os(s);
    runtime_.printHeapStats(os);
    return os.str();
  }

  // Overridden from jsi::Instrumentation
  // See include/hermes/VM/GCBase.h for documentation of the fields
  std::unordered_map<std::string, int64_t> getHeapInfo(
      bool includeExpensive) override {
    vm::GCBase::HeapInfo info;
    if (includeExpensive) {
      runtime_.getHeap().getHeapInfoWithMallocSize(info);
    } else {
      runtime_.getHeap().getHeapInfo(info);
    }
#ifndef NDEBUG
    vm::GCBase::DebugHeapInfo debugInfo;
    runtime_.getHeap().getDebugHeapInfo(debugInfo);
#endif

    std::unordered_map<std::string, int64_t> jsInfo;

#define BRIDGE_INFO(TYPE, HOLDER, NAME) \
  jsInfo["hermes_" #NAME] = static_cast<TYPE>(HOLDER.NAME);

    BRIDGE_INFO(int, info, numCollections);
    BRIDGE_INFO(double, info, totalAllocatedBytes);
    BRIDGE_INFO(double, info, allocatedBytes);
    BRIDGE_INFO(double, info, heapSize);
    BRIDGE_INFO(double, info, va);
    if (includeExpensive) {
      BRIDGE_INFO(double, info, mallocSizeEstimate);
    }

#ifndef NDEBUG
    BRIDGE_INFO(int, debugInfo, numAllocatedObjects);
    BRIDGE_INFO(int, debugInfo, numReachableObjects);
    BRIDGE_INFO(int, debugInfo, numCollectedObjects);
    BRIDGE_INFO(int, debugInfo, numFinalizedObjects);
    BRIDGE_INFO(int, debugInfo, numMarkedSymbols);
    BRIDGE_INFO(int, debugInfo, numHiddenClasses);
    BRIDGE_INFO(int, debugInfo, numLeafHiddenClasses);
#endif

#undef BRIDGE_INFO

    jsInfo["hermes_peakAllocatedBytes"] =
        runtime_.getHeap().getPeakAllocatedBytes();
    jsInfo["hermes_peakLiveAfterGC"] = runtime_.getHeap().getPeakLiveAfterGC();

#define BRIDGE_GEN_INFO(NAME, STAT_EXPR, FACTOR)                    \
  jsInfo["hermes_full_" #NAME] = info.fullStats.STAT_EXPR * FACTOR; \
  jsInfo["hermes_yg_" #NAME] = info.youngGenStats.STAT_EXPR * FACTOR;

    BRIDGE_GEN_INFO(numCollections, numCollections, 1.0);
    // Times are converted from seconds to milliseconds for the logging pipeline
    // ...
    BRIDGE_GEN_INFO(gcTime, gcWallTime.sum(), 1000);
    BRIDGE_GEN_INFO(maxPause, gcWallTime.max(), 1000);
    BRIDGE_GEN_INFO(gcCPUTime, gcCPUTime.sum(), 1000);
    BRIDGE_GEN_INFO(gcMaxCPUPause, gcCPUTime.max(), 1000);
    // ... and since this is square seconds, we must square the 1000 too.
    BRIDGE_GEN_INFO(gcTimeSquares, gcWallTime.sumOfSquares(), 1000 * 1000);
    BRIDGE_GEN_INFO(gcCPUTimeSquares, gcCPUTime.sumOfSquares(), 1000 * 1000);

#undef BRIDGE_GEN_INFO

    return jsInfo;
  }

  // Overridden from jsi::Instrumentation
  void collectGarbage() override {
    runtime_.getHeap().collect();
  }

  // Overridden from jsi::Instrumentation
  bool createSnapshotToFile(const std::string &path) override {
    return runtime_.getHeap().createSnapshotToFile(path);
  }

  // Overridden from jsi::Instrumentation
  bool createSnapshotToStream(std::ostream &os) override {
    llvm::raw_os_ostream ros(os);
    runtime_.getHeap().createSnapshot(ros);
    return !os;
  }

  // Overridden from jsi::Instrumentation
  void writeBridgeTrafficTraceToFile(
      const std::string &fileName) const override {
    throw std::logic_error(
        "Bridge traffic trace is only supported by TracingRuntime");
  }

  // Overridden from jsi::Instrumentation
  void writeBasicBlockProfileTraceToFile(
      const std::string &fileName) const override {
#ifdef HERMESVM_PROFILER_BB
    std::error_code ec;
    llvm::raw_fd_ostream os(fileName.c_str(), ec, llvm::sys::fs::F_Text);
    if (ec) {
      throw std::system_error(ec);
    }
    runtime_.dumpBasicBlockProfileTrace(os);
#else
    throw std::logic_error(
        "Cannot write the basic block profile trace out if Hermes wasn't built with "
        "hermes.profiler=BB");
#endif
  }

  // Overridden from jsi::Instrumentation
  void dumpProfilerSymbolsToFile(const std::string &fileName) const override {
#ifdef HERMESVM_PROFILER_EXTERN
    dumpProfilerSymbolMap(&runtime_, fileName);
#else
    throw std::logic_error(
        "Cannot dump profiler symbols out if Hermes wasn't built with "
        "hermes.profiler=EXTERN");
#endif
  }

  // These are all methods which do pointer type gymnastics and should
  // mostly inline and optimize away.

  static const ::hermes::vm::PinnedHermesValue &phv(
      const jsi::Pointer &pointer) {
    assert(
        dynamic_cast<const HermesPointerValue *>(getPointerValue(pointer)) &&
        "Pointer does not contain a HermesPointerValue");
    return static_cast<const HermesPointerValue *>(getPointerValue(pointer))
        ->phv;
  }

  static const ::hermes::vm::PinnedHermesValue &phv(const jsi::Value &value) {
    assert(
        dynamic_cast<const HermesPointerValue *>(getPointerValue(value)) &&
        "Pointer does not contain a HermesPointerValue");
    return static_cast<const HermesPointerValue *>(getPointerValue(value))->phv;
  }

  static ::hermes::vm::Handle<::hermes::vm::HermesValue> stringHandle(
      const jsi::String &str) {
    return ::hermes::vm::Handle<::hermes::vm::HermesValue>::vmcast(&phv(str));
  }

  static ::hermes::vm::Handle<::hermes::vm::JSObject> handle(
      const jsi::Object &obj) {
    return ::hermes::vm::Handle<::hermes::vm::JSObject>::vmcast(&phv(obj));
  }

  static ::hermes::vm::Handle<::hermes::vm::JSArray> arrayHandle(
      const jsi::Array &arr) {
    return ::hermes::vm::Handle<::hermes::vm::JSArray>::vmcast(&phv(arr));
  }

  static ::hermes::vm::Handle<::hermes::vm::JSArrayBuffer> arrayBufferHandle(
      const jsi::ArrayBuffer &arr) {
    return ::hermes::vm::Handle<::hermes::vm::JSArrayBuffer>::vmcast(&phv(arr));
  }

  static const ::hermes::vm::WeakRef<vm::HermesValue> &wrhv(
      const jsi::Pointer &pointer) {
    assert(
        dynamic_cast<const WeakRefPointerValue *>(getPointerValue(pointer)) &&
        "Pointer does not contain a WeakRefPointerValue");
    return static_cast<const WeakRefPointerValue *>(getPointerValue(pointer))
        ->wr;
  }

  // These helpers use public (mostly) interfaces on the runtime and
  // value types to convert between jsi and vm types.

  static vm::HermesValue hvFromValue(const jsi::Value &value) {
    if (value.isUndefined()) {
      return vm::HermesValue::encodeUndefinedValue();
    } else if (value.isNull()) {
      return vm::HermesValue::encodeNullValue();
    } else if (value.isBool()) {
      return vm::HermesValue::encodeBoolValue(value.getBool());
    } else if (value.isNumber()) {
      return vm::HermesValue::encodeNumberValue(value.getNumber());
    } else if (value.isSymbol() || value.isString() || value.isObject()) {
      return phv(value);
    } else {
      llvm_unreachable("unknown value kind");
    }
  }

  vm::Handle<> vmHandleFromValue(const jsi::Value &value) {
    if (value.isUndefined()) {
      return vm::Runtime::getUndefinedValue();
    } else if (value.isNull()) {
      return vm::Runtime::getNullValue();
    } else if (value.isBool()) {
      return vm::Runtime::getBoolValue(value.getBool());
    } else if (value.isNumber()) {
      return runtime_.makeHandle(
          vm::HermesValue::encodeNumberValue(value.getNumber()));
    } else if (value.isSymbol() || value.isString() || value.isObject()) {
      return vm::Handle<vm::HermesValue>(&phv(value));
    } else {
      llvm_unreachable("unknown value kind");
    }
  }

  jsi::Value valueFromHermesValue(vm::HermesValue hv) {
    if (hv.isUndefined() || hv.isEmpty()) {
      return jsi::Value::undefined();
    } else if (hv.isNull()) {
      return nullptr;
    } else if (hv.isBool()) {
      return hv.getBool();
    } else if (hv.isDouble()) {
      return hv.getDouble();
    } else if (hv.isSymbol()) {
      return add<jsi::Symbol>(hv);
    } else if (hv.isString()) {
      return add<jsi::String>(hv);
    } else if (hv.isObject()) {
      return add<jsi::Object>(hv);
    } else {
      llvm_unreachable("unknown HermesValue type");
    }
  }

  // Concrete declarations of jsi::Runtime pure virtual methods

  std::shared_ptr<const jsi::PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      std::string sourceURL) override;
  jsi::Value evaluatePreparedJavaScript(
      const std::shared_ptr<const jsi::PreparedJavaScript> &js) override;
  jsi::Value evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override;
  jsi::Object global() override;

  std::string description() override;
  bool isInspectable() override;
  jsi::Instrumentation &instrumentation() override;

  PointerValue *cloneSymbol(const Runtime::PointerValue *pv) override;
  PointerValue *cloneString(const Runtime::PointerValue *pv) override;
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override;
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override;

  jsi::PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override;
  jsi::PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override;
  jsi::PropNameID createPropNameIDFromString(const jsi::String &str) override;
  std::string utf8(const jsi::PropNameID &) override;
  bool compare(const jsi::PropNameID &, const jsi::PropNameID &) override;

  std::string symbolToString(const jsi::Symbol &) override;

  jsi::String createStringFromAscii(const char *str, size_t length) override;
  jsi::String createStringFromUtf8(const uint8_t *utf8, size_t length) override;
  std::string utf8(const jsi::String &) override;

  jsi::Value createValueFromJsonUtf8(const uint8_t *json, size_t length)
      override;

  jsi::Object createObject() override;
  jsi::Object createObject(std::shared_ptr<jsi::HostObject> ho) override;
  std::shared_ptr<jsi::HostObject> getHostObject(const jsi::Object &) override;
  jsi::HostFunctionType &getHostFunction(const jsi::Function &) override;
  jsi::Value getProperty(const jsi::Object &, const jsi::PropNameID &name)
      override;
  jsi::Value getProperty(const jsi::Object &, const jsi::String &name) override;
  bool hasProperty(const jsi::Object &, const jsi::PropNameID &name) override;
  bool hasProperty(const jsi::Object &, const jsi::String &name) override;
  void setPropertyValue(
      jsi::Object &,
      const jsi::PropNameID &name,
      const jsi::Value &value) override;
  void setPropertyValue(
      jsi::Object &,
      const jsi::String &name,
      const jsi::Value &value) override;
  bool isArray(const jsi::Object &) const override;
  bool isArrayBuffer(const jsi::Object &) const override;
  bool isFunction(const jsi::Object &) const override;
  bool isHostObject(const jsi::Object &) const override;
  bool isHostFunction(const jsi::Function &) const override;
  jsi::Array getPropertyNames(const jsi::Object &) override;

  jsi::WeakObject createWeakObject(const jsi::Object &) override;
  jsi::Value lockWeakObject(const jsi::WeakObject &) override;

  jsi::Array createArray(size_t length) override;
  size_t size(const jsi::Array &) override;
  size_t size(const jsi::ArrayBuffer &) override;
  uint8_t *data(const jsi::ArrayBuffer &) override;
  jsi::Value getValueAtIndex(const jsi::Array &, size_t i) override;
  void setValueAtIndexImpl(jsi::Array &, size_t i, const jsi::Value &value)
      override;

  jsi::Function createFunctionFromHostFunction(
      const jsi::PropNameID &name,
      unsigned int paramCount,
      jsi::HostFunctionType func) override;
  jsi::Value call(
      const jsi::Function &,
      const jsi::Value &jsThis,
      const jsi::Value *args,
      size_t count) override;
  jsi::Value callAsConstructor(
      const jsi::Function &,
      const jsi::Value *args,
      size_t count) override;

  bool strictEquals(const jsi::Symbol &a, const jsi::Symbol &b) const override;
  bool strictEquals(const jsi::String &a, const jsi::String &b) const override;
  bool strictEquals(const jsi::Object &a, const jsi::Object &b) const override;

  bool instanceOf(const jsi::Object &o, const jsi::Function &ctor) override;

  ScopeState *pushScope() override;
  void popScope(ScopeState *prv) override;

  void checkStatus(vm::ExecutionStatus);
  vm::HermesValue stringHVFromAscii(const char *ascii, size_t length);
  vm::HermesValue stringHVFromUtf8(const uint8_t *utf8, size_t length);
  size_t getLength(vm::Handle<vm::ArrayImpl> arr);
  size_t getByteLength(vm::Handle<vm::JSArrayBuffer> arr);

  struct JsiProxyBase : public vm::HostObjectProxy {
    JsiProxyBase(HermesRuntimeImpl &rt, std::shared_ptr<jsi::HostObject> ho)
        : rt_(rt), ho_(ho) {}

    HermesRuntimeImpl &rt_;
    std::shared_ptr<jsi::HostObject> ho_;
  };

  struct JsiProxy final : public JsiProxyBase {
    using JsiProxyBase::JsiProxyBase;
    vm::CallResult<vm::HermesValue> get(vm::SymbolID id) override {
      auto &stats = rt_.runtime_.getRuntimeStats();
      const vm::instrumentation::RAIITimer timer{
          "HostObject.get", stats, stats.hostFunction};
      jsi::PropNameID sym =
          rt_.add<jsi::PropNameID>(vm::HermesValue::encodeSymbolValue(id));
      jsi::Value ret;
      try {
        ret = ho_->get(rt_, sym);
      } catch (const jsi::JSError &error) {
        return rt_.runtime_.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt_.runtime_.setThrownValue(
            hvFromValue(rt_.global()
                            .getPropertyAsFunction(rt_, "Error")
                            .call(
                                rt_,
                                std::string("Exception in HostObject::get: ") +
                                    ex.what())));
      } catch (...) {
        return rt_.runtime_.setThrownValue(hvFromValue(
            rt_.global()
                .getPropertyAsFunction(rt_, "Error")
                .call(rt_, "Exception in HostObject::get: <unknown>")));
      }

      return hvFromValue(ret);
    }

    vm::CallResult<bool> set(vm::SymbolID id, vm::HermesValue value) override {
      auto &stats = rt_.runtime_.getRuntimeStats();
      const vm::instrumentation::RAIITimer timer{
          "HostObject.set", stats, stats.hostFunction};
      jsi::PropNameID sym =
          rt_.add<jsi::PropNameID>(vm::HermesValue::encodeSymbolValue(id));
      try {
        ho_->set(rt_, sym, rt_.valueFromHermesValue(value));
      } catch (const jsi::JSError &error) {
        return rt_.runtime_.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt_.runtime_.setThrownValue(
            hvFromValue(rt_.global()
                            .getPropertyAsFunction(rt_, "Error")
                            .call(
                                rt_,
                                std::string("Exception in HostObject::set: ") +
                                    ex.what())));
      } catch (...) {
        return rt_.runtime_.setThrownValue(hvFromValue(
            rt_.global()
                .getPropertyAsFunction(rt_, "Error")
                .call(rt_, "Exception in HostObject::set: <unknown>")));
      }
      return true;
    }

    vm::CallResult<vm::Handle<vm::JSArray>> getHostPropertyNames() override {
      auto &stats = rt_.runtime_.getRuntimeStats();
      const vm::instrumentation::RAIITimer timer{
          "HostObject.getHostPropertyNames", stats, stats.hostFunction};
      try {
        auto names = ho_->getPropertyNames(rt_);

        auto arrayRes =
            vm::JSArray::create(&rt_.runtime_, names.size(), names.size());
        if (arrayRes == vm::ExecutionStatus::EXCEPTION) {
          return vm::ExecutionStatus::EXCEPTION;
        }
        vm::Handle<vm::JSArray> arrayHandle =
            vm::toHandle(&rt_.runtime_, std::move(*arrayRes));

        vm::GCScope gcScope{&rt_.runtime_};
        vm::MutableHandle<vm::SymbolID> tmpHandle{&rt_.runtime_};
        size_t i = 0;
        for (auto &name : names) {
          tmpHandle = phv(name).getSymbol();
          vm::JSArray::setElementAt(arrayHandle, &rt_.runtime_, i++, tmpHandle);
        }

        return arrayHandle;
      } catch (const jsi::JSError &error) {
        return rt_.runtime_.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt_.runtime_.setThrownValue(hvFromValue(
            rt_.global()
                .getPropertyAsFunction(rt_, "Error")
                .call(
                    rt_,
                    std::string("Exception in HostObject::getPropertyNames: ") +
                        ex.what())));
      } catch (...) {
        return rt_.runtime_.setThrownValue(hvFromValue(
            rt_.global()
                .getPropertyAsFunction(rt_, "Error")
                .call(
                    rt_,
                    "Exception in HostObject::getPropertyNames: <unknown>")));
      }
    };
  };

  struct HFContextBase {
    HFContextBase(jsi::HostFunctionType hf, HermesRuntimeImpl &hri)
        : hostFunction(std::move(hf)), hermesRuntimeImpl(hri) {}

    jsi::HostFunctionType hostFunction;
    HermesRuntimeImpl &hermesRuntimeImpl;
  };

  struct HFContext final : public HFContextBase {
    using HFContextBase::HFContextBase;

    static vm::CallResult<vm::HermesValue>
    func(void *context, vm::Runtime *runtime, vm::NativeArgs hvArgs) {
      HFContext *hfc = reinterpret_cast<HFContext *>(context);
      HermesRuntimeImpl &rt = hfc->hermesRuntimeImpl;
      assert(runtime == &rt.runtime_);
      auto &stats = rt.runtime_.getRuntimeStats();
      const vm::instrumentation::RAIITimer timer{
          "Host Function", stats, stats.hostFunction};

      llvm::SmallVector<jsi::Value, 8> apiArgs;
      for (vm::HermesValue hv : hvArgs) {
        apiArgs.push_back(rt.valueFromHermesValue(hv));
      }

      jsi::Value ret;
      const jsi::Value *args = apiArgs.empty() ? nullptr : &apiArgs.front();

      try {
        ret = (hfc->hostFunction)(
            rt,
            rt.valueFromHermesValue(hvArgs.getThisArg()),
            args,
            apiArgs.size());
      } catch (const jsi::JSError &error) {
        return runtime->setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt.runtime_.setThrownValue(hvFromValue(
            rt.global()
                .getPropertyAsFunction(rt, "Error")
                .call(
                    rt,
                    std::string("Exception in HostFunction: ") + ex.what())));
      } catch (...) {
        return rt.runtime_.setThrownValue(
            hvFromValue(rt.global()
                            .getPropertyAsFunction(rt, "Error")
                            .call(rt, "Exception in HostFunction: <unknown>")));
      }

      return hvFromValue(ret);
    }

    static void finalize(void *context) {
      delete reinterpret_cast<HFContext *>(context);
    }
  };

  template <typename T>
  struct ManagedValues {
#ifdef ASSERT_ON_DANGLING_VM_REFS
    // If we have active HermesValuePointers whhen deconstructing, these will
    // now be dangling. We deliberately allocate and immediately leak heap
    // memory to hold the internal list. This keeps alive memory holding the
    // ref-count of the now dangling references, allowing them to detect the
    // dangling case safely and assert when they are eventually released. By
    // deferring the assert it's a bit easier to see what's holding the pointers
    // for too long.
    ~ManagedValues() {
      bool anyDangling = false;
      for (auto it = values.begin(); it != values.end();) {
        if (it->get() == 0) {
          it = values.erase(it);
        } else {
          anyDangling = true;
          it->markDangling();
          ++it;
        }
      }
      if (anyDangling) {
        // This is the deliberate memory leak described above.
        new std::list<T>(std::move(values));
      }
    }
#endif

    std::list<T> *operator->() {
      return &values;
    }

    const std::list<T> *operator->() const {
      return &values;
    }

    std::list<T> values;
  };

 protected:
  /// Helper function that is parameterized over the type of context being
  /// created.
  template <typename ContextType>
  jsi::Function createFunctionFromHostFunction(
      ContextType *context,
      const jsi::PropNameID &name,
      unsigned int paramCount);

 public:
  ManagedValues<HermesPointerValue> hermesValues_;
  ManagedValues<WeakRefPointerValue> weakHermesValues_;
#ifdef HERMESJSI_ON_STACK
  StackRuntime stackRuntime_;
#else
  std::shared_ptr<::hermes::vm::Runtime> rt_;
#endif
  ::hermes::vm::Runtime &runtime_;
#ifdef HERMES_ENABLE_DEBUGGER
  friend class debugger::Debugger;
  std::unique_ptr<debugger::Debugger> debugger_;
#endif
  std::shared_ptr<vm::CrashManager> crashMgr_;

  /// Compilation flags used by prepareJavaScript().
  ::hermes::hbc::CompileFlags compileFlags_{};
};

namespace {

inline HermesRuntimeImpl *impl(HermesRuntime *rt) {
  // This is guaranteed safe because HermesRuntime is abstract so
  // cannot be constructed, and the only instances created are
  // HermesRuntimeImpl's created by the factory function.  It's kind
  // of like pimpl, but different.

  return static_cast<HermesRuntimeImpl *>(rt);
}

inline const HermesRuntimeImpl *impl(const HermesRuntime *rt) {
  // See above comment

  return static_cast<const HermesRuntimeImpl *>(rt);
}

} // namespace

bool HermesRuntime::isHermesBytecode(const uint8_t *data, size_t len) {
  return hbc::BCProviderFromBuffer::isBytecodeStream(
      llvm::ArrayRef<uint8_t>(data, len));
}

void HermesRuntime::prefetchHermesBytecode(const uint8_t *data, size_t len) {
  hbc::BCProviderFromBuffer::prefetch(llvm::ArrayRef<uint8_t>(data, len));
}

bool HermesRuntime::hermesBytecodeSanityCheck(
    const uint8_t *data,
    size_t len,
    std::string *errorMessage) {
  return hbc::BCProviderFromBuffer::bytecodeStreamSanityCheck(
      llvm::ArrayRef<uint8_t>(data, len), errorMessage);
}

std::pair<const uint8_t *, size_t> HermesRuntime::getBytecodeEpilogue(
    const uint8_t *data,
    size_t len) {
  auto epi = hbc::BCProviderFromBuffer::getEpilogueFromBytecode(
      llvm::ArrayRef<uint8_t>(data, len));
  return std::make_pair(epi.data(), epi.size());
}

void HermesRuntime::enableSamplingProfiler() {
  ::hermes::vm::SamplingProfiler::getInstance()->enable();
}

void HermesRuntime::disableSamplingProfiler() {
  ::hermes::vm::SamplingProfiler::getInstance()->disable();
}

void HermesRuntime::dumpSampledTraceToFile(const std::string &fileName) {
  std::error_code ec;
  llvm::raw_fd_ostream os(fileName.c_str(), ec, llvm::sys::fs::F_Text);
  if (ec) {
    throw std::system_error(ec);
  }
  ::hermes::vm::SamplingProfiler::getInstance()->dumpChromeTrace(os);
}

void HermesRuntime::setFatalHandler(void (*handler)(const std::string &)) {
  detail::sApiFatalHandler = handler;
}

namespace {
// A class which adapts a jsi buffer to a Hermes buffer.
class BufferAdapter final : public ::hermes::Buffer {
 public:
  BufferAdapter(std::shared_ptr<const jsi::Buffer> buf) : buf_(std::move(buf)) {
    data_ = buf_->data();
    size_ = buf_->size();
  }

 private:
  std::shared_ptr<const jsi::Buffer> buf_;
};
} // namespace

void HermesRuntime::loadSegment(
    std::unique_ptr<const jsi::Buffer> buffer,
    const jsi::Value &context) {
  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::make_unique<BufferAdapter>(std::move(buffer)));
  if (!ret.first) {
    throw jsi::JSINativeException("Error evaluating javascript: " + ret.second);
  }

  auto requireContext = vm::Handle<vm::RequireContext>::dyn_vmcast(
      impl(this)->vmHandleFromValue(context));
  if (!requireContext) {
    throw jsi::JSINativeException("Error loading segment: Invalid context");
  }

  vm::RuntimeModuleFlags flags;
  flags.persistent = true;
  impl(this)->checkStatus(impl(this)->runtime_.loadSegment(
      std::move(ret.first), requireContext, flags));
}

uint64_t HermesRuntime::getUniqueID(const jsi::Object &o) const {
  return impl(this)->runtime_.getHeap().getObjectID(
      static_cast<vm::GCCell *>(impl(this)->phv(o).getObject()));
}

/// Get a structure representing the enviroment-dependent behavior, so
/// it can be written into the trace for later replay.
const ::hermes::vm::MockedEnvironment &HermesRuntime::getMockedEnvironment()
    const {
  return static_cast<const HermesRuntimeImpl *>(this)
      ->runtime_.getCommonStorage()
      ->tracedEnv;
}

void HermesRuntime::setMockedEnvironment(
    const ::hermes::vm::MockedEnvironment &env) {
  static_cast<HermesRuntimeImpl *>(this)->runtime_.setMockedEnvironment(env);
}

#ifdef HERMESVM_PROFILER_BB
void HermesRuntime::dumpBasicBlockProfileTrace(llvm::raw_ostream &os) const {
  static_cast<const HermesRuntimeImpl *>(this)
      ->runtime_.dumpBasicBlockProfileTrace(os);
}
#endif

#ifdef HERMESVM_PROFILER_OPCODE
void HermesRuntime::dumpOpcodeStats(llvm::raw_ostream &os) const {
  static_cast<const HermesRuntimeImpl *>(this)->runtime_.dumpOpcodeStats(os);
}
#endif

#ifdef HERMES_ENABLE_DEBUGGER

debugger::Debugger &HermesRuntime::getDebugger() {
  return *(impl(this)->debugger_);
}

void HermesRuntime::debugJavaScript(
    const std::string &src,
    const std::string &sourceURL,
    const DebugFlags &debugFlags) {
  vm::Runtime &runtime = impl(this)->runtime_;
  vm::GCScope gcScope(&runtime);
  hbc::CompileFlags flags{};
  flags.debug = true;
  flags.lazy = debugFlags.lazy;
  vm::ExecutionStatus res = runtime.run(src, sourceURL, flags).getStatus();
  impl(this)->checkStatus(res);
}
#endif

void HermesRuntime::registerForProfiling() {
  ::hermes::vm::SamplingProfiler::getInstance()->registerRuntime(
      &(impl(this)->runtime_));
}

void HermesRuntime::unregisterForProfiling() {
  ::hermes::vm::SamplingProfiler::getInstance()->unregisterRuntime(
      &(impl(this)->runtime_));
}

void HermesRuntime::watchTimeLimit(uint32_t timeoutInMs) {
  impl(this)->compileFlags_.emitAsyncBreakCheck = true;
  ::hermes::vm::TimeLimitMonitor::getInstance().watchRuntime(
      &(impl(this)->runtime_), timeoutInMs);
}

void HermesRuntime::unwatchTimeLimit() {
  impl(this)->compileFlags_.emitAsyncBreakCheck = false;
  ::hermes::vm::TimeLimitMonitor::getInstance().unwatchRuntime(
      &(impl(this)->runtime_));
}

size_t HermesRuntime::rootsListLength() const {
  return impl(this)->hermesValues_->size();
}

namespace {

/// An implementation of PreparedJavaScript that wraps a BytecodeProvider.
class HermesPreparedJavaScript final : public jsi::PreparedJavaScript {
  std::shared_ptr<hbc::BCProvider> bcProvider_;
  vm::RuntimeModuleFlags runtimeFlags_;
  std::string sourceURL_;

 public:
  explicit HermesPreparedJavaScript(
      std::unique_ptr<hbc::BCProvider> bcProvider,
      vm::RuntimeModuleFlags runtimeFlags,
      std::string sourceURL)
      : bcProvider_(std::move(bcProvider)),
        runtimeFlags_(runtimeFlags),
        sourceURL_(std::move(sourceURL)) {}

  std::shared_ptr<hbc::BCProvider> bytecodeProvider() const {
    return bcProvider_;
  }

  vm::RuntimeModuleFlags runtimeFlags() const {
    return runtimeFlags_;
  }

  const std::string &sourceURL() const {
    return sourceURL_;
  }
};

} // namespace

std::shared_ptr<const jsi::PreparedJavaScript>
HermesRuntimeImpl::prepareJavaScript(
    const std::shared_ptr<const jsi::Buffer> &jsiBuffer,
    std::string sourceURL) {
  std::pair<std::unique_ptr<hbc::BCProvider>, std::string> bcErr{};
  auto buffer = std::make_unique<BufferAdapter>(std::move(jsiBuffer));
  vm::RuntimeModuleFlags runtimeFlags{};
  runtimeFlags.persistent = true;

  bool isBytecode = isHermesBytecode(buffer->data(), buffer->size());
#ifdef HERMESVM_PLATFORM_LOGGING
  hermesLog(
      "HermesVM", "Prepare JS on %s.", isBytecode ? "bytecode" : "source");
#endif

  // Construct the BC provider either from buffer or source.
  if (isBytecode) {
    bcErr = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
        std::move(buffer));
  } else {
    compileFlags_.lazy =
        (buffer->size() >=
         ::hermes::hbc::kDefaultSizeThresholdForLazyCompilation);
#if defined(HERMESVM_LEAN)
    bcErr.second = "prepareJavaScript source compilation not supported";
#else
    bcErr = hbc::BCProviderFromSrc::createBCProviderFromSrc(
        std::move(buffer), sourceURL, compileFlags_);
#endif
  }
  if (!bcErr.first) {
    throw jsi::JSINativeException(std::move(bcErr.second));
  }
  return std::make_shared<const HermesPreparedJavaScript>(
      std::move(bcErr.first), runtimeFlags, std::move(sourceURL));
}

jsi::Value HermesRuntimeImpl::evaluatePreparedJavaScript(
    const std::shared_ptr<const jsi::PreparedJavaScript> &js) {
  return maybeRethrow([&] {
    assert(
        dynamic_cast<const HermesPreparedJavaScript *>(js.get()) &&
        "js must be an instance of HermesPreparedJavaScript");
    ::hermes::instrumentation::HighFreqPerfMarker m("jsi-hermes-evaluate");
    auto &stats = runtime_.getRuntimeStats();
    const vm::instrumentation::RAIITimer timer{
        "Evaluate JS", stats, stats.evaluateJS};
    const auto *hermesPrep =
        static_cast<const HermesPreparedJavaScript *>(js.get());
    vm::GCScope gcScope(&runtime_);
    auto res = runtime_.runBytecode(
        hermesPrep->bytecodeProvider(),
        hermesPrep->runtimeFlags(),
        hermesPrep->sourceURL(),
        vm::Runtime::makeNullHandle<vm::Environment>());
    checkStatus(res.getStatus());
    return valueFromHermesValue(*res);
  });
}

jsi::Value HermesRuntimeImpl::evaluateJavaScript(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::string &sourceURL) {
  return evaluatePreparedJavaScript(prepareJavaScript(buffer, sourceURL));
}

jsi::Object HermesRuntimeImpl::global() {
  return add<jsi::Object>(runtime_.getGlobal().getHermesValue());
}

std::string HermesRuntimeImpl::description() {
  return runtime_.getHeap().getName();
}

bool HermesRuntimeImpl::isInspectable() {
#ifdef HERMES_ENABLE_DEBUGGER
  return true;
#else
  return false;
#endif
}

jsi::Instrumentation &HermesRuntimeImpl::instrumentation() {
  return *this;
}

jsi::Runtime::PointerValue *HermesRuntimeImpl::cloneSymbol(
    const Runtime::PointerValue *pv) {
  return clone(pv);
}

jsi::Runtime::PointerValue *HermesRuntimeImpl::cloneString(
    const Runtime::PointerValue *pv) {
  return clone(pv);
}

jsi::Runtime::PointerValue *HermesRuntimeImpl::cloneObject(
    const Runtime::PointerValue *pv) {
  return clone(pv);
}

jsi::Runtime::PointerValue *HermesRuntimeImpl::clonePropNameID(
    const Runtime::PointerValue *pv) {
  return clone(pv);
}

jsi::PropNameID HermesRuntimeImpl::createPropNameIDFromAscii(
    const char *str,
    size_t length) {
  return maybeRethrow([&] {
#ifndef NDEBUG
    for (size_t i = 0; i < length; ++i) {
      assert(
          static_cast<unsigned char>(str[i]) < 128 &&
          "non-ASCII character in property name");
    }
#endif

    vm::GCScope gcScope(&runtime_);
    auto cr = vm::stringToSymbolID(
        &runtime_,
        vm::StringPrimitive::createNoThrow(
            &runtime_, llvm::StringRef(str, length)));
    checkStatus(cr.getStatus());
    return add<jsi::PropNameID>(cr->getHermesValue());
  });
}

jsi::PropNameID HermesRuntimeImpl::createPropNameIDFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    auto cr = vm::stringToSymbolID(
        &runtime_,
        vm::createPseudoHandle(stringHVFromUtf8(utf8, length).getString()));
    checkStatus(cr.getStatus());
    return add<jsi::PropNameID>(cr->getHermesValue());
  });
}

jsi::PropNameID HermesRuntimeImpl::createPropNameIDFromString(
    const jsi::String &str) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    auto cr = vm::stringToSymbolID(
        &runtime_, vm::createPseudoHandle(phv(str).getString()));
    checkStatus(cr.getStatus());
    return add<jsi::PropNameID>(cr->getHermesValue());
  });
}

std::string HermesRuntimeImpl::utf8(const jsi::PropNameID &sym) {
  vm::GCScope gcScope(&runtime_);
  vm::SymbolID id = phv(sym).getSymbol();
  auto view = runtime_.getIdentifierTable().getStringView(&runtime_, id);
  vm::SmallU16String<32> allocator;
  std::string ret;
  ::hermes::convertUTF16ToUTF8WithReplacements(
      ret, view.getUTF16Ref(allocator));
  return ret;
}

bool HermesRuntimeImpl::compare(
    const jsi::PropNameID &a,
    const jsi::PropNameID &b) {
  return phv(a).getSymbol() == phv(b).getSymbol();
}

namespace {

std::string toStdString(
    vm::Runtime *runtime,
    vm::Handle<vm::StringPrimitive> handle) {
  auto view = vm::StringPrimitive::createStringView(runtime, handle);
  vm::SmallU16String<32> allocator;
  std::string ret;
  ::hermes::convertUTF16ToUTF8WithReplacements(
      ret, view.getUTF16Ref(allocator));
  return ret;
}

} // namespace

std::string HermesRuntimeImpl::symbolToString(const jsi::Symbol &sym) {
  vm::GCScope gcScope(&runtime_);
  auto res = symbolDescriptiveString(
      &runtime_,
      ::hermes::vm::Handle<::hermes::vm::SymbolID>::vmcast(&phv(sym)));
  checkStatus(res.getStatus());

  return toStdString(&runtime_, res.getValue());
}

jsi::String HermesRuntimeImpl::createStringFromAscii(
    const char *str,
    size_t length) {
  return maybeRethrow([&] {
#ifndef NDEBUG
    for (size_t i = 0; i < length; ++i) {
      assert(
          static_cast<unsigned char>(str[i]) < 128 &&
          "non-ASCII character in string");
    }
#endif
    vm::GCScope gcScope(&runtime_);
    return add<jsi::String>(stringHVFromAscii(str, length));
  });
}

jsi::String HermesRuntimeImpl::createStringFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    return add<jsi::String>(stringHVFromUtf8(utf8, length));
  });
}

std::string HermesRuntimeImpl::utf8(const jsi::String &str) {
  vm::GCScope gcScope(&runtime_);
  return maybeRethrow([&] {
    vm::Handle<vm::StringPrimitive> handle(
        &runtime_, stringHandle(str)->getString());
    return toStdString(&runtime_, handle);
  });
}

static void
convertUtf8ToUtf16(const uint8_t *utf8, size_t length, std::u16string &out) {
  // length is the number of input bytes
  out.resize(length);
  const llvm::UTF8 *sourceStart = (const llvm::UTF8 *)utf8;
  const llvm::UTF8 *sourceEnd = sourceStart + length;
  llvm::UTF16 *targetStart = (llvm::UTF16 *)&out[0];
  llvm::UTF16 *targetEnd = targetStart + out.size();
  llvm::ConversionResult cRes;
  cRes = ConvertUTF8toUTF16(
      &sourceStart,
      sourceEnd,
      &targetStart,
      targetEnd,
      llvm::lenientConversion);
  (void)cRes;
  assert(
      cRes != llvm::ConversionResult::targetExhausted &&
      "not enough space allocated for UTF16 conversion");
  out.resize((char16_t *)targetStart - &out[0]);
}

jsi::Value HermesRuntimeImpl::createValueFromJsonUtf8(
    const uint8_t *json,
    size_t length) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    llvm::ArrayRef<uint8_t> ref(json, length);
    vm::CallResult<vm::HermesValue> res =
        runtimeJSONParseRef(&runtime_, ::hermes::UTF16Stream(ref));
    checkStatus(res.getStatus());
    return valueFromHermesValue(*res);
  });
}

jsi::Object HermesRuntimeImpl::createObject() {
  vm::GCScope gcScope(&runtime_);
  return maybeRethrow([&] {
    return add<jsi::Object>(vm::JSObject::create(&runtime_).getHermesValue());
  });
}

jsi::Object HermesRuntimeImpl::createObject(
    std::shared_ptr<jsi::HostObject> ho) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);

    auto objRes = vm::HostObject::createWithoutPrototype(
        &runtime_, std::make_shared<JsiProxy>(*this, ho));
    checkStatus(objRes.getStatus());
    return add<jsi::Object>(*objRes);
  });
}

std::shared_ptr<jsi::HostObject> HermesRuntimeImpl::getHostObject(
    const jsi::Object &obj) {
  return std::static_pointer_cast<JsiProxyBase>(
             vm::vmcast<vm::HostObject>(phv(obj))->getProxy())
      ->ho_;
}

jsi::Value HermesRuntimeImpl::getProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  return maybeRethrow([&] {
    ::hermes::instrumentation::PerfMarker m("jsi-hermes-getProperty-string");
    vm::GCScope gcScope(&runtime_);
    auto h = handle(obj);
    auto res = h->getComputed_RJS(h, &runtime_, stringHandle(name));
    checkStatus(res.getStatus());
    return valueFromHermesValue(*res);
  });
}

jsi::Value HermesRuntimeImpl::getProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  return maybeRethrow([&] {
    ::hermes::instrumentation::LowFreqPerfMarker m(
        "jsi-hermes-getProperty-nameid");
    vm::GCScope gcScope(&runtime_);
    auto h = handle(obj);
    vm::SymbolID nameID = phv(name).getSymbol();
    auto res = h->getNamedOrIndexed(h, &runtime_, nameID);
    checkStatus(res.getStatus());
    return valueFromHermesValue(*res);
  });
}

bool HermesRuntimeImpl::hasProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  vm::GCScope gcScope(&runtime_);
  auto h = handle(obj);
  auto result = h->hasComputed(h, &runtime_, stringHandle(name));
  checkStatus(result.getStatus());
  return result.getValue();
}

bool HermesRuntimeImpl::hasProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  vm::GCScope gcScope(&runtime_);
  auto h = handle(obj);
  vm::SymbolID nameID = phv(name).getSymbol();
  auto result = h->hasNamedOrIndexed(h, &runtime_, nameID);
  checkStatus(result.getStatus());
  return result.getValue();
}

void HermesRuntimeImpl::setPropertyValue(
    jsi::Object &obj,
    const jsi::String &name,
    const jsi::Value &value) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    auto h = handle(obj);
    checkStatus(h->putComputed_RJS(
                     h,
                     &runtime_,
                     stringHandle(name),
                     vmHandleFromValue(value),
                     vm::PropOpFlags().plusThrowOnError())
                    .getStatus());
  });
}

void HermesRuntimeImpl::setPropertyValue(
    jsi::Object &obj,
    const jsi::PropNameID &name,
    const jsi::Value &value) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    auto h = handle(obj);
    vm::SymbolID nameID = phv(name).getSymbol();
    checkStatus(h->putNamedOrIndexed(
                     h,
                     &runtime_,
                     nameID,
                     vmHandleFromValue(value),
                     vm::PropOpFlags().plusThrowOnError())
                    .getStatus());
  });
}

bool HermesRuntimeImpl::isArray(const jsi::Object &obj) const {
  return vm::vmisa<vm::JSArray>(phv(obj));
}

bool HermesRuntimeImpl::isArrayBuffer(const jsi::Object &obj) const {
  return vm::vmisa<vm::JSArrayBuffer>(phv(obj));
}

bool HermesRuntimeImpl::isFunction(const jsi::Object &obj) const {
  return vm::vmisa<vm::Callable>(phv(obj));
}

bool HermesRuntimeImpl::isHostObject(const jsi::Object &obj) const {
  return vm::vmisa<vm::HostObject>(phv(obj));
}

bool HermesRuntimeImpl::isHostFunction(const jsi::Function &func) const {
  return vm::vmisa<vm::FinalizableNativeFunction>(phv(func));
}

jsi::Array HermesRuntimeImpl::getPropertyNames(const jsi::Object &obj) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    uint32_t beginIndex;
    uint32_t endIndex;
    vm::CallResult<vm::Handle<vm::SegmentedArray>> cr =
        vm::getForInPropertyNames(&runtime_, handle(obj), beginIndex, endIndex);
    checkStatus(cr.getStatus());
    vm::Handle<vm::SegmentedArray> arr = *cr;
    size_t length = endIndex - beginIndex;

    auto ret = createArray(length);
    for (size_t i = 0; i < length; ++i) {
      vm::HermesValue name = arr->at(beginIndex + i);
      if (name.isString()) {
        ret.setValueAtIndex(*this, i, valueFromHermesValue(name));
      } else if (name.isNumber()) {
        std::string s;
        llvm::raw_string_ostream os(s);
        os << static_cast<size_t>(name.getNumber());
        ret.setValueAtIndex(
            *this, i, jsi::String::createFromAscii(*this, os.str()));
      } else {
        llvm_unreachable("property name is not String or Number");
      }
    }

    return ret;
  });
}

jsi::WeakObject HermesRuntimeImpl::createWeakObject(const jsi::Object &obj) {
  return maybeRethrow([&] {
    return addWeak(
        vm::WeakRef<vm::HermesValue>(&(runtime_.getHeap()), phv(obj)));
  });
}

jsi::Value HermesRuntimeImpl::lockWeakObject(const jsi::WeakObject &wo) {
  const vm::WeakRef<vm::HermesValue> &wr = wrhv(wo);
  if (!wr.isValid()) {
    return jsi::Value();
  }

  vm::HermesValue hv = wr.unsafeGetHermesValue();
  assert(hv.isObject() && "jsi::WeakObject referent is not an Object");
  return add<jsi::Object>(hv);
}

jsi::Array HermesRuntimeImpl::createArray(size_t length) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    auto result = vm::JSArray::create(&runtime_, length, length);
    checkStatus(result.getStatus());
    return add<jsi::Object>(result->getHermesValue()).getArray(*this);
  });
}

size_t HermesRuntimeImpl::size(const jsi::Array &arr) {
  vm::GCScope gcScope(&runtime_);
  return getLength(arrayHandle(arr));
}

size_t HermesRuntimeImpl::size(const jsi::ArrayBuffer &arr) {
  vm::GCScope gcScope(&runtime_);
  return getByteLength(arrayBufferHandle(arr));
}

uint8_t *HermesRuntimeImpl::data(const jsi::ArrayBuffer &arr) {
  return vm::vmcast<vm::JSArrayBuffer>(phv(arr))->getDataBlock();
}

jsi::Value HermesRuntimeImpl::getValueAtIndex(const jsi::Array &arr, size_t i) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    if (LLVM_UNLIKELY(i >= size(arr))) {
      throw makeJSError(
          *this,
          "getValueAtIndex: index ",
          i,
          " is out of bounds [0, ",
          size(arr),
          ")");
    }

    auto res = vm::JSObject::getComputed_RJS(
        arrayHandle(arr),
        &runtime_,
        runtime_.makeHandle(vm::HermesValue::encodeNumberValue(i)));
    checkStatus(res.getStatus());

    return valueFromHermesValue(*res);
  });
}

void HermesRuntimeImpl::setValueAtIndexImpl(
    jsi::Array &arr,
    size_t i,
    const jsi::Value &value) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    if (LLVM_UNLIKELY(i >= size(arr))) {
      throw makeJSError(
          *this,
          "setValueAtIndex: index ",
          i,
          " is out of bounds [0, ",
          size(arr),
          ")");
    }

    auto h = arrayHandle(arr);
    h->setElementAt(h, &runtime_, i, vmHandleFromValue(value));
  });
}

jsi::Function HermesRuntimeImpl::createFunctionFromHostFunction(
    const jsi::PropNameID &name,
    unsigned int paramCount,
    jsi::HostFunctionType func) {
  return maybeRethrow([&] {
    auto context = ::hermes::make_unique<HFContext>(std::move(func), *this);
    auto hostfunc =
        createFunctionFromHostFunction(context.get(), name, paramCount);
    context.release();
    return hostfunc;
  });
}

template <typename ContextType>
jsi::Function HermesRuntimeImpl::createFunctionFromHostFunction(
    ContextType *context,
    const jsi::PropNameID &name,
    unsigned int paramCount) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    vm::SymbolID nameID = phv(name).getSymbol();
    auto funcRes = vm::FinalizableNativeFunction::createWithoutPrototype(
        &runtime_,
        context,
        &ContextType::func,
        &ContextType::finalize,
        nameID,
        paramCount);
    checkStatus(funcRes.getStatus());
    jsi::Function ret = add<jsi::Object>(*funcRes).getFunction(*this);
    return ret;
  });
}

jsi::HostFunctionType &HermesRuntimeImpl::getHostFunction(
    const jsi::Function &func) {
  return static_cast<HFContextBase *>(
             vm::vmcast<vm::FinalizableNativeFunction>(phv(func))->getContext())
      ->hostFunction;
}

jsi::Value HermesRuntimeImpl::call(
    const jsi::Function &func,
    const jsi::Value &jsThis,
    const jsi::Value *args,
    size_t count) {
  return maybeRethrow([&] {
    ::hermes::instrumentation::PerfMarker m("jsi-hermes-call");
    vm::GCScope gcScope(&runtime_);
    vm::Handle<vm::Callable> handle =
        vm::Handle<vm::Callable>::vmcast(&phv(func));
    if (count > std::numeric_limits<uint32_t>::max() ||
        !runtime_.checkAvailableStack((uint32_t)count)) {
      throw jsi::JSINativeException(
          "HermesRuntimeImpl::call: Unable to call function: stack overflow");
    }

    auto &stats = runtime_.getRuntimeStats();
    const vm::instrumentation::RAIITimer timer{
        "Incoming Function", stats, stats.incomingFunction};
    vm::ScopedNativeCallFrame newFrame{&runtime_,
                                       static_cast<uint32_t>(count),
                                       handle.getHermesValue(),
                                       vm::HermesValue::encodeUndefinedValue(),
                                       hvFromValue(jsThis)};
    if (LLVM_UNLIKELY(newFrame.overflowed())) {
      checkStatus(runtime_.raiseStackOverflow(
          ::hermes::vm::StackRuntime::StackOverflowKind::NativeStack));
    }

    for (uint32_t i = 0; i != count; ++i) {
      newFrame->getArgRef(i) = hvFromValue(args[i]);
    }
    auto callRes = vm::Callable::call(handle, &runtime_);
    checkStatus(callRes.getStatus());

    return valueFromHermesValue(*callRes);
  });
}

jsi::Value HermesRuntimeImpl::callAsConstructor(
    const jsi::Function &func,
    const jsi::Value *args,
    size_t count) {
  return maybeRethrow([&] {
    ::hermes::instrumentation::PerfMarker m("jsi-hermes-callAsConstructor");
    vm::GCScope gcScope(&runtime_);
    vm::Handle<vm::Callable> funcHandle =
        vm::Handle<vm::Callable>::vmcast(&phv(func));

    if (count > std::numeric_limits<uint32_t>::max() ||
        !runtime_.checkAvailableStack((uint32_t)count)) {
      throw jsi::JSINativeException(
          "HermesRuntimeImpl::call: Unable to call function: stack overflow");
    }

    auto &stats = runtime_.getRuntimeStats();
    const vm::instrumentation::RAIITimer timer{
        "Incoming Function: Call As Constructor",
        stats,
        stats.incomingFunction};

    // We follow es5 13.2.2 [[Construct]] here. Below F == func.
    // 13.2.2.5:
    //    Let proto be the value of calling the [[Get]] internal property of
    //    F with argument "prototype"
    // 13.2.2.6:
    //    If Type(proto) is Object, set the [[Prototype]] internal property
    //    of obj to proto
    // 13.2.2.7:
    //    If Type(proto) is not Object, set the [[Prototype]] internal property
    //    of obj to the standard built-in Object prototype object as described
    //    in 15.2.4
    //
    // Note that 13.2.2.1-4 are also handled by the call to newObject.
    auto thisRes = vm::Callable::createThisForConstruct(funcHandle, &runtime_);
    // We need to capture this in case the ctor doesn't return an object,
    // we need to return this object.
    auto objHandle = runtime_.makeHandle<vm::JSObject>(*thisRes);

    // 13.2.2.8:
    //    Let result be the result of calling the [[Call]] internal property of
    //    F, providing obj as the this value and providing the argument list
    //    passed into [[Construct]] as args.
    //
    // For us result == res.

    vm::ScopedNativeCallFrame newFrame{&runtime_,
                                       static_cast<uint32_t>(count),
                                       funcHandle.getHermesValue(),
                                       funcHandle.getHermesValue(),
                                       objHandle.getHermesValue()};
    if (newFrame.overflowed()) {
      checkStatus(runtime_.raiseStackOverflow(
          ::hermes::vm::StackRuntime::StackOverflowKind::NativeStack));
    }
    for (uint32_t i = 0; i != count; ++i) {
      newFrame->getArgRef(i) = hvFromValue(args[i]);
    }
    // The last parameter indicates that this call should construct an object.
    auto callRes = vm::Callable::call(funcHandle, &runtime_);
    checkStatus(callRes.getStatus());

    // 13.2.2.9:
    //    If Type(result) is Object then return result
    // 13.2.2.10:
    //    Return obj
    auto resultValue = *callRes;
    vm::HermesValue resultHValue =
        resultValue.isObject() ? resultValue : objHandle.getHermesValue();
    return valueFromHermesValue(resultHValue);
  });
}

bool HermesRuntimeImpl::strictEquals(const jsi::Symbol &a, const jsi::Symbol &b)
    const {
  return phv(a).getSymbol() == phv(b).getSymbol();
}

bool HermesRuntimeImpl::strictEquals(const jsi::String &a, const jsi::String &b)
    const {
  return phv(a).getString()->equals(phv(b).getString());
}

bool HermesRuntimeImpl::strictEquals(const jsi::Object &a, const jsi::Object &b)
    const {
  return phv(a).getRaw() == phv(b).getRaw();
}

bool HermesRuntimeImpl::instanceOf(
    const jsi::Object &o,
    const jsi::Function &f) {
  return maybeRethrow([&] {
    vm::GCScope gcScope(&runtime_);
    auto result = vm::instanceOfOperator_RJS(
        &runtime_, runtime_.makeHandle(phv(o)), runtime_.makeHandle(phv(f)));
    checkStatus(result.getStatus());
    return *result;
  });
}

jsi::Runtime::ScopeState *HermesRuntimeImpl::pushScope() {
  hermesValues_->emplace_front(
      vm::HermesValue::encodeNativeValue(kSentinelNativeValue));
  return reinterpret_cast<ScopeState *>(&hermesValues_->front());
}

void HermesRuntimeImpl::popScope(ScopeState *prv) {
  HermesPointerValue *sentinel = reinterpret_cast<HermesPointerValue *>(prv);
  assert(sentinel->phv.isNativeValue());
  assert(sentinel->phv.getNativeValue() == kSentinelNativeValue);

  for (auto it = hermesValues_->begin(); it != hermesValues_->end();) {
    auto &value = *it;

    if (&value == sentinel) {
      hermesValues_->erase(it);
      return;
    }

    if (value.phv.isNativeValue()) {
      // We reached another sentinel value or we started added another native
      // value to the hermesValue_ list. This should not happen.
      std::terminate();
    }

    if (value.get() == 0) {
      it = hermesValues_->erase(it);
    } else {
      ++it;
    }
  }

  // We did not find a sentinel value.
  std::terminate();
}

void HermesRuntimeImpl::checkStatus(vm::ExecutionStatus status) {
  if (LLVM_UNLIKELY(status == vm::ExecutionStatus::EXCEPTION)) {
    jsi::Value exception = valueFromHermesValue(runtime_.getThrownValue());
    runtime_.clearThrownValue();
    throw jsi::JSError(*this, std::move(exception));
  }
}

vm::HermesValue HermesRuntimeImpl::stringHVFromAscii(
    const char *str,
    size_t length) {
  auto strRes = vm::StringPrimitive::createEfficient(
      &runtime_, llvm::makeArrayRef(str, length));
  checkStatus(strRes.getStatus());
  return *strRes;
}

vm::HermesValue HermesRuntimeImpl::stringHVFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  if (::hermes::isAllASCII(utf8, utf8 + length)) {
    return stringHVFromAscii((const char *)utf8, length);
  }
  std::u16string out;
  convertUtf8ToUtf16(utf8, length, out);
  auto strRes = vm::StringPrimitive::createEfficient(&runtime_, std::move(out));
  checkStatus(strRes.getStatus());

  return *strRes;
}

size_t HermesRuntimeImpl::getLength(vm::Handle<vm::ArrayImpl> arr) {
  return maybeRethrow([&] {
    auto res = vm::JSObject::getNamed_RJS(
        arr, &runtime_, vm::Predefined::getSymbolID(vm::Predefined::length));
    checkStatus(res.getStatus());
    if (!res->isNumber()) {
      throw jsi::JSError(*this, "getLength: property 'length' is not a number");
    }
    return static_cast<size_t>(res->getDouble());
  });
}

size_t HermesRuntimeImpl::getByteLength(vm::Handle<vm::JSArrayBuffer> arr) {
  return maybeRethrow([&] {
    auto res = vm::JSObject::getNamed_RJS(
        arr,
        &runtime_,
        vm::Predefined::getSymbolID(vm::Predefined::byteLength));
    checkStatus(res.getStatus());
    if (!res->isNumber()) {
      throw jsi::JSError(
          *this, "getLength: property 'byteLength' is not a number");
    }
    return static_cast<size_t>(res->getDouble());
  });
}

namespace {

class HermesMutex : public std::recursive_mutex {
 public:
  // ThreadSafeRuntimeImpl expects that the lock ctor takes a
  // reference to the Runtime.  Otherwise, this is a
  // std::recursive_mutex.
  HermesMutex(HermesRuntimeImpl &) {}
};

} // namespace

std::unique_ptr<HermesRuntime> makeHermesRuntime(
    const vm::RuntimeConfig &runtimeConfig) {
  // This is insurance against someone adding data members to
  // HermesRuntime.  If on some weird platform it fails, it can be
  // updated or removed.
  static_assert(
      sizeof(HermesRuntime) == sizeof(void *),
      "HermesRuntime should only include a vtable ptr");

#if defined(HERMESVM_PLATFORM_LOGGING)
  auto ret = std::make_unique<HermesRuntimeImpl>(
      runtimeConfig.rebuild()
          .withGCConfig(runtimeConfig.getGCConfig()
                            .rebuild()
                            .withShouldRecordStats(true)
                            .build())
          .build());
#else
  auto ret = std::make_unique<HermesRuntimeImpl>(runtimeConfig);
#endif

#ifdef HERMES_ENABLE_DEBUGGER
  // Only HermesRuntime can create a debugger instance.  This requires
  // the setter and not using make_unique, so the call to new is here
  // in this function, which is a friend of debugger::Debugger.
  ret->setDebugger(std::unique_ptr<debugger::Debugger>(
      new debugger::Debugger(ret.get(), &(ret->runtime_.getDebugger()))));
#endif

  return ret;
}

std::unique_ptr<jsi::ThreadSafeRuntime> makeThreadSafeHermesRuntime(
    const vm::RuntimeConfig &runtimeConfig) {
#if defined(HERMESVM_PLATFORM_LOGGING)
  const vm::RuntimeConfig &actualRuntimeConfig =
      runtimeConfig.rebuild()
          .withGCConfig(runtimeConfig.getGCConfig()
                            .rebuild()
                            .withShouldRecordStats(true)
                            .build())
          .build();
#else
  const vm::RuntimeConfig &actualRuntimeConfig = runtimeConfig;
#endif

  auto ret = std::make_unique<
      jsi::detail::ThreadSafeRuntimeImpl<HermesRuntimeImpl, HermesMutex>>(
      actualRuntimeConfig);

#ifdef HERMES_ENABLE_DEBUGGER
  auto &hermesRt = ret->getUnsafeRuntime();
  // Only HermesRuntime can create a debugger instance.  This requires
  // the setter and not using make_unique, so the call to new is here
  // in this function, which is a friend of debugger::Debugger.
  hermesRt.setDebugger(std::unique_ptr<debugger::Debugger>(
      new debugger::Debugger(&hermesRt, &(hermesRt.runtime_.getDebugger()))));
#endif

  return ret;
}

#ifdef HERMES_ENABLE_DEBUGGER
/// Glue code enabling the Debugger to produce a jsi::Value from a HermesValue.
jsi::Value debugger::Debugger::jsiValueFromHermesValue(vm::HermesValue hv) {
  return impl(runtime_)->valueFromHermesValue(hv);
}
#endif

} // namespace hermes
} // namespace facebook
