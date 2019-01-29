/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "SemanticValidator.h"

#include "hermes/Support/RegExpSerialization.h"

#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/SaveAndRestore.h"

using llvm::cast;
using llvm::cast_or_null;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;
using llvm::SaveAndRestore;

using namespace hermes::ESTree;

namespace hermes {
namespace sem {

//===----------------------------------------------------------------------===//
// Keywords

Keywords::Keywords(Context &astContext)
    : identArguments(
          astContext.getIdentifier("arguments").getUnderlyingPointer()),
      identEval(astContext.getIdentifier("eval").getUnderlyingPointer()),
      identDelete(astContext.getIdentifier("delete").getUnderlyingPointer()),
      identUseStrict(
          astContext.getIdentifier("use strict").getUnderlyingPointer()) {}

//===----------------------------------------------------------------------===//
// SemanticValidator

SemanticValidator::SemanticValidator(
    Context &astContext,
    sem::SemContext &semCtx)
    : astContext_(astContext),
      sm_(astContext.getSourceErrorManager()),
      semCtx_(semCtx),
      initialErrorCount_(sm_.getErrorCount()),
      kw_(astContext) {}

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
#ifndef NDEBUG
  strictnessIsPreset_ = node->strictness != Strictness::NotSet;
#endif
  FunctionContext newFuncCtx{this, astContext_.isStrictMode(), node};

  scanDirectivePrologue(node->_body);
  updateNodeStrictness(node);

