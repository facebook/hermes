/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ContribExtensions.h"

#include "ContribDummy.h"
// Add new contrib extension headers here.

namespace facebook {
namespace hermes {

void installContribExtensions(jsi::Runtime &rt, jsi::Object &extensions) {
  installContribDummy(rt, extensions);
  // Add new contrib extension install calls here.
}

} // namespace hermes
} // namespace facebook
