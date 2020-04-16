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
#include <llvm/Support/Process.h>
#endif

namespace facebook {
namespace hermes {

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    int traceFileDescriptor,
    const std::string &traceFilename) {
  if (runtimeConfig.getTraceEnabled()) {
#ifdef HERMESVM_API_TRACE
    llvm::sys::Process::SafelyCloseFileDescriptor(traceFileDescriptor);
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime),
        runtimeConfig,
        traceFilename,
        traceFilename,
        []() { return true; });
#endif
  }
  return hermesRuntime;
}

} // namespace hermes
} // namespace facebook
