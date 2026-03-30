/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Executor.h"

#include "hermes/BCGen/HBC/BCProviderFromSrc.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/ConsoleHost/ConsoleHost.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/TimeLimitMonitor.h"
#include "hermes/hermes.h"

#include "jsi/jsi.h"

#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

#include <thread>

namespace hermes {
namespace testrunner {

namespace {

using Clock = std::chrono::steady_clock;

/// Bundle of runtime objects needed for test execution.
struct TestRuntimeEnv {
  std::unique_ptr<facebook::jsi::Runtime> hermesRuntime;
  vm::Runtime *runtime;
  std::shared_ptr<vm::TimeLimitMonitor> timeLimitMonitor;
};

/// Capture the exception message from the runtime and clear the thrown value.
std::string captureException(vm::Runtime &runtime) {
  auto thrownVal = runtime.makeHandle(runtime.getThrownValue());
  std::string buf;
  llvh::raw_string_ostream sos(buf);
  runtime.printException(sos, thrownVal);
  runtime.clearThrownValue();
  return sos.str();
}

/// Create a fresh Hermes runtime configured for test262 execution.
TestRuntimeEnv createTestRuntime(unsigned timeoutSeconds) {
  auto runtimeConfig = vm::RuntimeConfig::Builder()
                           .withES6Proxy(true)
                           .withMicrotaskQueue(true)
                           .withEnableHermesInternal(true)
                           .withEnableHermesInternalTestMethods(true)
                           .withEnableAsyncGenerators(true)
                           .withTest262(true)
                           .withEnableEval(true)
                           .withAsyncBreakCheckInEval(true)
                           .build();

  auto hermesRuntime = facebook::hermes::makeHermesRuntime(runtimeConfig);
  auto *runtime = static_cast<vm::Runtime *>(
      facebook::jsi::castInterface<facebook::hermes::IHermes>(
          hermesRuntime.get())
          ->getVMRuntimeUnsafe());

  std::shared_ptr<vm::TimeLimitMonitor> timeLimitMonitor;
  if (timeoutSeconds > 0) {
    timeLimitMonitor = vm::TimeLimitMonitor::getOrCreate();
    runtime->timeLimitMonitor = timeLimitMonitor;
    timeLimitMonitor->watchRuntime(
        *runtime, std::chrono::milliseconds(timeoutSeconds * 1000u));
  }

  return {std::move(hermesRuntime), runtime, timeLimitMonitor};
}

/// Drain the setTimeout task queue, executing each callback and draining
/// microtasks after each one. Returns true if an exception was thrown,
/// and sets exceptionMsg.
bool drainTaskQueue(
    vm::Runtime &runtime,
    vm::GCScope &scope,
    ConsoleHostContext &ctx,
    std::string &exceptionMsg) {
  vm::GCScopeMarkerRAII marker{scope};
  vm::MutableHandle<vm::Callable> task{runtime};
  while (auto optTask = ctx.dequeueTask()) {
    task = std::move(*optTask);
    auto callRes = vm::Callable::executeCall0(
        task, runtime, vm::Runtime::getUndefinedValue(), false);
    if (callRes == vm::ExecutionStatus::EXCEPTION) {
      exceptionMsg = captureException(runtime);
      return true;
    }
    microtask::performCheckpoint(runtime);
  }
  return false;
}

/// Execute already-compiled bytecode in a fresh runtime, handling timeouts,
/// microtasks, setTimeout callbacks, and negative expectations.
TestResult executeCompiledTest(
    const std::string &testName,
    std::unique_ptr<hbc::BCProvider> bytecode,
    const std::string &sourceURL,
    bool expectRuntimeError,
    const NegativeExpectation &negative,
    unsigned timeoutSeconds,
    Clock::time_point startTime) {
  auto makeResult = [&](ResultCode code, const std::string &msg) {
    auto endTime = Clock::now();
    TestResult r;
    r.testName = testName;
    r.code = code;
    r.message = msg;
    r.duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime);
    return r;
  };

