/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_SHAPETABLEENTRY_H
#define HERMES_BCGEN_SHAPETABLEENTRY_H

#include <cstdint>

namespace hermes {

/// An entry in the shape table. A shape uniquely describes the sequence of
/// property keys of an object literal. It does so by encoding the beginning
/// position of the key sequence in the literal key buffer, and the number of
/// properties of the literal.
struct ShapeTableEntry {
  /// The number of bytes into the key buffer this shape begins.
  uint32_t keyBufferOffset;
  /// The number of properties in this shape.
  uint32_t numProps;
};

} // namespace hermes

#endif // HERMES_BCGEN_SHAPETABLEENTRY_H
