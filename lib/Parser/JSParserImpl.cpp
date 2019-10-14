/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "JSParserImpl.h"

#include "hermes/Support/PerfSection.h"

#include "llvm/Support/SaveAndRestore.h"

using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;

namespace hermes {
namespace parser {
namespace detail {

// If a function's source code body is smaller than this number of bytes,
// compile it immediately instead of creating a lazy stub.
static const int PreemptiveCompilationThresholdBytes = 160;

/// Declare a RAII recursion tracker. Check whether the recursion limit has
/// been exceeded, and if so generate an error and return an empty
/// llvm::Optional<>.
#define CHECK_RECURSION                \
  TrackRecursion trackRecursion{this}; \
  if (recursionDepthExceeded())        \
    return llvm::None;

JSParserImpl::JSParserImpl(
    Context &context,
    std::unique_ptr<llvm::MemoryBuffer> input)
    : context_(context),
      sm_(context.getSourceErrorManager()),
      lexer_(
          std::move(input),
          context.getSourceErrorManager(),
          context.getAllocator(),
          &context.getStringTable(),
          context.isStrictMode()),
      pass_(FullParse) {
  initializeIdentifiers();
}

JSParserImpl::JSParserImpl(Context &context, uint32_t bufferId, ParserPass pass)
    : context_(context),
      sm_(context.getSourceErrorManager()),
      lexer_(
          bufferId,
          context.getSourceErrorManager(),
          context.getAllocator(),
          &context.getStringTable(),
          context.isStrictMode()),
      pass_(pass) {
  preParsed_ = context.getPreParsedBufferInfo(bufferId);
  initializeIdentifiers();
}

void JSParserImpl::initializeIdentifiers() {
  getIdent_ = lexer_.getIdentifier("get");
  setIdent_ = lexer_.getIdentifier("set");
  initIdent_ = lexer_.getIdentifier("init");
  useStrictIdent_ = lexer_.getIdentifier("use strict");
  letIdent_ = lexer_.getIdentifier("let");
  ofIdent_ = lexer_.getIdentifier("of");
  useStaticBuiltinIdent_ = lexer_.getIdentifier("use static builtin");
  fromIdent_ = lexer_.getIdentifier("from");
  asIdent_ = lexer_.getIdentifier("as");
  implementsIdent_ = lexer_.getIdentifier("implements");
  interfaceIdent_ = lexer_.getIdentifier("interface");
  packageIdent_ = lexer_.getIdentifier("package");
  privateIdent_ = lexer_.getIdentifier("private");
  protectedIdent_ = lexer_.getIdentifier("protected");
  publicIdent_ = lexer_.getIdentifier("public");
  staticIdent_ = lexer_.getIdentifier("static");
  methodIdent_ = lexer_.getIdentifier("method");
  constructorIdent_ = lexer_.getIdentifier("constructor");
  yieldIdent_ = lexer_.getIdentifier("yield");
  newIdent_ = lexer_.getIdentifier("new");
  targetIdent_ = lexer_.getIdentifier("target");

  // Generate the string representation of all tokens.
  for (unsigned i = 0; i != NUM_JS_TOKENS; ++i)
    tokenIdent_[i] = lexer_.getIdentifier(tokenKindStr((TokenKind)i));
}

Optional<ESTree::ProgramNode *> JSParserImpl::parse() {
  PerfSection parsing("Parsing JavaScript");
  tok_ = lexer_.advance();
  auto res = parseProgram();
  if (!res)
    return None;
  if (lexer_.getSourceMgr().getErrorCount() != 0)
    return None;
  return res.getValue();
}

void JSParserImpl::errorExpected(
    ArrayRef<TokenKind> toks,
    const char *where,
    const char *what,
    SMLoc whatLoc) {
  llvm::SmallString<4> str;
  llvm::raw_svector_ostream ss{str};

  for (unsigned i = 0; i < toks.size(); ++i) {
    // Insert a separator after the first token.
    if (i > 0) {
      // Use " or " instead of ", " before the last token.
      if (i == toks.size() - 1)
        ss << " or ";
      else
        ss << ", ";
    }
    ss << "'" << tokenKindStr(toks[i]) << "'";
  }

  ss << " expected";

  // Optionally append the 'where' description.
  if (where)
    ss << " " << where;

  SMLoc errorLoc = tok_->getStartLoc();
  SourceErrorManager::SourceCoords curCoords;
  SourceErrorManager::SourceCoords whatCoords;

  // If the location of 'what' is provided, find its and the error's source
  // coordinates.
  if (whatLoc.isValid()) {
    sm_.findBufferLineAndLoc(errorLoc, curCoords);
    sm_.findBufferLineAndLoc(whatLoc, whatCoords);
  }

  if (whatCoords.isSameSourceLineAs(curCoords)) {
    // If the what source coordinates are on the same line as the error, show
    // them both.
    lexer_.error(
        errorLoc,
        SourceErrorManager::combineIntoRange(whatLoc, errorLoc),
        ss.str());
  } else {
    lexer_.error(errorLoc, ss.str());

    if (what && whatCoords.isValid())
      sm_.note(whatLoc, what);
  }
}

bool JSParserImpl::need(
    TokenKind kind,
    const char *where,
    const char *what,
    SMLoc whatLoc) {
  if (tok_->getKind() == kind) {
    return true;
  }
  errorExpected(kind, where, what, whatLoc);
  return false;
}

bool JSParserImpl::eat(
    TokenKind kind,
    JSLexer::GrammarContext grammarContext,
    const char *where,
    const char *what,
    SMLoc whatLoc) {
  if (need(kind, where, what, whatLoc)) {
    advance(grammarContext);
    return true;
  }
  return false;
}

bool JSParserImpl::checkAndEat(TokenKind kind) {
  if (tok_->getKind() == kind) {
    advance();
    return true;
  }
  return false;
}

bool JSParserImpl::checkAndEat(UniqueString *ident) {
  if (check(ident)) {
    advance();
    return true;
  }
  return false;
}

bool JSParserImpl::checkAssign() const {
  return checkN(
      TokenKind::equal,
      TokenKind::starequal,
      TokenKind::slashequal,
      TokenKind::percentequal,
      TokenKind::plusequal,
      TokenKind::minusequal,
      TokenKind::lesslessequal,
      TokenKind::greatergreaterequal,
      TokenKind::greatergreatergreaterequal,
      TokenKind::starstarequal,
      TokenKind::ampequal,
      TokenKind::caretequal,
      TokenKind::pipeequal);
}

bool JSParserImpl::checkEndAssignmentExpression() const {
  return checkN(
             TokenKind::rw_in,
             ofIdent_,
             TokenKind::r_paren,
             TokenKind::r_brace,
             TokenKind::r_square,
             TokenKind::comma,
             TokenKind::semi,
             TokenKind::colon,
             TokenKind::eof) ||
      lexer_.isNewLineBeforeCurrentToken();
}

bool JSParserImpl::eatSemi(SMLoc &endLoc, bool optional) {
  if (tok_->getKind() == TokenKind::semi) {
    endLoc = tok_->getEndLoc();
    advance();
    return true;
  }

  if (tok_->getKind() == TokenKind::r_brace ||
      tok_->getKind() == TokenKind::eof ||
      lexer_.isNewLineBeforeCurrentToken()) {
    return true;
  }

  if (!optional)
    lexer_.error(tok_->getStartLoc(), "';' expected");
  return false;
}

void JSParserImpl::processDirective(UniqueString *directive) {
  if (directive == useStrictIdent_)
    setStrictMode(true);
  if (directive == useStaticBuiltinIdent_)
    setUseStaticBuiltin();
}

bool JSParserImpl::recursionDepthExceeded() {
  if (recursionDepth_ < MAX_RECURSION_DEPTH)
    return false;
  lexer_.error(
      tok_->getStartLoc(),
      "Too many nested expressions/statements/declarations");
  return true;
}

Optional<ESTree::ProgramNode *> JSParserImpl::parseProgram() {
  SMLoc startLoc = tok_->getStartLoc();
  SaveStrictMode saveStrict{this};
  ESTree::NodeList stmtList;

  if (!parseStatementList(
          Param{}, TokenKind::eof, true, AllowImportExport::Yes, stmtList))
    return None;

  SMLoc endLoc = startLoc;
  if (!stmtList.empty()) {
    endLoc = stmtList.back().getEndLoc();
  }
  auto *program = setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ProgramNode(std::move(stmtList)));
  program->strictness = ESTree::makeStrictness(isStrictMode());
  return program;
}

Optional<ESTree::FunctionDeclarationNode *>
JSParserImpl::parseFunctionDeclaration(Param param, bool forceEagerly) {
  auto optRes = parseFunctionHelper(param, true, forceEagerly);
  if (!optRes)
    return None;
  return cast<ESTree::FunctionDeclarationNode>(*optRes);
}

Optional<ESTree::FunctionLikeNode *> JSParserImpl::parseFunctionHelper(
    Param param,
    bool isDeclaration,
    bool forceEagerly) {
  // function
  assert(tok_->getKind() == TokenKind::rw_function);
  SMLoc startLoc = advance().Start;

  bool isGenerator = checkAndEat(TokenKind::star);

  // newParamYield setting per the grammar:
  // FunctionDeclaration: BindingIdentifier[?Yield]
  // FunctionExpression: BindingIdentifier[~Yield]
  // GeneratorFunctionDeclaration: BindingIdentifier[?Yield]
  // GeneratorFunctionExpression: BindingIdentifier[+Yield]
  bool nameParamYield = isDeclaration ? paramYield_ : isGenerator;
  llvm::SaveAndRestore<bool> saveNameParamYield(paramYield_, nameParamYield);

  // identifier
  auto optId = parseBindingIdentifier(Param{});
  // If this is a default function declaration, then we can match
  // [+Default] function ( FormalParameters ) { FunctionBody }
  // so the identifier is optional and we can make it nullptr.
  if (isDeclaration && !param.has(ParamDefault) && !optId) {
    errorExpected(
        TokenKind::identifier,
        "after 'function'",
        "location of 'function'",
        startLoc);
    return None;
  }

  // (
  if (!need(
          TokenKind::l_paren,
          "at start of function parameter list",
          isDeclaration ? "function declaration starts here"
                        : "function expression starts here",
          startLoc)) {
    return None;
  }

  ESTree::NodeList paramList;

  llvm::SaveAndRestore<bool> saveArgsAndBodyParamYield(
      paramYield_, isGenerator);

  if (!parseFormalParameters(param, paramList))
    return None;

  // {
  if (!need(
          TokenKind::l_brace,
          isDeclaration ? "in function declaration" : "in function expression",
          isDeclaration ? "start of function declaration"
                        : "start of function expression",
          startLoc)) {
    return None;
  }

  SaveStrictMode saveStrictMode{this};

  // Grammar context to be used when lexing the closing brace.
  auto grammarContext =
      isDeclaration ? JSLexer::AllowRegExp : JSLexer::AllowDiv;

  if (pass_ == PreParse) {
    // Create the nodes we want to keep before the AllocationScope.
    ESTree::FunctionLikeNode *node;

    if (isDeclaration) {
      auto *decl = new (context_) ESTree::FunctionDeclarationNode(
          optId ? *optId : nullptr,
          std::move(paramList),
          nullptr,
          nullptr,
          isGenerator);
      // Initialize the node with a blank body.
      decl->_body = new (context_) ESTree::BlockStatementNode({});
      node = decl;
    } else {
      auto *expr = new (context_) ESTree::FunctionExpressionNode(
          optId ? *optId : nullptr, std::move(paramList), nullptr, isGenerator);
      // Initialize the node with a blank body.
      expr->_body = new (context_) ESTree::BlockStatementNode({});
      node = expr;
    }

    AllocationScope scope(context_.getAllocator());
    auto body = parseFunctionBody(Param{}, false, grammarContext, true);
    if (!body)
      return None;

    node->strictness = ESTree::makeStrictness(isStrictMode());
    return setLocation(startLoc, body.getValue(), node);
  }

  auto parsedBody =
      parseFunctionBody(Param{}, forceEagerly, grammarContext, true);
  if (!parsedBody)
    return None;
  auto *body = parsedBody.getValue();

  ESTree::FunctionLikeNode *node;
  if (isDeclaration) {
    auto *decl = new (context_) ESTree::FunctionDeclarationNode(
        optId ? *optId : nullptr,
        std::move(paramList),
        body,
        nullptr,
        isGenerator);
    decl->strictness = ESTree::makeStrictness(isStrictMode());
    node = decl;
  } else {
    auto *expr = new (context_) ESTree::FunctionExpressionNode(
        optId ? *optId : nullptr, std::move(paramList), body, isGenerator);
    expr->strictness = ESTree::makeStrictness(isStrictMode());
    node = expr;
  }
  return setLocation(startLoc, body, node);
}

bool JSParserImpl::parseFormalParameters(
    Param param,
    ESTree::NodeList &paramList) {
  assert(check(TokenKind::l_paren) && "FormalParameters must start with '('");
  // (
  SMLoc lparenLoc = advance().Start;

  while (!check(TokenKind::r_paren)) {
    if (check(TokenKind::dotdotdot)) {
      // BindingRestElement.
      auto optRestElem = parseBindingRestElement(param);
      if (!optRestElem)
        return false;
      paramList.push_back(*optRestElem.getValue());
      break;
    }

    // BindingElement.
    auto optElem = parseBindingElement(param);
    if (!optElem)
      return false;

    paramList.push_back(*optElem.getValue());

    if (!checkAndEat(TokenKind::comma))
      break;
  }

  // )
  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "at end of function parameter list",
          "start of parameter list",
          lparenLoc)) {
    return false;
  }

  return true;
}

Optional<ESTree::Node *> JSParserImpl::parseStatement(Param param) {
  CHECK_RECURSION;

#define _RET(parseFunc)       \
  if (auto res = (parseFunc)) \
    return res.getValue();    \
  else                        \
    return None;

  switch (tok_->getKind()) {
    case TokenKind::l_brace:
      _RET(parseBlock(param));
    case TokenKind::rw_var:
      _RET(parseVariableStatement(Param{}));
    case TokenKind::semi:
      _RET(parseEmptyStatement());
    case TokenKind::rw_if:
      _RET(parseIfStatement(param.get(ParamReturn)));
    case TokenKind::rw_while:
      _RET(parseWhileStatement(param.get(ParamReturn)));
    case TokenKind::rw_do:
      _RET(parseDoWhileStatement(param.get(ParamReturn)));
    case TokenKind::rw_for:
      _RET(parseForStatement(param.get(ParamReturn)));
    case TokenKind::rw_continue:
      _RET(parseContinueStatement());
    case TokenKind::rw_break:
      _RET(parseBreakStatement());
    case TokenKind::rw_return:
      _RET(parseReturnStatement());
    case TokenKind::rw_with:
      _RET(parseWithStatement(param.get(ParamReturn)));
    case TokenKind::rw_switch:
      _RET(parseSwitchStatement(param.get(ParamReturn)));
    case TokenKind::rw_throw:
      _RET(parseThrowStatement(Param{}));
    case TokenKind::rw_try:
      _RET(parseTryStatement(param.get(ParamReturn)));
    case TokenKind::rw_debugger:
      _RET(parseDebuggerStatement());

    default:
      _RET(parseExpressionOrLabelledStatement(param.get(ParamReturn)));
  }

#undef _RET
}

