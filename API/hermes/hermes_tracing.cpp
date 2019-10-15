/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_tracing.h"

#include <hermes/TracingRuntime.h>

namespace facebook {
namespace hermes {

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  if (runtimeConfig.getTraceEnvironmentInteractions()) {
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime), runtimeConfig);
  } else {
    return hermesRuntime;
  }
}

} // namespace hermes
} // namespace facebook