  TestRuntimeEnv env = createTestRuntime(timeoutSeconds);
  auto *runtime = env.runtime;

  // Install console bindings (including $262, alert, setTimeout, etc.).
  vm::GCScope scope(*runtime);
  ConsoleHostContext ctx{*runtime};
  ctx.enableTestMethods_ = true;
  installConsoleBindings(*runtime, ctx);

  // Run the bytecode.
  vm::RuntimeModuleFlags rmFlags;
  rmFlags.persistent = true;

  vm::CallResult<vm::HermesValue> status = runtime->runBytecode(
      std::move(bytecode),
      rmFlags,
      sourceURL,
      vm::Runtime::makeNullHandle<vm::Environment>());

  bool threwException = status == vm::ExecutionStatus::EXCEPTION;
  std::string exceptionMsg;

  if (threwException) {
    exceptionMsg = captureException(*runtime);
  }

  // Check for timeout.
  if (threwException &&
      exceptionMsg.find("Javascript execution has timed out") !=
          std::string::npos) {
    if (env.timeLimitMonitor) {
      env.timeLimitMonitor->unwatchRuntime(*runtime);
    }
    return makeResult(ResultCode::ExecuteTimeout, "FAIL: Test timed out");
  }

  // Drain microtask queue and task queue while still under timeout protection.
  if (!threwException) {
    microtask::performCheckpoint(*runtime);
    if (drainTaskQueue(*runtime, scope, ctx, exceptionMsg)) {
      threwException = true;
    }
  }

  // Unwatch runtime from time limit monitor after all execution is complete.
  if (env.timeLimitMonitor) {
    env.timeLimitMonitor->unwatchRuntime(*runtime);
  }

  // Check for timeout during microtask/task draining.
  if (threwException &&
      exceptionMsg.find("Javascript execution has timed out") !=
          std::string::npos) {
    return makeResult(ResultCode::ExecuteTimeout, "FAIL: Test timed out");
  }

  // Evaluate result based on expectations.
  if (expectRuntimeError) {
    if (!threwException) {
      return makeResult(
          ResultCode::ExecuteFailed,
          "FAIL: Expected runtime " + negative.errorType + " but test passed");
    }
    if (!negative.errorType.empty() &&
        exceptionMsg.find(negative.errorType) == std::string::npos) {
      return makeResult(
          ResultCode::ExecuteFailed,
          "FAIL: Expected " + negative.errorType + " but got: " + exceptionMsg);
    }
    return makeResult(ResultCode::Passed, "PASS (expected runtime error)");
  }

  if (threwException) {
    return makeResult(ResultCode::ExecuteFailed, "FAIL: " + exceptionMsg);
  }

