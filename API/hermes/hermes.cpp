/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes.h"

#include "llvh/Support/Compiler.h"

#include "hermes/ADT/ManagedChunkedList.h"
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/DebuggerAPI.h"
#include "hermes/Platform/Logging.h"
#include "hermes/Public/JSOutOfMemoryError.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/Support/SimpleDiagHandler.h"
#include "hermes/Support/UTF16Stream.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Debugger/Debugger.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/JSLib/RuntimeJSONUtils.h"
#include "hermes/VM/NativeState.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Profiler/CodeCoverageProfiler.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/SymbolID.h"
#include "hermes/VM/TimeLimitMonitor.h"
#include "hermes/VM/WeakRoot-inline.h"

#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/SHA1.h"
#include "llvh/Support/raw_os_ostream.h"

#include <algorithm>
#include <atomic>
#include <limits>
#include <list>
#include <mutex>
#include <system_error>
#include <unordered_map>

#include <jsi/instrumentation.h>
#include <jsi/threadsafe.h>

#ifdef HERMESVM_LLVM_PROFILE_DUMP
extern "C" {
int __llvm_profile_dump(void);
}
#endif

// Android OSS has a bug where exception data can get mangled when going via
// fbjni. This macro can be used to expose the root cause in adb log. It serves
// no purpose other than as a backup.
#ifdef __ANDROID__
#define LOG_EXCEPTION_CAUSE(...) hermesLog("HermesVM", __VA_ARGS__)
#else
#define LOG_EXCEPTION_CAUSE(...) \
  do {                           \
  } while (0)
#endif

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

void raw_ostream_append(llvh::raw_ostream &os) {}

template <typename Arg0, typename... Args>
void raw_ostream_append(llvh::raw_ostream &os, Arg0 &&arg0, Args &&...args) {
  os << arg0;
  raw_ostream_append(os, args...);
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
      llvh::install_fatal_error_handler(detail::hermesFatalErrorHandler);
      return 0;
    })();
    (void)dummy;
  }
};

#if !HERMESVM_SAMPLING_PROFILER_AVAILABLE
void throwHermesNotCompiledWithSamplingProfilerSupport() {
  throw std::logic_error(
      "Hermes was not compiled with SamplingProfiler support");
}
#endif // !HERMESVM_SAMPLING_PROFILER_AVAILABLE
} // namespace

