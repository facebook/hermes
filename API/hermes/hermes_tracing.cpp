/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_tracing.h"

#ifdef HERMESVM_API_TRACE
#include <hermes/Support/Algorithms.h>
#include <hermes/TracingRuntime.h>
#endif

namespace facebook {
namespace hermes {

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  if (runtimeConfig.getTraceEnabled()) {
#ifdef HERMESVM_API_TRACE
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime),
        runtimeConfig,
        runtimeConfig.getTraceScratchPath(),
        runtimeConfig.getTraceResultPath(),
        runtimeConfig.getTraceRegisterCallback());
#endif
  }
  return hermesRuntime;
}

} // namespace hermes
} // namespace facebook
