/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Executor.h"
#include "Frontmatter.h"
#include "HarnessCache.h"
#include "Skiplist.h"
#include "TestDiscovery.h"

#include "hermes/VM/RuntimeFlags.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/raw_ostream.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using namespace hermes::testrunner;

// CompilerDriver.h (transitively included via RuntimeFlags.h) declares a
// real `namespace cl {}`, so we cannot use `namespace cl = llvh::cl;`.
// Instead, bring llvh::cl contents into the existing cl namespace.
namespace cl {
using namespace llvh::cl;
} // namespace cl

namespace {

/// Instantiate RuntimeFlags to register all VM CLI options (-Xjit,
/// -gc-sanitize-handles, -Xes6-proxy, -test262, etc.) in the global
/// llvh::CommandLine state.
hermes::cli::RuntimeFlags runtimeFlags;

cl::list<std::string> TestPaths(
    cl::Positional,
    cl::desc("<test paths (files or directories)>"),
    cl::OneOrMore);

cl::opt<unsigned> NumThreads(
    "j",
    cl::desc("Number of parallel workers (default: hardware concurrency)"),
    cl::init(
        std::thread::hardware_concurrency()
            ? std::thread::hardware_concurrency()
            : 1));

cl::opt<unsigned> Timeout(
    "timeout",
    cl::desc("Per-test timeout in seconds (default: 200)"),
    cl::init(200));

cl::opt<unsigned> ShowSlowestTests(
    "show-slowest-tests",
    cl::desc("Show N slowest tests (default: 0)"),
    cl::init(0));

cl::opt<bool> DumpSource(
    "dump-source",
    cl::desc("Print preprocessed test source and exit"),
    cl::init(false));

cl::opt<std::string>
    SkiplistPath("skiplist", cl::desc("Path to skiplist.json"), cl::Required);

cl::opt<bool> TestIntl(
    "test-intl",
    cl::desc("Include Intl (intl402) tests instead of skipping them"),
    cl::init(false));

cl::opt<bool> Lazy(
    "lazy",
    cl::desc("Force lazy compilation (default: off)"),
    cl::init(false));

cl::opt<bool> Optimize(
    "O",
    cl::desc("Enable optimization passes (default: off)"),
    cl::init(false));

cl::opt<bool> Shermes(
    "shermes",
    cl::desc("Use shermes subprocess for compilation and execution"),
    cl::init(false));

cl::opt<std::string> ShermesBinary(
    "shermes-binary",
    cl::desc("Path to the shermes executable (required with --shermes)"),
    cl::init(""));

cl::list<std::string> ShermesFlags(
    "shermes-flag",
    cl::desc("Extra flag to pass to shermes (can be repeated)"));

cl::opt<std::string> TestSuiteDir(
    "test-suite-dir",
    cl::desc("Path to test262 suite root"),
    cl::init(""));

/// Try to auto-detect the test262 suite root by looking for a harness/
/// directory. Walks up from each test path.
std::string findTest262Dir() {
  if (!TestSuiteDir.empty())
    return llvh::sys::path::convert_to_slash(TestSuiteDir);

  for (const auto &p : TestPaths) {
    // Normalize once up front so the "test262/" substring search and any
    // returned path are platform-agnostic. Subsequent llvh::sys::path::append
    // calls may use the OS-native separator, so we re-normalize before
    // returning.
    std::string normP = llvh::sys::path::convert_to_slash(p);

    // Check if the path itself contains test262.
    llvh::StringRef pathRef(normP);
    auto pos = pathRef.rfind("test262/");
    if (pos != llvh::StringRef::npos) {
      std::string candidate = pathRef.substr(0, pos + 8).str(); // "test262/"
      llvh::SmallString<256> harnessCheck(candidate);
      llvh::sys::path::append(harnessCheck, "harness");
      if (llvh::sys::fs::is_directory(harnessCheck))
        return llvh::sys::path::convert_to_slash(candidate);
    }

    // Walk up looking for a test262 directory with harness/.
    llvh::SmallString<256> dir(normP);
    for (int i = 0; i < 10; ++i) {
      llvh::sys::path::append(dir, "..");
      llvh::SmallString<256> candidate(dir);
      llvh::sys::path::append(candidate, "harness");
      if (llvh::sys::fs::is_directory(candidate)) {
        // Verify this looks like a test262 root.
        llvh::SmallString<256> staCheck(candidate);
        llvh::sys::path::append(staCheck, "sta.js");
        if (llvh::sys::fs::exists(staCheck))
          return llvh::sys::path::convert_to_slash(dir.str());
      }
    }
  }
  return "";
}

/// Result of filtering tests by the skiplist.
struct FilterResult {
  std::vector<TestEntry> tests;
  size_t skippedCount = 0;
  size_t permanentlySkippedCount = 0;
};

/// Filter test entries by the skiplist, separating tests to run from skipped.
FilterResult filterBySkiplist(
    std::vector<TestEntry> &allTests,
    const Skiplist &skiplist,
    bool testIntl,
    bool lazy) {
  FilterResult result;
  for (auto &entry : allTests) {
    SkipReason reason = skiplist.shouldSkipPath(entry.path);
    if (reason != SkipReason::NotSkipped) {
      // When --test-intl is set, don't skip intl tests, but still
      // honor other skip reasons (e.g. platform_skip_list). This
      // mirrors the Python runner's two-pass design where it checks
      // skip_list/platform_skip_list first, then intl_tests separately.
      if (testIntl && reason == SkipReason::IntlTests) {
        reason = skiplist.shouldSkipPathNonIntl(entry.path);
      }
      // Only skip lazy_skip_list tests when --lazy is enabled,
      // matching the Python runner's conditional:
      //   if lazy: skip_categories.append(LAZY_SKIP_LIST)
      if (!lazy && reason == SkipReason::LazySkipList) {
        reason = SkipReason::NotSkipped;
      }
      if (reason != SkipReason::NotSkipped) {
        ++result.skippedCount;
        if (reason == SkipReason::PermanentSkipList)
          ++result.permanentlySkippedCount;
        continue;
      }
    }
    result.tests.push_back(std::move(entry));
  }
  return result;
}

/// Tallied results from test execution.
/// Each test file produces exactly one result (short-circuit on first failure).
struct ResultTally {
  size_t passed = 0;
  size_t compileFail = 0;
  size_t compileTimeout = 0;
  size_t executeFail = 0;
  size_t executeTimeout = 0;
  std::vector<TestResult> failures;