class HermesRuntimeImpl final : public HermesRuntime,
                                private InstallHermesFatalErrorHandler,
                                private jsi::Instrumentation {
 public:
  static constexpr uint32_t kSentinelNativeValue = 0x6ef71fe1;

  HermesRuntimeImpl(const vm::RuntimeConfig &runtimeConfig)
      : hermesValues_(runtimeConfig.getGCConfig().getOccupancyTarget()),
        weakHermesValues_(runtimeConfig.getGCConfig().getOccupancyTarget()),
        rt_(::hermes::vm::Runtime::create(runtimeConfig)),
        runtime_(*rt_),
        vmExperimentFlags_(runtimeConfig.getVMExperimentFlags()) {
#ifdef HERMES_ENABLE_DEBUGGER
    compileFlags_.debug = true;
#endif

    switch (runtimeConfig.getCompilationMode()) {
      case vm::SmartCompilation:
        compileFlags_.lazy = true;
        // (Leaves thresholds at default values)
        break;
      case vm::ForceEagerCompilation:
        compileFlags_.lazy = false;
        break;
      case vm::ForceLazyCompilation:
        compileFlags_.lazy = true;
        compileFlags_.preemptiveFileCompilationThreshold = 0;
        compileFlags_.preemptiveFunctionCompilationThreshold = 0;
        break;
    }

    compileFlags_.enableBlockScoping = runtimeConfig.getEnableBlockScoping();
    compileFlags_.enableGenerator = runtimeConfig.getEnableGenerator();
    compileFlags_.emitAsyncBreakCheck = defaultEmitAsyncBreakCheck_ =
        runtimeConfig.getAsyncBreakCheckInEval();
    runtime_.addCustomRootsFunction(
        [this](vm::GC *, vm::RootAcceptor &acceptor) {
          hermesValues_.forEach([&acceptor](HermesPointerValue &element) {
            acceptor.accept(element.value());
          });
        });
    runtime_.addCustomWeakRootsFunction(
        [this](vm::GC *, vm::WeakRootAcceptor &acceptor) {
          weakHermesValues_.forEach([&acceptor](WeakRefPointerValue &element) {
            acceptor.acceptWeak(element.value());
          });
        });
#ifdef HERMES_MEMORY_INSTRUMENTATION
    runtime_.addCustomSnapshotFunction(
        [this](vm::HeapSnapshot &snap) {
          snap.beginNode();
          snap.endNode(
              vm::HeapSnapshot::NodeType::Native,
              "ManagedValues",
              vm::GCBase::IDTracker::reserved(
                  vm::GCBase::IDTracker::ReservedObjectID::JSIHermesValueList),
              hermesValues_.capacity() * sizeof(HermesPointerValue),
              0);
          snap.beginNode();
          snap.endNode(
              vm::HeapSnapshot::NodeType::Native,
              "ManagedValues",
              vm::GCBase::IDTracker::reserved(
                  vm::GCBase::IDTracker::ReservedObjectID::
                      JSIWeakHermesValueList),
              weakHermesValues_.capacity() * sizeof(WeakRefPointerValue),
              0);
        },
        [](vm::HeapSnapshot &snap) {
          snap.addNamedEdge(
              vm::HeapSnapshot::EdgeType::Internal,
              "hermesValues",
              vm::GCBase::IDTracker::reserved(
                  vm::GCBase::IDTracker::ReservedObjectID::JSIHermesValueList));
          snap.addNamedEdge(
              vm::HeapSnapshot::EdgeType::Internal,
              "weakHermesValues",
              vm::GCBase::IDTracker::reserved(
                  vm::GCBase::IDTracker::ReservedObjectID::
                      JSIWeakHermesValueList));
        });
#endif // HERMES_MEMORY_INSTRUMENTATION
  }

 public:
  ~HermesRuntimeImpl() override {
#ifdef HERMES_ENABLE_DEBUGGER
    // Deallocate the debugger so it frees any HermesPointerValues it may hold.
    // This must be done before we check hermesValues_ below.
    debugger_.reset();
#endif
  }

  // This should only be called once by the factory.
  void setDebugger(std::unique_ptr<debugger::Debugger> d) {
    debugger_ = std::move(d);
  }

  template <typename T>
  T add(::hermes::vm::HermesValue hv) {
    static_assert(
        std::is_base_of<jsi::Pointer, T>::value, "this type cannot be added");
    return make<T>(&hermesValues_.add(hv));
  }

  jsi::WeakObject addWeak(::hermes::vm::WeakRoot<vm::JSObject> wr) {
    return make<jsi::WeakObject>(&weakHermesValues_.add(wr));
  }

  // overriden from jsi::Instrumentation
  std::string getRecordedGCStats() override {
    std::string s;
    llvh::raw_string_ostream os(s);
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
    BRIDGE_INFO(double, info, externalBytes);
    BRIDGE_INFO(int, info, numMarkStackOverflows);
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
  void collectGarbage(std::string cause) override {
    if ((vmExperimentFlags_ & vm::experiments::IgnoreMemoryWarnings) &&
        cause == "TRIM_MEMORY_RUNNING_CRITICAL") {
      // Do nothing if the GC is a memory warning.
      // TODO(T79835917): Remove this after proving this is the cause of OOMs
      // and finding a better resolution.
      return;
    }
    runtime_.collect(std::move(cause));
  }

  // Overridden from jsi::Instrumentation
  void startTrackingHeapObjectStackTraces(
      std::function<void(
          uint64_t,
          std::chrono::microseconds,
          std::vector<HeapStatsUpdate>)> fragmentCallback) override {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    runtime_.enableAllocationLocationTracker(std::move(fragmentCallback));
#else
    throw std::logic_error(
        "Cannot track heap object stack traces if Hermes isn't "
        "built with memory instrumentation.");
#endif
  }

  // Overridden from jsi::Instrumentation
  void stopTrackingHeapObjectStackTraces() override {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    runtime_.disableAllocationLocationTracker();
#else
    throw std::logic_error(
        "Cannot track heap object stack traces if Hermes isn't "
        "built with memory instrumentation.");
#endif
  }

  // Overridden from jsi::Instrumentation
  void startHeapSampling(size_t samplingInterval) override {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    runtime_.enableSamplingHeapProfiler(samplingInterval);
#else
    throw std::logic_error(
        "Cannot perform heap sampling if Hermes isn't built with "
        "memory instrumentation.");
#endif
  }

  // Overridden from jsi::Instrumentation
  void stopHeapSampling(std::ostream &os) override {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    llvh::raw_os_ostream ros(os);
    runtime_.disableSamplingHeapProfiler(ros);
#else
    throw std::logic_error(
        "Cannot perform heap sampling if Hermes isn't built with "
        " memory instrumentation.");
#endif
  }

  // Overridden from jsi::Instrumentation
  void createSnapshotToFile(const std::string &path) override {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    std::error_code code;
    llvh::raw_fd_ostream os(path, code, llvh::sys::fs::FileAccess::FA_Write);
    if (code) {
      throw std::system_error(code);
    }
    runtime_.getHeap().createSnapshot(os);
#else
    throw std::logic_error(
        "Cannot create heap snapshots if Hermes isn't built with "
        "memory instrumentation.");
#endif
  }

  // Overridden from jsi::Instrumentation
  void createSnapshotToStream(std::ostream &os) override {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    llvh::raw_os_ostream ros(os);
    runtime_.getHeap().createSnapshot(ros);
#else
    throw std::logic_error(
        "Cannot create heap snapshots if Hermes isn't built with "
        "memory instrumentation.");
#endif
  }

  // Overridden from jsi::Instrumentation
  std::string flushAndDisableBridgeTrafficTrace() override {
    throw std::logic_error(
        "Bridge traffic trace is only supported by TracingRuntime");
  }

  // Overridden from jsi::Instrumentation
  void writeBasicBlockProfileTraceToFile(
      const std::string &fileName) const override {
#ifdef HERMESVM_PROFILER_BB
    std::error_code ec;
    llvh::raw_fd_ostream os(fileName.c_str(), ec, llvh::sys::fs::F_Text);
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

  void dumpProfilerSymbolsToFile(const std::string &fileName) const override {
    throw std::logic_error(
        "Cannot dump profiler symbols out if Hermes wasn't built with "
        "hermes.profiler=EXTERN");
  }

  // These are all methods which do pointer type gymnastics and should
  // mostly inline and optimize away.

  static const ::hermes::vm::PinnedHermesValue &phv(
      const jsi::Pointer &pointer) {
    assert(
        dynamic_cast<const HermesPointerValue *>(getPointerValue(pointer)) &&
        "Pointer does not contain a HermesPointerValue");
    return static_cast<const HermesPointerValue *>(getPointerValue(pointer))
        ->value();
  }

  static const ::hermes::vm::PinnedHermesValue &phv(const jsi::Value &value) {
    assert(
        dynamic_cast<const HermesPointerValue *>(getPointerValue(value)) &&
        "Pointer does not contain a HermesPointerValue");
    return static_cast<const HermesPointerValue *>(getPointerValue(value))
        ->value();
  }

  static ::hermes::vm::Handle<vm::StringPrimitive> stringHandle(
      const jsi::String &str) {
    return ::hermes::vm::Handle<vm::StringPrimitive>::vmcast(&phv(str));
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

  static const ::hermes::vm::WeakRoot<vm::JSObject> &weakRoot(
      const jsi::Pointer &pointer) {
    assert(
        dynamic_cast<const WeakRefPointerValue *>(getPointerValue(pointer)) &&
        "Pointer does not contain a WeakRefPointerValue");
    return static_cast<const WeakRefPointerValue *>(getPointerValue(pointer))
        ->value();
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
      return vm::HermesValue::encodeUntrustedNumberValue(value.getNumber());
    } else if (
        value.isSymbol() || value.isBigInt() || value.isString() ||
        value.isObject()) {
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
          vm::HermesValue::encodeUntrustedNumberValue(value.getNumber()));
    } else if (
        value.isSymbol() || value.isBigInt() || value.isString() ||
        value.isObject()) {
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
    } else if (hv.isBigInt()) {
      return add<jsi::BigInt>(hv);
    } else if (hv.isString()) {
      return add<jsi::String>(hv);
    } else if (hv.isObject()) {
      return add<jsi::Object>(hv);
    } else {
      llvm_unreachable("unknown HermesValue type");
    }
  }

  /// Same as \c prepareJavaScript but with a source map.
  std::shared_ptr<const jsi::PreparedJavaScript> prepareJavaScriptWithSourceMap(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::shared_ptr<const jsi::Buffer> &sourceMapBuf,
      std::string sourceURL);

  // Concrete declarations of jsi::Runtime pure virtual methods

  std::shared_ptr<const jsi::PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      std::string sourceURL) override;
  jsi::Value evaluatePreparedJavaScript(
      const std::shared_ptr<const jsi::PreparedJavaScript> &js) override;
  jsi::Value evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override;
  bool drainMicrotasks(int maxMicrotasksHint = -1) override;
  jsi::Object global() override;

  std::string description() override;
  bool isInspectable() override;
  jsi::Instrumentation &instrumentation() override;

  PointerValue *cloneSymbol(const Runtime::PointerValue *pv) override;
  PointerValue *cloneBigInt(const Runtime::PointerValue *pv) override;
  PointerValue *cloneString(const Runtime::PointerValue *pv) override;
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override;
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override;

  jsi::PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override;
  jsi::PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override;
  jsi::PropNameID createPropNameIDFromString(const jsi::String &str) override;
  jsi::PropNameID createPropNameIDFromSymbol(const jsi::Symbol &sym) override;
  std::string utf8(const jsi::PropNameID &) override;
  bool compare(const jsi::PropNameID &, const jsi::PropNameID &) override;

  std::string symbolToString(const jsi::Symbol &) override;

  jsi::BigInt createBigIntFromInt64(int64_t) override;
  jsi::BigInt createBigIntFromUint64(uint64_t) override;
  bool bigintIsInt64(const jsi::BigInt &) override;
  bool bigintIsUint64(const jsi::BigInt &) override;
  uint64_t truncate(const jsi::BigInt &) override;
  jsi::String bigintToString(const jsi::BigInt &, int) override;

  jsi::String createStringFromAscii(const char *str, size_t length) override;
  jsi::String createStringFromUtf8(const uint8_t *utf8, size_t length) override;
  std::string utf8(const jsi::String &) override;

  jsi::Value createValueFromJsonUtf8(const uint8_t *json, size_t length)
      override;

  jsi::Object createObject() override;
  jsi::Object createObject(std::shared_ptr<jsi::HostObject> ho) override;
  std::shared_ptr<jsi::HostObject> getHostObject(const jsi::Object &) override;
  jsi::HostFunctionType &getHostFunction(const jsi::Function &) override;
  bool hasNativeState(const jsi::Object &) override;
  std::shared_ptr<jsi::NativeState> getNativeState(
      const jsi::Object &) override;
  void setNativeState(const jsi::Object &, std::shared_ptr<jsi::NativeState>)
      override;
  jsi::Value getProperty(const jsi::Object &, const jsi::PropNameID &name)
      override;
  jsi::Value getProperty(const jsi::Object &, const jsi::String &name) override;
  bool hasProperty(const jsi::Object &, const jsi::PropNameID &name) override;
  bool hasProperty(const jsi::Object &, const jsi::String &name) override;
  void setPropertyValue(
      const jsi::Object &,
      const jsi::PropNameID &name,
      const jsi::Value &value) override;
  void setPropertyValue(
      const jsi::Object &,
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
  jsi::ArrayBuffer createArrayBuffer(
      std::shared_ptr<jsi::MutableBuffer> buffer) override;
  size_t size(const jsi::Array &) override;
  size_t size(const jsi::ArrayBuffer &) override;
  uint8_t *data(const jsi::ArrayBuffer &) override;
  jsi::Value getValueAtIndex(const jsi::Array &, size_t i) override;
  void setValueAtIndexImpl(
      const jsi::Array &,
      size_t i,
      const jsi::Value &value) override;

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
  bool strictEquals(const jsi::BigInt &a, const jsi::BigInt &b) const override;
  bool strictEquals(const jsi::String &a, const jsi::String &b) const override;
  bool strictEquals(const jsi::Object &a, const jsi::Object &b) const override;

  bool instanceOf(const jsi::Object &o, const jsi::Function &ctor) override;

  ScopeState *pushScope() override;
  void popScope(ScopeState *prv) override;

  void checkStatus(vm::ExecutionStatus);
  vm::HermesValue stringHVFromAscii(const char *ascii, size_t length);
  vm::HermesValue stringHVFromUtf8(const uint8_t *utf8, size_t length);
  std::string utf8FromStringView(vm::StringView view);

  struct JsiProxy final : public vm::HostObjectProxy {
    HermesRuntimeImpl &rt_;
    std::shared_ptr<jsi::HostObject> ho_;

    JsiProxy(HermesRuntimeImpl &rt, std::shared_ptr<jsi::HostObject> ho)
        : rt_(rt), ho_(ho) {}

    vm::CallResult<vm::HermesValue> get(vm::SymbolID id) override {
      jsi::PropNameID sym =
          rt_.add<jsi::PropNameID>(vm::HermesValue::encodeSymbolValue(id));
      jsi::Value ret;
      try {
        ret = ho_->get(rt_, sym);
      }
#ifdef HERMESVM_EXCEPTION_ON_OOM
      catch (const vm::JSOutOfMemoryError &) {
        throw;
      }
#endif
      catch (const jsi::JSError &error) {
        return rt_.runtime_.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt_.runtime_.raiseError(
            vm::TwineChar16{"Exception in HostObject::get for prop '"} +
            rt_.runtime_.getIdentifierTable().getStringViewForDev(
                rt_.runtime_, id) +
            "': " + ex.what());
      } catch (...) {
        return rt_.runtime_.raiseError(
            vm::TwineChar16{"Exception in HostObject::get: for prop '"} +
            rt_.runtime_.getIdentifierTable().getStringViewForDev(
                rt_.runtime_, id) +
            "': <unknown exception>");
      }

      return hvFromValue(ret);
    }

    vm::CallResult<bool> set(vm::SymbolID id, vm::HermesValue value) override {
      jsi::PropNameID sym =
          rt_.add<jsi::PropNameID>(vm::HermesValue::encodeSymbolValue(id));
      try {
        ho_->set(rt_, sym, rt_.valueFromHermesValue(value));
      }
#ifdef HERMESVM_EXCEPTION_ON_OOM
      catch (const vm::JSOutOfMemoryError &) {
        throw;
      }
#endif
      catch (const jsi::JSError &error) {
        return rt_.runtime_.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt_.runtime_.raiseError(
            vm::TwineChar16{"Exception in HostObject::set for prop '"} +
            rt_.runtime_.getIdentifierTable().getStringViewForDev(
                rt_.runtime_, id) +
            "': " + ex.what());
      } catch (...) {
        return rt_.runtime_.raiseError(
            vm::TwineChar16{"Exception in HostObject::set: for prop '"} +
            rt_.runtime_.getIdentifierTable().getStringViewForDev(
                rt_.runtime_, id) +
            "': <unknown exception>");
      }
      return true;
    }

    vm::CallResult<vm::Handle<vm::JSArray>> getHostPropertyNames() override {
      try {
        auto names = ho_->getPropertyNames(rt_);

        auto arrayRes =
            vm::JSArray::create(rt_.runtime_, names.size(), names.size());
        if (arrayRes == vm::ExecutionStatus::EXCEPTION) {
          return vm::ExecutionStatus::EXCEPTION;
        }
        vm::Handle<vm::JSArray> arrayHandle = *arrayRes;

        vm::GCScope gcScope{rt_.runtime_};
        vm::MutableHandle<vm::SymbolID> tmpHandle{rt_.runtime_};
        size_t i = 0;
        for (auto &name : names) {
          tmpHandle = phv(name).getSymbol();
          vm::JSArray::setElementAt(arrayHandle, rt_.runtime_, i++, tmpHandle);
        }

        return arrayHandle;
      }
#ifdef HERMESVM_EXCEPTION_ON_OOM
      catch (const vm::JSOutOfMemoryError &) {
        throw;
      }
#endif
      catch (const jsi::JSError &error) {
        return rt_.runtime_.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return rt_.runtime_.raiseError(
            vm::TwineChar16{"Exception in HostObject::getPropertyNames: "} +
            ex.what());
      } catch (...) {
        return rt_.runtime_.raiseError(vm::TwineChar16{
            "Exception in HostObject::getPropertyNames: <unknown>"});
      }
    };
  };

  struct HFContext final {
    HFContext(jsi::HostFunctionType hf, HermesRuntimeImpl &hri)
        : hostFunction(std::move(hf)), hermesRuntimeImpl(hri) {}

    static vm::CallResult<vm::HermesValue>
    func(void *context, vm::Runtime &runtime, vm::NativeArgs hvArgs) {
      HFContext *hfc = reinterpret_cast<HFContext *>(context);
      HermesRuntimeImpl &rt = hfc->hermesRuntimeImpl;
      assert(&runtime == &rt.runtime_);

      llvh::SmallVector<jsi::Value, 8> apiArgs;
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
      }
#ifdef HERMESVM_EXCEPTION_ON_OOM
      catch (const vm::JSOutOfMemoryError &) {
        throw;
      }
#endif
      catch (const jsi::JSError &error) {
        return runtime.setThrownValue(hvFromValue(error.value()));
      } catch (const std::exception &ex) {
        return runtime.raiseError(
            vm::TwineChar16{"Exception in HostFunction: "} + ex.what());
      } catch (...) {
        return runtime.raiseError("Exception in HostFunction: <unknown>");
      }

      return hvFromValue(ret);
    }

    static void finalize(void *context) {
      delete reinterpret_cast<HFContext *>(context);
    }

    jsi::HostFunctionType hostFunction;
    HermesRuntimeImpl &hermesRuntimeImpl;
  };

  // A ManagedChunkedList element that indicates whether it's occupied based on
  // a refcount.
  template <typename T>
  struct ManagedValue : PointerValue {
    ManagedValue() : refCount_(0) {}

    // Determine whether the element is occupied by inspecting the refcount.
    bool isFree() const {
      return refCount_.load(std::memory_order_relaxed) == 0;
    }

    // Store a value and start the refcount at 1. After invocation, this
    // instance is occupied with a value, and the "nextFree" methods should
    // not be used until the value is released.
    template <typename... Args>
    void emplace(Args &&...args) {
      assert(isFree() && "Emplacing already occupied value");
      refCount_.store(1, std::memory_order_relaxed);
      new (&value_) T(std::forward<Args>(args)...);
    }

    // Get the next free element. Must not be called when this instance is
    // occupied with a value.
    ManagedValue<T> *getNextFree() {
      assert(isFree() && "Free pointer unusuable while occupied");
      return nextFree_;
    }

    // Set the next free element. Must not be called when this instance is
    // occupied with a value.
    void setNextFree(ManagedValue<T> *nextFree) {
      assert(isFree() && "Free pointer unusuable while occupied");
      nextFree_ = nextFree;
    }

    T &value() {
      assert(!isFree() && "Value not present");
      return value_;
    }

    const T &value() const {
      assert(!isFree() && "Value not present");
      return value_;
    }

    void invalidate() override {
#ifdef ASSERT_ON_DANGLING_VM_REFS
      assert(
          ((1 << 31) & refCount_) == 0 &&
          "This PointerValue was left dangling after the Runtime was destroyed.");
#endif
      dec();
    }

    void inc() {
      // It is always safe to use relaxed operations for incrementing the
      // reference count, because the only operation that may occur concurrently
      // with it is decrementing the reference count, and we do not need to
      // enforce any ordering between the two.
      auto oldCount = refCount_.fetch_add(1, std::memory_order_relaxed);
      assert(oldCount && "Cannot resurrect a pointer");
      assert(oldCount + 1 != 0 && "Ref count overflow");
      (void)oldCount;
    }

    void dec() {
      // It is safe to use relaxed operations here because decrementing the
      // reference count is the only access that may be performed without proper
      // synchronisation. As a result, the only ordering we need to enforce when
      // decrementing is that the vtable pointer used to call \c invalidate is
      // loaded from before the decrement, in case the decrement ends up causing
      // this value to be freed. We get this ordering from the fact that the
      // vtable read and the reference count update form a load-store control
      // dependency, which preserves their ordering on any reasonable hardware.
      auto oldCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
      assert(oldCount > 0 && "Ref count underflow");
      (void)oldCount;
    }

#ifdef ASSERT_ON_DANGLING_VM_REFS
    void markDangling() {
      // Mark this PointerValue as dangling by setting the top bit AND the
      // second-top bit. The top bit is used to determine if the pointer is
      // dangling. Setting the second-top bit ensures that accidental
      // over-calling the dec() function doesn't clear the top bit without
      // complicating the implementation of dec().
      refCount_ |= 0b11 << 30;
    }
#endif

   private:
    std::atomic<uint32_t> refCount_;
    union {
      T value_;
      ManagedValue<T> *nextFree_;
    };
  };

  template <typename T>
  class ManagedValues : public ::hermes::ManagedChunkedList<ManagedValue<T>> {
    static constexpr double kSizingWeight_ = 0.5;

   public:
    explicit ManagedValues(double occupancyRatio)
        : ::hermes::ManagedChunkedList<ManagedValue<T>>(
              occupancyRatio,
              kSizingWeight_) {}

#ifdef ASSERT_ON_DANGLING_VM_REFS
    // If we have active HermesValuePointers when deconstructing, these will
    // now be dangling. We deliberately allocate and immediately leak heap
    // memory to hold the internal list. This keeps alive memory holding the
    // ref-count of the now dangling references, allowing them to detect the
    // dangling case safely and assert when they are eventually released. By
    // deferring the assert it's a bit easier to see what's holding the pointers
    // for too long.
    ~ManagedValues() {
      ::hermes::ManagedChunkedList<ManagedValue<T>>::collect();
      bool empty = true;
      ::hermes::ManagedChunkedList<ManagedValue<T>>::forEach(
          [&empty](ManagedValue<T> &element) {
            element.markDangling();
            empty = false;
          });
      if (!empty) {
        // This is the deliberate memory leak described above.
        new ::hermes::ManagedChunkedList(std::move(*this));
      }
    }
#endif
  };

  using HermesPointerValue = ManagedValue<vm::PinnedHermesValue>;
  using WeakRefPointerValue = ManagedValue<vm::WeakRoot<vm::JSObject>>;

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

 protected:
  /// Helper function that is parameterized over the type of context being
  /// created.
  template <typename ContextType>
  jsi::Function createFunctionFromHostFunction(
      ContextType *context,
      const jsi::PropNameID &name,
      unsigned int paramCount);

  /// Throw the exception stored in the Runtime as a jsi::JSError.
  LLVM_ATTRIBUTE_NORETURN void throwPendingError();

  /// Throw a jsi::JSError with a message created by concatenating the string
  /// representations of \p args.
  template <typename... Args>
  LLVM_ATTRIBUTE_NORETURN void throwJSErrorWithMessage(Args &&...args);

 public:
  ManagedValues<vm::PinnedHermesValue> hermesValues_;
  ManagedValues<vm::WeakRoot<vm::JSObject>> weakHermesValues_;
  std::shared_ptr<::hermes::vm::Runtime> rt_;
  ::hermes::vm::Runtime &runtime_;
  friend class debugger::Debugger;
  std::unique_ptr<debugger::Debugger> debugger_;
  ::hermes::vm::experiments::VMExperimentFlags vmExperimentFlags_{0};

  /// Compilation flags used by prepareJavaScript().
  ::hermes::hbc::CompileFlags compileFlags_{};
  /// The default setting of "emit async break check" in this runtime.
  bool defaultEmitAsyncBreakCheck_{false};
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
      llvh::ArrayRef<uint8_t>(data, len));
}

