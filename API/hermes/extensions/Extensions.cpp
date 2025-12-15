/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Extensions.h"

#include "Dummy.h"
#include "Intrinsics.h"
#include "TextDecoder.h"
#include "TextEncoder.h"

#if HERMES_ENABLE_CONTRIB_EXTENSIONS
#include "contrib/ContribExtensions.h"
#endif

#include "jsi/jsi.h"

namespace facebook {
namespace hermes {

void installExtensions(jsi::Runtime &rt, jsi::Object extensions) {
  // Capture intrinsics before any extension code runs.
  captureIntrinsics(rt);

  // Delegate to each extension's install function.
  installTextDecoder(rt, extensions);
  installTextEncoder(rt, extensions);
  installDummy(rt, extensions);

#if HERMES_ENABLE_CONTRIB_EXTENSIONS
  // Install community-contributed extensions.
  installContribExtensions(rt, extensions);
#endif
}

} // namespace hermes
} // namespace facebook
