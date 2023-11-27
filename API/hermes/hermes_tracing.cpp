/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_tracing.h"

#include <hermes/Support/Algorithms.h>
#include <hermes/TracingRuntime.h>

namespace facebook {
namespace hermes {

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  auto mode = runtimeConfig.getSynthTraceMode();
  if (mode == ::hermes::vm::SynthTraceMode::Tracing ||
      mode == ::hermes::vm::SynthTraceMode::TracingAndReplaying) {
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime),
        runtimeConfig,
        runtimeConfig.getTraceScratchPath(),
        runtimeConfig.getTraceResultPath(),
        runtimeConfig.getTraceRegisterCallback());
  }
  return hermesRuntime;
}

} // namespace hermes
} // namespace facebook
