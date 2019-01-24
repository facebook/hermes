/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_PARSER_JSPARSER_H
#define HERMES_PARSER_JSPARSER_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/PreParser.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/DataTypes.h"

#include <utility>

namespace hermes {
namespace parser {

using llvm::ArrayRef;
using llvm::None;
using llvm::Optional;

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
/// It is a standard recursive descent LL(1) parser with no tricks. The only
/// complication, is the need to communicate information to the lexer whether
/// a regexp is allowed or not.
/// We go to some effort to avoid the need for more than one token lookahead
/// for performance. Some things (like recognizing a label) would have been
/// simplified with larger lookahead.
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

  Context &getContext() {
    return context_;
  }

  bool isStrictMode() const {
    return lexer_.isStrictMode();
  }

  void setStrictMode(bool mode) {
    lexer_.setStrictMode(mode);
  }

  Optional<ESTree::FileNode *> parse();

  void seek(SMLoc startPos) {
    lexer_.seek(startPos);
    tok_ = lexer_.advance();
  }

  /// Parse the given buffer id, indexing all functions and storing them in the
  /// \p Context. Returns true on success, at which point the file can be
  /// processed on demand in \p LazyParse mode.
  static bool preParseBuffer(Context &context, uint32_t bufferId);

  /// Parse the AST of a specified function type at a given starting point.
  /// This is used for lazy compilation to parse and compile the function on
  /// the first call.
  Optional<ESTree::NodePtr> parseLazyFunction(
      ESTree::NodeKind kind,
      SMLoc start);

 private:
  /// Called during construction to initialize Identifiers used for parsing,
  /// such as "var". The lexer and parser uses these to avoid passing strings
  /// around.
  void initializeIdentifiers();

  /// Current compilation context.
  Context &context_;
  /// Source error and buffer manager.
  SourceErrorManager &sm_;
  /// Source code lexer.
  JSLexer lexer_;
  /// Current token.
  const Token *tok_{};
  /// The current parser mode (see \p ParserPass).
  ParserPass pass_{FullParse};
  /// Function offsets. PreParse mode fills it in, LazyParse mode uses it
  /// to skip spans while parsing.
  PreParsedBufferInfo *preParsed_{nullptr};

  /// Track the parser recursion depth to avoid stack overflow.
  /// We don't have to track it precisely as long as we increment it once in
  /// every possible recursion cycle.
  unsigned recursionDepth_{0};

  /// Self-explanatory: the maximum depth of parser recursion.
  static constexpr unsigned MAX_RECURSION_DEPTH = 1024;

  // Certain known identifiers which we need to use when constructing the
  // ESTree.
  UniqueString *varIdent_;
  UniqueString *getIdent_;
  UniqueString *setIdent_;
  UniqueString *useStrictIdent_;
  /// String representation of all tokens.
  UniqueString *tokenIdent_[NUM_JS_TOKENS];

  UniqueString *getTokenIdent(TokenKind kind) const {
    return tokenIdent_[(unsigned)kind];
  }

  /// The $hermes:direct-use-only annotation.
  Identifier hermesOnlyDirectAccess_;
  /// The $hermes:init-once annotation.
  Identifier hermesInitOnce_;

  /// Allocate an ESTree node of a certain type with supplied location and
  /// construction arguments. All nodes are allocated using the supplied
  /// allocator.
  template <class Node, class StartLoc, class EndLoc>
  Node *setLocation(StartLoc start, EndLoc end, Node *node) {
    node->setStartLoc(getStartLoc(start));
    node->setEndLoc(getEndLoc(end));
    node->setDebugLoc(getStartLoc(start));
    return node;
  }

  /// Sets staart, end and debug lcoations of an ast node.
  template <class Node, class StartLoc, class EndLoc, class DebugLoc>
  Node *setLocation(StartLoc start, EndLoc end, DebugLoc debugLoc, Node *node) {
    node->setStartLoc(getStartLoc(start));
    node->setEndLoc(getEndLoc(end));
    node->setDebugLoc(getStartLoc(debugLoc));
    return node;
  }

