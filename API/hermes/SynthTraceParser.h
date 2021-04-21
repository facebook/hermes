/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SYNTHTRACEPARSER_H
#define HERMES_SYNTHTRACEPARSER_H

#include <tuple>

#include "hermes/Public/RuntimeConfig.h"
#include "hermes/SynthTrace.h"
#include "hermes/VM/MockedEnvironment.h"

#include "llvh/Support/MemoryBuffer.h"

namespace facebook {
namespace hermes {
namespace tracing {

/// Parse a trace from a JSON string stored in a MemoryBuffer.
std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig::Builder,
    ::hermes::vm::GCConfig::Builder,
    ::hermes::vm::MockedEnvironment>
parseSynthTrace(std::unique_ptr<llvh::MemoryBuffer> trace);

/// Parse a trace from a JSON string stored in the given file name.
std::tuple<
    SynthTrace,
    ::hermes::vm::RuntimeConfig::Builder,
    ::hermes::vm::GCConfig::Builder,
    ::hermes::vm::MockedEnvironment>
parseSynthTrace(const std::string &tracefile);

} // namespace tracing
} // namespace hermes
} // namespace facebook

#endif // HERMES_SYNTHTRACEPARSER_H