uint32_t HermesRuntime::getBytecodeVersion() {
  return hbc::BYTECODE_VERSION;
}

void HermesRuntime::prefetchHermesBytecode(const uint8_t *data, size_t len) {
  hbc::BCProviderFromBuffer::prefetch(llvh::ArrayRef<uint8_t>(data, len));
}

bool HermesRuntime::hermesBytecodeSanityCheck(
    const uint8_t *data,
    size_t len,
    std::string *errorMessage) {
  return hbc::BCProviderFromBuffer::bytecodeStreamSanityCheck(
      llvh::ArrayRef<uint8_t>(data, len), errorMessage);
}

std::pair<const uint8_t *, size_t> HermesRuntime::getBytecodeEpilogue(
    const uint8_t *data,
    size_t len) {
  auto epi = hbc::BCProviderFromBuffer::getEpilogueFromBytecode(
      llvh::ArrayRef<uint8_t>(data, len));
  return std::make_pair(epi.data(), epi.size());
}

void HermesRuntime::enableSamplingProfiler() {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  ::hermes::vm::SamplingProfiler::enable();
#else
  throwHermesNotCompiledWithSamplingProfilerSupport();
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

void HermesRuntime::disableSamplingProfiler() {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  ::hermes::vm::SamplingProfiler::disable();
#else
  throwHermesNotCompiledWithSamplingProfilerSupport();
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

void HermesRuntime::dumpSampledTraceToFile(const std::string &fileName) {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  std::error_code ec;
  llvh::raw_fd_ostream os(fileName.c_str(), ec, llvh::sys::fs::F_Text);
  if (ec) {
    throw std::system_error(ec);
  }
  ::hermes::vm::SamplingProfiler::dumpChromeTraceGlobal(os);
#else
  throw std::logic_error(
      "Hermes was not compiled with SamplingProfilerSupport");
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

void HermesRuntime::dumpSampledTraceToStream(std::ostream &stream) {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  llvh::raw_os_ostream os(stream);
  ::hermes::vm::SamplingProfiler::dumpChromeTraceGlobal(os);
#else
  throwHermesNotCompiledWithSamplingProfilerSupport();
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

void HermesRuntime::sampledTraceToStreamInDevToolsFormat(std::ostream &stream) {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  vm::SamplingProfiler *sp = impl(this)->runtime_.samplingProfiler.get();
  if (!sp) {
    throw jsi::JSINativeException("Runtime not registered for profiling");
  }
  llvh::raw_os_ostream os(stream);
  sp->serializeInDevToolsFormat(os);
#else
  throwHermesNotCompiledWithSamplingProfilerSupport();
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

/*static*/ std::unordered_map<std::string, std::vector<std::string>>
HermesRuntime::getExecutedFunctions() {
  std::unordered_map<
      std::string,
      std::vector<::hermes::vm::CodeCoverageProfiler::FuncInfo>>
      executedFunctionsByVM =
          ::hermes::vm::CodeCoverageProfiler::getExecutedFunctions();
  std::unordered_map<std::string, std::vector<std::string>> result;

  for (auto const &x : executedFunctionsByVM) {
    std::vector<std::string> res;
    std::transform(
        x.second.begin(),
        x.second.end(),
        std::back_inserter(res),
        [](const ::hermes::vm::CodeCoverageProfiler::FuncInfo &entry) {
          std::stringstream ss;
          ss << entry.moduleId;
          ss << ":";
          ss << entry.funcVirtualOffset;
          ss << ":";
          ss << entry.debugInfo;
          return ss.str();
        });
    result.emplace(x.first, res);
  }
  return result;
}

/*static*/ bool HermesRuntime::isCodeCoverageProfilerEnabled() {
  return ::hermes::vm::CodeCoverageProfiler::globallyEnabled();
}

/*static*/ void HermesRuntime::enableCodeCoverageProfiler() {
  ::hermes::vm::CodeCoverageProfiler::enableGlobal();
}

/*static*/ void HermesRuntime::disableCodeCoverageProfiler() {
  ::hermes::vm::CodeCoverageProfiler::disableGlobal();
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
    LOG_EXCEPTION_CAUSE("Error evaluating javascript: %s", ret.second.c_str());
    throw jsi::JSINativeException("Error evaluating javascript: " + ret.second);
  }

  auto requireContext = vm::Handle<vm::RequireContext>::dyn_vmcast(
      impl(this)->vmHandleFromValue(context));
  if (!requireContext) {
    LOG_EXCEPTION_CAUSE("Error loading segment: Invalid context");
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
uint64_t HermesRuntime::getUniqueID(const jsi::BigInt &s) const {
  return impl(this)->runtime_.getHeap().getObjectID(
      static_cast<vm::GCCell *>(impl(this)->phv(s).getBigInt()));
}
uint64_t HermesRuntime::getUniqueID(const jsi::String &s) const {
  return impl(this)->runtime_.getHeap().getObjectID(
      static_cast<vm::GCCell *>(impl(this)->phv(s).getString()));
}

// TODO(T111638575): PropNameID and Symbol can have the same unique ID. We
// should either add a way to distinguish them, or explicitly state that the
// unique ID may not be used to distinguish a PropNameID from a Value.
uint64_t HermesRuntime::getUniqueID(const jsi::PropNameID &pni) const {
  return impl(this)->runtime_.getHeap().getObjectID(
      impl(this)->phv(pni).getSymbol());
}
uint64_t HermesRuntime::getUniqueID(const jsi::Symbol &sym) const {
  return impl(this)->runtime_.getHeap().getObjectID(
      impl(this)->phv(sym).getSymbol());
}

uint64_t HermesRuntime::getUniqueID(const jsi::Value &val) const {
  vm::HermesValue hv = HermesRuntimeImpl::hvFromValue(val);
  // 0 is reserved as a non-ID.
  return impl(this)->runtime_.getHeap().getSnapshotID(hv).getValueOr(0);
}

jsi::Value HermesRuntime::getObjectForID(uint64_t id) {
  vm::GCCell *ptr = static_cast<vm::GCCell *>(
      impl(this)->runtime_.getHeap().getObjectForID(id));
  if (ptr && vm::vmisa<vm::JSObject>(ptr)) {
    return impl(this)->add<jsi::Object>(
        vm::HermesValue::encodeObjectValue(ptr));
  }
  // If the ID doesn't map to a pointer, or that pointer isn't an object,
  // return null.
  // This is because a jsi::Object can't be used to represent something internal
  // to the VM like a HiddenClass.
  return jsi::Value::null();
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

const ::hermes::vm::GCExecTrace &HermesRuntime::getGCExecTrace() const {
  return static_cast<const HermesRuntimeImpl *>(this)
      ->runtime_.getGCExecTrace();
}

std::string HermesRuntime::getIOTrackingInfoJSON() {
  std::string buf;
  llvh::raw_string_ostream strstrm(buf);
  static_cast<HermesRuntimeImpl *>(this)->runtime_.getIOTrackingInfoJSON(
      strstrm);
  strstrm.flush();
  return buf;
}

#ifdef HERMESVM_PROFILER_BB
void HermesRuntime::dumpBasicBlockProfileTrace(std::ostream &stream) const {
  llvh::raw_os_ostream os(stream);
  static_cast<const HermesRuntimeImpl *>(this)
      ->runtime_.dumpBasicBlockProfileTrace(os);
}
#endif

#ifdef HERMESVM_PROFILER_OPCODE
void HermesRuntime::dumpOpcodeStats(std::ostream &stream) const {
  llvh::raw_os_ostream os(stream);
  static_cast<const HermesRuntimeImpl *>(this)->runtime_.dumpOpcodeStats(os);
}
#endif

debugger::Debugger &HermesRuntime::getDebugger() {
  return *(impl(this)->debugger_);
}

#ifdef HERMES_ENABLE_DEBUGGER

void HermesRuntime::debugJavaScript(
    const std::string &src,
    const std::string &sourceURL,
    const DebugFlags &debugFlags) {
  vm::Runtime &runtime = impl(this)->runtime_;
  vm::GCScope gcScope(runtime);
  vm::ExecutionStatus res =
      runtime.run(src, sourceURL, impl(this)->compileFlags_).getStatus();
  impl(this)->checkStatus(res);
}

#endif

void HermesRuntime::registerForProfiling() {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  vm::Runtime &runtime = impl(this)->runtime_;
  if (runtime.samplingProfiler) {
    ::hermes::hermes_fatal(
        "re-registering HermesVMs for profiling is not allowed");
  }
  runtime.samplingProfiler = ::hermes::vm::SamplingProfiler::create(runtime);
#else
  throwHermesNotCompiledWithSamplingProfilerSupport();
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

void HermesRuntime::unregisterForProfiling() {
#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  if (!impl(this)->runtime_.samplingProfiler) {
    ::hermes::hermes_fatal(
        "unregistering HermesVM not registered for profiling is not allowed");
  }
  impl(this)->runtime_.samplingProfiler.reset();
#else
  throwHermesNotCompiledWithSamplingProfilerSupport();
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
}

void HermesRuntime::asyncTriggerTimeout() {
  impl(this)->runtime_.triggerTimeoutAsyncBreak();
}

void HermesRuntime::watchTimeLimit(uint32_t timeoutInMs) {
  HermesRuntimeImpl &concrete = *impl(this);
  vm::Runtime &runtime = concrete.runtime_;
  auto &runtimeTimeLimitMonitor = runtime.timeLimitMonitor;
  if (!runtimeTimeLimitMonitor) {
    concrete.compileFlags_.emitAsyncBreakCheck = true;
    runtimeTimeLimitMonitor = ::hermes::vm::TimeLimitMonitor::getOrCreate();
  }
  runtimeTimeLimitMonitor->watchRuntime(
      runtime, std::chrono::milliseconds(timeoutInMs));
}

void HermesRuntime::unwatchTimeLimit() {
  vm::Runtime &runtime = impl(this)->runtime_;
  if (auto &runtimeTimeLimitMonitor = runtime.timeLimitMonitor) {
    runtimeTimeLimitMonitor->unwatchRuntime(runtime);
  }
}

jsi::Value HermesRuntime::evaluateJavaScriptWithSourceMap(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::shared_ptr<const jsi::Buffer> &sourceMapBuf,
    const std::string &sourceURL) {
  return impl(this)->evaluatePreparedJavaScript(
      impl(this)->prepareJavaScriptWithSourceMap(
          buffer, sourceMapBuf, sourceURL));
}

size_t HermesRuntime::rootsListLengthForTests() const {
  return impl(this)->hermesValues_.sizeForTests();
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
HermesRuntimeImpl::prepareJavaScriptWithSourceMap(
    const std::shared_ptr<const jsi::Buffer> &jsiBuffer,
    const std::shared_ptr<const jsi::Buffer> &sourceMapBuf,
    std::string sourceURL) {
  std::pair<std::unique_ptr<hbc::BCProvider>, std::string> bcErr{};
  auto buffer = std::make_unique<BufferAdapter>(jsiBuffer);
  vm::RuntimeModuleFlags runtimeFlags{};
  runtimeFlags.persistent = true;

  bool isBytecode = isHermesBytecode(buffer->data(), buffer->size());
#ifdef HERMESVM_PLATFORM_LOGGING
  hermesLog(
      "HermesVM", "Prepare JS on %s.", isBytecode ? "bytecode" : "source");
#endif
  // Save the first few bytes of the buffer so that we can later append them
  // to any error message.
  uint8_t bufPrefix[16];
  const size_t bufSize = buffer->size();
  memcpy(bufPrefix, buffer->data(), std::min(sizeof(bufPrefix), bufSize));

  // Construct the BC provider either from buffer or source.
  if (isBytecode) {
    if (sourceMapBuf) {
      throw std::logic_error("Source map cannot be specified with bytecode");
    }
    bcErr = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
        std::move(buffer));
  } else {
#if defined(HERMESVM_LEAN)
    bcErr.second = "prepareJavaScript source compilation not supported";
#else
    std::unique_ptr<::hermes::SourceMap> sourceMap{};
    if (sourceMapBuf) {
      // Convert the buffer into a form the parser needs.
      llvh::MemoryBufferRef mbref(
          llvh::StringRef(
              (const char *)sourceMapBuf->data(), sourceMapBuf->size()),
          "");
      ::hermes::SimpleDiagHandler diag;
      ::hermes::SourceErrorManager sm;
      diag.installInto(sm);
      sourceMap = ::hermes::SourceMapParser::parse(mbref, sm);
      if (!sourceMap) {
        auto errorStr = diag.getErrorString();
        LOG_EXCEPTION_CAUSE("Error parsing source map: %s", errorStr.c_str());
        throw std::runtime_error("Error parsing source map:" + errorStr);
      }
    }
    bcErr = hbc::BCProviderFromSrc::createBCProviderFromSrc(
        std::move(buffer), sourceURL, std::move(sourceMap), compileFlags_);
#endif
  }
  if (!bcErr.first) {
    std::string storage;
    llvh::raw_string_ostream os(storage);
    os << " Buffer size " << bufSize << " starts with: ";
    for (size_t i = 0; i < sizeof(bufPrefix) && i < bufSize; ++i)
      os << llvh::format_hex_no_prefix(bufPrefix[i], 2);
    std::string bufferModes = "";
    for (const auto &mode : ::hermes::oscompat::get_vm_protect_modes(
             jsiBuffer->data(), jsiBuffer->size())) {
      // We only expect one match, but if there are multiple, we want to know.
      bufferModes += mode;
    }
    if (!bufferModes.empty()) {
      os << " and has protection mode(s): " << bufferModes;
    }
    LOG_EXCEPTION_CAUSE(
        "Compiling JS failed: %s, %s", bcErr.second.c_str(), os.str().c_str());
    throw jsi::JSINativeException(
        "Compiling JS failed: " + std::move(bcErr.second) + os.str());
  }
  return std::make_shared<const HermesPreparedJavaScript>(
      std::move(bcErr.first), runtimeFlags, std::move(sourceURL));
}

std::shared_ptr<const jsi::PreparedJavaScript>
HermesRuntimeImpl::prepareJavaScript(
    const std::shared_ptr<const jsi::Buffer> &jsiBuffer,
    std::string sourceURL) {
  return prepareJavaScriptWithSourceMap(jsiBuffer, nullptr, sourceURL);
}

jsi::Value HermesRuntimeImpl::evaluatePreparedJavaScript(
    const std::shared_ptr<const jsi::PreparedJavaScript> &js) {
  assert(
      dynamic_cast<const HermesPreparedJavaScript *>(js.get()) &&
      "js must be an instance of HermesPreparedJavaScript");
  const auto *hermesPrep =
      static_cast<const HermesPreparedJavaScript *>(js.get());
  vm::GCScope gcScope(runtime_);
  auto res = runtime_.runBytecode(
      hermesPrep->bytecodeProvider(),
      hermesPrep->runtimeFlags(),
      hermesPrep->sourceURL(),
      vm::Runtime::makeNullHandle<vm::Environment>());
  checkStatus(res.getStatus());
  return valueFromHermesValue(*res);
}

jsi::Value HermesRuntimeImpl::evaluateJavaScript(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::string &sourceURL) {
  return evaluateJavaScriptWithSourceMap(buffer, nullptr, sourceURL);
}

bool HermesRuntimeImpl::drainMicrotasks(int maxMicrotasksHint) {
  if (runtime_.hasMicrotaskQueue()) {
    checkStatus(runtime_.drainJobs());
  }
  // \c drainJobs is currently an unbounded execution, hence no exceptions
  // implies drained until TODO(T89426441): \c maxMicrotasksHint is supported
  runtime_.clearKeptObjects();
  return true;
}

jsi::Object HermesRuntimeImpl::global() {
  return add<jsi::Object>(runtime_.getGlobal().getHermesValue());
}

std::string HermesRuntimeImpl::description() {
  std::string gcName = runtime_.getHeap().getName();
  if (gcName.empty()) {
    return "HermesRuntime";
  } else {
    return "HermesRuntime[" + gcName + "]";
  }
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

jsi::Runtime::PointerValue *HermesRuntimeImpl::cloneBigInt(
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
#ifndef NDEBUG
  for (size_t i = 0; i < length; ++i) {
    assert(
        static_cast<unsigned char>(str[i]) < 128 &&
        "non-ASCII character in property name");
  }
#endif

  vm::GCScope gcScope(runtime_);
  auto cr = vm::stringToSymbolID(
      runtime_,
      vm::StringPrimitive::createNoThrow(
          runtime_, llvh::StringRef(str, length)));
  checkStatus(cr.getStatus());
  return add<jsi::PropNameID>(cr->getHermesValue());
}

jsi::PropNameID HermesRuntimeImpl::createPropNameIDFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  vm::GCScope gcScope(runtime_);
  auto cr = vm::stringToSymbolID(
      runtime_,
      vm::createPseudoHandle(stringHVFromUtf8(utf8, length).getString()));
  checkStatus(cr.getStatus());
  return add<jsi::PropNameID>(cr->getHermesValue());
}

jsi::PropNameID HermesRuntimeImpl::createPropNameIDFromString(
    const jsi::String &str) {
  vm::GCScope gcScope(runtime_);
  auto cr = vm::stringToSymbolID(
      runtime_, vm::createPseudoHandle(phv(str).getString()));
  checkStatus(cr.getStatus());
  return add<jsi::PropNameID>(cr->getHermesValue());
}

jsi::PropNameID HermesRuntimeImpl::createPropNameIDFromSymbol(
    const jsi::Symbol &sym) {
  return add<jsi::PropNameID>(phv(sym));
}

std::string HermesRuntimeImpl::utf8(const jsi::PropNameID &sym) {
  vm::GCScope gcScope(runtime_);
  vm::SymbolID id = phv(sym).getSymbol();
  auto view = runtime_.getIdentifierTable().getStringView(runtime_, id);
  return utf8FromStringView(view);
}

bool HermesRuntimeImpl::compare(
    const jsi::PropNameID &a,
    const jsi::PropNameID &b) {
  return phv(a).getSymbol() == phv(b).getSymbol();
}

std::string HermesRuntimeImpl::utf8FromStringView(vm::StringView view) {
  if (view.isASCII())
    return std::string{view.castToCharPtr(), view.length()};

  std::string ret;
  ::hermes::convertUTF16ToUTF8WithReplacements(
      ret, llvh::ArrayRef{view.castToChar16Ptr(), view.length()});
  return ret;
}

std::string HermesRuntimeImpl::symbolToString(const jsi::Symbol &sym) {
  vm::GCScope gcScope(runtime_);
  auto res = symbolDescriptiveString(
      runtime_,
      ::hermes::vm::Handle<::hermes::vm::SymbolID>::vmcast(&phv(sym)));
  checkStatus(res.getStatus());

  return utf8FromStringView(
      vm::StringPrimitive::createStringView(runtime_, *res));
}

jsi::BigInt HermesRuntimeImpl::createBigIntFromInt64(int64_t value) {
  vm::GCScope gcScope(runtime_);
  vm::CallResult<vm::HermesValue> res =
      vm::BigIntPrimitive::fromSigned(runtime_, value);
  checkStatus(res.getStatus());
  return add<jsi::BigInt>(*res);
}

jsi::BigInt HermesRuntimeImpl::createBigIntFromUint64(uint64_t value) {
  vm::GCScope gcScope(runtime_);
  vm::CallResult<vm::HermesValue> res =
      vm::BigIntPrimitive::fromUnsigned(runtime_, value);
  checkStatus(res.getStatus());
  return add<jsi::BigInt>(*res);
}

bool HermesRuntimeImpl::bigintIsInt64(const jsi::BigInt &bigint) {
  constexpr bool signedTruncation = true;
  return phv(bigint).getBigInt()->isTruncationToSingleDigitLossless(
      signedTruncation);
}

bool HermesRuntimeImpl::bigintIsUint64(const jsi::BigInt &bigint) {
  constexpr bool signedTruncation = false;
  return phv(bigint).getBigInt()->isTruncationToSingleDigitLossless(
      signedTruncation);
}

uint64_t HermesRuntimeImpl::truncate(const jsi::BigInt &bigint) {
  auto digit = phv(bigint).getBigInt()->truncateToSingleDigit();
  static_assert(
      sizeof(digit) == sizeof(uint64_t),
      "BigInt digit is no longer sizeof(uint64_t) bytes.");
  return digit;
}

jsi::String HermesRuntimeImpl::bigintToString(
    const jsi::BigInt &bigint,
    int radix) {
  if (radix < 2 || radix > 36) {
    throwJSErrorWithMessage("Invalid radix ", radix, " to BigInt.toString");
  }

  vm::GCScope gcScope(runtime_);
  vm::CallResult<vm::HermesValue> toStringRes = vm::BigIntPrimitive::toString(
      runtime_, vm::createPseudoHandle(phv(bigint).getBigInt()), radix);

  checkStatus(toStringRes.getStatus());
  return add<jsi::String>(*toStringRes);
}

jsi::String HermesRuntimeImpl::createStringFromAscii(
    const char *str,
    size_t length) {
#ifndef NDEBUG
  for (size_t i = 0; i < length; ++i) {
    assert(
        static_cast<unsigned char>(str[i]) < 128 &&
        "non-ASCII character in string");
  }
#endif
  vm::GCScope gcScope(runtime_);
  return add<jsi::String>(stringHVFromAscii(str, length));
}

jsi::String HermesRuntimeImpl::createStringFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  vm::GCScope gcScope(runtime_);
  return add<jsi::String>(stringHVFromUtf8(utf8, length));
}

std::string HermesRuntimeImpl::utf8(const jsi::String &str) {
  return utf8FromStringView(
      vm::StringPrimitive::createStringView(runtime_, stringHandle(str)));
}

jsi::Value HermesRuntimeImpl::createValueFromJsonUtf8(
    const uint8_t *json,
    size_t length) {
  vm::GCScope gcScope(runtime_);
  llvh::ArrayRef<uint8_t> ref(json, length);
  vm::CallResult<vm::HermesValue> res =
      runtimeJSONParseRef(runtime_, ::hermes::UTF16Stream(ref));
  checkStatus(res.getStatus());
  return valueFromHermesValue(*res);
}

jsi::Object HermesRuntimeImpl::createObject() {
  vm::GCScope gcScope(runtime_);
  return add<jsi::Object>(vm::JSObject::create(runtime_).getHermesValue());
}

jsi::Object HermesRuntimeImpl::createObject(
    std::shared_ptr<jsi::HostObject> ho) {
  vm::GCScope gcScope(runtime_);

  auto objRes = vm::HostObject::createWithoutPrototype(
      runtime_, std::make_unique<JsiProxy>(*this, ho));
  checkStatus(objRes.getStatus());
  return add<jsi::Object>(*objRes);
}

std::shared_ptr<jsi::HostObject> HermesRuntimeImpl::getHostObject(
    const jsi::Object &obj) {
  const vm::HostObjectProxy *proxy =
      vm::vmcast<vm::HostObject>(phv(obj))->getProxy();
  return static_cast<const JsiProxy *>(proxy)->ho_;
}

bool HermesRuntimeImpl::hasNativeState(const jsi::Object &obj) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  if (h->isProxyObject() || h->isHostObject()) {
    return false;
  }
  vm::NamedPropertyDescriptor desc;
  return vm::JSObject::getOwnNamedDescriptor(
      h,
      runtime_,
      vm::Predefined::getSymbolID(vm::Predefined::InternalPropertyNativeState),
      desc);
}

static void deleteShared(void *context) {
  delete reinterpret_cast<std::shared_ptr<jsi::NativeState> *>(context);
}

void HermesRuntimeImpl::setNativeState(
    const jsi::Object &obj,
    std::shared_ptr<jsi::NativeState> state) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  if (h->isProxyObject()) {
    throw jsi::JSINativeException("native state unsupported on Proxy");
  } else if (h->isHostObject()) {
    throw jsi::JSINativeException("native state unsupported on HostObject");
  }
  // Allocate a shared_ptr on the C++ heap and use it as context of
  // NativeState.
  auto *ptr = new std::shared_ptr<jsi::NativeState>(std::move(state));
  auto ns =
      runtime_.makeHandle(vm::NativeState::create(runtime_, ptr, deleteShared));
  auto res = vm::JSObject::defineOwnProperty(
      h,
      runtime_,
      vm::Predefined::getSymbolID(vm::Predefined::InternalPropertyNativeState),
      vm::DefinePropertyFlags::getDefaultNewPropertyFlags(),
      ns);
  // NB: If setting the property failed, then the NativeState cell will soon
  // be unreachable, and when it's later finalized, the shared_ptr will be
  // deleted.
  checkStatus(res.getStatus());
  if (!*res) {
    throw jsi::JSINativeException(
        "failed to define internal native state property");
  }
}

std::shared_ptr<jsi::NativeState> HermesRuntimeImpl::getNativeState(
    const jsi::Object &obj) {
  vm::GCScope gcScope(runtime_);
  assert(hasNativeState(obj) && "object lacks native state");
  auto h = handle(obj);
  vm::NamedPropertyDescriptor desc;
  bool exists = vm::JSObject::getOwnNamedDescriptor(
      h,
      runtime_,
      vm::Predefined::getSymbolID(vm::Predefined::InternalPropertyNativeState),
      desc);
  (void)exists;
  assert(exists && "hasNativeState lied");
  // Raw pointers below.
  vm::NoAllocScope scope(runtime_);
  vm::NativeState *ns = vm::vmcast<vm::NativeState>(
      vm::JSObject::getNamedSlotValueUnsafe(*h, runtime_, desc)
          .getObject(runtime_));
  return std::shared_ptr(
      *reinterpret_cast<std::shared_ptr<jsi::NativeState> *>(ns->context()));
}

jsi::Value HermesRuntimeImpl::getProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  auto res = h->getComputed_RJS(h, runtime_, stringHandle(name));
  checkStatus(res.getStatus());
  return valueFromHermesValue(res->get());
}

jsi::Value HermesRuntimeImpl::getProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  vm::SymbolID nameID = phv(name).getSymbol();
  auto res = h->getNamedOrIndexed(h, runtime_, nameID);
  checkStatus(res.getStatus());
  return valueFromHermesValue(res->get());
}

bool HermesRuntimeImpl::hasProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  auto result = h->hasComputed(h, runtime_, stringHandle(name));
  checkStatus(result.getStatus());
  return result.getValue();
}

bool HermesRuntimeImpl::hasProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  vm::SymbolID nameID = phv(name).getSymbol();
  auto result = h->hasNamedOrIndexed(h, runtime_, nameID);
  checkStatus(result.getStatus());
  return result.getValue();
}