Optional<ESTree::BlockStatementNode *> JSParserImpl::parseFunctionBody(
    Param param,
    bool eagerly,
    JSLexer::GrammarContext grammarContext,
    bool parseDirectives) {
  if (pass_ == LazyParse && !eagerly) {
    auto startLoc = tok_->getStartLoc();
    assert(preParsed_->bodyStartToEnd.count(startLoc) == 1);
    auto endLoc = preParsed_->bodyStartToEnd[startLoc];
    if (endLoc.getPointer() - startLoc.getPointer() >
        PreemptiveCompilationThresholdBytes) {
      lexer_.seek(endLoc);
      advance();

      auto *body = new (context_) ESTree::BlockStatementNode({});
      body->isLazyFunctionBody = true;
      body->bufferId = lexer_.getBufferId();
      return setLocation(startLoc, endLoc, body);
    }
  }

  auto body = parseBlock(ParamReturn, grammarContext, parseDirectives);
  if (!body)
    return None;

  if (pass_ == PreParse) {
    preParsed_->bodyStartToEnd[(*body)->getStartLoc()] = (*body)->getEndLoc();
  }

  return body;
}

Optional<ESTree::Node *> JSParserImpl::parseDeclaration(Param param) {
  CHECK_RECURSION;

  assert(checkDeclaration() && "invalid start for declaration");

  if (tok_->getKind() == TokenKind::rw_function) {
    auto fdecl = parseFunctionDeclaration(Param{});
    if (!fdecl)
      return None;

    return *fdecl;
  }

  if (tok_->getKind() == TokenKind::rw_class) {
    auto optClass = parseClassDeclaration(Param{});
    if (!optClass)
      return None;

    return *optClass;
  }

  assert(
      (check(TokenKind::rw_const) || check(letIdent_)) &&
      "declaration can only be let or const");

  auto optLexDecl = parseLexicalDeclaration(ParamIn);
  if (!optLexDecl)
    return None;

  return *optLexDecl;
}

bool JSParserImpl::parseStatementListItem(
    Param param,
    bool parseDirectives,
    AllowImportExport allowImportExport,
    ESTree::NodeList &stmtList) {
  if (checkDeclaration()) {
    auto decl = parseDeclaration(Param{});
    if (!decl)
      return false;

    stmtList.push_back(*decl.getValue());
  } else if (tok_->getKind() == TokenKind::rw_import) {
    auto importDecl = parseImportDeclaration();
    if (!importDecl) {
      return false;
    }

    if (allowImportExport == AllowImportExport::Yes) {
      stmtList.push_back(*importDecl.getValue());
    } else {
      sm_.error(
          importDecl.getValue()->getSourceRange(),
          "import declaration must be at top level of module");
    }
  } else if (tok_->getKind() == TokenKind::rw_export) {
    auto exportDecl = parseExportDeclaration();
    if (!exportDecl) {
      return false;
    }

    if (allowImportExport == AllowImportExport::Yes) {
      stmtList.push_back(**exportDecl);
    } else {
      sm_.error(
          exportDecl.getValue()->getSourceRange(),
          "export declaration must be at top level of module");
    }
  } else {
    auto stmt = parseStatement(param.get(ParamReturn));
    if (!stmt)
      return false;

    stmtList.push_back(*stmt.getValue());
  }

  return true;
}

template <typename... Tail>
Optional<bool> JSParserImpl::parseStatementList(
    Param param,
    TokenKind until,
    bool parseDirectives,
    AllowImportExport allowImportExport,
    ESTree::NodeList &stmtList,
    Tail... otherUntil) {
  if (parseDirectives) {
    ESTree::ExpressionStatementNode *dirStmt;
    while (check(TokenKind::string_literal) &&
           (dirStmt = parseDirective()) != nullptr) {
      stmtList.push_back(*dirStmt);
    }
  }

  while (!check(TokenKind::eof) && !checkN(until, otherUntil...)) {
    if (!parseStatementListItem(
            param, parseDirectives, allowImportExport, stmtList)) {
      return None;
    }
  }

  return true;
}

Optional<ESTree::BlockStatementNode *> JSParserImpl::parseBlock(
    Param param,
    JSLexer::GrammarContext grammarContext,
    bool parseDirectives) {
  // {
  assert(check(TokenKind::l_brace));
  SMLoc startLoc = advance().Start;

  ESTree::NodeList stmtList;

  if (!parseStatementList(
          param,
          TokenKind::r_brace,
          parseDirectives,
          AllowImportExport::No,
          stmtList)) {
    return None;
  }

  // }
  auto *body = setLocation(
      startLoc,
      tok_,
      new (context_) ESTree::BlockStatementNode(std::move(stmtList)));
  if (!eat(
          TokenKind::r_brace,
          grammarContext,
          "at end of block",
          "block starts here",
          startLoc))
    return None;

  return body;
}

bool JSParserImpl::validateBindingIdentifier(
    Param param,
    UniqueString *id,
    TokenKind kind) {
  if (id == yieldIdent_) {
    // yield is permitted as BindingIdentifier in the grammar,
    // and prohibited with static semantics.
    if (isStrictMode() || paramYield_) {
      lexer_.error(
          tok_->getSourceRange(),
          "Unexpected usage of 'yield' as an identifier");
    }
  }

  if (isStrictMode() && id == letIdent_) {
    // ES9.0 12.1.1
    // BindingIdentifier : Identifier
    // Identifier : IdentifierName (but not ReservedWord)
    // It is a Syntax Error if this phrase is contained in strict mode code
    // and the StringValue of IdentifierName is: "implements", "interface",
    // "let", "package", "private", "protected", "public", "static", or
    // "yield".
    // NOTE: All except 'let' are scanned as reserved words instead of
    // identifiers, so we only check for `let` here.
    lexer_.error(
        tok_->getSourceRange(),
        "Invalid use of strict mode reserved word as binding identifier");
  }

  return kind == TokenKind::identifier || kind == TokenKind::rw_yield;
}

Optional<ESTree::IdentifierNode *> JSParserImpl::parseBindingIdentifier(
    Param param) {
  if (!check(TokenKind::identifier) && !tok_->isResWord()) {
    return None;
  }

  // If we have an identifier or reserved word, then store it and the kind,
  // and pass it to the validateBindingIdentifier function.
  UniqueString *id = tok_->getResWordOrIdentifier();
  TokenKind kind = tok_->getKind();
  if (!validateBindingIdentifier(param, id, kind)) {
    return None;
  }

  auto *node = setLocation(
      tok_, tok_, new (context_) ESTree::IdentifierNode(id, nullptr));
  advance();
  return node;
}

Optional<ESTree::VariableDeclarationNode *>
JSParserImpl::parseLexicalDeclaration(Param param) {
  assert(
      (check(TokenKind::rw_var) || check(TokenKind::rw_const) ||
       check(letIdent_)) &&
      "parseLexicalDeclaration() expects var/const/let");
  bool isConst = check(TokenKind::rw_const);
  auto kindIdent = tok_->getResWordOrIdentifier();

  SMLoc startLoc = advance().Start;

  ESTree::NodeList declList;
  if (!parseVariableDeclarationList(param, declList, startLoc))
    return None;

  auto endLoc = declList.back().getEndLoc();
  if (!eatSemi(endLoc))
    return None;

  if (isConst) {
    for (const ESTree::Node &decl : declList) {
      const auto *varDecl = cast<ESTree::VariableDeclaratorNode>(&decl);
      if (!varDecl->_init) {
        // ES9.0 13.3.1.1
        // LexicalBinding : BindingIdentifier Initializer
        // It is a Syntax Error if Initializer is not present and
        // IsConstantDeclaration of the LexicalDeclaration containing this
        // LexicalBinding is true.
        // Note that we don't perform this check in the SemanticValidator
        // because `const` declarations in `for` loops don't need initializers.
        sm_.error(
            varDecl->getSourceRange(),
            "missing initializer in const declaration");
      }
    }
  }

  auto *res = setLocation(
      startLoc,
      endLoc,
      new (context_)
          ESTree::VariableDeclarationNode(kindIdent, std::move(declList)));

  ensureDestructuringInitialized(res);

  return res;
}

Optional<ESTree::VariableDeclarationNode *>
JSParserImpl::parseVariableStatement(Param param) {
  return parseLexicalDeclaration(ParamIn);
}

Optional<const char *> JSParserImpl::parseVariableDeclarationList(
    Param param,
    ESTree::NodeList &declList,
    SMLoc declLoc) {
  do {
    auto optDecl = parseVariableDeclaration(param, declLoc);
    if (!optDecl)
      return None;
    declList.push_back(*optDecl.getValue());
  } while (checkAndEat(TokenKind::comma));

  return "OK";
}

void JSParserImpl::ensureDestructuringInitialized(
    ESTree::VariableDeclarationNode *declNode) {
  for (auto &elem : declNode->_declarations) {
    auto *declarator = cast<ESTree::VariableDeclaratorNode>(&elem);

    if (!isa<ESTree::PatternNode>(declarator->_id) || declarator->_init)
      continue;

    lexer_.error(
        declarator->_id->getSourceRange(),
        "destucturing declaration must be initialized");
  }
}

Optional<ESTree::VariableDeclaratorNode *>
JSParserImpl::parseVariableDeclaration(Param param, SMLoc declLoc) {
  ESTree::Node *target;

  if (check(TokenKind::l_square, TokenKind::l_brace)) {
    auto optPat = parseBindingPattern(param);
    if (!optPat)
      return None;

    target = *optPat;
  } else {
    auto optIdent = parseBindingIdentifier(Param{});
    if (!optIdent) {
      errorExpected(
          TokenKind::identifier,
          "in declaration",
          "declaration started here",
          declLoc);
      return None;
    }

    target = *optIdent;
  }

  // No initializer?
  if (!check(TokenKind::equal)) {
    return setLocation(
        target,
        target,
        new (context_) ESTree::VariableDeclaratorNode(nullptr, target));
  };

  // Parse the initializer.
  auto debugLoc = advance().Start;

  auto expr = parseAssignmentExpression(param);
  if (!expr)
    return None;

  return setLocation(
      target,
      *expr,
      debugLoc,
      new (context_) ESTree::VariableDeclaratorNode(*expr, target));
}

Optional<ESTree::Node *> JSParserImpl::parseBindingPattern(Param param) {
  assert(
      check(TokenKind::l_square, TokenKind::l_brace) &&
      "BindingPattern expects '{' or '['");
  if (check(TokenKind::l_square)) {
    auto optAB = parseArrayBindingPattern(param);
    if (!optAB)
      return None;
    return *optAB;
  } else {
    auto optOB = parseObjectBindingPattern(param);
    if (!optOB)
      return None;
    return *optOB;
  }
}

Optional<ESTree::ArrayPatternNode *> JSParserImpl::parseArrayBindingPattern(
    Param param) {
  assert(check(TokenKind::l_square) && "ArrayBindingPattern expects '['");

  // Eat the '[', recording the start location.
  auto startLoc = advance().Start;

  ESTree::NodeList elemList;

  if (!check(TokenKind::r_square)) {
    for (;;) {
      if (check(TokenKind::comma)) {
        // Elision.
        elemList.push_back(
            *setLocation(tok_, tok_, new (context_) ESTree::EmptyNode()));
      } else if (check(TokenKind::dotdotdot)) {
        // BindingRestElement.
        auto optRestElem = parseBindingRestElement(param);
        if (!optRestElem)
          return None;
        elemList.push_back(*optRestElem.getValue());
        break;
      } else {
        // BindingElement.
        auto optElem = parseBindingElement(param);
        if (!optElem)
          return None;
        elemList.push_back(*optElem.getValue());
      }

      if (!checkAndEat(TokenKind::comma))
        break;
      if (check(TokenKind::r_square)) // Check for ",]".
        break;
    }
  }

  SMLoc endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_square,
          JSLexer::AllowDiv,
          "at end of array binding pattern '[...'",
          "location of '['",
          startLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ArrayPatternNode(std::move(elemList)));
}

Optional<ESTree::Node *> JSParserImpl::parseBindingElement(Param param) {
  ESTree::Node *elem;

  if (check(TokenKind::l_square, TokenKind::l_brace)) {
    auto optPat = parseBindingPattern(param);
    if (!optPat)
      return None;
    elem = *optPat;
  } else {
    auto optIdent = parseBindingIdentifier(param);
    if (!optIdent) {
      lexer_.error(
          tok_->getStartLoc(),
          "identifier, '{' or '[' expected in binding pattern");
      return None;
    }
    elem = *optIdent;
  }

  // No initializer?
  if (!check(TokenKind::equal))
    return elem;

  auto optInit = parseBindingInitializer(param, elem);
  if (!optInit)
    return None;
  return *optInit;
}

Optional<ESTree::Node *> JSParserImpl::parseBindingRestElement(Param param) {
  assert(
      check(TokenKind::dotdotdot) &&
      "BindingRestElement expected to start with '...'");

  auto startLoc = advance().Start;

  auto optElem = parseBindingElement(param);
  if (!optElem)
    return None;
  if (isa<ESTree::AssignmentPatternNode>(*optElem)) {
    lexer_.error(
        optElem.getValue()->getSourceRange(),
        "rest elemenent may not have a default initializer");
    return None;
  }

  return setLocation(
      startLoc, *optElem, new (context_) ESTree::RestElementNode(*optElem));
}

Optional<ESTree::AssignmentPatternNode *> JSParserImpl::parseBindingInitializer(
    Param param,
    ESTree::Node *left) {
  assert(check(TokenKind::equal) && "binding initializer requires '='");

  // Parse the initializer.
  auto debugLoc = advance().Start;

  auto expr = parseAssignmentExpression(ParamIn + param);
  if (!expr)
    return None;

  return setLocation(
      left,
      *expr,
      debugLoc,
      new (context_) ESTree::AssignmentPatternNode(left, *expr));
}

Optional<ESTree::ObjectPatternNode *> JSParserImpl::parseObjectBindingPattern(
    Param param) {
  assert(check(TokenKind::l_brace) && "ObjectBindingPattern expects '{'");

  // Eat the '{', recording the start location.
  auto startLoc = advance().Start;

  ESTree::NodeList propList{};

  if (!check(TokenKind::r_brace)) {
    for (;;) {
      if (check(TokenKind::dotdotdot)) {
        // BindingRestProperty.
        auto optRestElem = parseBindingRestProperty(param);
        if (!optRestElem)
          return None;
        propList.push_back(*optRestElem.getValue());
        break;
      }
      auto optProp = parseBindingProperty(param);
      if (!optProp)
        return None;

      propList.push_back(**optProp);

      if (!checkAndEat(TokenKind::comma))
        break;
      if (check(TokenKind::r_brace)) // check for ",}"
        break;
    }
  }

  SMLoc endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_brace,
          JSLexer::AllowDiv,
          "at end of object binding pattern '{...'",
          "location of '{'",
          startLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ObjectPatternNode(std::move(propList)));
}

