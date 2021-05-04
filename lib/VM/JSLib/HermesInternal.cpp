/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/Support/Base64vlq.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSLib.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringView.h"

#include <cstring>
#include <random>

namespace hermes {
namespace vm {

/// \return a SymbolID  for a given C string \p s.
static inline CallResult<Handle<SymbolID>> symbolForCStr(
    Runtime *rt,
    const char *s) {
  return rt->getIdentifierTable().getSymbolHandle(rt, ASCIIRef{s, strlen(s)});
}

// ES7 24.1.1.3
CallResult<HermesValue>
hermesInternalDetachArrayBuffer(void *, Runtime *runtime, NativeArgs args) {
  auto buffer = args.dyncastArg<JSArrayBuffer>(0);
  if (!buffer) {
    return runtime->raiseTypeError(
        "Cannot use detachArrayBuffer on something which "
        "is not an ArrayBuffer foo");
  }
  buffer->detach(&runtime->getHeap());
  // "void" return
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
hermesInternalGetEpilogues(void *, Runtime *runtime, NativeArgs args) {
  // Create outer array with one element per module.
  auto eps = runtime->getEpilogues();
  auto outerLen = eps.size();
  auto outerResult = JSArray::create(runtime, outerLen, outerLen);

  if (outerResult == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto outer = *outerResult;
  if (outer->setStorageEndIndex(outer, runtime, outerLen) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // Set each element to a Uint8Array holding the epilogue for that module.
  for (unsigned i = 0; i < outerLen; ++i) {
    auto innerLen = eps[i].size();
    if (innerLen != 0) {
      auto result = JSTypedArray<uint8_t, CellKind::Uint8ArrayKind>::allocate(
          runtime, innerLen);
      if (result == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      auto ta = result.getValue();
      std::memcpy(ta->begin(runtime), eps[i].begin(), innerLen);
      JSArray::unsafeSetExistingElementAt(
          *outer, runtime, i, ta.getHermesValue());
    }
  }
  return HermesValue::encodeObjectValue(*outer);
}

/// Used for testing, determines how many live values
/// are in the given WeakMap or WeakSet.
CallResult<HermesValue>
hermesInternalGetWeakSize(void *, Runtime *runtime, NativeArgs args) {
  if (auto M = args.dyncastArg<JSWeakMap>(0)) {
    return HermesValue::encodeNumberValue(JSWeakMap::debugFreeSlotsAndGetSize(
        static_cast<PointerBase *>(runtime), &runtime->getHeap(), *M));
  }

  if (auto S = args.dyncastArg<JSWeakSet>(0)) {
    return HermesValue::encodeNumberValue(JSWeakSet::debugFreeSlotsAndGetSize(
        static_cast<PointerBase *>(runtime), &runtime->getHeap(), *S));
  }

  return runtime->raiseTypeError(
      "getWeakSize can only be called on a WeakMap/WeakSet");
}

/// \return an object containing various instrumented statistics.
CallResult<HermesValue>
hermesInternalGetInstrumentedStats(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto resultHandle = runtime->makeHandle(JSObject::create(runtime));
  // Printing the values would be unstable, so prevent that.
  if (runtime->shouldStabilizeInstructionCount())
    return resultHandle.getHermesValue();
  MutableHandle<> tmpHandle{runtime};

  namespace P = Predefined;
/// Adds a property to \c resultHandle. \p KEY provides its name as a \c
/// Predefined enum value, and its value is rooted in \p VALUE.  If property
/// definition fails, the exceptional execution status will be propogated to the
/// outer function.
#define SET_PROP(KEY, VALUE)                                                \
  do {                                                                      \
    GCScopeMarkerRAII marker{gcScope};                                      \
    double val = VALUE;                                                     \
    if (statsTable) {                                                       \
      auto array = runtime->getPredefinedString(KEY)->getStringRef<char>(); \
      std::string key(array.data(), array.size());                          \
      if (statsTable->count(key.c_str())) {                                 \
        val = (*statsTable)[StringRef(key.c_str(), key.size())].num();      \
      }                                                                     \
    }                                                                       \
    tmpHandle = HermesValue::encodeDoubleValue(val);                        \
    auto status = JSObject::defineNewOwnProperty(                           \
        resultHandle,                                                       \
        runtime,                                                            \
        Predefined::getSymbolID(KEY),                                       \
        PropertyFlags::defaultNewNamedPropertyFlags(),                      \
        tmpHandle);                                                         \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {              \
      return ExecutionStatus::EXCEPTION;                                    \
    }                                                                       \
    if (newStatsTable) {                                                    \
      auto array = runtime->getPredefinedString(KEY)->getStringRef<char>(); \
      std::string key(array.data(), array.size());                          \
      newStatsTable->try_emplace(key, val);                                 \
    }                                                                       \
  } while (false)

  auto &stats = runtime->getRuntimeStats();
  MockedEnvironment::StatsTable *statsTable = nullptr;
  auto *const storage = runtime->getCommonStorage();
  if (storage->env) {
    // For now, we'll allow replay to exhaust the recorded getInstrumentedStats
    // calls, and allow further calls to take values from the replay execution.
    // This allows backwards compatibility with existing traces that do not
    // record getInstrumentedStats.  In the future, we might wish to disallow
    // this, and throw an exception.
    if (!storage->env->callsToHermesInternalGetInstrumentedStats.empty()) {
      statsTable =
          &storage->env->callsToHermesInternalGetInstrumentedStats.front();
    }
  }

  std::unique_ptr<MockedEnvironment::StatsTable> newStatsTable;
  if (storage->shouldTrace) {
    newStatsTable.reset(new MockedEnvironment::StatsTable());
  }

  // Ensure that the timers measuring the current execution are up to date.
  stats.flushPendingTimers();

  SET_PROP(P::js_hostFunctionTime, stats.hostFunction.wallDuration);
  SET_PROP(P::js_hostFunctionCPUTime, stats.hostFunction.cpuDuration);
  SET_PROP(P::js_hostFunctionCount, stats.hostFunction.count);

  SET_PROP(P::js_evaluateJSTime, stats.evaluateJS.wallDuration);
  SET_PROP(P::js_evaluateJSCPUTime, stats.evaluateJS.cpuDuration);
  SET_PROP(P::js_evaluateJSCount, stats.evaluateJS.count);

  SET_PROP(P::js_incomingFunctionTime, stats.incomingFunction.wallDuration);
  SET_PROP(P::js_incomingFunctionCPUTime, stats.incomingFunction.cpuDuration);
  SET_PROP(P::js_incomingFunctionCount, stats.incomingFunction.count);
  SET_PROP(P::js_VMExperiments, runtime->getVMExperimentFlags());

  auto makeHermesTime = [](double host, double eval, double incoming) {
    return eval - host + incoming;
  };

  SET_PROP(
      P::js_hermesTime,
      makeHermesTime(
          stats.hostFunction.wallDuration,
          stats.evaluateJS.wallDuration,
          stats.incomingFunction.wallDuration));
  SET_PROP(
      P::js_hermesCPUTime,
      makeHermesTime(
          stats.hostFunction.cpuDuration,
          stats.evaluateJS.cpuDuration,
          stats.incomingFunction.cpuDuration));

  if (stats.shouldSample) {
    SET_PROP(
        P::js_hermesThreadMinorFaults,
        makeHermesTime(
            stats.hostFunction.sampled.threadMinorFaults,
            stats.evaluateJS.sampled.threadMinorFaults,
            stats.incomingFunction.sampled.threadMinorFaults));
    SET_PROP(
        P::js_hermesThreadMajorFaults,
        makeHermesTime(
            stats.hostFunction.sampled.threadMajorFaults,
            stats.evaluateJS.sampled.threadMajorFaults,
            stats.incomingFunction.sampled.threadMajorFaults));
  }

  auto &heap = runtime->getHeap();
  SET_PROP(P::js_numGCs, heap.getNumGCs());
  SET_PROP(P::js_gcCPUTime, heap.getGCCPUTime());
  SET_PROP(P::js_gcTime, heap.getGCTime());

#undef SET_PROP

/// Adds a property to \c resultHandle. \p KEY provides its name as a C string,
/// and its value is rooted in \p VALUE.  If property definition fails, the
/// exceptional execution status will be propogated to the outer function.
#define SET_PROP_NEW(KEY, VALUE)                               \
  do {                                                         \
    GCScopeMarkerRAII marker{gcScope};                         \
    auto keySym = symbolForCStr(runtime, KEY);                 \
    if (LLVM_UNLIKELY(keySym == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
    double val = VALUE;                                        \
    if (statsTable && statsTable->count(KEY)) {                \
      val = (*statsTable)[KEY].num();                          \
    }                                                          \
    tmpHandle = HermesValue::encodeDoubleValue(val);           \
    auto status = JSObject::defineNewOwnProperty(              \
        resultHandle,                                          \
        runtime,                                               \
        **keySym,                                              \
        PropertyFlags::defaultNewNamedPropertyFlags(),         \
        tmpHandle);                                            \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
    if (newStatsTable) {                                       \
      newStatsTable->try_emplace(KEY, val);                    \
    }                                                          \
  } while (false)

  {
    GCBase::HeapInfo info;
    heap.getHeapInfo(info);
    SET_PROP_NEW("js_totalAllocatedBytes", info.totalAllocatedBytes);
    SET_PROP_NEW("js_allocatedBytes", info.allocatedBytes);
    SET_PROP_NEW("js_heapSize", info.heapSize);
    SET_PROP_NEW("js_mallocSizeEstimate", info.mallocSizeEstimate);
    SET_PROP_NEW("js_vaSize", info.va);
    SET_PROP_NEW("js_markStackOverflows", info.numMarkStackOverflows);
  }

  if (stats.shouldSample) {
    SET_PROP_NEW(
        "js_hermesVolCtxSwitches",
        makeHermesTime(
            stats.hostFunction.sampled.volCtxSwitches,
            stats.evaluateJS.sampled.volCtxSwitches,
            stats.incomingFunction.sampled.volCtxSwitches));
    SET_PROP_NEW(
        "js_hermesInvolCtxSwitches",
        makeHermesTime(
            stats.hostFunction.sampled.involCtxSwitches,
            stats.evaluateJS.sampled.involCtxSwitches,
            stats.incomingFunction.sampled.involCtxSwitches));
    // Sampled because it doesn't vary much, not because it's expensive to get.
    SET_PROP_NEW("js_pageSize", oscompat::page_size());
  }

/// Adds a property to \c resultHandle. \p KEY and \p VALUE provide its name and
/// value as a C string and ASCIIRef respectively. If property definition fails,
/// the exceptional execution status will be propogated to the outer function.
#define SET_PROP_STR(KEY, VALUE)                               \
  do {                                                         \
    auto keySym = symbolForCStr(runtime, KEY);                 \
    if (LLVM_UNLIKELY(keySym == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
    ASCIIRef val = VALUE;                                      \
    std::string valStdStr(val.data(), val.size());             \
    if (statsTable && statsTable->count(KEY)) {                \
      valStdStr = (*statsTable)[std::string(KEY)].str();       \
    }                                                          \
    val = ASCIIRef(valStdStr.c_str(), valStdStr.size());       \
    auto valStr = StringPrimitive::create(runtime, val);       \
    if (LLVM_UNLIKELY(valStr == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
    tmpHandle = *valStr;                                       \
    auto status = JSObject::defineNewOwnProperty(              \
        resultHandle,                                          \
        runtime,                                               \
        **keySym,                                              \
        PropertyFlags::defaultNewNamedPropertyFlags(),         \
        tmpHandle);                                            \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
    if (newStatsTable) {                                       \
      newStatsTable->try_emplace(KEY, valStdStr);              \
    }                                                          \
  } while (false)

  if (runtime->getRuntimeStats().shouldSample) {
    // Build a string showing the set of cores on which we may run.
    std::string mask;
    for (auto b : oscompat::sched_getaffinity())
      mask += b ? '1' : '0';
    SET_PROP_STR("js_threadAffinityMask", ASCIIRef(mask.data(), mask.size()));
    SET_PROP_NEW("js_threadCPU", oscompat::sched_getcpu());

    size_t bytecodePagesResident = 0;
    size_t bytecodePagesResidentRuns = 0;
    for (auto &module : runtime->getRuntimeModules()) {
      auto buf = module.getBytecode()->getRawBuffer();
      if (buf.size()) {
        llvh::SmallVector<int, 64> runs;
        int pages = oscompat::pages_in_ram(buf.data(), buf.size(), &runs);
        if (pages >= 0) {
          bytecodePagesResident += pages;
          bytecodePagesResidentRuns += runs.size();
        }
      }
    }
    SET_PROP_NEW("js_bytecodePagesResident", bytecodePagesResident);
    SET_PROP_NEW("js_bytecodePagesResidentRuns", bytecodePagesResidentRuns);

    // Stats for the module with most accesses.
    uint32_t bytecodePagesAccessed = 0;
    uint32_t bytecodeSize = 0;
    JenkinsHash bytecodePagesTraceHash = 0;
    double bytecodeIOus = 0;
    // Sample a small number of (position in access order, page id) pairs
    // encoded as a base64vlq stream.
    static constexpr unsigned NUM_SAMPLES = 32;
    std::string sample;
    for (auto &module : runtime->getRuntimeModules()) {
      auto tracker = module.getBytecode()->getPageAccessTracker();
      if (tracker) {
        auto ids = tracker->getPagesAccessed();
        if (ids.size() <= bytecodePagesAccessed)
          continue;
        bytecodePagesAccessed = ids.size();
        bytecodeSize = module.getBytecode()->getRawBuffer().size();
        bytecodePagesTraceHash = 0;
        for (auto id : ids) {
          // char16_t is at least 16 bits unsigned, so the quality of this hash
          // might degrade if accessing bytecode above 2^16 * 4 kB = 256 MB.
          bytecodePagesTraceHash = updateJenkinsHash(
              bytecodePagesTraceHash, static_cast<char16_t>(id));
        }
        bytecodeIOus = 0;
        for (auto us : tracker->getMicros()) {
          bytecodeIOus += us;
        }
        sample.clear();
        llvh::raw_string_ostream str(sample);
        std::random_device rng;
        for (unsigned sampleIdx = 0; sampleIdx < NUM_SAMPLES; ++sampleIdx) {
          int32_t accessOrderPos = rng() % ids.size();
          base64vlq::encode(str, accessOrderPos);
          base64vlq::encode(str, ids[accessOrderPos]);
        }
      }
    }
    // If we have are replaying a trace, override the value.
    if (statsTable) {
      bytecodePagesAccessed = (*statsTable)["js_bytecodePagesAccessed"].num();
    }

    if (bytecodePagesAccessed) {
      SET_PROP_NEW("js_bytecodePagesAccessed", bytecodePagesAccessed);
      SET_PROP_NEW("js_bytecodeSize", bytecodeSize);
      SET_PROP_NEW("js_bytecodePagesTraceHash", bytecodePagesTraceHash);
      SET_PROP_NEW("js_bytecodeIOTime", bytecodeIOus / 1e6);
      SET_PROP_STR(
          "js_bytecodePagesTraceSample",
          ASCIIRef(sample.data(), sample.size()));
    }
  }

  if (storage->env && statsTable) {
    storage->env->callsToHermesInternalGetInstrumentedStats.pop_front();
  }
  if (LLVM_UNLIKELY(storage->shouldTrace)) {
    storage->tracedEnv.callsToHermesInternalGetInstrumentedStats.push_back(
        *newStatsTable);
  }

  return resultHandle.getHermesValue();

#undef SET_PROP_NEW
}

/// \return a static string summarising the presence and resolution type of
/// CommonJS modules across all RuntimeModules that have been loaded into \c
/// runtime.
static const char *getCJSModuleModeDescription(Runtime *runtime) {
  bool hasCJSModulesDynamic = false;
  bool hasCJSModulesStatic = false;
  for (const auto &runtimeModule : runtime->getRuntimeModules()) {
    if (runtimeModule.hasCJSModules()) {
      hasCJSModulesDynamic = true;
    }
    if (runtimeModule.hasCJSModulesStatic()) {
      hasCJSModulesStatic = true;
    }
  }
  if (hasCJSModulesDynamic && hasCJSModulesStatic) {
    return "Mixed dynamic/static";
  }
  if (hasCJSModulesDynamic) {
    return "Dynamically resolved";
  }
  if (hasCJSModulesStatic) {
    return "Statically resolved";
  }
  return "None";
}

/// \return an object mapping keys to runtime property values.
CallResult<HermesValue>
hermesInternalGetRuntimeProperties(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto resultHandle = runtime->makeHandle(JSObject::create(runtime));
  MutableHandle<> tmpHandle{runtime};

  /// Add a property \p value keyed under \p key to resultHandle.
  /// Return an ExecutionStatus.
  auto addProperty = [&](Handle<> value, const char *key) {
    auto keySym = symbolForCStr(runtime, key);
    if (LLVM_UNLIKELY(keySym == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return JSObject::defineNewOwnProperty(
        resultHandle,
        runtime,
        **keySym,
        PropertyFlags::defaultNewNamedPropertyFlags(),
        value);
  };

#ifdef HERMES_FACEBOOK_BUILD
  tmpHandle =
      HermesValue::encodeBoolValue(std::strstr(__FILE__, "hermes-snapshot"));
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "Snapshot VM") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
#endif

  tmpHandle = HermesValue::encodeDoubleValue(::hermes::hbc::BYTECODE_VERSION);
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "Bytecode Version") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  tmpHandle = HermesValue::encodeBoolValue(runtime->builtinsAreFrozen());
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "Builtins Frozen") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  tmpHandle = HermesValue::encodeNumberValue(runtime->getVMExperimentFlags());
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "VM Experiments") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const char buildMode[] =
#ifdef HERMES_SLOW_DEBUG
      "SlowDebug"
#elif !defined(NDEBUG)
      "Debug"
#else
      "Release"
#endif
      ;
  auto buildModeRes = StringPrimitive::create(
      runtime, ASCIIRef(buildMode, sizeof(buildMode) - 1));
  if (LLVM_UNLIKELY(buildModeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = *buildModeRes;
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "Build") == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  std::string gcKind = runtime->getHeap().getKindAsStr();
  auto gcKindRes = StringPrimitive::create(
      runtime, ASCIIRef(gcKind.c_str(), gcKind.length()));
  if (LLVM_UNLIKELY(gcKindRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = *gcKindRes;
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "GC") == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

#ifdef HERMES_RELEASE_VERSION
  auto relVerRes =
      StringPrimitive::create(runtime, createASCIIRef(HERMES_RELEASE_VERSION));
  if (LLVM_UNLIKELY(relVerRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = *relVerRes;
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "OSS Release Version") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
#endif

  const char *cjsModuleMode = getCJSModuleModeDescription(runtime);
  auto cjsModuleModeRes =
      StringPrimitive::create(runtime, createASCIIRef(cjsModuleMode));
  if (LLVM_UNLIKELY(cjsModuleModeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = *cjsModuleModeRes;
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "CommonJS Modules") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return resultHandle.getHermesValue();
}

#ifdef HERMESVM_PLATFORM_LOGGING
static void logGCStats(Runtime *runtime, const char *msg) {
  // The GC stats can exceed the android logcat length limit, of
  // 1024 bytes.  Break it up.
  std::string stats;
  {
    llvh::raw_string_ostream os(stats);
    runtime->printHeapStats(os);
  }
  auto copyRegionFrom = [&stats](size_t from) -> size_t {
    size_t rBrace = stats.find("},", from);
    if (rBrace == std::string::npos) {
      std::string portion = stats.substr(from);
      hermesLog("HermesVM", "%s", portion.c_str());
      return stats.size();
    }

    // Add 2 for the length of the search string, to get to the end.
    const size_t to = rBrace + 2;
    std::string portion = stats.substr(from, to - from);
    hermesLog("HermesVM", "%s", portion.c_str());
    return to;
  };

  hermesLog("HermesVM", "%s:", msg);
  for (size_t ind = 0; ind < stats.size(); ind = copyRegionFrom(ind))
    ;
}
#endif

CallResult<HermesValue>
hermesInternalTTIReached(void *, Runtime *runtime, NativeArgs args) {
  runtime->ttiReached();
#ifdef HERMESVM_LLVM_PROFILE_DUMP
  __llvm_profile_dump();
  throw jsi::JSINativeException("TTI reached; profiling done");
#endif
#ifdef HERMESVM_PLATFORM_LOGGING
  logGCStats(runtime, "TTI call");
#endif
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
hermesInternalTTRCReached(void *, Runtime *runtime, NativeArgs args) {
  // Currently does nothing, but could change in the future.
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
hermesInternalIsProxy(void *, Runtime *runtime, NativeArgs args) {
  Handle<JSObject> obj = args.dyncastArg<JSObject>(0);
  return HermesValue::encodeBoolValue(obj && obj->isProxyObject());
}

CallResult<HermesValue>
hermesInternalHasPromise(void *, Runtime *runtime, NativeArgs args) {
  return HermesValue::encodeBoolValue(runtime->hasES6Promise());
}

CallResult<HermesValue>
hermesInternalUseEngineQueue(void *, Runtime *runtime, NativeArgs args) {
  return HermesValue::encodeBoolValue(runtime->useJobQueue());
}

/// \code
///   HermesInternal.enqueueJob = function (func) {}
/// \endcode
CallResult<HermesValue>
hermesInternalEnqueueJob(void *, Runtime *runtime, NativeArgs args) {
  auto callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return runtime->raiseTypeError(
        "Argument to HermesInternal.enqueueJob must be callable");
  }
  runtime->enqueueJob(callable.get());
  return HermesValue::encodeUndefinedValue();
}

/// \code
///   HermesInternal.drainJobs = function () {}
/// \endcode
/// Throw if the drainJobs throws.
CallResult<HermesValue>
hermesInternalDrainJobs(void *, Runtime *runtime, NativeArgs args) {
  auto drainRes = runtime->drainJobs();
  if (drainRes == ExecutionStatus::EXCEPTION) {
    // No need to rethrow since it's already throw.
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeUndefinedValue();
}

#ifdef HERMESVM_EXCEPTION_ON_OOM
/// Gets the current call stack as a JS String value.  Intended (only)
/// to allow testing of Runtime::callStack() from JS code.
CallResult<HermesValue>
hermesInternalGetCallStack(void *, Runtime *runtime, NativeArgs args) {
  std::string stack = runtime->getCallStackNoAlloc();
  return StringPrimitive::create(runtime, ASCIIRef(stack.data(), stack.size()));
}
#endif // HERMESVM_EXCEPTION_ON_OOM

/// \return the code block associated with \p callableHandle if it is a
/// (possibly bound) JS function, or nullptr otherwise.
static const CodeBlock *getLeafCodeBlock(
    Handle<Callable> callableHandle,
    Runtime *runtime) {
  const Callable *callable = callableHandle.get();
  while (auto *bound = dyn_vmcast<BoundFunction>(callable)) {
    callable = bound->getTarget(runtime);
  }
  if (auto *asFunction = dyn_vmcast<const JSFunction>(callable)) {
    return asFunction->getCodeBlock();
  }
  return nullptr;
}

/// \return the file name associated with \p codeBlock, if any.
/// This mirrors the way we print file names for code blocks in JSError.
static CallResult<HermesValue> getCodeBlockFileName(
    Runtime *runtime,
    const CodeBlock *codeBlock,
    OptValue<hbc::DebugSourceLocation> location) {
  RuntimeModule *runtimeModule = codeBlock->getRuntimeModule();
  if (location) {
    auto debugInfo = runtimeModule->getBytecode()->getDebugInfo();
    return StringPrimitive::createEfficient(
        runtime, debugInfo->getFilenameByID(location->filenameId));
  } else {
    llvh::StringRef sourceURL = runtimeModule->getSourceURL();
    if (!sourceURL.empty()) {
      return StringPrimitive::createEfficient(runtime, sourceURL);
    }
  }
  return HermesValue::encodeUndefinedValue();
}

/// \code
///   HermesInternal.getFunctionLocation function (func) {}
/// \endcode
/// Returns an object describing the source location of func.
/// The following properties may be present:
/// * fileName (string)
/// * lineNumber (number) - 1 based
/// * columnNumber (number) - 1 based
/// * segmentID (number) - 0 based
/// * virtualOffset (number) - 0 based
/// * isNative (boolean)
/// TypeError if func is not a function.
CallResult<HermesValue>
hermesInternalGetFunctionLocation(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);

  auto callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return runtime->raiseTypeError(
        "Argument to HermesInternal.getFunctionLocation must be callable");
  }
  auto resultHandle = runtime->makeHandle(JSObject::create(runtime));
  MutableHandle<> tmpHandle{runtime};

  auto codeBlock = getLeafCodeBlock(callable, runtime);
  bool isNative = !codeBlock;
  auto res = JSObject::defineOwnProperty(
      resultHandle,
      runtime,
      Predefined::getSymbolID(Predefined::isNative),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      runtime->getBoolValue(isNative));
  assert(res != ExecutionStatus::EXCEPTION && "Failed to set isNative");
  (void)res;

  if (codeBlock) {
    OptValue<hbc::DebugSourceLocation> location =
        codeBlock->getSourceLocation();
    if (location) {
      tmpHandle = HermesValue::encodeNumberValue(location->line);
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::lineNumber),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(res != ExecutionStatus::EXCEPTION && "Failed to set lineNumber");
      (void)res;

      tmpHandle = HermesValue::encodeNumberValue(location->column);
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::columnNumber),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(res != ExecutionStatus::EXCEPTION && "Failed to set columnNumber");
      (void)res;
    } else {
      tmpHandle = HermesValue::encodeNumberValue(
          codeBlock->getRuntimeModule()->getBytecode()->getSegmentID());
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::segmentID),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(res != ExecutionStatus::EXCEPTION && "Failed to set segmentID");
      (void)res;

      tmpHandle = HermesValue::encodeNumberValue(codeBlock->getVirtualOffset());
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::virtualOffset),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(
          res != ExecutionStatus::EXCEPTION && "Failed to set virtualOffset");
      (void)res;
    }

    auto fileNameRes = getCodeBlockFileName(runtime, codeBlock, location);
    if (LLVM_UNLIKELY(fileNameRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    tmpHandle = *fileNameRes;
    res = JSObject::defineOwnProperty(
        resultHandle,
        runtime,
        Predefined::getSymbolID(Predefined::fileName),
        DefinePropertyFlags::getDefaultNewPropertyFlags(),
        tmpHandle);
    assert(res != ExecutionStatus::EXCEPTION && "Failed to set fileName");
    (void)res;
  }
  JSObject::preventExtensions(*resultHandle);
  return resultHandle.getHermesValue();
}

/// \code
///   HermesInternal.setPromiseRejectionTrackingHook = function (func) {}
/// \endcode
/// Register the function which can be used to *enable* Promise rejection
/// tracking when the user calls it.
/// For example, when using the npm `promise` polyfill:
/// \code
///   HermesInternal.setPromiseRejectionTrackingHook(
///     require('./rejection-tracking.js').enable
///   );
/// \endcode
CallResult<HermesValue> hermesInternalSetPromiseRejectionTrackingHook(
    void *,
    Runtime *runtime,
    NativeArgs args) {
  runtime->promiseRejectionTrackingHook_ = args.getArg(0);
  return HermesValue::encodeUndefinedValue();
}

/// \code
///   HermesInternal.enablePromiseRejectionTracker = function (opts) {}
/// \endcode
/// Enable promise rejection tracking with the given opts.
CallResult<HermesValue> hermesInternalEnablePromiseRejectionTracker(
    void *,
    Runtime *runtime,
    NativeArgs args) {
  auto opts = args.getArgHandle(0);
  auto func = Handle<Callable>::dyn_vmcast(
      Handle<>(&runtime->promiseRejectionTrackingHook_));
  if (!func) {
    return runtime->raiseTypeError(
        "Promise rejection tracking hook was not registered");
  }
  return Callable::executeCall1(
             func, runtime, Runtime::getUndefinedValue(), opts.getHermesValue())
      .toCallResultHermesValue();
}

Handle<JSObject> createHermesInternalObject(
    Runtime *runtime,
    const JSLibFlags &flags) {
  namespace P = Predefined;
  Handle<JSObject> intern = runtime->makeHandle(JSObject::create(runtime));
  GCScope gcScope{runtime};

  DefinePropertyFlags constantDPF =
      DefinePropertyFlags::getDefaultNewPropertyFlags();
  constantDPF.enumerable = 0;
  constantDPF.writable = 0;
  constantDPF.configurable = 0;

  auto defineInternMethod =
      [&](Predefined::Str symID, NativeFunctionPtr func, uint8_t count = 0) {
        (void)defineMethod(
            runtime,
            intern,
            Predefined::getSymbolID(symID),
            nullptr /* context */,
            func,
            count,
            constantDPF);
      };

  auto defineInternMethodAndSymbol =
      [&](const char *name, NativeFunctionPtr func, uint8_t count = 0) {
        ASCIIRef ref = createASCIIRef(name);
        Handle<SymbolID> symHandle = runtime->ignoreAllocationFailure(
            runtime->getIdentifierTable().getSymbolHandle(runtime, ref));
        (void)defineMethod(
            runtime,
            intern,
            *symHandle,
            nullptr /* context */,
            func,
            count,
            constantDPF);
      };

  // suppress unused-variable warning
  (void)defineInternMethodAndSymbol;

  // Make a copy of the original String.prototype.concat implementation that we
  // can use internally.
  // TODO: we can't make HermesInternal.concat a static builtin method now
  // because this method should be called with a meaningful `this`, but
  // CallBuiltin instruction does not support it.
  auto propRes = JSObject::getNamed_RJS(
      runtime->makeHandle<JSObject>(runtime->stringPrototype),
      runtime,
      Predefined::getSymbolID(Predefined::concat));
  assert(
      propRes != ExecutionStatus::EXCEPTION && !(*propRes)->isUndefined() &&
      "Failed to get String.prototype.concat.");
  auto putRes = JSObject::defineOwnProperty(
      intern,
      runtime,
      Predefined::getSymbolID(Predefined::concat),
      constantDPF,
      runtime->makeHandle(std::move(*propRes)));
  assert(
      putRes != ExecutionStatus::EXCEPTION && *putRes &&
      "Failed to set HermesInternal.concat.");
  (void)putRes;

  // HermesInternal functions that are known to be safe and are required to be
  // present by the VM internals even under a security-sensitive environment
  // where HermesInternal might be explicitly disabled.
  defineInternMethod(P::hasPromise, hermesInternalHasPromise);
  defineInternMethod(P::enqueueJob, hermesInternalEnqueueJob);
  defineInternMethod(
      P::setPromiseRejectionTrackingHook,
      hermesInternalSetPromiseRejectionTrackingHook);
  defineInternMethod(
      P::enablePromiseRejectionTracker,
      hermesInternalEnablePromiseRejectionTracker);
  defineInternMethod(P::useEngineQueue, hermesInternalUseEngineQueue);

  // All functions are known to be safe can be defined above this flag check.
  if (!flags.enableHermesInternal) {
    JSObject::preventExtensions(*intern);
    return intern;
  }

  // HermesInternal functions that are not necessarily required but are
  // generally considered harmless to be exposed by default.
  defineInternMethod(P::getEpilogues, hermesInternalGetEpilogues);
  defineInternMethod(
      P::getInstrumentedStats, hermesInternalGetInstrumentedStats);
  defineInternMethod(
      P::getRuntimeProperties, hermesInternalGetRuntimeProperties);
  defineInternMethod(P::ttiReached, hermesInternalTTIReached);
  defineInternMethod(P::ttrcReached, hermesInternalTTRCReached);
  defineInternMethod(P::getFunctionLocation, hermesInternalGetFunctionLocation);

  // HermesInternal function that are only meant to be used for testing purpose.
  // They can change language semantics and are security risks.
  if (flags.enableHermesInternalTestMethods) {
    defineInternMethod(
        P::detachArrayBuffer, hermesInternalDetachArrayBuffer, 1);
    defineInternMethod(P::getWeakSize, hermesInternalGetWeakSize);
    defineInternMethod(
        P::copyDataProperties, hermesBuiltinCopyDataProperties, 3);
    defineInternMethodAndSymbol("isProxy", hermesInternalIsProxy);
    defineInternMethod(P::drainJobs, hermesInternalDrainJobs);
  }

#ifdef HERMESVM_EXCEPTION_ON_OOM
  defineInternMethodAndSymbol("getCallStack", hermesInternalGetCallStack, 0);
#endif // HERMESVM_EXCEPTION_ON_OOM

  JSObject::preventExtensions(*intern);

  return intern;
}

} // namespace vm
} // namespace hermes
