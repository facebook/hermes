/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CELLKIND_H
#define HERMES_VM_CELLKIND_H

#include <cstddef>
#include <type_traits>

namespace hermes {
namespace vm {

/// Define all cell kinds known to the garbage collector.
enum class CellKind {
#define CELL_KIND(name) name##Kind,
#define CELL_RANGE(rangeName, first, last) \
  rangeName##Kind_first = first##Kind, rangeName##Kind_last = last##Kind,
#include "hermes/VM/CellKinds.def"
};

/// \return true if the specified kind \p value is in the inclusive range
/// between \p from and \p to.
inline bool kindInRange(CellKind value, CellKind from, CellKind to) {
  return value >= from && value <= to;
}

/// \return whether a sequence of CellKinds is contiguous ascending
/// (like 3, 4, 5, 6).
constexpr bool cellKindsContiguousAscending(CellKind v1, CellKind v2) {
  using raw_t = typename std::underlying_type<CellKind>::type;
  return static_cast<raw_t>(v1) + 1 == static_cast<raw_t>(v2);
}

template <typename... T>
constexpr bool
cellKindsContiguousAscending(CellKind v1, CellKind v2, T... rest) {
  return cellKindsContiguousAscending(v1, v2) &&
      cellKindsContiguousAscending(v2, rest...);
}

const char *cellKindStr(CellKind kind);

static constexpr size_t kNumCellKinds = 0
#define CELL_KIND(name) +1
#include "hermes/VM/CellKinds.def"
    ;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CELLKIND_H
