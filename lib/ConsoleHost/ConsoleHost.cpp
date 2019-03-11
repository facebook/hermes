/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/ConsoleHost/ConsoleHost.h"

#include "hermes/Support/UTF8.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/instrumentation/PerfEvents.h"

namespace hermes {

/// Exits the program entirely.
static vm::CallResult<vm::HermesValue>
quit(void *, vm::Runtime *runtime, vm::NativeArgs) {
  std::exit(0);
}

static void printStats(vm::Runtime *runtime, llvm::raw_ostream &os) {
  std::string stats;
  llvm::raw_string_ostream tmp{stats};
  runtime->getHeap().printAllCollectedStats(tmp);
  tmp.flush();
  vm::instrumentation::PerfEvents::endAndInsertStats(stats);
  os << stats;
}

/// Exits the program entirely, and also prints out any accumulated statistics
/// by the GC.
///
/// \p context A pointer to a currently running instance of StatSamplingThread,
///     which the call to quit should stop and print the result of.  This
///     parameter can be null, if there is no sampling thread that needs to be
///     stopped.
static vm::CallResult<vm::HermesValue>
printStatsAndQuit(void *context, vm::Runtime *runtime, vm::NativeArgs) {
  if (context) {
    auto &statSampler = *reinterpret_cast<vm::StatSamplingThread *>(context);
    llvm::errs() << "Process stats:\n";
    statSampler.stop().printJSON(llvm::errs());
  }

  printStats(runtime, llvm::errs());
  std::exit(0);
}

static vm::CallResult<vm::HermesValue>
createHeapSnapshot(void *, vm::Runtime *runtime, vm::NativeArgs args) {
  using namespace vm;
  std::string fileName;
  if (args.getArgCount() >= 1) {
    auto res = toString(runtime, args.getArgHandle(runtime, 0));
    if (res == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto jsFileName = StringPrimitive::createStringView(
        runtime, toHandle(runtime, std::move(*res)));
    llvm::SmallVector<char16_t, 16> buf;
    convertUTF16ToUTF8WithReplacements(fileName, jsFileName.getUTF16Ref(buf));
  }
  if (fileName.empty()) {
    return runtime->raiseTypeError("Must give a non-empty file name");
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

void installConsoleBindings(
    vm::Runtime *runtime,
    bool gcPrintStats,
    vm::StatSamplingThread *statSampler) {
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
      runtime->getPredefinedSymbolID(vm::Predefined::quit),
      // Use the version which prints stats if that was requested.
      gcPrintStats ? printStatsAndQuit : quit,
      statSampler,
      0);
  defineGlobalFunc(
      runtime->getPredefinedSymbolID(vm::Predefined::createHeapSnapshot),
      createHeapSnapshot,
      nullptr,
      2);
}

/// Executes the HBC bytecode provided in HermesVM.
/// \return true on success, false on error.
bool executeHBCBytecode(
    std::shared_ptr<hbc::BCProvider> &&bytecode,
    const ExecuteOptions &options) {
  bool shouldRecordGCStats =
      options.runtimeConfig.getGCConfig().getShouldRecordStats();
  if (shouldRecordGCStats) {
    vm::instrumentation::PerfEvents::begin();
  }
  std::unique_ptr<vm::StatSamplingThread> statSampler;
  auto runtime = vm::Runtime::create(options.runtimeConfig);
  runtime->getJITContext().setDumpJITCode(options.dumpJITCode);
  runtime->getJITContext().setCrashOnError(options.jitCrashOnError);

  if (shouldRecordGCStats) {
    statSampler = llvm::make_unique<vm::StatSamplingThread>(
        std::chrono::milliseconds(100));
  }

#ifdef HERMESVM_SANITIZE_HANDLES
  {
    double sanitizeRate = options.runtimeConfig.getGCConfig().getSanitizeRate();
    if (sanitizeRate > 0.0 && sanitizeRate < 1.0) {
      llvm::errs()
          << "Warning: you are using handle sanitation with random sampling.\n"
          << "Re-run with -gc-sanitize-handles=1 for deterministic crashes.\n";
    }
  }
#endif

  vm::GCScope scope(runtime.get());
  installConsoleBindings(runtime.get(), shouldRecordGCStats, statSampler.get());

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
    profiler->dumpSampledStack(llvm::errs());
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
