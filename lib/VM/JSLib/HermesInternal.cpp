/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringView.h"

#include <cstring>
#include <random>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

/// \return a SymbolID  for a given C string \p s.
static inline CallResult<Handle<SymbolID>> symbolForCStr(
    Runtime &rt,
    const char *s) {
  return rt.getIdentifierTable().getSymbolHandle(rt, ASCIIRef{s, strlen(s)});
}

// ES7 24.1.1.3
CallResult<HermesValue>
hermesInternalDetachArrayBuffer(void *, Runtime &runtime, NativeArgs args) {
  auto buffer = args.dyncastArg<JSArrayBuffer>(0);
  if (!buffer) {
    return runtime.raiseTypeError(
        "Cannot use detachArrayBuffer on something which "
        "is not an ArrayBuffer foo");
  }
  if (LLVM_UNLIKELY(
          JSArrayBuffer::detach(runtime, buffer) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  // "void" return
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
hermesInternalGetEpilogues(void *, Runtime &runtime, NativeArgs args) {
  // Create outer array with one element per module.
  auto eps = runtime.getEpilogues();
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
      auto result = Uint8Array::allocate(runtime, innerLen);
      if (result == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      auto ta = result.getValue();
      std::memcpy(ta->begin(runtime), eps[i].begin(), innerLen);
      const auto shv = SmallHermesValue::encodeObjectValue(*ta, runtime);
      JSArray::unsafeSetExistingElementAt(*outer, runtime, i, shv);
    }
  }
  return HermesValue::encodeObjectValue(*outer);
}

/// Used for testing, determines how many live values
/// are in the given WeakMap or WeakSet.
CallResult<HermesValue>
hermesInternalGetWeakSize(void *, Runtime &runtime, NativeArgs args) {
  if (auto M = args.dyncastArg<JSWeakMap>(0)) {
    return HermesValue::encodeUntrustedNumberValue(
        JSWeakMap::debugFreeSlotsAndGetSize(runtime, *M));
  }

  if (auto S = args.dyncastArg<JSWeakSet>(0)) {
    return HermesValue::encodeUntrustedNumberValue(
        JSWeakSet::debugFreeSlotsAndGetSize(runtime, *S));
  }

  return runtime.raiseTypeError(
      "getWeakSize can only be called on a WeakMap/WeakSet");
}

/// \return an object containing various instrumented statistics.
CallResult<HermesValue>
hermesInternalGetInstrumentedStats(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto resultHandle = runtime.makeHandle(JSObject::create(runtime));

  /// Adds \p key with \p val to the resultHandle object.
  auto addToResultHandle = [&](llvh::StringRef key, double v) {
    GCScopeMarkerRAII marker{gcScope};
    HermesValue val = HermesValue::encodeUntrustedNumberValue(v);
    Handle<> valHandle = runtime.makeHandle(val);
    auto keySym = symbolForCStr(runtime, key.data());
    if (LLVM_UNLIKELY(keySym == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    return JSObject::defineNewOwnProperty(
        resultHandle,
        runtime,
        **keySym,
        PropertyFlags::defaultNewNamedPropertyFlags(),
        valHandle);
  };

  /// Adds a property to resultHandle. \p key provides its name, and \p val,
  /// its value.
#define ADD_PROP(name, value)                                              \
  if (LLVM_UNLIKELY(                                                       \
          addToResultHandle(name, value) == ExecutionStatus::EXCEPTION)) { \
    return ExecutionStatus::EXCEPTION;                                     \
  }

  auto &heap = runtime.getHeap();
  GCBase::HeapInfo info;
  heap.getHeapInfo(info);

  ADD_PROP("js_VMExperiments", runtime.getVMExperimentFlags());
  ADD_PROP("js_numGCs", heap.getNumGCs());
  ADD_PROP("js_gcCPUTime", heap.getGCCPUTime());
  ADD_PROP("js_gcTime", heap.getGCTime());
  ADD_PROP("js_totalAllocatedBytes", info.totalAllocatedBytes);
  ADD_PROP("js_allocatedBytes", info.allocatedBytes);
  ADD_PROP("js_heapSize", info.heapSize);
  ADD_PROP("js_mallocSizeEstimate", info.mallocSizeEstimate);
  ADD_PROP("js_vaSize", info.va);
  ADD_PROP("js_externalBytes", info.externalBytes);
  ADD_PROP("js_markStackOverflows", info.numMarkStackOverflows);
#undef ADD_PROP

  return resultHandle.getHermesValue();
}

/// \return a static string summarising the presence and resolution type of
/// CommonJS modules across all RuntimeModules that have been loaded into \c
/// runtime.
static const char *getCJSModuleModeDescription(Runtime &runtime) {
  bool hasCJSModulesDynamic = false;
  bool hasCJSModulesStatic = false;
  for (const auto &runtimeModule : runtime.getRuntimeModules()) {
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
hermesInternalGetRuntimeProperties(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto resultHandle = runtime.makeHandle(JSObject::create(runtime));
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

  tmpHandle =
      HermesValue::encodeUntrustedNumberValue(::hermes::hbc::BYTECODE_VERSION);
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "Bytecode Version") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  tmpHandle = HermesValue::encodeBoolValue(runtime.builtinsAreFrozen());
  if (LLVM_UNLIKELY(
          addProperty(tmpHandle, "Builtins Frozen") ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  tmpHandle =
      HermesValue::encodeUntrustedNumberValue(runtime.getVMExperimentFlags());
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

  std::string gcKind = runtime.getHeap().getKindAsStr();
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
static void logGCStats(Runtime &runtime, const char *msg) {
  // The GC stats can exceed the android logcat length limit, of
  // 1024 bytes.  Break it up.
  std::string stats;
  {
    llvh::raw_string_ostream os(stats);
    runtime.printHeapStats(os);
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
hermesInternalTTIReached(void *, Runtime &runtime, NativeArgs args) {
  runtime.ttiReached();
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
hermesInternalTTRCReached(void *, Runtime &runtime, NativeArgs args) {
  // Currently does nothing, but could change in the future.
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
hermesInternalIsProxy(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> obj = args.dyncastArg<JSObject>(0);
  return HermesValue::encodeBoolValue(obj && obj->isProxyObject());
}

CallResult<HermesValue>
hermesInternalHasPromise(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeBoolValue(runtime.hasES6Promise());
}

CallResult<HermesValue>
hermesInternalHasES6Class(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeBoolValue(runtime.hasES6Class());
}

CallResult<HermesValue>
hermesInternalUseEngineQueue(void *, Runtime &runtime, NativeArgs args) {
  return HermesValue::encodeBoolValue(runtime.hasMicrotaskQueue());
}

/// \code
///   HermesInternal.enqueueJob = function (func) {}
/// \endcode
CallResult<HermesValue>
hermesInternalEnqueueJob(void *, Runtime &runtime, NativeArgs args) {
  auto callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return runtime.raiseTypeError(
        "Argument to HermesInternal.enqueueJob must be callable");
  }
  runtime.enqueueJob(callable.get());
  return HermesValue::encodeUndefinedValue();
}

/// \code
///   HermesInternal.drainJobs = function () {}
/// \endcode
/// Throw if the drainJobs throws.
CallResult<HermesValue>
hermesInternalDrainJobs(void *, Runtime &runtime, NativeArgs args) {
  auto drainRes = runtime.drainJobs();
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
hermesInternalGetCallStack(void *, Runtime &runtime, NativeArgs args) {
  std::string stack = runtime.getCallStackNoAlloc();
  return StringPrimitive::create(runtime, ASCIIRef(stack.data(), stack.size()));
}
#endif // HERMESVM_EXCEPTION_ON_OOM

/// \return the code block associated with \p callableHandle if it is a
/// (possibly bound) JS function, or nullptr otherwise.
static const CodeBlock *getLeafCodeBlock(
    Handle<Callable> callableHandle,
    Runtime &runtime) {
  const Callable *callable = callableHandle.get();
  while (auto *bound = dyn_vmcast<BoundFunction>(callable)) {
    callable = bound->getTarget(runtime);
  }
  if (auto *asFunction = dyn_vmcast<const JSFunction>(callable)) {
    return asFunction->getCodeBlock(runtime);
  }
  return nullptr;
}

/// \return the file name associated with \p codeBlock, if any.
/// This mirrors the way we print file names for code blocks in JSError.
static CallResult<HermesValue> getCodeBlockFileName(
    Runtime &runtime,
    const CodeBlock *codeBlock,
    OptValue<hbc::DebugSourceLocation> location) {
  RuntimeModule *runtimeModule = codeBlock->getRuntimeModule();
  if (!runtimeModule->getBytecode()->isLazy()) {
    // Lazy code blocks do not have debug information (and will hermes_fatal if
    // you try to access it), so only touch it for non-lazy blocks.
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
hermesInternalGetFunctionLocation(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);

  auto callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return runtime.raiseTypeError(
        "Argument to HermesInternal.getFunctionLocation must be callable");
  }
  auto resultHandle = runtime.makeHandle(JSObject::create(runtime));
  MutableHandle<> tmpHandle{runtime};

  auto codeBlock = getLeafCodeBlock(callable, runtime);
  bool isNative = !codeBlock;
  auto res = JSObject::defineOwnProperty(
      resultHandle,
      runtime,
      Predefined::getSymbolID(Predefined::isNative),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      runtime.getBoolValue(isNative));
  assert(res != ExecutionStatus::EXCEPTION && "Failed to set isNative");
  (void)res;

  if (codeBlock) {
    OptValue<hbc::DebugSourceLocation> location =
        codeBlock->getSourceLocation();
    if (location) {
      tmpHandle = HermesValue::encodeUntrustedNumberValue(location->line);
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::lineNumber),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(res != ExecutionStatus::EXCEPTION && "Failed to set lineNumber");
      (void)res;

      tmpHandle = HermesValue::encodeUntrustedNumberValue(location->column);
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::columnNumber),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(res != ExecutionStatus::EXCEPTION && "Failed to set columnNumber");
      (void)res;
    } else {
      tmpHandle = HermesValue::encodeUntrustedNumberValue(
          codeBlock->getRuntimeModule()->getBytecode()->getSegmentID());
      res = JSObject::defineOwnProperty(
          resultHandle,
          runtime,
          Predefined::getSymbolID(Predefined::segmentID),
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          tmpHandle);
      assert(res != ExecutionStatus::EXCEPTION && "Failed to set segmentID");
      (void)res;

      tmpHandle = HermesValue::encodeUntrustedNumberValue(
          codeBlock->getVirtualOffset());
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
    Runtime &runtime,
    NativeArgs args) {
  runtime.promiseRejectionTrackingHook_ = args.getArg(0);
  return HermesValue::encodeUndefinedValue();
}

/// \code
///   HermesInternal.enablePromiseRejectionTracker = function (opts) {}
/// \endcode
/// Enable promise rejection tracking with the given opts.
CallResult<HermesValue> hermesInternalEnablePromiseRejectionTracker(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  auto opts = args.getArgHandle(0);
  auto func = Handle<Callable>::dyn_vmcast(
      Handle<>(&runtime.promiseRejectionTrackingHook_));
  if (!func) {
    return runtime.raiseTypeError(
        "Promise rejection tracking hook was not registered");
  }
  return Callable::executeCall1(
             func, runtime, Runtime::getUndefinedValue(), opts.getHermesValue())
      .toCallResultHermesValue();
}

#ifdef HERMES_ENABLE_FUZZILLI

/// Internal "fuzzilli" function used by the Fuzzilli fuzzer
/// (https://github.com/googleprojectzero/fuzzilli) to sanity-check the engine.
/// This function is conditionally defined in Hermes internal VM code rather
/// than in an external fuzzing module so to catch build misconfigurations, e.g.
/// we want to make sure that the VM is compiled with assertions enabled and
/// doing this check out of the VM (e.g. in the fuzzing harness) doesn't
/// guarantee that the VM has asserts on.
///
/// This function is defined as follow:
/// \code
/// HermesInternal.fuzzilli = function(op, arg) {}
/// \endcode
/// The first argument "op", is a string specifying the operation to be
/// performed. Currently supported values of "op" are "FUZZILLI_CRASH", used to
/// simulate a crash, and "FUZZILLI_PRINT", used to send data over Fuzzilli's
/// ata write file decriptor (REPRL_DWFD). The secong argument "arg" can be an
/// integer specifying the type of crash (if op is "FUZZILLI_CRASH") or a string
/// which value will be sent to fuzzilli (if op is "FUZZILLI_PRINT")
CallResult<HermesValue>
hermesInternalFuzzilli(void *, Runtime &runtime, NativeArgs args) {
  // REPRL = read-eval-print-reset-loop
  // This file descriptor is being opened by Fuzzilli
  constexpr int REPRL_DWFD = 103; // Data write file decriptor

  auto operationRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(operationRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto operation = StringPrimitive::createStringView(
      runtime, runtime.makeHandle(std::move(*operationRes)));

  if (operation.equals(createUTF16Ref(u"FUZZILLI_CRASH"))) {
    auto crashTypeRes = toIntegerOrInfinity(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(crashTypeRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    switch (crashTypeRes->getNumberAs<int>()) {
      case 0:
        *((int *)0x41414141) = 0x1337;
        break;
      case 1:
        assert(0);
        break;
      case 2:
        std::abort();
        break;
    }
  } else if (operation.equals(createUTF16Ref(u"FUZZILLI_PRINT"))) {
    static FILE *fzliout = fdopen(REPRL_DWFD, "w");
    if (!fzliout) {
      fprintf(
          stderr,
          "Fuzzer output channel not available, printing to stdout instead\n");
      fzliout = stdout;
    }

    auto printRes = toString_RJS(runtime, args.getArgHandle(1));
    if (LLVM_UNLIKELY(printRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto print = StringPrimitive::createStringView(
        runtime, runtime.makeHandle(std::move(*printRes)));

    vm::SmallU16String<32> allocator;
    std::string outputString;
    ::hermes::convertUTF16ToUTF8WithReplacements(
        outputString, print.getUTF16Ref(allocator));
    fprintf(fzliout, "%s\n", outputString.c_str());
    fflush(fzliout);
  }

  return HermesValue::encodeUndefinedValue();
}
#endif // HERMES_ENABLE_FUZZILLI

static CallResult<HermesValue>
hermesInternalIsLazy(void *, Runtime &runtime, NativeArgs args) {
  auto callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return HermesValue::encodeBoolValue(false);
  }

  auto codeBlock = getLeafCodeBlock(callable, runtime);
  if (!codeBlock) {
    // Native function is never lazy.
    return HermesValue::encodeBoolValue(false);
  }

  RuntimeModule *runtimeModule = codeBlock->getRuntimeModule();
  return HermesValue::encodeBoolValue(
      runtimeModule && runtimeModule->getBytecode()->isLazy());
}

Handle<JSObject> createHermesInternalObject(
    Runtime &runtime,
    const JSLibFlags &flags) {
  namespace P = Predefined;
  Handle<JSObject> intern = runtime.makeHandle(JSObject::create(runtime));
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
        Handle<SymbolID> symHandle = runtime.ignoreAllocationFailure(
            runtime.getIdentifierTable().getSymbolHandle(runtime, ref));
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
      runtime.makeHandle<JSObject>(runtime.stringPrototype),
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
      runtime.makeHandle(std::move(*propRes)));
  assert(
      putRes != ExecutionStatus::EXCEPTION && *putRes &&
      "Failed to set HermesInternal.concat.");
  (void)putRes;

  // HermesInternal functions that are known to be safe and are required to be
  // present by the VM internals even under a security-sensitive environment
  // where HermesInternal might be explicitly disabled.
  defineInternMethod(P::hasPromise, hermesInternalHasPromise);
  defineInternMethod(P::hasES6Class, hermesInternalHasES6Class);
  defineInternMethod(P::enqueueJob, hermesInternalEnqueueJob);
  defineInternMethod(
      P::setPromiseRejectionTrackingHook,
      hermesInternalSetPromiseRejectionTrackingHook);
  defineInternMethod(
      P::enablePromiseRejectionTracker,
      hermesInternalEnablePromiseRejectionTracker);
  defineInternMethod(P::useEngineQueue, hermesInternalUseEngineQueue);

#ifdef HERMES_ENABLE_FUZZILLI
  defineInternMethod(P::fuzzilli, hermesInternalFuzzilli);
#endif

  // All functions are known to be safe can be defined above this flag check.
  if (!flags.enableHermesInternal) {
    JSObject::preventExtensions(*intern);
    return intern;
  }

  // HermesInternal functions that are not necessarily required but are
  // generally considered harmless to be exposed by default.
  defineInternMethod(P::getEpilogues, hermesInternalGetEpilogues);
  defineInternMethod(
      P::getRuntimeProperties, hermesInternalGetRuntimeProperties);
  defineInternMethod(P::ttiReached, hermesInternalTTIReached);
  defineInternMethod(P::ttrcReached, hermesInternalTTRCReached);
  defineInternMethod(P::getFunctionLocation, hermesInternalGetFunctionLocation);

  if (LLVM_UNLIKELY(runtime.traceMode != SynthTraceMode::None)) {
    // Use getNewNonEnumerableFlags() so that getInstrumentedStats can be
    // overridden for synth trace case. See TracingRuntime.cpp.
    (void)defineMethod(
        runtime,
        intern,
        Predefined::getSymbolID(P::getInstrumentedStats),
        nullptr /* context */,
        hermesInternalGetInstrumentedStats,
        0,
        DefinePropertyFlags::getNewNonEnumerableFlags());
  } else {
    defineInternMethod(
        P::getInstrumentedStats, hermesInternalGetInstrumentedStats);
  }

  // HermesInternal function that are only meant to be used for testing purpose.
  // They can change language semantics and are security risks.
  if (flags.enableHermesInternalTestMethods) {
    defineInternMethod(
        P::detachArrayBuffer, hermesInternalDetachArrayBuffer, 1);
    defineInternMethod(P::getWeakSize, hermesInternalGetWeakSize);
    defineInternMethod(
        P::copyDataProperties, hermesBuiltinCopyDataProperties, 3);
    defineInternMethodAndSymbol("isProxy", hermesInternalIsProxy);
    defineInternMethodAndSymbol("isLazy", hermesInternalIsLazy);
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
