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
#include <llvm/Support/raw_ostream.h>
#endif

namespace facebook {
namespace hermes {

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvm::raw_ostream> traceStream,
    const std::string &traceFilename) {
  if (runtimeConfig.getTraceEnvironmentInteractions()) {
#ifdef HERMESVM_API_TRACE
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime),
        runtimeConfig,
        std::move(traceStream),
        traceFilename);
#endif
  }
  return hermesRuntime;
}

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    int traceFileDescriptor,
    const std::string &traceFilename) {
  if (runtimeConfig.getTraceEnvironmentInteractions()) {
#ifdef HERMESVM_API_TRACE
    std::unique_ptr<llvm::raw_ostream> traceStream;
    if (traceFileDescriptor != -1) {
      traceStream = ::hermes::make_unique<llvm::raw_fd_ostream>(
          traceFileDescriptor, /*shouldClose*/ true);
    }
    return tracing::makeTracingHermesRuntime(
        std::move(hermesRuntime),
        runtimeConfig,
        std::move(traceStream),
        traceFilename);
#endif
  }
  return hermesRuntime;
}

} // namespace hermes
} // namespace facebook
