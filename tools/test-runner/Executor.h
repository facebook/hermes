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
struct TestResult {
  /// Full test name including variant suffix.
  std::string testName;
  ResultCode code;
  std::string message;
  /// Duration of this variant's execution.
  std::chrono::microseconds duration{0};
};

/// Configuration for test execution.
struct ExecConfig {
  unsigned numThreads = 1;
  unsigned timeoutSeconds = 30;
};

/// Compile JS source to bytecode in-memory.
///
/// \p source the JavaScript source (must be null-terminated).
/// \p sourceURL filename for error messages.
/// \p strict whether to compile in strict mode.
/// \p[out] errorMsg set to the compile error message on failure.
///
/// Returns the BCProvider on success, nullptr on failure.
std::unique_ptr<hbc::BCProvider> compileSource(
    llvh::StringRef source,
    llvh::StringRef sourceURL,
    bool strict,
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
    const std::string &testName,
    const std::string &source,
    const std::string &sourceURL,
    bool isStrict,
    const NegativeExpectation &negative,
    unsigned timeoutSeconds,
    bool disableHandleSan = false);

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
