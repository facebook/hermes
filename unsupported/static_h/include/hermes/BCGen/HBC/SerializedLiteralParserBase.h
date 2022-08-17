/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_SERIALIZEDLITERALPARSERBASE_H
#define HERMES_BCGEN_HBC_SERIALIZEDLITERALPARSERBASE_H

#include "hermes/Support/Conversions.h"
#include "llvh/ADT/ArrayRef.h"

namespace hermes {
namespace hbc {

/// SerializedLiteralParserBase is the base class for SerializedLiteralParser
/// containing VM independent parsing logic.
class SerializedLiteralParserBase {
 protected:
  using CharArray = llvh::ArrayRef<unsigned char>;

  /// Stores the unsigned char buffer in which the literals are serialized.
  CharArray buffer_;

  /// Stores the elements left in the generator.
  unsigned int elemsLeft_;

  /// Stores the last sequence type.
  unsigned char lastTag_;

  /// Stores the amount of consecutive elements of the same type that are ahead
  /// of the current index.
  int leftInSeq_{0};

  /// Stores the current index into the buffer.
  int currIdx_{0};

  /// Called whenever the parser needs to read in the tag from the buffer
  ///   i.e. during the first call to get, and whenever leftInSeq_ == 0
  /// Checks the first byte (or two) of the buffer and treats it as a tag
  /// of the same format as outlined above.
  /// It sets `lastTag_` and `leftInSeq_`, and increments `currIdx_`
  /// to make it point to the next byte after the tag.
  void parseTagAndSeqLength();

 protected:
  /// Creates a parser which generates HermesValues from a char buffer.
  /// buff represents the char buffer that will be parsed.
  /// totalLen represents the amount of elements to be parsed.
  /// runtimeModule represents the runtimeModule from which to get
  /// string primitives.
  ///   If the nullptr is passed instead of a runtimeModule, the parser
  ///   knows to return the StringID directly (for object keys)
  explicit SerializedLiteralParserBase(CharArray buff, unsigned int totalLen)
      : buffer_(buff), elemsLeft_(totalLen) {}

 public:
  bool hasNext() const {
    return elemsLeft_ != 0;
  }
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_SERIALIZEDLITERALPARSERBASE_H