void HermesRuntimeImpl::setPropertyValue(
    const jsi::Object &obj,
    const jsi::String &name,
    const jsi::Value &value) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  checkStatus(h->putComputed_RJS(
                   h,
                   runtime_,
                   stringHandle(name),
                   vmHandleFromValue(value),
                   vm::PropOpFlags().plusThrowOnError())
                  .getStatus());
}

void HermesRuntimeImpl::setPropertyValue(
    const jsi::Object &obj,
    const jsi::PropNameID &name,
    const jsi::Value &value) {
  vm::GCScope gcScope(runtime_);
  auto h = handle(obj);
  vm::SymbolID nameID = phv(name).getSymbol();
  checkStatus(h->putNamedOrIndexed(
                   h,
                   runtime_,
                   nameID,
                   vmHandleFromValue(value),
                   vm::PropOpFlags().plusThrowOnError())
                  .getStatus());
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
  vm::GCScope gcScope(runtime_);
  uint32_t beginIndex;
  uint32_t endIndex;
  vm::CallResult<vm::Handle<vm::SegmentedArray>> cr =
      vm::getForInPropertyNames(runtime_, handle(obj), beginIndex, endIndex);
  checkStatus(cr.getStatus());
  vm::Handle<vm::SegmentedArray> arr = *cr;
  size_t length = endIndex - beginIndex;

  auto ret = createArray(length);
  for (size_t i = 0; i < length; ++i) {
    vm::HermesValue name = arr->at(runtime_, beginIndex + i);
    if (name.isString()) {
      ret.setValueAtIndex(*this, i, valueFromHermesValue(name));
    } else if (name.isNumber()) {
      std::string s;
      llvh::raw_string_ostream os(s);
      os << static_cast<size_t>(name.getNumber());
      ret.setValueAtIndex(
          *this, i, jsi::String::createFromAscii(*this, os.str()));
    } else {
      llvm_unreachable("property name is not String or Number");
    }
  }

  return ret;
}