  return makeResult(ResultCode::Passed, "PASS");
}

/// Print progress at regular intervals.
void reportProgress(size_t done, size_t total) {
  if (done % 100 == 0 || done == total) {
    llvh::outs() << "\r" << done << "/" << total << " tests complete";
    llvh::outs().flush();
  }
}

/// Process a single test entry: read file, parse frontmatter, determine
/// variants, build source, and execute each variant.
void processTestEntry(
    const TestEntry &entry,
    const HarnessCache &harness,
    const Skiplist *skiplist,
    const ExecConfig &config,
    std::vector<TestResult> &results,
    std::mutex &resultsMutex,
    std::atomic<size_t> &completedCount,
    size_t totalTests,
    std::atomic<size_t> &featureSkipped,
    std::atomic<size_t> &permanentFeatureSkipped) {
  // Read test file.
  auto fileBuf = llvh::MemoryBuffer::getFile(entry.path);
  if (!fileBuf) {
    TestResult r;
    r.testName = entry.fullName;
    r.code = ResultCode::Failed;
    r.message = "FAIL: Cannot read file";
    std::lock_guard<std::mutex> lock(resultsMutex);
    results.push_back(std::move(r));
    ++completedCount;
    return;
  }

  llvh::StringRef content = (*fileBuf)->getBuffer();
  TestRecord record = parseFrontmatter(content);

  // Feature-based skipping.
  if (skiplist) {
    for (const auto &feat : record.features) {
      SkipReason reason = skiplist->shouldSkipFeature(feat);
      if (reason != SkipReason::NotSkipped) {
        ++featureSkipped;
        if (reason == SkipReason::PermanentUnsupportedFeature)
          ++permanentFeatureSkipped;
        ++completedCount;
        return;
      }
    }
  }

  // Skip tests that include testIntl.js — Hermes doesn't implement all
  // Intl constructors required by this harness (matching Python runner).
  for (const auto &inc : record.includes) {
    if (inc == "testIntl.js") {
      ++featureSkipped;
      ++completedCount;
      return;
    }
  }

  // Determine variants.
  bool runStrict = !record.isNoStrict() && !record.isRaw();
  bool runNonStrict = !record.isOnlyStrict() && !record.isRaw();
  bool runRaw = record.isRaw();

  if (record.isModule()) {
    runStrict = false;
    runNonStrict = true;
    runRaw = false;
  }

  // Build includes list.
  std::vector<std::string> includes;
  if (entry.suiteKind == SuiteKind::Test262) {
    includes.push_back("sta.js");
    includes.push_back("assert.js");
  }
  for (const auto &inc : record.includes) {
    includes.push_back(inc);
  }
  if (record.isAsync()) {
    includes.push_back("doneprintHandle.js");
  }

  // Run variants in order, short-circuiting on first failure (like the Python
  // runner). Push exactly one result per test file.
  auto runVariant = [&](const char *suffix, bool isStrict) -> TestResult {
    std::string source = harness.buildSource(includes, record.src, isStrict);
    std::string variantName = entry.fullName + " (" + suffix + ")";

    return executeTestVariant(
        variantName,
        source,
        entry.path,
        isStrict,
        record.negative,
        config.timeoutSeconds);
  };

  TestResult lastResult;
  if (runRaw) {
    lastResult = runVariant("raw", false);
  } else {
    if (runNonStrict) {
      lastResult = runVariant("default", false);
      if (lastResult.code != ResultCode::Passed) {
        std::lock_guard<std::mutex> lock(resultsMutex);
        results.push_back(std::move(lastResult));
        size_t done = ++completedCount;
        reportProgress(done, totalTests);
        return;
      }
    }
    if (runStrict) {
      lastResult = runVariant("strict", true);
    }
  }

  {
    std::lock_guard<std::mutex> lock(resultsMutex);
    results.push_back(std::move(lastResult));
  }

  size_t done = ++completedCount;
  reportProgress(done, totalTests);
}

} // namespace

std::unique_ptr<hbc::BCProvider> compileSource(
    llvh::StringRef source,
    llvh::StringRef sourceURL,
    bool strict,
    std::string &errorMsg) {
  // BCProviderFromSrc requires a hermes::Buffer (via OwnedMemoryBuffer).
  auto llvmBuf = llvh::MemoryBuffer::getMemBufferCopy(source, sourceURL);
  auto buf = std::make_unique<OwnedMemoryBuffer>(std::move(llvmBuf));

  hbc::CompileFlags flags;
  flags.strict = strict;
  flags.test262 = true;
  flags.emitAsyncBreakCheck = true;
  flags.enableGenerator = true;
  flags.enableAsyncGenerators = true;

  auto [provider, error] = hbc::BCProviderFromSrc::create(
      std::move(buf),
      sourceURL,
      /*sourceMap=*/"",
      flags);

  if (!provider) {
    errorMsg = error;
    return nullptr;
  }

  return std::move(provider);
}

TestResult executeTestVariant(
    const std::string &testName,
    const std::string &source,
    const std::string &sourceURL,
    bool isStrict,
    const NegativeExpectation &negative,
    unsigned timeoutSeconds) {
  auto startTime = Clock::now();

  auto makeResult = [&](ResultCode code, const std::string &msg) {
    auto endTime = Clock::now();
    TestResult r;
    r.testName = testName;
    r.code = code;
    r.message = msg;
    r.duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime);
    return r;
  };

