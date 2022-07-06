/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSPARSERIMPL_H
#define HERMES_PARSER_JSPARSERIMPL_H

#include "hermes/AST/Config.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Parser/PreParser.h"
#include "hermes/Support/Compiler.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/DataTypes.h"

#include <utility>

namespace hermes {
namespace parser {
namespace detail {

using llvh::ArrayRef;
using llvh::None;
using llvh::Optional;

/// A convenience class to encapsulate passing multiple boolean parameters
/// between parser functions.
class Param {
  unsigned flags_;

 public:
  constexpr Param() : flags_(0) {}
  constexpr explicit Param(unsigned f) : flags_(f) {}

  constexpr Param operator+(Param b) const {
    return Param{flags_ | b.flags_};
  }
  constexpr Param operator-(Param b) const {
    return Param{flags_ & ~(b.flags_)};
  }

  /// \return true if any of the flags in \p p are set in this instance.
  bool has(Param p) const {
    return flags_ & p.flags_;
  }

  /// \return true if all of the flags in \p p are set in this instance.
  bool hasAll(Param p) const {
    return (flags_ & p.flags_) == p.flags_;
  }

  /// \return \p p if at least on of its bits is set in this instance,
  ///   otherwise returns an empty param.
  constexpr Param get(Param p) const {
    return Param{flags_ & p.flags_};
  }

  template <typename... T>
  constexpr Param get(Param p, T... tail) const {
    return Param{get(p).flags_ | get(tail...).flags_};
  }
};

/// If set, "in" is recognized as a binary operator in RelationalExpression.
static constexpr Param ParamIn{1 << 0};
static constexpr Param ParamReturn{1 << 1};
static constexpr Param ParamDefault{1 << 2};
static constexpr Param ParamTagged{1 << 3};

/// An EcmaScript 5.1 parser.
/// It is a standard recursive descent LL(1) parser with no tricks. The only
/// complication, is the need to communicate information to the lexer whether
/// a regexp is allowed or not.
/// We go to some effort to avoid the need for more than one token lookahead
/// for performance. Some things (like recognizing a label) would have been
/// simplified with larger lookahead.
class JSParserImpl {
 public:
  explicit JSParserImpl(
      Context &context,
      std::unique_ptr<llvh::MemoryBuffer> input);

  explicit JSParserImpl(Context &context, uint32_t bufferId, ParserPass pass);

  JSParserImpl(Context &context, StringRef input)
      : JSParserImpl(
            context,
            llvh::MemoryBuffer::getMemBuffer(input, "JavaScript")) {}

  JSParserImpl(Context &context, llvh::MemoryBufferRef input)
      : JSParserImpl(context, llvh::MemoryBuffer::getMemBuffer(input)) {}

  Context &getContext() {
    return context_;
  }

  bool isStrictMode() const {
    return lexer_.isStrictMode();
  }

  void setStrictMode(bool mode) {
    lexer_.setStrictMode(mode);
  }

  llvh::SmallVector<UniqueString *, 1> &getSeenDirectives() {
    return seenDirectives_;
  }

  /// Copy the seen directives from a vector of \c UniqueString* into a vector
  /// of \c SmallString to be stored in \c PreParseFunctionInfo safely.
  llvh::SmallVector<llvh::SmallString<24>, 1> copySeenDirectives() const;

  llvh::ArrayRef<StoredComment> getStoredComments() const {
    return lexer_.getStoredComments();
  }

  llvh::ArrayRef<StoredToken> getStoredTokens() const {
    return lexer_.getStoredTokens();
  }

  void setStoreComments(bool storeComments) {
    lexer_.setStoreComments(storeComments);
  }

  void setStoreTokens(bool storeTokens) {
    lexer_.setStoreTokens(storeTokens);
  }

  Optional<ESTree::ProgramNode *> parse();

  void seek(SMLoc startPos) {
    lexer_.seek(startPos);
    tok_ = lexer_.advance();
  }

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
  Optional<ESTree::NodePtr> parseLazyFunction(
      ESTree::NodeKind kind,
      bool paramYield,
      bool paramAwait,
      SMLoc start);

  /// Return true if the parser detected 'use static builtin' directive from the
  /// source.
  bool getUseStaticBuiltin() const {
    return useStaticBuiltin_;
  }

 private:
  /// Called when the parser detects 'use static builtin' directive from the
  /// source.
  void setUseStaticBuiltin() {
    useStaticBuiltin_ = true;
  }

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
  static constexpr unsigned MAX_RECURSION_DEPTH =
#ifdef HERMES_LIMIT_STACK_DEPTH
      128
#elif defined(_MSC_VER) && defined(HERMES_SLOW_DEBUG)
      128
#elif defined(_MSC_VER)
      512
#else
      1024
#endif
      ;

  /// Set when the parser sees the 'use static builtin' directive in any scope.
  bool useStaticBuiltin_{false};

  /// Set when the parser is inside a generator function.
  /// This is used when checking if `yield` is a valid Identifier name.
  bool paramYield_{false};

  /// Set when the parser is inside an async function.
  /// This is used when checking if `await` is a valid Identifier name.
  bool paramAwait_{false};

  /// Appended when the parser has seen an directive being visited in the
  /// current function scope (It's intended to be used with
  /// `SaveStrictModeAndSeenDirectives`).
  /// This is used to store directives for lazy functions in the preParse pass,
  /// so we can recover directive nodes back in the lazyParse pass.
  llvh::SmallVector<UniqueString *, 1> seenDirectives_{};

#if HERMES_PARSE_JSX
  /// Incremented when inside a JSX tag and decremented when leaving it.
  /// Used to know whether to lex JS values or JSX text.
  uint32_t jsxDepth_{0};
#endif

#if HERMES_PARSE_FLOW
  bool allowAnonFunctionType_{false};
#endif

