/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/PredefinedStringIDs.h"
#include "hermes/VM/Predefined.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/ADT/DenseMap.h"

namespace {

using namespace hermes::vm;
using StringIDMap = llvm::DenseMap<llvm::StringRef, SymbolID>;

StringIDMap createPredefinedStringSet() {
  StringIDMap predefined;

#define STR(name, string)                                                 \
  do {                                                                    \
    auto pid = Predefined::name;                                          \
    auto res = predefined.insert({string, Predefined::getSymbolID(pid)}); \
    assert(res.second && "Duplicate predefined string.");                 \
    (void)res;                                                            \
  } while (0);
#include "hermes/VM/PredefinedStrings.def"

  return predefined;
}

} // namespace

namespace hermes {
namespace hbc {

llvm::Optional<SymbolID> getPredefinedStringID(llvm::StringRef str) {
  static const auto predefined = createPredefinedStringSet();
  auto it = predefined.find(str);
  if (it == predefined.end()) {
    return {};
  }

  return it->second;
}

} // namespace hbc
} // namespace hermes