Optional<ESTree::PropertyNode *> JSParserImpl::parseBindingProperty(
    Param param) {
  bool computed = check(TokenKind::l_square);
  SMLoc startLoc = tok_->getStartLoc();
  auto optKey = parsePropertyName();
  if (!optKey)
    return None;
  ESTree::Node *key = optKey.getValue();

  ESTree::Node *value = nullptr;

  if (checkAndEat(TokenKind::colon)) {
    // PropertyName ":" BindingElement
    //               ^
    auto optElement = parseBindingElement(Param{});
    if (!optElement)
      return None;
    value = optElement.getValue();
  } else {
    // SingleNameBinding :
    //   BindingIdentifier Initializer[opt]
    //                     ^

    // Must validate BindingIdentifier, because there are certain identifiers
    // which are valid as PropertyName but not as BindingIdentifier.
    auto *ident = dyn_cast<ESTree::IdentifierNode>(key);
    if (!ident ||
        !validateBindingIdentifier(
            Param{}, ident->_name, TokenKind::identifier)) {
      lexer_.error(startLoc, "identifier expected in object binding pattern");
      return None;
    }

    if (check(TokenKind::equal)) {
      // BindingIdentifier Initializer
      //                   ^
      // Clone the key because parseBindingInitializer will wrap it.
      auto *left = setLocation(
          ident,
          ident,
          new (context_) ESTree::IdentifierNode(ident->_name, nullptr));
      auto optInit = parseBindingInitializer(param + ParamIn, left);
      if (!optInit)
        return None;

      value = *optInit;
    } else {
      // BindingIdentifier
      //                   ^
      // Shorthand property initialization, clone the key directly.
      value = setLocation(
          ident,
          ident,
          new (context_) ESTree::IdentifierNode(ident->_name, nullptr));
    }
  }

  return setLocation(
      key,
      value,
      new (context_) ESTree::PropertyNode(key, value, initIdent_, computed));
}

Optional<ESTree::Node *> JSParserImpl::parseBindingRestProperty(
    hermes::parser::detail::Param param) {
  assert(
      check(TokenKind::dotdotdot) &&
      "BindingRestProperty expected to start with '...'");

  auto startLoc = advance().Start;

  // NOTE: the spec says that this cannot be another pattern, even though it
  // would make sense.
#if 0
  auto optElem = parseBindingElement(param);
#else
  auto optElem = parseBindingIdentifier(param);
#endif
  if (!optElem) {
    sm_.error(
        tok_->getStartLoc(),
        "identifier expected after '...' in object pattern");
    return None;
  }

  return setLocation(
      startLoc, *optElem, new (context_) ESTree::RestElementNode(*optElem));
}

Optional<ESTree::EmptyStatementNode *> JSParserImpl::parseEmptyStatement() {
  assert(check(TokenKind::semi));
  auto *empty =
      setLocation(tok_, tok_, new (context_) ESTree::EmptyStatementNode());
  advance();

  return empty;
}

Optional<ESTree::Node *> JSParserImpl::parseExpressionOrLabelledStatement(
    Param param) {
  bool startsWithIdentifier = check(TokenKind::identifier);

  // ES9.0 13.5
  // Lookahead cannot be any of: {, function, async function, class, let [
  // Allow execution to continue because the expression may be parsed,
  // but report an error because it will be ambiguous whether the parse was
  // correct.
  if (checkN(TokenKind::l_brace, TokenKind::rw_function, TokenKind::rw_class)) {
    // There's no need to stop reporting errors.
    sm_.error(
        tok_->getSourceRange(),
        "declaration not allowed as expression statement");
  }

  if (check(letIdent_)) {
    SMLoc letLoc = advance().Start;
    if (check(TokenKind::l_square)) {
      // let [
      sm_.error(
          {letLoc, tok_->getEndLoc()},
          "ambiguous 'let [': either a 'let' binding or a member expression");
    }
    lexer_.seek(letLoc);
    advance();
  }

  auto optExpr = parseExpression();
  if (!optExpr)
    return None;

  // Check whether this is a label. The expression must have started with an
  // identifier, be just an identifier and be
  // followed by ':'
  if (startsWithIdentifier && isa<ESTree::IdentifierNode>(optExpr.getValue()) &&
      checkAndEat(TokenKind::colon)) {
    auto *id = cast<ESTree::IdentifierNode>(optExpr.getValue());

    ESTree::Node *body = nullptr;
    if (check(TokenKind::rw_function)) {
      auto optFunc = parseFunctionDeclaration(param);
      if (!optFunc)
        return None;
      /// ES9.0 13.13.1
      /// It is a Syntax Error if any source text matches this rule.
      /// LabelledItem : FunctionDeclaration
      /// NOTE: GeneratorDeclarations are disallowed as part of the grammar
      /// as well, so all FunctionDeclarations are disallowed as labeled
      /// items, except via an AnnexB extension which is unsupported in
      /// Hermes.
      sm_.error(
          optFunc.getValue()->getSourceRange().Start,
          "Function declaration not allowed as body of labeled statement");
      body = optFunc.getValue();
    } else {
      // Statement
      auto optBody = parseStatement(param.get(ParamReturn));
      if (!optBody)
        return None;
      body = optBody.getValue();
    }

    return setLocation(
        id, body, new (context_) ESTree::LabeledStatementNode(id, body));
  } else {
    auto endLoc = optExpr.getValue()->getEndLoc();
    if (!eatSemi(endLoc))
      return None;

    return setLocation(
        optExpr.getValue(),
        endLoc,
        new (context_)
            ESTree::ExpressionStatementNode(optExpr.getValue(), nullptr));
  }
}

Optional<ESTree::IfStatementNode *> JSParserImpl::parseIfStatement(
    Param param) {
  assert(check(TokenKind::rw_if));
  SMLoc startLoc = advance().Start;

  SMLoc condLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "after 'if'",
          "location of 'if'",
          startLoc))
    return None;
  auto optTest = parseExpression();
  if (!optTest)
    return None;
  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "at end of 'if' condition",
          "'if' condition starts here",
          condLoc))
    return None;

  auto optConsequent = parseStatement(param.get(ParamReturn));
  if (!optConsequent)
    return None;

  if (checkAndEat(TokenKind::rw_else)) {
    auto optAlternate = parseStatement(param.get(ParamReturn));
    if (!optAlternate)
      return None;

    return setLocation(
        startLoc,
        optAlternate.getValue(),
        new (context_) ESTree::IfStatementNode(
            optTest.getValue(),
            optConsequent.getValue(),
            optAlternate.getValue()));
  } else {
    return setLocation(
        startLoc,
        optConsequent.getValue(),
        new (context_) ESTree::IfStatementNode(
            optTest.getValue(), optConsequent.getValue(), nullptr));
  }
}

Optional<ESTree::WhileStatementNode *> JSParserImpl::parseWhileStatement(
    Param param) {
  assert(check(TokenKind::rw_while));
  SMLoc startLoc = advance().Start;

  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "after 'while'",
          "location of 'while'",
          startLoc))
    return None;
  auto optTest = parseExpression();
  if (!optTest)
    return None;
  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "at end of 'while' condition",
          "location of 'while'",
          startLoc))
    return None;

  auto optBody = parseStatement(param.get(ParamReturn));
  if (!optBody)
    return None;

  return setLocation(
      startLoc,
      optBody.getValue(),
      new (context_)
          ESTree::WhileStatementNode(optBody.getValue(), optTest.getValue()));
}

Optional<ESTree::DoWhileStatementNode *> JSParserImpl::parseDoWhileStatement(
    Param param) {
  assert(check(TokenKind::rw_do));
  SMLoc startLoc = advance().Start;

  auto optBody = parseStatement(param.get(ParamReturn));
  if (!optBody)
    return None;

  SMLoc whileLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::rw_while,
          JSLexer::AllowRegExp,
          "at end of 'do-while'",
          "'do-while' starts here",
          startLoc))
    return None;

  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "after 'do-while'",
          "location of 'while'",
          whileLoc))
    return None;
  auto optTest = parseExpression();
  if (!optTest)
    return None;
  SMLoc endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "at end of 'do-while' condition",
          "location of 'while'",
          whileLoc))
    return None;

  eatSemi(endLoc, true);

  return setLocation(
      startLoc,
      endLoc,
      new (context_)
          ESTree::DoWhileStatementNode(optBody.getValue(), optTest.getValue()));
}

Optional<ESTree::Node *> JSParserImpl::parseForStatement(Param param) {
  assert(check(TokenKind::rw_for));
  SMLoc startLoc = advance().Start;

  SMLoc lparenLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "after 'for'",
          "location of 'for'",
          startLoc))
    return None;

  ESTree::VariableDeclarationNode *decl = nullptr;
  ESTree::NodePtr expr1 = nullptr;

  if (checkN(TokenKind::rw_var, TokenKind::rw_const, letIdent_)) {
    // Productions valid here:
    //   for ( var/let/const VariableDeclarationList
    //   for ( var/let/const VariableDeclaration
    SMLoc varStartLoc = tok_->getStartLoc();
    auto *declIdent = tok_->getResWordOrIdentifier();
    advance();

    ESTree::NodeList declList;
    if (!parseVariableDeclarationList(Param{}, declList, varStartLoc))
      return None;

    auto endLoc = declList.back().getEndLoc();
    decl = setLocation(
        varStartLoc,
        endLoc,
        new (context_)
            ESTree::VariableDeclarationNode(declIdent, std::move(declList)));
  } else {
    // Productions valid here:
    //   for ( Expression_opt
    //   for ( LeftHandSideExpression

    if (!check(TokenKind::semi)) {
      auto optExpr1 = parseExpression(Param{});
      if (!optExpr1)
        return None;
      expr1 = optExpr1.getValue();
    }
  }

  if (checkN(TokenKind::rw_in, ofIdent_)) {
    // Productions valid here:
    //   for ( var/let/const VariableDeclaration[In] in/of
    //   for ( LeftHandSideExpression in/of

    if (decl && decl->_declarations.size() > 1) {
      lexer_.error(
          decl->getSourceRange(),
          "Only one binding must be declared in a for-in/for-of loop");
      return None;
    }

    // Check for destructuring pattern on the left and reparse it.
    if (expr1 &&
        (isa<ESTree::ArrayExpressionNode>(expr1) ||
         isa<ESTree::ObjectExpressionNode>(expr1))) {
      auto optExpr1 = reparseAssignmentPattern(expr1, false);
      if (!optExpr1)
        return None;
      expr1 = *optExpr1;
    }

    // Remember whether we are parsing for-in or for-of.
    bool const forInLoop = check(TokenKind::rw_in);
    advance();

    auto optRightExpr =
        forInLoop ? parseExpression() : parseAssignmentExpression(ParamIn);

    if (!eat(
            TokenKind::r_paren,
            JSLexer::AllowRegExp,
            "after 'for(... in/of ...'",
            "location of '('",
            lparenLoc))
      return None;

    auto optBody = parseStatement(param.get(ParamReturn));
    if (!optBody || !optRightExpr)
      return None;

    ESTree::Node *node;
    if (forInLoop) {
      node = new (context_) ESTree::ForInStatementNode(
          decl ? decl : expr1, optRightExpr.getValue(), optBody.getValue());
    } else {
      node = new (context_) ESTree::ForOfStatementNode(
          decl ? decl : expr1, optRightExpr.getValue(), optBody.getValue());
    }
    return setLocation(startLoc, optBody.getValue(), node);
  } else if (checkAndEat(TokenKind::semi)) {
    // Productions valid here:
    //   for ( var/let/const VariableDeclarationList[In] ; Expressionopt ;
    //   Expressionopt )
    //       Statement
    //   for ( Expression[In]opt ; Expressionopt ; Expressionopt ) Statement

    if (decl)
      ensureDestructuringInitialized(decl);

    ESTree::NodePtr test = nullptr;
    if (!check(TokenKind::semi)) {
      auto optTest = parseExpression();
      if (!optTest)
        return None;
      test = optTest.getValue();
    }

    if (!eat(
            TokenKind::semi,
            JSLexer::AllowRegExp,
            "after 'for( ... ; ...'",
            "location of '('",
            lparenLoc))
      return None;

    ESTree::NodePtr update = nullptr;
    if (!check(TokenKind::r_paren)) {
      auto optUpdate = parseExpression();
      if (!optUpdate)
        return None;
      update = optUpdate.getValue();
    }

    if (!eat(
            TokenKind::r_paren,
            JSLexer::AllowRegExp,
            "after 'for( ... ; ... ; ...'",
            "location of '('",
            lparenLoc))
      return None;

    auto optBody = parseStatement(param.get(ParamReturn));
    if (!optBody)
      return None;

    return setLocation(
        startLoc,
        optBody.getValue(),
        new (context_) ESTree::ForStatementNode(
            decl ? decl : expr1, test, update, optBody.getValue()));
  } else {
    errorExpected(
        TokenKind::semi,
        TokenKind::rw_in,
        "inside 'for'",
        "location of the 'for'",
        startLoc);
    return None;
  }
}

Optional<ESTree::ContinueStatementNode *>
JSParserImpl::parseContinueStatement() {
  assert(check(TokenKind::rw_continue));
  SMRange loc = advance();

  if (eatSemi(loc.End, true))
    return setLocation(
        loc, loc, new (context_) ESTree::ContinueStatementNode(nullptr));

  if (!need(
          TokenKind::identifier,
          "after 'continue'",
          "location of 'continue'",
          loc.Start))
    return None;
  auto *id = setLocation(
      tok_,
      tok_,
      new (context_) ESTree::IdentifierNode(tok_->getIdentifier(), nullptr));
  advance();

  loc.End = id->getEndLoc();
  if (!eatSemi(loc.End))
    return None;

  return setLocation(
      loc, loc, new (context_) ESTree::ContinueStatementNode(id));
}

Optional<ESTree::BreakStatementNode *> JSParserImpl::parseBreakStatement() {
  assert(check(TokenKind::rw_break));
  SMRange loc = advance();

  if (eatSemi(loc.End, true))
    return setLocation(
        loc, loc, new (context_) ESTree::BreakStatementNode(nullptr));

  if (!need(
          TokenKind::identifier,
          "after 'break'",
          "location of 'break'",
          loc.Start))
    return None;
  auto *id = setLocation(
      tok_,
      tok_,
      new (context_) ESTree::IdentifierNode(tok_->getIdentifier(), nullptr));
  advance();

  loc.End = id->getEndLoc();
  if (!eatSemi(loc.End))
    return None;

  return setLocation(loc, loc, new (context_) ESTree::BreakStatementNode(id));
}

Optional<ESTree::ReturnStatementNode *> JSParserImpl::parseReturnStatement() {
  assert(check(TokenKind::rw_return));
  SMRange loc = advance();

  if (eatSemi(loc.End, true))
    return setLocation(
        loc, loc, new (context_) ESTree::ReturnStatementNode(nullptr));

  auto optArg = parseExpression();
  if (!optArg)
    return None;

  loc.End = optArg.getValue()->getEndLoc();
  if (!eatSemi(loc.End))
    return None;

  return setLocation(
      loc, loc, new (context_) ESTree::ReturnStatementNode(optArg.getValue()));
}

Optional<ESTree::WithStatementNode *> JSParserImpl::parseWithStatement(
    Param param) {
  assert(check(TokenKind::rw_with));
  SMLoc startLoc = advance().Start;

  SMLoc lparenLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "after 'with'",
          "location of 'with'",
          startLoc))
    return None;

  auto optExpr = parseExpression();
  if (!optExpr)
    return None;

  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "after 'with (...'",
          "location of '('",
          lparenLoc))
    return None;

  auto optBody = parseStatement(param.get(ParamReturn));
  if (!optBody)
    return None;

  return setLocation(
      startLoc,
      optBody.getValue(),
      new (context_)
          ESTree::WithStatementNode(optExpr.getValue(), optBody.getValue()));
}