jsi::WeakObject HermesRuntimeImpl::createWeakObject(const jsi::Object &obj) {
  return addWeak(vm::WeakRoot<vm::JSObject>(
      static_cast<vm::JSObject *>(phv(obj).getObject()), runtime_));
}

jsi::Value HermesRuntimeImpl::lockWeakObject(const jsi::WeakObject &wo) {
  const vm::WeakRoot<vm::JSObject> &wr = weakRoot(wo);

  if (const auto ptr = wr.get(runtime_, runtime_.getHeap()))
    return add<jsi::Object>(vm::HermesValue::encodeObjectValue(ptr));

  return jsi::Value();
}

jsi::Array HermesRuntimeImpl::createArray(size_t length) {
  vm::GCScope gcScope(runtime_);
  auto result = vm::JSArray::create(runtime_, length, length);
  checkStatus(result.getStatus());
  return add<jsi::Array>(result->getHermesValue());
}

jsi::ArrayBuffer HermesRuntimeImpl::createArrayBuffer(
    std::shared_ptr<jsi::MutableBuffer> buffer) {
  vm::GCScope gcScope(runtime_);
  auto buf = runtime_.makeHandle(vm::JSArrayBuffer::create(
      runtime_,
      vm::Handle<vm::JSObject>::vmcast(&runtime_.arrayBufferPrototype)));
  auto size = buffer->size();
  auto *data = buffer->data();
  auto *ctx = new std::shared_ptr<jsi::MutableBuffer>(std::move(buffer));
  auto finalize = [](void *ctx) {
    delete static_cast<std::shared_ptr<jsi::MutableBuffer> *>(ctx);
  };
  auto res = vm::JSArrayBuffer::setExternalDataBlock(
      runtime_, buf, data, size, ctx, finalize);
  checkStatus(res);
  return add<jsi::ArrayBuffer>(buf.getHermesValue());
}

