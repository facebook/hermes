/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "stream_base.h"
#include "../RuntimeState.h"
#include "hermes/hermes.h"

#include "uv.h"

using namespace facebook;

class WriteContext {
 public:
  std::string data_;
  WriteContext(std::string data) {
    data_ = data;
  }
};

/// Callback for cleaning up the allocations made in streamBaseWriteUtf8String.
/// Necessary because uv_write can be async.
static void afterWrite(uv_write_t *req, int status) {
  WriteContext *context = reinterpret_cast<WriteContext *>(req->data);
  delete context;
  free(req);
}

int facebook::streamBaseWriteUtf8String(
    RuntimeState &rs,
    const jsi::Value &arg,
    uv_stream_t *stream) {
  jsi::Runtime &rt = rs.getRuntime();

  WriteContext *writeContext = new WriteContext(arg.toString(rt).utf8(rt));
  uv_write_t *writeReq = (uv_write_t *)malloc(sizeof(uv_write_t));
  writeReq->data = writeContext;
  uv_buf_t buf =
      uv_buf_init(&writeContext->data_[0], writeContext->data_.size());

  int err = uv_write(writeReq, stream, &buf, 1, afterWrite);
  return err;
}