Optional<ESTree::SwitchStatementNode *> JSParserImpl::parseSwitchStatement(
    Param param) {
  assert(check(TokenKind::rw_switch));
  SMLoc startLoc = advance().Start;

  SMLoc lparenLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "after 'switch'",
          "location of 'switch'",
          startLoc))
    return None;

  auto optDiscriminant = parseExpression();
  if (!optDiscriminant)
    return None;

  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "after 'switch (...'",
          "location of '('",
          lparenLoc))
    return None;

  SMLoc lbraceLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_brace,
          JSLexer::AllowRegExp,
          "after 'switch (...)'",
          "'switch' starts here",
          startLoc))
    return None;

  ESTree::NodeList clauseList;
  SMLoc defaultLocation; // location of the 'default' clause

  // Parse the switch body.
  while (!check(TokenKind::r_brace)) {
    SMLoc clauseStartLoc = tok_->getStartLoc();

    ESTree::NodePtr testExpr = nullptr;
    bool ignoreClause = false; // Set to true in error recovery when we want to
                               // parse but ignore the parsed statements.
    ESTree::NodeList stmtList;

    SMLoc caseLoc = tok_->getStartLoc();
    if (checkAndEat(TokenKind::rw_case)) {
      auto optTestExpr = parseExpression();
      if (!optTestExpr)
        return None;
      testExpr = optTestExpr.getValue();
    } else if (checkAndEat(TokenKind::rw_default)) {
      if (defaultLocation.isValid()) {
        lexer_.error(
            clauseStartLoc, "more than one 'default' clause in 'switch'");
        sm_.note(defaultLocation, "first 'default' clause was defined here");

        // We want to continue parsing but ignore the statements.
        ignoreClause = true;
      } else {
        defaultLocation = clauseStartLoc;
      }
    } else {
      errorExpected(
          TokenKind::rw_case,
          TokenKind::rw_default,
          "inside 'switch'",
          "location of 'switch'",
          startLoc);
      return None;
    }

    SMLoc colonLoc =
        tok_->getEndLoc(); // save the location in case the clause is empty
    if (!eat(
            TokenKind::colon,
            JSLexer::AllowRegExp,
            "after 'case ...' or 'default'",
            "location of 'case'/'default'",
            caseLoc))
      return None;

    /// case Expression : StatementList[opt]
    ///                   ^
    if (!parseStatementList(
            param.get(ParamReturn),
            TokenKind::rw_default,
            false,
            AllowImportExport::No,
            stmtList,
            TokenKind::rw_case,
            TokenKind::r_brace))
      return None;

    if (!ignoreClause) {
      auto clauseEndLoc =
          stmtList.empty() ? colonLoc : stmtList.back().getEndLoc();
      clauseList.push_back(*setLocation(
          clauseStartLoc,
          clauseEndLoc,
          new (context_)
              ESTree::SwitchCaseNode(testExpr, std::move(stmtList))));
    }
  }

  SMLoc endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_brace,
          JSLexer::AllowRegExp,
          "at end of 'switch' statement",
          "location of '{'",
          lbraceLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::SwitchStatementNode(
          optDiscriminant.getValue(), std::move(clauseList)));
}

Optional<ESTree::ThrowStatementNode *> JSParserImpl::parseThrowStatement(
    Param param) {
  assert(check(TokenKind::rw_throw));
  SMLoc startLoc = advance().Start;

  if (lexer_.isNewLineBeforeCurrentToken()) {
    lexer_.error(
        tok_->getStartLoc(), "'throw' argument must be on the same line");
    sm_.note(startLoc, "location of the 'throw'");
    return None;
  }

  auto optExpr = parseExpression();
  if (!optExpr)
    return None;

  SMLoc endLoc = optExpr.getValue()->getEndLoc();
  if (!eatSemi(endLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ThrowStatementNode(optExpr.getValue()));
}

Optional<ESTree::TryStatementNode *> JSParserImpl::parseTryStatement(
    Param param) {
  assert(check(TokenKind::rw_try));
  SMLoc startLoc = advance().Start;

  if (!need(TokenKind::l_brace, "after 'try'", "location of 'try'", startLoc))
    return None;
  auto optTryBody = parseBlock(param.get(ParamReturn));
  if (!optTryBody)
    return None;

  ESTree::CatchClauseNode *catchHandler = nullptr;
  ESTree::BlockStatementNode *finallyHandler = nullptr;

  // Parse the optional 'catch' handler.
  SMLoc handlerStartLoc = tok_->getStartLoc();
  if (checkAndEat(TokenKind::rw_catch)) {
    if (!eat(
            TokenKind::l_paren,
            JSLexer::AllowRegExp,
            "after 'catch'",
            "location of 'catch'",
            handlerStartLoc))
      return None;

    ESTree::Node *catchParam;
    if (check(TokenKind::l_square, TokenKind::l_brace)) {
      auto optPattern = parseBindingPattern(Param{});
      if (!optPattern)
        return None;
      catchParam = *optPattern;
    } else {
      auto optIdent = parseBindingIdentifier(Param{});
      if (!optIdent) {
        errorExpected(
            TokenKind::identifier,
            "inside catch list",
            "location of 'catch'",
            handlerStartLoc);
        return None;
      }
      catchParam = *optIdent;
    }

    if (!eat(
            TokenKind::r_paren,
            JSLexer::AllowRegExp,
            "after 'catch (...'",
            "location of 'catch'",
            handlerStartLoc))
      return None;

    if (!need(
            TokenKind::l_brace,
            "after 'catch(...)'",
            "location of 'catch'",
            handlerStartLoc))
      return None;
    auto optCatchBody = parseBlock(param.get(ParamReturn));
    if (!optCatchBody)
      return None;

    catchHandler = setLocation(
        handlerStartLoc,
        optCatchBody.getValue(),
        new (context_)
            ESTree::CatchClauseNode(catchParam, optCatchBody.getValue()));
  }

  // Parse the optional 'finally' handler.
  SMLoc finallyLoc = tok_->getStartLoc();
  if (checkAndEat(TokenKind::rw_finally)) {
    if (!need(
            TokenKind::l_brace,
            "after 'finally'",
            "location of 'finally'",
            finallyLoc))
      return None;
    auto optFinallyBody = parseBlock(param.get(ParamReturn));
    if (!optFinallyBody)
      return None;

    finallyHandler = optFinallyBody.getValue();
  }

  // At least one handler must be present.
  if (!catchHandler && !finallyHandler) {
    errorExpected(
        TokenKind::rw_catch,
        TokenKind::rw_finally,
        "after 'try' block",
        "location of 'try'",
        startLoc);
    return None;
  }

  // Use the last handler's location as the end location.
  SMLoc endLoc =
      finallyHandler ? finallyHandler->getEndLoc() : catchHandler->getEndLoc();
  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::TryStatementNode(
          optTryBody.getValue(), catchHandler, finallyHandler));
}

Optional<ESTree::DebuggerStatementNode *>
JSParserImpl::parseDebuggerStatement() {
  assert(check(TokenKind::rw_debugger));
  SMRange loc = advance();

  if (!eatSemi(loc.End))
    return None;

  return setLocation(loc, loc, new (context_) ESTree::DebuggerStatementNode());
}

Optional<ESTree::Node *> JSParserImpl::parsePrimaryExpression() {
  CHECK_RECURSION;

  switch (tok_->getKind()) {
    case TokenKind::rw_this: {
      auto *res =
          setLocation(tok_, tok_, new (context_) ESTree::ThisExpressionNode());
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::identifier: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_)
              ESTree::IdentifierNode(tok_->getIdentifier(), nullptr));
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::rw_null: {
      auto *res =
          setLocation(tok_, tok_, new (context_) ESTree::NullLiteralNode());
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::rw_true:
    case TokenKind::rw_false: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::BooleanLiteralNode(
              tok_->getKind() == TokenKind::rw_true));
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::numeric_literal: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::NumericLiteralNode(tok_->getNumericLiteral()));
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::string_literal: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::regexp_literal: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::RegExpLiteralNode(
              tok_->getRegExpLiteral()->getBody(),
              tok_->getRegExpLiteral()->getFlags()));
      advance(JSLexer::AllowDiv);
      return res;
    }

    case TokenKind::l_square: {
      auto res = parseArrayLiteral();
      if (!res)
        return None;
      return res.getValue();
    }

    case TokenKind::l_brace: {
      auto res = parseObjectLiteral();
      if (!res)
        return None;
      return res.getValue();
    }

    case TokenKind::l_paren: {
      SMLoc startLoc = advance().Start;

      // Cover "()".
      if (check(TokenKind::r_paren)) {
        SMLoc endLoc = advance().End;
        return setLocation(
            startLoc, endLoc, new (context_) ESTree::CoverEmptyArgsNode());
      }

      ESTree::Node *expr;
      if (check(TokenKind::dotdotdot)) {
        auto optRest = parseBindingRestElement(ParamIn);
        if (!optRest)
          return None;

        expr = setLocation(
            *optRest,
            *optRest,
            new (context_) ESTree::CoverRestElementNode(*optRest));
      } else {
        auto optExpr = parseExpression();
        if (!optExpr)
          return None;
        expr = *optExpr;
      }

      if (!eat(
              TokenKind::r_paren,
              JSLexer::AllowDiv,
              "at end of parenthesized expression",
              "started here",
              startLoc))
        return None;
      // Record the number of parens surrounding an expression.
      expr->incParens();
      return expr;
    }

    case TokenKind::rw_function: {
      auto fExpr = parseFunctionExpression();
      if (!fExpr)
        return None;
      return fExpr.getValue();
    }

    case TokenKind::rw_class: {
      auto optClass = parseClassExpression();
      if (!optClass)
        return None;
      return optClass.getValue();
    }

    case TokenKind::no_substitution_template:
    case TokenKind::template_head: {
      auto optTemplate = parseTemplateLiteral(Param{});
      if (!optTemplate) {
        return None;
      }
      return optTemplate.getValue();
    }

    default:
      lexer_.error(tok_->getStartLoc(), "invalid expression");
      return None;
  }
}

Optional<ESTree::ArrayExpressionNode *> JSParserImpl::parseArrayLiteral() {
  assert(check(TokenKind::l_square));
  SMLoc startLoc = advance().Start;

  ESTree::NodeList elemList;

  bool trailingComma = false;

  if (!check(TokenKind::r_square)) {
    for (;;) {
      // Elision.
      if (check(TokenKind::comma)) {
        elemList.push_back(
            *setLocation(tok_, tok_, new (context_) ESTree::EmptyNode()));
      } else if (check(TokenKind::dotdotdot)) {
        // Spread.
        auto optSpread = parseSpreadElement();
        if (!optSpread)
          return None;

        elemList.push_back(**optSpread);
      } else {
        auto expr = parseAssignmentExpression();
        if (!expr)
          return None;
        elemList.push_back(*expr.getValue());
      }

      if (!checkAndEat(TokenKind::comma))
        break;
      if (check(TokenKind::r_square)) {
        // Check for ",]".
        trailingComma = true;
        break;
      }
    }
  }

  SMLoc endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_square,
          JSLexer::AllowDiv,
          "at end of array literal '[...'",
          "location of '['",
          startLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_)
          ESTree::ArrayExpressionNode(std::move(elemList), trailingComma));
}

Optional<ESTree::ObjectExpressionNode *> JSParserImpl::parseObjectLiteral() {
  assert(check(TokenKind::l_brace));
  SMLoc startLoc = advance().Start;

  ESTree::NodeList elemList;

  if (!check(TokenKind::r_brace)) {
    for (;;) {
      if (check(TokenKind::dotdotdot)) {
        // Spread.
        auto optSpread = parseSpreadElement();
        if (!optSpread)
          return None;

        elemList.push_back(**optSpread);
      } else {
        auto prop = parsePropertyAssignment(false);
        if (!prop)
          return None;

        elemList.push_back(*prop.getValue());
      }

      if (!checkAndEat(TokenKind::comma))
        break;
      if (check(TokenKind::r_brace)) // check for ",}"
        break;
    }
  }

  SMLoc endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_brace,
          JSLexer::AllowDiv,
          "at end of object literal '{...'",
          "location of '{'",
          startLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ObjectExpressionNode(std::move(elemList)));
}

Optional<ESTree::Node *> JSParserImpl::parseSpreadElement() {
  assert(check(TokenKind::dotdotdot) && "SpreadElement must start with '...'");
  auto spreadStartLoc = advance();

  auto optExpr = parseAssignmentExpression();
  if (!optExpr)
    return None;

  return setLocation(
      spreadStartLoc,
      *optExpr,
      new (context_) ESTree::SpreadElementNode(*optExpr));
}

Optional<ESTree::Node *> JSParserImpl::parsePropertyAssignment(bool eagerly) {
  SMLoc startLoc = tok_->getStartLoc();
  ESTree::NodePtr key = nullptr;

  SaveStrictMode saveStrictMode{this};

  bool computed = false;
  bool generator = false;

  if (check(getIdent_)) {
    UniqueString *ident = tok_->getResWordOrIdentifier();
    SMRange identRng = tok_->getSourceRange();
    advance();

    // This could either be a getter, or a property named 'get'.
    if (check(TokenKind::colon, TokenKind::l_paren)) {
      // This is just a property.
      key = setLocation(
          identRng,
          identRng,
          new (context_) ESTree::IdentifierNode(ident, nullptr));
    } else {
      // A getter method.
      computed = check(TokenKind::l_square);
      auto optKey = parsePropertyName();
      if (!optKey)
        return None;

      if (!eat(
              TokenKind::l_paren,
              JSLexer::AllowRegExp,
              "in getter declaration",
              "start of getter declaration",
              startLoc))
        return None;
      if (!eat(
              TokenKind::r_paren,
              JSLexer::AllowRegExp,
              "in empty getter parameter list",
              "start of getter declaration",
              startLoc))
        return None;
      if (!need(
              TokenKind::l_brace,
              "in getter declaration",
              "start of getter declaration",
              startLoc))
        return None;
      auto block =
          parseFunctionBody(ParamReturn, eagerly, JSLexer::AllowRegExp, true);
      if (!block)
        return None;

      auto *funcExpr = new (context_) ESTree::FunctionExpressionNode(
          nullptr, ESTree::NodeList{}, block.getValue(), false);
      funcExpr->strictness = ESTree::makeStrictness(isStrictMode());
      funcExpr->isMethodDefinition = true;
      setLocation(startLoc, block.getValue(), funcExpr);

      auto *node = new (context_) ESTree::PropertyNode(
          optKey.getValue(), funcExpr, getIdent_, computed);
      return setLocation(startLoc, block.getValue(), node);
    }
  } else if (check(setIdent_)) {
    UniqueString *ident = tok_->getResWordOrIdentifier();
    SMRange identRng = tok_->getSourceRange();
    advance();

    // This could either be a setter, or a property named 'set'.
    if (check(TokenKind::colon, TokenKind::l_paren)) {
      // This is just a property.
      key = setLocation(
          identRng,
          identRng,
          new (context_) ESTree::IdentifierNode(ident, nullptr));
    } else {
      // A setter method.
      computed = check(TokenKind::l_square);
      auto optKey = parsePropertyName();
      if (!optKey)
        return None;

      ESTree::NodeList params;
      eat(TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "in setter declaration",
          "start of setter declaration",
          startLoc);

      if (!need(
              TokenKind::identifier,
              "in setter parameter list",
              "start of setter declaration",
              startLoc))
        return None;
      auto *param = setLocation(
          tok_,
          tok_,
          new (context_)
              ESTree::IdentifierNode(tok_->getIdentifier(), nullptr));
      params.push_back(*param);
      advance();

      if (!eat(
              TokenKind::r_paren,
              JSLexer::AllowRegExp,
              "at end of setter parameter list",
              "start of setter declaration",
              startLoc))
        return None;
      if (!need(
              TokenKind::l_brace,
              "in setter declaration",
              "start of setter declaration",
              startLoc))
        return None;
      auto block =
          parseFunctionBody(ParamReturn, eagerly, JSLexer::AllowRegExp, true);
      if (!block)
        return None;

      auto *funcExpr = new (context_) ESTree::FunctionExpressionNode(
          nullptr, std::move(params), block.getValue(), false);
      funcExpr->strictness = ESTree::makeStrictness(isStrictMode());
      funcExpr->isMethodDefinition = true;
      setLocation(startLoc, block.getValue(), funcExpr);

      auto *node = new (context_) ESTree::PropertyNode(
          optKey.getValue(), funcExpr, setIdent_, computed);
      return setLocation(startLoc, block.getValue(), node);
    }
  } else if (check(TokenKind::identifier)) {
    auto *ident = tok_->getIdentifier();
    key = setLocation(
        tok_, tok_, new (context_) ESTree::IdentifierNode(ident, nullptr));
    advance();
    // If the next token is "," or "}", this is a shorthand property definition.
    if (check(TokenKind::comma, TokenKind::r_brace)) {
      auto *value = setLocation(
          key, key, new (context_) ESTree::IdentifierNode(ident, nullptr));

      return setLocation(
          startLoc,
          value,
          new (context_) ESTree::PropertyNode(key, value, initIdent_, false));
    }
  } else {
    generator = checkAndEat(TokenKind::star);
    computed = check(TokenKind::l_square);
    auto optKey = parsePropertyName();
    if (!optKey)
      return None;

    key = optKey.getValue();
  }

  ESTree::Node *value;

  if (isa<ESTree::IdentifierNode>(key) && check(TokenKind::equal)) {
    // Check for CoverInitializedName: IdentifierReference Initializer
    auto startLoc = advance().Start;
    auto optInit = parseAssignmentExpression();
    if (!optInit)
      return None;

    value = setLocation(
        startLoc,
        *optInit,
        new (context_) ESTree::CoverInitializerNode(*optInit));
  } else if (check(TokenKind::l_paren)) {
    // Parse the MethodDefinition manually here.
    // Do not use `parseMethodDefinition` because we had to parsePropertyName
    // in this function ourselves and check for SingleNameBindings, which are
    // not parsed with `parsePropertyName`.
    // MethodDefinition:
    // PropertyName "(" UniqueFormalParameters ")" "{" FunctionBody "}"
    //               ^
    llvm::SaveAndRestore<bool> oldParamYield(paramYield_, generator);

    // (
    if (!need(
            TokenKind::l_paren,
            "in method definition",
            "start of method definition",
            startLoc))
      return None;

    ESTree::NodeList args{};
    if (!parseFormalParameters(Param{}, args))
      return None;

    if (!need(
            TokenKind::l_brace,
            "in method definition",
            "start of method definition",
            startLoc))
      return None;
    auto optBody =
        parseFunctionBody(ParamReturn, eagerly, JSLexer::AllowRegExp, true);
    if (!optBody)
      return None;

    auto *funcExpr = new (context_) ESTree::FunctionExpressionNode(
        nullptr, std::move(args), optBody.getValue(), generator);
    funcExpr->strictness = ESTree::makeStrictness(isStrictMode());
    funcExpr->isMethodDefinition = true;
    setLocation(startLoc, optBody.getValue(), funcExpr);

    value = funcExpr;
  } else {
    if (!eat(
            TokenKind::colon,
            JSLexer::AllowRegExp,
            "in property initialization",
            "start of property initialization",
            startLoc))
      return None;

    auto optValue = parseAssignmentExpression();
    if (!optValue)
      return None;
    value = *optValue;
  }

  return setLocation(
      startLoc,
      value,
      new (context_) ESTree::PropertyNode(key, value, initIdent_, computed));
}

