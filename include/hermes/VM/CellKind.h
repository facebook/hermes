/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_CELLKIND_H
#define HERMES_VM_CELLKIND_H

namespace hermes {
namespace vm {

/// Define all cell kinds known to the garbage collector.
enum class CellKind {
#define CELL_KIND(name, ...) name##Kind,
#define CELL_RANGE(rangeName, first, last) \
  rangeName##Kind_first = first##Kind, rangeName##Kind_last = last##Kind,
#include "hermes/VM/CellKinds.def"
};

/// \return true if the specified kind \p value is in the inclusive range
/// between \p from and \p to.
inline bool kindInRange(CellKind value, CellKind from, CellKind to) {
  return value >= from && value <= to;
}

const char *cellKindStr(CellKind kind);

}; // namespace vm
} // namespace hermes

#endif // HERMES_VM_CELLKIND_H
