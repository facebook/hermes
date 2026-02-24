/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ContribDummy.h"

namespace facebook {
namespace hermes {

void installContribDummy(jsi::Runtime &rt, jsi::Object &extensions) {
  // Get the setup function from the precompiled extensions object
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "ContribDummy");

  // Call the setup function (no native helpers needed for ContribDummy)
  setup.call(rt);
}

} // namespace hermes
} // namespace facebook