Optional<ESTree::Node *> JSParserImpl::parsePropertyName() {
  switch (tok_->getKind()) {
    case TokenKind::string_literal: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));
      advance();
      return res;
    }

    case TokenKind::numeric_literal: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::NumericLiteralNode(tok_->getNumericLiteral()));
      advance();
      return res;
    }

    case TokenKind::identifier: {
      auto *res = setLocation(
          tok_,
          tok_,
          new (context_)
              ESTree::IdentifierNode(tok_->getIdentifier(), nullptr));
      advance();
      return res;
    }

    case TokenKind::l_square: {
      SMLoc start = advance().Start;
      auto optExpr = parseAssignmentExpression(ParamIn);
      if (!optExpr) {
        return None;
      }
      if (!need(
              TokenKind::r_square,
              "at end of computed property key",
              "start of property key",
              start)) {
        return llvm::None;
      }
      advance();
      return *optExpr;
    }

    default:
      if (tok_->isResWord()) {
        auto *res = setLocation(
            tok_,
            tok_,
            new (context_)
                ESTree::IdentifierNode(tok_->getResWordIdentifier(), nullptr));
        advance();
        return res;
      } else {
        lexer_.error(
            tok_->getSourceRange(),
            "invalid property name - must be a string, number or identifier");
        return None;
      }
  }
}

Optional<ESTree::Node *> JSParserImpl::parseTemplateLiteral(Param param) {
  assert(checkTemplateLiteral() && "invalid template literal start");

  SMLoc start = tok_->getStartLoc();

  ESTree::NodeList quasis;
  ESTree::NodeList expressions;

  /// Push the current TemplateElement onto quasis and advance the lexer.
  /// \param tail true if pushing the last element.
  /// \return false on failure.
  auto pushTemplateElement = [&quasis, &param, this](bool tail) -> bool {
    if (tok_->getTemplateLiteralContainsNotEscapes() &&
        !param.has(ParamTagged)) {
      sm_.error(
          tok_->getSourceRange(),
          "untagged template literal contains invalid escape sequence");
      return false;
    }
    auto *quasi = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::TemplateElementNode(
            tail, tok_->getTemplateValue(), tok_->getTemplateRawValue()));
    quasis.push_back(*quasi);
    return true;
  };

  // TemplateSpans
  while (
      !check(TokenKind::no_substitution_template, TokenKind::template_tail)) {
    // TemplateSpans
    // Alternate TemplateMiddle and Expression until TemplateTail.
    if (!check(TokenKind::template_head, TokenKind::template_middle)) {
      sm_.error(tok_->getSourceRange(), "expected template literal");
      return None;
    }

    // First, push the TemplateElement.
    if (!pushTemplateElement(false))
      return None;
    SMLoc subStart = advance().Start;

    // Parse the next expression and add it to the expressions.
    auto optExpr = parseExpression(ParamIn);
    if (!optExpr)
      return None;
    expressions.push_back(*optExpr.getValue());

    if (!check(TokenKind::r_brace)) {
      errorExpected(
          TokenKind::r_brace,
          "at end of substition in template literal",
          "start of substitution",
          subStart);
      return None;
    }

    // The } at the end of the expression must be rescanned as a
    // TemplateMiddle or TemplateTail.
    lexer_.rescanRBraceInTemplateLiteral();
  }

  // TemplateTail or NoSubstitutionTemplate
  if (!pushTemplateElement(true))
    return None;

  return setLocation(
      start,
      advance().End,
      new (context_) ESTree::TemplateLiteralNode(
          std::move(quasis), std::move(expressions)));
}

Optional<ESTree::FunctionExpressionNode *>
JSParserImpl::parseFunctionExpression(bool forceEagerly) {
  auto optRes = parseFunctionHelper(Param{}, false, forceEagerly);
  if (!optRes)
    return None;
  return cast<ESTree::FunctionExpressionNode>(*optRes);
}

Optional<ESTree::Node *> JSParserImpl::parseMemberExpressionExceptNew() {
  SMLoc startLoc = tok_->getStartLoc();

  ESTree::NodePtr expr;
  bool allowTemplateLiteral = true;

  if (check(TokenKind::rw_super)) {
    // SuperProperty can be used the same way as PrimaryExpression, but
    // must not have a TemplateLiteral immediately after the `super` keyword.
    expr = setLocation(tok_, tok_, new (context_) ESTree::SuperNode());
    advance();
    allowTemplateLiteral = false;
  } else {
    auto primExpr = parsePrimaryExpression();
    if (!primExpr)
      return None;
    expr = primExpr.getValue();
  }

  SMLoc objectLoc = startLoc;
  while (check(TokenKind::l_square, TokenKind::period) ||
         checkTemplateLiteral()) {
    SMLoc nextObjectLoc = tok_->getStartLoc();
    if (check(TokenKind::l_square, TokenKind::period)) {
      // MemberExpression [ Expression ]
      // MemberExpression . IdentifierName
      auto msel = parseMemberSelect(objectLoc, expr);
      if (!msel)
        return None;
      objectLoc = nextObjectLoc;
      expr = msel.getValue();
    } else {
      assert(checkTemplateLiteral());
      if (!allowTemplateLiteral) {
        sm_.error(
            expr->getSourceRange(),
            "invalid use of 'super' as a template literal tag");
        return None;
      }
      // MemberExpression TemplateLiteral
      auto optTemplate = parseTemplateLiteral(ParamTagged);
      if (!optTemplate)
        return None;
      expr = setLocation(
          expr,
          optTemplate.getValue(),
          new (context_) ESTree::TaggedTemplateExpressionNode(
              expr, optTemplate.getValue()));
      objectLoc = nextObjectLoc;
    }
    allowTemplateLiteral = true;
  }

  return expr;
}

Optional<const char *> JSParserImpl::parseArguments(
    ESTree::NodeList &argList,
    SMLoc &endLoc) {
  assert(check(TokenKind::l_paren));
  SMLoc startLoc = advance().Start;
  if (!check(TokenKind::r_paren)) {
    for (;;) {
      SMLoc argStart = tok_->getStartLoc();
      bool isSpread = checkAndEat(TokenKind::dotdotdot);

      auto arg = parseAssignmentExpression();
      if (!arg)
        return None;

      if (isSpread) {
        argList.push_back(*setLocation(
            argStart,
            arg.getValue(),
            new (context_) ESTree::SpreadElementNode(arg.getValue())));
      } else {
        argList.push_back(*arg.getValue());
      }

      if (!checkAndEat(TokenKind::comma))
        break;

      // Check for ",)".
      if (check(TokenKind::r_paren))
        break;
    }
  }
  endLoc = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowDiv,
          "at end of function call",
          "location of '('",
          startLoc))
    return None;

  return "OK";
}

Optional<ESTree::Node *> JSParserImpl::parseMemberSelect(
    SMLoc objectLoc,
    ESTree::NodePtr expr) {
  assert(check(TokenKind::l_square, TokenKind::period));
  SMLoc startLoc = tok_->getStartLoc();
  if (checkAndEat(TokenKind::l_square)) {
    auto propExpr = parseExpression();
    if (!propExpr)
      return None;
    SMLoc endLoc = tok_->getEndLoc();
    if (!eat(
            TokenKind::r_square,
            JSLexer::AllowDiv,
            "at end of member expression '[...'",
            "location iof '['",
            startLoc))
      return None;

    return setLocation(
        expr,
        endLoc,
        startLoc,
        new (context_)
            ESTree::MemberExpressionNode(expr, propExpr.getValue(), true));
  } else if (checkAndEat(TokenKind::period)) {
    if (tok_->getKind() != TokenKind::identifier && !tok_->isResWord()) {
      // Just use the pattern here, even though we know it will fail.
      if (!need(
              TokenKind::identifier,
              "after '.' in member expression",
              "start of member expression",
              objectLoc))
        return None;
    }

    auto *id = setLocation(
        tok_,
        tok_,
        new (context_)
            ESTree::IdentifierNode(tok_->getResWordOrIdentifier(), nullptr));
    advance(JSLexer::AllowDiv);

    return setLocation(
        expr,
        id,
        startLoc,
        new (context_) ESTree::MemberExpressionNode(expr, id, false));
  } else {
    assert(false);
    return None;
  }
}

Optional<ESTree::Node *> JSParserImpl::parseCallExpression(
    SMLoc startLoc,
    ESTree::NodePtr expr) {
  assert(checkN(
      TokenKind::l_paren,
      TokenKind::no_substitution_template,
      TokenKind::template_head));

  for (;;) {
    if (check(TokenKind::l_paren)) {
      auto debugLoc = tok_->getStartLoc();
      ESTree::NodeList argList;
      SMLoc endLoc;
      if (!parseArguments(argList, endLoc))
        return None;

      expr = setLocation(
          expr,
          endLoc,
          debugLoc,
          new (context_) ESTree::CallExpressionNode(expr, std::move(argList)));
    } else if (check(TokenKind::l_square, TokenKind::period)) {
      SMLoc nextStartLoc = tok_->getStartLoc();
      auto msel = parseMemberSelect(startLoc, expr);
      if (!msel)
        return None;
      startLoc = nextStartLoc;
      expr = msel.getValue();
    } else if (check(
                   TokenKind::no_substitution_template,
                   TokenKind::template_head)) {
      auto debugLoc = tok_->getStartLoc();
      auto optTemplate = parseTemplateLiteral(ParamTagged);
      if (!optTemplate)
        return None;
      expr = setLocation(
          expr,
          optTemplate.getValue(),
          debugLoc,
          new (context_) ESTree::TaggedTemplateExpressionNode(
              expr, optTemplate.getValue()));
    } else {
      break;
    }
  }

  return expr;
}

Optional<ESTree::Node *> JSParserImpl::parseNewExpressionOrMemberExpression() {
  if (!check(TokenKind::rw_new))
    return parseMemberExpressionExceptNew();

  SMRange newRange = advance();

  if (checkAndEat(TokenKind::period)) {
    // NewTarget: new . target
    //                  ^
    if (!check(targetIdent_)) {
      sm_.error(
          tok_->getSourceRange(), "'target' expected in member expression");
      sm_.note(newRange.Start, "start of member expression");
      return None;
    }
    auto *meta = setLocation(
        newRange,
        newRange,
        new (context_) ESTree::IdentifierNode(newIdent_, nullptr));
    auto *prop = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::IdentifierNode(targetIdent_, nullptr));
    advance();
    return setLocation(
        meta, prop, new (context_) ESTree::MetaPropertyNode(meta, prop));
  }

  auto optExpr = parseNewExpressionOrMemberExpression();
  if (!optExpr)
    return None;
  ESTree::NodePtr expr = optExpr.getValue();

  // Do we have arguments to a child MemberExpression? If yes, then it really
  // was a 'new MemberExpression(args)', otherwise it is a NewExpression
  if (!check(TokenKind::l_paren)) {
    return setLocation(
        newRange,
        expr,
        new (context_) ESTree::NewExpressionNode(expr, ESTree::NodeList{}));
  }

  auto debugLoc = tok_->getStartLoc();
  ESTree::NodeList argList;
  SMLoc endLoc;
  if (!parseArguments(argList, endLoc))
    return None;

  expr = setLocation(
      newRange,
      endLoc,
      debugLoc,
      new (context_) ESTree::NewExpressionNode(expr, std::move(argList)));

  SMLoc objectLoc = newRange.Start;
  while (check(TokenKind::l_square, TokenKind::period)) {
    SMLoc nextObjectLoc = tok_->getStartLoc();
    auto optMSel = parseMemberSelect(objectLoc, expr);
    if (!optMSel)
      return None;
    objectLoc = nextObjectLoc;
    expr = optMSel.getValue();
  }

  return expr;
}

Optional<ESTree::Node *> JSParserImpl::parseLeftHandSideExpression() {
  SMLoc startLoc = tok_->getStartLoc();

  auto optExpr = parseNewExpressionOrMemberExpression();
  if (!optExpr)
    return None;
  auto *expr = optExpr.getValue();

  // Is this a CallExpression?
  if (checkN(
          TokenKind::l_paren,
          TokenKind::no_substitution_template,
          TokenKind::template_head)) {
    auto optCallExpr = parseCallExpression(startLoc, expr);
    if (!optCallExpr)
      return None;
    expr = optCallExpr.getValue();
  }

  return expr;
}

