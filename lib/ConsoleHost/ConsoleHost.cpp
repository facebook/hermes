/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/ConsoleHost/ConsoleHost.h"

#include "hermes/CompilerDriver/CompilerDriver.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/MockedEnvironment.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/TimeLimitMonitor.h"
#include "hermes/VM/instrumentation/PerfEvents.h"

namespace hermes {

ConsoleHostContext::ConsoleHostContext(vm::Runtime *runtime) {
  runtime->addCustomRootsFunction([this](vm::GC *, vm::RootAcceptor &acceptor) {
    for (auto &entry : taskQueue_) {
      acceptor.acceptPtr(entry.second);
    }
  });
}

/// Raises an uncatchable quit exception.
static vm::CallResult<vm::HermesValue>
quit(void *, vm::Runtime *runtime, vm::NativeArgs) {
  return runtime->raiseQuitError();
}

static void printStats(vm::Runtime *runtime, llvh::raw_ostream &os) {
  std::string stats;
  {
    llvh::raw_string_ostream tmp{stats};
    runtime->printHeapStats(tmp);
  }
  vm::instrumentation::PerfEvents::endAndInsertStats(stats);
  os << stats;
}

static vm::CallResult<vm::HermesValue>
createHeapSnapshot(void *, vm::Runtime *runtime, vm::NativeArgs args) {
  using namespace vm;
  std::string fileName;
  if (args.getArgCount() >= 1 && !args.getArg(0).isUndefined()) {
    if (!args.getArg(0).isString()) {
      return runtime->raiseTypeError("Filename argument must be a string");
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
    return runtime->raiseTypeError(
        "Filename must end in .heapsnapshot or .heaptimeline");
  }
  if (auto err = runtime->getHeap().createSnapshotToFile(fileName)) {
    // This isn't a TypeError, but no other built-in can express file errors,
    // so this will have to do.
    return runtime->raiseTypeError(
        TwineChar16("Could not write out to the file located at \"") +
        llvh::StringRef(fileName) +
        "\". System error: " + llvh::StringRef(err.message()));
  }
  return HermesValue::encodeUndefinedValue();
}

static vm::CallResult<vm::HermesValue>
loadSegment(void *ctx, vm::Runtime *runtime, vm::NativeArgs args) {
  using namespace hermes::vm;
  const auto *baseFilename = reinterpret_cast<std::string *>(ctx);

  auto requireContext = args.dyncastArg<RequireContext>(0);
  if (!requireContext) {
    return runtime->raiseTypeError(
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
    return runtime->raiseTypeError(
        TwineChar16("Failed to open segment: ") + segment);
  }

  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      std::make_unique<OwnedMemoryBuffer>(std::move(*fileBufRes)));
  if (!ret.first) {
    return runtime->raiseTypeError("Error deserializing bytecode");
  }

  if (LLVM_UNLIKELY(
          runtime->loadSegment(std::move(ret.first), requireContext) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeUndefinedValue();
}

static vm::CallResult<vm::HermesValue>
setTimeout(void *ctx, vm::Runtime *runtime, vm::NativeArgs args) {
  ConsoleHostContext *consoleHost = (ConsoleHostContext *)ctx;
  using namespace hermes::vm;
  Handle<Callable> callable = args.dyncastArg<Callable>(0);
  if (!callable) {
    return runtime->raiseTypeError("Argument to setTimeout must be a function");
  }
  CallResult<HermesValue> boundFunction = BoundFunction::create(
      runtime, callable, args.getArgCount() - 1, args.begin() + 1);
  if (boundFunction == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  uint32_t taskId = consoleHost->queueTask(
      PseudoHandle<Callable>::vmcast(createPseudoHandle(*boundFunction)));
  return HermesValue::encodeNumberValue(taskId);
}

static vm::CallResult<vm::HermesValue>
clearTimeout(void *ctx, vm::Runtime *runtime, vm::NativeArgs args) {
  ConsoleHostContext *consoleHost = (ConsoleHostContext *)ctx;
  using namespace hermes::vm;
  if (!args.getArg(0).isNumber()) {
    return runtime->raiseTypeError("Argument to clearTimeout must be a number");
  }
  consoleHost->clearTask(args.getArg(0).getNumberAs<uint32_t>());
  return HermesValue::encodeUndefinedValue();
}

#ifdef HERMESVM_SERIALIZE
static std::vector<void *> getNativeFunctionPtrs();

/// serializeVM(funciton() {/*resumed*/}, [filename]) will serialize the VM
/// state to a file. When deserialize from the file, we will continue to execute
/// the closure funciton provided. Serialize filename is specified by
/// -serializevm-path when provided, otherwise we will use the second argument.
static vm::CallResult<vm::HermesValue>
serializeVM(void *ctx, vm::Runtime *runtime, vm::NativeArgs args) {
  using namespace vm;

  if (!args.getArg(0).isObject()) {
    return runtime->raiseTypeError("Invalid/Missing function argument");
  }

  std::unique_ptr<llvh::raw_ostream> serializeStream = nullptr;
  if (ctx) {
    const auto *fileName = reinterpret_cast<std::string *>(ctx);
    std::error_code EC;
    serializeStream =
        std::make_unique<llvh::raw_fd_ostream>(llvh::StringRef(*fileName), EC);
    if (EC) {
      return runtime->raiseTypeError(
          TwineChar16("Could not write to file located at ") +
          llvh::StringRef(*fileName));
    }
  } else {
    // See if filename is provided as an argument.
    if (!args.getArg(1).isString()) {
      return runtime->raiseTypeError(
          "Missing filename argument or filename argument not a string");
    }
    std::string fileName;

    // In the rare events where we have a UTF16 string, convert it to ASCII.
    auto str = Handle<StringPrimitive>::vmcast(args.getArgHandle(1));
    auto jsFileName = StringPrimitive::createStringView(runtime, str);
    llvh::SmallVector<char16_t, 16> buf;
    convertUTF16ToUTF8WithReplacements(fileName, jsFileName.getUTF16Ref(buf));

    if (fileName.empty()) {
      return runtime->raiseTypeError("Filename must not be empty");
    }
    std::error_code EC;
    serializeStream =
        std::make_unique<llvh::raw_fd_ostream>(llvh::StringRef(fileName), EC);
    if (EC) {
      return runtime->raiseTypeError(
          TwineChar16("Could not write to file located at ") +
          llvh::StringRef(fileName));
    }
  }

  auto closureFunction = Handle<JSFunction>::vmcast(args.getArgHandle(0));

  Serializer s(*serializeStream, runtime, getNativeFunctionPtrs);
  runtime->setSerializeClosure(closureFunction);
  runtime->serialize(s);
  return HermesValue::encodeUndefinedValue();
}

/// Gather function pointers of native functions and put them in \p vec.
static std::vector<void *> getNativeFunctionPtrs() {
  std::vector<void *> res;
  res.push_back((void *)quit);
  res.push_back((void *)createHeapSnapshot);
  res.push_back((void *)serializeVM);
  res.push_back((void *)loadSegment);
  res.push_back((void *)setTimeout);
  res.push_back((void *)clearTimeout);
  return res;
}
#endif

void installConsoleBindings(
    vm::Runtime *runtime,
    ConsoleHostContext &ctx,
    vm::StatSamplingThread *statSampler,
#ifdef HERMESVM_SERIALIZE
    const std::string *serializePath,
#endif
    const std::string *filename) {
  vm::DefinePropertyFlags normalDPF =
      vm::DefinePropertyFlags::getNewNonEnumerableFlags();

#if defined HERMESVM_SERIALIZE && !defined NDEBUG
  // Verify that all native pointers can be captured by getNativeFunctionPtrs.
  std::vector<void *> pointers = getNativeFunctionPtrs();
#endif

  auto defineGlobalFunc = [&](vm::SymbolID name,
                              vm::NativeFunctionPtr functionPtr,
                              void *context,
                              unsigned paramCount) -> void {
    vm::GCScopeMarkerRAII marker{runtime};
#ifdef HERMESVM_SERIALIZE
    assert(
        (std::find(pointers.begin(), pointers.end(), (void *)functionPtr) !=
         pointers.end()) &&
        "All function pointers must be added in getNativeFunctionPtrs");
#endif
    auto func = vm::NativeFunction::createWithoutPrototype(
        runtime, context, functionPtr, name, paramCount);
    auto res = vm::JSObject::defineOwnProperty(
        runtime->getGlobal(), runtime, name, normalDPF, func);
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
#ifdef HERMESVM_SERIALIZE
  defineGlobalFunc(
      runtime
          ->ignoreAllocationFailure(
              runtime->getIdentifierTable().getSymbolHandle(
                  runtime, llvh::createASCIIRef("serializeVM")))
          .get(),
      serializeVM,
      reinterpret_cast<void *>(const_cast<std::string *>(serializePath)),
      1);
#endif

  // Define the 'loadSegment' function.
  defineGlobalFunc(
      runtime
          ->ignoreAllocationFailure(
              runtime->getIdentifierTable().getSymbolHandle(
                  runtime, llvh::createASCIIRef("loadSegment")))
          .get(),
      loadSegment,
      reinterpret_cast<void *>(const_cast<std::string *>(filename)),
      2);

  defineGlobalFunc(
      runtime
          ->ignoreAllocationFailure(
              runtime->getIdentifierTable().getSymbolHandle(
                  runtime, llvh::createASCIIRef("setTimeout")))
          .get(),
      setTimeout,
      &ctx,
      2);
  defineGlobalFunc(
      runtime
          ->ignoreAllocationFailure(
              runtime->getIdentifierTable().getSymbolHandle(
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
          ->ignoreAllocationFailure(
              runtime->getIdentifierTable().getSymbolHandle(
                  runtime, llvh::createASCIIRef("setImmediate")))
          .get(),
      setTimeout,
      &ctx,
      1);
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

#ifdef HERMESVM_SERIALIZE
  // Handle Serialization/Deserialization options
  std::shared_ptr<llvh::raw_ostream> serializeFile = nullptr;
  std::shared_ptr<llvh::MemoryBuffer> deserializeFile = nullptr;
  if (!options.SerializeAfterInitFile.empty()) {
    if (!options.DeserializeFile.empty()) {
      llvh::errs()
          << "Cannot serialize and deserialize in the same execution\n";
      return false;
    }
    std::error_code EC;
    serializeFile = std::make_shared<llvh::raw_fd_ostream>(
        llvh::StringRef(options.SerializeAfterInitFile), EC);
    if (EC) {
      llvh::errs() << "Failed to read Serialize file: "
                   << options.SerializeAfterInitFile << "\n";
      return false;
    }
  }

  if (options.DeserializeFile != "") {
    auto inputFileOrErr = llvh::MemoryBuffer::getFile(options.DeserializeFile);
    if (!inputFileOrErr) {
      llvh::errs() << "Failed to read Deserialize file: "
                   << options.DeserializeFile << '\n';
      return false;
    }
    deserializeFile = std::move(*inputFileOrErr);
  }
#endif

  std::unique_ptr<vm::StatSamplingThread> statSampler;
#ifdef HERMESVM_SERIALIZE
  auto runtime = vm::Runtime::create(
      options.runtimeConfig.rebuild()
          .withSerializeAfterInitFile(serializeFile)
          .withDeserializeFile(deserializeFile)
          .withExternalPointersVectorCallBack(getNativeFunctionPtrs)
          .build());
#else
  auto runtime = vm::Runtime::create(options.runtimeConfig);
#endif
  if (options.stabilizeInstructionCount) {
    // Try to limit features that can introduce unpredictable CPU instruction
    // behavior. Date is a potential cause, but is not handled currently.
    vm::MockedEnvironment env;
    env.mathRandomSeed = 0;
    env.stabilizeInstructionCount = true;
    runtime->setMockedEnvironment(env);
  }

  if (options.timeLimit > 0) {
    vm::TimeLimitMonitor::getInstance().watchRuntime(
        runtime.get(), options.timeLimit);
  }

  if (shouldRecordGCStats) {
    statSampler = std::make_unique<vm::StatSamplingThread>(
        std::chrono::milliseconds(100));
  }

  if (options.heapTimeline) {
    runtime->enableAllocationLocationTracker();
  }

  vm::GCScope scope(runtime.get());
  ConsoleHostContext ctx{runtime.get()};

  installConsoleBindings(
      runtime.get(),
      ctx,
      statSampler.get(),
#ifdef HERMESVM_SERIALIZE
      options.SerializeVMPath.empty() ? nullptr : &options.SerializeVMPath,
#endif
      filename);

  vm::RuntimeModuleFlags flags;
  flags.persistent = true;

  if (options.stopAfterInit) {
    vm::Handle<vm::Domain> domain =
        runtime->makeHandle(vm::Domain::create(runtime.get()));
    if (LLVM_UNLIKELY(
            vm::RuntimeModule::create(
                runtime.get(),
                domain,
                facebook::hermes::debugger::kInvalidLocation,
                std::move(bytecode),
                flags) == vm::ExecutionStatus::EXCEPTION)) {
      llvh::errs() << "Failed to initialize main RuntimeModule\n";
      return false;
    }

    return true;
  }

  if (options.sampleProfiling) {
    vm::SamplingProfiler::enable();
  }

  llvh::StringRef sourceURL{};
  if (filename)
    sourceURL = *filename;
  vm::CallResult<vm::HermesValue> status = runtime->runBytecode(
      std::move(bytecode),
      flags,
      sourceURL,
      vm::Runtime::makeNullHandle<vm::Environment>());

  if (options.sampleProfiling) {
    vm::SamplingProfiler::disable();
    vm::SamplingProfiler::dumpChromeTraceGlobal(llvh::errs());
  }

  bool threwException = status == vm::ExecutionStatus::EXCEPTION;

  if (threwException) {
    // Make sure stdout catches up to stderr.
    llvh::outs().flush();
    runtime->printException(
        llvh::errs(), runtime->makeHandle(runtime->getThrownValue()));
  }

  // Perform a microtask checkpoint after running script.
  microtask::performCheckpoint(runtime.get());

  if (!ctx.tasksEmpty()) {
    vm::GCScopeMarkerRAII marker{scope};
    // Run the tasks until there are no more.
    vm::MutableHandle<vm::Callable> task{runtime.get()};
    while (auto optTask = ctx.dequeueTask()) {
      task = std::move(*optTask);
      auto callRes = vm::Callable::executeCall0(
          task, runtime.get(), vm::Runtime::getUndefinedValue(), false);
      if (LLVM_UNLIKELY(callRes == vm::ExecutionStatus::EXCEPTION)) {
        threwException = true;
        llvh::outs().flush();
        runtime->printException(
            llvh::errs(), runtime->makeHandle(runtime->getThrownValue()));
        break;
      }

      // Perform a microtask checkpoint at the end of every task tick.
      microtask::performCheckpoint(runtime.get());
    }
  }

  if (options.timeLimit > 0) {
    vm::TimeLimitMonitor::getInstance().unwatchRuntime(runtime.get());
  }

#ifdef HERMESVM_PROFILER_OPCODE
  runtime->dumpOpcodeStats(llvh::outs());
#endif

#ifdef HERMESVM_PROFILER_JSFUNCTION
  runtime->dumpJSFunctionStats();
#endif

#ifdef HERMESVM_PROFILER_EXTERN
  if (options.patchProfilerSymbols) {
    patchProfilerSymbols(runtime.get());
  } else {
    dumpProfilerSymbolMap(runtime.get(), options.profilerSymbolsFile);
  }
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
    printStats(runtime.get(), llvh::errs());
  }

#ifdef HERMESVM_PROFILER_BB
  if (options.basicBlockProfiling) {
    runtime->getBasicBlockExecutionInfo().dump(llvh::errs());
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
