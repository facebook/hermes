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

/// Adds the 'util' object as a property of internalBinding.
jsi::Value facebook::utilBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object util{rt};

  rs.setInternalBindingProp("util", std::move(util));
  return rs.getInternalBindingProp("util");
}