  // Certain known identifiers which we need to use when constructing the
  // ESTree or when parsing;
  UniqueString *getIdent_;
  UniqueString *setIdent_;
  UniqueString *initIdent_;
  UniqueString *useStrictIdent_;
  UniqueString *showSourceIdent_;
  UniqueString *hideSourceIdent_;
  UniqueString *sensitiveIdent_;
  UniqueString *useStaticBuiltinIdent_;
  UniqueString *letIdent_;
  UniqueString *ofIdent_;
  UniqueString *fromIdent_;
  UniqueString *asIdent_;
  UniqueString *implementsIdent_;
  UniqueString *interfaceIdent_;
  UniqueString *packageIdent_;
  UniqueString *privateIdent_;
  UniqueString *protectedIdent_;
  UniqueString *publicIdent_;
  UniqueString *staticIdent_;
  UniqueString *methodIdent_;
  UniqueString *constructorIdent_;
  UniqueString *yieldIdent_;
  UniqueString *newIdent_;
  UniqueString *importIdent_;
  UniqueString *targetIdent_;
  UniqueString *metaIdent_;
  UniqueString *valueIdent_;
  UniqueString *typeIdent_;
  UniqueString *asyncIdent_;
  UniqueString *awaitIdent_;
  UniqueString *assertIdent_;

#if HERMES_PARSE_FLOW

  UniqueString *typeofIdent_;
  UniqueString *declareIdent_;
  UniqueString *protoIdent_;
  UniqueString *opaqueIdent_;
  UniqueString *plusIdent_;
  UniqueString *minusIdent_;
  UniqueString *moduleIdent_;
  UniqueString *exportsIdent_;
  UniqueString *esIdent_;
  UniqueString *commonJSIdent_;
  UniqueString *mixinsIdent_;
  UniqueString *thisIdent_;

  UniqueString *anyIdent_;
  UniqueString *mixedIdent_;
  UniqueString *emptyIdent_;
  UniqueString *booleanIdent_;
  UniqueString *boolIdent_;
  UniqueString *numberIdent_;
  UniqueString *stringIdent_;
  UniqueString *voidIdent_;
  UniqueString *nullIdent_;
  UniqueString *symbolIdent_;

  UniqueString *checksIdent_;

#endif

#if HERMES_PARSE_TS
  UniqueString *namespaceIdent_;
  UniqueString *readonlyIdent_;
  UniqueString *isIdent_;
#endif

  /// String representation of all tokens.
  UniqueString *tokenIdent_[NUM_JS_TOKENS];

  UniqueString *getTokenIdent(TokenKind kind) const {
    return tokenIdent_[(unsigned)kind];
  }

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