Optional<ESTree::Node *> JSParserImpl::parsePostfixExpression() {
  auto optLHandExpr = parseLeftHandSideExpression();
  if (!optLHandExpr)
    return None;

  if (check(TokenKind::plusplus, TokenKind::minusminus) &&
      !lexer_.isNewLineBeforeCurrentToken()) {
    auto *res = setLocation(
        optLHandExpr.getValue(),
        tok_,
        tok_,
        new (context_) ESTree::UpdateExpressionNode(
            getTokenIdent(tok_->getKind()), optLHandExpr.getValue(), false));
    advance(JSLexer::AllowDiv);
    return res;
  } else {
    return optLHandExpr.getValue();
  }
}

Optional<ESTree::Node *> JSParserImpl::parseUnaryExpression() {
  SMLoc startLoc = tok_->getStartLoc();

  switch (tok_->getKind()) {
    case TokenKind::rw_delete:
    case TokenKind::rw_void:
    case TokenKind::rw_typeof:
    case TokenKind::plus:
    case TokenKind::minus:
    case TokenKind::tilde:
    case TokenKind::exclaim: {
      UniqueString *op = getTokenIdent(tok_->getKind());
      advance();
      auto expr = parseUnaryExpression();
      if (!expr)
        return None;

      if (check(TokenKind::starstar)) {
        // ExponentiationExpression only allows UpdateExpressionNode on the
        // left. The simplest way to enforce that the left operand is not
        // an unparenthesized UnaryExpression is to check here.
        sm_.error(
            {startLoc, tok_->getEndLoc()},
            "Unary operator before ** must use parens to disambiguate");
      }

      return setLocation(
          startLoc,
          expr.getValue(),
          new (context_)
              ESTree::UnaryExpressionNode(op, expr.getValue(), true));
    }

    case TokenKind::plusplus:
    case TokenKind::minusminus: {
      UniqueString *op = getTokenIdent(tok_->getKind());
      advance();
      auto expr = parseUnaryExpression();
      if (!expr)
        return None;

      return setLocation(
          startLoc,
          expr.getValue(),
          new (context_)
              ESTree::UpdateExpressionNode(op, expr.getValue(), true));
    }

    default:
      return parsePostfixExpression();
  }
}

namespace {

/// Associates precedence levels with binary operators. Higher precedences are
/// represented by higher values.
/// \returns the precedence level starting from 1, or 0 if not a binop.
inline unsigned getPrecedence(TokenKind kind) {
  // Record the precedence of all binary operators.
  static const unsigned precedence[] = {
#define TOK(...) 0,
#define BINOP(name, str, precedence) precedence,

// There are two reserved words that are binary operators.
#define RESWORD(name)                                       \
  (TokenKind::rw_##name == TokenKind::rw_in ||              \
           TokenKind::rw_##name == TokenKind::rw_instanceof \
       ? 7                                                  \
       : 0),
#include "hermes/Parser/TokenKinds.def"
  };

  return precedence[static_cast<unsigned>(kind)];
}

/// \return true if \p kind is left associative, false if right associative.
inline bool isLeftAssoc(TokenKind kind) {
  return kind != TokenKind::starstar;
}

/// Return the precedence of \p kind unless it happens to be equal to \p except,
/// in which case return 0.
inline unsigned getPrecedenceExcept(TokenKind kind, TokenKind except) {
  return LLVM_LIKELY(kind != except) ? getPrecedence(kind) : 0;
}
} // namespace

Optional<ESTree::Node *> JSParserImpl::parseBinaryExpression(Param param) {
  // The stack can never go deeper than the number of precedence levels,
  // and we have 10.
  static const unsigned STACK_SIZE = 16;

  // Operator and value stack.
  ESTree::NodePtr valueStack[STACK_SIZE];
  TokenKind opStack[STACK_SIZE];

  // The stack grows down, because it is more natural to point one past the end
  // of an array, rather than one before.
  unsigned sp = STACK_SIZE;

  // Decide whether to recognize "in" as a binary operator.
  const TokenKind exceptKind =
      !param.has(ParamIn) ? TokenKind::rw_in : TokenKind::none;

  auto optExpr = parseUnaryExpression();
  if (!optExpr)
    return None;
  ESTree::NodePtr topExpr = optExpr.getValue();

  // While the current token is a binary operator.
  while (unsigned precedence =
             getPrecedenceExcept(tok_->getKind(), exceptKind)) {
    // If the next operator has no greater precedence than the operator on the
    // stack, pop the stack, creating a new binary expression.
    while (sp != STACK_SIZE && precedence <= getPrecedence(opStack[sp])) {
      if (precedence == getPrecedence(opStack[sp]) &&
          !isLeftAssoc(opStack[sp])) {
        // If the precedences are equal, then we avoid popping for
        // right-associative operators to allow for the entire right-associative
        // expression to be built from the right.
        break;
      }
      topExpr = newBinNode(valueStack[sp], opStack[sp], topExpr);
      ++sp;
    }

    // The next operator has a higher precedence than the previous one (or there
    // is no previous one). The situation looks something like this:
    //    .... + topExpr * rightExpr ....
    //                     ^
    //                 We are here
    // Push topExpr and the '*', so we can parse rightExpr.
    --sp;
    opStack[sp] = tok_->getKind();
    advance();

    auto optRightExpr = parseUnaryExpression();
    if (!optRightExpr)
      return None;

    valueStack[sp] = topExpr;
    topExpr = optRightExpr.getValue();
  }

  // We have consumed all binary operators. Pop the stack, creating expressions.
  while (sp != STACK_SIZE) {
    topExpr = newBinNode(valueStack[sp], opStack[sp], topExpr);
    ++sp;
  }

  assert(sp == STACK_SIZE);
  return topExpr;
}

Optional<ESTree::Node *> JSParserImpl::parseConditionalExpression(Param param) {
  auto optTest = parseBinaryExpression(param);
  if (!optTest)
    return None;

  SMLoc questionLoc = tok_->getStartLoc();
  if (!checkAndEat(TokenKind::question))
    return optTest.getValue();

  auto optConsequent = parseAssignmentExpression(ParamIn);
  if (!optConsequent)
    return None;

  if (!eat(
          TokenKind::colon,
          JSLexer::AllowRegExp,
          "in conditional expression after '... ? ...'",
          "location of '?'",
          questionLoc))
    return None;

  auto optAlternate = parseAssignmentExpression(param);
  if (!optAlternate)
    return None;

  return setLocation(
      optTest.getValue(),
      optAlternate.getValue(),
      new (context_) ESTree::ConditionalExpressionNode(
          optTest.getValue(),
          optAlternate.getValue(),
          optConsequent.getValue()));
}

Optional<ESTree::YieldExpressionNode *> JSParserImpl::parseYieldExpression(
    Param param) {
  assert(
      paramYield_ && check(TokenKind::rw_yield, TokenKind::identifier) &&
      tok_->getResWordOrIdentifier() == yieldIdent_ &&
      "yield expression must start with 'yield'");
  SMRange yieldLoc = advance();

  if (eatSemi(yieldLoc.End, true) || checkEndAssignmentExpression())
    return setLocation(
        yieldLoc,
        yieldLoc,
        new (context_) ESTree::YieldExpressionNode(nullptr, false));

  bool delegate = checkAndEat(TokenKind::star);

  auto optArg = parseAssignmentExpression();
  if (!optArg)
    return None;

  return setLocation(
      yieldLoc,
      optArg.getValue(),
      new (context_) ESTree::YieldExpressionNode(optArg.getValue(), delegate));
}

Optional<ESTree::ClassDeclarationNode *> JSParserImpl::parseClassDeclaration(
    Param param) {
  assert(check(TokenKind::rw_class) && "class must start with 'class'");
  // NOTE: Class definition is always strict mode code.
  SaveStrictMode saveStrictMode{this};
  setStrictMode(true);

  SMLoc startLoc = advance().Start;

  ESTree::Node *name = nullptr;

  if (!check(TokenKind::rw_extends, TokenKind::l_brace)) {
    auto optName = parseBindingIdentifier(Param{});
    if (!optName) {
      errorExpected(
          TokenKind::identifier,
          "in class declaration",
          "location of 'class'",
          startLoc);
      return None;
    }
    name = *optName;
  } else if (!param.has(ParamDefault)) {
    // Identifier is required unless we have +Default parameter.
    errorExpected(
        TokenKind::identifier,
        "after 'class'",
        "location of 'class'",
        startLoc);
    return None;
  }

  ESTree::NodePtr superClass = nullptr;
  auto optBody = parseClassTail(startLoc, superClass);
  if (!optBody)
    return None;

  return setLocation(
      startLoc,
      optBody.getValue(),
      new (context_)
          ESTree::ClassDeclarationNode(name, superClass, optBody.getValue()));
}

Optional<ESTree::ClassExpressionNode *> JSParserImpl::parseClassExpression() {
  assert(check(TokenKind::rw_class) && "class must start with 'class'");
  // NOTE: A class definition is always strict mode code.
  SaveStrictMode saveStrictMode{this};
  setStrictMode(true);

  SMLoc start = advance().Start;

  ESTree::Node *name = nullptr;
  if (!check(TokenKind::rw_extends, TokenKind::l_brace)) {
    // Try to parse a BindingIdentifier if we did not see a ClassHeritage
    // or a '{'.
    auto optName = parseBindingIdentifier(Param{});
    if (!optName) {
      errorExpected(
          TokenKind::identifier,
          "in class expression",
          "location of 'class'",
          start);
      return None;
    }
    name = *optName;
  }

  ESTree::NodePtr superClass = nullptr;
  auto optBody = parseClassTail(start, superClass);
  if (!optBody)
    return None;

  return setLocation(
      start,
      optBody.getValue(),
      new (context_)
          ESTree::ClassExpressionNode(name, superClass, optBody.getValue()));
}

Optional<ESTree::ClassBodyNode *> JSParserImpl::parseClassTail(
    SMLoc startLoc,
    ESTree::NodePtr &superClass) {
  if (checkAndEat(TokenKind::rw_extends)) {
    // ClassHeritage[opt] { ClassBody[opt] }
    // ^
    auto optSuperClass = parseLeftHandSideExpression();
    if (!optSuperClass)
      return None;
    superClass = *optSuperClass;
  }

  if (!need(
          TokenKind::l_brace,
          "in class definition",
          "start of class",
          startLoc)) {
    return None;
  }

  return parseClassBody(startLoc);
}

Optional<ESTree::ClassBodyNode *> JSParserImpl::parseClassBody(SMLoc startLoc) {
  assert(check(TokenKind::l_brace) && "class body must begin with '{'");
  SMLoc braceLoc = advance().Start;

  // It is a Syntax Error if PrototypePropertyNameList of ClassElementList
  // contains more than one occurrence of  "constructor".
  ESTree::Node *constructor = nullptr;

  ESTree::NodeList body{};
  while (!check(TokenKind::r_brace)) {
    bool isStatic = false;
    SMRange startRange = tok_->getSourceRange();
    switch (tok_->getKind()) {
      case TokenKind::semi:
        advance();
        break;

      case TokenKind::rw_static:
        // static MethodDefinition
        isStatic = true;
        advance();
        // intentional fallthrough
      default: {
        // MethodDefinition
        auto optMethod = parseMethodDefinition(isStatic, startRange);
        if (!optMethod)
          return None;
        ESTree::MethodDefinitionNode *method = *optMethod;

        if (method->_kind == constructorIdent_) {
          if (constructor) {
            // Cannot have duplicate constructors, but report the error
            // and move on to parse the rest of the class.
            sm_.error(
                method->getSourceRange(), "duplicate constructors in class");
            sm_.note(
                constructor->getSourceRange(), "first constructor definition");
          } else {
            constructor = method;
          }
        }

        body.push_back(*method);
      }
    }
  }

  if (!need(
          TokenKind::r_brace,
          "at end of class definition",
          "start of class",
          startLoc)) {
    return None;
  }

  return setLocation(
      braceLoc,
      advance().End,
      new (context_) ESTree::ClassBodyNode(std::move(body)));
}

Optional<ESTree::MethodDefinitionNode *> JSParserImpl::parseMethodDefinition(
    bool isStatic,
    SMRange startRange,
    bool eagerly) {
  SMLoc startLoc = tok_->getStartLoc();

  enum class SpecialKind {
    None,
    Get,
    Set,
    Generator,
  };

  // Indicates if this method is out of the ordinary.
  // In particular, indicates getters and setters.
  SpecialKind special = SpecialKind::None;

  // When true, call parsePropertyName.
  // Set to false if the identifiers 'get' or 'set' were already parsed as
  // function names instead of as getter/setter specifiers.
  bool doParsePropertyName = true;

  ESTree::Node *prop = nullptr;
  if (check(getIdent_)) {
    SMRange range = advance();
    if (!check(TokenKind::l_paren)) {
      // If we don't see '(' then this was actually a getter.
      special = SpecialKind::Get;
    } else {
      prop = setLocation(
          range,
          range,
          new (context_) ESTree::IdentifierNode(getIdent_, nullptr));
      doParsePropertyName = false;
    }
  } else if (check(setIdent_)) {
    SMRange range = advance();
    if (!check(TokenKind::l_paren)) {
      // If we don't see '(' then this was actually a setter.
      special = SpecialKind::Set;
    } else {
      prop = setLocation(
          range,
          range,
          new (context_) ESTree::IdentifierNode(setIdent_, nullptr));
      doParsePropertyName = false;
    }
  } else if (checkAndEat(TokenKind::star)) {
    special = SpecialKind::Generator;
  } else if (check(TokenKind::l_paren) && isStatic) {
    // We've already parsed 'static', but there is nothing between 'static'
    // and the '(', so it must be used as the PropertyName and not as an
    // indicator for a static function.
    prop = setLocation(
        startRange,
        startRange,
        new (context_) ESTree::IdentifierNode(staticIdent_, nullptr));
    isStatic = false;
    doParsePropertyName = false;
  }

  bool computed = false;
  if (doParsePropertyName) {
    computed = check(TokenKind::l_square);
    auto optProp = parsePropertyName();
    if (!optProp)
      return None;
    prop = *optProp;
  }

  // Store the propName for comparisons, used for SyntaxErrors.
  UniqueString *propName = nullptr;
  if (auto *id = dyn_cast<ESTree::IdentifierNode>(prop)) {
    propName = id->_name;
  } else if (auto *str = dyn_cast<ESTree::StringLiteralNode>(prop)) {
    propName = str->_value;
  }
  bool isConstructor =
      !isStatic && !computed && propName && propName->str() == "constructor";

  // (
  if (!need(
          TokenKind::l_paren,
          "in method definition",
          "start of method definition",
          startLoc))
    return None;
  ESTree::NodeList args{};

  llvm::SaveAndRestore<bool> saveArgsAndBodyParamYield(
      paramYield_, special == SpecialKind::Generator);

  if (!parseFormalParameters(Param{}, args))
    return None;

  if (!need(
          TokenKind::l_brace,
          "in method definition",
          "start of method definition",
          startLoc))
    return None;

  auto optBody =
      parseFunctionBody(ParamReturn, eagerly, JSLexer::AllowRegExp, true);
  if (!optBody)
    return None;

  auto *funcExpr = setLocation(
      startLoc,
      optBody.getValue(),
      new (context_) ESTree::FunctionExpressionNode(
          nullptr,
          std::move(args),
          optBody.getValue(),
          special == SpecialKind::Generator));
  assert(
      isStrictMode() &&
      "parseMethodDefinition should only be used for classes");
  funcExpr->strictness = ESTree::makeStrictness(true);
  funcExpr->isMethodDefinition = true;

  if (special == SpecialKind::Get && funcExpr->_params.size() != 0) {
    sm_.error(
        funcExpr->getSourceRange(),
        Twine("getter method must no one formal arguments, found ") +
            Twine(funcExpr->_params.size()));
  }

  if (special == SpecialKind::Set && funcExpr->_params.size() != 1) {
    sm_.error(
        funcExpr->getSourceRange(),
        Twine("setter method must have exactly one formal argument, found ") +
            Twine(funcExpr->_params.size()));
  }

  if (isStatic && propName && propName->str() == "prototype") {
    // ClassElement : static MethodDefinition
    // It is a Syntax Error if PropName of MethodDefinition is "prototype".
    sm_.error(
        funcExpr->getSourceRange(), "prototype method must not be static");
    return None;
  }

  UniqueString *kind = methodIdent_;
  if (isConstructor) {
    if (special != SpecialKind::None) {
      // It is a Syntax Error if PropName of MethodDefinition is "constructor"
      // and SpecialMethod of MethodDefinition is true.
      // TODO: Account for generator methods in SpecialMethod here.
      sm_.error(
          funcExpr->getSourceRange(),
          "constructor method must not be a getter or setter");
      return None;
    }
    kind = constructorIdent_;
  } else if (special == SpecialKind::Get) {
    kind = getIdent_;
  } else if (special == SpecialKind::Set) {
    kind = setIdent_;
  }

  return setLocation(
      startLoc,
      optBody.getValue(),
      new (context_) ESTree::MethodDefinitionNode(
          prop, funcExpr, kind, computed, isStatic));
}

