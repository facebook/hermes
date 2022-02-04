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

//// A HostObject subclass to be used as a TTY wrapper object.
class TTYStreamWrap : public jsi::HostObject {
 public:
  uv_tty_t *getTTYStreamHandle() {
    return &ttyStreamHandle_;
  }

 private:
  // Handle used for tty libuv stream operations.
  uv_tty_t ttyStreamHandle_;
};

/// Creates a tty stream instance. Sets the error property in the second
/// argument if an error occurred. Sets the jsi::HostObject that is a wrapper
/// for the uv_tty_t stream handle on the returned jsi::Value.
static jsi::Value TTY(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 2) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into TTY initialization call.");
  }
  auto ttyStreamObject = std::make_shared<TTYStreamWrap>();
  uv_file fd = args[0].asNumber();
  int err = uv_tty_init(
      rs.getLoop(), ttyStreamObject.get()->getTTYStreamHandle(), fd, 0);
  if (err != 0)
    args[1].asObject(rt).setProperty(rt, "code", err);
  else
    thisValue.asObject(rt).setProperty(
        rt,
        rs.getTTYStreamPropId(),
        jsi::Object::createFromHostObject(rt, ttyStreamObject));

  return jsi::Value(rt, thisValue);
}

/// Checks whether the stream handle is a tty handle.
static jsi::Value isTTY(
    RuntimeState &rs,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 1) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into isTTY call.");
  }
  int fd = args[0].asNumber();
  bool rc = uv_guess_handle(fd) == UV_TTY;
  return rc;
}

/// Returns a pointer to the uv_tty_t stream object associated with the
/// instance of the tty object. Obtains the host object associated
/// with the stream to do this.
static uv_tty_t *getTTYStreamHandle(
    RuntimeState &rs,
    const jsi::Value &ttyObj,
    jsi::Runtime &rt) {
  auto ttyObject =
      ttyObj.asObject(rt).getProperty(rt, rs.getTTYStreamPropId()).asObject(rt);

  if (ttyObject.isHostObject(rt)) {
    auto ttyHostObject = ttyObject.getHostObject(rt);
    if (auto ttyStream = dynamic_cast<TTYStreamWrap *>(ttyHostObject.get()))
      return ttyStream->getTTYStreamHandle();
  }

  throw jsi::JSError(rt, "An invalid tty handle was provided.");
}

/// Returns the width and height of the terminal. Is a wrapper for
/// uv_tty_get_winsize.
static jsi::Value getWindowSize(
    RuntimeState &rs,
    const jsi::Value &thisValue,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 1) {
    throw jsi::JSError(
        rt, "Not enough arguments being passed into getWindowSize call.");
  }

  uv_tty_t *ttyStreamHandle = getTTYStreamHandle(rs, thisValue, rt);
  int width, height;
  int err = uv_tty_get_winsize(ttyStreamHandle, &width, &height);
  if (err == 0) {
    jsi::Array retArray = args[0].asObject(rt).asArray(rt);
    retArray.setValueAtIndex(rt, 0, width);
    retArray.setValueAtIndex(rt, 1, height);
  }
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
        "Not enough arguments being passed into tty writeUtf8String call.");

  uv_tty_t *ttyStreamHandle =
      getTTYStreamHandle(rs, thisValue, rs.getRuntime());

  return streamBaseWriteUtf8String(
      rs, args[1], reinterpret_cast<uv_stream_t *>(ttyStreamHandle));
}

/// Adds the 'tty_wrap' object as a property of internalBinding.
jsi::Value facebook::ttyBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object tty_wrap{rt};

  rs.defineJSFunction(TTY, "TTY", 2, tty_wrap);
  rs.defineJSFunction(isTTY, "isTTY", 1, tty_wrap);

  jsi::Object prototypeProp{rt};
  rs.defineJSFunction(getWindowSize, "getWindowSize", 1, prototypeProp);
  rs.defineJSFunction(writeUtf8String, "writeUtf8String", 1, prototypeProp);
  tty_wrap.getProperty(rt, "TTY").asObject(rt).setProperty(
      rt, "prototype", prototypeProp);

  rs.setInternalBindingProp("tty_wrap", std::move(tty_wrap));
  return rs.getInternalBindingProp("tty_wrap");
}