  SMLoc getPrevTokenEndLoc() const {
    return lexer_.getPrevTokenEndLoc();
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

  /// Report an error to the SourceErrorManager.
  void error(SMLoc loc, const llvh::Twine &message) {
    sm_.error(loc, message, Subsystem::Parser);
  }

  /// Report an error to the SourceErrorManager.
  void error(SMRange range, const llvh::Twine &message) {
    sm_.error(range, message, Subsystem::Parser);
  }

  /// Report an error using the current token's location.
  void error(const llvh::Twine &msg) {
    error(tok_->getSourceRange(), msg);
  }

  /// Emit an error at the specified source location and range. If the maximum
  /// number of errors has been reached, return false and move the scanning
  /// pointer to EOF.
  /// \return false if too many errors have been emitted and we need to abort.
  bool error(SMLoc loc, SMRange range, const llvh::Twine &msg);

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
  bool checkAndEat(
      TokenKind kind,
      JSLexer::GrammarContext grammarContext =
          JSLexer::GrammarContext::AllowRegExp);
  /// Check whether the current token is the specified identifier and consume it
  /// if so. \returns true if the token matched.
  bool checkAndEat(
      UniqueString *ident,
      JSLexer::GrammarContext grammarContext =
          JSLexer::GrammarContext::AllowRegExp);
  /// Check whether the current token is the specified one. \returns true if it
  /// is.
  bool check(TokenKind kind) const {
    return tok_->getKind() == kind;
  }
  /// \return true if the current token is the specified identifier.
  bool check(UniqueString *ident) const {
    return tok_->getKind() == TokenKind::identifier &&
        tok_->getIdentifier() == ident;
  }
  /// Check whether the current token is one of the specified ones. \returns
  /// true if it is.
  bool check(TokenKind kind1, TokenKind kind2) const {
    return tok_->getKind() == kind1 || tok_->getKind() == kind2;
  }

  template <typename T>
  inline bool checkN(T t) const {
    return check(t);
  }
  /// Convenience function to compare against more than 2 token kinds. We still
  /// use check() for 2 or 1 kinds because it is more typesafe.
  template <typename Head, typename... Tail>
  inline bool checkN(Head h, Tail... tail) const {
    return check(h) || checkN(tail...);
  }

  /// Check whether the current token is an assignment operator.
  bool checkAssign() const;

  /// Check whether the current token begins a Declaration.
  bool checkDeclaration() {
    if (checkN(
            TokenKind::rw_function,
            letIdent_,
            TokenKind::rw_const,
            TokenKind::rw_class) ||
        (check(asyncIdent_) && checkAsyncFunction())) {
      return true;
    }

#if HERMES_PARSE_FLOW
    if (context_.getParseFlow()) {
      if (check(opaqueIdent_)) {
        auto optNext = lexer_.lookahead1(llvh::None);
        return optNext.hasValue() && (*optNext == TokenKind::identifier);
      }
      if (checkN(typeIdent_, interfaceIdent_)) {
        auto optNext = lexer_.lookahead1(llvh::None);
        return optNext.hasValue() && *optNext == TokenKind::identifier;
      }
      if (check(TokenKind::rw_interface)) {
        return true;
      }
      if (check(TokenKind::rw_enum)) {
        return true;
      }
    }
#endif

#if HERMES_PARSE_TS
    if (context_.getParseTS()) {
      if (checkN(typeIdent_, interfaceIdent_, namespaceIdent_)) {
        auto optNext = lexer_.lookahead1(llvh::None);
        return optNext.hasValue() && *optNext == TokenKind::identifier;
      }
      if (check(TokenKind::rw_interface)) {
        return true;
      }
      if (check(TokenKind::rw_enum)) {
        return true;
      }
    }
#endif

    return false;
  }

  bool checkDeclareType() {
#if HERMES_PARSE_FLOW
    if (check(declareIdent_)) {
      auto optNext = lexer_.lookahead1(llvh::None);
      if (!optNext)
        return false;
      TokenKind next = *optNext;
      return next == TokenKind::identifier || next == TokenKind::rw_interface ||
          next == TokenKind::rw_var || next == TokenKind::rw_function ||
          next == TokenKind::rw_class || next == TokenKind::rw_export;
    }
#endif
    return false;
  }

  /// Check whether the current token begins a template literal.
  bool checkTemplateLiteral() const {
    return check(TokenKind::no_substitution_template, TokenKind::template_head);
  }

  /// Check whether the current token can be the token after the end of an
  /// AssignmentExpression.
  bool checkEndAssignmentExpression() const;

  /// Check whether we match 'async [no LineTerminator here] function'.
  /// \pre the current token is 'async'.
  bool checkAsyncFunction();

  /// Performs automatic semicolon insertion and optionally reports an error
  /// if a semicolon is missing and cannot be inserted.
  /// \param optional if set to true, an error will not be reported.
  /// \return false if a semi was not found and it could not be inserted.
  bool eatSemi(bool optional = false);

  /// Process a directive by updating internal flags appropriately. Currently
  /// we only care about "use strict".
  void processDirective(UniqueString *directive);

  /// Check whether the recursion depth has been exceeded, and if so generate
  /// and error and return true.
  /// If the depth has not been exceeded return false.
  /// NOTE: This is intended to stay inline to avoid a function call unless the
  /// depth was actually exceeded.
  inline bool recursionDepthCheck() {
    if (LLVM_LIKELY(recursionDepth_ < MAX_RECURSION_DEPTH)) {
      return false;
    }
    return recursionDepthExceeded();
  }

  /// Generate an error and return true.
  bool recursionDepthExceeded();

  // Parser functions. All of these correspond more or less directly to grammar
  // productions, except in cases where the grammar is ambiguous, but even then
  // the name should be self-explanatory.

  Optional<ESTree::ProgramNode *> parseProgram();
  /// Parse a function declaration, and optionally force an eager parse.
  /// Otherwise, the function will be skipped in lazy mode and a dummy returned.
  /// \param param [Yield]
  Optional<ESTree::FunctionDeclarationNode *> parseFunctionDeclaration(
      Param param,
      bool forceEagerly = false);

  /// Parse a function declaration or expression, and optionally force an eager
  /// parse. Otherwise, the function will be skipped in lazy mode and a dummy
  /// returned.
  /// \param param [Yield, Default]
  Optional<ESTree::FunctionLikeNode *> parseFunctionHelper(
      Param param,
      bool isDeclaration,
      bool forceEagerly = false);

  /// Parse FormalParameters or UniqueFormalParameters with the leading '(' and
  /// the trailing ')'.
  /// \pre the current token must be '('.
  /// \param[out] paramList populated with the FormalParameters.
  /// \return true on success, false on failure.
  bool parseFormalParameters(Param param, ESTree::NodeList &paramList);

  /// \param param [Yield, Return]
  Optional<ESTree::Node *> parseStatement(Param param);

  enum class AllowImportExport {
    No,
    Yes,
  };

  /// Parse a statement list.
  /// \param param [Yield]
  /// \param until stop parsing when this token is enountered
  /// \param parseDirectives if true, recognize directives in the beginning of
  ///   the block. Specifically, it will recognize "use strict" and enable
  ///   strict mode.
  /// \return a dummy value for consistency.
  template <typename... Tail>
  Optional<bool> parseStatementList(
      Param param,
      TokenKind until,
      bool parseDirectives,
      AllowImportExport allowImportExport,
      ESTree::NodeList &stmtList,
      Tail... otherUntil);

  bool parseStatementListItem(
      Param param,
      AllowImportExport allowImportExport,
      ESTree::NodeList &stmtList);

  /// Parse a statement block.
  /// \param param [Yield, Return]
  /// \param grammarContext context to be used when consuming the closing brace.
  /// \param parseDirectives if true, recognize directives in the beginning of
  ///   the block. Specifically, it will recognize "use strict" and enable
  ///   strict mode.
  Optional<ESTree::BlockStatementNode *> parseBlock(
      Param param,
      JSLexer::GrammarContext grammarContext = JSLexer::AllowRegExp,
      bool parseDirectives = false);

  /// Parse a function body. This is a wrapper around parseBlock for the
  /// purposes of lazy parsing.
  /// \param paramYield the value of paramYield at the start of the function,
  ///   used for lazy compilation.
  /// \param paramAwait the value of paramAwait at the start of the function,
  ///   used for lazy compilation.
  /// \param param [Yield]
  Optional<ESTree::BlockStatementNode *> parseFunctionBody(
      Param param,
      bool eagerly,
      bool paramYield,
      bool paramAwait,
      JSLexer::GrammarContext grammarContext = JSLexer::AllowRegExp,
      bool parseDirectives = false);

  /// Parse a declaration.
  /// \param param [Yield]
  Optional<ESTree::Node *> parseDeclaration(Param param);

  /// Check if the provided string is a valid binding identifier.
  /// Can be used to validate identifiers after we've passed lexing them.
  /// The caller must report any errors if this function returns false.
  /// \param param [Yield]
  /// \param range the source range of the identifier to validate.
  /// \param id the string to be validated.
  /// \param kind the TokenKind provided when the string was lexed.
  /// \return true if \p id is a valid binding identifier.
  bool validateBindingIdentifier(
      Param param,
      SMRange range,
      UniqueString *id,
      TokenKind kind);

  /// ES 2015 12.1
  /// Does not generate an error. It is expected that the caller will do it.
  /// \param param [Yield]
  Optional<ESTree::IdentifierNode *> parseBindingIdentifier(Param param);
  /// Parse a VariableStatement or LexicalDeclaration.
  /// \param param [In, Yield]
  Optional<ESTree::VariableDeclarationNode *> parseLexicalDeclaration(
      Param param);
  /// Parse a VariableStatement or LexicalDeclaration.
  /// \param param [Yield]
  Optional<ESTree::VariableDeclarationNode *> parseVariableStatement(
      Param param);

  /// Parse a PrivateName starting with the '#'.
  Optional<ESTree::PrivateNameNode *> parsePrivateName();

  /// Parse a list of variable declarations. \returns a dummy value but the
  /// optionality still encodes the error condition.
  /// \param param [In, Yield]
  /// \param declLoc is the location of the `rw_var` token and is used for error
  /// display.
  Optional<const char *> parseVariableDeclarationList(
      Param param,
      ESTree::NodeList &declList,
      SMLoc declLoc);

  /// \param param [In, Yield]
  /// \param declLoc is the location of the let/var/const token and is used for
  /// error display.
  Optional<ESTree::VariableDeclaratorNode *> parseVariableDeclaration(
      Param param,
      SMLoc declLoc);

  /// Ensure that all destructuring declarations in the specified declaration
  /// node are initialized and report errors if they are not.
  void ensureDestructuringInitialized(
      ESTree::VariableDeclarationNode *declNode);

  Optional<ESTree::Node *> parseBindingPattern(Param param);
  Optional<ESTree::ArrayPatternNode *> parseArrayBindingPattern(Param param);
  Optional<ESTree::Node *> parseBindingElement(Param param);
  Optional<ESTree::Node *> parseBindingRestElement(Param param);
  /// Parse "'=' Initializer" in a binding pattern.
  Optional<ESTree::AssignmentPatternNode *> parseBindingInitializer(
      Param param,
      ESTree::Node *left);
  Optional<ESTree::ObjectPatternNode *> parseObjectBindingPattern(Param param);
  Optional<ESTree::PropertyNode *> parseBindingProperty(Param param);
  Optional<ESTree::Node *> parseBindingRestProperty(Param param);

  Optional<ESTree::EmptyStatementNode *> parseEmptyStatement();
  /// \param param [Yield, Return]
  Optional<ESTree::Node *> parseExpressionOrLabelledStatement(Param param);
  /// \param param [Yield, Return]
  Optional<ESTree::IfStatementNode *> parseIfStatement(Param param);
  /// \param param [Yield, Return]
  Optional<ESTree::WhileStatementNode *> parseWhileStatement(Param param);
  /// \param param [Yield, Return]
  Optional<ESTree::DoWhileStatementNode *> parseDoWhileStatement(Param param);
  /// \param param [Yield, Return]
  Optional<ESTree::Node *> parseForStatement(Param param);
  Optional<ESTree::ContinueStatementNode *> parseContinueStatement();
  Optional<ESTree::BreakStatementNode *> parseBreakStatement();
  Optional<ESTree::ReturnStatementNode *> parseReturnStatement();
  /// \param param [Yield, Return]
  Optional<ESTree::WithStatementNode *> parseWithStatement(Param param);
  /// \param param [Yield, Return]
  Optional<ESTree::SwitchStatementNode *> parseSwitchStatement(Param param);
  /// \param param [Yield]
  Optional<ESTree::ThrowStatementNode *> parseThrowStatement(Param param);
  /// \param param [Yield, Return]
  Optional<ESTree::TryStatementNode *> parseTryStatement(Param param);
  Optional<ESTree::DebuggerStatementNode *> parseDebuggerStatement();

  Optional<ESTree::Node *> parsePrimaryExpression();
  Optional<ESTree::ArrayExpressionNode *> parseArrayLiteral();
  Optional<ESTree::ObjectExpressionNode *> parseObjectLiteral();
  Optional<ESTree::Node *> parseSpreadElement();
  Optional<ESTree::Node *> parsePropertyAssignment(bool eagerly);

  /// Parse a property key which is a string, number or identifier. If it is
  /// neither, reports an error.
  Optional<ESTree::Node *> parsePropertyName();

  /// Parse a template literal starting at either TemplateHead or
  /// NoSubstitutionTemplate.
  /// \param param [Yield, Tagged]
  Optional<ESTree::Node *> parseTemplateLiteral(Param param);

  Optional<ESTree::FunctionExpressionNode *> parseFunctionExpression(
      bool forceEagerly = false);

  /// Indicates whether certain functions should recognize `?.` as a chaining
  /// operator. `?.` is not allowed in a NewExpression, for example.
  enum class IsConstructorCall { No, Yes };

  /// Parse OptionalExpression except the MemberExpression production starting
  /// with "new".
  Optional<ESTree::Node *> parseOptionalExpressionExceptNew(
      IsConstructorCall isConstructorCall);

  /// The "tail" of \c parseOptionalExpressionExceptNew(). It parses the
  /// optional MemberExpression following the base PrimaryExpression. It is
  /// ordinarily called by \c parseOptionalExpressionExceptNew(), but we need
  /// to call it explicitly after parsing "new.target".
  Optional<ESTree::Node *> parseOptionalExpressionExceptNew_tail(
      IsConstructorCall isConstructorCall,
      SMLoc startLoc,
      ESTree::Node *expr);

  /// Returns a dummy Optional<> just to indicate success or failure like all
  /// other functions.
  Optional<const char *> parseArguments(
      ESTree::NodeList &argList,
      SMLoc &endLoc);

  /// \param startLoc the start location of the expression
  /// \param objectLoc the location of the object part of the expression and is
  ///     used for error display.
  /// \param seenOptionalChain true if there was a ?. leading up to the
  ///     member select (set by parseOptionalExpressionExceptNew)
  Optional<ESTree::Node *> parseMemberSelect(
      SMLoc startLoc,
      SMLoc objectLoc,
      ESTree::NodePtr expr,
      bool seenOptionalChain);

  /// \param startLoc the start location of the expression, used for error
  ///     display.
  /// \param typeArgs the optional type arguments parsed before the '('.
  /// \param seenOptionalChain true when `?.` is used in the chain leading
  ///     to this call expression
  /// \param optional true when `?.` is used immediately prior to the Arguments.
  Optional<ESTree::Node *> parseCallExpression(
      SMLoc startLoc,
      ESTree::NodePtr expr,
      ESTree::NodePtr typeArgs,
      bool seenOptionalChain,
      bool optional);

  /// Parse a \c NewExpression or a \c OptionalExpression.
  /// After we have recognized "new", there is an apparent ambiguity in the
  /// grammar between \c NewExpression and \c MemberExpression:
  ///
  /// \code
  ///     NewExpression:
  ///         MemberExpression
  ///         new NewExpression
  ///
  ///     MemberExpression:
  ///         new MemberExpression Arguments
  /// \endcode
  ///
  /// The difference is that in the first case there are no arguments to the
  /// constructor.
  /// \param isConstructorCall is Yes when we have already recognized a "new".
  ///     This is used because we must disallow the ?. token before the
  ///     arguments to a constructor call only when "new" is used. For example,
  ///     the code `new a?.b()` is not valid.
  Optional<ESTree::Node *> parseNewExpressionOrOptionalExpression(
      IsConstructorCall isConstructorCall);
  Optional<ESTree::Node *> parseLeftHandSideExpression();
  Optional<ESTree::Node *> parsePostfixExpression();
  Optional<ESTree::Node *> parseUnaryExpression();

  /// Parse a binary expression using a precedence table, in order to decrease
  /// recursion depth.
  Optional<ESTree::Node *> parseBinaryExpression(Param param);

  /// Whether to allow a typed arrow function in the assignment expression.
  enum class AllowTypedArrowFunction { No, Yes };

  /// Whether to parse CoverTypedIdentifier nodes when seeing a `:`.
  /// These can only be used as typed parameters in certain contexts.
  enum class CoverTypedParameters { No, Yes };

  Optional<ESTree::Node *> parseConditionalExpression(
      Param param = ParamIn,
      CoverTypedParameters coverTypedParameters = CoverTypedParameters::Yes);
  Optional<ESTree::YieldExpressionNode *> parseYieldExpression(
      Param param = ParamIn);

  Optional<ESTree::ClassDeclarationNode *> parseClassDeclaration(Param param);
  Optional<ESTree::ClassExpressionNode *> parseClassExpression();

  enum class ClassParseKind { Declaration, Expression };

  /// Parse the class starting after the name (which is optional).
  /// \param name the name if provided, nullptr if otherwise.
  /// \param if the name is provided, the type params if provided, nullptr
  /// otherwise.
  /// \param kind whether the class is a declaration or expression.
  Optional<ESTree::Node *> parseClassTail(
      SMLoc startLoc,
      ESTree::Node *name,
      ESTree::Node *typeParams,
      ClassParseKind kind);

  Optional<ESTree::ClassBodyNode *> parseClassBody(SMLoc startLoc);

  Optional<ESTree::Node *> parseClassElement(
      bool isStatic,
      SMRange startRange,
      bool declare,
      bool isPrivate,
      bool eagerly = false);

  /// Reparse the specified node as arrow function parameter list and store the
  /// parameter list in \p paramList. Print an error and return false on error,
  /// otherwise return true.
  /// \param[in/out] isAsync the arrow function is async. The caller may already
  /// know this prior to calling this function, in which case `true` should be
  /// passed. Otherwise, this function will try to reparse a call expression
  /// into an async arrow function.
  bool reparseArrowParameters(
      ESTree::Node *node,
      ESTree::NodeList &paramList,
      bool &isAsync);

  /// \param forceAsync set to true when it is already known that the arrow
  ///   function expression is 'async'. This occurs when there are no parens
  ///   around the argument list.
  Optional<ESTree::Node *> parseArrowFunctionExpression(
      Param param,
      ESTree::Node *leftExpr,
      ESTree::Node *typeParams,
      ESTree::Node *returnType,
      ESTree::Node *predicate,
      SMLoc startLoc,
      AllowTypedArrowFunction allowTypedArrowFunction,
      bool forceAsync);

#if HERMES_PARSE_FLOW
  Optional<ESTree::Node *> tryParseTypedAsyncArrowFunction(Param param);

  /// Attempt to parse a CoverTypedIdentifierNode which consists of a
  /// node which may be an arrow parameter, a colon, and a type.
  /// \param test the LHS of the potential CoverTypedIdentifierNode.
  /// \param optional whether the potential CoverTypedIdentifierNode is
  /// optional, meaning there was a question mark preceding the type annotation
  /// \return nullptr if there was no error but attempting to parse the
  ///   node is not possible because \p test can't be a formal parameter,
  ///   or there wasn't a colon in the first place, None on error.
  Optional<ESTree::Node *> tryParseCoverTypedIdentifierNode(
      ESTree::Node *test,
      bool optional);
#endif

  /// Reparse an ArrayExpression into an ArrayPattern.
  /// \param inDecl whether this is a declaration context or assignment.
  Optional<ESTree::Node *> reparseArrayAsignmentPattern(
      ESTree::ArrayExpressionNode *AEN,
      bool inDecl);

  /// Reparse an ObjectExpression into an ObjectPattern.
  /// \param inDecl whether this is a declaration context or assignment.
  Optional<ESTree::Node *> reparseObjectAssignmentPattern(
      ESTree::ObjectExpressionNode *OEN,
      bool inDecl);

  /// Reparse an ArrayExpression or ObjectExpression into ArrayPattern or
  /// ObjectPattern.
  /// \param inDecl whether this is a declaration context or assignment.
  Optional<ESTree::Node *> reparseAssignmentPattern(
      ESTree::Node *node,
      bool inDecl);

  Optional<ESTree::Node *> parseAssignmentExpression(
      Param param = ParamIn,
      AllowTypedArrowFunction allowTypedArrowFunction =
          AllowTypedArrowFunction::Yes,
      CoverTypedParameters coverTypedParameters = CoverTypedParameters::Yes,
      ESTree::Node *typeParams = nullptr);

  Optional<ESTree::Node *> parseExpression(
      Param param = ParamIn,
      CoverTypedParameters coverTypedParameters = CoverTypedParameters::Yes);

  /// Parse a FromClause and return the string literal representing the source.
  Optional<ESTree::StringLiteralNode *> parseFromClause();

  bool parseAssertClause(ESTree::NodeList &attributes);

  Optional<ESTree::ImportDeclarationNode *> parseImportDeclaration();

  /// \return the kind of the import.
  Optional<UniqueString *> parseImportClause(ESTree::NodeList &specifiers);

  Optional<ESTree::Node *> parseNameSpaceImport();
  bool parseNamedImports(ESTree::NodeList &specifiers);
  Optional<ESTree::ImportSpecifierNode *> parseImportSpecifier(SMLoc importLoc);

  Optional<ESTree::Node *> parseExportDeclaration();

  /// \param[out] specifiers the list of parsed specifiers.
  /// \param[out] invalids ranges of potentially invalid exported symbols,
  ///             only if the clause is eventually followed by a FromClause.
  bool parseExportClause(
      ESTree::NodeList &specifiers,
      llvh::SmallVectorImpl<SMRange> &invalids);

  /// \param[out] invalids ranges of potentially invalid exported symbols,
  ///             only if the clause is eventually followed by a FromClause.
  ///             Appended to if an exported name may be invalid.
  Optional<ESTree::Node *> parseExportSpecifier(
      SMLoc exportLoc,
      llvh::SmallVectorImpl<SMRange> &invalids);

  /// If the current token can be recognised as a directive (ES5.1 14.1),
  /// process the directive and return the allocated directive statement.
  /// Note that this function never needs to returns an error.
  /// \return the allocated directive statement if this is a directive, or
  ///    null if it isn't.
  ESTree::ExpressionStatementNode *parseDirective();

#if HERMES_PARSE_JSX
  Optional<ESTree::Node *> parseJSX();
  Optional<ESTree::Node *> parseJSXElement(SMLoc start);
  Optional<ESTree::Node *> parseJSXFragment(SMLoc start);

  Optional<ESTree::JSXOpeningElementNode *> parseJSXOpeningElement(SMLoc start);
  Optional<ESTree::Node *> parseJSXSpreadAttribute();
  Optional<ESTree::Node *> parseJSXAttribute();

  /// \param children populated with the JSX children.
  /// \return the JSXClosingElement or JSXClosingFragment.
  Optional<ESTree::Node *> parseJSXChildren(ESTree::NodeList &children);
  Optional<ESTree::Node *> parseJSXChildExpression(SMLoc start);

  /// Parse JSXClosingElement or JSXClosingFragment.
  Optional<ESTree::Node *> parseJSXClosing(SMLoc start);

  enum class AllowJSXMemberExpression { No, Yes };

  /// \param allowMemberExpression whether JSXMemberExpression (foo.bar) is a
  /// valid parse of the JSXElementName.
  Optional<ESTree::Node *> parseJSXElementName(
      AllowJSXMemberExpression allowJSXMemberExpression);
#endif

#if HERMES_PARSE_FLOW || HERMES_PARSE_TS
  enum class AllowAnonFunctionType { No, Yes };

  Optional<ESTree::Node *> parseTypeAnnotation(
      Optional<SMLoc> wrappedStart = None,
      AllowAnonFunctionType allowAnonFunctionType =
          AllowAnonFunctionType::Yes) {
    assert(context_.getParseFlow() || context_.getParseTS());
#if HERMES_PARSE_FLOW
    if (context_.getParseFlow())
      return parseTypeAnnotationFlow(wrappedStart, allowAnonFunctionType);
#endif
#if HERMES_PARSE_TS
    return parseTypeAnnotationTS(wrappedStart);
#endif
  }
#endif

#if HERMES_PARSE_FLOW
  /// \param wrappedStart if set, the type annotation should be wrapped in a
  /// TypeAnnotationNode starting at this location. If not set, the type
  /// annotation should not be wrapped in a TypeAnnotationNode.
  Optional<ESTree::Node *> parseTypeAnnotationFlow(
      Optional<SMLoc> wrappedStart = None,
      AllowAnonFunctionType allowAnonFunctionType = AllowAnonFunctionType::Yes);

  /// Allow 'declare export type', which is only allowed in 'declare module'.
  enum class AllowDeclareExportType { No, Yes };

  Optional<ESTree::Node *> parseFlowDeclaration();
  Optional<ESTree::Node *> parseDeclareFLow(
      SMLoc start,
      AllowDeclareExportType allowDeclareExportType);

  enum class TypeAliasKind { None, Declare, Opaque, DeclareOpaque };
  Optional<ESTree::Node *> parseTypeAliasFlow(SMLoc start, TypeAliasKind kind);

  /// \param declareStart if set, parse a DeclareInterfaceNode starting at
  /// this location. If not set, parse an InterfaceDeclarationNode.
  Optional<ESTree::Node *> parseInterfaceDeclarationFlow(
      Optional<SMLoc> declareStart = None);

  /// \pre current token is 'extends' or '{'.
  /// \param[out] extends the super-interfaces for the parsed interface.
  /// \return the body of the interface
  Optional<ESTree::Node *> parseInterfaceTailFlow(
      SMLoc start,
      ESTree::NodeList &extends);
  bool parseInterfaceExtends(SMLoc start, ESTree::NodeList &extends);

  Optional<ESTree::Node *> parseDeclareFunctionFlow(SMLoc start);
  Optional<ESTree::Node *> parseDeclareClassFlow(SMLoc start);
  Optional<ESTree::Node *> parseDeclareExportFlow(
      SMLoc start,
      AllowDeclareExportType allowDeclareExportType);
  Optional<ESTree::Node *> parseDeclareModuleFlow(SMLoc start);

  Optional<ESTree::Node *> parseExportTypeDeclarationFlow(SMLoc start);

  Optional<ESTree::Node *> parseUnionTypeAnnotationFlow();
  Optional<ESTree::Node *> parseIntersectionTypeAnnotationFlow();
  Optional<ESTree::Node *> parseAnonFunctionWithoutParensTypeAnnotationFlow();
  Optional<ESTree::Node *> parsePrefixTypeAnnotationFlow();
  Optional<ESTree::Node *> parsePostfixTypeAnnotationFlow();
  Optional<ESTree::Node *> parsePrimaryTypeAnnotationFlow();
  Optional<ESTree::Node *> parseTupleTypeAnnotationFlow();
  Optional<ESTree::Node *> parseFunctionTypeAnnotationFlow();
  Optional<ESTree::Node *> parseFunctionTypeAnnotationWithParamsFlow(
      SMLoc start,
      ESTree::NodeList &&params,
      ESTree::Node *thisConstraint,
      ESTree::Node *rest,
      ESTree::Node *typeParams);
  Optional<ESTree::Node *> parseFunctionOrGroupTypeAnnotationFlow();

  /// Whether to allow 'proto' properties in an object type annotation.
  enum class AllowProtoProperty { No, Yes };

  /// Whether to allow 'static' properties in an object type annotation.
  enum class AllowStaticProperty { No, Yes };

  /// Whether to allow spread properties in an object type annotation.
  enum class AllowSpreadProperty { No, Yes };

  Optional<ESTree::Node *> parseObjectTypeAnnotationFlow(
      AllowProtoProperty allowProtoProperty,
      AllowStaticProperty allowStaticProperty,
      AllowSpreadProperty allowSpreadProperty);
  bool parseObjectTypePropertiesFlow(
      AllowProtoProperty allowProtoProperty,
      AllowStaticProperty allowStaticProperty,
      AllowSpreadProperty allowSpreadProperty,
      ESTree::NodeList &properties,
      ESTree::NodeList &indexers,
      ESTree::NodeList &callProperties,
      ESTree::NodeList &internalSlots,
      bool &inexact);
  bool parsePropertyTypeAnnotationFlow(
      AllowProtoProperty allowProtoProperty,
      AllowStaticProperty allowStaticProperty,
      ESTree::NodeList &properties,
      ESTree::NodeList &indexers,
      ESTree::NodeList &callProperties,
      ESTree::NodeList &internalSlots);

  /// Current token must be immediately after opening '['.
  Optional<ESTree::Node *> parseTypeIndexerPropertyFlow(
      SMLoc start,
      ESTree::Node *variance,
      bool isStatic);

  Optional<ESTree::Node *> parseTypePropertyFlow(
      SMLoc start,
      ESTree::Node *variance,
      bool isStatic,
      bool proto,
      ESTree::Node *key);
  Optional<ESTree::Node *>
  parseMethodTypePropertyFlow(SMLoc start, bool isStatic, ESTree::Node *key);
  Optional<ESTree::Node *> parseGetOrSetTypePropertyFlow(
      SMLoc start,
      bool isStatic,
      bool isGetter,
      ESTree::Node *key);

  Optional<ESTree::Node *> parseTypeParamsFlow();
  Optional<ESTree::Node *> parseTypeParamFlow();
  Optional<ESTree::Node *> parseTypeArgsFlow();

  /// \param[out] params the parameters, populated by reference.
  /// \param[out] thisConstraint the type annotation for 'this'.
  /// \return the rest parameter if it exists, nullptr otherwise. None still
  /// indicates an error.
  Optional<ESTree::FunctionTypeParamNode *>
  parseFunctionTypeAnnotationParamsFlow(
      ESTree::NodeList &params,
      ESTree::NodePtr &thisConstraint);
  Optional<ESTree::FunctionTypeParamNode *>
  parseFunctionTypeAnnotationParamFlow();

  Optional<ESTree::Node *> parseTypeCallPropertyFlow(
      SMLoc start,
      bool isStatic);

  Optional<ESTree::GenericTypeAnnotationNode *> parseGenericTypeFlow();

  Optional<ESTree::ClassImplementsNode *> parseClassImplementsFlow();

  /// Parse a property which looks like a method, starting at the opening '('.
  /// \param typeParams (optional) type params between '<' and '>' before '('.
  Optional<ESTree::FunctionTypeAnnotationNode *>
  parseMethodishTypeAnnotationFlow(SMLoc start, ESTree::Node *typeParams);

  Optional<ESTree::Node *> parsePredicateFlow();

  Optional<ESTree::IdentifierNode *> reparseTypeAnnotationAsIdentifierFlow(
      ESTree::Node *typeAnnotation);

  enum class EnumKind {
    String,
    Number,
    Boolean,
    Symbol,
  };

  static llvh::StringRef enumKindStrFlow(EnumKind kind) {
    switch (kind) {
      case EnumKind::String:
        return "string";
      case EnumKind::Number:
        return "number";
      case EnumKind::Boolean:
        return "boolean";
      case EnumKind::Symbol:
        return "symbol";
    }
    llvm_unreachable("No other kind of enum");
  }

  static OptValue<EnumKind> getMemberEnumKindFlow(ESTree::Node *member) {
    switch (member->getKind()) {
      case ESTree::NodeKind::EnumStringMember:
        return EnumKind::String;
      case ESTree::NodeKind::EnumNumberMember:
        return EnumKind::Number;
      case ESTree::NodeKind::EnumBooleanMember:
        return EnumKind::Boolean;
      default:
        return None;
    }
  }

  Optional<ESTree::Node *> parseEnumDeclarationFlow();
  Optional<ESTree::Node *> parseEnumBodyFlow(
      OptValue<EnumKind> optKind,
      Optional<SMLoc> explicitTypeStart);
  Optional<ESTree::Node *> parseEnumMemberFlow();
#endif

#if HERMES_PARSE_TS
  /// Whether a TS function type is a constructor.
  enum class IsConstructorType { No, Yes };

  Optional<ESTree::Node *> parseTypeAnnotationTS(
      Optional<SMLoc> wrappedStart = None);

  Optional<ESTree::Node *> parseTSDeclaration();
  Optional<ESTree::Node *> parseTSTypeAliasDeclaration(SMLoc start);
  Optional<ESTree::Node *> parseTSInterfaceDeclaration();
  Optional<ESTree::Node *> parseTSEnumDeclaration();
  Optional<ESTree::Node *> parseTSEnumMember();
  Optional<ESTree::Node *> parseTSNamespaceDeclaration();

  Optional<ESTree::Node *> parseTSTypeParameters();
  Optional<ESTree::Node *> parseTSTypeParameter();

  Optional<ESTree::Node *> parseTSUnionType();
  Optional<ESTree::Node *> parseTSIntersectionType();
  Optional<ESTree::Node *> parseTSTupleType();
  Optional<ESTree::Node *> parseTSFunctionOrParenthesizedType(
      SMLoc start,
      ESTree::Node *typeParams,
      IsConstructorType isConstructorType);
  bool parseTSFunctionTypeParams(SMLoc start, ESTree::NodeList &params);
  Optional<ESTree::Node *> parseTSFunctionTypeParam();
  Optional<ESTree::Node *> parseTSFunctionTypeWithParams(
      SMLoc start,
      ESTree::NodeList &&params,
      ESTree::Node *typeParams);

  Optional<ESTree::Node *> parseTSObjectType();
  Optional<ESTree::Node *> parseTSObjectTypeMember();
  Optional<ESTree::Node *> parseTSIndexSignature(SMLoc start);

  Optional<ESTree::Node *> parseTSPostfixType();
  Optional<ESTree::Node *> parseTSPrimaryType();
  Optional<ESTree::TSTypeReferenceNode *> parseTSTypeReference();
  Optional<ESTree::Node *> parseTSQualifiedName();
  Optional<ESTree::Node *> parseTSTypeQuery();
  Optional<ESTree::Node *> parseTSTypeArguments();

  ESTree::Node *reparseIdentifierAsTSTypeAnnotation(
      ESTree::IdentifierNode *ident);
#endif

  /// RAII to save and restore the current setting of "strict mode" and
  /// "seen directives".
  class SaveStrictModeAndSeenDirectives {
    JSParserImpl *const parser_;
    const bool oldStrictMode_;
    const unsigned oldSeenDirectiveSize_;

   public:
    explicit SaveStrictModeAndSeenDirectives(JSParserImpl *parser)
        : parser_(parser),
          oldStrictMode_(parser->isStrictMode()),
          oldSeenDirectiveSize_(parser->getSeenDirectives().size()) {}
    ~SaveStrictModeAndSeenDirectives() {
      parser_->setStrictMode(oldStrictMode_);
      parser_->getSeenDirectives().resize(oldSeenDirectiveSize_);
    }
  };

  /// RAII to track the recursion depth.
  class TrackRecursion {
    JSParserImpl *const parser_;

   public:
    TrackRecursion(JSParserImpl *parser) : parser_(parser) {
      ++parser_->recursionDepth_;
    }
    ~TrackRecursion() {
      --parser_->recursionDepth_;
    }
  };

/// Declare a RAII recursion tracker. Check whether the recursion limit has
/// been exceeded, and if so generate an error and return an empty
/// llvh::Optional<>.
/// The macro only works from inside JSParserImpl methods.
#define CHECK_RECURSION                \
  TrackRecursion trackRecursion{this}; \
  if (recursionDepthCheck())           \
    return llvh::None;
};

} // namespace detail
} // namespace parser
} // namespace hermes

#endif // HERMES_PARSER_JSPARSERIMPL_H
