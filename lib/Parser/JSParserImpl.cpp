/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "JSParserImpl.h"

#include "hermes/Support/PerfSection.h"

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

  // Generate the string representation of all tokens.
  for (unsigned i = 0; i != NUM_JS_TOKENS; ++i)
    tokenIdent_[i] = lexer_.getIdentifier(tokenKindStr((TokenKind)i));
}

Optional<ESTree::FileNode *> JSParserImpl::parse() {
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
    sm_.error(
        errorLoc,
        SourceErrorManager::combineIntoRange(whatLoc, errorLoc),
        ss.str());
  } else {
    sm_.error(errorLoc, ss.str());

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
      TokenKind::ampequal,
      TokenKind::caretequal,
      TokenKind::pipeequal);
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
    sm_.error(tok_->getStartLoc(), "';' expected");
  return false;
}

void JSParserImpl::processDirective(UniqueString *directive) {
  if (directive == useStrictIdent_)
    setStrictMode(true);
}

bool JSParserImpl::recursionDepthExceeded() {
  if (recursionDepth_ < MAX_RECURSION_DEPTH)
    return false;
  sm_.error(tok_->getStartLoc(), "Too many nested expressions/statements");
  return true;
}

Optional<ESTree::FileNode *> JSParserImpl::parseProgram() {
  SMLoc startLoc = tok_->getStartLoc();
  SaveStrictMode saveStrict{this};
  ESTree::NodeList stmtList;

  if (!parseStatementList(Param{}, TokenKind::eof, true, stmtList))
    return None;

  auto *program = setLocation(
      startLoc, tok_, new (context_) ESTree::ProgramNode(std::move(stmtList)));
  program->strictness = ESTree::makeStrictness(isStrictMode());
  return setLocation(
      program, program, new (context_) ESTree::FileNode(program));
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

  // identifier
  auto optId = parseBindingIdentifier(param.get(ParamYield));
  if (isDeclaration && !optId) {
    errorExpected(
        TokenKind::identifier,
        "after 'function'",
        "location of 'function'",
        startLoc);
    return None;
  }

  // (
  SMLoc lparenLoc = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_paren,
          JSLexer::AllowRegExp,
          "at start of function parameter list",
          isDeclaration ? "function declaration starts here"
                        : "function expression starts here",
          startLoc)) {
    return None;
  }

  ESTree::NodeList paramList;

  if (!check(TokenKind::r_paren)) {
    for (;;) {
      auto optElem = parseBindingElement(param);
      if (!optElem)
        return None;

      paramList.push_back(**optElem);

      if (!checkAndEat(TokenKind::comma))
        break;

      // Check for ",)".
      if (check(TokenKind::r_paren))
        break;
    }
  }

  // )
  if (!eat(
          TokenKind::r_paren,
          JSLexer::AllowRegExp,
          "at end of function parameter list",
          "start of parameter list",
          lparenLoc)) {
    return None;
  }

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
          *optId, std::move(paramList), nullptr, nullptr);
      // Initialize the node with a blank body.
      decl->_body = new (context_) ESTree::BlockStatementNode({});
      node = decl;
    } else {
      auto *expr = new (context_) ESTree::FunctionExpressionNode(
          optId ? *optId : nullptr, std::move(paramList), nullptr);
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
        *optId, std::move(paramList), body, nullptr);
    decl->strictness = ESTree::makeStrictness(isStrictMode());
    node = decl;
  } else {
    auto *expr = new (context_) ESTree::FunctionExpressionNode(
        optId ? *optId : nullptr, std::move(paramList), body);
    expr->strictness = ESTree::makeStrictness(isStrictMode());
    node = expr;
  }
  return setLocation(startLoc, body, node);
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
      _RET(parseVariableStatement(param.get(ParamYield)));
    case TokenKind::semi:
      _RET(parseEmptyStatement());
    case TokenKind::rw_if:
      _RET(parseIfStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_while:
      _RET(parseWhileStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_do:
      _RET(parseDoWhileStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_for:
      _RET(parseForStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_continue:
      _RET(parseContinueStatement());
    case TokenKind::rw_break:
      _RET(parseBreakStatement());
    case TokenKind::rw_return:
      _RET(parseReturnStatement());
    case TokenKind::rw_with:
      _RET(parseWithStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_switch:
      _RET(parseSwitchStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_throw:
      _RET(parseThrowStatement(param.get(ParamYield)));
    case TokenKind::rw_try:
      _RET(parseTryStatement(param.get(ParamYield, ParamReturn)));
    case TokenKind::rw_debugger:
      _RET(parseDebuggerStatement());

    default:
      _RET(parseExpressionOrLabelledStatement(
          param.get(ParamYield, ParamReturn)));
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

  auto body = parseBlock(
      param.get(ParamYield) + ParamReturn, grammarContext, parseDirectives);
  if (!body)
    return None;

  if (pass_ == PreParse) {
    preParsed_->bodyStartToEnd[(*body)->getStartLoc()] = (*body)->getEndLoc();
  }

  return body;
}

Optional<bool> JSParserImpl::parseStatementList(
    Param param,
    TokenKind until,
    bool parseDirectives,
    ESTree::NodeList &stmtList) {
  if (parseDirectives)
    while (auto *dirStmt = parseDirective())
      stmtList.push_back(*dirStmt);

  while (!check(until, TokenKind::eof)) {
    if (tok_->getKind() == TokenKind::rw_function) {
      auto fdecl = parseFunctionDeclaration(param.get(ParamYield));
      if (!fdecl)
        return None;

      stmtList.push_back(*fdecl.getValue());
    } else if (check(TokenKind::rw_const) || check(letIdent_)) {
      auto optLexDecl =
          parseLexicalDeclaration(ParamIn + param.get(ParamYield));
      if (!optLexDecl)
        return None;

      stmtList.push_back(*optLexDecl.getValue());
    } else {
      auto stmt = parseStatement(param.get(ParamYield, ParamReturn));
      if (!stmt)
        return None;

      stmtList.push_back(*stmt.getValue());
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
          param, TokenKind::r_brace, parseDirectives, stmtList)) {
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

Optional<ESTree::IdentifierNode *> JSParserImpl::parseBindingIdentifier(
    Param param) {
  UniqueString *id;
  if (check(TokenKind::identifier)) {
    id = tok_->getIdentifier();
  } else if (check(TokenKind::rw_yield)) {
    id = tok_->getResWordIdentifier();
    if (isStrictMode() || param.has(ParamYield))
      sm_.error(
          tok_->getSourceRange(),
          "Unexpected usage of 'yield' as an identifier");
  } else {
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
  auto kindIdent = tok_->getResWordOrIdentifier();

  SMLoc startLoc = advance().Start;

  ESTree::NodeList declList;
  if (!parseVariableDeclarationList(param, declList, startLoc))
    return None;

  auto endLoc = declList.back().getEndLoc();
  if (!eatSemi(endLoc))
    return None;

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
  return parseLexicalDeclaration(ParamIn + param.get(ParamYield));
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

    sm_.error(
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
    auto optIdent = parseBindingIdentifier(param.get(ParamYield));
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
        elemList.push_back(
            *setLocation(tok_, tok_, new (context_) ESTree::EmptyNode()));
      } else {
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
      sm_.error(
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
  auto optIdent = parseBindingIdentifier(param);
  if (!optIdent) {
    sm_.error(
        tok_->getStartLoc(), "identifier expected in object binding pattern");
    return None;
  }

  auto *key = *optIdent;
  ESTree::Node *value;

  if (check(TokenKind::equal)) {
    // BindingIdentifier Initializer
    //                   ^

    // Clone the key.
    auto *left = setLocation(
        key, key, new (context_) ESTree::IdentifierNode(key->_name, nullptr));
    auto optInit = parseBindingInitializer(param, left);
    if (!optInit)
      return None;

    value = *optInit;
  } else if (checkAndEat(TokenKind::colon)) {
    // BindingIdentifier ":" BindingElement
    //                       ^

    auto optBinding = parseBindingElement(param);
    if (!optBinding)
      return None;

    value = *optBinding;
  } else {
    // BindingIdentifier
    //                   ^

    // Clone the key.
    value = setLocation(
        key, key, new (context_) ESTree::IdentifierNode(key->_name, nullptr));
  }

  return setLocation(
      key, value, new (context_) ESTree::PropertyNode(key, value, initIdent_));
}

Optional<ESTree::EmptyStatementNode *> JSParserImpl::parseEmptyStatement() {
  assert(check(TokenKind::semi));
  auto *empty =
      setLocation(tok_, tok_, new (context_) ESTree::EmptyStatementNode());
  advance();

  std::string s;
  s.data();
  return empty;
}

Optional<ESTree::Node *> JSParserImpl::parseExpressionOrLabelledStatement(
    Param param) {
  bool startsWithIdentifier = check(TokenKind::identifier);
  auto optExpr = parseExpression();
  if (!optExpr)
    return None;

  // Check whether this is a label. The expression must have started with an
  // identifier, be just an identifier and be
  // followed by ':'
  if (startsWithIdentifier && isa<ESTree::IdentifierNode>(optExpr.getValue()) &&
      checkAndEat(TokenKind::colon)) {
    auto *id = cast<ESTree::IdentifierNode>(optExpr.getValue());

    auto optBody = parseStatement(param.get(ParamYield, ParamReturn));
    if (!optBody)
      return None;

    return setLocation(
        id,
        optBody.getValue(),
        new (context_) ESTree::LabeledStatementNode(id, optBody.getValue()));
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

  auto optConsequent = parseStatement(param.get(ParamYield, ParamReturn));
  if (!optConsequent)
    return None;

  if (checkAndEat(TokenKind::rw_else)) {
    auto optAlternate = parseStatement(param.get(ParamYield, ParamReturn));
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

  auto optBody = parseStatement(param.get(ParamYield, ParamReturn));
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

  auto optBody = parseStatement(param.get(ParamYield, ParamReturn));
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

  if (!eatSemi(endLoc))
    return None;

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
      sm_.error(
          decl->getSourceRange(),
          "Only one binding must be declared in a for-in/for-of loop");
      return None;
    }

    // Check for destructuring pattern on the left and reparse it.
    if (expr1 &&
        (isa<ESTree::ArrayExpressionNode>(expr1) ||
         isa<ESTree::ObjectExpressionNode>(expr1))) {
      auto optExpr1 = reparseAssignmentPattern(expr1);
      if (!optExpr1)
        return None;
      expr1 = *optExpr1;
    }

    // Remember whether we are parsing for-in or for-of.
    bool const forInLoop = check(TokenKind::rw_in);
    advance();

    auto optRightExpr = forInLoop
        ? parseExpression()
        : parseAssignmentExpression(ParamIn + param.get(ParamYield));

    if (!eat(
            TokenKind::r_paren,
            JSLexer::AllowRegExp,
            "after 'for(... in/of ...'",
            "location of '('",
            lparenLoc))
      return None;

    auto optBody = parseStatement(param.get(ParamYield, ParamReturn));
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

    auto optBody = parseStatement(param.get(ParamYield, ParamReturn));
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

  auto optBody = parseStatement(param.get(ParamYield, ParamReturn));
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
        sm_.error(clauseStartLoc, "more than one 'default' clause in 'switch'");
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

    while (!checkN(
        TokenKind::rw_default, TokenKind::rw_case, TokenKind::r_brace)) {
      auto optStmt = parseStatement(param.get(ParamYield, ParamReturn));
      if (!optStmt)
        return None;
      stmtList.push_back(*optStmt.getValue());
    }

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
    sm_.error(tok_->getStartLoc(), "'throw' argument must be on the same line");
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
  auto optTryBody = parseBlock(param.get(ParamYield, ParamReturn));
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

    if (!need(
            TokenKind::identifier,
            "inside catch list",
            "location of 'catch'",
            handlerStartLoc))
      return None;
    auto *identifier = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::IdentifierNode(tok_->getIdentifier(), nullptr));
    advance();

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
    auto optCatchBody = parseBlock(param.get(ParamYield, ParamReturn));
    if (!optCatchBody)
      return None;

    catchHandler = setLocation(
        handlerStartLoc,
        optCatchBody.getValue(),
        new (context_)
            ESTree::CatchClauseNode(identifier, optCatchBody.getValue()));
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
    auto optFinallyBody = parseBlock(param.get(ParamYield, ParamReturn));
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

      auto expr = parseExpression();
      if (!expr)
        return None;
      if (!eat(
              TokenKind::r_paren,
              JSLexer::AllowDiv,
              "at end of parenthesized expression",
              "started here",
              startLoc))
        return None;
      // Record the number of parens surrounding an expression.
      expr.getValue()->incParens();
      return expr.getValue();
    }

    case TokenKind::rw_function: {
      auto fExpr = parseFunctionExpression();
      if (!fExpr)
        return None;
      return fExpr.getValue();
    }

    default:
      sm_.error(tok_->getStartLoc(), "invalid expression");
      return None;
  }
}

Optional<ESTree::ArrayExpressionNode *> JSParserImpl::parseArrayLiteral() {
  assert(check(TokenKind::l_square));
  SMLoc startLoc = advance().Start;

  ESTree::NodeList elemList;

  if (!check(TokenKind::r_square)) {
    for (;;) {
      if (check(TokenKind::comma)) {
        elemList.push_back(
            *setLocation(tok_, tok_, new (context_) ESTree::EmptyNode()));
      } else {
        auto expr = parseAssignmentExpression();
        if (!expr)
          return None;
        elemList.push_back(*expr.getValue());
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
          "at end of array literal '[...'",
          "location of '['",
          startLoc))
    return None;

  return setLocation(
      startLoc,
      endLoc,
      new (context_) ESTree::ArrayExpressionNode(std::move(elemList)));
}

Optional<ESTree::ObjectExpressionNode *> JSParserImpl::parseObjectLiteral() {
  assert(check(TokenKind::l_brace));
  SMLoc startLoc = advance().Start;

  ESTree::NodeList elemList;

  if (!check(TokenKind::r_brace)) {
    for (;;) {
      auto prop = parsePropertyAssignment();
      if (!prop)
        return None;

      elemList.push_back(*prop.getValue());

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

Optional<ESTree::Node *> JSParserImpl::parsePropertyAssignment() {
  SMLoc startLoc = tok_->getStartLoc();
  ESTree::NodePtr key = nullptr;

  SaveStrictMode saveStrictMode{this};

  if (check(getIdent_)) {
    UniqueString *ident = tok_->getResWordOrIdentifier();
    SMRange identRng = tok_->getSourceRange();
    advance();

    // This could either be a getter, or a property named 'get'.
    if (check(TokenKind::colon)) {
      // This is just a property.
      key = setLocation(
          identRng,
          identRng,
          new (context_) ESTree::IdentifierNode(ident, nullptr));
    } else {
      // A getter method.
      auto optKey = parsePropertyKey();
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
      auto block = parseBlock(ParamReturn, JSLexer::AllowRegExp, true);
      if (!block)
        return None;

      auto *funcExpr = new (context_) ESTree::FunctionExpressionNode(
          nullptr, ESTree::NodeList{}, block.getValue());
      funcExpr->strictness = ESTree::makeStrictness(isStrictMode());
      setLocation(startLoc, block.getValue(), funcExpr);

      auto *node = new (context_)
          ESTree::PropertyNode(optKey.getValue(), funcExpr, getIdent_);
      return setLocation(startLoc, block.getValue(), node);
    }
  } else if (check(setIdent_)) {
    UniqueString *ident = tok_->getResWordOrIdentifier();
    SMRange identRng = tok_->getSourceRange();
    advance();

    // This could either be a setter, or a property named 'set'.
    if (check(TokenKind::colon)) {
      // This is just a property.
      key = setLocation(
          identRng,
          identRng,
          new (context_) ESTree::IdentifierNode(ident, nullptr));
    } else {
      // A setter method.
      auto optKey = parsePropertyKey();
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
      auto block = parseBlock(ParamReturn, JSLexer::AllowRegExp, true);
      if (!block)
        return None;

      auto *funcExpr = new (context_) ESTree::FunctionExpressionNode(
          nullptr, std::move(params), block.getValue());
      funcExpr->strictness = ESTree::makeStrictness(isStrictMode());
      setLocation(startLoc, block.getValue(), funcExpr);

      auto *node = new (context_)
          ESTree::PropertyNode(optKey.getValue(), funcExpr, setIdent_);
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
          new (context_) ESTree::PropertyNode(key, value, initIdent_));
    }
  } else {
    auto optKey = parsePropertyKey();
    if (!optKey)
      return None;

    key = optKey.getValue();
  }

  ESTree::Node *value;

  // Check for CoverInitializedName: IdentifierReference Initializer
  if (isa<ESTree::IdentifierNode>(key) && check(TokenKind::equal)) {
    auto startLoc = advance().Start;
    auto optInit = parseAssignmentExpression();
    if (!optInit)
      return None;

    value = setLocation(
        startLoc,
        *optInit,
        new (context_) ESTree::CoverInitializerNode(*optInit));
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
      new (context_) ESTree::PropertyNode(key, value, initIdent_));
}

Optional<ESTree::Node *> JSParserImpl::parsePropertyKey() {
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
        sm_.error(
            tok_->getSourceRange(),
            "invalid property name - must be a string, number or identifier");
        return None;
      }
  }
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

  auto primExpr = parsePrimaryExpression();
  if (!primExpr)
    return None;
  expr = primExpr.getValue();

  SMLoc objectLoc = startLoc;
  while (check(TokenKind::l_square, TokenKind::period)) {
    SMLoc nextObjectLoc = tok_->getStartLoc();
    auto msel = parseMemberSelect(objectLoc, expr);
    if (!msel)
      return None;
    objectLoc = nextObjectLoc;
    expr = msel.getValue();
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
      auto arg = parseAssignmentExpression();
      if (!arg)
        return None;
      argList.push_back(*arg.getValue());

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
  assert(check(TokenKind::l_paren));

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
    } else {
      break;
    }
  }

  return expr;
}

Optional<ESTree::Node *> JSParserImpl::parseNewExpressionOrMemberExpression() {
  if (!check(TokenKind::rw_new))
    return parseMemberExpressionExceptNew();

  SMLoc startLoc = advance().Start;

  auto optExpr = parseNewExpressionOrMemberExpression();
  if (!optExpr)
    return None;
  ESTree::NodePtr expr = optExpr.getValue();

  // Do we have arguments to a child MemberExpression? If yes, then it really
  // was a 'new MemberExpression(args)', otherwise it is a NewExpression
  if (!check(TokenKind::l_paren)) {
    return setLocation(
        startLoc,
        expr,
        new (context_) ESTree::NewExpressionNode(expr, ESTree::NodeList{}));
  }

  auto debugLoc = tok_->getStartLoc();
  ESTree::NodeList argList;
  SMLoc endLoc;
  if (!parseArguments(argList, endLoc))
    return None;

  expr = setLocation(
      startLoc,
      endLoc,
      debugLoc,
      new (context_) ESTree::NewExpressionNode(expr, std::move(argList)));

  SMLoc objectLoc = startLoc;
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
  if (check(TokenKind::l_paren)) {
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
    // If the next operator has lower precedence than the operator on the stack,
    // pop the stack, creating a new binary expression.
    while (sp != STACK_SIZE && precedence <= getPrecedence(opStack[sp])) {
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

bool JSParserImpl::matchArrowParameters(
    ESTree::Node *node,
    ESTree::NodeList &paramList) {
  // Empty argument list "()".
  if (node->getParens() == 0 && isa<ESTree::CoverEmptyArgsNode>(node))
    return true;

  // A single identifier.
  if (auto *identNode = dyn_cast<ESTree::IdentifierNode>(node)) {
    // "a" and "(a)" are both OK, but "((a))" isn't.
    if (identNode->getParens() > 1)
      return false;
    paramList.push_back(*identNode);
    return true;
  }

  // A sequence of identifiers.
  if (auto *seqNode = dyn_cast<ESTree::SequenceExpressionNode>(node)) {
    // The sequence must be surrounded by a single pair of parens.
    if (seqNode->getParens() != 1)
      return false;

    for (auto &expr : seqNode->_expressions) {
      // No parens around parameters.
      if (expr.getParens() != 0)
        return false;

      if (isa<ESTree::IdentifierNode>(expr))
        continue;
      if (isa<ESTree::CoverTrailingCommaNode>(expr))
        continue;

      return false;
    }

    paramList = std::move(seqNode->_expressions);

    // Erase the trailing comma node.
    if (!paramList.empty() &&
        isa<ESTree::CoverTrailingCommaNode>(paramList.back())) {
      paramList.pop_back();
    }

    return true;
  }

  return false;
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
  if (!matchArrowParameters(leftExpr, paramList)) {
    sm_.error(
        leftExpr->getSourceRange(), "Invalid argument list for arrow function");
    return None;
  }

  SaveStrictMode saveStrictMode{this};
  ESTree::Node *body;
  bool expression;

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
    ESTree::Node *node) {
  if (auto *AEN = dyn_cast<ESTree::ArrayExpressionNode>(node)) {
    return reparseArrayAsignmentPattern(AEN);
  } else if (auto *OEN = dyn_cast<ESTree::ObjectExpressionNode>(node)) {
    return reparseObjectAssignmentPattern(OEN);
  } else {
    return node;
  }
}

Optional<ESTree::Node *> JSParserImpl::reparseArrayAsignmentPattern(
    ESTree::ArrayExpressionNode *AEN) {
  ESTree::NodeList elements{};

  for (auto it = AEN->_elements.begin(), e = AEN->_elements.end(); it != e;) {
    ESTree::Node *elem = &*it++;
    AEN->_elements.remove(*elem);

    ESTree::Node *init = nullptr;

    // If we encounter an initializer, unpack it.
    if (auto *asn = dyn_cast<ESTree::AssignmentExpressionNode>(elem)) {
      if (asn->_operator == getTokenIdent(TokenKind::equal)) {
        elem = asn->_left;
        init = asn->_right;
      }
    }

    // Reparse {...} or [...]
    auto optSubPattern = reparseAssignmentPattern(elem);
    if (!optSubPattern)
      continue;
    elem = *optSubPattern;

    if (init) {
      elem = setLocation(
          elem, init, new (context_) ESTree::AssignmentPatternNode(elem, init));
    }

    elements.push_back(*elem);
  }

  return setLocation(
      AEN->getStartLoc(),
      AEN->getEndLoc(),
      new (context_) ESTree::ArrayPatternNode(std::move(elements)));
}

Optional<ESTree::Node *> JSParserImpl::reparseObjectAssignmentPattern(
    ESTree::ObjectExpressionNode *OEN) {
  ESTree::NodeList elements{};

  for (auto it = OEN->_properties.begin(), e = OEN->_properties.end();
       it != e;) {
    auto *propNode = cast<ESTree::PropertyNode>(&*it++);
    OEN->_properties.remove(*propNode);

    if (propNode->_kind != initIdent_) {
      sm_.error(
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
    auto optSubPattern = reparseAssignmentPattern(value);
    if (!optSubPattern)
      continue;
    value = *optSubPattern;

    // If we have an initializer, create an AssignmentPattern.
    if (init) {
      value = new (context_) ESTree::AssignmentPatternNode(value, init);
      value->copyLocationFrom(propNode->_value);
    }

    propNode->_value = value;
    elements.push_back(*propNode);
  }

  auto *OP = new (context_) ESTree::ObjectPatternNode(std::move(elements));
  OP->copyLocationFrom(OEN);
  return OP;
}

Optional<ESTree::Node *> JSParserImpl::parseAssignmentExpression(Param param) {
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
    optLeftExpr = reparseAssignmentPattern(*optLeftExpr);
    if (!optLeftExpr)
      return None;
  }

  UniqueString *op = getTokenIdent(tok_->getKind());
  auto debugLoc = advance().Start;

  auto optRightExpr = parseAssignmentExpression(param);
  if (!optRightExpr)
    return None;

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

    auto optExpr2 = parseAssignmentExpression(param);
    if (!optExpr2)
      return None;

    exprList.push_back(*optExpr2.getValue());
  }

  auto *firstExpr = &exprList.front();
  auto *lastExpr = &exprList.back();
  return setLocation(
      firstExpr,
      lastExpr,
      new (context_) ESTree::SequenceExpressionNode(std::move(exprList)));
}

ESTree::ExpressionStatementNode *JSParserImpl::parseDirective() {
  // Is the current token a directive?
  auto tok = lexer_.rescanCurrentTokenAsDirective();
  if (!tok)
    return nullptr;

  // Save the updated current token. (This is technically a no-op, since the
  // same token is being reused, but that could change in the future.
  tok_ = tok;

  // Allocate a SingLiteralNode for the directive.
  auto *strLit = setLocation(
      tok_,
      tok_,
      new (context_) ESTree::StringLiteralNode(tok_->getDirective()));
  strLit->potentialDirective = true;
  strLit->directive = true;
  auto endLoc = tok_->getEndLoc();

  // Actually process the directive. Note that we want to do that before we
  // have consumed any more tokens - strictness can affect the interpretation
  // of tokens.
  processDirective(strLit->_value);

  advance(JSLexer::AllowDiv);

  // Consume the optional semicolon.
  if (check(TokenKind::semi)) {
    endLoc = tok_->getEndLoc();
    advance();
  }

  // Allocate an ExpressionStatementNode for the directive.
  return setLocation(
      strLit,
      endLoc,
      new (context_) ESTree::ExpressionStatementNode(strLit, nullptr));
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

bool JSParserImpl::preParseBuffer(Context &context, uint32_t bufferId) {
  PerfSection preparsing("Pre-Parsing JavaScript");
  AllocationScope scope(context.getAllocator());
  JSParserImpl parser(context, bufferId, PreParse);
  auto result = parser.parse();
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

    default:
      llvm_unreachable("Asked to parse unexpected node type");
  }
}

}; // namespace detail
}; // namespace parser
}; // namespace hermes