  size_t totalFailed() const {
    return compileFail + compileTimeout + executeFail + executeTimeout;
  }
};

/// Tally results. Each entry is already one-per-file (the executor
/// short-circuits on first variant failure, matching the Python runner).
ResultTally tallyResults(const std::vector<TestResult> &results) {
  ResultTally tally;
  for (const auto &r : results) {
    switch (r.code) {
      case ResultCode::Passed:
        ++tally.passed;
        break;
      case ResultCode::CompileFailed:
        ++tally.compileFail;
        tally.failures.push_back(r);
        break;
      case ResultCode::CompileTimeout:
        ++tally.compileTimeout;
        tally.failures.push_back(r);
        break;
      case ResultCode::Failed:
      case ResultCode::ExecuteFailed:
        ++tally.executeFail;
        tally.failures.push_back(r);
        break;
      case ResultCode::ExecuteTimeout:
        ++tally.executeTimeout;
        tally.failures.push_back(r);
        break;
      case ResultCode::Skipped:
      case ResultCode::PermanentlySkipped:
        break;
    }
  }
  return tally;
}

/// Print the N slowest tests by execution time.
void printSlowestTests(const std::vector<TestResult> &results, unsigned count) {
  if (count == 0 || results.empty())
    return;

  std::vector<const TestResult *> sorted;
  sorted.reserve(results.size());
  for (const auto &r : results) {
    // Skip slots are pre-allocated but never executed; exclude them so
    // they don't pad the list when count exceeds the executed-test count.
    if (r.code == ResultCode::Skipped ||
        r.code == ResultCode::PermanentlySkipped)
      continue;
    sorted.push_back(&r);
  }
  std::sort(sorted.begin(), sorted.end(), [](const auto *a, const auto *b) {
    return a->duration > b->duration;
  });
  unsigned n = std::min((unsigned)sorted.size(), count);
  llvh::outs() << "\nSlowest " << n << " tests:\n";
  llvh::outs() << "-----------------------------------\n";
  for (unsigned i = 0; i < n; ++i) {
    double secs = sorted[i]->duration.count() / 1000000.0;
    llvh::outs() << llvh::format("   %.2fs  ", secs) << sorted[i]->testName
                 << "\n";
  }
  llvh::outs() << "-----------------------------------\n";
}

/// Print summary table and failure details. Returns the number of failed
/// test files.
size_t printResults(
    const std::vector<TestResult> &results,
    size_t skippedCount,
    size_t permanentlySkippedCount,
    size_t featureSkippedCount,
    size_t permanentFeatureSkippedCount,
    double wallSeconds) {
  ResultTally tally = tallyResults(results);

  size_t totalPermanentlySkipped =
      permanentlySkippedCount + permanentFeatureSkippedCount;
  assert(
      skippedCount + featureSkippedCount >= totalPermanentlySkipped &&
      "permanent skips must be a subset of total skips");
  size_t totalSkipped =
      skippedCount + featureSkippedCount - totalPermanentlySkipped;
  size_t totalFailed = tally.totalFailed();
  size_t totalTests =
      tally.passed + totalFailed + totalSkipped + totalPermanentlySkipped;
  size_t executed = tally.passed + totalFailed;
  double passRate =
      executed > 0 ? (double)tally.passed / (double)executed * 100.0 : 0.0;

  // Print failure details.
  if (!tally.failures.empty()) {
    llvh::outs() << "\nFailed tests:\n";
    for (const auto &f : tally.failures) {
      llvh::outs() << "  " << resultCodeName(f.code) << ": " << f.testName
                   << "\n";
      llvh::outs() << "    " << f.message << "\n";
    }
  }

  llvh::outs() << "-----------------------------------\n";
  printSlowestTests(results, ShowSlowestTests);

  // Print result summary table.
  llvh::outs() << llvh::format("Testing time: %.2fs\n", wallSeconds);
  llvh::outs() << "-----------------------------------\n";
  if (totalFailed == 0) {
    llvh::outs() << "| Results              |   PASS   |\n";
  } else {
    llvh::outs() << "| Results              |   FAIL   |\n";
  }
  llvh::outs() << "|----------------------+----------|\n";
  llvh::outs() << llvh::format("| Total                | %8zu |\n", totalTests);
  llvh::outs() << llvh::format(
      "| Passes               | %8zu |\n", tally.passed);
  llvh::outs() << llvh::format(
      "| Failures             | %8zu |\n", totalFailed);
  llvh::outs() << llvh::format(
      "| Skipped              | %8zu |\n", totalSkipped);
  llvh::outs() << llvh::format(
      "| Permanently Skipped  | %8zu |\n", totalPermanentlySkipped);
  llvh::outs() << llvh::format(
      "| Pass Rate            |  %6.2f%% |\n", passRate);
  llvh::outs() << "-----------------------------------\n";
  llvh::outs() << "| Failures             |          |\n";
  llvh::outs() << "|----------------------+----------|\n";
  llvh::outs() << llvh::format(
      "| Compile fail         | %8zu |\n", tally.compileFail);
  llvh::outs() << llvh::format(
      "| Compile timeout      | %8zu |\n", tally.compileTimeout);
  llvh::outs() << llvh::format(
      "| Execute fail         | %8zu |\n", tally.executeFail);
  llvh::outs() << llvh::format(
      "| Execute timeout      | %8zu |\n", tally.executeTimeout);
  llvh::outs() << "-----------------------------------\n";

  return tally.totalFailed();
}

/// Print preprocessed test source for a single test (--dump-source mode).
void dumpTestSource(const TestEntry &entry, const HarnessCache &harness) {
  auto fileBuf = llvh::MemoryBuffer::getFile(entry.path);
  if (!fileBuf) {
    llvh::errs() << "Error: cannot read '" << entry.path << "'\n";
    return;
  }

  llvh::StringRef content = (*fileBuf)->getBuffer();
  TestRecord record = parseFrontmatter(content);

  // Add the 'use strict' prelude unless the test forbids it, is raw, or is a
  // module (modules are implicitly strict).
  bool runStrict =
      !record.isNoStrict() && !record.isRaw() && !record.isModule();

  std::vector<std::string> includes = buildTestIncludes(entry, record);
  // --dump-source defaults to the in-process flavor. The shermes-mode
  // assembly only differs in suppressing our injected $DONE; inspecting
  // it locally is rarely needed.
  llvh::outs() << harness.buildSource(
                      includes,
                      record.src,
                      runStrict,
                      record.isAsync(),
                      /*isShermes=*/false)
               << "\n";
}
} // namespace

