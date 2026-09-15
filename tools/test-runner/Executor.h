/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_TESTRUNNER_EXECUTOR_H
#define HERMES_TOOLS_TESTRUNNER_EXECUTOR_H

#include "Frontmatter.h"
#include "HarnessCache.h"
#include "Skiplist.h"
#include "TestDiscovery.h"

#include "hermes/BCGen/HBC/BCProvider.h"

#include "llvh/ADT/StringRef.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace hermes {

namespace cli {
struct RuntimeFlags;
} // namespace cli

namespace testrunner {

/// Result code for a single test variant.
enum class ResultCode {
  Passed,
  Failed,
  Skipped,
  PermanentlySkipped,
  CompileFailed,
  CompileTimeout,
  ExecuteFailed,
  ExecuteTimeout,
};

/// Display name for a result code.
inline const char *resultCodeName(ResultCode code) {
  switch (code) {
    case ResultCode::Passed:
      return "PASS";
    case ResultCode::Failed:
      return "FAIL";
    case ResultCode::Skipped:
      return "SKIP";
    case ResultCode::PermanentlySkipped:
      return "PERMANENTLY_SKIP";
    case ResultCode::CompileFailed:
      return "COMPILE_FAIL";
    case ResultCode::CompileTimeout:
      return "COMPILE_TIMEOUT";
    case ResultCode::ExecuteFailed:
      return "EXECUTE_FAIL";
    case ResultCode::ExecuteTimeout:
      return "EXECUTE_TIMEOUT";
  }
  return "UNKNOWN";
}

/// Result of running a single test variant (strict or non-strict).
/// Default-constructed slots represent skipped tests; runAllTests pre-sizes
/// the results vector so workers can write to a unique index without locking.
struct TestResult {
  /// Non-owning reference to the test name. Borrowed from the corresponding
  /// `TestEntry::fullName` in the `tests` vector passed to `runAllTests`,
  /// which outlives `results`. Matches the Python runner's naming — no
  /// variant suffix.
  llvh::StringRef testName;
  ResultCode code = ResultCode::Skipped;
  std::string message;
  /// Duration of this variant's execution.
  std::chrono::microseconds duration{0};
};

/// Configuration for test execution.
struct ExecConfig {
  unsigned numThreads = 1;
  unsigned timeoutSeconds = 200;
  bool optimize = false;
  bool lazy = false;
  bool shermes = false;
  std::string shermesBinary;
  /// Extra flags to pass to shermes during compilation.
  std::vector<std::string> shermesExtraFlags;
  /// Parsed command line runtime flags (registered via RuntimeFlags.h).
  const cli::RuntimeFlags *runtimeFlags = nullptr;
};

/// Compile JS source to bytecode in-memory.
///
/// \p source the JavaScript source (must be null-terminated).
/// \p sourceURL filename for error messages.
/// \p strict whether to compile in strict mode.
/// \p optimize whether to run optimization passes.
/// \p lazy whether to enable lazy compilation.
/// \p[out] errorMsg set to the compile error message on failure.
///
/// Returns the BCProvider on success, nullptr on failure.
std::unique_ptr<hbc::BCProvider> compileSource(
    llvh::StringRef source,
    llvh::StringRef sourceURL,
    bool strict,
    bool optimize,
    bool lazy,
    std::string &errorMsg);

/// Build the list of harness includes for a test entry.
/// Handles test262 default includes (sta.js, assert.js), test-specified
/// includes, and doneprintHandle.js for async tests.
std::vector<std::string> buildTestIncludes(
    const TestEntry &entry,
    const TestRecord &record);

/// Execute a single test variant (compile + run) in-process.
///
/// Creates a fresh HermesRuntime, compiles the source to bytecode,
/// runs it, handles negative expectations, and drains microtasks.
/// When \p disableHandleSan is true, GC handle sanitization is disabled
/// for this test (sanitize rate set to 0), matching the Python runner's
/// behavior for handlesan_skip_list tests.
TestResult executeTestVariant(
    llvh::StringRef testName,
    llvh::StringRef source,
    llvh::StringRef sourceURL,
    bool isStrict,
    bool isAsync,
    const NegativeExpectation &negative,
    const ExecConfig &config,
    bool disableHandleSan = false);

/// Execute a single test variant using shermes subprocess compilation
/// and execution (two-step approach matching the Python runner).
///
/// Step 1: Compile JS source to a native binary using shermes.
/// Step 2: Run the native binary and evaluate the result.
///
/// \p testName display name for the test (non-owning, must outlive the
///    returned TestResult).
/// \p source preprocessed JavaScript source.
/// \p isStrict whether to compile in strict mode.
/// \p isAsync whether this is an async test (check stdout patterns).
/// \p negative expected failure specification.
/// \p timeoutSeconds per-step timeout.
/// \p optimize whether to pass -O (vs -O0) to shermes.
/// \p disableHandleSan whether to disable GC handle sanitization.
/// \p shermesBinary path to the shermes executable.
/// \p shermesExtraFlags extra flags to pass to shermes.
TestResult executeTestVariantShermes(
    llvh::StringRef testName,
    llvh::StringRef source,
    bool isStrict,
    bool isAsync,
    const NegativeExpectation &negative,
    unsigned timeoutSeconds,
    bool optimize,
    bool disableHandleSan,
    const std::string &shermesBinary,
    const std::vector<std::string> &shermesExtraFlags);

/// Thread-safe work queue for distributing tests to worker threads.
class WorkQueue {
  std::mutex mutex_;
  std::condition_variable cv_;
  std::deque<std::function<void()>> tasks_;
  bool done_ = false;

 public:
  /// Add a task to the queue.
  void push(std::function<void()> task);

  /// Get the next task. Returns false if the queue is done.
  bool pop(std::function<void()> &task);

  /// Signal that no more tasks will be added.
  void finish();
};

/// Run all test entries using a thread pool.
void runAllTests(
    const std::vector<TestEntry> &tests,
    const HarnessCache &harness,
    const Skiplist *skiplist,
    const ExecConfig &config,
    std::vector<TestResult> &results,
    std::atomic<size_t> &featureSkipped,
    std::atomic<size_t> &permanentFeatureSkipped);

} // namespace testrunner
} // namespace hermes

#endif // HERMES_TOOLS_TESTRUNNER_EXECUTOR_H
