/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes_tracing.h"
#include "llvm/Support/ErrorHandling.h"

namespace facebook {
namespace hermes {

// This function is here for ABI linking compatibility.
std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  if (!runtimeConfig.getTraceEnvironmentInteractions()) {
    return hermesRuntime;
  }
  llvm_unreachable("Invoked compat makeTracingHermesRuntime!");
}

} // namespace hermes
} // namespace facebook
