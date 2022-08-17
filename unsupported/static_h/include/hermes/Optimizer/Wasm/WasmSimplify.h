/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_RUN_WASM
#ifndef HERMES_OPTIMIZER_WASM_WASMSIMPLIFY_H
#define HERMES_OPTIMIZER_WASM_WASMSIMPLIFY_H

#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Performs Asm.js/Wasm simplification.
class WasmSimplify : public FunctionPass {
 public:
  explicit WasmSimplify() : FunctionPass("WasmSimplify") {}
  ~WasmSimplify() override = default;

  bool runOnFunction(Function *F) override;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_WASM_WASMSIMPLIFY_H
#endif // HERMES_RUN_WASM