bool JSParserImpl::reparseArrowParameters(
    ESTree::Node *node,
    ESTree::NodeList &paramList) {
  // Empty argument list "()".
  if (node->getParens() == 0 && isa<ESTree::CoverEmptyArgsNode>(node))
    return true;

  // A single identifier without parens.
  if (node->getParens() == 0 && isa<ESTree::IdentifierNode>(node)) {
    paramList.push_back(*node);
    return true;
  }

  if (node->getParens() != 1) {
    sm_.error(node->getSourceRange(), "invalid arrow function parameter list");
    return false;
  }

  ESTree::NodeList nodeList{};

  if (auto *seqNode = dyn_cast<ESTree::SequenceExpressionNode>(node)) {
    nodeList = std::move(seqNode->_expressions);
  } else {
    node->clearParens();
    nodeList.push_back(*node);
  }

  // If the node has 0 parentheses, return true, otherwise print an error and
  // return false.
  auto checkParens = [this](ESTree::Node *n) {
    if (n->getParens() == 0)
      return true;

    lexer_.error(
        n->getSourceRange(), "parentheses are not allowed around parameters");
    return false;
  };

  for (auto it = nodeList.begin(), e = nodeList.end(); it != e;) {
    auto *expr = &*it;
    it = nodeList.erase(it);

    if (!checkParens(expr))
      continue;

    if (auto *CRE = dyn_cast<ESTree::CoverRestElementNode>(expr)) {
      if (it != e)
        lexer_.error(expr->getSourceRange(), "rest parameter must be last");
      else
        paramList.push_back(*CRE->_rest);
      continue;
    }

    if (isa<ESTree::CoverTrailingCommaNode>(expr)) {
      assert(
          it == e &&
          "CoverTrailingCommaNode should have been only parsed last");
      // Just skip it.
      continue;
    }

    ESTree::Node *init = nullptr;

    // If we encounter an initializer, unpack it.
    if (auto *asn = dyn_cast<ESTree::AssignmentExpressionNode>(expr)) {
      if (asn->_operator == getTokenIdent(TokenKind::equal)) {
        expr = asn->_left;
        init = asn->_right;

        if (!checkParens(expr))
          continue;
      }
    }

    auto optParam = reparseAssignmentPattern(expr, true);
    if (!optParam)
      continue;
    expr = *optParam;

    if (init) {
      expr = setLocation(
          expr, init, new (context_) ESTree::AssignmentPatternNode(expr, init));
    }

    paramList.push_back(*expr);
  }

  return true;
}

Optional<ESTree::Node *> JSParserImpl::parseArrowFunctionExpression(
    Param param,
    ESTree::Node *leftExpr) {
  // ArrowFunction : ArrowParameters [no line terminator] => ConciseBody.
  assert(
      check(TokenKind::equalgreater) && !lexer_.isNewLineBeforeCurrentToken() &&
      "ArrowFunctionExpression expects [no new line] '=>'");

  // Eat the '=>'.
  advance();

  ESTree::NodeList paramList;
  if (!reparseArrowParameters(leftExpr, paramList))
    return None;

  SaveStrictMode saveStrictMode{this};
  ESTree::Node *body;
  bool expression;

  llvm::SaveAndRestore<bool> oldParamYield(paramYield_, false);
  if (check(TokenKind::l_brace)) {
    auto optBody = parseFunctionBody(Param{}, true, JSLexer::AllowDiv, true);
    if (!optBody)
      return None;
    body = *optBody;
    expression = false;
  } else {
    auto optConcise = parseAssignmentExpression(param.get(ParamIn));
    if (!optConcise)
      return None;
    body = *optConcise;
    expression = true;
  }

  auto *arrow = new (context_) ESTree::ArrowFunctionExpressionNode(
      nullptr, std::move(paramList), body, expression);

  arrow->strictness = ESTree::makeStrictness(isStrictMode());
  return setLocation(leftExpr, body, arrow);
}

Optional<ESTree::Node *> JSParserImpl::reparseAssignmentPattern(
    ESTree::Node *node,
    bool inDecl) {
  if (!node->getParens()) {
    if (auto *AEN = dyn_cast<ESTree::ArrayExpressionNode>(node)) {
      return reparseArrayAsignmentPattern(AEN, inDecl);
    }
    if (auto *OEN = dyn_cast<ESTree::ObjectExpressionNode>(node)) {
      return reparseObjectAssignmentPattern(OEN, inDecl);
    }
    if (isa<ESTree::IdentifierNode>(node) || isa<ESTree::PatternNode>(node)) {
      return node;
    }
  }

  if (inDecl) {
    lexer_.error(node->getSourceRange(), "identifier or pattern expected");
    return None;
  }

  return node;
}

Optional<ESTree::Node *> JSParserImpl::reparseArrayAsignmentPattern(
    ESTree::ArrayExpressionNode *AEN,
    bool inDecl) {
  ESTree::NodeList elements{};

  for (auto it = AEN->_elements.begin(), e = AEN->_elements.end(); it != e;) {
    ESTree::Node *elem = &*it++;
    AEN->_elements.remove(*elem);

    // Every element in the array assignment pattern is optional,
    // because we can parse the Elision production.
    if (auto *empty = dyn_cast<ESTree::EmptyNode>(elem)) {
      elements.push_back(*elem);
      continue;
    }

    if (auto *spread = dyn_cast<ESTree::SpreadElementNode>(elem)) {
      if (it != e || AEN->_trailingComma) {
        lexer_.error(spread->getSourceRange(), "rest element must be last");
        continue;
      }

      auto optSubPattern = reparseAssignmentPattern(spread->_argument, inDecl);
      if (!optSubPattern)
        continue;
      elem = setLocation(
          spread,
          *optSubPattern,
          new (context_) ESTree::RestElementNode(*optSubPattern));
    } else {
      ESTree::Node *init = nullptr;

      // If we encounter an initializer, unpack it.
      if (!elem->getParens()) {
        if (auto *asn = dyn_cast<ESTree::AssignmentExpressionNode>(elem)) {
          if (asn->_operator == getTokenIdent(TokenKind::equal)) {
            elem = asn->_left;
            init = asn->_right;
          }
        }
      }

      // Reparse {...} or [...]
      auto optSubPattern = reparseAssignmentPattern(elem, inDecl);
      if (!optSubPattern)
        continue;
      elem = *optSubPattern;

      if (init) {
        elem = setLocation(
            elem,
            init,
            new (context_) ESTree::AssignmentPatternNode(elem, init));
      }
    }

    elements.push_back(*elem);
  }

  return setLocation(
      AEN->getStartLoc(),
      AEN->getEndLoc(),
      new (context_) ESTree::ArrayPatternNode(std::move(elements)));
}

Optional<ESTree::Node *> JSParserImpl::reparseObjectAssignmentPattern(
    ESTree::ObjectExpressionNode *OEN,
    bool inDecl) {
  ESTree::NodeList elements{};

  for (auto it = OEN->_properties.begin(), e = OEN->_properties.end();
       it != e;) {
    auto *node = &*it++;
    OEN->_properties.remove(*node);

    if (auto *spread = dyn_cast<ESTree::SpreadElementNode>(node)) {
      if (it != e) {
        lexer_.error(spread->getSourceRange(), "rest property must be last");
        continue;
      }

      // NOTE: the spec says that AssignmentRestProperty cannot be another
      // pattern (see
      // https://www.ecma-international.org/ecma-262/9.0/index.html#sec-destructuring-assignment-static-semantics-early-errors)
      // even though it would be logical.
#if 0
      auto optSubPattern = reparseAssignmentPattern(spread->_argument, inDecl);
      if (!optSubPattern)
        continue;
      node = *optSubPattern;
#else
      node = spread->_argument;
      if (inDecl) {
        if (!isa<ESTree::IdentifierNode>(node)) {
          lexer_.error(
              node->getSourceRange(), "identifier expected in parameter list");
          continue;
        }
      }
#endif
      node = setLocation(
          spread, node, new (context_) ESTree::RestElementNode(node));
    } else {
      auto *propNode = cast<ESTree::PropertyNode>(node);

      if (propNode->_kind != initIdent_) {
        lexer_.error(
            SourceErrorManager::combineIntoRange(
                propNode->getStartLoc(), propNode->_key->getStartLoc()),
            "invalid destructuring target");
        continue;
      }

      ESTree::Node *value = propNode->_value;
      ESTree::Node *init = nullptr;

      // If we encounter an initializer, unpack it.
      if (auto *asn = dyn_cast<ESTree::AssignmentExpressionNode>(value)) {
        if (asn->_operator == getTokenIdent(TokenKind::equal)) {
          value = asn->_left;
          init = asn->_right;
        }
      } else if (
          auto *coverInitializer =
              dyn_cast<ESTree::CoverInitializerNode>(value)) {
        assert(
            isa<ESTree::IdentifierNode>(propNode->_key) &&
            "CoverInitializedName must start with an identifier");
        // Clone the key.
        value = new (context_) ESTree::IdentifierNode(
            cast<ESTree::IdentifierNode>(propNode->_key)->_name, nullptr);
        value->copyLocationFrom(propNode->_key);

        init = coverInitializer->_init;
      }

      // Reparse {...} or [...]
      auto optSubPattern = reparseAssignmentPattern(value, inDecl);
      if (!optSubPattern)
        continue;
      value = *optSubPattern;

      // If we have an initializer, create an AssignmentPattern.
      if (init) {
        value = new (context_) ESTree::AssignmentPatternNode(value, init);
        value->copyLocationFrom(propNode->_value);
      }

      propNode->_value = value;
    }

    elements.push_back(*node);
  }

  auto *OP = new (context_) ESTree::ObjectPatternNode(std::move(elements));
  OP->copyLocationFrom(OEN);
  return OP;
}

Optional<ESTree::Node *> JSParserImpl::parseAssignmentExpression(Param param) {
  // Check for yield, which may be lexed as a reserved word, but only in strict
  // mode.
  if (paramYield_ && check(TokenKind::rw_yield, TokenKind::identifier) &&
      tok_->getResWordOrIdentifier() == yieldIdent_) {
    auto optYieldExpr = parseYieldExpression();
    if (!optYieldExpr)
      return None;
    assert(
        checkEndAssignmentExpression() &&
        "invalid end token in AssignmentExpression");
    return *optYieldExpr;
  }

  auto optLeftExpr = parseConditionalExpression(param);
  if (!optLeftExpr)
    return None;

  // Check for ArrowFunction.
  //   ArrowFunction : ArrowParameters [no line terminator] => ConciseBody.
  if (check(TokenKind::equalgreater) && !lexer_.isNewLineBeforeCurrentToken()) {
    return parseArrowFunctionExpression(param, *optLeftExpr);
  }

  if (!checkAssign())
    return *optLeftExpr;

  // Check for destructuring assignment.
  if (check(TokenKind::equal) &&
      (isa<ESTree::ArrayExpressionNode>(*optLeftExpr) ||
       isa<ESTree::ObjectExpressionNode>(*optLeftExpr))) {
    optLeftExpr = reparseAssignmentPattern(*optLeftExpr, false);
    if (!optLeftExpr)
      return None;
  }

  UniqueString *op = getTokenIdent(tok_->getKind());
  auto debugLoc = advance().Start;

  auto optRightExpr = parseAssignmentExpression(param);
  if (!optRightExpr)
    return None;

  if (!checkEndAssignmentExpression()) {
    // Note: We don't assert the valid end of an AssignmentExpression here
    // because we do not know yet whether the entire file is well-formed.
    // This check errors here to ensure that we still catch missing elements
    // in `checkEndAssignmentExpression` while allowing us to avoid actually
    // asserting and crashing.
    sm_.error(
        tok_->getStartLoc(), "unexpected token after assignment expression");
    return None;
  }
  return setLocation(
      optLeftExpr.getValue(),
      optRightExpr.getValue(),
      debugLoc,
      new (context_) ESTree::AssignmentExpressionNode(
          op, optLeftExpr.getValue(), optRightExpr.getValue()));
}

Optional<ESTree::Node *> JSParserImpl::parseExpression(Param param) {
  auto optExpr = parseAssignmentExpression(param);
  if (!optExpr)
    return None;

  if (!check(TokenKind::comma))
    return optExpr.getValue();

  ESTree::NodeList exprList;
  exprList.push_back(*optExpr.getValue());

  while (check(TokenKind::comma)) {
    // Eat the ",".
    auto commaRng = advance();

    // CoverParenthesizedExpressionAndArrowParameterList: (Expression ,)
    if (check(TokenKind::r_paren)) {
      auto *coverNode = setLocation(
          commaRng,
          tok_->getStartLoc(),
          new (context_) ESTree::CoverTrailingCommaNode());
      exprList.push_back(*coverNode);
      break;
    }

    ESTree::Node *expr2;

    if (check(TokenKind::dotdotdot)) {
      auto optRest = parseBindingRestElement(param);
      if (!optRest)
        return None;
      expr2 = setLocation(
          *optRest,
          *optRest,
          new (context_) ESTree::CoverRestElementNode(*optRest));
    } else {
      auto optExpr2 = parseAssignmentExpression(param);
      if (!optExpr2)
        return None;
      expr2 = *optExpr2;
    }

    exprList.push_back(*expr2);
  }

  auto *firstExpr = &exprList.front();
  auto *lastExpr = &exprList.back();
  return setLocation(
      firstExpr,
      lastExpr,
      new (context_) ESTree::SequenceExpressionNode(std::move(exprList)));
}

Optional<ESTree::StringLiteralNode *> JSParserImpl::parseFromClause() {
  SMLoc startLoc = tok_->getStartLoc();

  if (!checkAndEat(fromIdent_)) {
    sm_.error(startLoc, "'from' expected");
    return None;
  }

  if (!need(
          TokenKind::string_literal,
          "after 'from'",
          "location of 'from'",
          startLoc)) {
    return None;
  }

  auto *source = setLocation(
      tok_,
      tok_,
      new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));

  advance();
  return source;
}

