/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SERIALIZEDLITERALPARSER_H
#define HERMES_VM_SERIALIZEDLITERALPARSER_H

#include "hermes/BCGen/HBC/SerializedLiteralParserBase.h"
#include "hermes/Support/Conversions.h"
#include "hermes/VM/HermesValue.h"
#include "llvh/ADT/ArrayRef.h"

namespace hermes {
namespace vm {

class Runtime;
class RuntimeModule;

/// SerializedLiteralParser is a parser that returns HermesValues
/// from Literals that have been serialized in a variable length format.
/// See SerializedLiteralGenerator.h for detailed format.
class SerializedLiteralParser : public hbc::SerializedLiteralParserBase {
  /// Stores the runtimeModule the code is running in.
  /// Used to generate String Primitives from IdentifierIDs.
  /// If the nullptr is passed as an argument, the generator knows to
  /// return IdentifierIDs as opposed to encoding strings.
  RuntimeModule *runtimeModule_;

 public:
  /// Creates a parser which generates HermesValues from a char buffer.
  /// buff represents the char buffer that will be parsed.
  /// totalLen represents the amount of elements to be parsed.
  /// runtimeModule represents the runtimeModule from which to get
  /// string primitives.
  ///   If the nullptr is passed instead of a runtimeModule, the parser
  ///   knows to return the StringID directly (for object keys)
  explicit SerializedLiteralParser(
      CharArray buff,
      unsigned int totalLen,
      RuntimeModule *runtimeModule)
      : SerializedLiteralParserBase(buff, totalLen),
        runtimeModule_(runtimeModule) {}

  /// Extract and return the next literal. Note: performs GC allocations.
  HermesValue get(Runtime &);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SERIALIZEDLITERALPARSER_H
