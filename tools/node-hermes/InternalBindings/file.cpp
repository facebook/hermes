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

/// Adds the 'fs' object as a property of internalBinding.
jsi::Value facebook::fsBinding(RuntimeState &rs) {
  jsi::Runtime &rt = rs.getRuntime();
  jsi::Object fs{rt};

  jsi::String fsLabel = jsi::String::createFromAscii(rt, "fs");
  rs.setInternalBindingProp(fsLabel, std::move(fs));
  return rs.getInternalBindingProp(fsLabel);
}
