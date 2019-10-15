/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSPARSER_H
#define HERMES_PARSER_JSPARSER_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"

#include "llvm/ADT/Optional.h"

namespace hermes {
namespace parser {

namespace detail {
class JSParserImpl;
}

/// The parser can operate in one of three modes defined here.
enum ParserPass {
  /// Parse and index the file's functions, without building an AST.
  PreParse,

  /// Parse parts of a file for lazy compilation, using the \p PreParse index
  /// to find and skip over functions.
  LazyParse,

  /// Completely parse the full file, for optimized ahead-of-time compilation.
  FullParse
};

/// An EcmaScript 5.1 parser.
class JSParser {
 public:
  explicit JSParser(
      Context &context,
      std::unique_ptr<llvm::MemoryBuffer> input);

  explicit JSParser(Context &context, uint32_t bufferId, ParserPass pass);

  JSParser(Context &context, StringRef input)
      : JSParser(
            context,
            llvm::MemoryBuffer::getMemBuffer(input, "JavaScript")) {}

  JSParser(Context &context, llvm::MemoryBufferRef input)
      : JSParser(context, llvm::MemoryBuffer::getMemBuffer(input)) {}

  ~JSParser();

  Context &getContext();

  bool isStrictMode() const;

  void setStrictMode(bool mode);

  /// Return true if the parser detected 'use static builtin' directive from the
  /// source.
  bool getUseStaticBuiltin() const;

  llvm::Optional<ESTree::ProgramNode *> parse();

  void seek(SMLoc startPos);

  /// Parse the given buffer id, indexing all functions and storing them in the
  /// \p Context. Returns true on success, at which point the file can be
  /// processed on demand in \p LazyParse mode. \p useStaticBuiltinDetected will
  /// be set to true if 'use static builtin' directive is detected in the
  /// source.
  static bool preParseBuffer(
      Context &context,
      uint32_t bufferId,
      bool &useStaticBuiltinDetected);

  /// Parse the AST of a specified function type at a given starting point.
  /// This is used for lazy compilation to parse and compile the function on
  /// the first call.
  llvm::Optional<ESTree::NodePtr> parseLazyFunction(
      ESTree::NodeKind kind,
      SMLoc start);

 private:
  /// Self-explanatory.
  std::unique_ptr<detail::JSParserImpl> const impl_;
};

}; // namespace parser
}; // namespace hermes

#endif // HERMES_PARSER_JSPARSER_H