size_t HermesRuntimeImpl::size(const jsi::Array &arr) {
  return vm::JSArray::getLength(*arrayHandle(arr), runtime_);
}

size_t HermesRuntimeImpl::size(const jsi::ArrayBuffer &arr) {
  auto ab = arrayBufferHandle(arr);
  if (LLVM_UNLIKELY(!ab->attached()))
    throw jsi::JSINativeException("ArrayBuffer is detached.");
  return ab->size();
}

uint8_t *HermesRuntimeImpl::data(const jsi::ArrayBuffer &arr) {
  auto ab = arrayBufferHandle(arr);
  if (LLVM_UNLIKELY(!ab->attached()))
    throw jsi::JSINativeException("ArrayBuffer is detached.");
  return ab->getDataBlock(runtime_);
}

jsi::Value HermesRuntimeImpl::getValueAtIndex(const jsi::Array &arr, size_t i) {
  vm::GCScope gcScope(runtime_);
  if (LLVM_UNLIKELY(i >= size(arr))) {
    throwJSErrorWithMessage(
        "getValueAtIndex: index ", i, " is out of bounds [0, ", size(arr), ")");
  }

  auto res = vm::JSObject::getComputed_RJS(
      arrayHandle(arr),
      runtime_,
      runtime_.makeHandle(vm::HermesValue::encodeUntrustedNumberValue(i)));
  checkStatus(res.getStatus());

  return valueFromHermesValue(res->get());
}