Optional<ESTree::Node *> JSParserImpl::parseImportDeclaration() {
  assert(
      check(TokenKind::rw_import) &&
      "import declaration must start with 'import'");
  SMLoc startLoc = advance().Start;

  if (check(TokenKind::string_literal)) {
    // import ModuleSpecifier ;
    // If the first token is a string literal, there are no specifiers,
    // so the import clause should not be parsed.
    auto *source = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));
    auto endLoc = advance().End;
    if (!eatSemi(endLoc)) {
      return None;
    }
    return setLocation(
        startLoc,
        endLoc,
        new (context_) ESTree::ImportDeclarationNode({}, source));
  }

  ESTree::NodeList specifiers;
  if (!parseImportClause(specifiers)) {
    return None;
  }

  auto optFromClause = parseFromClause();
  if (!optFromClause) {
    return None;
  }

  SMLoc endLoc = optFromClause.getValue()->getEndLoc();
  if (!eatSemi(endLoc)) {
    return None;
  }

  return setLocation(
      startLoc,
      endLoc,
      new (context_)
          ESTree::ImportDeclarationNode(std::move(specifiers), *optFromClause));
}

bool JSParserImpl::parseImportClause(ESTree::NodeList &specifiers) {
  SMLoc startLoc = tok_->getStartLoc();
  if (check(TokenKind::identifier)) {
    // ImportedDefaultBinding
    // ImportedDefaultBinding , NameSpaceImport
    // ImportedDefaultBinding , NamedImports
    auto optDefaultBinding = parseBindingIdentifier(Param{});
    if (!optDefaultBinding) {
      errorExpected(
          TokenKind::identifier,
          "in import clause",
          "start of import clause",
          startLoc);
      return false;
    }
    SMLoc endLoc = optDefaultBinding.getValue()->getEndLoc();
    specifiers.push_back(*setLocation(
        startLoc,
        endLoc,
        new (context_) ESTree::ImportDefaultSpecifierNode(*optDefaultBinding)));
    if (!checkAndEat(TokenKind::comma)) {
      // If there was no comma, there's no more bindings to parse,
      // so return immediately.
      return true;
    }
  }

  // At this point, either:
  // - the ImportedDefaultBinding was parsed and had a comma after it
  // - there was no ImportedDefaultBinding and we simply continue

  if (check(TokenKind::star)) {
    // NameSpaceImport
    auto optNsImport = parseNameSpaceImport();
    if (!optNsImport) {
      return false;
    }
    specifiers.push_back(*optNsImport.getValue());
    return true;
  }

  // NamedImports is the only remaining possibility.
  if (!need(
          TokenKind::l_brace,
          "in import specifier clause",
          "location of import specifiers",
          startLoc)) {
    return false;
  }

  return parseNamedImports(specifiers);
}

Optional<ESTree::Node *> JSParserImpl::parseNameSpaceImport() {
  assert(check(TokenKind::star) && "import namespace must start with *");

  SMLoc startLoc = advance().Start;
  if (!checkAndEat(asIdent_)) {
    sm_.error(tok_->getStartLoc(), "'as' expected");
    return None;
  }

  auto optLocal = parseBindingIdentifier(Param{});
  if (!optLocal) {
    errorExpected(
        TokenKind::identifier,
        "in namespace import",
        "location of namespace import",
        startLoc);
    return None;
  }

  return setLocation(
      startLoc,
      *optLocal,
      new (context_) ESTree::ImportNamespaceSpecifierNode(*optLocal));
}

bool JSParserImpl::parseNamedImports(ESTree::NodeList &specifiers) {
  assert(check(TokenKind::l_brace) && "named imports must start with {");
  SMLoc startLoc = advance().Start;

  // BoundNames to check for duplicate entries in ImportDeclaration.
  // Values are the actual IdentifierNodes, used for error reporting.
  llvm::DenseMap<UniqueString *, ESTree::IdentifierNode *> boundNames{};

  while (!check(TokenKind::r_brace)) {
    auto optSpecifier = parseImportSpecifier(startLoc);
    if (!optSpecifier) {
      return false;
    }

    // Check if the bound name was duplicated.
    ESTree::IdentifierNode *localIdent =
        cast<ESTree::IdentifierNode>(optSpecifier.getValue()->_local);
    auto insertRes = boundNames.try_emplace(localIdent->_name, localIdent);
    if (insertRes.second) {
      specifiers.push_back(*optSpecifier.getValue());
    } else {
      // Report the error but continue parsing to see if there's any others.
      sm_.error(
          localIdent->getSourceRange(),
          "Duplicate entry in import declaration list");
      sm_.note(
          insertRes.first->second->getSourceRange(), "first usage of name");
    }

    if (!checkAndEat(TokenKind::comma)) {
      break;
    }
  }
  if (!eat(
          TokenKind::r_brace,
          JSLexer::AllowDiv,
          "at end of named imports",
          "location of '{'",
          startLoc)) {
    return false;
  }

  return true;
}

Optional<ESTree::ImportSpecifierNode *> JSParserImpl::parseImportSpecifier(
    SMLoc importLoc) {
  // ImportSpecifier:
  //   ImportedBinding
  //   IdentifierName as ImportedBinding
  SMLoc startLoc = tok_->getStartLoc();

  if (!check(TokenKind::identifier) && !tok_->isResWord()) {
    errorExpected(
        TokenKind::identifier,
        "in import specifier",
        "specifiers start",
        importLoc);
    return None;
  }
  auto *imported = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getResWordOrIdentifier(), nullptr));
  TokenKind localKind = tok_->getKind();

  SMLoc endLoc = advance().End;
  ESTree::IdentifierNode *local = imported;

  if (checkAndEat(asIdent_)) {
    if (!check(TokenKind::identifier) && !tok_->isResWord()) {
      errorExpected(
          TokenKind::identifier,
          "in import specifier",
          "specifiers start",
          importLoc);
      return None;
    }
    local = setLocation(
        tok_,
        tok_,
        new (context_)
            ESTree::IdentifierNode(tok_->getResWordOrIdentifier(), nullptr));
    localKind = tok_->getKind();
    endLoc = advance().End;
  }

  // Only the local name must be parsed as a binding identifier.
  // We need to check for 'as' before knowing what the local name is.
  // Thus, we need to validate the binding identifier for the local name
  // after the fact.
  if (!validateBindingIdentifier(Param{}, local->_name, localKind)) {
    sm_.error(local->getSourceRange(), "Invalid local name for import");
  }

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ImportSpecifierNode(imported, local));
}

Optional<ESTree::Node *> JSParserImpl::parseExportDeclaration() {
  assert(
      check(TokenKind::rw_export) &&
      "parseExportDeclaration requires 'export'");
  SMLoc startLoc = advance().Start;

  if (checkAndEat(TokenKind::star)) {
    // export * FromClause;
    auto optFromClause = parseFromClause();
    if (!optFromClause) {
      return None;
    }
    SMLoc endLoc = optFromClause.getValue()->getEndLoc();
    if (!eatSemi(endLoc)) {
      return None;
    }
    return setLocation(
        startLoc,
        endLoc,
        new (context_) ESTree::ExportAllDeclarationNode(*optFromClause));
  } else if (checkAndEat(TokenKind::rw_default)) {
    // export default
    if (check(TokenKind::rw_function)) {
      // export default HoistableDeclaration
      // Currently, the only hoistable declarations are functions.
      auto optFunDecl = parseFunctionDeclaration(ParamDefault);
      if (!optFunDecl) {
        return None;
      }
      return setLocation(
          startLoc,
          *optFunDecl,
          new (context_) ESTree::ExportDefaultDeclarationNode(*optFunDecl));
    } else if (check(TokenKind::rw_class)) {
      auto optClassDecl = parseClassDeclaration(ParamDefault);
      if (!optClassDecl) {
        return None;
      }
      return setLocation(
          startLoc,
          *optClassDecl,
          new (context_) ESTree::ExportDefaultDeclarationNode(*optClassDecl));
    } else {
      // export default AssignmentExpression ;
      auto optExpr = parseAssignmentExpression(ParamIn);
      if (!optExpr) {
        return None;
      }
      SMLoc endLoc = optExpr.getValue()->getEndLoc();
      if (!eatSemi(endLoc)) {
        return None;
      }
      return setLocation(
          startLoc,
          endLoc,
          new (context_) ESTree::ExportDefaultDeclarationNode(*optExpr));
    }
  } else if (check(TokenKind::l_brace)) {
    // export ExportClause FromClause ;
    // export ExportClause ;
    ESTree::NodeList specifiers{};
    SMLoc endLoc;
    llvm::SmallVector<SMRange, 2> invalids{};

    auto optExportClause = parseExportClause(specifiers, endLoc, invalids);
    if (!optExportClause) {
      return None;
    }

    ESTree::Node *source = nullptr;
    if (check(fromIdent_)) {
      // export ExportClause FromClause ;
      auto optFromClause = parseFromClause();
      if (!optFromClause) {
        return None;
      }
      source = *optFromClause;
      endLoc = source->getEndLoc();
    } else {
      // export ExportClause ;
      // ES9.0 15.2.3.1
      // When there is no FromClause, any ranges added to invalids are actually
      // invalid, and should be reported as errors.
      for (const SMRange &range : invalids) {
        sm_.error(range, "Invalid exported name");
      }
    }

    if (!eatSemi(endLoc)) {
      return None;
    }

    return setLocation(
        startLoc,
        endLoc,
        new (context_) ESTree::ExportNamedDeclarationNode(
            nullptr, std::move(specifiers), source));
  } else if (check(TokenKind::rw_var)) {
    // export VariableStatement
    auto optVar = parseVariableStatement(Param{});
    if (!optVar) {
      return None;
    }
    return setLocation(
        startLoc,
        *optVar,
        new (context_)
            ESTree::ExportNamedDeclarationNode(*optVar, {}, nullptr));
  }

  // export Declaration [~Yield]

  if (!checkDeclaration()) {
    lexer_.error(tok_->getSourceRange(), "expected declaration in export");
    return None;
  }

  auto optDecl = parseDeclaration(Param{});
  if (!optDecl) {
    return None;
  }

  return setLocation(
      startLoc,
      *optDecl,
      new (context_) ESTree::ExportNamedDeclarationNode(*optDecl, {}, nullptr));
}

bool JSParserImpl::parseExportClause(
    ESTree::NodeList &specifiers,
    SMLoc &endLoc,
    llvm::SmallVectorImpl<SMRange> &invalids) {
  // ExportClause:
  //   { }
  //   { ExportsList }
  //   { ExportsList , '}

  assert(check(TokenKind::l_brace) && "ExportClause requires '{'");
  SMLoc startLoc = advance().Start;

  while (!check(TokenKind::r_brace)) {
    // Read all the elements of the ExportsList.
    auto optSpecifier = parseExportSpecifier(startLoc, invalids);
    if (!optSpecifier) {
      return false;
    }
    specifiers.push_back(*optSpecifier.getValue());

    if (!checkAndEat(TokenKind::comma)) {
      break;
    }
  }

  endLoc = tok_->getEndLoc();
  return eat(
      TokenKind::r_brace,
      JSLexer::AllowDiv,
      "at end of export clause",
      "location of export",
      startLoc);
}

Optional<ESTree::Node *> JSParserImpl::parseExportSpecifier(
    SMLoc exportLoc,
    llvm::SmallVectorImpl<SMRange> &invalids) {
  // ExportSpecifier:
  //   IdentifierName
  //   IdentifierName as IdentifierName

  if (!check(TokenKind::identifier) && !tok_->isResWord()) {
    errorExpected(
        TokenKind::identifier,
        "in export clause",
        "location of export clause",
        exportLoc);
  }

  // ES9.0 15.2.3.1 Early errors for ReferencedBindings in ExportClause.
  // Add potentially error-raising identifier ranges to the invalids list here,
  // and the owner of the invalids list will report the ranges as errors if
  // necessary.
  if (tok_->isResWord() || check(implementsIdent_) || check(interfaceIdent_) ||
      check(letIdent_) || check(packageIdent_) || check(privateIdent_) ||
      check(protectedIdent_) || check(publicIdent_) || check(staticIdent_)) {
    invalids.push_back(tok_->getSourceRange());
  }

  auto *local = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getResWordOrIdentifier(), nullptr));
  advance();
  ESTree::Node *exported;
  if (checkAndEat(asIdent_)) {
    // IdentifierName as IdentifierName
    if (!check(TokenKind::identifier) && !tok_->isResWord()) {
      errorExpected(
          TokenKind::identifier,
          "in export clause",
          "location of export clause",
          exportLoc);
    }
    exported = setLocation(
        tok_,
        tok_,
        new (context_)
            ESTree::IdentifierNode(tok_->getResWordOrIdentifier(), nullptr));
    advance();
  } else {
    // IdentifierName
    exported = local;
  }

  return setLocation(
      local,
      exported,
      new (context_) ESTree::ExportSpecifierNode(exported, local));
}

ESTree::ExpressionStatementNode *JSParserImpl::parseDirective() {
  // Is the current token a directive?
  if (!lexer_.isCurrentTokenADirective())
    return nullptr;

  // Allocate a StringLiteralNode for the directive.
  auto *strLit = setLocation(
      tok_,
      tok_,
      new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));
  auto endLoc = tok_->getEndLoc();

  // Actually process the directive. Note that we want to do that before we
  // have consumed any more tokens - strictness can affect the interpretation
  // of tokens.
  processDirective(strLit->_value);

  advance(JSLexer::AllowDiv);

  // Consume the optional semicolon.
  if (check(TokenKind::semi))
    endLoc = advance().End;

  // Allocate an ExpressionStatementNode for the directive.
  return setLocation(
      strLit,
      endLoc,
      new (context_) ESTree::ExpressionStatementNode(strLit, strLit->_value));
}

namespace {
/// Upcast an Optional node type to a generic NodePtr, e.g.
/// \p Optional<FunctionExpressionNode> to \p Optional<NodePtr>.
template <typename T>
Optional<ESTree::NodePtr> castNode(Optional<T> node) {
  if (!node.hasValue())
    return None;
  return Optional<ESTree::NodePtr>(node.getValue());
}
} // namespace

bool JSParserImpl::preParseBuffer(
    Context &context,
    uint32_t bufferId,
    bool &useStaticBuiltinDetected) {
  PerfSection preparsing("Pre-Parsing JavaScript");
  AllocationScope scope(context.getAllocator());
  JSParserImpl parser(context, bufferId, PreParse);
  auto result = parser.parse();
  useStaticBuiltinDetected = parser.getUseStaticBuiltin();
  return result.hasValue();
}

Optional<ESTree::NodePtr> JSParserImpl::parseLazyFunction(
    ESTree::NodeKind kind,
    SMLoc start) {
  seek(start);

  switch (kind) {
    case ESTree::NodeKind::FunctionExpression:
      return castNode(parseFunctionExpression(true));

    case ESTree::NodeKind::FunctionDeclaration:
      return castNode(parseFunctionDeclaration(ParamReturn, true));

    case ESTree::NodeKind::Property: {
      auto node = parsePropertyAssignment(true);
      assert(node && "Reparsing of property assignment failed");
      if (auto *prop = dyn_cast<ESTree::PropertyNode>(*node)) {
        return prop->_value;
      } else {
        // This isn't technically (llvm_)unreachable, since it'll happen if you
        // fudge the source buffer during execution. Assert and fail instead.
        assert(false && "Expected a getter/setter function");
        return None;
      }
    }

    default:
      llvm_unreachable("Asked to parse unexpected node type");
  }
}
}; // namespace detail
}; // namespace parser
}; // namespace hermes
