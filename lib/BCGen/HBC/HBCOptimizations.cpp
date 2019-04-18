/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/HBCOptimizations.h"
#include "hermes/BCGen/HBC/TraverseLiteralStrings.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"

using namespace hermes;
using namespace hbc;

ConsecutiveStringStorage hbc::getOrderedStringStorage(
    Module *M,
    const BytecodeGenerationOptions &options,
    std::function<bool(Function *)> shouldVisitFunction) {
  llvm::DenseMap<llvm::StringRef, int> stringFreqs{};
  traverseLiteralStrings(
      M,
      /* includeFunctionNames */ !options.stripFunctionNames,
      shouldVisitFunction,
      [&stringFreqs](llvm::StringRef str, bool) { stringFreqs[str]++; });

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
