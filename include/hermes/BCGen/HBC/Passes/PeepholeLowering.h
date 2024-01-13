/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_PEEPHOLELOWERING_H
#define HERMES_BCGEN_HBC_PASSES_PEEPHOLELOWERING_H

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {
namespace hbc {

class PeepholeLowering : public FunctionPass {
 public:
  explicit PeepholeLowering() : FunctionPass("PeepholeLowering") {}

  bool runOnFunction(Function *F) override;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_PASSES_INSERTPROFILEPOINT_H
