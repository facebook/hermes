/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings.h"
#include "hermes/hermes.h"

#include "uv.h"

using namespace facebook;

/// Returns the const char* equivalent of the uv_handle_type.
static const char *handleType(uv_handle_type t) {
  switch (t) {
    case UV_TCP:
      return "TCP";
    case UV_TTY:
      return "TTY";
    case UV_UDP:
      return "UDP";
    case UV_FILE:
      return "FILE";
    case UV_NAMED_PIPE:
      return "PIPE";
    case UV_UNKNOWN_HANDLE:
      return "UNKNOWN";
    default:
      llvm_unreachable("No other uv type exists.");
  }
}

/// Given a file descriptor, this function will return the type it is. This
/// function is just a wrapper for uv_guess_handle.
static jsi::Value guessHandleType(
    RuntimeState &rs,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  jsi::Runtime &rt = rs.getRuntime();
  if (count < 1) {
    throw jsi::JSError(
        rt,
        "Not enough arguments being passed into synchronous guessHandleType call.");
  }
  int fd = args[0].asNumber();
  uv_handle_type t = uv_guess_handle(fd);
  const char *type = handleType(t);

  return jsi::String::createFromAscii(rt, type);
}

/// Adds the 'util' object as a property of internalBinding.
jsi::Value facebook::utilBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object util{rt};

  rs.defineJSFunction(guessHandleType, "guessHandleType", 1, util);

  rs.setInternalBindingProp("util", std::move(util));
  return rs.getInternalBindingProp("util");
}