void HermesRuntimeImpl::setValueAtIndexImpl(
    const jsi::Array &arr,
    size_t i,
    const jsi::Value &value) {
  vm::GCScope gcScope(runtime_);
  if (LLVM_UNLIKELY(i >= size(arr))) {
    throwJSErrorWithMessage(
        "setValueAtIndex: index ", i, " is out of bounds [0, ", size(arr), ")");
  }

  auto res = vm::JSObject::putComputed_RJS(
      arrayHandle(arr),
      runtime_,
      runtime_.makeHandle(vm::HermesValue::encodeTrustedNumberValue(i)),
      vmHandleFromValue(value));
  checkStatus(res.getStatus());
}

jsi::Function HermesRuntimeImpl::createFunctionFromHostFunction(
    const jsi::PropNameID &name,
    unsigned int paramCount,
    jsi::HostFunctionType func) {
  auto context = std::make_unique<HFContext>(std::move(func), *this);
  auto hostfunc =
      createFunctionFromHostFunction(context.get(), name, paramCount);
  context.release();
  return hostfunc;
}

template <typename ContextType>
jsi::Function HermesRuntimeImpl::createFunctionFromHostFunction(
    ContextType *context,
    const jsi::PropNameID &name,
    unsigned int paramCount) {
  vm::GCScope gcScope(runtime_);
  vm::SymbolID nameID = phv(name).getSymbol();
  auto funcRes = vm::FinalizableNativeFunction::createWithoutPrototype(
      runtime_,
      context,
      &ContextType::func,
      &ContextType::finalize,
      nameID,
      paramCount);
  checkStatus(funcRes.getStatus());
  jsi::Function ret = add<jsi::Function>(*funcRes);
  return ret;
}

jsi::HostFunctionType &HermesRuntimeImpl::getHostFunction(
    const jsi::Function &func) {
  return static_cast<HFContext *>(
             vm::vmcast<vm::FinalizableNativeFunction>(phv(func))->getContext())
      ->hostFunction;
}

