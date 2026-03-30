/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/CommandLine.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/raw_ostream.h"

#include <string>
#include <thread>
#include <vector>

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

} // namespace

int main(int argc, char **argv) {
  llvh::InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(
      argc,
      argv,
      "Hermes test262 runner\n\n"
      "  Runs test262 tests against the Hermes VM.\n"
      "  Accepts individual .js files or directories.\n");

  llvh::outs() << "Received " << TestPaths.size() << " input path(s).\n";
  llvh::outs() << "Workers: " << NumThreads << "\n";
  llvh::outs() << "Timeout: " << Timeout << "s\n";

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
