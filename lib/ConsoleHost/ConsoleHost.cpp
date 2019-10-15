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
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/TimeLimitMonitor.h"
#include "hermes/VM/instrumentation/PerfEvents.h"

namespace hermes {

/// Raises an uncatchable quit exception.
static vm::CallResult<vm::HermesValue>
quit(void *, vm::Runtime *runtime, vm::NativeArgs) {
  return runtime->raiseQuitError();
}

static void printStats(vm::Runtime *runtime, llvm::raw_ostream &os) {
  std::string stats;
  {
    llvm::raw_string_ostream tmp{stats};
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
    llvm::SmallVector<char16_t, 16> buf;
    convertUTF16ToUTF8WithReplacements(fileName, jsFileName.getUTF16Ref(buf));
  }

  if (fileName.empty()) {
    // "-" is recognized as stdout.
    fileName = "-";
  } else if (!llvm::StringRef{fileName}.endswith(".heapsnapshot")) {
    return runtime->raiseTypeError("Filename must end in .heapsnapshot");
  }
  bool compact = args.getArgCount() >= 2 ? toBoolean(args.getArg(1)) : true;
  if (!runtime->getHeap().createSnapshotToFile(fileName, compact)) {
    // This isn't a TypeError, but no other built-in can express file errors,
    // so this will have to do.
    return runtime->raiseTypeError(
        TwineChar16("Could not write out to the file located at ") +
        llvm::StringRef(fileName));
  }
  return HermesValue::encodeUndefinedValue();
}

static vm::CallResult<vm::HermesValue>
loadSegment(void *ctx, vm::Runtime *runtime, vm::NativeArgs args) {
  using namespace hermes::vm;
  const auto *baseFilename = reinterpret_cast<std::string *>(ctx);

  auto requireContext = args.dyncastArg<RequireContext>(0);
  if (!requireContext) {
    runtime->raiseTypeError("First argument to loadSegment must be context");
  }

  auto segmentRes = toUInt32_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(segmentRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t segment = segmentRes->getNumberAs<uint32_t>();

  auto fileBufRes =
      llvm::MemoryBuffer::getFile(Twine(*baseFilename) + "." + Twine(segment));
  if (!fileBufRes) {
    return runtime->raiseTypeError(
        TwineChar16("Failed to open segment: ") + segment);
  }

  auto ret = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
      llvm::make_unique<OwnedMemoryBuffer>(std::move(*fileBufRes)));
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

  std::unique_ptr<llvm::raw_ostream> serializeStream = nullptr;
  if (ctx) {
    const auto *fileName = reinterpret_cast<std::string *>(ctx);
    std::error_code EC;
    serializeStream =
        std::make_unique<llvm::raw_fd_ostream>(llvm::StringRef(*fileName), EC);
    if (EC) {
      return runtime->raiseTypeError(
          TwineChar16("Could not write to file located at ") +
          llvm::StringRef(*fileName));
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
    llvm::SmallVector<char16_t, 16> buf;
    convertUTF16ToUTF8WithReplacements(fileName, jsFileName.getUTF16Ref(buf));

    if (fileName.empty()) {
      return runtime->raiseTypeError("Filename must not be empty");
    }
    std::error_code EC;
    serializeStream =
        std::make_unique<llvm::raw_fd_ostream>(llvm::StringRef(fileName), EC);
    if (EC) {
      return runtime->raiseTypeError(
          TwineChar16("Could not write to file located at ") +
          llvm::StringRef(fileName));
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
  return res;
}
#endif

void installConsoleBindings(
    vm::Runtime *runtime,
    vm::StatSamplingThread *statSampler,
#ifdef HERMESVM_SERIALIZE
    const std::string *serializePath,
#endif
    const std::string *filename) {
  vm::DefinePropertyFlags normalDPF{};
  normalDPF.setEnumerable = 1;
  normalDPF.setWritable = 1;
  normalDPF.setConfigurable = 1;
  normalDPF.setValue = 1;
  normalDPF.enumerable = 0;
  normalDPF.writable = 1;
  normalDPF.configurable = 1;

#if defined HERMESVM_SERIALIZE && !defined NDEBUG
  // Verify that all native pointers can be captured by getNativeFunctionPtrs.
  std::vector<void *> pointers = getNativeFunctionPtrs();
#endif

  auto defineGlobalFunc = [&](vm::SymbolID name,
                              vm::NativeFunctionPtr functionPtr,
                              void *context,
                              unsigned paramCount) {
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
      2);
#ifdef HERMESVM_SERIALIZE
  defineGlobalFunc(
      runtime
          ->ignoreAllocationFailure(
              runtime->getIdentifierTable().getSymbolHandle(
                  runtime, llvm::createASCIIRef("serializeVM")))
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
                  runtime, llvm::createASCIIRef("loadSegment")))
          .get(),
      loadSegment,
      reinterpret_cast<void *>(const_cast<std::string *>(filename)),
      2);
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
    llvm::errs() << ex.what();
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
  std::shared_ptr<llvm::raw_ostream> serializeFile = nullptr;
  std::shared_ptr<llvm::MemoryBuffer> deserializeFile = nullptr;
  if (!options.SerializeAfterInitFile.empty()) {
    if (!options.DeserializeFile.empty()) {
      llvm::errs()
          << "Cannot serialize and deserialize in the same execution\n";
      return false;
    }
    std::error_code EC;
    serializeFile = std::make_shared<llvm::raw_fd_ostream>(
        llvm::StringRef(options.SerializeAfterInitFile), EC);
    if (EC) {
      llvm::errs() << "Failed to read Serialize file: "
                   << options.SerializeAfterInitFile << "\n";
      return false;
    }
  }

  if (options.DeserializeFile != "") {
    auto inputFileOrErr = llvm::MemoryBuffer::getFile(options.DeserializeFile);
    if (!inputFileOrErr) {
      llvm::errs() << "Failed to read Deserialize file: "
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
  runtime->getJITContext().setDumpJITCode(options.dumpJITCode);
  runtime->getJITContext().setCrashOnError(options.jitCrashOnError);
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
    statSampler = llvm::make_unique<vm::StatSamplingThread>(
        std::chrono::milliseconds(100));
  }

  vm::GCScope scope(runtime.get());
  installConsoleBindings(
      runtime.get(),
      statSampler.get(),
#ifdef HERMESVM_SERIALIZE
      options.SerializeVMPath.empty() ? nullptr : &options.SerializeVMPath,
#endif
      filename);

  vm::RuntimeModuleFlags flags;
  flags.persistent = true;

  if (options.stopAfterInit) {
    vm::Handle<vm::Domain> domain =
        vm::toHandle(runtime.get(), vm::Domain::create(runtime.get()));
    if (LLVM_UNLIKELY(
            vm::RuntimeModule::create(
                runtime.get(), domain, std::move(bytecode), flags) ==
            vm::ExecutionStatus::EXCEPTION)) {
      llvm::errs() << "Failed to initialize main RuntimeModule\n";
      return false;
    }

    return true;
  }

  if (options.runtimeConfig.getEnableSampleProfiling()) {
    vm::SamplingProfiler::getInstance()->enable();
  }

  llvm::StringRef sourceURL{};
  vm::CallResult<vm::HermesValue> status = runtime->runBytecode(
      std::move(bytecode),
      flags,
      sourceURL,
      vm::Runtime::makeNullHandle<vm::Environment>());

  if (options.runtimeConfig.getEnableSampleProfiling()) {
    auto profiler = vm::SamplingProfiler::getInstance();
    profiler->dumpChromeTrace(llvm::errs());
    profiler->disable();
  }

  bool threwException = status == vm::ExecutionStatus::EXCEPTION;

  if (threwException) {
    // Make sure stdout catches up to stderr.
    llvm::outs().flush();
    runtime->printException(
        llvm::errs(), runtime->makeHandle(runtime->getThrownValue()));
  }

  if (options.timeLimit > 0) {
    vm::TimeLimitMonitor::getInstance().unwatchRuntime(runtime.get());
  }

#ifdef HERMESVM_PROFILER_OPCODE
  runtime->dumpOpcodeStats(llvm::outs());
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
  runtime->dumpNativeCallStats(llvm::outs());
#endif

  if (shouldRecordGCStats) {
    llvm::errs() << "Process stats:\n";
    statSampler->stop().printJSON(llvm::errs());

    if (options.forceGCBeforeStats) {
      runtime->collect();
    }
    printStats(runtime.get(), llvm::errs());
  }

#ifdef HERMESVM_PROFILER_BB
  if (options.basicBlockProfiling) {
    runtime->getBasicBlockExecutionInfo().dump(llvm::errs());
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