  // A group of overrides to extract the start and end location of various
  // objects. The purpose is to allow flexibility when passing the location
  // information to setLocation(). We could pass existing nodes, locations,
  // tokens, or a combination of them.

  static SMLoc getStartLoc(const Token *tok) {
    return tok->getStartLoc();
  }
  static SMLoc getStartLoc(const ESTree::Node *from) {
    return from->getStartLoc();
  }
  static SMLoc getStartLoc(SMLoc loc) {
    return loc;
  }
  static SMLoc getStartLoc(const SMRange &rng) {
    return rng.Start;
  }

  static SMLoc getEndLoc(const Token *tok) {
    return tok->getEndLoc();
  }
  static SMLoc getEndLoc(const ESTree::Node *from) {
    return from->getEndLoc();
  }
  static SMLoc getEndLoc(SMLoc loc) {
    return loc;
  }
  static SMLoc getEndLoc(const SMRange &rng) {
    return rng.End;
  }

  /// Obtain the next token from the lexer and store it in tok_.
  /// \param grammarContext enable recognizing either "/" and "/=", or a regexp.
  /// \return the source location of the just consumed (previous) token.
  SMRange advance(
      JSLexer::GrammarContext grammarContext = JSLexer::AllowRegExp) {
    SMRange loc = tok_->getSourceRange();
    tok_ = lexer_.advance(grammarContext);
    return loc;
  }

  /// Report an error that one of the specified tokens was expected at the
  /// location of the current token.
  /// \param where (optional) If non-null, it is appended to the error message,
  ///           and is intended to explain the context in which these tokens
  ///           were expected (e.g. "after 'if'")
  /// \param what (optional) If not null, showen as an additional hint about the
  ///           error at the location specified by whatLoc. If whatLoc and the
  ///           current token are on the same line, `what` is not displayed but
  ///           the entire region between the location is emphasized.
  void errorExpected(
      ArrayRef<TokenKind> toks,
      const char *where,
      const char *what,
      SMLoc whatLoc);

  /// Convenience wrapper around errorExpected().
  void errorExpected(
      TokenKind k1,
      const char *where,
      const char *what,
      SMLoc whatLoc) {
    errorExpected(ArrayRef<TokenKind>(k1), where, what, whatLoc);
  }

  /// Convenience wrapper around errorExpected().
  void errorExpected(
      TokenKind k1,
      TokenKind k2,
      const char *where,
      const char *what,
      SMLoc whatLoc) {
    TokenKind toks[] = {k1, k2};
    errorExpected(ArrayRef<TokenKind>(toks, 2), where, what, whatLoc);
  };

  /// Check whether the current token is the specified one and report an error
  /// if it isn't. \returns false if it reported an error.
  /// The extra params \p where, \p what and \p whatLoc are optional and are
  /// documented in errorExpected().
  bool need(TokenKind kind, const char *where, const char *what, SMLoc whatLoc);

  /// Check whether the current token is the specified one and if it is, consume
  /// it, otherwise an report an error. \returns false if it reported an error.
  /// \param grammarContext enable recognizing either "/" and "/=", or a regexp
  ///     when consuming the next token.
  /// The extra params \p where, \p what and \p whatLoc are optional and are
  /// documented in errorExpected().
  bool eat(
      TokenKind kind,
      JSLexer::GrammarContext grammarContext,
      const char *where,
      const char *what,
      SMLoc whatLoc);

  /// Check whether the current token is the specified one and consume it if so.
  /// \returns true if the token matched.
  bool checkAndEat(TokenKind kind);
  /// Check whether the current token is the specified one. \returns true if it
  /// is.
  bool check(TokenKind kind) const {
    return tok_->getKind() == kind;
  }
  /// Check whether the current token is one of the specified ones. \returns
  /// true if it is.
  bool check(TokenKind kind1, TokenKind kind2) const {
    return tok_->getKind() == kind1 || tok_->getKind() == kind2;
  }
  /// Check whether the current token is an identifier or a reserved word and it
  /// is the same as the specified identifier.
  bool checkIdentifier(UniqueString *ident) const {
    return (tok_->getKind() == TokenKind::identifier || tok_->isResWord()) &&
        tok_->getResWordOrIdentifier() == ident;
  }

