/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HeapProfilerDomainAgent.h"
#include "CallbackOStream.h"

#include <hermes/cdp/MessageConverters.h>
#include <hermes/cdp/RemoteObjectConverters.h>
#include <jsi/instrumentation.h>

namespace facebook {
namespace hermes {
namespace cdp {

HeapProfilerDomainAgent::HeapProfilerDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable)
    : DomainAgent(executionContextID, messageCallback, objTable),
      runtime_(runtime) {}

/// Handles HeapProfiler.takeHeapSnapshot request
void HeapProfilerDomainAgent::takeHeapSnapshot(
    const m::heapProfiler::TakeHeapSnapshotRequest &req) {
  sendSnapshot(req.id, req.reportProgress && *req.reportProgress);
}

void HeapProfilerDomainAgent::sendSnapshot(int reqId, bool reportProgress) {
  if (reportProgress) {
    // A progress notification with finished = true indicates the
    // snapshot has been captured and is ready to be sent.  Our
    // implementation streams the snapshot as it is being captured,
    // so we must send this notification first.
    m::heapProfiler::ReportHeapSnapshotProgressNotification note;
    note.done = 1;
    note.total = 1;
    note.finished = true;
    sendNotificationToClient(note);
  }

  // The CallbackOStream buffers data and invokes the callback whenever
  // the chunk size is reached. It can also invoke the callback once more
  // upon destruction, emitting the final partially-filled chunk. Make sure
  // the stream goes out of scope and the final chunk is emitted before
  // sending the OK response.
  {
    // Size picked to match V8:
    // https://github.com/v8/v8/blob/45a5a44dd4397af6fdaee623f72999c8490cd8e3/src/inspector/v8-heap-profiler-agent-impl.cc#L93
    CallbackOStream cos(
        /* sz */ 100 << 10, [this](std::string s) {
          m::heapProfiler::AddHeapSnapshotChunkNotification note;
          note.chunk = std::move(s);
          sendNotificationToClient(note);
          return true;
        });

    runtime_.instrumentation().createSnapshotToStream(cos);
  }
  sendResponseToClient(m::makeOkResponse(reqId));
}

void HeapProfilerDomainAgent::getObjectByHeapObjectId(
    const m::heapProfiler::GetObjectByHeapObjectIdRequest &req) {
  uint64_t objID = atoi(req.objectId.c_str());
  jsi::Value val = runtime_.getObjectForID(objID);
  if (val.isNull()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::ServerError, "Unknown object"));
    return;
  }

  std::string group = req.objectGroup.value_or("");
  m::runtime::RemoteObject remoteObj = m::runtime::makeRemoteObject(
      runtime_, val, *objTable_, group, false, false);
  if (remoteObj.type.empty()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::ServerError, "Remote object is not available"));
    return;
  }

  m::heapProfiler::GetObjectByHeapObjectIdResponse resp;
  resp.id = req.id;
  resp.result = std::move(remoteObj);
  sendResponseToClient(resp);
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
