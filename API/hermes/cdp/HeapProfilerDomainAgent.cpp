/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HeapProfilerDomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

HeapProfilerDomainAgent::HeapProfilerDomainAgent(
    int32_t executionContextID,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable)
    : DomainAgent(executionContextID, messageCallback, objTable) {}

} // namespace cdp
} // namespace hermes
} // namespace facebook