jsi::Value HermesRuntimeImpl::call(
    const jsi::Function &func,
    const jsi::Value &jsThis,
    const jsi::Value *args,
    size_t count) {
  vm::GCScope gcScope(runtime_);
  vm::Handle<vm::Callable> handle =
      vm::Handle<vm::Callable>::vmcast(&phv(func));
  if (count > std::numeric_limits<uint32_t>::max()) {
    LOG_EXCEPTION_CAUSE(
        "HermesRuntimeImpl::call: Unable to call function: stack overflow");
    throw jsi::JSINativeException(
        "HermesRuntimeImpl::call: Unable to call function: stack overflow");
  }

  vm::ScopedNativeCallFrame newFrame{
      runtime_,
      static_cast<uint32_t>(count),
      handle.getHermesValue(),
      vm::HermesValue::encodeUndefinedValue(),
      hvFromValue(jsThis)};
  if (LLVM_UNLIKELY(newFrame.overflowed())) {
    checkStatus(runtime_.raiseStackOverflow(
        ::hermes::vm::Runtime::StackOverflowKind::NativeStack));
  }

  for (uint32_t i = 0; i != count; ++i) {
    newFrame->getArgRef(i) = hvFromValue(args[i]);
  }
  auto callRes = vm::Callable::call(handle, runtime_);
  checkStatus(callRes.getStatus());

  return valueFromHermesValue(callRes->get());
}

jsi::Value HermesRuntimeImpl::callAsConstructor(
    const jsi::Function &func,
    const jsi::Value *args,
    size_t count) {
  vm::GCScope gcScope(runtime_);
  vm::Handle<vm::Callable> funcHandle =
      vm::Handle<vm::Callable>::vmcast(&phv(func));

  if (count > std::numeric_limits<uint32_t>::max()) {
    LOG_EXCEPTION_CAUSE(
        "HermesRuntimeImpl::call: Unable to call function: stack overflow");
    throw jsi::JSINativeException(
        "HermesRuntimeImpl::call: Unable to call function: stack overflow");
  }

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
  auto thisRes = vm::Callable::createThisForConstruct_RJS(funcHandle, runtime_);
  // We need to capture this in case the ctor doesn't return an object,
  // we need to return this object.
  auto objHandle = runtime_.makeHandle<vm::JSObject>(std::move(*thisRes));

  // 13.2.2.8:
  //    Let result be the result of calling the [[Call]] internal property of
  //    F, providing obj as the this value and providing the argument list
  //    passed into [[Construct]] as args.
  //
  // For us result == res.

  vm::ScopedNativeCallFrame newFrame{
      runtime_,
      static_cast<uint32_t>(count),
      funcHandle.getHermesValue(),
      funcHandle.getHermesValue(),
      objHandle.getHermesValue()};
  if (newFrame.overflowed()) {
    checkStatus(runtime_.raiseStackOverflow(
        ::hermes::vm::Runtime::StackOverflowKind::NativeStack));
  }
  for (uint32_t i = 0; i != count; ++i) {
    newFrame->getArgRef(i) = hvFromValue(args[i]);
  }
  // The last parameter indicates that this call should construct an object.
  auto callRes = vm::Callable::call(funcHandle, runtime_);
  checkStatus(callRes.getStatus());

  // 13.2.2.9:
  //    If Type(result) is Object then return result
  // 13.2.2.10:
  //    Return obj
  auto resultValue = callRes->get();
  vm::HermesValue resultHValue =
      resultValue.isObject() ? resultValue : objHandle.getHermesValue();
  return valueFromHermesValue(resultHValue);
}

bool HermesRuntimeImpl::strictEquals(const jsi::Symbol &a, const jsi::Symbol &b)
    const {
  return phv(a).getSymbol() == phv(b).getSymbol();
}

bool HermesRuntimeImpl::strictEquals(const jsi::BigInt &a, const jsi::BigInt &b)
    const {
  return phv(a).getBigInt()->compare(phv(b).getBigInt()) == 0;
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
  vm::GCScope gcScope(runtime_);
  auto result = vm::instanceOfOperator_RJS(runtime_, handle(o), handle(f));
  checkStatus(result.getStatus());
  return *result;
}

jsi::Runtime::ScopeState *HermesRuntimeImpl::pushScope() {
  return nullptr;
}

void HermesRuntimeImpl::popScope(ScopeState *prv) {
  assert(!prv && "pushScope only returns nullptrs");
}

void HermesRuntimeImpl::checkStatus(vm::ExecutionStatus status) {
  if (LLVM_LIKELY(status != vm::ExecutionStatus::EXCEPTION)) {
    return;
  }

  throwPendingError();
}

vm::HermesValue HermesRuntimeImpl::stringHVFromAscii(
    const char *str,
    size_t length) {
  auto strRes = vm::StringPrimitive::createEfficient(
      runtime_, llvh::makeArrayRef(str, length));
  checkStatus(strRes.getStatus());
  return *strRes;
}

vm::HermesValue HermesRuntimeImpl::stringHVFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  const bool IgnoreInputErrors = true;
  auto strRes = vm::StringPrimitive::createEfficient(
      runtime_, llvh::makeArrayRef(utf8, length), IgnoreInputErrors);
  checkStatus(strRes.getStatus());
  return *strRes;
}

void HermesRuntimeImpl::throwPendingError() {
  vm::GCScope scope{runtime_};

  // Retrieve the exception value and clear as we will rethrow it as a C++
  // exception.
  auto hv = runtime_.getThrownValue();
  runtime_.clearThrownValue();
  auto jsiVal = valueFromHermesValue(hv);
  auto hnd = vmHandleFromValue(jsiVal);

  std::string msg = "No message";
  std::string stack = "No stack";
  if (auto str = vm::Handle<vm::StringPrimitive>::dyn_vmcast(hnd)) {
    // If the exception is a string, use it as the message.
    msg = utf8FromStringView(
        vm::StringPrimitive::createStringView(runtime_, str));
  } else if (auto obj = vm::Handle<vm::JSObject>::dyn_vmcast(hnd)) {
    // If the exception is an object try to retrieve its message and stack
    // properties.

    /// Attempt to retrieve a string property \p sym from \c obj and store it
    /// in \p out. Ignore any catchable errors and non-string properties.
    auto getStrProp = [this, obj](vm::SymbolID sym, std::string &out) {
      auto propRes = vm::JSObject::getNamed_RJS(obj, runtime_, sym);
      if (LLVM_UNLIKELY(propRes == vm::ExecutionStatus::EXCEPTION)) {
        // An exception was thrown while retrieving the property, if it is
        // catchable, suppress it. Otherwise, rethrow this exception without
        // trying to invoke any more JavaScript.
        auto propExHv = runtime_.getThrownValue();
        runtime_.clearThrownValue();

        if (!vm::isUncatchableError(propExHv))
          return;

        // An uncatchable error occurred, it is unsafe to do anything that might
        // execute more JavaScript.
        throw jsi::JSError(
            valueFromHermesValue(propExHv),
            "Uncatchable exception thrown while creating error",
            "No stack");
      }

      // If the property is a string, update out. Otherwise ignore it.
      auto prop = propRes->get();
      if (prop.isString()) {
        auto view = vm::StringPrimitive::createStringView(
            runtime_, runtime_.makeHandle(prop.getString()));
        out = utf8FromStringView(view);
      }
    };

    getStrProp(vm::Predefined::getSymbolID(vm::Predefined::message), msg);
    getStrProp(vm::Predefined::getSymbolID(vm::Predefined::stack), stack);
  }

  // Use the constructor of jsi::JSError that cannot run additional
  // JS, since that may then result in additional exceptions and infinite
  // recursion.
  throw jsi::JSError(std::move(jsiVal), msg, stack);
}

template <typename... Args>
void HermesRuntimeImpl::throwJSErrorWithMessage(Args &&...args) {
  // TODO: Add support for size_t in TwineChar16 and directly construct that
  //       instead of using a stream.
  std::string s;
  llvh::raw_string_ostream os(s);
  raw_ostream_append(os, std::forward<Args>(args)...);
  LOG_EXCEPTION_CAUSE("JSError: %s", os.str().c_str());
  // Raise an error with this message in the Runtime and rethrow it with
  // throwPendingError.
  (void)runtime_.raiseError(vm::TwineChar16(s));
  throwPendingError();
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

vm::RuntimeConfig hardenedHermesRuntimeConfig() {
  vm::RuntimeConfig::Builder config;
  // Disable optional JS features.
  config.withEnableEval(false);
  config.withArrayBuffer(false);
  config.withES6Proxy(false);
  config.withEnableHermesInternal(false);
  config.withEnableHermesInternalTestMethods(false);

  // Enabled hardening options.
  config.withRandomizeMemoryLayout(true);

  // This flag is misnamed - it doesn't only apply to eval() calls but to
  // all compilation performed by the HermesRuntime, so it should be enabled
  // even when eval() is disabled, to ensure that watchTimeLimit works.
  config.withAsyncBreakCheckInEval(true);
  return config.build();
}

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
#else
  ret->setDebugger(std::make_unique<debugger::Debugger>());
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

  auto &hermesRt = ret->getUnsafeRuntime();
#ifdef HERMES_ENABLE_DEBUGGER
  // Only HermesRuntime can create a debugger instance.  This requires
  // the setter and not using make_unique, so the call to new is here
  // in this function, which is a friend of debugger::Debugger.
  hermesRt.setDebugger(std::unique_ptr<debugger::Debugger>(
      new debugger::Debugger(&hermesRt, &(hermesRt.runtime_.getDebugger()))));
#else
  hermesRt.setDebugger(std::make_unique<debugger::Debugger>());
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
