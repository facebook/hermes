/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSPARSER_H
#define HERMES_PARSER_JSPARSER_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Parser/JSLexer.h"

#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"

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
  explicit JSParser(std::shared_ptr<detail::JSParserImpl> &&impl)
      : impl_(std::move(impl)) {}

 public:
  explicit JSParser(
      Context &context,
      std::unique_ptr<llvh::MemoryBuffer> input);

  explicit JSParser(Context &context, uint32_t bufferId, ParserPass pass);

  JSParser(Context &context, llvh::StringRef input)
      : JSParser(
            context,
            llvh::MemoryBuffer::getMemBuffer(input, "JavaScript")) {}

  JSParser(Context &context, llvh::MemoryBufferRef input)
      : JSParser(context, llvh::MemoryBuffer::getMemBuffer(input)) {}

  ~JSParser();

  Context &getContext();

  bool isStrictMode() const;

  void setStrictMode(bool mode);

  /// \return the source URL from the magic comment, or an empty string if there
  /// was no magic comment.
  llvh::StringRef getSourceURL() const;

  /// \return the source mapping URL from the magic comment, or an empty string
  /// if there was no magic comment.
  llvh::StringRef getSourceMappingURL() const;

  /// Define bitmask flags for magic comments.
  struct MCFlag {
    using Type = uint8_t;
    enum : Type {
      SourceURL = 1 << 0,
      SourceMappingURL = 1 << 1,
      All = SourceURL | SourceMappingURL,
    };
  };
  /// Apply the values of sourceURL and sourceMappingURL magic comments to the
  /// SourceErrorManager in the associated Context.
  /// \param flags a bitmask of \c MCFlag values.
  void registerMagicURLs(MCFlag::Type flags = MCFlag::All) const;

  llvh::ArrayRef<StoredComment> getStoredComments() const;

  std::vector<StoredComment> moveStoredComments() const;

  llvh::ArrayRef<StoredToken> getStoredTokens() const;

  void setStoreComments(bool storeComments);

  void setStoreTokens(bool storeTokens);

  /// Return true if the parser detected 'use static builtin' directive from the
  /// source.
  bool getUseStaticBuiltin() const;

  llvh::Optional<ESTree::ProgramNode *> parse();

  void seek(SMLoc startPos);

  /// Parse the given buffer id, indexing all functions and storing them in the
  /// \p Context. On failure returns nullptr.
  /// On success, returns a pointer to the \c JSParser object that can be
  /// queried for various attributes of the just pre-parsed file, e.g. static
  /// builtins or magic URLs.
  static std::unique_ptr<JSParser> preParseBuffer(
      Context &context,
      uint32_t bufferId);

  /// Parse the AST of a specified function type at a given starting point.
  /// This is used for lazy compilation to parse and compile the function on
  /// the first call.
  llvh::Optional<ESTree::NodePtr> parseLazyFunction(
      ESTree::NodeKind kind,
      bool paramYield,
      bool paramAwait,
      SMLoc start);

 private:
  /// Self-explanatory.
  /// Needs to be std::shared_ptr because of \c JSParserImpl::preParseBuffer().
  std::shared_ptr<detail::JSParserImpl> const impl_;
};

} // namespace parser
} // namespace hermes

#endif // HERMES_PARSER_JSPARSER_H
