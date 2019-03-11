/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "JSLibInternal.h"

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"

namespace hermes {
namespace vm {

namespace {

/// \return a SymbolID  for a given C string \p s.
static inline CallResult<Handle<SymbolID>> symbolForCStr(
    Runtime *rt,
    const char *s) {
  return rt->getIdentifierTable().getSymbolHandle(rt, ASCIIRef{s, strlen(s)});
}

// ES7 24.1.1.3
CallResult<HermesValue>
hermesInternalDetachArrayBuffer(void *, Runtime *runtime, NativeArgs args) {
  auto buffer = args.dyncastArg<JSArrayBuffer>(runtime, 0);
  if (!buffer) {
    return runtime->raiseTypeError(
        "Cannot use detachArrayBuffer on something which "
        "is not an ArrayBuffer foo");
  }
  buffer->detach(&runtime->getHeap());
  // "void" return
  return HermesValue::encodeUndefinedValue();
}

/// An API for JS code to request a heap snapshot at any point of execution.
/// This runs a garbage collection first, in order to not show any garbage in
/// the snapshot.
CallResult<HermesValue>
hermesInternalCreateHeapSnapshot(void *, Runtime *runtime, NativeArgs args) {
  bool compact = true;
  if (args.getArgCount() >= 1) {
    compact = toBoolean(args.getArg(0));
  }
  runtime->getHeap().createSnapshot(llvm::errs(), compact);
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
  auto outer = toHandle(runtime, std::move(*outerResult));
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
      std::memcpy(ta->begin(), eps[i].begin(), innerLen);
      JSArray::unsafeSetExistingElementAt(
          *outer, runtime, i, ta.getHermesValue());
    }
  }
  return HermesValue::encodeObjectValue(*outer);
}

/// Set the parent of an object failing silently on any error.
CallResult<HermesValue>
silentObjectSetPrototypeOf(void *, Runtime *runtime, NativeArgs args) {
  JSObject *O = dyn_vmcast<JSObject>(args.getArg(0));
  if (!O)
    return HermesValue::encodeUndefinedValue();

  JSObject *parent;
  HermesValue V = args.getArg(1);
  if (V.isNull())
    parent = nullptr;
  else if (V.isObject())
    parent = vmcast<JSObject>(V);
  else
    return HermesValue::encodeUndefinedValue();

  JSObject::setParent(O, runtime, parent);

  // Ignore exceptions.
  runtime->clearThrownValue();

  return HermesValue::encodeUndefinedValue();
}

/// Used for testing, determines how many live values
/// are in the given WeakMap or WeakSet.
CallResult<HermesValue>
hermesInternalGetWeakSize(void *, Runtime *runtime, NativeArgs args) {
  auto M = args.dyncastArg<JSWeakMap>(runtime, 0);
  if (M) {
    return HermesValue::encodeNumberValue(JSWeakMap::debugGetSize(*M));
  }

  auto S = args.dyncastArg<JSWeakSet>(runtime, 0);
  if (S) {
    return HermesValue::encodeNumberValue(JSWeakSet::debugGetSize(*S));
  }

  return runtime->raiseTypeError(
      "getWeakSize can only be called on a WeakMap/WeakSet");
}

/// \return an object containing various instrumented statistics.
CallResult<HermesValue>
hermesInternalGetInstrumentedStats(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto resultHandle = toHandle(runtime, JSObject::create(runtime));
  MutableHandle<> tmpHandle{runtime};

  namespace P = Predefined;
/// Adds a property to \c resultHandle. \p KEY provides its name as a \c
/// Predefined enum value, and its value is rooted in \p VALUE.  If property
/// definition fails, the exceptional execution status will be propogated to the
/// outer function.
#define SET_PROP(KEY, VALUE)                                   \
  do {                                                         \
    GCScopeMarkerRAII marker{gcScope};                         \
    tmpHandle = HermesValue::encodeDoubleValue(VALUE);         \
    auto status = JSObject::defineNewOwnProperty(              \
        resultHandle,                                          \
        runtime,                                               \
        runtime->getPredefinedSymbolID(KEY),                   \
        PropertyFlags::defaultNewNamedPropertyFlags(),         \
        tmpHandle);                                            \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
  } while (false)

  auto &stats = runtime->getRuntimeStats();

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

  const auto &heap = runtime->getHeap();
  SET_PROP(P::js_numGCs, heap.getNumGCs());
  SET_PROP(P::js_gcCPUTime, heap.getGCCPUTime());
  SET_PROP(P::js_gcTime, heap.getGCTime());

#undef SET_PROP

/// Adds a property to \c resultHandle. \p KEY provides its name as a C string,
/// and its value is rooted in \p VALUE.  If property definition fails, the
/// exceptional execution status will be propogated to the outer function.
#define SET_PROP_NEW(KEY, VALUE)                               \
  do {                                                         \
    auto keySym = symbolForCStr(runtime, KEY);                 \
    if (LLVM_UNLIKELY(keySym == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
    tmpHandle = HermesValue::encodeDoubleValue(VALUE);         \
    auto status = JSObject::defineNewOwnProperty(              \
        resultHandle,                                          \
        runtime,                                               \
        **keySym,                                              \
        PropertyFlags::defaultNewNamedPropertyFlags(),         \
        tmpHandle);                                            \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) { \
      return ExecutionStatus::EXCEPTION;                       \
    }                                                          \
  } while (false)

  if (runtime->getRuntimeStats().shouldSample) {
    size_t bytecodePagesResident = 0;
    for (auto &module : runtime->getRuntimeModules()) {
      auto buf = module.getBytecode()->getRawBuffer();
      if (buf.size()) {
        int pages = oscompat::pages_in_ram(buf.data(), buf.size());
        if (pages >= 0) {
          bytecodePagesResident += pages;
        }
      }
    }
    SET_PROP_NEW("js_bytecodePagesResident", bytecodePagesResident);
  }

  return resultHandle.getHermesValue();

#undef SET_PROP_NEW
}

/// \return an object mapping keys to runtime property values.
CallResult<HermesValue>
hermesInternalGetRuntimeProperties(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto resultHandle = toHandle(runtime, JSObject::create(runtime));
  MutableHandle<> tmpHandle{runtime};

  auto bcVersion = symbolForCStr(runtime, "Bytecode Version");
  if (LLVM_UNLIKELY(bcVersion == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = HermesValue::encodeDoubleValue(::hermes::hbc::BYTECODE_VERSION);
  auto status = JSObject::defineNewOwnProperty(
      resultHandle,
      runtime,
      **bcVersion,
      PropertyFlags::defaultNewNamedPropertyFlags(),
      tmpHandle);

  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto builtinsAreFrozen = symbolForCStr(runtime, "Builtins Frozen");
  if (LLVM_UNLIKELY(builtinsAreFrozen == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  tmpHandle = HermesValue::encodeBoolValue(runtime->builtinsAreFrozen());
  status = JSObject::defineNewOwnProperty(
      resultHandle,
      runtime,
      **builtinsAreFrozen,
      PropertyFlags::defaultNewNamedPropertyFlags(),
      tmpHandle);
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return resultHandle.getHermesValue();
}

} // namespace

Handle<JSObject> createHermesInternalObject(Runtime *runtime) {
  Handle<JSObject> intern = toHandle(runtime, JSObject::create(runtime));

  DefinePropertyFlags constantDPF{};
  constantDPF.setEnumerable = 1;
  constantDPF.setWritable = 1;
  constantDPF.setConfigurable = 1;
  constantDPF.setValue = 1;
  constantDPF.enumerable = 0;
  constantDPF.writable = 0;
  constantDPF.configurable = 0;

  auto defineInternMethod =
      [&](Predefined::Str symID, NativeFunctionPtr func, uint8_t count = 0) {
        (void)defineMethod(
            runtime,
            intern,
            runtime->getPredefinedSymbolID(symID),
            nullptr /* context */,
            func,
            count,
            constantDPF);
      };

  // HermesInternal function properties
  namespace P = Predefined;
  defineInternMethod(P::detachArrayBuffer, hermesInternalDetachArrayBuffer, 1);
  defineInternMethod(P::createHeapSnapshot, hermesInternalCreateHeapSnapshot);
  defineInternMethod(P::getEpilogues, hermesInternalGetEpilogues);
  defineInternMethod(P::silentSetPrototypeOf, silentObjectSetPrototypeOf, 2);
  defineInternMethod(P::getWeakSize, hermesInternalGetWeakSize);
  defineInternMethod(
      P::getInstrumentedStats, hermesInternalGetInstrumentedStats);
  defineInternMethod(
      P::getRuntimeProperties, hermesInternalGetRuntimeProperties);

  // Define the 'require' function.
  runtime->requireFunction = *defineMethod(
      runtime,
      intern,
      runtime->getPredefinedSymbolID(Predefined::require),
      nullptr,
      require,
      1,
      constantDPF);

  // Define the 'requireFast' function, which takes a number argument.
  (void)defineMethod(
      runtime,
      intern,
      runtime->getPredefinedSymbolID(Predefined::requireFast),
      nullptr,
      requireFast,
      1,
      constantDPF);

  JSObject::preventExtensions(*intern);

  return intern;
}

} // namespace vm
} // namespace hermes
