/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_LITERALBUFFERBUILDER_H
#define HERMES_BCGEN_LITERALBUFFERBUILDER_H

#include "hermes/BCGen/SerializedLiteralGenerator.h"
#include "hermes/BCGen/ShapeTableEntry.h"
#include "llvh/ADT/DenseMap.h"

namespace hermes {

class Instruction;
class Module;
class Function;

namespace LiteralBufferBuilder {

/// Offset information stored per object/array literal instruction.
struct LiteralOffset {
  /// Index into the shape table. Unused for array instructions.
  uint32_t shapeTableIdx;
  /// Byte offset into the value buffer.
  uint32_t valueBufferOffset;
};

using LiteralOffsetMapTy = llvh::DenseMap<const Instruction *, LiteralOffset>;

/// The LiteralBufferBuilder will build this struct as its output.
struct Result {
  std::vector<unsigned char> literalValBuffer;
  std::vector<unsigned char> keyBuffer;
  /// Contains the unique'd shapes of all object literals.
  std::vector<ShapeTableEntry> shapeTable;
  LiteralOffsetMapTy offsetMap;
};

/// Collect the literals, optionally deduplicate them, and generate the literal
/// buffers.
Result generate(
    Module *m,
    const std::function<bool(Function const *)> &shouldVisitFunction,
    const SerializedLiteralGenerator::StringLookupFn &getIdentifier,
    const SerializedLiteralGenerator::StringLookupFn &getString,
    bool optimize);
} // namespace LiteralBufferBuilder
} // namespace hermes

#endif // HERMES_BCGEN_LITERALBUFFERBUILDER_H
