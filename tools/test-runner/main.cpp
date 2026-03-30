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

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/raw_ostream.h"

#include <algorithm>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

using namespace hermes::testrunner;
namespace cl = llvh::cl;

namespace {

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
    cl::desc("Per-test timeout in seconds (default: 30)"),
    cl::init(30));

cl::opt<unsigned> ShowSlowestTests(
    "show-slowest-tests",
    cl::desc("Show N slowest tests (default: 0)"),
    cl::init(0));

cl::opt<bool> DumpSource(
    "dump-source",
    cl::desc("Print preprocessed test source and exit"),
    cl::init(false));

cl::opt<std::string>
    SkiplistPath("skiplist", cl::desc("Path to skiplist.json"), cl::init(""));

cl::opt<std::string> TestSuiteDir(
    "test-suite-dir",
    cl::desc("Path to test262 suite root"),
    cl::init(""));

/// Try to auto-detect the test262 suite root by looking for a harness/
/// directory. Walks up from each test path.
std::string findTest262Dir() {
  if (!TestSuiteDir.empty())
    return TestSuiteDir;

  for (const auto &p : TestPaths) {
    // Check if the path itself contains test262.
    llvh::StringRef pathRef(p);
    auto pos = pathRef.rfind("test262/");
    if (pos != llvh::StringRef::npos) {
      std::string candidate = pathRef.substr(0, pos + 8).str(); // "test262/"
      llvh::SmallString<256> harnessCheck(candidate);
      llvh::sys::path::append(harnessCheck, "harness");
      if (llvh::sys::fs::is_directory(harnessCheck))
        return candidate;
    }

    // Walk up looking for a test262 directory with harness/.
    llvh::SmallString<256> dir(p);
    for (int i = 0; i < 10; ++i) {
      llvh::sys::path::append(dir, "..");
      llvh::SmallString<256> candidate(dir);
      llvh::sys::path::append(candidate, "harness");
      if (llvh::sys::fs::is_directory(candidate)) {
        // Verify this looks like a test262 root.
        llvh::SmallString<256> staCheck(candidate);
        llvh::sys::path::append(staCheck, "sta.js");
        if (llvh::sys::fs::exists(staCheck))
          return std::string(dir.str());
      }
    }
  }
  return "";
}

/// Check if a test should be skipped due to unsupported features.
bool shouldSkipByFeature(
    const TestRecord &record,
    const Skiplist &skiplist,
    bool hasSkiplist) {
  if (!hasSkiplist)
    return false;
  for (const auto &feat : record.features) {
    if (skiplist.shouldSkipFeature(feat) != SkipReason::NotSkipped)
      return true;
  }
  return false;
}

/// Build the list of harness includes for a test entry.
std::vector<std::string> buildIncludes(
    const TestEntry &entry,
    const TestRecord &record) {
  std::vector<std::string> includes;
  if (entry.suiteKind == SuiteKind::Test262) {
    includes.push_back("sta.js");
    includes.push_back("assert.js");
  }
  for (const auto &inc : record.includes) {
    includes.push_back(inc);
  }
  return includes;
}

/// Result of filtering tests by the skiplist.
struct FilterResult {
  std::vector<TestEntry> tests;
  size_t skippedCount = 0;
};

/// Filter test entries by the skiplist, separating tests to run from skipped.
FilterResult filterBySkiplist(
    std::vector<TestEntry> &allTests,
    const Skiplist *skiplist) {
  FilterResult result;
  for (auto &entry : allTests) {
    if (skiplist) {
      SkipReason reason = skiplist->shouldSkipPath(entry.path);
      if (reason != SkipReason::NotSkipped) {
        ++result.skippedCount;
        continue;
      }
    }
    result.tests.push_back(std::move(entry));
  }
  return result;
}

