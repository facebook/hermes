/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/BackendContext.h"

namespace hermes {

BackendContext::BackendContext() = default;
BackendContext::~BackendContext() = default;

BackendContext &BackendContext::get(Context &ctx) {
  if (!ctx.getBackendContext())
    ctx.setBackendContext(
        std::shared_ptr<BackendContext>{new BackendContext()});
  return *ctx.getBackendContext();
};

} // namespace hermes