  visitESTreeChildren(*this, node);
}

void SemanticValidator::visit(VariableDeclaratorNode *varDecl) {
  validateDeclarationName(varDecl->_id);
  curFunction()->semInfo->decls.push_back(varDecl);
  visitESTreeChildren(*this, varDecl);
}

void SemanticValidator::visit(MetaPropertyNode *metaProp) {
  auto *meta = cast<IdentifierNode>(metaProp->_meta);
  auto *property = cast<IdentifierNode>(metaProp->_property);

  if (meta->_name->str() == "new" && property->_name->str() == "target") {
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
  if (arrowFunc->_expression) {
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
  curFunction()->semInfo->containsArrowFunctionsUsingArguments |=
      arrowFunc->getSemInfo()->containsArrowFunctionsUsingArguments |
      arrowFunc->getSemInfo()->usesArguments;
}

/// Ensure that the left side of for-in is an l-value.
void SemanticValidator::visit(ForInStatementNode *forIn) {
  forIn->setLabelIndex(curFunction()->allocateLabel());

  SaveAndRestore<LoopStatementNode *> saveLoop(
      curFunction()->activeLoop, forIn);
  SaveAndRestore<StatementNode *> saveSwitch(
      curFunction()->activeSwitchOrLoop, forIn);

  if (!isa<VariableDeclarationNode>(forIn->_left))
    if (!isLValue(forIn->_left))
      sm_.error(
          forIn->_left->getSourceRange(),
          "invalid left-hand side in for-in-loop");
  visitESTreeChildren(*this, forIn);
}

/// Ensure that the left side of assgnments is an l-value.
void SemanticValidator::visit(AssignmentExpressionNode *assignment) {
  // Check if the left-hand side is valid.
  if (!isLValue(assignment->_left))
    sm_.error(
        assignment->_left->getSourceRange(),
        "invalid assignment left-hand side");
  visitESTreeChildren(*this, assignment);
}

/// Ensure that the operand of ++/-- is an l-value.
void SemanticValidator::visit(UpdateExpressionNode *update) {
  // Check if the left-hand side is valid.
  if (!isLValue(update->_argument))
    sm_.error(
        update->_argument->getSourceRange(),
        "invalid operand in update operation");
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
        llvm::Twine("label '") + id->_name->str() + "' is already defined");
    sm_.note(
        insertRes.first->second.declarationNode->getSourceRange(),
        "previous definition");
  }
  // Auto-erase the label on exit, if we inserted it.
  const auto &deleter = llvm::make_scope_exit([=]() {
    if (insertRes.second)
      curFunction()->labelMap.erase(id->_name);
  });
  (void)deleter;

  visitESTreeChildren(*this, labelStmt);
}

/// Check RegExp syntax.
void SemanticValidator::visit(RegExpLiteralNode *regexp) {
  llvm::StringRef regexpError;
  if (!CompiledRegExp::tryCompile(
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
  if (tryStatement->_handler && tryStatement->_finalizer) {
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

  tryStatement->surroundingTry = curFunction()->activeTry;

  {
    SaveAndRestore<TryStatementNode *> saveTry(
        curFunction()->activeTry, tryStatement);

    visitESTreeNode(*this, tryStatement->_block);
  }
  visitESTreeNode(*this, tryStatement->_handler);
  visitESTreeNode(*this, tryStatement->_finalizer);
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
  breakStmt->surroundingTry = curFunction()->activeTry;

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
  continueStmt->surroundingTry = curFunction()->activeTry;

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
            llvm::Twine("continue label '") + id->_name->str() +
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
  returnStmt->surroundingTry = curFunction()->activeTry;

  if (curFunction()->isGlobalScope())
    sm_.error(returnStmt->getSourceRange(), "'return' not in a function");
  visitESTreeChildren(*this, returnStmt);
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

void SemanticValidator::visitFunction(
    FunctionLikeNode *node,
    const Node *id,
    NodeList &params,
    Node *body) {
  FunctionContext newFuncCtx{
      this, haveActiveContext() && curFunction()->strictMode, node};

  // Note that body might me empty (for lazy functions) or an expression (for
  // arrow functions).
  if (isa<ESTree::BlockStatementNode>(body)) {
    scanDirectivePrologue(cast<ESTree::BlockStatementNode>(body)->_body);
    updateNodeStrictness(node);
  }

  if (id)
    validateDeclarationName(id);

  for (auto &param : params)
    validateDeclarationName(&param);

  // Check if we have seen this parameter name before.
  if (curFunction()->strictMode) {
    llvm::SmallSet<NodeLabel, 8> paramNameSet;
    for (auto &param : params) {
      auto &curIdNode = cast<IdentifierNode>(param);
      auto insert_result = paramNameSet.insert(curIdNode._name);
      if (insert_result.second == false) {
        sm_.error(
            curIdNode.getSourceRange(),
            "cannot declare two parameters with the same name '" +
                curIdNode._name->str() + "'");
      }
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticValidator::scanDirectivePrologue(NodeList &body) {
  for (auto &nodeRef : body) {
    auto directive = ESTree::matchDirective(&nodeRef);
    if (!directive)
      break;

    if (*directive == kw_.identUseStrict)
      curFunction()->strictMode = true;
  }
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

bool SemanticValidator::isValidDeclarationName(const Node *node) const {
  // The identifier is sometimes optional, in which case it is valid.
  if (!node)
    return true;

  auto *idNode = cast<IdentifierNode>(node);

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

void SemanticValidator::validateDeclarationName(const Node *node) {
  if (!isValidDeclarationName(node)) {
    sm_.error(
        node->getSourceRange(),
        "cannot declare '" + cast<IdentifierNode>(node)->_name->str() + "'");
  }
}

void SemanticValidator::updateNodeStrictness(FunctionLikeNode *node) {
  auto strictness = ESTree::makeStrictness(curFunction()->strictMode);
  assert(
      (!strictnessIsPreset_ || node->strictness == strictness) &&
      "Preset strictness is different from detected strictness");
  node->strictness = strictness;
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

//===----------------------------------------------------------------------===//
// FunctionContext

FunctionContext::FunctionContext(
    SemanticValidator *validator,
    bool strictMode,
    FunctionLikeNode *node)
    : validator_(validator),
      oldContextValue_(validator->funcCtx_),
      semInfo(validator->semCtx_.createFunction()),
      strictMode(strictMode) {
  validator->funcCtx_ = this;

  if (node)
    node->setSemInfo(semInfo);
}

FunctionContext::~FunctionContext() {
  validator_->funcCtx_ = oldContextValue_;
}
} // namespace sem
} // namespace hermes
