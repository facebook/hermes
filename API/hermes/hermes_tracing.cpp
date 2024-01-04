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
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    const std::string &traceScratchPath,
    const std::string &traceResultPath,
    std::function<bool()> traceCompletionCallback) {
  auto mode = runtimeConfig.getSynthTraceMode();
  if (mode == ::hermes::vm::SynthTraceMode::Tracing ||
      mode == ::hermes::vm::SynthTraceMode::TracingAndReplaying) {
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime),
        runtimeConfig,
        traceScratchPath,
        traceResultPath,
        std::move(traceCompletionCallback));
  }
  return hermesRuntime;
}

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    bool forReplay) {
  return tracing::makeTracingHermesRuntime(
      std::move(hermesRuntime),
      runtimeConfig,
      std::move(traceStream),
      forReplay);
}

} // namespace hermes
} // namespace facebook
