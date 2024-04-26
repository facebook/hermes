/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/inspector/chrome/MessageConverters.h>
#include <jsi/instrumentation.h>

#include "RuntimeDomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

RuntimeDomainAgent::RuntimeDomainAgent(
    HermesRuntime &runtime,
    SynchronizedOutboundCallback messageCallback)
    : DomainAgent(std::move(messageCallback)),
      runtime_(runtime),
      enabled_(false) {}

RuntimeDomainAgent::~RuntimeDomainAgent() {}

void RuntimeDomainAgent::enable(const m::runtime::EnableRequest &req) {
  if (enabled_) {
    // Can't enable twice without disabling
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Runtime domain already enabled"));
    return;
  }

  // Enable
  enabled_ = true;
  sendResponseToClient(m::makeOkResponse(req.id));

  // Notify the client about the hard-coded Hermes execution context.
  m::runtime::ExecutionContextCreatedNotification note;
  note.context.id = kHermesExecutionContextId;
  note.context.name = "hermes";
  sendNotificationToClient(note);
}

void RuntimeDomainAgent::disable(const m::runtime::DisableRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }
  enabled_ = false;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::getHeapUsage(
    const m::runtime::GetHeapUsageRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  auto heapInfo = runtime_.instrumentation().getHeapInfo(false);
  m::runtime::GetHeapUsageResponse resp;
  resp.id = req.id;
  resp.usedSize = heapInfo["hermes_allocatedBytes"];
  resp.totalSize = heapInfo["hermes_heapSize"];
  sendResponseToClient(resp);
}

bool RuntimeDomainAgent::checkRuntimeEnabled(const m::Request &req) {
  if (!enabled_) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "Runtime domain not enabled"));
    return false;
  }
  return true;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
