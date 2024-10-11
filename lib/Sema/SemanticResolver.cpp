/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SemanticResolver.h"

#include "ASTEval.h"
#include "ScopedFunctionPromoter.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/Sema/SemContext.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/Support/SaveAndRestore.h"

using namespace hermes::ESTree;

namespace hermes {
namespace sema {

SemanticResolver::SemanticResolver(
    Context &astContext,
    sema::SemContext &semCtx,
    const DeclarationFileListTy &ambientDecls,
    DeclCollectorMapTy *saveDecls,
    bool compile)
    : astContext_(astContext),
      sm_(astContext.getSourceErrorManager()),
      bufferMessages_{&sm_},
      semCtx_(semCtx),
      kw_{astContext},
      ambientDecls_(ambientDecls),
      saveDecls_(saveDecls),
      bindingTable_(semCtx.getBindingTable()),
      compile_(compile) {}

bool SemanticResolver::run(ESTree::ProgramNode *rootNode) {
  if (sm_.getErrorCount())
    return false;
  visitESTreeNodeNoReplace(*this, rootNode);
  return sm_.getErrorCount() == 0;
}

bool SemanticResolver::runLazy(
    ESTree::FunctionLikeNode *rootNode,
    sema::FunctionInfo *semInfo,
    bool parentHadSuperBinding) {
  semCtx_.assertGlobalFunctionAndScope();

  if (sm_.getErrorCount())
    return false;

  llvh::SaveAndRestore<BindingTableScopePtrTy> setGlobalScope(
      globalScope_, semCtx_.getBindingTableGlobalScope());

  // For clarity: not going to generate anything in the global function context.
  globalFunctionContext_ = nullptr;

  // Activate the function's binding table scope.
  assert(semInfo->bindingTableScope && "semInfo must have a scope");
  bindingTable_.activateScope(semInfo->bindingTableScope);
  auto restoreScope = llvh::make_scope_exit([this, semInfo]() {
    bindingTable_.activateScope({});
    // Decrement refcount of the binding table scope to allow it to be freed.
    // If we want to keep the binding table scope around for 'eval',
    // do not reset it.
    if (astContext_.getDebugInfoSetting() != DebugInfoSetting::ALL) {
      semInfo->bindingTableScope.reset();
    }
  });

  canReferenceSuper_ = rootNode->isMethodDefinition ||
      (llvh::isa<ESTree::ArrowFunctionExpressionNode>(rootNode) &&
       parentHadSuperBinding);

  // Run the resolver on the function body.
  FunctionContext newFuncCtx{
      *this, rootNode, semInfo, FunctionContext::LazyTag{}};
  visitFunctionLikeInFunctionContext(
      rootNode,
      llvh::cast_or_null<ESTree::IdentifierNode>(
          ESTree::getIdentifier(rootNode)),
      ESTree::getBlockStatement(rootNode),
      ESTree::getParams(rootNode));

  return sm_.getErrorCount() == 0;
}

bool SemanticResolver::runInScope(
    ESTree::ProgramNode *rootNode,
    sema::FunctionInfo *semInfo,
    bool parentHadSuperBinding) {
  llvh::SaveAndRestore<BindingTableScopePtrTy> setGlobalScope(
      globalScope_, semCtx_.getBindingTableGlobalScope());

  assert(semInfo->bindingTableScope && "semInfo must have a scope");
  bindingTable_.activateScope(semInfo->bindingTableScope);
  auto restoreScope = llvh::make_scope_exit([this]() {
    // Pop the scope to prepare for another run of SemanticResolver.
    bindingTable_.activateScope({});
  });
  rootNode->strictness = makeStrictness(semInfo->strict);

  canReferenceSuper_ = parentHadSuperBinding;

  // Run the resolver on the function body.
  FunctionContext newFuncCtx{*this, rootNode, semInfo, semInfo->strict, {}};
  {
    ScopeRAII programScope{*this, rootNode};
    if (sm_.getErrorCount())
      return false;
    // Promote hoisted functions.
    if (!curFunctionInfo()->strict) {
      processPromotedFuncDecls(getPromotedScopedFuncDecls(*this, rootNode));
    }
    processCollectedDeclarations(rootNode);
    visitESTreeChildren(*this, rootNode);
  }

  return sm_.getErrorCount() == 0;
}

bool SemanticResolver::runCommonJSModule(
    ESTree::FunctionExpressionNode *rootNode) {
  semCtx_.assertGlobalFunctionAndScope();

  if (sm_.getErrorCount())
    return false;

  FunctionContext newFuncCtx{
      *this,
      semCtx_.getGlobalFunction(),
      FunctionContext::ExistingGlobalScopeTag{}};
  llvh::SaveAndRestore setGlobalContext{globalFunctionContext_, &newFuncCtx};

  {
    BindingTableScopeTy programBindingScope(bindingTable_);
    llvh::SaveAndRestore<LexicalScope *> saveLexicalScope(
        curScope_, semCtx_.getGlobalScope());
    llvh::SaveAndRestore<BindingTableScopePtrTy> setGlobalScope(
        globalScope_, programBindingScope.ptr());

    visitESTreeNodeNoReplace(*this, rootNode);
  }

  return sm_.getErrorCount() == 0;
}

void SemanticResolver::visit(ESTree::ProgramNode *node) {
  FunctionContext newFuncCtx{
      *this,
      node,
      nullptr,
      astContext_.isStrictMode(),
      CustomDirectives{
          .sourceVisibility = SourceVisibility::Default,
          .alwaysInline = false}};
  llvh::SaveAndRestore setGlobalContext{globalFunctionContext_, &newFuncCtx};
  FoundDirectives directives = scanDirectives(node->_body);
  if (directives.useStrictNode)
    curFunctionInfo()->strict = true;
  node->strictness = makeStrictness(curFunctionInfo()->strict);
  if (directives.sourceVisibility >
      curFunctionInfo()->customDirectives.sourceVisibility)
    curFunctionInfo()->customDirectives.sourceVisibility =
        directives.sourceVisibility;

  {
    ScopeRAII programScope{*this, node};
    newFuncCtx.bindingTableScopeDepth =
        programScope.getBindingScope().getDepth();
    llvh::SaveAndRestore<BindingTableScopePtrTy> setGlobalScope(
        globalScope_, programScope.getBindingScope().ptr());
    semCtx_.setBindingTableGlobalScope(globalScope_);
    if (astContext_.getDebugInfoSetting() == DebugInfoSetting::ALL) {
      curFunctionInfo()->bindingTableScope = globalScope_;
    }

    processCollectedDeclarations(node);
    if (!curFunctionInfo()->strict) {
      // Promote hoisted functions.
      processPromotedFuncDecls(getPromotedScopedFuncDecls(*this, node));
    }
    processAmbientDecls();
    visitESTreeChildren(*this, node);
  }
}

void SemanticResolver::visit(
    ESTree::FunctionDeclarationNode *funcDecl,
    ESTree::Node *parent) {
  curScope_->hoistedFunctions.push_back(funcDecl);
  visitFunctionLike(
      funcDecl,
      llvh::cast_or_null<ESTree::IdentifierNode>(funcDecl->_id),
      funcDecl->_body,
      funcDecl->_params,
      parent);
}
void SemanticResolver::visit(
    ESTree::FunctionExpressionNode *funcExpr,
    ESTree::Node *parent) {
  visitFunctionExpression(funcExpr, funcExpr->_body, funcExpr->_params, parent);
}
void SemanticResolver::visit(
    ESTree::ArrowFunctionExpressionNode *arrowFunc,
    ESTree::Node *parent) {
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
  visitFunctionLike(
      arrowFunc, nullptr, arrowFunc->_body, arrowFunc->_params, parent);

  curFunctionInfo()->containsArrowFunctions = true;
  curFunctionInfo()->containsArrowFunctionsUsingArguments =
      curFunctionInfo()->containsArrowFunctionsUsingArguments ||
      arrowFunc->getSemInfo()->containsArrowFunctionsUsingArguments ||
      arrowFunc->getSemInfo()->usesArguments;
}

void SemanticResolver::visit(
    ESTree::IdentifierNode *identifier,
    ESTree::Node *parent) {
  if (auto *prop = llvh::dyn_cast<PropertyNode>(parent)) {
    if (!prop->_computed && prop->_key == identifier) {
      // { identifier: ... }
      return;
    }
  }

  if (auto *mem = llvh::dyn_cast<MemberExpressionLikeNode>(parent)) {
    if (!ESTree::getComputed(mem) && ESTree::getProperty(mem) == identifier) {
      // expr.identifier
      // expr?.identifier
      return;
    }
  }

  // Identifiers that aren't variables.
  if (llvh::isa<MetaPropertyNode>(parent) ||
      llvh::isa<BreakStatementNode>(parent) ||
      llvh::isa<ContinueStatementNode>(parent) ||
      llvh::isa<LabeledStatementNode>(parent)) {
    return;
  }

  // typeof
  if (auto *unary = llvh::dyn_cast<UnaryExpressionNode>(parent)) {
    if (unary->_operator == kw_.identTypeof) {
      resolveIdentifier(identifier, true);
    }
  }

  // $SHBuiltin should have been replaced with SHBuiltinNode as part of a member
  // call expression earlier. Any use that gets here is invalid.
  if (identifier->_name == kw_.identSHBuiltin) {
    sm_.error(identifier->getSourceRange(), "invalid use of $SHBuiltin");
  }

  resolveIdentifier(identifier, false);
}

void SemanticResolver::visit(ESTree::VariableDeclarationNode *node) {
  visitESTreeChildren(*this, node);

  // ES5Catch, var
  //          -> valid, special case ES10 B.3.5
  // let, var
  //          -> always invalid
  // Ordinarily, we check this in validateAndDeclareIdentifier, but if the
  // declarations are in a nested scope like x or y here:
  //
  // function f() {
  //   { let x; var x; }
  //   { let y; { var y; } }
  // }
  //
  // then the var has been hoisted to the function-level scope by
  // DeclCollector and we aren't able to detect that both declarations are
  // actually in the same scope and conflict.
  // Only perform this check for nested scopes, because the var will have
  // been hoisted into a different scope.
  if (node->_kind == kw_.identVar &&
      curScope_ != curScope_->parentFunction->getFunctionScope()) {
    llvh::SmallVector<ESTree::IdentifierNode *, 2> idents;
    extractIdentsFromDecl(node, idents);
    // Check every identifier declared as a 'var'.
    for (ESTree::IdentifierNode *ident : idents) {
      const auto [prevName, prevDepth] =
          bindingTable_.findWithDepth(ident->_name);

      if (!prevName) {
        // No existing declaration, move on.
        continue;
      }

      // Whether the prevName is the lexical binding for a promoted function
      // which reuses the same Decl.
      // If it is a lexical binding of a promoted function,
      // that's an error due to a lexically-scoped and var-scoped naming
      // conflict.
      bool prevIsLexicalBindingOfPromotedFunc =
          functionContext()->promotedFuncDecls.count(ident->_name) &&
          prevDepth != functionContext()->bindingTableScopeDepth;

      if (prevName->decl->scope ==
              prevName->decl->scope->parentFunction->getFunctionScope() &&
          !prevIsLexicalBindingOfPromotedFunc) {
        // If the previous declaration is in the function scope, the error would
        // have been reported when validating declarations in the function
        // scope.
        continue;
      }

      // Report an error if the var is trying to override a let-like
      // declaration.
      //
      // ES10.0 B.3.4: ES5Catch (only used for simple binding ident in catch
      // block) is not an error if it conflicts with VarDeclaredNames in its
      // body.
      const Decl::Kind prevKind = prevName->decl->kind;
      if ((Decl::isKindLetLike(prevKind) && prevKind != Decl::Kind::ES5Catch) ||
          prevIsLexicalBindingOfPromotedFunc) {
        sm_.error(
            ident->getSourceRange(),
            llvh::Twine("Identifier '") + ident->_name->str() +
                "' is already declared");
        if (prevName->ident)
          sm_.note(prevName->ident->getSourceRange(), "previous declaration");
      }
    }
  }
}

void SemanticResolver::visit(
    ESTree::BinaryExpressionNode *node,
    ESTree::Node **ppNode) {
  // Handle nested +/- non-recursively.
  if (node->_operator == kw_.identPlus || node->_operator == kw_.identMinus) {
    auto list = linearizeLeft(node, {"+", "-"});
    if (list.size() > MAX_NESTED_BINARY) {
      recursionDepthExceeded(node);
      return;
    }

    visitESTreeNode(*this, list[0]->_left, list[0]);
    for (auto *e : list)
      visitESTreeNode(*this, e->_right, e);

    // If compiling, fold all expressions bottom up (left to right).
    if (compile_) {
      for (size_t i = 0, e = list.size(); i < e; ++i) {
        ESTree::Node **foldResult = i + 1 < e ? &list[i + 1]->_left : ppNode;
        // Attempt to fold the expression. If it fails, stop folding, since
        // all subsequent expressions depend on the result of this one.
        if (!astFoldBinaryExpression(kw_, list[i], foldResult))
          break;
      }
    }
    return;
  }

  visitESTreeChildren(*this, node);
  if (compile_)
    astFoldBinaryExpression(kw_, node, ppNode);
}

void SemanticResolver::visit(ESTree::AssignmentExpressionNode *assignment) {
  // Handle nested "=" non-recursively.
  if (assignment->_operator == kw_.identAssign) {
    auto list = linearizeRight(assignment, {"="});
    if (list.size() > MAX_NESTED_ASSIGNMENTS) {
      recursionDepthExceeded(assignment);
      return;
    }

    for (auto *e : list) {
      visitESTreeNode(*this, e->_left, e);
      validateAssignmentTarget(e->_left);
    }
    visitESTreeNode(*this, list.back()->_right, list.back());
    return;
  }

  visitESTreeNode(*this, assignment->_left, assignment);
  validateAssignmentTarget(assignment->_left);
  visitESTreeNode(*this, assignment->_right, assignment);
}

void SemanticResolver::visit(ESTree::UpdateExpressionNode *node) {
  visitESTreeChildren(*this, node);
  if (!isLValue(node->_argument)) {
    sm_.error(
        node->_argument->getSourceRange(),
        "invalid operand in update operation");
  }
}

void SemanticResolver::visit(
    ESTree::UnaryExpressionNode *node,
    ESTree::Node **ppNode) {
  // Check for unqualified delete in strict mode.
  if (node->_operator == kw_.identDelete) {
    if (curFunctionInfo()->strict &&
        llvh::isa<IdentifierNode>(node->_argument)) {
      sm_.error(
          node->getSourceRange(),
          "'delete' of a variable is not allowed in strict mode");
    }
    // Unless we are running under compliance tests, report an error on
    // `delete super.x`.
    if (!astContext_.getCodeGenerationSettings().test262) {
      if (auto *mem = llvh::dyn_cast<MemberExpressionNode>(node->_argument);
          mem && llvh::isa<SuperNode>(mem->_object)) {
        sm_.error(
            node->getSourceRange(),
            "'delete' of super property is not allowed");
      }
    }
  }
  visitESTreeChildren(*this, node);
  if (compile_)
    astFoldUnaryExpression(kw_, node, ppNode);
}

void SemanticResolver::visit(
    ESTree::BlockStatementNode *node,
    ESTree::Node *parent) {
  // Some nodes with attached BlockStatement have already dealt with the scope.
  if (llvh::isa<FunctionDeclarationNode>(parent) ||
      llvh::isa<FunctionExpressionNode>(parent) ||
      llvh::isa<ArrowFunctionExpressionNode>(parent) ||
      llvh::isa<CatchClauseNode>(parent)) {
    return visitESTreeChildren(*this, node);
  }

  ScopeRAII blockScope{*this, node};
  if (const ScopeDecls *declsOpt =
          functionContext()->decls->getScopeDeclsForNode(node)) {
    processDeclarations(*declsOpt);
  }
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::SwitchStatementNode *node) {
  // Visit the discriminant before creating a new scope.
  visitESTreeNode(*this, node->_discriminant, node);

  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<StatementNode *> saveSwitch(
      functionContext()->currentLoopOrSwitch, node);

  ScopeRAII nameScope{*this, node};
  if (const ScopeDecls *declsOpt =
          functionContext()->decls->getScopeDeclsForNode(node)) {
    // Only process a lexical scope if there are declarations in it.
    processDeclarations(*declsOpt);
  }

  visitESTreeNodeList(*this, node->_cases, node);
}

void SemanticResolver::visit(ESTree::ForInStatementNode *node) {
  visitForInOf(node, node, node->_left, node->_right, node->_body);
}

void SemanticResolver::visit(ESTree::ForOfStatementNode *node) {
  if (compile_ && node->_await)
    sm_.error(node->getStartLoc(), "for await is not supported");
  visitForInOf(node, node, node->_left, node->_right, node->_body);
}

void SemanticResolver::visitForInOf(
    ESTree::LoopStatementNode *node,
    ESTree::ScopeDecorationBase *scopeDeco,
    ESTree::Node *left,
    ESTree::Node *right,
    ESTree::Node *body) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(
      functionContext()->currentLoop, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(
      functionContext()->currentLoopOrSwitch, node);

  ScopeRAII nameScope{*this, scopeDeco};
  if (const ScopeDecls *declsOpt =
          functionContext()->decls->getScopeDeclsForNode(node)) {
    processDeclarations(*declsOpt);
  }
  visitESTreeNode(*this, left, node);

  // Ensure the initializer is valid.
  if (auto *vd = llvh::dyn_cast<VariableDeclarationNode>(left)) {
    assert(
        vd->_declarations.size() == 1 &&
        "for-in/for-of must have a single binding");

    auto *declarator =
        llvh::cast<ESTree::VariableDeclaratorNode>(&vd->_declarations.front());

    if (declarator->_init) {
      if (llvh::isa<ESTree::PatternNode>(declarator->_id)) {
        sm_.error(
            declarator->_init->getSourceRange(),
            "destructuring declaration cannot be initialized in for-in/for-of loop");
      } else if (!(llvh::isa<ForInStatementNode>(node) &&
                   !curFunctionInfo()->strict && vd->_kind == kw_.identVar)) {
        sm_.error(
            declarator->_init->getSourceRange(),
            "for-in/for-of variable declaration may not be initialized");
      }
    }
  } else {
    validateAssignmentTarget(left);
  }

  visitESTreeNode(*this, right, node);
  visitESTreeNode(*this, body, node);
}

void SemanticResolver::visit(ESTree::ForStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(
      functionContext()->currentLoop, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(
      functionContext()->currentLoopOrSwitch, node);

  ScopeRAII nameScope{*this, node};
  if (const ScopeDecls *declsOpt =
          functionContext()->decls->getScopeDeclsForNode(node)) {
    processDeclarations(*declsOpt);
  }
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::DoWhileStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(
      functionContext()->currentLoop, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(
      functionContext()->currentLoopOrSwitch, node);

  visitESTreeChildren(*this, node);
}
void SemanticResolver::visit(ESTree::WhileStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(
      functionContext()->currentLoop, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(
      functionContext()->currentLoopOrSwitch, node);

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::LabeledStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  // Determine the target statement. We need to check if it directly encloses
  // a loop or another label enclosing a loop.
  StatementNode *targetStmt = node;
  {
    Node *curStmt = node;
    while (auto *curLabeled = llvh::dyn_cast<LabeledStatementNode>(curStmt)) {
      if (auto *ls = llvh::dyn_cast<LoopStatementNode>(curLabeled->_body)) {
        targetStmt = ls;
        break;
      }
      curStmt = curLabeled->_body;
    }
  }
  assert(
      (llvh::isa<LoopStatementNode>(targetStmt) ||
       llvh::isa<LabeledStatementNode>(targetStmt)) &&
      "invalid target statement detected for label");

  auto *id = cast<IdentifierNode>(node->_label);
  // Define the new label, checking for a previous definition.
  auto insertRes = functionContext()->labelMap.try_emplace(
      id->_name, FunctionContext::Label{id, targetStmt});
  if (!insertRes.second) {
    sm_.error(
        id->getSourceRange(),
        llvh::Twine("label '") + id->_name->str() + "' is already defined");
    sm_.note(
        insertRes.first->second.declarationNode->getSourceRange(),
        "previous definition");
  }
  // Auto-erase the label on exit, if we inserted it.
  const auto &deleter = llvh::make_scope_exit([this, id, insertRes]() {
    if (insertRes.second)
      functionContext()->labelMap.erase(id->_name);
  });
  (void)deleter;

  visitESTreeChildren(*this, node);
}

/// Get the LabelDecorationBase depending on the node type.
static LabelDecorationBase *getLabelDecorationBase(StatementNode *node) {
  if (auto *LS = llvh::dyn_cast<LoopStatementNode>(node))
    return LS;
  if (auto *SS = llvh::dyn_cast<SwitchStatementNode>(node))
    return SS;
  if (auto *BS = llvh::dyn_cast<BreakStatementNode>(node))
    return BS;
  if (auto *CS = llvh::dyn_cast<ContinueStatementNode>(node))
    return CS;
  if (auto *LabS = llvh::dyn_cast<LabeledStatementNode>(node))
    return LabS;
  llvm_unreachable("invalid node type");
  return nullptr;
}

void SemanticResolver::visit(ESTree::BreakStatementNode *node) {
  if (node->_label) {
    const NodeLabel &name = llvh::cast<IdentifierNode>(node->_label)->_name;
    auto it = functionContext()->labelMap.find(name);
    if (it != functionContext()->labelMap.end()) {
      auto labelIndex =
          getLabelDecorationBase(it->second.targetStatement)->getLabelIndex();
      node->setLabelIndex(labelIndex);
    } else {
      sm_.error(
          node->_label->getSourceRange(),
          llvh::Twine("label '") + name->str() + "' is not defined");
    }
  } else {
    if (functionContext()->currentLoopOrSwitch) {
      auto labelIndex =
          getLabelDecorationBase(functionContext()->currentLoopOrSwitch)
              ->getLabelIndex();
      node->setLabelIndex(labelIndex);
    } else {
      sm_.error(
          node->getSourceRange(), "'break' not within a loop or a switch");
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::ContinueStatementNode *node) {
  if (node->_label) {
    const NodeLabel &name = llvh::cast<IdentifierNode>(node->_label)->_name;
    auto it = functionContext()->labelMap.find(name);
    if (it != functionContext()->labelMap.end()) {
      if (llvh::isa<LoopStatementNode>(it->second.targetStatement)) {
        auto labelIndex =
            getLabelDecorationBase(it->second.targetStatement)->getLabelIndex();
        node->setLabelIndex(labelIndex);
      } else {
        sm_.error(
            node->_label->getSourceRange(),
            llvh::Twine("'continue' label '") + name->str() +
                "' is not a loop label");
        sm_.note(
            it->second.declarationNode->getSourceRange(), "label defined here");
      }
    } else {
      sm_.error(
          node->_label->getSourceRange(),
          llvh::Twine("label '") + name->str() + "' is not defined");
    }
  } else {
    if (functionContext()->currentLoop) {
      auto labelIndex = functionContext()->currentLoop->getLabelIndex();
      node->setLabelIndex(labelIndex);
    } else {
      sm_.error(node->getSourceRange(), "'continue' not within a loop");
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::WithStatementNode *node) {
  if (compile_)
    sm_.error(node->getStartLoc(), "with statement is not supported");

  visitESTreeChildren(*this, node);

  uint32_t depth = curScope_->depth;
  // Run the Unresolver to avoid resolving to variables past the depth of the
  // `with`.
  // Pass `depth + 1` because variables declared in this scope also cannot be
  // trusted.
  Unresolver::run(semCtx_, depth + 1, node->_body);
}

void SemanticResolver::visit(ESTree::TryStatementNode *tryStatement) {
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

void SemanticResolver::visit(ESTree::CatchClauseNode *node) {
  ScopeRAII scope{*this, node};
  // Process body's declarations, skip visiting the block, visit its children.
  processCollectedDeclarations(node);
  // Visit the catch param, in case there is destructuring.
  visitESTreeNode(*this, node->_param, node);
  // The scope declarations are associated with the CatchClauseNode in the
  // DeclCollector.
  visitESTreeChildren(*this, node->_body);
  assert(
      !functionContext()->decls->getScopeDeclsForNode(node->_body) &&
      "CatchClause body block shouldn't have any decls associated");
}

void SemanticResolver::visit(RegExpLiteralNode *regexp) {
  llvh::StringRef regexpError;
  if (compile_) {
    if (auto compiled = CompiledRegExp::tryCompile(
            regexp->_pattern->str(), regexp->_flags->str(), &regexpError)) {
      astContext_.addCompiledRegExp(
          regexp->_pattern, regexp->_flags, std::move(*compiled));
    } else {
      sm_.error(
          regexp->getSourceRange(),
          "Invalid regular expression: " + Twine(regexpError));
    }
  }
  visitESTreeChildren(*this, regexp);
}

void SemanticResolver::visit(ESTree::MetaPropertyNode *node) {
  auto *meta = llvh::cast<IdentifierNode>(node->_meta);
  auto *property = llvh::cast<IdentifierNode>(node->_property);

  if (meta->_name == kw_.identNew && property->_name == kw_.identTarget) {
    if (functionContext()->isGlobalScope()) {
      // ES9.0 15.1.1:
      // It is a Syntax Error if StatementList Contains NewTarget unless the
      // source code containing NewTarget is eval code that is being processed
      // by a direct eval.
      // Hermes does not support local eval, so we assume that this is not
      // inside a local eval call.
      sm_.error(node->getSourceRange(), "'new.target' not in a function");
    }
    return;
  }

  if (meta->_name->str() == "import" && property->_name->str() == "meta") {
    if (compile_) {
      sm_.error(
          node->getSourceRange(), "'import.meta' is currently unsupported");
    }
    return;
  }

  sm_.error(
      node->getSourceRange(),
      llvh::Twine("invalid meta property ") + meta->_name->str() + "." +
          property->_name->str());
}

void SemanticResolver::visit(ESTree::ImportDeclarationNode *importDecl) {
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

  curFunctionInfo()->imports.push_back(importDecl);
  visitESTreeChildren(*this, importDecl);
}

void SemanticResolver::visit(ESTree::ClassDeclarationNode *node) {
  // Classes must be in strict mode.
  llvh::SaveAndRestore<bool> oldStrict{curFunctionInfo()->strict, true};
  ClassContext classCtx(*this, node);
  visitESTreeChildren(*this, node);
  curClassContext_->createImplicitConstructorFunctionInfo();
}

void SemanticResolver::visit(ESTree::ClassExpressionNode *node) {
  // Classes must be in strict mode.
  llvh::SaveAndRestore<bool> oldStrict{curFunctionInfo()->strict, true};

  ClassContext classCtx(*this, node);

  if (ESTree::IdentifierNode *ident =
          llvh::dyn_cast_or_null<IdentifierNode>(node->_id)) {
    // If there is a name, declare it.
    ScopeRAII scope{*this, node};
    if (validateDeclarationName(Decl::Kind::ClassExprName, ident)) {
      Decl *decl = semCtx_.newDeclInScope(
          ident->_name, Decl::Kind::ClassExprName, curScope_);
      semCtx_.setDeclarationDecl(ident, decl);
      bindingTable_.try_emplace(ident->_name, Binding{decl, ident});
    }
    visitESTreeChildren(*this, node);
  } else {
    // Otherwise, no extra scope needed, just move on.
    visitESTreeChildren(*this, node);
  }
  curClassContext_->createImplicitConstructorFunctionInfo();
}

void SemanticResolver::visit(PrivateNameNode *node) {
  if (compile_)
    sm_.error(node->getSourceRange(), "private properties are not supported");
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ClassPrivatePropertyNode *node) {
  if (compile_)
    sm_.error(node->getSourceRange(), "private properties are not supported");
  // Only visit the init expression, since it needs to be resolved.
  if (node->_value) {
    sm_.error(
        node->getSourceRange(), "property initialization is not supported yet");
    if (0) {
      // TODO: visit the properties in the context of a synthetic method.
      visitESTreeNode(*this, node->_value, node);
    }
  }
}

void SemanticResolver::visit(ESTree::ClassPropertyNode *node) {
  // If computed property, the key expression needs to be resolved.
  if (node->_computed) {
    // Computed keys cannot reference super.
    llvh::SaveAndRestore<bool> oldCanRefSuper{canReferenceSuper_, false};
    visitESTreeNode(*this, node->_key, node);
  }

  // Visit the init expression, since it needs to be resolved.
  if (node->_value) {
    // We visit the initializer expression in the context of a synthesized
    // method that performs the initializations.
    // Field initializers can always reference super.
    llvh::SaveAndRestore<bool> oldCanRefSuper{canReferenceSuper_, true};
    FunctionContext funcCtx(
        *this, curClassContext_->getOrCreateFieldInitFunctionInfo());
    visitESTreeNode(*this, node->_value, node);
  }
}

void SemanticResolver::visit(ESTree::SuperNode *node, ESTree::Node *parent) {
  // Error if we try to reference super but there is currently no valid binding
  // to it.
  if (llvh::isa<MemberExpressionLikeNode>(parent) && !canReferenceSuper_) {
    sm_.error(parent->getSourceRange(), "super not allowed here");
  }
}

void SemanticResolver::visit(
    ESTree::MethodDefinitionNode *node,
    ESTree::Node *parent) {
  // If computed property, the key expression needs to be resolved.
  if (node->_computed)
    visitESTreeNode(*this, node->_key, node);

  // Visit the body.
  visitESTreeNode(*this, node->_value, node);
}

void SemanticResolver::visit(ESTree::CallExpressionNode *node) {
  // Check for a direct call to local `eval()`.
  if (auto *identifier = llvh::dyn_cast<IdentifierNode>(node->_callee)) {
    if (identifier->_name == kw_.identEval) {
      // Check to see whether it looks like attempting to call the actual
      // global eval and generate a warning.
      bool isEval;
      if (Binding *binding = bindingTable_.find(identifier->_name)) {
        Decl *decl = binding->decl;
        isEval = decl->scope == semCtx_.getGlobalScope() &&
            (decl->kind == Decl::Kind::UndeclaredGlobalProperty ||
             decl->kind == Decl::Kind::GlobalProperty);
      } else {
        isEval = true;
      }

      // Register the local eval, but only if eval is enabled.
      if (astContext_.getEnableEval()) {
        if (isEval) {
          sm_.warning(
              Warning::DirectEval,
              node->_callee->getSourceRange(),
              "Direct call to eval(), but lexical scope is not supported.");
        }
        registerLocalEval(curScope_);
      } else {
        if (isEval) {
          sm_.warning(
              Warning::EvalDisabled,
              node->_callee->getSourceRange(),
              "eval() is disabled at runtime");
        }
      }
    }
  }

  // Check for $SHBuiltin, and transform the node if necessary to SHBuiltinNode.
  // This allows typechecker/IRGen to simply match on SHBuiltinNode.
  if (auto *methodCallee =
          llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
    if (auto *ident =
            llvh::dyn_cast<ESTree::IdentifierNode>(methodCallee->_object)) {
      if (ident->_name == kw_.identSHBuiltin && !methodCallee->_computed) {
        Decl *decl = resolveIdentifier(ident, false);
        if (decl && decl->kind == sema::Decl::Kind::UndeclaredGlobalProperty) {
          auto *shBuiltin = new (astContext_) ESTree::SHBuiltinNode();
          shBuiltin->copyLocationFrom(methodCallee->_object);
          methodCallee->_object = shBuiltin;
        }
      }
    }
  }

  if (llvh::isa<SuperNode>(node->_callee)) {
    if (!functionContext()->nearestNonArrow()->isConstructor) {
      sm_.error(
          node->getSourceRange(), "super() call only allowed in constructor");
    }
    if (!(curClassContext_ && curClassContext_->isDerivedClass())) {
      sm_.error(
          node->getSourceRange(),
          "super() call only allowed in subclass constructor");
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::SpreadElementNode *node, Node *parent) {
  if (!llvh::isa<ESTree::ObjectExpressionNode>(parent) &&
      !llvh::isa<ESTree::ArrayExpressionNode>(parent) &&
      !llvh::isa<ESTree::CallExpressionNode>(parent) &&
      !llvh::isa<ESTree::OptionalCallExpressionNode>(parent) &&
      !llvh::isa<ESTree::NewExpressionNode>(parent))
    sm_.error(node->getSourceRange(), "spread operator is not supported");
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::ReturnStatementNode *returnStmt) {
  if (functionContext()->isGlobalScope() &&
      !astContext_.allowReturnOutsideFunction())
    sm_.error(returnStmt->getSourceRange(), "'return' not in a function");
  visitESTreeChildren(*this, returnStmt);
}

void SemanticResolver::visit(ESTree::YieldExpressionNode *node) {
  if (functionContext()->isGlobalScope() ||
      (functionContext()->node &&
       !ESTree::isGenerator(functionContext()->node))) {
    sm_.error(node->getSourceRange(), "'yield' not in a generator function");
  }

  if (functionContext()->isFormalParams) {
    // For generators functions (the only time YieldExpression is parsed):
    // It is a Syntax Error if UniqueFormalParameters Contains YieldExpression
    // is true.
    sm_.error(
        node->getSourceRange(), "'yield' not allowed in a formal parameter");
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::AwaitExpressionNode *awaitExpr) {
  if (functionContext()->isGlobalScope() ||
      (functionContext()->node && !ESTree::isAsync(functionContext()->node))) {
    sm_.error(awaitExpr->getSourceRange(), "'await' not in an async function");
  }

  if (functionContext()->isFormalParams) {
    // ES14.0 15.8.1
    // It is a Syntax Error if FormalParameters Contains AwaitExpression
    // is true.
    sm_.error(
        awaitExpr->getSourceRange(),
        "'await' not allowed in a formal parameter");
  }

  visitESTreeChildren(*this, awaitExpr);
}

void SemanticResolver::visit(ESTree::ExportNamedDeclarationNode *node) {
  if (compile_ && !astContext_.getUseCJSModules()) {
    sm_.error(
        node->getSourceRange(), "'export' statement requires module mode");
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::ExportDefaultDeclarationNode *node) {
  if (compile_ && !astContext_.getUseCJSModules()) {
    sm_.error(
        node->getSourceRange(), "'export' statement requires module mode");
  }

  if (auto *funcDecl =
          llvh::dyn_cast<ESTree::FunctionDeclarationNode>(node->_declaration)) {
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

      node->_declaration = funcExpr;
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::ExportAllDeclarationNode *node) {
  if (compile_ && !astContext_.getUseCJSModules()) {
    sm_.error(
        node->getSourceRange(),
        "'export' statement requires CommonJS module mode");
  }
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(CoverEmptyArgsNode *node) {
  sm_.error(node->getSourceRange(), "invalid empty parentheses '( )'");
}

void SemanticResolver::visit(CoverTrailingCommaNode *node) {
  sm_.error(node->getSourceRange(), "expression expected after ','");
}

void SemanticResolver::visit(CoverInitializerNode *node) {
  sm_.error(node->getStartLoc(), "':' expected in property initialization");
}

void SemanticResolver::visit(CoverRestElementNode *node) {
  sm_.error(node->getSourceRange(), "'...' not allowed in this context");
}

#if HERMES_PARSE_FLOW
void SemanticResolver::visit(CoverTypedIdentifierNode *node) {
  sm_.error(node->getSourceRange(), "typecast not allowed in this context");
}

void SemanticResolver::visit(TypeAliasNode *node) {
  // Do nothing.
}

void SemanticResolver::visit(TypeParameterDeclarationNode *node) {
  // Do nothing.
}

void SemanticResolver::visit(TypeParameterInstantiationNode *node) {
  // Do nothing.
}

void SemanticResolver::visit(TypeCastExpressionNode *node) {
  // Visit the expression, but not the type annotation.
  visitESTreeNode(*this, node->_expression, node);
}

void SemanticResolver::visit(AsExpressionNode *node) {
  // Visit the expression, but not the type annotation.
  visitESTreeNode(*this, node->_expression, node);
}

/// Process a component declaration by creating a new FunctionContext.
void SemanticResolver::visit(
    ComponentDeclarationNode *componentDecl,
    ESTree::Node *parent) {
  visitFunctionLike(
      componentDecl,
      llvh::cast<ESTree::IdentifierNode>(componentDecl->_id),
      componentDecl->_body,
      componentDecl->_params,
      parent);
}

void SemanticResolver::visit(
    HookDeclarationNode *hookDecl,
    ESTree::Node *parent) {
  visitFunctionLike(
      hookDecl,
      llvh::cast<ESTree::IdentifierNode>(hookDecl->_id),
      hookDecl->_body,
      hookDecl->_params,
      parent);
}

#endif

#if HERMES_PARSE_TS

void SemanticResolver::visit(ESTree::TSTypeAliasDeclarationNode *node) {
  // Do nothing.
}

void SemanticResolver::visit(ESTree::TSTypeParameterDeclarationNode *node) {
  // Do nothing.
}

void SemanticResolver::visit(ESTree::TSTypeParameterInstantiationNode *node) {
  // Do nothing.
}

void SemanticResolver::visit(ESTree::TSAsExpressionNode *node) {
  // Do nothing.
}

#endif

void SemanticResolver::visitFunctionLike(
    ESTree::FunctionLikeNode *node,
    ESTree::IdentifierNode *id,
    ESTree::Node *body,
    ESTree::NodeList &params,
    ESTree::Node *parent) {
  FunctionContext newFuncCtx{
      *this,
      node,
      curFunctionInfo(),
      curFunctionInfo()->strict,
      curFunctionInfo()->customDirectives};
  if (auto *method =
          llvh::dyn_cast_or_null<ESTree::MethodDefinitionNode>(parent)) {
    newFuncCtx.isConstructor = method->_kind == kw_.identConstructor;
    if (newFuncCtx.isConstructor) {
      curClassContext_->hasConstructor = true;
    }
  }

  // Arrow functions should inherit their current super binding. All other
  // functions can only reference super properties if it was defined as a
  // method.
  bool newCanRefSuper = llvh::isa<ArrowFunctionExpressionNode>(node)
      ? canReferenceSuper_
      : node->isMethodDefinition;
  llvh::SaveAndRestore<bool> oldCanRefSuper{canReferenceSuper_, newCanRefSuper};
  visitFunctionLikeInFunctionContext(node, id, body, params);
}

void SemanticResolver::visitFunctionLikeInFunctionContext(
    ESTree::FunctionLikeNode *node,
    ESTree::IdentifierNode *id,
    ESTree::Node *body,
    ESTree::NodeList &params) {
  if (compile_ && ESTree::isAsync(node) && ESTree::isGenerator(node)) {
    sm_.error(node->getSourceRange(), "async generators are unsupported");
  }

  FoundDirectives directives{};

  // Arrow functions have their bodies turned into BlockStatement before visit.
  auto *blockBody = llvh::cast<BlockStatementNode>(body);
  directives = scanDirectives(blockBody->_body);

  // Set the strictness if necessary.
  if (directives.useStrictNode)
    curFunctionInfo()->strict = true;
  node->strictness = makeStrictness(curFunctionInfo()->strict);
  if (directives.sourceVisibility >
      curFunctionInfo()->customDirectives.sourceVisibility)
    curFunctionInfo()->customDirectives.sourceVisibility =
        directives.sourceVisibility;
  curFunctionInfo()->customDirectives.alwaysInline = directives.alwaysInline;
  curFunctionInfo()->customDirectives.noInline = directives.noInline;

  if (id) {
    // Set the expression decl of the id.
    semCtx_.setExpressionDecl(id, semCtx_.getDeclarationDecl(id));
    validateDeclarationName(Decl::Kind::FunctionExprName, id);
  }

  if (blockBody->isLazyFunctionBody) {
    // Don't descend into lazy functions, don't create a scope.
    // But do record the surrounding scope in the FunctionInfo.
    assert(node->getSemInfo() && "semInfo must be set in first pass");
    node->getSemInfo()->bindingTableScope = bindingTable_.getCurrentScope();
    node->getSemInfo()->containsArrowFunctions =
        blockBody->containsArrowFunctions;
    node->getSemInfo()->containsArrowFunctionsUsingArguments =
        blockBody->mayContainArrowFunctionsUsingArguments;
    return;
  }

  // Create the function scope.
  // Note that we are not associating the new scope with an AST node. It should
  // be accessed from FunctionInfo::getFunctionScope().
  ScopeRAII scope{*this};

  functionContext()->bindingTableScopeDepth =
      scope.getBindingScope().getDepth();

  if (astContext_.getDebugInfoSetting() == DebugInfoSetting::ALL) {
    // Store the current scope, for compiling children of this function in
    // 'eval'.
    node->getSemInfo()->bindingTableScope = bindingTable_.getCurrentScope();
  }

  // Set to false if the parameter list contains binding patterns.
  bool simpleParameterList = true;
  bool hasParameterExpressions = false;
  // All parameter identifiers.
  llvh::SmallVector<IdentifierNode *, 4> paramIds{};
  for (auto &param : params) {
    simpleParameterList &= !llvh::isa<PatternNode>(param);
    hasParameterExpressions |= extractDeclaredIdentsFromID(&param, paramIds);
  }
  curFunctionInfo()->simpleParameterList = simpleParameterList;

  if (!simpleParameterList && directives.useStrictNode) {
    sm_.error(
        directives.useStrictNode->getSourceRange(),
        "'use strict' not allowed inside function with non-simple parameter list");
  }

  // Whether parameters must be unique.
  bool const uniqueParams = !simpleParameterList || curFunctionInfo()->strict ||
      llvh::isa<ArrowFunctionExpressionNode>(node);

  // Do we have a parameter named "arguments".
  bool hasParameterNamedArguments = false;

  // Declare the parameters
  for (IdentifierNode *paramId : paramIds) {
    if (LLVM_UNLIKELY(paramId->_name == kw_.identArguments))
      hasParameterNamedArguments = true;

    validateDeclarationName(Decl::Kind::Parameter, paramId);

    Decl *paramDecl = semCtx_.newDeclInScope(
        paramId->_name, Decl::Kind::Parameter, curScope_);
    semCtx_.setBothDecl(paramId, paramDecl);
    Binding *prevName = bindingTable_.find(paramId->_name);
    if (prevName && prevName->decl->scope == curScope_) {
      // Check for parameter re-declaration.
      if (uniqueParams) {
        sm_.error(
            paramId->getSourceRange(),
            "cannot declare two parameters with the same name '" +
                paramId->_name->str() + "'");
      }

      // Update the name binding to point to the latest declaration.
      prevName->decl = paramDecl;
      prevName->ident = paramId;
    } else {
      // Just add the new parameter.
      bindingTable_.try_emplace(paramId->_name, Binding{paramDecl, paramId});
    }
  }

  /// Declare a pseudo-variable "arguments".
  auto declareArguments = [this]() {
    Decl *argsDecl =
        semCtx_.funcArgumentsDecl(curFunctionInfo(), kw_.identArguments);
    bindingTable_.try_emplace(kw_.identArguments, Binding{argsDecl, nullptr});
  };

  // Do not visit the identifier node, because that would try to resolve it
  // in an incorrect scope!
  // visitESTreeNode(*this, getIdentifier(node), node);

  /// Visits the parameters in the current scope.
  auto visitParams = [this, node]() -> void {
    llvh::SaveAndRestore<bool> oldIsFormalParams{
        functionContext()->isFormalParams, true};

    bool forbidAwaitAsIdentifier = false;
    if (auto *arrow = llvh::dyn_cast<ArrowFunctionExpressionNode>(node)) {
      // ES13.0 15.3 and 15.9
      // ArrowFunction:
      //  ArrowParameters[?Yield, ?Await]
      // AsyncArrowHead :
      //  async [no LineTerminator here] ArrowFormalParameters[~Yield, +Await]
      // 'await' is forbidden as an identifier in arrow params when:
      //  - It's already forbidden in a normal arrow function.
      //  - The function is an async arrow function.
      if (forbidAwaitAsIdentifier_ || arrow->_async)
        forbidAwaitAsIdentifier = true;
    }

    llvh::SaveAndRestore<bool> oldForbidAwait{
        forbidAwaitAsIdentifier_, forbidAwaitAsIdentifier};

    visitESTreeNodeList(*this, getParams(node), node);
  };

  // Visit the parameters before we have hoisted the body declarations.
  // Determine whether we need to declare "arguments" temporarily, while
  // processing the parameter init expressions, in case they refer to it.
  if (!llvh::isa<ESTree::ArrowFunctionExpressionNode>(node) &&
      !hasParameterNamedArguments && hasParameterExpressions) {
    // If we declared the arguments object temporarily, unbind it after visiting
    // the params, so it doesn't clash with potential declarations in the
    // function body.
    ScopeRAII temporaryArgumentsScope{*this};
    declareArguments();
    visitParams();
  } else {
    visitParams();
  }

  llvh::SaveAndRestore<bool> oldForbidAwait{
      forbidAwaitAsIdentifier_, ESTree::isAsync(node)};

  processCollectedDeclarations(node);

  // Promote hoisted functions.
  if (blockBody) {
    if (!curFunctionInfo()->strict) {
      processPromotedFuncDecls(getPromotedScopedFuncDecls(*this, node));
    }
  }

  // Do we need to declare the "arguments" object? Only if we are not an arrow,
  // and don't have a parameter or a variable with that name.
  //
  // IMPORTANT: this is not spec compliant!
  // The spec allows aliasing of "arguments" with "var arguments", but we treat
  // the latter as a new declaration, because of IRGen limitations preventing
  // assignment to "arguments".
  if (!llvh::isa<ESTree::ArrowFunctionExpressionNode>(node) &&
      !hasParameterNamedArguments) {
    Binding *prevArguments = bindingTable_.find(kw_.identArguments);
    if (!prevArguments || prevArguments->decl->scope != curScope_)
      declareArguments();
  }

  // Finally visit the body.
  visitESTreeNode(*this, body, node);

  // Check for local eval and run the unresolver pass in non-strict mode.
  // TODO: enable this when non-strict direct eval is supported.
  LexicalScope *lexScope = curFunctionInfo()->getFunctionScope();
  if (false && lexScope->localEval && !curFunctionInfo()->strict) {
    uint32_t depth = lexScope->depth;
    Unresolver::run(semCtx_, depth, node);
  }

  // Determine whether the function can run the implicit return.
  if (!sm_.getErrorCount()) {
    // CheckImplicitReturn relies on break and continue being properly resolved,
    // and if there's errors during resolution they might not be.
    curFunctionInfo()->mayReachImplicitReturn = mayReachImplicitReturn(node);
  }
}

void SemanticResolver::visitFunctionExpression(
    ESTree::FunctionExpressionNode *node,
    ESTree::Node *body,
    ESTree::NodeList &params,
    ESTree::Node *parent) {
  if (ESTree::IdentifierNode *ident =
          llvh::dyn_cast_or_null<IdentifierNode>(node->_id)) {
    // If there is a name, declare it.
    ScopeRAII scope{*this, node};
    Decl *decl = semCtx_.newDeclInScope(
        ident->_name, Decl::Kind::FunctionExprName, curScope_);
    semCtx_.setDeclarationDecl(ident, decl);
    bindingTable_.try_emplace(ident->_name, Binding{decl, ident});
    visitFunctionLike(node, ident, body, params, parent);
  } else {
    // Otherwise, no extra scope needed, just move on.
    visitFunctionLike(node, nullptr, body, params, parent);
  }
}

Decl *SemanticResolver::resolveIdentifier(
    IdentifierNode *identifier,
    bool inTypeof) {
  Decl *decl = checkIdentifierResolved(identifier);

  // Is this the "arguments" object?
  if (decl && decl->special == Decl::Special::Arguments)
    curFunctionInfo()->usesArguments = true;

  if (LLVM_UNLIKELY(identifier->_name == kw_.identAwait) &&
      forbidAwaitAsIdentifier_) {
    sm_.error(
        identifier->getSourceRange(),
        "await is not a valid identifier name in an async function");
  }

  // Resolved the identifier to a declaration, done.
  if (decl) {
    return decl;
  }

  // Undeclared variables outside `typeof` cause runtime errors in strict mode.
  if (!inTypeof && curFunctionInfo()->strict) {
    llvh::StringRef funcName = functionContext()->getFunctionName()
        ? functionContext()->getFunctionName()->str()
        : "";
    if (functionContext()->isGlobalScope() && funcName.empty())
      funcName = "global";

    llvh::StringRef funcType(
        curFunctionInfo()->arrow ? "arrow function" : "function");
    std::string dispName = !funcName.empty()
        ? (funcType + " \"" + funcName + "\"").str()
        : ("anonymous " + funcType).str();

    sm_.warning(
        Warning::UndefinedVariable,
        identifier->getSourceRange(),
        Twine("the variable \"") + identifier->_name->str() +
            "\" was not declared in " + dispName);
  }

  // Declare an ambient global property.
  decl = semCtx_.newGlobal(
      identifier->_name, Decl::Kind::UndeclaredGlobalProperty);
  semCtx_.setExpressionDecl(identifier, decl);

  bindingTable_.tryEmplaceIntoScope(
      globalScope_, identifier->_name, Binding{decl, identifier});

  return decl;
}

Decl *SemanticResolver::checkIdentifierResolved(
    ESTree::IdentifierNode *identifier) {
  // If identifier already resolved or unresolvable,
  // pick the resolved declaration.
  if (LLVM_UNLIKELY(identifier->isUnresolvable()))
    return nullptr;

  if (sema::Decl *decl = semCtx_.getExpressionDecl(identifier))
    return decl;

  // If we find the binding, assign the associated declaration and return it.
  if (Binding *binding = bindingTable_.find(identifier->_name)) {
    semCtx_.setExpressionDecl(identifier, binding->decl);
    return binding->decl;
  }

  // Failed to resolve.
  return nullptr;
}

void SemanticResolver::processCollectedDeclarations(ESTree::Node *scopeNode) {
  if (const ScopeDecls *declsOpt =
          functionContext()->decls->getScopeDeclsForNode(scopeNode)) {
    processDeclarations(*declsOpt);
  }
}

void SemanticResolver::processDeclarations(const ScopeDecls &decls) {
  for (ESTree::Node *decl : decls) {
#if HERMES_PARSE_FLOW
    if (llvh::isa<ESTree::TypeAliasNode>(decl))
      continue;
#endif
#if HERMES_PARSE_TS
    if (llvh::isa<ESTree::TSTypeAliasDeclarationNode>(decl))
      continue;
#endif
    llvh::SmallVector<ESTree::IdentifierNode *, 4> idents{};
    Decl::Kind kind = extractIdentsFromDecl(decl, idents);

    for (ESTree::IdentifierNode *ident : idents) {
      validateAndDeclareIdentifier(kind, ident);
    }
  }
}

void SemanticResolver::processPromotedFuncDecls(
    llvh::ArrayRef<ESTree::FunctionDeclarationNode *> promotedFuncDecls) {
  // Use GlobalProperty in global scope.
  const Decl::Kind kind = functionContext()->isGlobalScope()
      ? Decl::Kind::GlobalProperty
      : Decl::Kind::Var;
  for (FunctionDeclarationNode *funcDecl : promotedFuncDecls) {
    auto *ident = llvh::cast<IdentifierNode>(funcDecl->_id);
    validateAndDeclareIdentifier(kind, ident);
    functionContext()->promotedFuncDecls.insert(ident->_name);
  }
}

Decl::Kind SemanticResolver::extractIdentsFromDecl(
    ESTree::Node *node,
    llvh::SmallVectorImpl<ESTree::IdentifierNode *> &idents) {
  assert(node && "Node is not optional");
  if (auto *varDecl = llvh::dyn_cast<VariableDeclarationNode>(node)) {
    for (auto &decl : varDecl->_declarations) {
      extractDeclaredIdentsFromID(
          llvh::cast<VariableDeclaratorNode>(&decl)->_id, idents);
    }
    if (varDecl->_kind == kw_.identVar) {
      if (functionContext()->isGlobalScope()) {
        return Decl::Kind::GlobalProperty;
      } else {
        return Decl::Kind::Var;
      }
    }
    if (varDecl->_kind == kw_.identLet) {
      return Decl::Kind::Let;
    } else {
      return Decl::Kind::Const;
    }
  }

  if (auto *funcDecl = llvh::dyn_cast<FunctionDeclarationNode>(node)) {
    extractDeclaredIdentsFromID(funcDecl->_id, idents);
    if (curScope_ == curFunctionInfo()->getFunctionScope()) {
      // It is possible to still have ScopedFunctions in the global function,
      // for example if we have
      // ```
      // let foo;
      // {
      //   function foo() {}
      // }
      // ```
      // then `foo` won't be promoted to functionScope of the global function.
      //
      // However, if `funcDecl` has been promoted to the functionScope of the
      // global function, it should be declared as a GlobalProperty, just like
      // `var` would be.
      //
      // See ScopedFunctionPromoter for rules on when function declarations are
      // promoted out of the child scoped in which they are declared.
      //
      // If the FunctionDeclaration is not at global scope but it is a top-level
      // declaration within a function, it's handled as Var.
      // See ES10.0 13.2.7 for how scoped function declarations are treated
      // specially in top-level.
      return functionContext()->isGlobalScope() ? Decl::Kind::GlobalProperty
                                                : Decl::Kind::Var;
    } else {
      return Decl::Kind::ScopedFunction;
    }
  }

  if (auto *classDecl = llvh::dyn_cast<ClassDeclarationNode>(node)) {
    extractDeclaredIdentsFromID(classDecl->_id, idents);
    return Decl::Kind::Class;
  }

  if (auto *catchClause = llvh::dyn_cast<CatchClauseNode>(node)) {
    extractDeclaredIdentsFromID(catchClause->_param, idents);
    if (auto *id =
            llvh::dyn_cast_or_null<IdentifierNode>(catchClause->_param)) {
      // For compatibility with ES5,
      // we need to treat a single catch variable specially, see:
      // B.3.5 VariableStatements in Catch Blocks
      // https://www.ecma-international.org/ecma-262/10.0/index.html#sec-variablestatements-in-catch-blocks
      return Decl::Kind::ES5Catch;
    } else {
      return Decl::Kind::Let;
    }
  }

  if (auto *importDecl = llvh::dyn_cast<ImportDeclarationNode>(node)) {
    for (auto &spec : importDecl->_specifiers) {
      if (auto *inner = llvh::dyn_cast<ImportSpecifierNode>(&spec)) {
        extractDeclaredIdentsFromID(inner->_local, idents);
      } else if (
          auto *inner = llvh::dyn_cast<ImportDefaultSpecifierNode>(&spec)) {
        extractDeclaredIdentsFromID(inner->_local, idents);
      } else if (
          auto *inner = llvh::dyn_cast<ImportNamespaceSpecifierNode>(&spec)) {
        extractDeclaredIdentsFromID(inner->_local, idents);
      }
    }
    return Decl::Kind::Import;
  }

  sm_.error(node->getSourceRange(), "unsuppported declaration kind");
  return Decl::Kind::Var;
}

bool SemanticResolver::extractDeclaredIdentsFromID(
    ESTree::Node *node,
    llvh::SmallVectorImpl<ESTree::IdentifierNode *> &idents) {
  // The identifier is sometimes optional, in which case it is valid.
  if (!node)
    return false;

  if (auto *idNode = llvh::dyn_cast<IdentifierNode>(node)) {
    idents.push_back(idNode);
    return false;
  }

  if (llvh::isa<EmptyNode>(node))
    return false;

  if (auto *assign = llvh::dyn_cast<AssignmentPatternNode>(node)) {
    extractDeclaredIdentsFromID(assign->_left, idents);
    return true;
  }

  bool containsExpr = false;

  if (auto *array = llvh::dyn_cast<ArrayPatternNode>(node)) {
    for (auto &elem : array->_elements)
      containsExpr |= extractDeclaredIdentsFromID(&elem, idents);
    return containsExpr;
  }

  if (auto *restElem = llvh::dyn_cast<RestElementNode>(node)) {
    return extractDeclaredIdentsFromID(restElem->_argument, idents);
  }

  if (auto *obj = llvh::dyn_cast<ObjectPatternNode>(node)) {
    for (auto &propNode : obj->_properties) {
      if (auto *prop = llvh::dyn_cast<PropertyNode>(&propNode)) {
        containsExpr |= extractDeclaredIdentsFromID(prop->_value, idents);
      } else {
        auto *rest = cast<RestElementNode>(&propNode);
        containsExpr |= extractDeclaredIdentsFromID(rest->_argument, idents);
      }
    }
    return containsExpr;
  }

  if (auto *param = llvh::dyn_cast<ComponentParameterNode>(node)) {
    return extractDeclaredIdentsFromID(param->_name, idents);
  }

  sm_.error(node->getSourceRange(), "invalid destructuring target");
  return false;
}

void SemanticResolver::validateAndDeclareIdentifier(
    Decl::Kind kind,
    ESTree::IdentifierNode *ident) {
  if (!validateDeclarationName(kind, ident))
    return;

  Binding prevName = bindingTable_.lookup(ident->_name);

  // IMPORTANT: this is not spec compliant!
  // For now, treat "var" declarations of "arguments" simply as a new variable.
  // instead of as an alias for the Arguments object.
  // It is simpler and makes a difference only in the following obscure case:
  // - non-strict mode
  // - "var arguments" without an initializer.
  // I am willing to live with this sacrifice.
  // Aliasing of "arguments" becomes especially iffy when type annotations are
  // added.
  if (false) {
    // Redeclaration of `arguments` in non-strict mode is allowed at the
    // function level, so we don't need to declare a new variable.
    if (!curFunctionInfo()->strict && ident->_name == kw_.identArguments &&
        kind == Decl::Kind::Var) {
      return;
    }
  }

  // Ignore declarations in enclosing functions.
  if (prevName.isValid() && !declInCurFunction(prevName.decl)) {
    prevName.invalidate();
  }

  Decl *decl = nullptr;

  // Whether to reuse the decl (above) for a new binding when it's not nullptr.
  bool reuseDeclForNewBinding = false;

  // Handle re-declarations, ignoring ambient properties.
  if (prevName.isValid() &&
      prevName.decl->kind != Decl::Kind::UndeclaredGlobalProperty) {
    const Decl::Kind prevKind = prevName.decl->kind;
    const bool sameScope = prevName.decl->scope == curScope_;

    // Check whether the redeclaration is invalid.
    // Note that since "var" declarations have been hoisted to the function
    // scope, we cannot catch cases where "var" follows something declared in a
    // surrounding lexical scope.
    // See visit(VariableDeclarationNode *) for when those are handled.
    //
    // The two rules in the spec ES10.0 (e.g. B.3.3.4) are:
    // * LexicallyDeclaredNames (in the same scope) can't conflict.
    // * LexicallyDeclaredNames can't conflict with VarDeclarationNames in their
    //   own scope or any of their child scopes (recursively).
    //
    // Parameter names must also not conflict with lexically scoped names
    // in the top-level of the function (ES10.0 14.1.2):
    // * It is a Syntax Error if any element of the BoundNames of
    //   FormalParameters also occurs in the LexicallyDeclaredNames of
    //   FunctionBody.
    //
    // Case by case explanations for our representation:
    //
    // ES5Catch, var
    //          -> valid, special case ES10 B.3.5, but we can't catch it here.
    //             See visit(VariableDeclarationNode *)
    // var, var
    //          -> always valid
    // scopedFunction, var
    //          -> can't happen because var is at top-level only
    // var, scopedFunction
    //          -> valid because scopedFunction is not at top-level
    // scopedFunction, scopedFunction
    //          -> strict mode: valid if not in the same scope
    //             loose mode: always valid
    //             See ES10.0 13.2.7
    //             scoped function declarations are treated specially
    //             if they're at the top-level of the function/script/module.
    //             'var' case is handled in visit(VariableDeclarationNode *).
    // let, var
    //          -> always invalid
    // let, scopedFunction
    //          -> invalid if same scope
    // var|scopedFunction|let, let
    //          -> invalid if the same scope
    // parameter, let
    //          -> invalid if let is top-level (same scope as parameter)

    assert(
        !(prevKind == Decl::Kind::ScopedFunction && kind == Decl::Kind::Var) &&
        "invalid state, scopedFunctions are not at top-level");

    if ((Decl::isKindLetLike(prevKind) && Decl::isKindVarLike(kind)) ||
        (Decl::isKindVarLike(prevKind) && Decl::isKindLetLike(kind) &&
         sameScope) ||
        (Decl::isKindLetLike(prevKind) && Decl::isKindLetLike(kind) &&
         sameScope &&
         // ES10.0 B.3.3.4
         // Annex B exception: non-strict mode ScopedFunctions are OK.
         !(!curFunctionInfo()->strict &&
           prevKind == Decl::Kind::ScopedFunction &&
           kind == Decl::Kind::ScopedFunction)) ||
        // Parameters are in the same scope as the top-level of the function.
        (prevKind == Decl::Kind::Parameter && Decl::isKindLetLike(kind) &&
         sameScope)) {
      sm_.error(
          ident->getSourceRange(),
          llvh::Twine("Identifier '") + ident->_name->str() +
              "' is already declared");
      if (prevName.ident)
        sm_.note(prevName.ident->getSourceRange(), "previous declaration");
      return;
    }

    // When to create a new declaration?
    //
    // Var, Var -> use prev
    if (Decl::isKindVarLike(prevKind) && Decl::isKindVarLike(kind)) {
      decl = prevName.decl;
    }
    // Var, ScopedFunc -> if non-param non-strict or same scope, then use prev,
    //                    else declare new
    else if (
        Decl::isKindVarLike(prevKind) &&
        Decl::isKindVarLikeOrScopedFunction(kind)) {
      if (sameScope) {
        decl = prevName.decl;
      } else if (functionContext()->promotedFuncDecls.count(ident->_name)) {
        // We've already promoted this function, so add a new binding
        // and point it to the original Decl.
        reuseDeclForNewBinding = true;
        decl = prevName.decl;
      } else {
        decl = nullptr;
      }
    }
    // ScopedFunc, ScopedFunc same scope -> error
    // ScopedFunc, ScopedFunc new scope -> declare new
    else if (
        prevKind == Decl::Kind::ScopedFunction &&
        kind == Decl::Kind::ScopedFunction) {
      decl = nullptr;
    }
  }

  // Create new decl.
  if (!decl) {
    if (Decl::isKindGlobal(kind))
      decl = semCtx_.newGlobal(ident->_name, kind);
    else
      decl = semCtx_.newDeclInScope(ident->_name, kind, curScope_);
    bindingTable_.try_emplace(ident->_name, Binding{decl, ident});
  } else if (reuseDeclForNewBinding) {
    bindingTable_.try_emplace(ident->_name, Binding{decl, ident});
  }

  semCtx_.setDeclarationDecl(ident, decl);
}

bool SemanticResolver::validateDeclarationName(
    Decl::Kind declKind,
    const ESTree::IdentifierNode *idNode) const {
  if (curFunctionInfo()->strict) {
    // - 'arguments' cannot be redeclared in strict mode.
    // - 'eval' cannot be redeclared in strict mode.
    if (idNode->_name == kw_.identArguments || idNode->_name == kw_.identEval) {
      sm_.error(
          idNode->getSourceRange(),
          "cannot declare '" + cast<IdentifierNode>(idNode)->_name->str() +
              "' in strict mode");
      return false;
    }

    // Parameter cannot be named "let".
    if (declKind == Decl::Kind::Parameter && idNode->_name == kw_.identLet) {
      sm_.error(
          idNode->getSourceRange(),
          "invalid parameter name 'let' in strict mode");
      return false;
    }
  }

  if ((declKind == Decl::Kind::Let || declKind == Decl::Kind::Const) &&
      idNode->_name == kw_.identLet) {
    // ES9.0 13.3.1.1
    // LexicalDeclaration : LetOrConst BindingList
    // It is a Syntax Error if the BoundNames of BindingList
    // contains "let".
    sm_.error(
        idNode->getSourceRange(),
        "'let' is disallowed as a lexically bound name");
    return false;
  }

  return true;
}

void SemanticResolver::validateAssignmentTarget(Node *node) {
  if (llvh::isa<EmptyNode>(node))
    return;

  if (auto *assign = llvh::dyn_cast<AssignmentPatternNode>(node)) {
    return validateAssignmentTarget(assign->_left);
  }

  if (auto *prop = llvh::dyn_cast<PropertyNode>(node)) {
    return validateAssignmentTarget(prop->_value);
  }

  if (auto *arr = llvh::dyn_cast<ArrayPatternNode>(node)) {
    for (auto &elem : arr->_elements) {
      validateAssignmentTarget(&elem);
    }
    return;
  }

  if (auto *obj = llvh::dyn_cast<ObjectPatternNode>(node)) {
    for (auto &propNode : obj->_properties) {
      validateAssignmentTarget(&propNode);
    }
    return;
  }

  if (auto *rest = llvh::dyn_cast<RestElementNode>(node)) {
    return validateAssignmentTarget(rest->_argument);
  }

  if (!isLValue(node))
    sm_.error(node->getSourceRange(), "invalid assignment left-hand side");
}

bool SemanticResolver::isLValue(ESTree::Node *node) {
  if (llvh::isa<MemberExpressionNode>(node))
    return true;

  if (auto *id = llvh::dyn_cast<IdentifierNode>(node)) {
    Decl *decl = semCtx_.getExpressionDecl(id);
    assert(decl && "Identifier must be resolved");

    // Unless we are running under compliance tests, report an error on
    // reassignment to const.
    if (decl->kind == Decl::Kind::Const)
      if (!astContext_.getCodeGenerationSettings().test262)
        return false;

    // In strict mode, assigning to the identifier "eval" or "arguments"
    // is invalid, regardless of what they are bound to in surrounding scopes.
    // This is invalid:
    //     let eval;
    //     function foo() {
    //       "use strict";
    //       eval = 0; // ERROR!
    //     }
    if (curFunctionInfo()->strict) {
      if (LLVM_UNLIKELY(
              id->_name == kw_.identArguments || id->_name == kw_.identEval)) {
        return false;
      }
    } else {
      // IMPORTANT: this is not spec compliant!
      // In loose mode it should be possible to assign to "arguments".
      // But that is a corner case that is difficult to handle, so for now
      // we are prohibiting it.
      if (decl->special == Decl::Special::Arguments)
        return false;
    }

    return true;
  }

  return false;
}

void SemanticResolver::recursionDepthExceeded(ESTree::Node *n) {
  sm_.error(
      n->getEndLoc(), "Too many nested expressions/statements/declarations");
}

auto SemanticResolver::scanDirectives(ESTree::NodeList &body) const
    -> FoundDirectives {
  FoundDirectives directives{};
  for (auto &node : body) {
    auto *exprSt = llvh::dyn_cast<ESTree::ExpressionStatementNode>(&node);
    if (!exprSt || !exprSt->_directive)
      break;

    auto *directive = exprSt->_directive;

    if (directive == kw_.identUseStrict) {
      if (!directives.useStrictNode)
        directives.useStrictNode = exprSt;
    } else if (directive == kw_.identShowSource) {
      if (SourceVisibility::ShowSource > directives.sourceVisibility)
        directives.sourceVisibility = SourceVisibility::ShowSource;
    } else if (directive == kw_.identHideSource) {
      if (SourceVisibility::HideSource > directives.sourceVisibility)
        directives.sourceVisibility = SourceVisibility::HideSource;
    } else if (directive == kw_.identSensitive) {
      if (SourceVisibility::Sensitive > directives.sourceVisibility)
        directives.sourceVisibility = SourceVisibility::Sensitive;
    }

    // Shouldn't have both 'inline' and 'noinline'.  But this shouldn't
    // prevent compilation.  So, give a warning, and take the most recent
    // directive.
    if (directive == kw_.identInline) {
      if (directives.noInline) {
        sm_.warning(
            exprSt->getSourceRange(),
            "Should not declare both 'inline' and 'noinline'.");
        directives.noInline = false;
      }
      directives.alwaysInline = true;
    }
    if (directive == kw_.identNoInline) {
      if (directives.alwaysInline) {
        sm_.warning(
            exprSt->getSourceRange(),
            "Should not declare both 'inline' and 'noinline'.");
        directives.alwaysInline = false;
      }
      directives.noInline = true;
    }
  }
  return directives;
}

/* static */ void SemanticResolver::registerLocalEval(LexicalScope *scope) {
  for (LexicalScope *curScope = scope; curScope;
       curScope = curScope->parentScope) {
    curScope->localEval = true;

    // This can also set a `canRename` flag on the identifier,
    // which we haven't implemented yet.
  }
}

/// Declare the list of ambient decls that was passed to the constructor.
void SemanticResolver::processAmbientDecls() {
  assert(
      globalScope_ &&
      "global scope must be created when declaring ambient globals");

  /// This visitor structs collects declarations within a single closure without
  /// descending into child closures.
  struct DeclHoisting {
    /// The list of collected identifiers (variables and functions).
    llvh::SmallVector<ESTree::VariableDeclaratorNode *, 8> decls{};

    /// A list of functions that need to be hoisted and materialized before we
    /// can generate the rest of the function.
    llvh::SmallVector<ESTree::FunctionDeclarationNode *, 8> closures;

    explicit DeclHoisting() = default;
    ~DeclHoisting() = default;

    /// Extract the variable name from the nodes that can define new variables.
    /// The nodes that can define a new variable in the scope are:
    /// VariableDeclarator and FunctionDeclaration>
    void collectDecls(ESTree::Node *V) {
      if (auto VD = llvh::dyn_cast<ESTree::VariableDeclaratorNode>(V)) {
        return decls.push_back(VD);
      }

      if (auto FD = llvh::dyn_cast<ESTree::FunctionDeclarationNode>(V)) {
        return closures.push_back(FD);
      }
    }

    bool shouldVisit(ESTree::Node *V) {
      // Collect declared names, even if we don't descend into children nodes.
      collectDecls(V);

      // Do not descend to child closures because the variables they define are
      // not exposed to the outside function.
      if (llvh::isa<ESTree::FunctionDeclarationNode>(V) ||
          llvh::isa<ESTree::FunctionExpressionNode>(V) ||
          llvh::isa<ESTree::ArrowFunctionExpressionNode>(V))
        return false;
      return true;
    }

    void enter(ESTree::Node *V) {}
    void leave(ESTree::Node *V) {}
  };

  auto declareAmbientGlobal = [this](ESTree::Node *identNode) {
    UniqueString *name = llvh::cast<ESTree::IdentifierNode>(identNode)->_name;
    // If we find the binding, do nothing.
    if (!bindingTable_.count(name)) {
      Decl *decl =
          semCtx_.newGlobal(name, Decl::Kind::UndeclaredGlobalProperty);
      bindingTable_.tryEmplaceIntoScope(
          globalScope_, name, Binding{decl, nullptr});
    }
  };

  for (auto *programNode : ambientDecls_) {
    DeclHoisting DH;
    programNode->visit(DH);
    // Create variable declarations for each of the hoisted variables.
    for (auto *vd : DH.decls)
      declareAmbientGlobal(vd->_id);
    for (auto *fd : DH.closures)
      declareAmbientGlobal(fd->_id);
  }
}

SemanticResolver::ScopeRAII::ScopeRAII(
    SemanticResolver &resolver,
    ESTree::ScopeDecorationBase *scopeNode)
    : resolver_(resolver),
      oldScope_(resolver_.curScope_),
      bindingScope_(resolver_.bindingTable_) {
  // Create a new scope.
  LexicalScope *scope = resolver.semCtx_.newScope(
      resolver_.curFunctionInfo(), resolver_.curScope_);
  resolver.curScope_ = scope;
  // Optionally associate the scope with the node.
  if (scopeNode)
    scopeNode->setScope(scope);
}
SemanticResolver::ScopeRAII::~ScopeRAII() {
  resolver_.curScope_ = oldScope_;
}

//===----------------------------------------------------------------------===//
// FunctionContext

FunctionContext::FunctionContext(
    SemanticResolver &resolver,
    FunctionInfo *globalSemInfo,
    ExistingGlobalScopeTag)
    : resolver_(resolver),
      prevContext_(resolver.curFunctionContext_),
      semInfo(globalSemInfo),
      node(nullptr) {
  resolver.curFunctionContext_ = this;
}

FunctionContext::FunctionContext(
    SemanticResolver &resolver,
    ESTree::FunctionLikeNode *node,
    FunctionInfo *parentSemInfo,
    bool strict,
    CustomDirectives customDirectives)
    : resolver_(resolver),
      prevContext_(resolver.curFunctionContext_),
      semInfo(resolver.semCtx_.newFunction(
          SemContext::nodeIsArrow(node),
          parentSemInfo,
          resolver.curScope_,
          strict,
          customDirectives)),
      node(node),
      decls(DeclCollector::run(
          node,
          resolver.keywords(),
          resolver.recursionDepth_,
          [&resolver](ESTree::Node *n) {
            // Inform the resolver that we have gone too deep.
            resolver.recursionDepth_ = 0;
            resolver.recursionDepthExceeded(n);
          })) {
  resolver.curFunctionContext_ = this;
  node->setSemInfo(this->semInfo);
}

FunctionContext::FunctionContext(
    SemanticResolver &resolver,
    FunctionInfo *newFunctionInfo)
    : resolver_(resolver),
      prevContext_(resolver.curFunctionContext_),
      semInfo(newFunctionInfo),
      node(nullptr) {
  resolver.curFunctionContext_ = this;
}

FunctionContext::FunctionContext(
    SemanticResolver &resolver,
    ESTree::FunctionLikeNode *node,
    FunctionInfo *semInfoLazy,
    LazyTag)
    : resolver_(resolver),
      prevContext_(nullptr),
      semInfo(semInfoLazy),
      node(node),
      decls(DeclCollector::run(
          node,
          resolver.keywords(),
          resolver.recursionDepth_,
          [&resolver](ESTree::Node *n) {
            // Inform the resolver that we have gone too deep.
            resolver.recursionDepth_ = 0;
            resolver.recursionDepthExceeded(n);
          })) {
  // Use the same semInfo as the lazy one.
  node->setSemInfo(semInfoLazy);
  resolver.curFunctionContext_ = this;
}

FunctionContext::~FunctionContext() {
  // If requested, save the collected declarations.
  if (resolver_.saveDecls_ && decls)
    resolver_.saveDecls_->try_emplace(node, std::move(decls));

  assert(
      resolver_.curFunctionInfo() == semInfo &&
      "FunctionContext out of sync with SemContext");
  resolver_.curFunctionContext_ = prevContext_;
}

UniqueString *FunctionContext::getFunctionName() const {
  if (node) {
    if (auto *idNode =
            llvh::dyn_cast_or_null<IdentifierNode>(getIdentifier(node)))
      return idNode->_name;
  }
  return nullptr;
}

const FunctionContext *FunctionContext::nearestNonArrow() const {
  const FunctionContext *cur = this;
  while (cur->semInfo->arrow) {
    cur = cur->prevContext_;
  }
  assert(cur && "All contexts should have a non-arrow ancestor.");
  return cur;
}

ClassContext::ClassContext(SemanticResolver &resolver, ClassLikeNode *classNode)
    : resolver_(resolver),
      prevContext_(resolver.curClassContext_),
      classNode_(classNode) {
  resolver.curClassContext_ = this;
};

void ClassContext::createImplicitConstructorFunctionInfo() {
  // Do nothing if the class has an explicit constructor.
  if (hasConstructor) {
    return;
  }
  auto *classDecoration = getDecoration<ClassLikeDecoration>(classNode_);
  assert(classDecoration->implicitCtorFunctionInfo == nullptr);
  FunctionInfo *implicitCtor = resolver_.semCtx_.newFunction(
      FuncIsArrow::No,
      resolver_.curFunctionInfo(),
      resolver_.curScope_,
      /*strict*/ true,
      CustomDirectives{});
  // This is callled for the side effect of associating the new scope with
  // implicitCtor.  We don't need the value now, but we will later.
  (void)resolver_.semCtx_.newScope(implicitCtor, resolver_.curScope_);
  classDecoration->implicitCtorFunctionInfo = implicitCtor;
}

FunctionInfo *ClassContext::getOrCreateFieldInitFunctionInfo() {
  auto *classDecoration = getDecoration<ClassLikeDecoration>(classNode_);
  if (classDecoration->fieldInitFunctionInfo == nullptr) {
    FunctionInfo *fieldInitFunc = resolver_.semCtx_.newFunction(
        FuncIsArrow::No,
        resolver_.curFunctionInfo(),
        resolver_.curScope_,
        /*strict*/ true,
        CustomDirectives{});
    // This is callled for the side effect of associating the new scope with
    // fieldInitFunc.  We don't need the value now, but we will later.
    (void)resolver_.semCtx_.newScope(fieldInitFunc, resolver_.curScope_);
    classDecoration->fieldInitFunctionInfo = fieldInitFunc;
  }
  return classDecoration->fieldInitFunctionInfo;
}

ClassContext::~ClassContext() {
  resolver_.curClassContext_ = prevContext_;
}

//===----------------------------------------------------------------------===//
// Unresolver

/* static */ void
Unresolver::run(SemContext &semCtx, uint32_t depth, ESTree::Node *root) {
  Unresolver unresolver{semCtx, depth};
  visitESTreeNodeNoReplace(unresolver, root);
}

void Unresolver::visit(ESTree::IdentifierNode *node) {
  if (node->isUnresolvable()) {
    return;
  }

  if (Decl *decl = semCtx_.getExpressionDecl(node)) {
    LexicalScope *scope = decl->scope;

    // The depth of this identifier's declaration is less than the `eval`/`with`
    // declaration that could shadow it, so we must declare this identifier as
    // unresolvable.
    if (scope->depth < depth_) {
      semCtx_.setExpressionDecl(node, nullptr);
      node->setUnresolvable();
    }
  }

  visitESTreeChildren(*this, node);
}

} // namespace sema
} // namespace hermes
