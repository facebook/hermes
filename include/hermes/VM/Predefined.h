/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PREDEFINED_H
#define HERMES_VM_PREDEFINED_H

#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

namespace Predefined {

enum IProp {
#define PROP(i) InternalProperty##i,
#define NAMED_PROP(name) InternalProperty##name,
#include "InternalProperties.def"
  _IPROP_AFTER_LAST
};

enum Str {
  _STRING_BEFORE_FIRST = _IPROP_AFTER_LAST - 1,
#define STR(name, string) name,
#include "PredefinedStrings.def"
  _STRING_AFTER_LAST
};
constexpr size_t NumStrings = _STRING_AFTER_LAST - _STRING_BEFORE_FIRST - 1;

enum Sym {
  _SYMBOL_BEFORE_FIRST = _STRING_AFTER_LAST - 1,
#define SYM(name, desc) name,
#include "PredefinedSymbols.def"
  _SYMBOL_AFTER_LAST
};
constexpr size_t NumSymbols = _SYMBOL_AFTER_LAST - _SYMBOL_BEFORE_FIRST - 1;

/// \return a \c SymbolID of a predefined symbol.
constexpr SymbolID getSymbolID(IProp predefined) {
  return SymbolID::unsafeCreateNotUniqued(predefined);
}

constexpr SymbolID getSymbolID(Str predefined) {
  return SymbolID::unsafeCreate(predefined);
}

constexpr SymbolID getSymbolID(Sym predefined) {
  return SymbolID::unsafeCreateNotUniqued(predefined);
}

constexpr bool isPredefined(SymbolID sym) {
  return sym.unsafeGetIndex() < _SYMBOL_AFTER_LAST;
}

} // namespace Predefined

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PREDEFINED_H
