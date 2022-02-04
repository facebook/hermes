/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings.h"
#include "hermes/hermes.h"
#include "stream_base.h"

#include "uv.h"

using namespace facebook;

/// A HostObject subclass to be used as a Pipe wrapper object.
class PipeStreamWrap : public jsi::HostObject {
 public:
  uv_pipe_t *getPipeStreamHandle() {
    return &pipeStreamHandle_;
  }

 private:
  // Handle used for pipe libuv stream operations.
  uv_pipe_t pipeStreamHandle_;
};

/// Pipe constructor, is basically just a wrapper for uv_pipe_init.
static jsi::Value Pipe(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 1) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into Pipe initialization call.");
  }
  auto pipeStreamObject = std::make_unique<PipeStreamWrap>();
  int type = args[0].asNumber();
  bool ipc = type == RuntimeState::IPC;

  int err =
      uv_pipe_init(rs.getLoop(), pipeStreamObject->getPipeStreamHandle(), ipc);
  if (err != 0)
    throw jsi::JSError(rt, "uv_pipe_init call failed.");
  thisValue.asObject(rt).setProperty(
      rt,
      rs.getPipeStreamPropId(),
      jsi::Object::createFromHostObject(rt, std::move(pipeStreamObject)));
  return jsi::Value(rt, thisValue);
}

/// Returns a pointer to the uv_tty_t stream object associated with the
/// instance of the tty object. Obtains the host object associated
/// with the stream to do this.
static uv_pipe_t *getPipeStreamHandle(
    RuntimeState &rs,
    const jsi::Value &pipeObj,
    jsi::Runtime &rt) {
  auto pipeObject = pipeObj.asObject(rt)
                        .getProperty(rt, rs.getPipeStreamPropId())
                        .asObject(rt);

  if (pipeObject.isHostObject(rt)) {
    auto pipeHostObject = pipeObject.getHostObject(rt);
    if (auto pipeStream = dynamic_cast<PipeStreamWrap *>(pipeHostObject.get()))
      return pipeStream->getPipeStreamHandle();
  }

  throw jsi::JSError(rt, "An invalid pipe handle was provided.");
}

/// Opens the pipe stream associated with the object (thisValue).
static jsi::Value open(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 1) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into TTY initialization call.");
  }
  uv_pipe_t *pipeStreamHandle = getPipeStreamHandle(rs, thisValue, rt);
  int fd = args[0].asNumber();
  int err = uv_pipe_open(pipeStreamHandle, fd);
  return err;
}

/// Given a utf8 string to write, calls uv_buf_init and uv_write to write the
/// string to the stream stored in RuntimeState.
static jsi::Value writeUtf8String(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  if (count < 2)
    throw jsi::JSError(
        rs.getRuntime(),
        "Not enough arguments being passed into pipe writeUtf8String call.");
  uv_pipe_t *pipeStreamHandle =
      getPipeStreamHandle(rs, thisValue, rs.getRuntime());

  return streamBaseWriteUtf8String(
      rs, args[1], reinterpret_cast<uv_stream_t *>(pipeStreamHandle));
}

/// Adds the 'pipe_wrap' object as a property of internalBinding.
jsi::Value facebook::pipeBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object pipe_wrap{rt};

  rs.defineJSFunction(Pipe, "Pipe", 1, pipe_wrap);

  jsi::Object constants{rt};
  constants.setProperty(rt, "SOCKET", (int)rs.SOCKET);
  constants.setProperty(rt, "SERVER", (int)rs.SERVER);
  constants.setProperty(rt, "IPC", (int)rs.IPC);
  constants.setProperty(rt, "UV_READABLE", (int)UV_READABLE);
  constants.setProperty(rt, "UV_WRITABLE", (int)UV_WRITABLE);
  pipe_wrap.setProperty(
      rt, jsi::String::createFromAscii(rt, "constants"), std::move(constants));

  jsi::Object prototypeProp{rt};
  rs.defineJSFunction(open, "open", 1, prototypeProp);
  rs.defineJSFunction(writeUtf8String, "writeUtf8String", 1, prototypeProp);
  pipe_wrap.getProperty(rt, "Pipe")
      .asObject(rt)
      .setProperty(rt, "prototype", prototypeProp);

  rs.setInternalBindingProp("pipe_wrap", std::move(pipe_wrap));
  return rs.getInternalBindingProp("pipe_wrap");
}
