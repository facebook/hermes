/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_RUN_WASM
#ifndef HERMES_OPTIMIZER_EMITWASMINTRINSICS_H
#define HERMES_OPTIMIZER_EMITWASMINTRINSICS_H

#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Detect calls to unsafe intrinsics like `__uasm.add32()` and replace them
/// with CallIntrinsicsInst.
class EmitWasmIntrinsics : public FunctionPass {
 public:
  explicit EmitWasmIntrinsics() : FunctionPass("EmitWasmIntrinsics") {}

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_EMITWASMINTRINSICS_H
#endif // HERMES_RUN_WASM
