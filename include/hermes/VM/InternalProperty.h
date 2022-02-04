/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

/// Named or anonymous internal properties. These are filtered so the user never
/// sees them.
namespace InternalProperty {

/// Number of InternalPropertyX symbols defined.
#define PROP(i) +1
#define NAMED_PROP(name) +1
constexpr unsigned NumInternalProperties = 0
#include "hermes/VM/InternalProperties.def"
    ;

/// Number of anonymous InternalPropertyX symbols defined.
#define PROP(i) +1
#define NAMED_PROP(name) +0
constexpr unsigned NumAnonymousInternalProperties = 0
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
