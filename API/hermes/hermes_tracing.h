/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_HERMES_TRACING_H
#define HERMES_HERMES_TRACING_H

#include <hermes/hermes.h>

#include "llvm/Support/raw_ostream.h"

namespace facebook {
namespace hermes {

/// Creates and returns a jsi::Runtime that traces JSI interactions.
/// If \p traceStream is non-null, writes the trace to \p traceStream.
/// If non-empty, \p traceFilename is the file to which \p traceStream writes.
std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvm::raw_ostream> traceStream = nullptr,
    const std::string &traceFilename = "");

/// Like the method above, except takes a file descriptor instead of
/// a stream.  If the \p traceFileDescriptor argument is -1, do not write
/// the trace to a file.
std::unique_ptr<jsi::Runtime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    int traceFileDescriptor,
    const std::string &traceFilename);

} // namespace hermes
} // namespace facebook

#endif
