/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"
#include "hermes/hermes.h"
#include "jsi/jsi.h"

extern "C" SHERMES_EXPORT void init_console_bindings(SHRuntime *shr) {
  using namespace facebook;
  auto &hrt = *_sh_get_hermes_runtime(shr);
  // TOOD: Implement some bindings.
  (void)hrt;
}