  bool hasNegative = !negative.phase.empty();
  bool expectCompileError = hasNegative && negative.phase == "parse";
  bool expectRuntimeError = hasNegative && negative.phase == "runtime";
  bool expectResolutionError = hasNegative && negative.phase == "resolution";

  // Compile the source.
  std::string compileError;
  auto bytecode = compileSource(source, sourceURL, isStrict, compileError);

  if (!bytecode) {
    if (expectCompileError || expectResolutionError) {
      if (!negative.errorType.empty() &&
          compileError.find(negative.errorType) == std::string::npos) {
        return makeResult(
            ResultCode::CompileFailed,
            "FAIL: Expected " + negative.errorType +
                " but got: " + compileError);
      }
      return makeResult(ResultCode::Passed, "PASS (expected compile error)");
    }
    return makeResult(
        ResultCode::CompileFailed, "FAIL: Compilation failed: " + compileError);
  }

  if (expectCompileError || expectResolutionError) {
    return makeResult(
        ResultCode::ExecuteFailed,
        "FAIL: Expected compile error but compilation succeeded");
  }

  // Check if compilation already exceeded the timeout budget.
  if (timeoutSeconds > 0) {
    auto elapsed = Clock::now() - startTime;
    if (elapsed >= std::chrono::seconds(timeoutSeconds)) {
      return makeResult(
          ResultCode::CompileTimeout, "FAIL: Compilation timed out");
    }
  }

  return executeCompiledTest(
      testName,
      std::move(bytecode),
      sourceURL,
      expectRuntimeError,
      negative,
      timeoutSeconds,
      startTime);
}

void WorkQueue::push(std::function<void()> task) {
  std::lock_guard<std::mutex> lock(mutex_);
  tasks_.push_back(std::move(task));
  cv_.notify_one();
}

bool WorkQueue::pop(std::function<void()> &task) {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [this] { return !tasks_.empty() || done_; });
  if (tasks_.empty())
    return false;
  task = std::move(tasks_.front());
  tasks_.pop_front();
  return true;
}

void WorkQueue::finish() {
  std::lock_guard<std::mutex> lock(mutex_);
  done_ = true;
  cv_.notify_all();
}

void runAllTests(
    const std::vector<TestEntry> &tests,
    const HarnessCache &harness,
    const Skiplist *skiplist,
    const ExecConfig &config,
    std::vector<TestResult> &results,
    std::atomic<size_t> &featureSkipped,
    std::atomic<size_t> &permanentFeatureSkipped) {
  std::mutex resultsMutex;
  std::atomic<size_t> completedCount{0};
  size_t totalTests = tests.size();

  WorkQueue queue;
  std::vector<std::thread> workers;

  // Launch worker threads.
  unsigned numWorkers = std::min(config.numThreads, (unsigned)tests.size());
  if (numWorkers == 0)
    numWorkers = 1;

  for (unsigned i = 0; i < numWorkers; ++i) {
    workers.emplace_back([&] {
      std::function<void()> task;
      while (queue.pop(task)) {
        task();
      }
    });
  }

  // Enqueue all test tasks.
  for (const auto &entry : tests) {
    queue.push([&entry,
                &harness,
                skiplist,
                &config,
                &results,
                &resultsMutex,
                &completedCount,
                totalTests,
                &featureSkipped,
                &permanentFeatureSkipped] {
      processTestEntry(
          entry,
          harness,
          skiplist,
          config,
          results,
          resultsMutex,
          completedCount,
          totalTests,
          featureSkipped,
          permanentFeatureSkipped);
    });
  }

  // Signal no more tasks and wait for workers.
  queue.finish();
  for (auto &w : workers) {
    w.join();
  }

  llvh::outs() << "\n";
}

} // namespace testrunner
} // namespace hermes
