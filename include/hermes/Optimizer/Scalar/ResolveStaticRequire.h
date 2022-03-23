/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_RESOLVESTATICREQUIRE_H
#define HERMES_OPTIMIZER_SCALAR_RESOLVESTATICREQUIRE_H

#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Ensure that all CommonJS require() calls are statically resolvable.
class ResolveStaticRequire : public ModulePass {
 public:
  explicit ResolveStaticRequire() : ModulePass("ResolveStaticRequire") {}

  bool runOnModule(Module *M) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_RESOLVESTATICREQUIRE_H
