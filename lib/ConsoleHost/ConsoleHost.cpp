/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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
    auto str = Handle<StringPrimitive>::vmcast(args.getArgHandle(runtime, 0));
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

  auto requireContext = args.dyncastArg<RequireContext>(runtime, 0);
  if (!requireContext) {
    runtime->raiseTypeError("First argument to loadSegment must be context");
  }

  auto segmentRes = toUInt32_RJS(runtime, args.getArgHandle(runtime, 1));
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

void installConsoleBindings(
    vm::Runtime *runtime,
    vm::StatSamplingThread *statSampler,
    const std::string *filename) {
  vm::DefinePropertyFlags normalDPF{};
  normalDPF.setEnumerable = 1;
  normalDPF.setWritable = 1;
  normalDPF.setConfigurable = 1;
  normalDPF.setValue = 1;
  normalDPF.enumerable = 0;
  normalDPF.writable = 1;
  normalDPF.configurable = 1;

  auto defineGlobalFunc = [&](vm::SymbolID name,
                              vm::NativeFunctionPtr functionPtr,
                              void *context,
                              unsigned paramCount) {
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

/// Executes the HBC bytecode provided in HermesVM.
/// \return true on success, false on error.
bool executeHBCBytecode(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    const ExecuteOptions &options,
    const std::string *filename) {
  bool shouldRecordGCStats =
      options.runtimeConfig.getGCConfig().getShouldRecordStats();
  if (shouldRecordGCStats) {
    vm::instrumentation::PerfEvents::begin();
  }
  std::unique_ptr<vm::StatSamplingThread> statSampler;
  auto runtime = vm::Runtime::create(options.runtimeConfig);
  runtime->getJITContext().setDumpJITCode(options.dumpJITCode);
  runtime->getJITContext().setCrashOnError(options.jitCrashOnError);

#ifdef HERMESVM_TIMELIMIT
  if (options.timeLimit > 0) {
    vm::TimeLimitMonitor::getInstance().watchRuntime(
        runtime.get(), options.timeLimit);
  }
#endif

  if (shouldRecordGCStats) {
    statSampler = llvm::make_unique<vm::StatSamplingThread>(
        std::chrono::milliseconds(100));
  }

  vm::GCScope scope(runtime.get());
  installConsoleBindings(runtime.get(), statSampler.get(), filename);

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
  auto status = runtime->runBytecode(
      std::move(bytecode),
      flags,
      sourceURL,
      runtime->makeNullHandle<vm::Environment>());

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

#ifdef HERMESVM_TIMELIMIT
  if (options.timeLimit > 0) {
    vm::TimeLimitMonitor::getInstance().unwatchRuntime(runtime.get());
  }
#endif

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

    printStats(runtime.get(), llvm::errs());
  }

#ifdef HERMESVM_PROFILER_BB
  if (options.basicBlockProfiling) {
    runtime->getBasicBlockExecutionInfo().dump(llvm::errs());
  }
#endif

  return !threwException;
}

} // namespace hermes
