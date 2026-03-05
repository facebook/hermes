/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/ConsoleHost/ConsoleHost.h"

#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/OutputStream.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/TimeLimitMonitor.h"
#include "hermes/VM/instrumentation/PerfEvents.h"
#include "hermes/hermes.h"

#include "jsi/jsi.h"

namespace hermes {

ConsoleHostContext::ConsoleHostContext(vm::Runtime &runtime) {
  runtime.addCustomRootsFunction([this](vm::GC *, vm::RootAcceptor &acceptor) {
    for (auto &entry : taskQueue_) {
      acceptor.acceptPtr(entry.second);
    }
  });
}

/// Raises an uncatchable quit exception.
static vm::CallResult<vm::HermesValue> quit(void *, vm::Runtime &runtime) {
  return runtime.raiseQuitError();
}

static void printStats(
    vm::Runtime &runtime,
    llvh::raw_ostream &os,
    const std::vector<vm::GCAnalyticsEvent> *gcAnalyticsEvents) {
  std::string stats;
  {
    llvh::raw_string_ostream tmp{stats};
    runtime.printHeapStats(tmp);
  }
  vm::instrumentation::PerfEvents::endAndInsertStats(stats);

  if (gcAnalyticsEvents) {
    llvh::raw_string_ostream tmp{stats};
    tmp << "Collections:\n";
    ::hermes::JSONEmitter json{tmp, /*pretty*/ true};
    json.openArray();
    for (const auto &event : *gcAnalyticsEvents) {
      json.openDict();
      json.emitKeyValue("runtimeDescription", event.runtimeDescription);
      json.emitKeyValue("gcKind", event.gcKind);
      json.emitKeyValue("collectionType", event.collectionType);
      json.emitKeyValue("cause", event.cause);
      json.emitKeyValue("duration", event.duration.count());
      json.emitKeyValue("cpuDuration", event.cpuDuration.count());
      json.emitKeyValue("preAllocated", event.allocated.before);
      json.emitKeyValue("postAllocated", event.allocated.after);
      json.emitKeyValue("preSize", event.size.before);
      json.emitKeyValue("postSize", event.size.after);
      json.emitKeyValue("preExternal", event.external.before);
      json.emitKeyValue("postExternal", event.external.after);
      json.emitKeyValue("survivalRatio", event.survivalRatio);
      json.emitKey("tags");
      json.openArray();
      for (const auto &tag : event.tags) {
        json.emitValue(tag);
      }
      json.closeArray();
      json.closeDict();
    }
    json.closeArray();
    tmp << "\n";
  }

  os << stats;
}

static vm::CallResult<vm::HermesValue> createHeapSnapshot(
    void *,
    vm::Runtime &runtime) {
#ifdef HERMES_MEMORY_INSTRUMENTATION
  vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  using namespace vm;
  std::string fileName;
  if (args.getArgCount() >= 1 && !args.getArg(0).isUndefined()) {
    if (!args.getArg(0).isString()) {
      return runtime.raiseTypeError("Filename argument must be a string");
    }
    auto str = Handle<StringPrimitive>::vmcast(args.getArgHandle(0));
    auto jsFileName = StringPrimitive::createStringView(runtime, str);
    llvh::SmallVector<char16_t, 16> buf;
    convertUTF16ToUTF8WithReplacements(fileName, jsFileName.getUTF16Ref(buf));
  }

  if (fileName.empty()) {
    // "-" is recognized as stdout.
    fileName = "-";
  } else if (
      !llvh::StringRef{fileName}.endswith(".heapsnapshot") &&
      !llvh::StringRef{fileName}.endswith(".heaptimeline")) {
    return runtime.raiseTypeError(
        "Filename must end in .heapsnapshot or .heaptimeline");
  }
  std::error_code err;
  llvh::raw_fd_ostream os(fileName, err, llvh::sys::fs::FileAccess::FA_Write);
  if (err) {
    // This isn't a TypeError, but no other built-in can express file errors,
    // so this will have to do.
    return runtime.raiseTypeError(
        TwineChar16("Could not write out to the file located at \"") +
        llvh::StringRef(fileName) +
        "\". System error: " + llvh::StringRef(err.message()));
  }
  // Taking a snapshot always starts with garbage collection.
  runtime.collect("snapshot");
  runtime.getHeap().createSnapshot(os, true);
  return HermesValue::encodeUndefinedValue();
#else // !defined(HERMES_MEMORY_INSTRUMENTATION)
  return runtime.raiseTypeError(
      "Heap snapshotting requires a build with memory instrumentation");
#endif // !defined(HERMES_MEMORY_INSTRUMENTATION)
}

static vm::CallResult<vm::HermesValue> loadSegment(
    void *ctx,
    vm::Runtime &runtime) {
  vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  using namespace hermes::vm;
  const auto *baseFilename = reinterpret_cast<std::string *>(ctx);

  auto requireContext = args.dyncastArg<RequireContext>(0);
  if (!requireContext) {
    return runtime.raiseTypeError(
        "First argument to loadSegment must be context");
  }

  auto segmentRes = toUInt32_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(segmentRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t segment = segmentRes->getNumberAs<uint32_t>();

  auto fileBufRes =
      llvh::MemoryBuffer::getFile(Twine(*baseFilename) + "." + Twine(segment));
  if (!fileBufRes) {
    return runtime.raiseTypeError(
        TwineChar16("Failed to open segment: ") + segment);
  }

  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::make_unique<OwnedMemoryBuffer>(std::move(*fileBufRes)));
  if (!ret.first) {
    return runtime.raiseTypeError("Error deserializing bytecode");
  }

  if (LLVM_UNLIKELY(
          runtime.loadSegment(std::move(ret.first), requireContext) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeUndefinedValue();
}

static vm::CallResult<vm::HermesValue> setTimeout(
    void *ctx,
    vm::Runtime &runtime) {
  vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  ConsoleHostContext *consoleHost = (ConsoleHostContext *)ctx;
  using namespace hermes::vm;
  Handle<Callable> callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return runtime.raiseTypeError("Argument to setTimeout must be a function");
  }
  CallResult<HermesValue> boundFunction = BoundFunction::create(
      runtime, callable, args.getArgCount() - 1, args.begin() + 1);
  if (boundFunction == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  uint32_t taskId = consoleHost->queueTask(
      PseudoHandle<Callable>::vmcast(createPseudoHandle(*boundFunction)));
  return HermesValue::encodeTrustedNumberValue(taskId);
}

static vm::CallResult<vm::HermesValue> clearTimeout(
    void *ctx,
    vm::Runtime &runtime) {
  vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  ConsoleHostContext *consoleHost = (ConsoleHostContext *)ctx;
  using namespace hermes::vm;
  if (!args.getArg(0).isNumber()) {
    return runtime.raiseTypeError("Argument to clearTimeout must be a number");
  }
  consoleHost->clearTask(args.getArg(0).getNumberAs<uint32_t>());
  return HermesValue::encodeUndefinedValue();
}

/// Synchronously read a file and return its contents as a Uint8Array.
static vm::CallResult<vm::HermesValue> hermescliLoadFile(
    void *,
    vm::Runtime &runtime) {
  vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  using namespace vm;

  if (args.getArgCount() < 1 || !args.getArg(0).isString()) {
    return runtime.raiseTypeError("loadFile requires a string path argument");
  }

  auto str = Handle<StringPrimitive>::vmcast(args.getArgHandle(0));
  auto jsPath = StringPrimitive::createStringView(runtime, str);
  std::string path;
  llvh::SmallVector<char16_t, 64> buf;
  convertUTF16ToUTF8WithReplacements(path, jsPath.getUTF16Ref(buf));

  auto fileBufRes = llvh::MemoryBuffer::getFile(path);
  if (!fileBufRes) {
    return runtime.raiseTypeError(
        TwineChar16("Failed to open file: ") + llvh::StringRef(path));
  }

  size_t len = (*fileBufRes)->getBufferSize();
  auto result = Uint8Array::allocate(runtime, len);
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto ta = result.getValue();
  std::memcpy(ta->data(runtime), (*fileBufRes)->getBufferStart(), len);

  return HermesValue::encodeObjectValue(*ta);
}

/// Load and execute HBC bytecode from a Uint8Array or ArrayBuffer.
/// Returns the result of the top-level function.
static vm::CallResult<vm::HermesValue> hermescliLoadHBC(
    void *,
    vm::Runtime &runtime) {
  vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  using namespace vm;

  uint8_t *data = nullptr;
  size_t len = 0;

  if (auto *ta = dyn_vmcast<JSTypedArrayBase>(args.getArg(0))) {
    data = ta->data(runtime);
    len = ta->getByteLength();
  } else if (auto *ab = dyn_vmcast<JSArrayBuffer>(args.getArg(0))) {
    data = ab->getDataBlock();
    len = ab->size();
  } else {
    return runtime.raiseTypeError(
        "loadHBC requires a Uint8Array or ArrayBuffer argument");
  }

  // Copy the data into an OwnedMemoryBuffer.
  auto ownedBuf =
      std::make_unique<OwnedMemoryBuffer>(llvh::MemoryBuffer::getMemBufferCopy(
          llvh::StringRef(reinterpret_cast<const char *>(data), len)));

  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::move(ownedBuf));
  if (!ret.first) {
    return runtime.raiseTypeError(
        TwineChar16("Error deserializing bytecode: ") +
        llvh::StringRef(ret.second));
  }

  Handle<Domain> domain = runtime.makeHandle(Domain::create(runtime));

  RuntimeModuleFlags flags;
  flags.persistent = true;

  return runtime.runBytecode(
      std::move(ret.first),
      flags,
      /*sourceURL*/ "",
      Runtime::makeNullHandle<Environment>(),
      domain);
}

/// Return an array of extra CLI arguments passed after the script filename.
static vm::CallResult<vm::HermesValue> hermescliGetScriptArgs(
    void *ctx,
    vm::Runtime &runtime) {
  using namespace vm;
  auto *consoleHost = reinterpret_cast<ConsoleHostContext *>(ctx);
  const auto &scriptArgs = consoleHost->scriptArgs_;
  auto len = static_cast<JSArray::size_type>(scriptArgs.size());

  auto arrayRes = JSArray::create(runtime, len, len);
  if (arrayRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  struct : public Locals {
    PinnedValue<JSArray> arr;
  } lv;
  LocalsRAII lraii(runtime, &lv);
  lv.arr = std::move(*arrayRes);

  if (lv.arr->setStorageEndIndex(lv.arr, runtime, len) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  for (JSArray::size_type i = 0; i < len; ++i) {
    GCScopeMarkerRAII marker{runtime};
    auto strRes = StringPrimitive::createEfficient(
        runtime, llvh::createASCIIRef(scriptArgs[i].c_str()));
    if (strRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto shv = SmallHermesValue::encodeStringValue(
        vmcast<StringPrimitive>(*strRes), runtime);
    JSArray::unsafeSetExistingElementAt(*lv.arr, runtime, i, shv);
  }

  return HermesValue::encodeObjectValue(*lv.arr);
}

/// The test262 testsuite requires below harness symbols:
/// - $262, with properties:
///     - global, alias to globalThis
///     - evalScript, alias to eval
///     - detachArrayBuffer, alias to HermesInternal.detachArrayBuffer (if it
///       exists)
/// - alert, alias to print
static void initTest262Harness(vm::Runtime &runtime) {
  vm::Handle<vm::JSObject> test262Obj =
      runtime.makeHandle(vm::JSObject::create(runtime));

  vm::DefinePropertyFlags nonEnumerableDPF =
      vm::DefinePropertyFlags::getNewNonEnumerableFlags();

  // Define $262.global
  auto global = runtime.getGlobal();
  runtime.ignoreAllocationFailure(
      vm::JSObject::defineOwnProperty(
          test262Obj,
          runtime,
          vm::Predefined::getSymbolID(vm::Predefined::global),
          nonEnumerableDPF,
          global));

  /// Try to get defined property on given JSObject \p selfHandle. If it does
  /// not exist, return None.
  auto tryGetDefinedProperty =
      [&runtime](
          vm::Handle<vm::JSObject> selfHandle,
          vm::SymbolID name) -> llvh::Optional<vm::Handle<>> {
    auto propRes = vm::JSObject::getNamed_RJS(selfHandle, runtime, name);
    if (propRes == vm::ExecutionStatus::EXCEPTION) {
      runtime.printException(
          llvh::outs(), runtime.makeHandle(runtime.getThrownValue()));
      return llvh::None;
    }

    // It may not really exist, e.g., HermesInternal.detachArrayBuffer.
    if (propRes.getValue()->isUndefined()) {
      return llvh::None;
    }

    return runtime.makeHandle(std::move(*propRes));
  };

  /// Try to copy the defined property \p srcName on \p srcSelfHandle to
  /// \p tgtSelfHandle with given name \p tgtName. If \p srcName does not exist,
  /// do nothing.
  auto tryCopyProperty = [&runtime, &nonEnumerableDPF, &tryGetDefinedProperty](
                             vm::Handle<vm::JSObject> srcSelfHandle,
                             vm::SymbolID srcName,
                             vm::Handle<vm::JSObject> tgtSelfHandle,
                             vm::SymbolID tgtName) {
    auto prop = tryGetDefinedProperty(srcSelfHandle, srcName);
    if (!prop)
      return;

    runtime.ignoreAllocationFailure(
        vm::JSObject::defineOwnProperty(
            tgtSelfHandle, runtime, tgtName, nonEnumerableDPF, *prop));
  };

  // Define $262.evalScript
  tryCopyProperty(
      global,
      vm::Predefined::getSymbolID(vm::Predefined::eval),
      test262Obj,
      vm::Predefined::getSymbolID(vm::Predefined::evalScript));

  // Define $262.detachArrayBuffer
  auto prop = tryGetDefinedProperty(
      global, vm::Predefined::getSymbolID(vm::Predefined::HermesInternal));
  if (prop) {
    vm::Handle<vm::JSObject> hermesInternalObj =
        vm::Handle<vm::JSObject>::vmcast(*prop);
    tryCopyProperty(
        hermesInternalObj,
        vm::Predefined::getSymbolID(vm::Predefined::detachArrayBuffer),
        test262Obj,
        vm::Predefined::getSymbolID(vm::Predefined::detachArrayBuffer));
  }

  // Define global object $262
  runtime.ignoreAllocationFailure(
      vm::JSObject::defineOwnProperty(
          global,
          runtime,
          vm::Predefined::getSymbolID(vm::Predefined::test262),
          nonEnumerableDPF,
          test262Obj));

  // Define global function alert()
  tryCopyProperty(
      global,
      vm::Predefined::getSymbolID(vm::Predefined::print),
      global,
      vm::Predefined::getSymbolID(vm::Predefined::alert));
}

void installConsoleBindings(
    vm::Runtime &runtime,
    ConsoleHostContext &ctx,
    vm::StatSamplingThread *statSampler,
    const std::string *filename) {
  vm::GCScopeMarkerRAII marker{runtime};
  vm::DefinePropertyFlags normalDPF =
      vm::DefinePropertyFlags::getNewNonEnumerableFlags();

  struct : public vm::Locals {
    vm::PinnedValue<vm::JSObject> console;
    vm::PinnedValue<> print;
  } lv;
  vm::LocalsRAII lraii{runtime, &lv};

  auto defineGlobalFunc = [&](vm::SymbolID name,
                              vm::NativeFunctionPtr functionPtr,
                              void *context,
                              unsigned paramCount) -> void {
    vm::GCScopeMarkerRAII marker{runtime};
    auto func = vm::NativeFunction::create(
        runtime,
        runtime.functionPrototype,
        vm::Runtime::makeNullHandle<vm::Environment>(),
        context,
        functionPtr,
        name,
        paramCount,
        vm::Runtime::makeNullHandle<vm::JSObject>());
    auto res = vm::JSObject::defineOwnProperty(
        runtime.getGlobal(), runtime, name, normalDPF, func);
    (void)res;
    assert(
        res != vm::ExecutionStatus::EXCEPTION && *res &&
        "global.defineOwnProperty() failed");
  };

  // Define the 'quit' function.
  defineGlobalFunc(
      vm::Predefined::getSymbolID(vm::Predefined::quit), quit, nullptr, 0);
  defineGlobalFunc(
      vm::Predefined::getSymbolID(vm::Predefined::createHeapSnapshot),
      createHeapSnapshot,
      nullptr,
      1);

  // Define the 'loadSegment' function.
  defineGlobalFunc(
      runtime
          .ignoreAllocationFailure(runtime.getIdentifierTable().getSymbolHandle(
              runtime, llvh::createASCIIRef("loadSegment")))
          .get(),
      loadSegment,
      reinterpret_cast<void *>(const_cast<std::string *>(filename)),
      2);

  defineGlobalFunc(
      runtime
          .ignoreAllocationFailure(runtime.getIdentifierTable().getSymbolHandle(
              runtime, llvh::createASCIIRef("setTimeout")))
          .get(),
      setTimeout,
      &ctx,
      2);
  defineGlobalFunc(
      runtime
          .ignoreAllocationFailure(runtime.getIdentifierTable().getSymbolHandle(
              runtime, llvh::createASCIIRef("clearTimeout")))
          .get(),
      clearTimeout,
      &ctx,
      1);

  // Define `setImmediate` to be the same as `setTimeout` here.
  // `setTimeout` doesn't use the time provided to it, and due to this
  // being CLI code, we don't have an event loop.
  // This allows the Promise polyfill to work enough for testing in the
  // terminal, though other hosts should provide their own implementation of the
  // event loop.
  defineGlobalFunc(
      runtime
          .ignoreAllocationFailure(runtime.getIdentifierTable().getSymbolHandle(
              runtime, llvh::createASCIIRef("setImmediate")))
          .get(),
      setTimeout,
      &ctx,
      1);

  lv.console = vm::JSObject::create(runtime);
  runtime.ignoreAllocationFailure(
      vm::JSObject::defineOwnProperty(
          runtime.getGlobal(),
          runtime,
          runtime
              .ignoreAllocationFailure(
                  runtime.getIdentifierTable().getSymbolHandle(
                      runtime, llvh::createASCIIRef("console")))
              .get(),
          normalDPF,
          lv.console));
  lv.print = runtime.ignoreAllocationFailure(
      vm::JSObject::getNamed_RJS(
          runtime.getGlobal(),
          runtime,
          vm::Predefined::getSymbolID(vm::Predefined::print)));
  runtime.ignoreAllocationFailure(
      vm::JSObject::defineOwnProperty(
          lv.console,
          runtime,
          vm::Predefined::getSymbolID(vm::Predefined::log),
          normalDPF,
          lv.print));

  initTest262Harness(runtime);

  // Register hermescli object with test utility functions, gated on the
  // -Xhermes-internal-test-methods flag.
  if (ctx.enableTestMethods_) {
    vm::Handle<vm::JSObject> hermescliObj =
        runtime.makeHandle(vm::JSObject::create(runtime));

    auto defineMethod = [&](const char *name,
                            vm::NativeFunctionPtr functionPtr,
                            void *context,
                            unsigned paramCount) {
      vm::GCScopeMarkerRAII marker{runtime};
      auto sym = runtime
                     .ignoreAllocationFailure(
                         runtime.getIdentifierTable().getSymbolHandle(
                             runtime, llvh::createASCIIRef(name)))
                     .get();
      auto func = vm::NativeFunction::create(
          runtime,
          runtime.functionPrototype,
          vm::Runtime::makeNullHandle<vm::Environment>(),
          context,
          functionPtr,
          sym,
          paramCount,
          vm::Runtime::makeNullHandle<vm::JSObject>());
      auto res = vm::JSObject::defineOwnProperty(
          hermescliObj, runtime, sym, normalDPF, func);
      (void)res;
      assert(
          res != vm::ExecutionStatus::EXCEPTION && *res &&
          "hermescli.defineOwnProperty() failed");
    };

    defineMethod("loadFile", hermescliLoadFile, nullptr, 1);
    defineMethod("loadHBC", hermescliLoadHBC, nullptr, 1);
    defineMethod("getScriptArgs", hermescliGetScriptArgs, &ctx, 0);

    runtime.ignoreAllocationFailure(
        vm::JSObject::defineOwnProperty(
            runtime.getGlobal(),
            runtime,
            runtime
                .ignoreAllocationFailure(
                    runtime.getIdentifierTable().getSymbolHandle(
                        runtime, llvh::createASCIIRef("hermescli")))
                .get(),
            normalDPF,
            hermescliObj));
  }
}

// If a function body might throw C++ exceptions other than
// jsi::JSError from Hermes, it should be wrapped in this form:
//
//   return maybeCatchException([&] { body })
//
// This will execute body; if exceptions are enabled, this execution
// will be wrapped in a try/catch that catches those exceptions, report it then
// exit.
namespace {

template <typename F>
auto maybeCatchException(const F &f) -> decltype(f()) {
#if defined(HERMESVM_EXCEPTION_ON_OOM)
  try {
    return f();
  } catch (const std::exception &ex) {
    // Report thrown exception and exit the process with failure code.
    llvh::errs() << ex.what();
    exit(1);
  }
#else // HERMESVM_EXCEPTION_ON_OOM
  return f();
#endif
}

bool executeHBCBytecodeImpl(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    const ExecuteOptions &options,
    const std::string *filename) {
  bool shouldRecordGCStats =
      options.runtimeConfig.getGCConfig().getShouldRecordStats();
  if (shouldRecordGCStats) {
    vm::instrumentation::PerfEvents::begin();
  }

  std::unique_ptr<vm::StatSamplingThread> statSampler;

  // Create HermesRuntime (JSI wrapper) - this installs JSI extensions like
  // TextEncoder. We then extract the underlying vm::Runtime for low-level
  // operations.
  auto hermesRuntime =
      facebook::hermes::makeHermesRuntime(options.runtimeConfig);
  auto *runtime = static_cast<vm::Runtime *>(
      facebook::jsi::castInterface<facebook::hermes::IHermes>(
          hermesRuntime.get())
          ->getVMRuntimeUnsafe());

  // Other JIT settings (forceJIT, jitThreshold, jitMemoryLimit) are now
  // configured via RuntimeConfig in the Runtime constructor.
  runtime->getJITContext().setDumpJITCode(options.dumpJITCode);
  runtime->getJITContext().setCrashOnError(options.jitCrashOnError);
  runtime->getJITContext().setEmitAsserts(options.jitEmitAsserts);
  runtime->getJITContext().setEmitCounters(options.jitEmitCounters);

  if (options.perfProfJitDumpFd != -1) {
    runtime->getJITContext().initPerfProfData(
        options.perfProfJitDumpFd,
        options.perfProfDebugInfoFd,
        options.perfProfDebugInfoFile);
  }

  if (options.timeLimit > 0) {
    runtime->timeLimitMonitor = vm::TimeLimitMonitor::getOrCreate();
    runtime->timeLimitMonitor->watchRuntime(
        *runtime, std::chrono::milliseconds(options.timeLimit));
  }

  if (shouldRecordGCStats) {
    statSampler = std::make_unique<vm::StatSamplingThread>(
        std::chrono::milliseconds(100));
  }

  if (options.heapTimeline) {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    runtime->enableAllocationLocationTracker();
#else
    llvh::errs() << "Failed to track allocation locations; build does not"
                    "include memory instrumentation\n";
#endif
  }

  vm::GCScope scope(*runtime);
  ConsoleHostContext ctx{*runtime};
  ctx.enableTestMethods_ =
      options.runtimeConfig.getEnableHermesInternalTestMethods();
  ctx.scriptArgs_ = options.scriptArgs;

  installConsoleBindings(*runtime, ctx, statSampler.get(), filename);

  vm::RuntimeModuleFlags flags;
  if (auto *bcProviderFromSrc =
          llvh::dyn_cast<hbc::BCProviderFromSrc>(bytecode.get())) {
    flags.persistent = bcProviderFromSrc->allowPersistent();
  } else {
    flags.persistent = true;
  }

  if (options.stopAfterInit) {
    vm::Handle<vm::Domain> domain =
        runtime->makeHandle(vm::Domain::create(*runtime));
    if (LLVM_UNLIKELY(
            vm::RuntimeModule::create(
                *runtime,
                domain,
                facebook::hermes::debugger::kInvalidLocation,
                std::move(bytecode),
                flags) == vm::ExecutionStatus::EXCEPTION)) {
      llvh::errs() << "Failed to initialize main RuntimeModule\n";
      return false;
    }

    return true;
  }

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  if (options.sampleProfiling != ExecuteOptions::SampleProfilingMode::None) {
    if (options.profilingOutFile.empty()) {
      llvh::errs()
          << "Please specify a profiling output file with -profiling-out\n";
      return false;
    }
    vm::SamplingProfiler::enable(options.sampleProfilingFreq);
  }
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

  llvh::StringRef sourceURL{};
  if (filename)
    sourceURL = *filename;
  vm::CallResult<vm::HermesValue> status = runtime->runBytecode(
      std::move(bytecode),
      flags,
      sourceURL,
      vm::Runtime::makeNullHandle<vm::Environment>());

  bool threwException = status == vm::ExecutionStatus::EXCEPTION;

  if (threwException) {
    // Make sure stdout catches up to stderr.
    llvh::outs().flush();
    runtime->printException(
        llvh::errs(), runtime->makeHandle(runtime->getThrownValue()));
  }

  // Perform a microtask checkpoint after running script.
  microtask::performCheckpoint(*runtime);

  if (!ctx.tasksEmpty()) {
    vm::GCScopeMarkerRAII marker{scope};
    // Run the tasks until there are no more.
    vm::MutableHandle<vm::Callable> task{*runtime};
    while (auto optTask = ctx.dequeueTask()) {
      task = std::move(*optTask);
      auto callRes = vm::Callable::executeCall0(
          task, *runtime, vm::Runtime::getUndefinedValue(), false);
      if (LLVM_UNLIKELY(callRes == vm::ExecutionStatus::EXCEPTION)) {
        threwException = true;
        llvh::outs().flush();
        runtime->printException(
            llvh::errs(), runtime->makeHandle(runtime->getThrownValue()));
        break;
      }

      // Perform a microtask checkpoint at the end of every task tick.
      microtask::performCheckpoint(*runtime);
    }
  }

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE
  if (options.sampleProfiling != ExecuteOptions::SampleProfilingMode::None) {
    assert(!options.profilingOutFile.empty() && "Must not be empty");
    OutputStream fileOS;
    if (!fileOS.open(options.profilingOutFile, llvh::sys::fs::F_Text))
      return false;
    switch (options.sampleProfiling) {
      case ExecuteOptions::SampleProfilingMode::None:
        llvm_unreachable("Cannot be none");
      case ExecuteOptions::SampleProfilingMode::Chrome:
        vm::SamplingProfiler::disable();
        runtime->samplingProfiler->dumpChromeTrace(fileOS.os());
        break;
      case ExecuteOptions::SampleProfilingMode::Tracery:
        vm::SamplingProfiler::disable();
        runtime->samplingProfiler->dumpTraceryTrace(fileOS.os());
        break;
    }
    if (!fileOS.close())
      return false;
  }
#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE

#ifdef HERMESVM_PROFILER_OPCODE
  runtime->dumpOpcodeStats(llvh::outs());
#endif

#ifdef HERMESVM_PROFILER_JSFUNCTION
  runtime->dumpJSFunctionStats();
#endif

#ifdef HERMESVM_PROFILER_NATIVECALL
  runtime->dumpNativeCallStats(llvh::outs());
#endif

  if (shouldRecordGCStats) {
    llvh::errs() << "Process stats:\n";
    statSampler->stop().printJSON(llvh::errs());

    if (options.forceGCBeforeStats) {
      runtime->collect("forced for stats");
    }
    printStats(*runtime, llvh::errs(), options.gcAnalyticsEvents);
  }

  if (options.runtimeConfig.getTrackIO()) {
    runtime->getIOTrackingInfoJSON(llvh::errs());
  }

  if (options.jitEmitCounters) {
    llvh::errs() << "JIT counters:\n";
    runtime->getJITContext().dumpCounters(llvh::errs());
  }

#ifdef HERMESVM_PROFILER_BB
  if (options.basicBlockProfiling) {
    OutputStream profilingFileOS(llvh::errs());
    if (!options.profilingOutFile.empty()) {
      if (!profilingFileOS.open(
              options.profilingOutFile, llvh::sys::fs::F_Text)) {
        llvh::errs() << "Failed to open file '" << options.profilingOutFile
                     << "'; writing to stderr instead.\n";
      }
    }
    runtime->getBasicBlockExecutionInfo().dump(profilingFileOS.os());
    if (!profilingFileOS.close()) {
      llvh::errs() << "Failed to close profiling file '"
                   << options.profilingOutFile << "'.\n";
    }
  }
#endif

  return !threwException;
}

} // namespace

/// Executes the HBC bytecode provided in HermesVM.
/// \return true on success, false on error.
bool executeHBCBytecode(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    const ExecuteOptions &options,
    const std::string *filename) {
  return maybeCatchException([&] {
    return executeHBCBytecodeImpl(std::move(bytecode), options, filename);
  });
}

} // namespace hermes
