/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IS_MOBILE_BUILD

#include "hermes/VM/RuntimeFlags.h"

namespace hermes::cli {

/// Build runtime config from the parsed command line flags.
vm::RuntimeConfig buildRuntimeConfig(const RuntimeFlags &flags) {
  return vm::RuntimeConfig::Builder()
      .withGCConfig(vm::GCConfig::Builder()
                        .withMinHeapSize(flags.MinHeapSize.bytes)
                        .withInitHeapSize(flags.InitHeapSize.bytes)
                        .withMaxHeapSize(flags.MaxHeapSize.bytes)
                        .withOccupancyTarget(flags.OccupancyTarget)
                        .withSanitizeConfig(
                            vm::GCSanitizeConfig::Builder()
                                .withSanitizeRate(flags.GCSanitizeRate)
                                .withRandomSeed(flags.GCSanitizeRandomSeed)
                                .build())
                        .withShouldReleaseUnused(vm::kReleaseUnusedOld)
                        .withAllocInYoung(flags.GCAllocYoung)
                        .withRevertToYGAtTTI(flags.GCRevertToYGAtTTI)
                        .build())
      .withEnableEval(flags.EnableEval)
      .withVerifyEvalIR(flags.VerifyIR)
      .withOptimizedEval(flags.OptimizedEval)
      .withAsyncBreakCheckInEval(flags.EmitAsyncBreakCheck)
      .withVMExperimentFlags(flags.VMExperimentFlags)
      .withES6Promise(flags.ES6Promise)
      .withES6Proxy(flags.ES6Proxy)
      .withIntl(flags.Intl)
      .withMicrotaskQueue(flags.MicrotaskQueue)
      .withEnableSampleProfiling(flags.SampleProfiling)
      .withRandomizeMemoryLayout(flags.RandomizeMemoryLayout)
      .withTrackIO(flags.TrackBytecodeIO)
      .withEnableHermesInternal(flags.EnableHermesInternal)
      .withEnableHermesInternalTestMethods(
          flags.EnableHermesInternalTestMethods)
      .build();
}

} // namespace hermes::cli

#endif // HERMES_IS_MOBILE_BUILD
