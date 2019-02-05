/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/HBCOptimizations.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"

using namespace hermes;
using namespace hbc;

ConsecutiveStringStorage hbc::getOrderedStringStorage(
    Module *M,
    const BytecodeGenerationOptions &options) {
  llvm::DenseMap<llvm::StringRef, int> stringFreqs{};
  auto markStr = [&](llvm::StringRef str) { stringFreqs[str]++; };

  // Walk declared global properties.
  for (auto *prop : M->getGlobalProperties()) {
    if (prop->isDeclared()) {
      markStr(prop->getName()->getValue().str());
    }
  }

  // Walk function names.
  if (!options.stripFunctionNames) {
    for (auto &F : *M) {
      markStr(F.getOriginalOrInferredName().str());
    }
  }

  // Walk function operands.
  for (auto &F : *M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        for (int i = 0, e = I.getNumOperands(); i < e; i++) {
          auto *op = I.getOperand(i);
          if (auto *str = dyn_cast<LiteralString>(op)) {
            markStr(str->getValue().str());
          }
        }
      }
    }
  }

  std::vector<llvm::StringRef> sortedStrings;
  for (const auto &keyVal : stringFreqs) {
    sortedStrings.push_back(keyVal.first);
  }

  // Sort by most frequent strings first.
  // In case of a tie, sort in alphabetical order.
  // Since each string in sortedStrings is unique, the string representations
  // will never be equal.
  auto sortingFunction = [&](llvm::StringRef left, llvm::StringRef right) {
    if (stringFreqs[left] == stringFreqs[right]) {
      return left < right;
    } else {
      return stringFreqs[left] > stringFreqs[right];
    }
  };
  std::sort(sortedStrings.begin(), sortedStrings.end(), sortingFunction);
  return ConsecutiveStringStorage{
      sortedStrings,
      ConsecutiveStringStorage::OptimizeOrdering |
          ConsecutiveStringStorage::OptimizePacking};
}
