/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Dummy.h"

namespace facebook {
namespace hermes {

void installDummy(jsi::Runtime &rt, jsi::Object &extensions) {
  // Get the setup function from the precompiled extensions object
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "Dummy");

  // Call the setup function (no native helpers needed for Dummy)
  setup.call(rt);
}

} // namespace hermes
} // namespace facebook
