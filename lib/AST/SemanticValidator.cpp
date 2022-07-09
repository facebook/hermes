/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SemanticValidator.h"

#include "hermes/Support/RegExpSerialization.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/ADT/SmallSet.h"
#include "llvh/Support/SaveAndRestore.h"

using llvh::cast_or_null;
using llvh::dyn_cast;
using llvh::isa;
using llvh::SaveAndRestore;

namespace hermes {
namespace sem {

//===----------------------------------------------------------------------===//
// Keywords

Keywords::Keywords(Context &astContext)
    : identArguments(
          astContext.getIdentifier("arguments").getUnderlyingPointer()),
      identEval(astContext.getIdentifier("eval").getUnderlyingPointer()),
      identDelete(astContext.getIdentifier("delete").getUnderlyingPointer()),
      identThis(astContext.getIdentifier("this").getUnderlyingPointer()),
      identUseStrict(
          astContext.getIdentifier("use strict").getUnderlyingPointer()),
      identShowSource(
          astContext.getIdentifier("show source").getUnderlyingPointer()),
      identHideSource(
          astContext.getIdentifier("hide source").getUnderlyingPointer()),
      identSensitive(
          astContext.getIdentifier("sensitive").getUnderlyingPointer()),
      identVar(astContext.getIdentifier("var").getUnderlyingPointer()),
      identLet(astContext.getIdentifier("let").getUnderlyingPointer()),
      identConst(astContext.getIdentifier("const").getUnderlyingPointer()),
      identPlus(astContext.getIdentifier("+").getUnderlyingPointer()),
      identMinus(astContext.getIdentifier("-").getUnderlyingPointer()),
      identAssign(astContext.getIdentifier("=").getUnderlyingPointer()) {}

//===----------------------------------------------------------------------===//
// SemanticValidator

SemanticValidator::SemanticValidator(
    Context &astContext,
    sem::SemContext &semCtx,
    bool compile)
    : astContext_(astContext),
      sm_(astContext.getSourceErrorManager()),
      bufferMessages_{&sm_},
      semCtx_(semCtx),
      initialErrorCount_(sm_.getErrorCount()),
      kw_(astContext),
      compile_(compile) {}

bool SemanticValidator::doIt(Node *rootNode) {
  visitESTreeNode(*this, rootNode);
  return sm_.getErrorCount() == initialErrorCount_;
}

bool SemanticValidator::doFunction(Node *function, bool strict) {
  // Create a wrapper context since a function always assumes there is an
  // existing context.
  FunctionContext wrapperContext(this, strict, nullptr);

  visitESTreeNode(*this, function);
  return sm_.getErrorCount() == initialErrorCount_;
}

void SemanticValidator::visit(ProgramNode *node) {
  FunctionContext newFuncCtx{this, astContext_.isStrictMode(), node};

  scanDirectivePrologue(node->_body);
  setDirectiveDerivedInfo(node);

  visitESTreeChildren(*this, node);
}

void SemanticValidator::visit(VariableDeclaratorNode *varDecl, Node *parent) {
  auto *declaration = cast<VariableDeclarationNode>(parent);

  FunctionInfo::VarDecl::Kind declKind;
  if (declaration->_kind == kw_.identLet)
    declKind = FunctionInfo::VarDecl::Kind::Let;
  else if (declaration->_kind == kw_.identConst)
    declKind = FunctionInfo::VarDecl::Kind::Const;
  else {
    assert(declaration->_kind == kw_.identVar);
    declKind = FunctionInfo::VarDecl::Kind::Var;
  }

  validateDeclarationNames(
      declKind, varDecl->_id, &curFunction()->semInfo->varDecls);
  visitESTreeChildren(*this, varDecl);
}

void SemanticValidator::visit(MetaPropertyNode *metaProp) {
  auto *meta = cast<IdentifierNode>(metaProp->_meta);
  auto *property = cast<IdentifierNode>(metaProp->_property);

  if (meta->_name->str() == "new" && property->_name->str() == "target") {
    if (curFunction()->isGlobalScope()) {
      // ES9.0 15.1.1:
      // It is a Syntax Error if StatementList Contains NewTarget unless the
      // source code containing NewTarget is eval code that is being processed
      // by a direct eval.
      // Hermes does not support local eval, so we assume that this is not
      // inside a local eval call.
      sm_.error(metaProp->getSourceRange(), "'new.target' not in a function");
    }
    return;
  }

  if (meta->_name->str() == "import" && property->_name->str() == "meta") {
    if (compile_) {
      sm_.error(
          metaProp->getSourceRange(), "'import.meta' is currently unsupported");
    }
    return;
  }

  sm_.error(
      metaProp->getSourceRange(),
      "invalid meta property " + meta->_name->str() + "." +
          property->_name->str());
}

void SemanticValidator::visit(IdentifierNode *identifier) {
  if (identifier->_name == kw_.identEval && !astContext_.getEnableEval())
    sm_.error(identifier->getSourceRange(), "'eval' is disabled");

  if (identifier->_name == kw_.identArguments)
    curFunction()->semInfo->usesArguments = true;
}

/// Process a function declaration by creating a new FunctionContext.
void SemanticValidator::visit(FunctionDeclarationNode *funcDecl) {
  curFunction()->semInfo->closures.push_back(funcDecl);
  visitFunction(funcDecl, funcDecl->_id, funcDecl->_params, funcDecl->_body);
}

/// Process a function expression by creating a new FunctionContext.
void SemanticValidator::visit(FunctionExpressionNode *funcExpr) {
  visitFunction(funcExpr, funcExpr->_id, funcExpr->_params, funcExpr->_body);
}

void SemanticValidator::visit(ArrowFunctionExpressionNode *arrowFunc) {
  // Convert expression functions to a full-body to simplify IRGen.
  if (compile_ && arrowFunc->_expression) {
    auto *retStmt = new (astContext_) ReturnStatementNode(arrowFunc->_body);
    retStmt->copyLocationFrom(arrowFunc->_body);

    ESTree::NodeList stmtList;
    stmtList.push_back(*retStmt);

    auto *blockStmt = new (astContext_) BlockStatementNode(std::move(stmtList));
    blockStmt->copyLocationFrom(arrowFunc->_body);

    arrowFunc->_body = blockStmt;
    arrowFunc->_expression = false;
  }

  visitFunction(arrowFunc, nullptr, arrowFunc->_params, arrowFunc->_body);

  curFunction()->semInfo->containsArrowFunctions = true;
  curFunction()->semInfo->containsArrowFunctionsUsingArguments =
      curFunction()->semInfo->containsArrowFunctionsUsingArguments ||
      arrowFunc->getSemInfo()->containsArrowFunctionsUsingArguments ||
      arrowFunc->getSemInfo()->usesArguments;
}

/// Ensure that the left side of for-in is an l-value.
void SemanticValidator::visit(ForInStatementNode *forIn) {
  visitForInOf(forIn, forIn->_left);
}
void SemanticValidator::visit(ForOfStatementNode *forOf) {
  if (compile_ && forOf->_await)
    sm_.error(
        forOf->getSourceRange(),
        "for await..of loops are currently unsupported");

  visitForInOf(forOf, forOf->_left);
}

void SemanticValidator::visitForInOf(LoopStatementNode *loopNode, Node *left) {
  loopNode->setLabelIndex(curFunction()->allocateLabel());

  SaveAndRestore<LoopStatementNode *> saveLoop(
      curFunction()->activeLoop, loopNode);
  SaveAndRestore<StatementNode *> saveSwitch(
      curFunction()->activeSwitchOrLoop, loopNode);

  if (auto *VD = dyn_cast<VariableDeclarationNode>(left)) {
    assert(
        VD->_declarations.size() == 1 &&
        "for-in/for-of must have a single binding");

    auto *declarator =
        cast<ESTree::VariableDeclaratorNode>(&VD->_declarations.front());

    if (declarator->_init) {
      if (isa<ESTree::PatternNode>(declarator->_id)) {
        sm_.error(
            declarator->_init->getSourceRange(),
            "destructuring declaration cannot be initialized in for-in/for-of loop");
      } else if (!(isa<ForInStatementNode>(loopNode) &&
                   !curFunction()->strictMode && VD->_kind == kw_.identVar)) {
        sm_.error(
            declarator->_init->getSourceRange(),
            "for-in/for-of variable declaration may not be initialized");
      }
    }
  } else {
    validateAssignmentTarget(left);
  }
  visitESTreeChildren(*this, loopNode);
}

void SemanticValidator::visit(BinaryExpressionNode *bin) {
  // Handle nested +/- non-recursively.
  if (bin->_operator == kw_.identPlus || bin->_operator == kw_.identMinus) {
    auto list = linearizeLeft(bin, {"+", "-"});
    if (list.size() > MAX_NESTED_BINARY) {
      recursionDepthExceeded(bin);
      return;
    }

    visitESTreeNode(*this, list[0]->_left, list[0]);
    for (auto *e : list) {
      visitESTreeNode(*this, e->_right, e);
    }
    return;
  }

  visitESTreeChildren(*this, bin);
}

/// Ensure that the left side of assgnments is an l-value.
void SemanticValidator::visit(AssignmentExpressionNode *assignment) {
  // Handle nested "=" non-recursively.
  if (assignment->_operator == kw_.identAssign) {
    auto list = linearizeRight(assignment, {"="});
    if (list.size() > MAX_NESTED_ASSIGNMENTS) {
      recursionDepthExceeded(assignment);
      return;
    }

    for (auto *e : list) {
      validateAssignmentTarget(e->_left);
      visitESTreeNode(*this, e->_left, e);
    }
    visitESTreeNode(*this, list.back()->_right, list.back());
    return;
  }

  validateAssignmentTarget(assignment->_left);
  visitESTreeChildren(*this, assignment);
}

/// Ensure that the operand of ++/-- is an l-value.
void SemanticValidator::visit(UpdateExpressionNode *update) {
  // Check if the left-hand side is valid.
  if (!isLValue(update->_argument)) {
    sm_.error(
        update->_argument->getSourceRange(),
        "invalid operand in update operation");
  }
  visitESTreeChildren(*this, update);
}

/// Declare named labels, checking for duplicates, etc.
void SemanticValidator::visit(LabeledStatementNode *labelStmt) {
  auto id = cast<IdentifierNode>(labelStmt->_label);

  labelStmt->setLabelIndex(curFunction()->allocateLabel());

  // Determine the target statement. We need to check if it directly encloses
  // a loop or another label enclosing a loop.
  StatementNode *targetStmt = labelStmt;
  {
    LabeledStatementNode *curStmt = labelStmt;
    do {
      if (auto *LS = dyn_cast<LoopStatementNode>(curStmt->_body)) {
        targetStmt = LS;
        break;
      }
    } while ((curStmt = dyn_cast<LabeledStatementNode>(curStmt->_body)));
  }

  // Define the new label, checking for a previous definition.
  auto insertRes =
      curFunction()->labelMap.insert({id->_name, {id, targetStmt}});
  if (!insertRes.second) {
    sm_.error(
        id->getSourceRange(),
        llvh::Twine("label '") + id->_name->str() + "' is already defined");
    sm_.note(
        insertRes.first->second.declarationNode->getSourceRange(),
        "previous definition");
  }
  // Auto-erase the label on exit, if we inserted it.
  const auto &deleter = llvh::make_scope_exit([=]() {
    if (insertRes.second)
      curFunction()->labelMap.erase(id->_name);
  });
  (void)deleter;

  visitESTreeChildren(*this, labelStmt);
}

/// Check RegExp syntax.
void SemanticValidator::visit(RegExpLiteralNode *regexp) {
  llvh::StringRef regexpError;
  if (compile_ &&
      !CompiledRegExp::tryCompile(
          regexp->_pattern->str(), regexp->_flags->str(), &regexpError)) {
    sm_.error(
        regexp->getSourceRange(),
        "Invalid regular expression: " + Twine(regexpError));
  }
  visitESTreeChildren(*this, regexp);
}

void SemanticValidator::visit(TryStatementNode *tryStatement) {
  // A try statement with both catch and finally handlers is technically
  // two nested try statements. Transform:
  //
  //    try {
  //      tryBody;
  //    } catch {
  //      catchBody;
  //    } finally {
  //      finallyBody;
  //    }
  //
  // into
  //
  //    try {
  //      try {
  //        tryBody;
  //      } catch {
  //        catchBody;
  //      }
  //    } finally {
  //      finallyBody;
  //    }
  if (compile_ && tryStatement->_handler && tryStatement->_finalizer) {
    auto *nestedTry = new (astContext_)
        TryStatementNode(tryStatement->_block, tryStatement->_handler, nullptr);
    nestedTry->copyLocationFrom(tryStatement);
    nestedTry->setEndLoc(nestedTry->_handler->getEndLoc());

    ESTree::NodeList stmtList;
    stmtList.push_back(*nestedTry);
    tryStatement->_block =
        new (astContext_) BlockStatementNode(std::move(stmtList));
    tryStatement->_block->copyLocationFrom(nestedTry);
    tryStatement->_handler = nullptr;
  }

  visitESTreeNode(*this, tryStatement->_block, tryStatement);
  visitESTreeNode(*this, tryStatement->_handler, tryStatement);
  visitESTreeNode(*this, tryStatement->_finalizer, tryStatement);
}

void SemanticValidator::visit(DoWhileStatementNode *loop) {
  loop->setLabelIndex(curFunction()->allocateLabel());

  SaveAndRestore<LoopStatementNode *> saveLoop(curFunction()->activeLoop, loop);
  SaveAndRestore<StatementNode *> saveSwitch(
      curFunction()->activeSwitchOrLoop, loop);

  visitESTreeChildren(*this, loop);
}
void SemanticValidator::visit(ForStatementNode *loop) {
  loop->setLabelIndex(curFunction()->allocateLabel());

  SaveAndRestore<LoopStatementNode *> saveLoop(curFunction()->activeLoop, loop);
  SaveAndRestore<StatementNode *> saveSwitch(
      curFunction()->activeSwitchOrLoop, loop);

  visitESTreeChildren(*this, loop);
}
void SemanticValidator::visit(WhileStatementNode *loop) {
  loop->setLabelIndex(curFunction()->allocateLabel());

  SaveAndRestore<LoopStatementNode *> saveLoop(curFunction()->activeLoop, loop);
  SaveAndRestore<StatementNode *> saveSwitch(
      curFunction()->activeSwitchOrLoop, loop);

  visitESTreeChildren(*this, loop);
}
void SemanticValidator::visit(SwitchStatementNode *switchStmt) {
  switchStmt->setLabelIndex(curFunction()->allocateLabel());

  SaveAndRestore<StatementNode *> saveSwitch(
      curFunction()->activeSwitchOrLoop, switchStmt);

  visitESTreeChildren(*this, switchStmt);
}

void SemanticValidator::visit(BreakStatementNode *breakStmt) {
  if (auto id = cast_or_null<IdentifierNode>(breakStmt->_label)) {
    // A labeled break.
    // Find the label in the label map.
    auto labelIt = curFunction()->labelMap.find(id->_name);
    if (labelIt != curFunction()->labelMap.end()) {
      auto labelIndex = getLabelDecorationBase(labelIt->second.targetStatement)
                            ->getLabelIndex();
      breakStmt->setLabelIndex(labelIndex);
    } else {
      sm_.error(
          id->getSourceRange(),
          Twine("label '") + id->_name->str() + "' is not defined");
    }
  } else {
    // Anonymous break.
    if (curFunction()->activeSwitchOrLoop) {
      auto labelIndex =
          getLabelDecorationBase(curFunction()->activeSwitchOrLoop)
              ->getLabelIndex();
      breakStmt->setLabelIndex(labelIndex);
    } else {
      sm_.error(
          breakStmt->getSourceRange(), "'break' not within a loop or a switch");
    }
  }

  visitESTreeChildren(*this, breakStmt);
}

void SemanticValidator::visit(ContinueStatementNode *continueStmt) {
  if (auto id = cast_or_null<IdentifierNode>(continueStmt->_label)) {
    // A labeled continue.
    // Find the label in the label map.
    auto labelIt = curFunction()->labelMap.find(id->_name);
    if (labelIt != curFunction()->labelMap.end()) {
      if (isa<LoopStatementNode>(labelIt->second.targetStatement)) {
        auto labelIndex =
            getLabelDecorationBase(labelIt->second.targetStatement)
                ->getLabelIndex();
        continueStmt->setLabelIndex(labelIndex);
      } else {
        sm_.error(
            id->getSourceRange(),
            llvh::Twine("continue label '") + id->_name->str() +
                "' is not a loop label");
        sm_.note(
            labelIt->second.declarationNode->getSourceRange(),
            "label defined here");
      }
    } else {
      sm_.error(
          id->getSourceRange(),
          Twine("label '") + id->_name->str() + "' is not defined");
    }
  } else {
    // Anonymous continue.
    if (curFunction()->activeLoop) {
      auto labelIndex = curFunction()->activeLoop->getLabelIndex();
      continueStmt->setLabelIndex(labelIndex);
    } else {
      sm_.error(continueStmt->getSourceRange(), "'continue' not within a loop");
    }
  }
  visitESTreeChildren(*this, continueStmt);
}

void SemanticValidator::visit(ReturnStatementNode *returnStmt) {
  if (curFunction()->isGlobalScope() &&
      !astContext_.allowReturnOutsideFunction())
    sm_.error(returnStmt->getSourceRange(), "'return' not in a function");
  visitESTreeChildren(*this, returnStmt);
}

void SemanticValidator::visit(YieldExpressionNode *yieldExpr) {
  if (curFunction()->isGlobalScope() ||
      (curFunction()->node && !ESTree::isGenerator(curFunction()->node)))
    sm_.error(
        yieldExpr->getSourceRange(), "'yield' not in a generator function");

  if (isFormalParams_) {
    // For generators functions (the only time YieldExpression is parsed):
    // It is a Syntax Error if UniqueFormalParameters Contains YieldExpression
    // is true.
    sm_.error(
        yieldExpr->getSourceRange(),
        "'yield' not allowed in a formal parameter");
  }

  visitESTreeChildren(*this, yieldExpr);
}

void SemanticValidator::visit(AwaitExpressionNode *awaitExpr) {
  if (curFunction()->isGlobalScope() ||
      (curFunction()->node && !ESTree::isAsync(curFunction()->node)))
    sm_.error(awaitExpr->getSourceRange(), "'await' not in an async function");

  visitESTreeChildren(*this, awaitExpr);
}

void SemanticValidator::visit(UnaryExpressionNode *unaryExpr) {
  // Check for unqualified delete in strict mode.
  if (unaryExpr->_operator == kw_.identDelete) {
    if (curFunction()->strictMode &&
        isa<IdentifierNode>(unaryExpr->_argument)) {
      sm_.error(
          unaryExpr->getSourceRange(),
          "'delete' of a variable is not allowed in strict mode");
    }
  }
  visitESTreeChildren(*this, unaryExpr);
}

void SemanticValidator::visit(ArrayPatternNode *AP) {
  visitESTreeChildren(*this, AP);
}

void SemanticValidator::visit(SpreadElementNode *S, Node *parent) {
  if (!isa<ESTree::ObjectExpressionNode>(parent) &&
      !isa<ESTree::ArrayExpressionNode>(parent) &&
      !isa<ESTree::CallExpressionNode>(parent) &&
      !isa<ESTree::OptionalCallExpressionNode>(parent) &&
      !isa<ESTree::NewExpressionNode>(parent))
    sm_.error(S->getSourceRange(), "spread operator is not supported");
  visitESTreeChildren(*this, S);
}

void SemanticValidator::visit(ClassExpressionNode *node) {
  SaveAndRestore<bool> oldStrictMode{curFunction()->strictMode, true};
  visitESTreeChildren(*this, node);
}

void SemanticValidator::visit(ClassDeclarationNode *node) {
  SaveAndRestore<bool> oldStrictMode{curFunction()->strictMode, true};
  visitESTreeChildren(*this, node);
}

void SemanticValidator::visit(PrivateNameNode *node) {
  if (compile_)
    sm_.error(node->getSourceRange(), "private properties are not supported");
  visitESTreeChildren(*this, node);
}

void SemanticValidator::visit(ClassPrivatePropertyNode *node) {
  if (compile_)
    sm_.error(node->getSourceRange(), "private properties are not supported");
  visitESTreeChildren(*this, node);
}

void SemanticValidator::visit(ImportDeclarationNode *importDecl) {
  // Like variable declarations, imported names must be hoisted.
  if (!astContext_.getUseCJSModules()) {
    sm_.error(
        importDecl->getSourceRange(),
        "'import' statement requires module mode");
  }

  if (compile_ && !importDecl->_assertions.empty()) {
    sm_.error(
        importDecl->getSourceRange(), "import assertions are not supported");
  }

  curFunction()->semInfo->imports.push_back(importDecl);
  visitESTreeChildren(*this, importDecl);
}

void SemanticValidator::visit(ImportDefaultSpecifierNode *importDecl) {
  // import defaultProperty from 'file.js';
  validateDeclarationNames(
      FunctionInfo::VarDecl::Kind::Var,
      importDecl->_local,
      &curFunction()->semInfo->varDecls);
  visitESTreeChildren(*this, importDecl);
}

void SemanticValidator::visit(ImportNamespaceSpecifierNode *importDecl) {
  // import * as File from 'file.js';
  validateDeclarationNames(
      FunctionInfo::VarDecl::Kind::Var,
      importDecl->_local,
      &curFunction()->semInfo->varDecls);
  visitESTreeChildren(*this, importDecl);
}

void SemanticValidator::visit(ImportSpecifierNode *importDecl) {
  // import {x as y} as File from 'file.js';
  // import {x} as File from 'file.js';
  validateDeclarationNames(
      FunctionInfo::VarDecl::Kind::Var,
      importDecl->_local,
      &curFunction()->semInfo->varDecls);
  visitESTreeChildren(*this, importDecl);
}

void SemanticValidator::visit(ExportNamedDeclarationNode *exportDecl) {
  if (!astContext_.getUseCJSModules()) {
    sm_.error(
        exportDecl->getSourceRange(),
        "'export' statement requires module mode");
  }

  visitESTreeChildren(*this, exportDecl);
}

void SemanticValidator::visit(ExportDefaultDeclarationNode *exportDecl) {
  if (!astContext_.getUseCJSModules()) {
    sm_.error(
        exportDecl->getSourceRange(),
        "'export' statement requires module mode");
  }

  if (auto *funcDecl =
          dyn_cast<ESTree::FunctionDeclarationNode>(exportDecl->_declaration)) {
    if (compile_ && !funcDecl->_id) {
      // If the default function declaration has no name, then change it to a
      // FunctionExpression node for cleaner IRGen.
      auto *funcExpr = new (astContext_) ESTree::FunctionExpressionNode(
          funcDecl->_id,
          std::move(funcDecl->_params),
          funcDecl->_body,
          funcDecl->_typeParameters,
          funcDecl->_returnType,
          funcDecl->_predicate,
          funcDecl->_generator,
          /* async */ false);
      funcExpr->strictness = funcDecl->strictness;
      funcExpr->copyLocationFrom(funcDecl);

      exportDecl->_declaration = funcExpr;
    }
  }

  visitESTreeChildren(*this, exportDecl);
}

void SemanticValidator::visit(ExportAllDeclarationNode *exportDecl) {
  if (!astContext_.getUseCJSModules()) {
    sm_.error(
        exportDecl->getSourceRange(),
        "'export' statement requires CommonJS module mode");
  }
  visitESTreeChildren(*this, exportDecl);
}

void SemanticValidator::visit(CoverEmptyArgsNode *CEA) {
  sm_.error(CEA->getSourceRange(), "invalid empty parentheses '( )'");
}

void SemanticValidator::visit(CoverTrailingCommaNode *CTC) {
  sm_.error(CTC->getSourceRange(), "expression expected after ','");
}

void SemanticValidator::visit(CoverInitializerNode *CI) {
  sm_.error(CI->getStartLoc(), "':' expected in property initialization");
}

void SemanticValidator::visit(CoverRestElementNode *R) {
  sm_.error(R->getSourceRange(), "'...' not allowed in this context");
}

#if HERMES_PARSE_FLOW
void SemanticValidator::visit(CoverTypedIdentifierNode *CTI) {
  sm_.error(CTI->getSourceRange(), "typecast not allowed in this context");
}
#endif

void SemanticValidator::visitFunction(
    FunctionLikeNode *node,
    Node *id,
    NodeList &params,
    Node *body) {
  FunctionContext newFuncCtx{
      this,
      haveActiveContext() && curFunction()->strictMode,
      node,
      haveActiveContext() ? curFunction()->sourceVisibility
                          : SourceVisibility::Default};

  // It is a Syntax Error if UniqueFormalParameters Contains YieldExpression
  // is true.
  // NOTE: isFormalParams_ is reset to false on encountering a new function,
  // because the semantics for "x Contains y" always return `false` when "x" is
  // a function definition.
  llvh::SaveAndRestore<bool> oldIsFormalParamsFn{isFormalParams_, false};

  Node *useStrictNode = nullptr;

  // Note that body might me empty (for lazy functions) or an expression (for
  // arrow functions).
  if (auto *bodyNode = dyn_cast<ESTree::BlockStatementNode>(body)) {
    if (bodyNode->isLazyFunctionBody) {
      // If it is a lazy function body, then the directive nodes in the body are
      // fabricated without location, so don't set useStrictNode.
      scanDirectivePrologue(bodyNode->_body);
    } else {
      useStrictNode = scanDirectivePrologue(bodyNode->_body);
    }
    setDirectiveDerivedInfo(node);
  }

  if (id)
    validateDeclarationNames(FunctionInfo::VarDecl::Kind::Var, id, nullptr);

#if HERMES_PARSE_FLOW
  if (astContext_.getParseFlow() && !params.empty()) {
    // Skip 'this' parameter annotation, and error if it's an arrow parameter,
    // because arrow functions inherit 'this'.
    if (auto *ident = dyn_cast<ESTree::IdentifierNode>(&params.front())) {
      if (ident->_name == kw_.identThis) {
        if (isa<ArrowFunctionExpressionNode>(node)) {
          sm_.error(
              ident->getSourceRange(), "'this' not allowed as parameter name");
        }
        if (compile_) {
          // Delete the node because it cannot be compiled.
          params.erase(params.begin());
        }
      }
    }
  }
#endif

  // Set to false if the parameter list contains binding patterns.
  bool simpleParameterList = true;
  for (auto &param : params) {
    simpleParameterList &= !isa<PatternNode>(param);
    validateDeclarationNames(
        FunctionInfo::VarDecl::Kind::Var,
        &param,
        &newFuncCtx.semInfo->paramNames);
  }

  if (!simpleParameterList && useStrictNode) {
    sm_.error(
        useStrictNode->getSourceRange(),
        "'use strict' not allowed inside function with non-simple parameter list");
  }

  // Check if we have seen this parameter name before.
  if (!simpleParameterList || curFunction()->strictMode ||
      isa<ArrowFunctionExpressionNode>(node)) {
    llvh::SmallSet<NodeLabel, 8> paramNameSet;
    for (const auto &curIdNode : newFuncCtx.semInfo->paramNames) {
      auto insert_result = paramNameSet.insert(curIdNode.identifier->_name);
      if (insert_result.second == false) {
        sm_.error(
            curIdNode.identifier->getSourceRange(),
            "cannot declare two parameters with the same name '" +
                curIdNode.identifier->_name->str() + "'");
      }
    }
  }

  visitParamsAndBody(node);
}

void SemanticValidator::visitParamsAndBody(FunctionLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::FunctionExpression: {
      auto *fe = cast<ESTree::FunctionExpressionNode>(node);
      visitESTreeNode(*this, fe->_id, fe);
      for (auto &param : fe->_params) {
        llvh::SaveAndRestore<bool> oldIsFormalParams{isFormalParams_, true};
        visitESTreeNode(*this, &param, fe);
      }
      visitESTreeNode(*this, fe->_body, fe);
      break;
    }
    case NodeKind::ArrowFunctionExpression: {
      auto *fe = cast<ESTree::ArrowFunctionExpressionNode>(node);
      visitESTreeNode(*this, fe->_id, fe);
      for (auto &param : fe->_params) {
        llvh::SaveAndRestore<bool> oldIsFormalParams{isFormalParams_, true};
        visitESTreeNode(*this, &param, fe);
      }
      visitESTreeNode(*this, fe->_body, fe);
      break;
    }
    case NodeKind::FunctionDeclaration: {
      auto *fe = cast<ESTree::FunctionDeclarationNode>(node);
      visitESTreeNode(*this, fe->_id, fe);
      for (auto &param : fe->_params) {
        llvh::SaveAndRestore<bool> oldIsFormalParams{isFormalParams_, true};
        visitESTreeNode(*this, &param, fe);
      }
      visitESTreeNode(*this, fe->_body, fe);
      visitESTreeNode(*this, fe->_returnType, fe);
      break;
    }
    default:
      visitESTreeChildren(*this, node);
  }
}

void SemanticValidator::tryOverrideSourceVisibility(
    SourceVisibility newSourceVisibility) {
  if (newSourceVisibility > curFunction()->sourceVisibility) {
    curFunction()->sourceVisibility = newSourceVisibility;
  }
}

Node *SemanticValidator::scanDirectivePrologue(NodeList &body) {
  Node *result = nullptr;
  for (auto &nodeRef : body) {
    auto *exprSt = dyn_cast<ESTree::ExpressionStatementNode>(&nodeRef);
    if (!exprSt || !exprSt->_directive)
      break;

    auto *directive = exprSt->_directive;

    if (directive == kw_.identUseStrict) {
      curFunction()->strictMode = true;
      if (!result)
        result = &nodeRef;
    }
    if (directive == kw_.identShowSource) {
      tryOverrideSourceVisibility(SourceVisibility::ShowSource);
    }
    if (directive == kw_.identHideSource) {
      tryOverrideSourceVisibility(SourceVisibility::HideSource);
    }
    if (directive == kw_.identSensitive) {
      tryOverrideSourceVisibility(SourceVisibility::Sensitive);
    }
  }

  return result;
}

bool SemanticValidator::isLValue(const Node *node) const {
  if (isa<MemberExpressionNode>(node))
    return true;
  if (!isa<IdentifierNode>(node))
    return false;

  auto *idNode = cast<IdentifierNode>(node);

  /// 'arguments' cannot be modified in strict mode, but we also don't
  /// support modifying it in non-strict mode yet.
  if (idNode->_name == kw_.identArguments)
    return false;

  // 'eval' cannot be used as a variable in strict mode. If it is disabled we
  // we don't report an error because it will be reported separately.
  if (idNode->_name == kw_.identEval && curFunction()->strictMode &&
      astContext_.getEnableEval())
    return false;

  return true;
}

bool SemanticValidator::isValidDeclarationName(
    const IdentifierNode *idNode) const {
  // 'arguments' cannot be redeclared in strict mode.
  if (idNode->_name == kw_.identArguments && curFunction()->strictMode)
    return false;

  // 'eval' cannot be redeclared in strict mode. If it is disabled we
  // we don't report an error because it will be reported separately.
  if (idNode->_name == kw_.identEval && curFunction()->strictMode &&
      astContext_.getEnableEval())
    return false;

  return true;
}

void SemanticValidator::validateDeclarationNames(
    FunctionInfo::VarDecl::Kind declKind,
    Node *node,
    llvh::SmallVectorImpl<FunctionInfo::VarDecl> *idents) {
  // The identifier is sometimes optional, in which case it is valid.
  if (!node)
    return;

  if (auto *idNode = dyn_cast<IdentifierNode>(node)) {
    if (idents)
      idents->push_back({declKind, idNode});
    if (!isValidDeclarationName(idNode)) {
      sm_.error(
          node->getSourceRange(),
          "cannot declare '" + cast<IdentifierNode>(node)->_name->str() + "'");
    }

    if (declKind != FunctionInfo::VarDecl::Kind::Var &&
        idNode->_name == kw_.identLet) {
      // ES9.0 13.3.1.1
      // LexicalDeclaration : LetOrConst BindingList
      // It is a Syntax Error if the BoundNames of BindingList
      // contains "let".
      sm_.error(
          node->getSourceRange(),
          "'let' is disallowed as a lexically bound name");
    }

    return;
  }

  if (isa<EmptyNode>(node))
    return;

  if (auto *assign = dyn_cast<AssignmentPatternNode>(node))
    return validateDeclarationNames(declKind, assign->_left, idents);

  if (auto *array = dyn_cast<ArrayPatternNode>(node)) {
    for (auto &elem : array->_elements) {
      validateDeclarationNames(declKind, &elem, idents);
    }
    return;
  }

  if (auto *restElem = dyn_cast<RestElementNode>(node)) {
    return validateDeclarationNames(declKind, restElem->_argument, idents);
  }

  if (auto *obj = dyn_cast<ObjectPatternNode>(node)) {
    for (auto &propNode : obj->_properties) {
      if (auto *prop = dyn_cast<PropertyNode>(&propNode)) {
        validateDeclarationNames(declKind, prop->_value, idents);
      } else {
        auto *rest = cast<RestElementNode>(&propNode);
        validateDeclarationNames(declKind, rest->_argument, idents);
      }
    }
    return;
  }

  sm_.error(node->getSourceRange(), "invalid destructuring target");
}

void SemanticValidator::validateAssignmentTarget(const Node *node) {
  if (isa<EmptyNode>(node) || isLValue(node)) {
    return;
  }

  if (auto *assign = dyn_cast<AssignmentPatternNode>(node)) {
    return validateAssignmentTarget(assign->_left);
  }

  if (auto *APN = dyn_cast<ArrayPatternNode>(node)) {
    for (auto &elem : APN->_elements) {
      validateAssignmentTarget(&elem);
    }
    return;
  }

  if (auto *RP = dyn_cast<RestElementNode>(node)) {
    return validateAssignmentTarget(RP->_argument);
  }

  if (auto *obj = dyn_cast<ObjectPatternNode>(node)) {
    for (auto &propNode : obj->_properties) {
      if (auto *prop = dyn_cast<PropertyNode>(&propNode)) {
        assert(
            prop->_kind->str() == "init" &&
            "getters and setters must have been reported by the parser");
        validateAssignmentTarget(prop->_value);
      } else {
        auto *rest = cast<RestElementNode>(&propNode);
        validateAssignmentTarget(rest->_argument);
      }
    }
    return;
  }

  sm_.error(node->getSourceRange(), "invalid assignment left-hand side");
}

void SemanticValidator::setDirectiveDerivedInfo(FunctionLikeNode *node) {
  node->strictness = ESTree::makeStrictness(curFunction()->strictMode);
  node->sourceVisibility = curFunction()->sourceVisibility;
}

LabelDecorationBase *SemanticValidator::getLabelDecorationBase(
    StatementNode *node) {
  if (auto *LS = dyn_cast<LoopStatementNode>(node))
    return LS;
  if (auto *SS = dyn_cast<SwitchStatementNode>(node))
    return SS;
  if (auto *BS = dyn_cast<BreakStatementNode>(node))
    return BS;
  if (auto *CS = dyn_cast<ContinueStatementNode>(node))
    return CS;
  if (auto *LabS = dyn_cast<LabeledStatementNode>(node))
    return LabS;
  llvm_unreachable("invalid node type");
  return nullptr;
}

void SemanticValidator::recursionDepthExceeded(Node *n) {
  sm_.error(
      n->getEndLoc(), "Too many nested expressions/statements/declarations");
}

//===----------------------------------------------------------------------===//
// FunctionContext

FunctionContext::FunctionContext(
    SemanticValidator *validator,
    bool strictMode,
    FunctionLikeNode *node,
    SourceVisibility sourceVisibility)
    : validator_(validator),
      oldContextValue_(validator->funcCtx_),
      node(node),
      semInfo(validator->semCtx_.createFunction()),
      strictMode(strictMode),
      sourceVisibility(sourceVisibility) {
  validator->funcCtx_ = this;

  if (node)
    node->setSemInfo(semInfo);
}

FunctionContext::~FunctionContext() {
  validator_->funcCtx_ = oldContextValue_;
}
} // namespace sem
} // namespace hermes
