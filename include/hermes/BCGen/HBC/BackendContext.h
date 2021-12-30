/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BACKENDCONTEXT_H
#define HERMES_BCGEN_HBC_BACKENDCONTEXT_H

#include "hermes/AST/Context.h"

namespace hermes {
namespace hbc {

class LowerBuiltinCallsContext;

/// Context encapsulating data shared inside the HBC backend.
class BackendContext {
  BackendContext();

 public:
  ~BackendContext();

  static BackendContext &get(Context &ctx);

  std::shared_ptr<LowerBuiltinCallsContext> lowerBuiltinCallsContext;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BACKENDCONTEXT_H
