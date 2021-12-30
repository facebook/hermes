/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_WARNINGS_H
#define HERMES_SUPPORT_WARNINGS_H

#include "llvh/ADT/DenseMapInfo.h"

#include <type_traits>

namespace hermes {

/// Categories of warnings the compiler might emit, that can be turned on or
/// off.
enum class Warning {
#define WARNING_CATEGORY_HIDDEN(name, specifier, description) name,
#include "hermes/Support/Warnings.def"

  _NumWarnings,
};

} // namespace hermes

namespace llvh {

using hermes::Warning;

/// Information to store a \p Warning as the key of a \p DenseMap or \p
/// DensetSet
template <>
struct DenseMapInfo<Warning> {
  using Underlying = std::underlying_type<Warning>::type;
  using UnderlyingInfo = DenseMapInfo<Underlying>;

  static inline Warning getEmptyKey() {
    return static_cast<Warning>(UnderlyingInfo::getEmptyKey());
  }

  static inline Warning getTombstoneKey() {
    return static_cast<Warning>(UnderlyingInfo::getTombstoneKey());
  }

  static unsigned getHashValue(Warning warning) {
    return UnderlyingInfo::getHashValue(static_cast<Underlying>(warning));
  }

  static bool isEqual(Warning lhs, Warning rhs) {
    return lhs == rhs;
  }
};

} // namespace llvh

#endif // HERMES_SUPPORT_SOURCEERRORMANAGER_H
