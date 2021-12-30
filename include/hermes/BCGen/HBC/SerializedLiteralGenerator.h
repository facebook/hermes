/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_SERIALIZEDLITERALGENERATOR_H
#define HERMES_BCGEN_HBC_SERIALIZEDLITERALGENERATOR_H

#include "llvh/ADT/ArrayRef.h"

#include <cstdint>

namespace hermes {
class Literal;

namespace hbc {

class BytecodeModuleGenerator;

/// Generator of literal value buffers for object/array literals.
/// SerializedLiteralParser.h is responsible to decode the serialized literal
/// buffer.
///
/// The format is designed to represent sequences of similarly typed Literals in
/// as small a representation as possible; it is therfore both variable length
/// and has a variable tag length.
///
/// If the sequence of similarly typed Literals is shorter than 16 Literals, the
/// tag is as follows:
/// 0 ttt llll
/// | |   |
/// 7 6   3  0
///
/// t: Type of the sequence (Tags are defined in the class)
/// l: Length of the sequence
///
/// If the sequence of similarly typed Literals is longer than 15 Literals, the
/// tag is as follows:
/// 1  ttt  llll,llllllll
/// |  |    |
/// 15 14   11          0
///
/// t: Type of the sequence (Tags are defined in the class)
/// l: Length of the sequence
///
/// Following the tags are the values of the sequences.
/// Note that `true`, `false`, and `null` don't need extra bytes, and are
/// therefore generated just from reading the tag length and type.
/// Short strings (indices into the string table smaller than 2^16) take up two
/// bytes, doubles take up eight bytes, and integers and long strings take up
/// four bytes. All values are serialized in little-endian format.
class SerializedLiteralGenerator {
 private:
  /// The bytecode module generator.
  BytecodeModuleGenerator &BMGen_;
  /// Whether to perform de-duplication optimization or not.
  bool deDuplicate_;

 public:
  SerializedLiteralGenerator(BytecodeModuleGenerator &BMGen, bool deDuplicate)
      : BMGen_(BMGen), deDuplicate_(deDuplicate) {}

  using TagType = unsigned char;

  // Since undefined is very rarely used in literals in practice, replacing
  // it with a single byte string tag allows us to save even more space.
  static constexpr TagType NullTag = 0;
  static constexpr TagType TrueTag = 1 << 4;
  static constexpr TagType FalseTag = 2 << 4;
  static constexpr TagType NumberTag = 3 << 4;
  static constexpr TagType LongStringTag = 4 << 4;
  static constexpr TagType ShortStringTag = 5 << 4;
  static constexpr TagType ByteStringTag = 6 << 4;
  static constexpr TagType IntegerTag = 7 << 4;
  static constexpr TagType TagMask = 0x70;

  static constexpr unsigned SequenceMax = (1 << 12) - 1;

  /// Serialize input \p literals into \p buff.
  /// \p isKeyBuffer: whether this is generating object literal key buffer or
  /// not.
  uint32_t serializeBuffer(
      llvh::ArrayRef<Literal *> literals,
      std::vector<unsigned char> &buff,
      bool isKeyBuffer);
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_SERIALIZEDLITERALGENERATOR_H
