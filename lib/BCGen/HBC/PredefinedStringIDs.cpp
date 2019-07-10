/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/PredefinedStringIDs.h"
#include "hermes/VM/Predefined.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/ADT/DenseMap.h"

#include <array>
#include <tuple>

namespace {

using namespace hermes::vm;
using StringIDMap = llvm::DenseMap<llvm::StringRef, SymbolID>;

StringIDMap createPredefinedStringSet() {
  namespace P = Predefined;
  constexpr static size_t NumPredefStrings =
      P::_STRING_AFTER_LAST - P::_STRING_BEFORE_FIRST - 1;

  static const char buffer[] =
#define STR(name, string) string
#include "hermes/VM/PredefinedStrings.def"
      ;

  static const uint8_t lengths[] = {
#define STR(name, string) sizeof(string) - 1,
#include "hermes/VM/PredefinedStrings.def"
  };

  static const Predefined::Str ids[] = {
#define STR(name, string) P::name,
#include "hermes/VM/PredefinedStrings.def"
  };

  StringIDMap predefined;

  assert(
      NumPredefStrings == sizeof(ids) / sizeof(Predefined::Str) &&
      "Mismatched count of predefined strings.");
  const char *chars = buffer;
  for (uint32_t i = 0; i < NumPredefStrings; chars += lengths[i++]) {
    auto res = predefined.try_emplace(
        {chars, lengths[i]}, Predefined::getSymbolID(ids[i]));
    assert(res.second && "Duplicate predefined string.");
    (void)res;
  }

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
