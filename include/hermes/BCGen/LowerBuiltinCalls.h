/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_LOWERBUILTINCALLS_H
#define HERMES_BCGEN_LOWERBUILTINCALLS_H

#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Detect calls to builtin methods like `Object.keys()` and replace them with
/// CallBuiltinInst.
class LowerBuiltinCalls : public FunctionPass {
 public:
  explicit LowerBuiltinCalls() : FunctionPass("LowerBuiltinCalls") {}

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_BCGEN_LOWERBUILTINCALLS_H
