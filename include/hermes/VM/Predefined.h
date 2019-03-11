/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_PREDEFINED_H
#define HERMES_VM_PREDEFINED_H

namespace hermes {
namespace vm {

namespace Predefined {

enum IProp {
#define RESERVED_SYMBOL(x) x,
#include "ReservedSymbolIDs.def"
  _IPROP_AFTER_LAST
};

enum Str {
  _STRING_BEFORE_FIRST = _IPROP_AFTER_LAST - 1,
#define STR(name, string) name,
#include "PredefinedStrings.def"
  _STRING_AFTER_LAST
};

enum Sym {
  _SYMBOL_BEFORE_FIRST = _STRING_AFTER_LAST - 1,
#define SYM(name, desc) name,
#include "PredefinedSymbols.def"
  _SYMBOL_AFTER_LAST
};

} // namespace Predefined

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PREDEFINED_H
