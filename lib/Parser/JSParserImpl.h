/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSPARSERIMPL_H
#define HERMES_PARSER_JSPARSERIMPL_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Parser/JSLexer.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Parser/PreParser.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/DataTypes.h"

#include <utility>

namespace hermes {
namespace parser {
namespace detail {

using llvm::ArrayRef;
using llvm::None;
using llvm::Optional;

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
      std::unique_ptr<llvm::MemoryBuffer> input);

  explicit JSParserImpl(Context &context, uint32_t bufferId, ParserPass pass);

  JSParserImpl(Context &context, StringRef input)
      : JSParserImpl(
            context,
            llvm::MemoryBuffer::getMemBuffer(input, "JavaScript")) {}

  JSParserImpl(Context &context, llvm::MemoryBufferRef input)
      : JSParserImpl(context, llvm::MemoryBuffer::getMemBuffer(input)) {}

  Context &getContext() {
    return context_;
  }

  bool isStrictMode() const {
    return lexer_.isStrictMode();
  }

  void setStrictMode(bool mode) {
    lexer_.setStrictMode(mode);
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
  static constexpr unsigned MAX_RECURSION_DEPTH = 1024;

  /// Set when the parser sees the 'use static builtin' directive in any scope.
  bool useStaticBuiltin_{false};

  /// Set when the parser is inside a generator function.
  /// This is used when checking if `yield` is a valid Identifier name.
  bool paramYield_{false};

  // Certain known identifiers which we need to use when constructing the
  // ESTree or when parsing;
  UniqueString *getIdent_;
  UniqueString *setIdent_;
  UniqueString *initIdent_;
  UniqueString *useStrictIdent_;
  UniqueString *letIdent_;
  UniqueString *ofIdent_;
  UniqueString *useStaticBuiltinIdent_;
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
  UniqueString *targetIdent_;
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
  /// Check whether the current token is the specified identifier and consume it
  /// if so. \returns true if the token matched.
  bool checkAndEat(UniqueString *ident);
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
  bool checkDeclaration() const {
    return checkN(
        TokenKind::rw_function,
        letIdent_,
        TokenKind::rw_const,
        TokenKind::rw_class);
  }

  /// Check whether the current token begins a template literal.
  bool checkTemplateLiteral() const {
    return check(TokenKind::no_substitution_template, TokenKind::template_head);
  }

  /// Check whether the current token can be the token after the end of an
  /// AssignmentExpression.
  bool checkEndAssignmentExpression() const;

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
      bool parseDirectives,
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
  /// \param param [Yield]
  Optional<ESTree::BlockStatementNode *> parseFunctionBody(
      Param param,
      bool eagerly,
      JSLexer::GrammarContext grammarContext = JSLexer::AllowRegExp,
      bool parseDirectives = false);

  /// Parse a declaration.
  /// \param param [Yield]
  Optional<ESTree::Node *> parseDeclaration(Param param);

  /// Check if the provided string is a valid binding identifier.
  /// Can be used to validate identifiers after we've passed lexing them.
  /// The caller must report any errors if this function returns false.
  /// \param param [Yield]
  /// \param id the string to be validated.
  /// \param kind the TokenKind provided when the string was lexed.
  /// \return true if \param id is a valid binding identifier.
  bool validateBindingIdentifier(Param param, UniqueString *id, TokenKind kind);

  /// ES 2015 12.1
  /// Does not generate an error. It is expected that the caller will do it.
  /// \param param [Yield]
  Optional<ESTree::IdentifierNode *> parseBindingIdentifier(Param param);
  /// Parse a VariableStatement or LexicalDeclaration.
  /// \param param [In, Yield]
  /// \param declLoc the location of the let/const for error messages.
  Optional<ESTree::VariableDeclarationNode *> parseLexicalDeclaration(
      Param param);
  /// Parse a VariableStatement or LexicalDeclaration.
  /// \param param [Yield]
  /// \param declLoc the location of the let/const for error messages.
  Optional<ESTree::VariableDeclarationNode *> parseVariableStatement(
      Param param);

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

  /// Parse MemberExpression except the production starting with "new".
  Optional<ESTree::Node *> parseMemberExpressionExceptNew();

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

  /// Parse a \c NewExpression or a \c MemberExpression.
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
  Optional<ESTree::Node *> parseNewExpressionOrMemberExpression();
  Optional<ESTree::Node *> parseLeftHandSideExpression();
  Optional<ESTree::Node *> parsePostfixExpression();
  Optional<ESTree::Node *> parseUnaryExpression();

  /// Parse a binary expression using a precedence table, in order to decrease
  /// recursion depth.
  Optional<ESTree::Node *> parseBinaryExpression(Param param);

  Optional<ESTree::Node *> parseConditionalExpression(Param param = ParamIn);
  Optional<ESTree::YieldExpressionNode *> parseYieldExpression(
      Param param = ParamIn);

  Optional<ESTree::ClassDeclarationNode *> parseClassDeclaration(Param param);
  Optional<ESTree::ClassExpressionNode *> parseClassExpression();
  Optional<ESTree::ClassBodyNode *> parseClassTail(
      SMLoc startLoc,
      ESTree::NodePtr &superClass);
  Optional<ESTree::ClassBodyNode *> parseClassBody(SMLoc startLoc);

  Optional<ESTree::MethodDefinitionNode *> parseMethodDefinition(
      bool isStatic,
      SMRange startRange,
      bool eagerly = false);

  /// Reparse the specified node as arrow function parameter list and store the
  /// parameter list in \p paramList. Print an error and return false on error,
  /// otherwise return true.
  bool reparseArrowParameters(ESTree::Node *node, ESTree::NodeList &paramList);

  Optional<ESTree::Node *> parseArrowFunctionExpression(
      Param param,
      ESTree::Node *leftExpr);

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

  Optional<ESTree::Node *> parseAssignmentExpression(Param param = ParamIn);
  Optional<ESTree::Node *> parseExpression(Param param = ParamIn);

  /// Parse a FromClause and return the string literal representing the source.
  Optional<ESTree::StringLiteralNode *> parseFromClause();

  Optional<ESTree::Node *> parseImportDeclaration();
  bool parseImportClause(ESTree::NodeList &specifiers);
  Optional<ESTree::Node *> parseNameSpaceImport();
  bool parseNamedImports(ESTree::NodeList &specifiers);
  Optional<ESTree::ImportSpecifierNode *> parseImportSpecifier(SMLoc importLoc);

  Optional<ESTree::Node *> parseExportDeclaration();

  /// \param[out] specifiers the list of parsed specifiers.
  /// \param[out] endLoc end location after export clause has been parsed.
  /// \param[out] invalids ranges of potentially invalid exported symbols,
  ///             only if the clause is eventually followed by a FromClause.
  bool parseExportClause(
      ESTree::NodeList &specifiers,
      SMLoc &endLoc,
      llvm::SmallVectorImpl<SMRange> &invalids);

  /// \param[out] invalids ranges of potentially invalid exported symbols,
  ///             only if the clause is eventually followed by a FromClause.
  ///             Appended to if an exported name may be invalid.
  Optional<ESTree::Node *> parseExportSpecifier(
      SMLoc exportLoc,
      llvm::SmallVectorImpl<SMRange> &invalids);

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
    JSParserImpl *const parser_;
    const bool oldValue_;

   public:
    SaveStrictMode(JSParserImpl *parser)
        : parser_(parser), oldValue_(parser->isStrictMode()) {}
    ~SaveStrictMode() {
      parser_->setStrictMode(oldValue_);
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
};

}; // namespace detail
}; // namespace parser
}; // namespace hermes

#endif // HERMES_PARSER_JSPARSERIMPL_H
