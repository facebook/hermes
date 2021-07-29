/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "InternalBindings.h"
#include "hermes/hermes.h"

#include "uv.h"

using namespace facebook;

/// Adds the 'pipe_wrap' object as a property of internalBinding.
jsi::Value facebook::pipeBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object pipe_wrap{rt};

  jsi::Object constants{rt};
  constants.setProperty(rt, "SOCKET", (int)rs.SOCKET);
  constants.setProperty(rt, "SERVER", (int)rs.SERVER);
  constants.setProperty(rt, "IPC", (int)rs.IPC);
  constants.setProperty(rt, "UV_READABLE", (int)UV_READABLE);
  constants.setProperty(rt, "UV_WRITABLE", (int)UV_WRITABLE);
  pipe_wrap.setProperty(
      rt, jsi::String::createFromAscii(rt, "constants"), std::move(constants));

  rs.setInternalBindingProp("pipe_wrap", std::move(pipe_wrap));
  return rs.getInternalBindingProp("pipe_wrap");
}
