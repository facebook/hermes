/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/TraverseLiteralStrings.h"

#include <functional>

namespace hermes {
namespace hbc {

void traverseLiteralStrings(
    Module *M,
    bool includeFunctionNames,
    std::function<bool(Function *)> shouldVisitFunction,
    std::function<void(llvm::StringRef)> traversal) {
  // Walk declared global properties.
  for (auto *prop : M->getGlobalProperties()) {
    if (prop->isDeclared()) {
      traversal(prop->getName()->getValue().str());
    }
  }

  // Walk function names.
  if (includeFunctionNames) {
    for (auto &F : *M) {
      if (shouldVisitFunction(&F)) {
        traversal(F.getOriginalOrInferredName().str());
      }
    }
  }

  // Walk function operands.
  for (auto &F : *M) {
    if (!shouldVisitFunction(&F)) {
      continue;
    }

    for (auto &BB : F) {
      for (auto &I : BB) {
        for (int i = 0, e = I.getNumOperands(); i < e; i++) {
          auto *op = I.getOperand(i);
          if (auto *str = dyn_cast<LiteralString>(op)) {
            traversal(str->getValue().str());
          }
        }
      }
    }
  }
}

} // namespace hbc
} // namespace hermes
