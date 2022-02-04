/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BackendContext.h"

namespace hermes {
namespace hbc {

BackendContext::BackendContext() = default;
BackendContext::~BackendContext() = default;

BackendContext &BackendContext::get(Context &ctx) {
  if (!ctx.getHBCBackendContext())
    ctx.setHBCBackendContext(
        std::shared_ptr<BackendContext>{new BackendContext()});
  return *ctx.getHBCBackendContext();
};

} // namespace hbc
} // namespace hermes
