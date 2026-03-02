/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace hermes::vm {

/// The kind of object allocation being performed, controlling property flags.
enum class ObjectAllocKind : uint8_t {
  /// Normal untyped allocation.
  Untyped,
  /// Typed allocation with enumerable properties (class instances).
  TypedEnumerable,
  /// Typed allocation with non-enumerable properties (home objects/vtables).
  TypedNonEnumerable,
};

/// \return true if the allocation kind is typed (either enumerable or not).
inline bool isTypedAllocKind(ObjectAllocKind k) {
  return k != ObjectAllocKind::Untyped;
}

} // namespace hermes::vm
