/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_RESERVEDSYMBOLIDS_H
#define HERMES_VM_RESERVEDSYMBOLIDS_H

#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

/// Reserved symbols that have a constant value and cannot be looked up by name.
namespace ReservedSymbolID {

/// An internal namespace mapping each symbol name to a consecutive number.
namespace Ordinal {
enum {
#define RESERVED_SYMBOL(x) x,
#include "hermes/VM/ReservedSymbolIDs.def"
  _LAST
};
} // namespace Ordinal

#define RESERVED_SYMBOL(x) \
  constexpr SymbolID x = SymbolID::unsafeCreate(Ordinal::x);
#include "hermes/VM/ReservedSymbolIDs.def"

constexpr unsigned NumSymbols = Ordinal::_LAST;
/// Number of InternalPropertyX symbols defined.
constexpr unsigned NumInternalProperties = 4;

inline bool isReserved(SymbolID id) {
  return id.unsafeGetIndex() < NumSymbols;
}

inline SymbolID internalProperty(unsigned i) {
  assert(i < NumInternalProperties && "Unsupported internal property index");
  return SymbolID::unsafeCreate(Ordinal::InternalProperty0 + i);
}

} // namespace ReservedSymbolID

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_RESERVEDSYMBOLIDS_H
