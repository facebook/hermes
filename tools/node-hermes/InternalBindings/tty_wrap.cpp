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

/// Adds the 'tty_wrap' object as a property of internalBinding.
jsi::Value facebook::ttyBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object tty_wrap{rt};

  rs.setInternalBindingProp("tty_wrap", std::move(tty_wrap));
  return rs.getInternalBindingProp("tty_wrap");
}