int main(int argc, char **argv) {
  llvh::InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(
      argc,
      argv,
      "Hermes test262 runner\n\n"
      "  Runs test262 tests against the Hermes VM.\n"
      "  Accepts individual .js files or directories.\n");

  // Validate --shermes options.
  if (Shermes) {
    if (ShermesBinary.empty()) {
      llvh::errs()
          << "Error: --shermes-binary is required when --shermes is used.\n";
      return 1;
    }
    if (Lazy) {
      llvh::errs() << "Error: --shermes and --lazy are mutually exclusive.\n";
      return 1;
    }
    if (runtimeFlags.JIT != hermes::cli::VMOnlyRuntimeFlags::JITMode::Off) {
      llvh::errs() << "Error: --shermes and -Xjit are mutually exclusive.\n";
      return 1;
    }
  }

  // Discover test files.
  std::vector<TestEntry> allTests = discoverTests(TestPaths);

  if (allTests.empty()) {
    llvh::errs() << "No test files found.\n";
    return 1;
  }

  // Load skiplist.
  Skiplist skiplist;
  // A valid skiplist is required to continue.
  if (!skiplist.load(SkiplistPath)) {
    return 1;
  }

  // Load harness files. The harness is required to assemble test sources.
  HarnessCache harness;
  std::string test262Dir = findTest262Dir();
  if (test262Dir.empty()) {
    llvh::errs() << "Error: cannot locate test262 suite directory\n";
    return 1;
  }
  llvh::SmallString<256> harnessDir(test262Dir);
  llvh::sys::path::append(harnessDir, "harness");
  if (!harness.load(harnessDir.str())) {
    return 1;
  }

  // --dump-source mode: parse frontmatter, assemble source, print to stdout.
  // Requires exactly one input path that is a .js file. Skips filtering since
  // dump-source is just preprocessing.
  if (DumpSource) {
    if (TestPaths.size() != 1 || allTests.size() != 1 ||
        !llvh::sys::fs::is_regular_file(TestPaths[0])) {
      llvh::errs()
          << "Error: --dump-source requires exactly one .js file path.\n";
      return 1;
    }
    dumpTestSource(allTests[0], harness);
    return 0;
  }

  // Filter tests by skiplist.
  FilterResult filtered = filterBySkiplist(allTests, skiplist, TestIntl, Lazy);

  llvh::outs() << "-- Testing: " << filtered.tests.size() << " tests"
               << ", max " << NumThreads << " concurrent tasks --\n";

  if (filtered.skippedCount > 0) {
    llvh::outs() << "Skipped " << filtered.skippedCount
                 << " tests via skiplist.\n";
  }

  // Execute tests with thread pool.
  ExecConfig execConfig;
  execConfig.numThreads = NumThreads;
  execConfig.timeoutSeconds = Timeout;
  execConfig.optimize = Optimize;
  execConfig.lazy = Lazy;
  execConfig.shermes = Shermes;
  execConfig.shermesBinary = ShermesBinary;
  execConfig.shermesExtraFlags.assign(ShermesFlags.begin(), ShermesFlags.end());
  execConfig.runtimeFlags = &runtimeFlags;

  std::vector<TestResult> results;
  std::atomic<size_t> featureSkippedCount{0};
  std::atomic<size_t> permanentFeatureSkippedCount{0};

  auto wallStart = std::chrono::steady_clock::now();

  runAllTests(
      filtered.tests,
      harness,
      &skiplist,
      execConfig,
      results,
      featureSkippedCount,
      permanentFeatureSkippedCount);

  auto wallEnd = std::chrono::steady_clock::now();
  double wallSeconds =
      std::chrono::duration<double>(wallEnd - wallStart).count();

  size_t totalFailures = printResults(
      results,
      filtered.skippedCount,
      filtered.permanentlySkippedCount,
      featureSkippedCount.load(),
      permanentFeatureSkippedCount.load(),
      wallSeconds);

  return totalFailures > 0 ? 1 : 0;
}
