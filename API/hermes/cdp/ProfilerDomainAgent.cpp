/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProfilerDomainAgent.h"

#include <sstream>

#include <hermes/hermes.h>

namespace facebook {
namespace hermes {
namespace cdp {

ProfilerDomainAgent::ProfilerDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable)
    : DomainAgent(executionContextID, messageCallback, objTable),
      runtime_(runtime) {}

void ProfilerDomainAgent::start(const m::profiler::StartRequest &req) {
  HermesRuntime::enableSamplingProfiler();
  sendResponseToClient(m::makeOkResponse(req.id));
}

void ProfilerDomainAgent::stop(const m::profiler::StopRequest &req) {
  HermesRuntime::disableSamplingProfiler();

  std::ostringstream profileStream;
  runtime_.sampledTraceToStreamInDevToolsFormat(profileStream);

  // Hermes can emit the proper format directly, but it still needs to
  // be parsed into a dynamic.
  try {
    m::profiler::StopResponse resp;
    resp.id = req.id;
    auto profile = m::profiler::makeProfile(std::move(profileStream).str());
    if (profile == nullptr) {
      throw std::runtime_error("Failed to make Profile");
    }
    resp.profile = std::move(*profile);
    sendResponseToClient(resp);
  } catch (const std::exception &) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InternalError,
        "Hermes profile output could not be parsed."));
  }
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
