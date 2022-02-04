/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_INSERTPROFILEPOINT_H
#define HERMES_BCGEN_HBC_PASSES_INSERTPROFILEPOINT_H

#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "llvh/Support/Casting.h"

namespace hermes {
class Instruction;
class BasicBlock;

namespace hbc {

/// Insert profile point at the beginning of each basic block.
class InsertProfilePoint : public FunctionPass {
 private:
  /// \returns the instruction in basic block \p BB which profile instruction
  /// will insert before. It needs to skip over certain instructions which
  /// are expected to be at the beginning of a block.
  Instruction *findInsertionPoint(BasicBlock &BB) const;

 public:
  explicit InsertProfilePoint() : FunctionPass("InsertProfilePoint") {}
  ~InsertProfilePoint() override = default;
  bool runOnFunction(Function *F) override;
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_PASSES_INSERTPROFILEPOINT_H
