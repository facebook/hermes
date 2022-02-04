/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_FUNCCALLNOPTS_H
#define HERMES_BCGEN_HBC_PASSES_FUNCCALLNOPTS_H

#include "hermes/IR/IR.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Detect Calls with small argument counts, and replace them with HBCCallN.
class FuncCallNOpts : public FunctionPass {
 public:
  explicit FuncCallNOpts() : hermes::FunctionPass("FuncCallNOpts") {}
  ~FuncCallNOpts() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif
