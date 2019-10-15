/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INTERNALPROPERTY_H
#define HERMES_VM_INTERNALPROPERTY_H

#include "hermes/VM/Predefined.h"
#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

/// Anonymous internal propertes. Not part of the object map.
namespace InternalProperty {

/// Number of InternalPropertyX symbols defined.
#define PROP(i) +1
constexpr unsigned NumInternalProperties = 0
#include "hermes/VM/InternalProperties.def"
    ;

inline bool isInternal(SymbolID id) {
  return id.unsafeGetIndex() < NumInternalProperties;
}

inline SymbolID getSymbolID(unsigned i) {
  assert(i < NumInternalProperties && "Unsupported internal property index");
  return Predefined::getSymbolID(
      static_cast<Predefined::IProp>(Predefined::InternalProperty0 + i));
}

} // namespace InternalProperty

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_INTERNALPROPERTY_H
