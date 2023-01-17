/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/ConsoleHost/RuntimeFlags.h"
#include "hermes/VM/StaticHUtils.h"

using namespace hermes;
using namespace hermes::vm;

namespace {
namespace cli {

llvh::cl::opt<bool> RandomizeMemoryLayout(
    "Xrandomize-memory-layout",
    llvh::cl::desc("Randomize stack placement etc."),
    llvh::cl::init(false),
    llvh::cl::Hidden);

llvh::cl::opt<bool> GCAllocYoung(
    "gc-alloc-young",
    llvh::cl::desc(
        "Determines whether to (initially) allocate in the young generation"),
    llvh::cl::cat(cl::GCCategory),
    llvh::cl::init(true));

llvh::cl::opt<bool> GCRevertToYGAtTTI(
    "gc-revert-to-yg-at-tti",
    llvh::cl::desc(
        "Determines whether to revert to young generation, if necessary, at "
        "TTI notification"),
    llvh::cl::cat(cl::GCCategory),
    llvh::cl::init(false));

llvh::cl::opt<bool> EnableEval(
    "enable-eval",
    llvh::cl::init(true),
    llvh::cl::desc("Enable support for eval()"));

// This is normally a compiler option, but it also applies to strings given
// to eval or the Function constructor.
llvh::cl::opt<bool> VerifyIR(
    "verify-ir",
#ifdef HERMES_SLOW_DEBUG
    llvh::cl::init(true),
#else
    llvh::cl::init(false),
    llvh::cl::Hidden,
#endif
    llvh::cl::desc("Verify the IR after creating it"));

llvh::cl::opt<bool> EmitAsyncBreakCheck(
    "emit-async-break-check",
    llvh::cl::desc("Emit instruction to check async break request"),
    llvh::cl::init(false));

llvh::cl::opt<bool> OptimizedEval(
    "optimized-eval",
    llvh::cl::desc("Turn on compiler optimizations in eval."),
    llvh::cl::init(false));

} // namespace cli
} // namespace

/// The lifetime of a Runtime is managed by a smart pointer, but the C API wants
/// to deal with a regular pointer. Keep all created runtimes here, so they can
/// be destroyed from a pointer.
static llvh::DenseMap<Runtime *, std::shared_ptr<Runtime>> s_runtimes{};

static RuntimeConfig buildRuntimeConfig();

extern "C" void _SH_MODEL(void) {}

extern "C" SHRuntime *_sh_init(int argc, char **argv) {
  if (argc)
    llvh::cl::ParseCommandLineOptions(argc, argv);

  auto config = buildRuntimeConfig();
  std::shared_ptr<Runtime> runtimePtr = Runtime::create(config);
  // Get the pointer first, since order of argument evaluation is not defined.
  Runtime *pRuntime = runtimePtr.get();
  s_runtimes.try_emplace(pRuntime, std::move(runtimePtr));
  return getSHRuntime(*pRuntime);
}

static RuntimeConfig buildRuntimeConfig() {
  return vm::RuntimeConfig::Builder()
      .withGCConfig(
          vm::GCConfig::Builder()
              .withMinHeapSize(cl::MinHeapSize.bytes)
              .withInitHeapSize(cl::InitHeapSize.bytes)
              .withMaxHeapSize(cl::MaxHeapSize.bytes)
              .withOccupancyTarget(cl::OccupancyTarget)
              .withSanitizeConfig(vm::GCSanitizeConfig::Builder()
                                      .withSanitizeRate(cl::GCSanitizeRate)
                                      .withRandomSeed(cl::GCSanitizeRandomSeed)
                                      .build())
              .withShouldReleaseUnused(vm::kReleaseUnusedOld)
              .withAllocInYoung(cli::GCAllocYoung)
              .withRevertToYGAtTTI(cli::GCRevertToYGAtTTI)
              .build())
      .withEnableEval(cli::EnableEval)
      .withVerifyEvalIR(cli::VerifyIR)
      .withOptimizedEval(cli::OptimizedEval)
      .withAsyncBreakCheckInEval(cli::EmitAsyncBreakCheck)
      .withVMExperimentFlags(cl::VMExperimentFlags)
      .withES6Promise(cl::ES6Promise)
      .withES6Proxy(cl::ES6Proxy)
      .withIntl(cl::Intl)
      .withMicrotaskQueue(cl::MicrotaskQueue)
      .withEnableSampleProfiling(cl::SampleProfiling)
      .withRandomizeMemoryLayout(cli::RandomizeMemoryLayout)
      .withTrackIO(cl::TrackBytecodeIO)
      .withEnableHermesInternal(cl::EnableHermesInternal)
      .withEnableHermesInternalTestMethods(cl::EnableHermesInternalTestMethods)
      .build();
}

extern "C" void _sh_done(SHRuntime *shr) {
  auto it = s_runtimes.find(&getRuntime(shr));
  if (it == s_runtimes.end()) {
    llvh::errs() << "SHRuntime not found\n";
    abort();
  }
  s_runtimes.erase(it);
}
