/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_HERMES_TRACING_H
#define HERMES_HERMES_TRACING_H

#include <hermes/hermes.h>

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace facebook {
namespace hermes {

/// Like the method above, except takes a file descriptor instead of
/// a stream.  If the \p traceFileDescriptor argument is -1, do not write
/// the trace to a file.
std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig);

std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    bool forReplay = false);

} // namespace hermes
} // namespace facebook

#endif