/// Print preprocessed test source for a single test (--dump-source mode).
void dumpTestSource(
    const TestEntry &entry,
    const HarnessCache &harness,
    const Skiplist &skiplist,
    bool hasSkiplist) {
  auto fileBuf = llvh::MemoryBuffer::getFile(entry.path);
  if (!fileBuf) {
    llvh::errs() << "Error: cannot read '" << entry.path << "'\n";
    return;
  }

  llvh::StringRef content = (*fileBuf)->getBuffer();
  TestRecord record = parseFrontmatter(content);

  if (shouldSkipByFeature(record, skiplist, hasSkiplist))
    return;

  // Determine strict mode variants.
  bool runStrict = !record.isNoStrict() && !record.isRaw();
  bool runNonStrict = !record.isOnlyStrict() && !record.isRaw();
  bool runRaw = record.isRaw();

  // For modules, only run non-strict (modules are implicitly strict).
  if (record.isModule()) {
    runStrict = false;
    runNonStrict = true;
    runRaw = false;
  }

  std::vector<std::string> includes = buildIncludes(entry, record);

  auto printVariant = [&](const char *label, bool isStrict) {
    std::string source = harness.buildSource(includes, record.src, isStrict);
    llvh::outs() << "=== " << entry.fullName << " (" << label << ") ===\n";
    if (record.hasNegative()) {
      llvh::outs() << "// negative: phase=" << record.negative.phase
                   << " type=" << record.negative.errorType << "\n";
    }
    llvh::outs() << source << "\n";
  };

  if (runRaw) {
    printVariant("raw", false);
  } else {
    if (runNonStrict) {
      printVariant("default", false);
    }
    if (runStrict) {
      printVariant("strict", true);
    }
  }
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

  // Discover test files.
  std::vector<TestEntry> allTests = discoverTests(TestPaths);

  if (allTests.empty()) {
    llvh::errs() << "No test files found.\n";
    return 1;
  }

  // Load skiplist if available.
  Skiplist skiplist;
  bool hasSkiplist = false;
  if (!SkiplistPath.empty()) {
    hasSkiplist = skiplist.load(SkiplistPath);
    if (hasSkiplist) {
      llvh::outs() << "Loaded skiplist: " << skiplist.totalSkipPaths()
                   << " skip paths, " << skiplist.totalUnsupportedFeatures()
                   << " unsupported features\n";
    }
  }

  // Load harness files.
  HarnessCache harness;
  std::string test262Dir = findTest262Dir();
  if (!test262Dir.empty()) {
    llvh::SmallString<256> harnessDir(test262Dir);
    llvh::sys::path::append(harnessDir, "harness");
    if (harness.load(harnessDir.str())) {
      llvh::outs() << "Loaded " << harness.size() << " harness files from "
                   << harnessDir << "\n";
    }
  }

  // Filter tests by skiplist.
  FilterResult filtered =
      filterBySkiplist(allTests, hasSkiplist ? &skiplist : nullptr);

  llvh::outs() << "-- Testing: " << filtered.tests.size() << " tests"
               << ", max " << NumThreads << " concurrent tasks --\n";

  if (filtered.skippedCount > 0) {
    llvh::outs() << "Skipped " << filtered.skippedCount
                 << " tests via skiplist.\n";
  }

  // --dump-source mode: parse frontmatter, assemble source, print to stdout.
  // Requires exactly one input path that is a .js file.
  if (DumpSource) {
    if (TestPaths.size() != 1 ||
        !llvh::sys::fs::is_regular_file(TestPaths[0])) {
      llvh::errs()
          << "Error: --dump-source requires exactly one .js file path.\n";
      return 1;
    }
    if (filtered.tests.empty()) {
      llvh::outs() << "Test was skipped by skiplist.\n";
      return 0;
    }
    dumpTestSource(filtered.tests[0], harness, skiplist, hasSkiplist);
    return 0;
  }

  // Execute tests with thread pool.
  ExecConfig execConfig;
  execConfig.numThreads = NumThreads;
  execConfig.timeoutSeconds = Timeout;

  std::vector<TestResult> results;
  std::atomic<size_t> featureSkippedCount{0};

  runAllTests(
      filtered.tests,
      harness,
      hasSkiplist ? &skiplist : nullptr,
      execConfig,
      results,
      featureSkippedCount);

  // Tally results.
  size_t passed = 0, executeFailed = 0, compileFailed = 0, compileTimedOut = 0,
         executeTimedOut = 0;
  std::vector<TestResult> failures;

  for (const auto &r : results) {
    switch (r.code) {
      case ResultCode::Passed:
        ++passed;
        break;
      case ResultCode::Failed:
      case ResultCode::ExecuteFailed:
        ++executeFailed;
        failures.push_back(r);
        break;
      case ResultCode::CompileFailed:
        ++compileFailed;
        failures.push_back(r);
        break;
      case ResultCode::CompileTimeout:
        ++compileTimedOut;
        failures.push_back(r);
        break;
      case ResultCode::ExecuteTimeout:
        ++executeTimedOut;
        failures.push_back(r);
        break;
      case ResultCode::Skipped:
      case ResultCode::PermanentlySkipped:
        break;
    }
  }

  // Print failures.
  if (!failures.empty()) {
    llvh::outs() << "\n--- FAILURES ---\n";
    for (const auto &f : failures) {
      llvh::outs() << resultCodeName(f.code) << ": " << f.testName << "\n";
      llvh::outs() << "  " << f.message << "\n";
    }
  }

  // Show slowest tests.
  if (ShowSlowestTests > 0 && !results.empty()) {
    std::vector<TestResult *> sorted;
    sorted.reserve(results.size());
    for (auto &r : results)
      sorted.push_back(&r);
    std::sort(sorted.begin(), sorted.end(), [](const auto *a, const auto *b) {
      return a->duration > b->duration;
    });
    unsigned count =
        std::min((unsigned)sorted.size(), ShowSlowestTests.getValue());
    llvh::outs() << "\n--- " << count << " SLOWEST TESTS ---\n";
    for (unsigned i = 0; i < count; ++i) {
      double ms = sorted[i]->duration.count() / 1000.0;
      llvh::outs() << llvh::format("%8.1f ms  ", ms) << sorted[i]->testName
                   << "\n";
    }
  }

  // Summary.
  llvh::outs() << "\n=== RESULTS ===\n";
  llvh::outs() << "  Passed:          " << passed << "\n";
  llvh::outs() << "  Execute failed:  " << executeFailed << "\n";
  llvh::outs() << "  Compile failed:  " << compileFailed << "\n";
  llvh::outs() << "  Compile timeout: " << compileTimedOut << "\n";
  llvh::outs() << "  Execute timeout: " << executeTimedOut << "\n";
  llvh::outs() << "  Skipped (path):  " << filtered.skippedCount << "\n";
  llvh::outs() << "  Skipped (feat):  " << featureSkippedCount.load() << "\n";
  llvh::outs() << "  Total variants:  " << results.size() << "\n";

  return (executeFailed + compileFailed + compileTimedOut + executeTimedOut) > 0
      ? 1
      : 0;
}