  template <typename T>
  inline bool checkN(T t) const {
    return tok_->getKind() == t;
  }
  /// Convenience function to compare against more than 2 token kinds. We still
  /// use check() for 2 or 1 kinds because it is more typesafe.
  template <typename Head, typename... Tail>
  inline bool checkN(Head h, Tail... tail) const {
    return tok_->getKind() == h || checkN(tail...);
  }

  /// Check whether the current token is an assignment operator.
  bool checkAssign() const;

  /// Performs automatic semicolon insertion and optionally reports an error
  /// if a semicolon is missing and cannot be inserted.
  /// \param endLoc is the previous end location before the semi. It will be
  ///               updated with the location of the semi, if present.
  /// \param optional if set to true, an error will not be reported.
  /// \return false if a semi was not found and it could not be inserted.
  bool eatSemi(SMLoc &endLoc, bool optional = false);

  /// Process a directive by updating internal flags appropriately. Currently
  /// we only care about "use strict".
  void processDirective(UniqueString *directive);

  /// Check whether the recursion depth has been exceeded, and if so generate
  /// and error and return true.
  bool recursionDepthExceeded();

  // Parser functions. All of these correspond more or less directly to grammar
  // productions, except in cases where the grammar is ambiguous, but even then
  // the name should be self-explanatory.
  // The "noIn" parameter allows us to implement the "...NoIn" productions in
  // the grammar without code duplication. Ultimately it controls whether
  // parseRelationalExpression() recognizes the "in" operator or not. The flag
  // is passed down along the chain of productions until
  // parseRelationalExpression().
  // Note that some expression parsong functions are actually duplicated into a
  // regular and a "...NoIn" form, instead of passing the flag, where it was
  // easy to do so.

  Optional<ESTree::FileNode *> parseProgram();
  /// Parse a function declaration, and optionally force an eager parse.
  /// Otherwise, the function will be skipped in lazy mode and a dummy returned.
  Optional<ESTree::FunctionDeclarationNode *> parseFunctionDeclaration(
      bool forceEagerly = false);
  Optional<ESTree::Node *> parseStatement();

  /// Parse a statement block.
  /// \param grammarContext context to be used when consuming the closing brace.
  /// \param parseDirectives if true, recognize directives in the beginning of
  ///   the block. Specifically, it will recognize "use strict" and enable
  ///   strict mode.
  Optional<ESTree::BlockStatementNode *> parseBlock(
      JSLexer::GrammarContext grammarContext = JSLexer::AllowRegExp,
      bool parseDirectives = false);

  /// Parse a function body. This is a wrapper around parseBlock for the
  /// purposes of lazy parsing.
  Optional<ESTree::BlockStatementNode *> parseFunctionBody(
      bool eagerly,
      JSLexer::GrammarContext grammarContext = JSLexer::AllowRegExp,
      bool parseDirectives = false);

  Optional<ESTree::VariableDeclarationNode *> parseVariableStatement();

  /// Parse a list of variable declarations. \returns a dummy value but the
  /// optionality still encodes the error condition.
  /// \param varLoc is the location of the `rw_var` token and is used for error
  /// display.
  Optional<const char *> parseVariableDeclarationList(
      SMLoc varLoc,
      ESTree::NodeList &declList,
      bool noIn = false);

  /// \param varLoc is the location of the `rw_var` token and is used for error
  /// display.
  Optional<ESTree::VariableDeclaratorNode *> parseVariableDeclaration(
      SMLoc varLoc,
      bool noIn = false);

  Optional<ESTree::EmptyStatementNode *> parseEmptyStatement();
  Optional<ESTree::Node *> parseExpressionOrLabelledStatement();
  Optional<ESTree::IfStatementNode *> parseIfStatement();
  Optional<ESTree::WhileStatementNode *> parseWhileStatement();
  Optional<ESTree::DoWhileStatementNode *> parseDoWhileStatement();
  Optional<ESTree::Node *> parseForStatement();
  Optional<ESTree::ContinueStatementNode *> parseContinueStatement();
  Optional<ESTree::BreakStatementNode *> parseBreakStatement();
  Optional<ESTree::ReturnStatementNode *> parseReturnStatement();
  Optional<ESTree::WithStatementNode *> parseWithStatement();
  Optional<ESTree::SwitchStatementNode *> parseSwitchStatement();
  Optional<ESTree::ThrowStatementNode *> parseThrowStatement();
  Optional<ESTree::TryStatementNode *> parseTryStatement();
  Optional<ESTree::DebuggerStatementNode *> parseDebuggerStatement();

