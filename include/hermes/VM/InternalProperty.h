/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_INTERNALPROPERTY_H
#define HERMES_VM_INTERNALPROPERTY_H

#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

/// Anonymous internal propertes. Not part of the object map.
namespace InternalProperty {

/// An internal namespace mapping each symbol name to a consecutive number.
namespace Ordinal {
enum {
#define PROP(i) InternalProperty##i,
#include "hermes/VM/InternalProperties.def"
  _LAST
};
} // namespace Ordinal

#define PROP(i)                            \
  constexpr SymbolID InternalProperty##i = \
      SymbolID::unsafeCreate(Ordinal::InternalProperty##i);
#include "hermes/VM/InternalProperties.def"

constexpr unsigned NumSymbols = Ordinal::_LAST;
/// Number of InternalPropertyX symbols defined.
constexpr unsigned NumInternalProperties = 4;

inline bool isInternal(SymbolID id) {
  return id.unsafeGetIndex() < NumSymbols;
}

inline SymbolID getSymbolID(unsigned i) {
  assert(i < NumInternalProperties && "Unsupported internal property index");
  return SymbolID::unsafeCreateNotUniqued(Ordinal::InternalProperty0 + i);
}

} // namespace InternalProperty

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_INTERNALPROPERTY_H
