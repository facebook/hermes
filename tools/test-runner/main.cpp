/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Skiplist.h"
#include "TestDiscovery.h"

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/raw_ostream.h"

#include <string>
#include <thread>
#include <vector>

namespace cl = llvh::cl;
using namespace hermes::testrunner;

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

  // Apply skiplist to filter tests.
  std::vector<TestEntry> testsToRun;
  size_t skippedCount = 0;
  for (auto &entry : allTests) {
    if (hasSkiplist) {
      SkipReason reason = skiplist.shouldSkipPath(entry.path);
      if (reason != SkipReason::NotSkipped) {
        ++skippedCount;
        continue;
      }
    }
    testsToRun.push_back(std::move(entry));
  }

  // Match Python output format.
  llvh::outs() << "-- Testing: " << testsToRun.size() << " tests"
               << ", max " << NumThreads << " concurrent tasks --\n";

  if (skippedCount > 0) {
    llvh::outs() << "Skipped " << skippedCount << " tests via skiplist.\n";
  }

  if (DumpSource) {
    llvh::outs() << "dump-source mode: would print preprocessed source.\n";
    // TODO: Implement test262 harness preprocessing and source dumping.
    return 0;
  }

  if (ShowSlowestTests > 0) {
    llvh::outs() << "Will show " << ShowSlowestTests << " slowest tests.\n";
  }

  // TODO: Implement test discovery and execution.
  llvh::outs() << "Test execution not yet implemented.\n";
  return 0;
}