  Optional<ESTree::Node *> parsePrimaryExpression();
  Optional<ESTree::ArrayExpressionNode *> parseArrayLiteral();
  Optional<ESTree::ObjectExpressionNode *> parseObjectLiteral();
  Optional<ESTree::Node *> parsePropertyAssignment();

  /// Parse a property key which is a string, number or identifier. If it is
  /// neither, reports an error.
  Optional<ESTree::Node *> parsePropertyKey();

  Optional<ESTree::FunctionExpressionNode *> parseFunctionExpression(
      bool forceEagerly = false);
  Optional<ESTree::Node *> parseMemberExpression();
  /// Returns a dummy Optional<> just to indicate success or failure like all
  /// other functions.
  Optional<const char *> parseArguments(
      ESTree::NodeList &argList,
      SMLoc &endLoc);

  /// \param objectLoc the location of the object part of the expression and is
  ///     used for error display.
  Optional<ESTree::Node *> parseMemberSelect(
      SMLoc objectLoc,
      ESTree::NodePtr expr);

  /// \param startLoc the start location of the expression, used for error
  ///     display.
  Optional<ESTree::Node *> parseCallExpression(
      SMLoc startLoc,
      ESTree::NodePtr expr);

  Optional<ESTree::Node *> parseNewExpression(bool &outWasNewExpression);
  Optional<ESTree::Node *> parseLeftHandSideExpression();
  Optional<ESTree::Node *> parsePostfixExpression();
  Optional<ESTree::Node *> parseUnaryExpression();

  /// Parse a binary expression using a precedence table, in order to decrease
  /// recursion depth.
  Optional<ESTree::Node *> parseBinaryExpression(bool noIn);

  Optional<ESTree::Node *> parseConditionalExpression(bool noIn = false);
  Optional<ESTree::Node *> parseAssignmentExpression(bool noIn = false);
  Optional<ESTree::Node *> parseExpression(bool noIn = false);

  /// If the current token can be recognised as a directive (ES5.1 14.1),
  /// process the directive and return the allocated directive statement.
  /// Note that this function never needs to returns an error.
  /// \return the allocated directive statement if this is a directive, or
  ///    null if it isn't.
  ESTree::ExpressionStatementNode *parseDirective();

  /// Allocate a binary expression node with the specified children and
  /// operator.
  inline ESTree::NodePtr
  newBinNode(ESTree::NodePtr left, TokenKind opKind, ESTree::NodePtr right) {
    UniqueString *opIdent = getTokenIdent(opKind);
    if (opKind == TokenKind::ampamp || opKind == TokenKind::pipepipe)
      return setLocation(
          left,
          right,
          new (context_) ESTree::LogicalExpressionNode(left, right, opIdent));
    else
      return setLocation(
          left,
          right,
          new (context_) ESTree::BinaryExpressionNode(left, right, opIdent));
  }

  /// RAII to save and restore the current setting of "strict mode".
  class SaveStrictMode {
    JSParser *const parser_;
    const bool oldValue_;

   public:
    SaveStrictMode(JSParser *parser)
        : parser_(parser), oldValue_(parser->isStrictMode()) {}
    ~SaveStrictMode() {
      parser_->setStrictMode(oldValue_);
    }
  };

  /// RAII to track the recursion depth.
  class TrackRecursion {
    JSParser *const parser_;

   public:
    TrackRecursion(JSParser *parser) : parser_(parser) {
      ++parser_->recursionDepth_;
    }
    ~TrackRecursion() {
      --parser_->recursionDepth_;
    }
  };
};

}; // namespace parser
}; // namespace hermes

#endif // HERMES_PARSER_JSPARSER_H
