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
  auto lengths = hermes::hbc::predefStringLengths;
  const size_t NumPredefStrings = P::NumStrings;

  static const Predefined::Str ids[] = {
#define STR(name, string) P::name,
#include "hermes/VM/PredefinedStrings.def"
  };

  StringIDMap predefined;

  assert(
      NumPredefStrings == sizeof(ids) / sizeof(Predefined::Str) &&
      "Mismatched count of predefined strings.");
  const char *chars = hermes::hbc::predefStringAndSymbolChars.begin();
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

// Use local constexpr arrays to avoid complaint about global constructors.
static constexpr uint8_t _predefStringLengths[] = {
#define STR(name, string) sizeof(string) - 1,
#include "hermes/VM/PredefinedStrings.def"
};

static constexpr uint8_t _predefSymbolLengths[] = {
#define SYM(name, symbol) sizeof(symbol) - 1,
#include "hermes/VM/PredefinedSymbols.def"
};

const llvm::ArrayRef<uint8_t> predefSymbolLengths = _predefSymbolLengths;
const llvm::ArrayRef<uint8_t> predefStringLengths = _predefStringLengths;

const llvm::ArrayRef<char> predefStringAndSymbolChars =
// One buffer contains all strings.
// This ensures that all the strings live together in memory,
// and that we don't touch multiple separate pages on startup.
#define STR(name, string) string
#include "hermes/VM/PredefinedStrings.def"
#define SYM(name, symbol) symbol
#include "hermes/VM/PredefinedSymbols.def"
    ;

} // namespace hbc
} // namespace hermes
