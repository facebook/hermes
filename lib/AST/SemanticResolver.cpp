/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SemanticResolver.h"

#include "ScopedFunctionPromoter.h"
#include "hermes/AST/SemContext.h"
#include "hermes/Support/RegExpSerialization.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/Support/SaveAndRestore.h"

using namespace hermes::ESTree;

namespace hermes {
namespace sema {

SemanticResolver::SemanticResolver(
    Context &astContext,
    sema::SemContext &semCtx,
    bool compile)
    : astContext_(astContext),
      sm_(astContext.getSourceErrorManager()),
      bufferMessages_{&sm_},
      semCtx_(semCtx),
      initialErrorCount_(sm_.getErrorCount()),
      kw_{astContext},
      compile_(compile) {}

bool SemanticResolver::run(ESTree::Node *rootNode) {
  visitESTreeNode(*this, rootNode);
  return sm_.getErrorCount() == initialErrorCount_;
}

void SemanticResolver::visit(ESTree::ProgramNode *node) {
  FunctionContext newFuncCtx{*this, node, nullptr, astContext_.isStrictMode()};
  if (findUseStrict(node->_body)) {
    curFunctionInfo()->strict = true;
  }
  node->strictness = makeStrictness(curFunctionInfo()->strict);

  {
    ScopeRAII programScope{*this, node};
    globalScope_ = &programScope.getBindingScope();

    processCollectedDeclarations(node);
    visitESTreeChildren(*this, node);
  }
}

void SemanticResolver::visit(ESTree::FunctionDeclarationNode *funcDecl) {
  curScope_->hoistedFunctions.push_back(funcDecl);
  visitFunctionLike(funcDecl, funcDecl->_body, funcDecl->_params);
}
void SemanticResolver::visit(ESTree::FunctionExpressionNode *funcExpr) {
  visitFunctionExpression(funcExpr, funcExpr->_body, funcExpr->_params);
}
void SemanticResolver::visit(ESTree::ArrowFunctionExpressionNode *arrowFunc) {
  visitFunctionLike(arrowFunc, arrowFunc->_body, arrowFunc->_params);
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

  if (auto *mem = llvh::dyn_cast<MemberExpressionNode>(parent)) {
    if (!mem->_computed && mem->_property == identifier) {
      // expr.identifier
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

  resolveIdentifier(identifier, false);
}

void SemanticResolver::visit(ESTree::AssignmentExpressionNode *assignment) {
  validateAssignmentTarget(assignment->_left);
  visitESTreeChildren(*this, assignment);
}

void SemanticResolver::visit(ESTree::UpdateExpressionNode *node) {
  if (!isLValue(node->_argument)) {
    sm_.error(
        node->_argument->getSourceRange(),
        "invalid operand in update operation");
  }
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::UnaryExpressionNode *node) {
  // Check for unqualified delete in strict mode.
  if (node->_operator == kw_.identDelete) {
    if (curFunctionInfo()->strict &&
        llvh::isa<IdentifierNode>(node->_argument)) {
      sm_.error(
          node->getSourceRange(),
          "'delete' of a variable is not allowed in strict mode");
    }
  }
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::BlockStatementNode *node) {
  // Some nodes with attached BlockStatement have already dealt with the scope.
  if (llvh::isa<FunctionDeclarationNode>(node) ||
      llvh::isa<FunctionExpressionNode>(node) ||
      llvh::isa<ArrowFunctionExpressionNode>(node) ||
      llvh::isa<CatchClauseNode>(node)) {
    return visitESTreeChildren(*this, node);
  }

  if (hermes::OptValue<llvh::ArrayRef<ESTree::Node *>> declsOpt =
          functionContext()->decls.getScopeDeclsForNode(node)) {
    // Only create a lexical scope if there are declarations in it.
    ScopeRAII blockScope{*this, node};
    processDeclarations(*declsOpt);
    visitESTreeChildren(*this, node);
  } else {
    visitESTreeChildren(*this, node);
  }
}

void SemanticResolver::visit(ESTree::SwitchStatementNode *node) {
  // Visit the discriminant before creating a new scope.
  visitESTreeNode(*this, node->_discriminant, node);

  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<StatementNode *> saveSwitch(currentLoopOrSwitch_, node);

  {
    ScopeRAII nameScope{*this, node};
    if (hermes::OptValue<llvh::ArrayRef<ESTree::Node *>> declsOpt =
            functionContext()->decls.getScopeDeclsForNode(node)) {
      // Only create a lexical scope if there are declarations in it.
      processDeclarations(*declsOpt);
    }

    for (ESTree::Node &c : node->_cases)
      visitESTreeNode(*this, &c, node);
  }
}

void SemanticResolver::visitForInOf(
    ESTree::LoopStatementNode *node,
    ESTree::ScopeDecorationBase *scopeDeco,
    ESTree::Node *left) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(currentLoop_, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(currentLoopOrSwitch_, node);

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

  if (hermes::OptValue<llvh::ArrayRef<ESTree::Node *>> declsOpt =
          functionContext()->decls.getScopeDeclsForNode(node)) {
    // Only create a lexical scope if there are declarations in it.
    ScopeRAII nameScope{*this, scopeDeco};
    processDeclarations(*declsOpt);
    visitESTreeChildren(*this, node);
  } else {
    visitESTreeChildren(*this, node);
  }
}

void SemanticResolver::visit(ESTree::ForStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(currentLoop_, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(currentLoopOrSwitch_, node);

  if (hermes::OptValue<llvh::ArrayRef<ESTree::Node *>> declsOpt =
          functionContext()->decls.getScopeDeclsForNode(node)) {
    // Only create a lexical scope if there are declarations in it.
    ScopeRAII nameScope{*this, node};
    processDeclarations(*declsOpt);
    visitESTreeChildren(*this, node);
  } else {
    visitESTreeChildren(*this, node);
  }
}

void SemanticResolver::visit(ESTree::DoWhileStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(currentLoop_, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(currentLoopOrSwitch_, node);

  visitESTreeChildren(*this, node);
}
void SemanticResolver::visit(ESTree::WhileStatementNode *node) {
  node->setLabelIndex(curFunctionInfo()->allocateLabel());

  llvh::SaveAndRestore<LoopStatementNode *> saveLoop(currentLoop_, node);
  llvh::SaveAndRestore<StatementNode *> saveSwitch(currentLoopOrSwitch_, node);

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
  const auto &deleter = llvh::make_scope_exit([=]() {
    if (insertRes.second)
      functionContext()->labelMap.erase(id->_name);
  });
  (void)deleter;

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::BreakStatementNode *node) {
  if (node->_label) {
    const NodeLabel &name = llvh::cast<IdentifierNode>(node->_label)->_name;
    auto it = functionContext()->labelMap.find(name);
    if (it == functionContext()->labelMap.end()) {
      sm_.error(
          node->getSourceRange(),
          llvh::Twine("label '") + name->str() + "' is not defined");
    }
  } else {
    if (!currentLoopOrSwitch_) {
      sm_.error(node->getSourceRange(), "'break' not within a loop or switch");
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::ContinueStatementNode *node) {
  if (node->_label) {
    const NodeLabel &name = llvh::cast<IdentifierNode>(node->_label)->_name;
    auto it = functionContext()->labelMap.find(name);
    if (it == functionContext()->labelMap.end()) {
      sm_.error(
          node->getSourceRange(),
          llvh::Twine("label '") + name->str() + "' is not defined");
    }
    if (!llvh::isa<LabeledStatementNode>(it->second.targetStatement)) {
      sm_.error(
          node->getSourceRange(),
          llvh::Twine("'continue' label '") + name->str() +
              "' is not a loop label");
    }
  } else {
    if (!currentLoopOrSwitch_) {
      sm_.error(
          node->getSourceRange(), "'continue' not within a loop or switch");
    }
  }

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::WithStatementNode *node) {
  visitESTreeChildren(*this, node);
  // FIXME: Run an unresolver pass.
}

void SemanticResolver::visit(ESTree::CatchClauseNode *node) {
  ScopeRAII scope{*this, node};

  if (auto *id = llvh::dyn_cast<IdentifierNode>(node->_param)) {
    // For compatibility with ES5,
    // we need to treat a single catch variable specially, see:
    // B.3.5 VariableStatements in Catch Blocks
    // https://www.ecma-international.org/ecma-262/10.0/index.html#sec-variablestatements-in-catch-blocks
    validateAndDeclareIdentifier(Decl::Kind::ES5Catch, id);
  } else {
    llvh::SmallVector<IdentifierNode *, 4> idents{};
    extractDeclaredIdentsFromID(node->_param, idents);
    for (IdentifierNode *id : idents) {
      validateAndDeclareIdentifier(Decl::Kind::Let, id);
    }
  }

  // Process body's declarations, skip visiting it, visit its children.
  processCollectedDeclarations(node->_body);
  visitESTreeChildren(*this, node->_body);
}

void SemanticResolver::visit(RegExpLiteralNode *regexp) {
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

void SemanticResolver::visit(ESTree::MetaPropertyNode *node) {
  auto *meta = llvh::cast<IdentifierNode>(node->_meta);
  auto *property = llvh::cast<IdentifierNode>(node->_property);

  if (meta->_name == kw_.identNew && property->_name == kw_.identTarget &&
      functionContext()->isGlobalScope()) {
    // ES9.0 15.1.1:
    // It is a Syntax Error if StatementList Contains NewTarget unless the
    // source code containing NewTarget is eval code that is being processed
    // by a direct eval.
    // Hermes does not support local eval, so we assume that this is not
    // inside a local eval call.
    sm_.error(node->getSourceRange(), "'new.target' not in a function");
  }

  sm_.error(
      node->getSourceRange(),
      llvh::Twine("invalid meta property ") + meta->_name->str() + "." +
          property->_name->str());
}

void SemanticResolver::visit(ESTree::ImportDeclarationNode *node) {
  visitESTreeChildren(*this, node);
  // TODO: Multi-file dependency resolution.
}
void SemanticResolver::visit(ESTree::ClassDeclarationNode *node) {
  // Classes must be in strict mode.
  llvh::SaveAndRestore<bool> oldStrict{curFunctionInfo()->strict, true};
  visitESTreeChildren(*this, node);
}
void SemanticResolver::visit(ESTree::ClassExpressionNode *node) {
  // Classes must be in strict mode.
  llvh::SaveAndRestore<bool> oldStrict{curFunctionInfo()->strict, true};
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(PrivateNameNode *node) {
  if (compile_)
    sm_.error(node->getSourceRange(), "private properties are not supported");
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ClassPrivatePropertyNode *node) {
  if (compile_)
    sm_.error(node->getSourceRange(), "private properties are not supported");
  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::CallExpressionNode *node) {
  // FIXME: Check for local 'eval'.
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

  visitESTreeChildren(*this, node);
}

void SemanticResolver::visit(ESTree::AwaitExpressionNode *awaitExpr) {
  if (functionContext()->isGlobalScope() ||
      (functionContext()->node && !ESTree::isAsync(functionContext()->node))) {
    sm_.error(awaitExpr->getSourceRange(), "'await' not in an async function");
  }

  visitESTreeChildren(*this, awaitExpr);
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
#endif

void SemanticResolver::visitFunctionLike(
    ESTree::FunctionLikeNode *node,
    ESTree::Node *body,
    ESTree::NodeList &params) {
  FunctionContext newFuncCtx{*this, node, nullptr, curFunctionInfo()->strict};

  llvh::SaveAndRestore<bool> oldIsFormalParamsFn{isFormalParams_, false};

  ESTree::Node *useStrictNode = nullptr;

  // Note that body might be empty (for lazy functions)
  // or an expression (for arrow functions).
  auto *blockBody = llvh::dyn_cast<BlockStatementNode>(body);
  if (blockBody) {
    useStrictNode = findUseStrict(blockBody->_body);
  }

  // Set the strictness if necessary.
  if (useStrictNode) {
    curFunctionInfo()->strict = true;
  }
  node->strictness = makeStrictness(curFunctionInfo()->strict);

  // Create the function body scope.
  // Note that we are associating the new scope with the body.
  // We are doing this because function expressions need an extra scope
  // for the name, and we associate that with the function expression itself.
  ScopeRAII scope{*this, blockBody};

  // Set to false if the parameter list contains binding patterns.
  bool simpleParameterList = true;
  // All parameter identifiers.
  llvh::SmallVector<IdentifierNode *, 4> paramIds{};
  for (auto &param : params) {
    simpleParameterList &= !llvh::isa<PatternNode>(param);
    extractDeclaredIdentsFromID(&param, paramIds);
  }

  if (!simpleParameterList && useStrictNode) {
    sm_.error(
        useStrictNode->getSourceRange(),
        "'use strict' not allowed inside function with non-simple parameter list");
  }

  // Whether parameters must be unique.
  bool const uniqueParams = !simpleParameterList || curFunctionInfo()->strict ||
      llvh::isa<ArrowFunctionExpressionNode>(node);

  // Declare the parameters
  for (IdentifierNode *paramId : paramIds) {
    validateDeclarationName(Decl::Kind::Parameter, paramId);

    Decl *paramDecl =
        semCtx_.newDecl(paramId->_name, Decl::Kind::Parameter, curScope_);
    paramId->setDecl(paramDecl);
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
      bindingTable_.insert(paramId->_name, Binding{paramDecl, paramId});
    }
  }

  // Do not visit the identifier node, because that would try to resolve it
  // in an incorrect scope!
  // visitESTreeNode(*this, getIdentifier(node), node);

  // Visit the parameters before we have hoisted the body declarations.
  {
    llvh::SaveAndRestore<bool> oldIsFormalParams{isFormalParams_, true};
    for (auto &param : getParams(node))
      visitESTreeNode(*this, &param, node);
  }

  // Promote hoisted functions.
  if (blockBody) {
    if (!curFunctionInfo()->strict) {
      promoteScopedFuncDecls(*this, node);
    }
  }
  processCollectedDeclarations(node);

  // Finally visit the body.
  visitESTreeNode(*this, body, node);

  // FIXME: Check for local eval and run the unresolver pass in non-strict mode.
}

void SemanticResolver::visitFunctionExpression(
    ESTree::FunctionExpressionNode *node,
    ESTree::Node *body,
    ESTree::NodeList &params) {
  if (ESTree::IdentifierNode *ident =
          llvh::dyn_cast_or_null<IdentifierNode>(node->_id)) {
    // If there is a name, declare it.
    ScopeRAII scope{*this, node};
    if (validateDeclarationName(Decl::Kind::FunctionExprName, ident)) {
      Decl *decl = semCtx_.newDecl(
          ident->_name, Decl::Kind::FunctionExprName, curScope_);
      ident->setDecl(decl);
      bindingTable_.insert(ident->_name, Binding{decl, ident});
    }
    visitFunctionLike(node, body, params);
  } else {
    // Otherwise, no extra scope needed, just move on.
    visitFunctionLike(node, body, params);
  }
}

void SemanticResolver::resolveIdentifier(
    IdentifierNode *identifier,
    bool inTypeof) {
  Decl *decl = checkIdentifierResolved(identifier);

  // Is this "arguments" in a function?
  if (identifier->_name == kw_.identArguments &&
      !functionContext()->isGlobalScope()) {
    if (!decl || decl->scope->parentFunction != curFunctionInfo()) {
      hermes::OptValue<Decl *> argumentsDeclOpt =
          semCtx_.funcArgumentsDecl(curFunctionInfo(), identifier->_name);
      if (argumentsDeclOpt) {
        identifier->setDecl(argumentsDeclOpt.getValue());
      }
    }
    return;
  }

  // Resolved the identifier to a declaration, done.
  if (decl) {
    return;
  }

  // Undeclared variables outside `typeof` cause runtime errors in strict mode.
  if (!inTypeof && curFunctionInfo()->strict) {
    UniqueString *funcName = functionContext()->getFunctionName();

    sm_.warning(
        Warning::UndefinedVariable,
        identifier->getSourceRange(),
        Twine("the variable \"") + identifier->_name->str() +
            "\" was not declared in function \"" +
            (funcName ? funcName->str() : "global") + "\"");
  }

  // Declare an ambient global property.
  Decl *globalDecl = semCtx_.newGlobal(
      identifier->_name, Decl::Kind::UndeclaredGlobalProperty);
  identifier->setDecl(globalDecl);

  bindingTable_.insertIntoScope(
      globalScope_, identifier->_name, Binding{decl, identifier});
}

Decl *SemanticResolver::checkIdentifierResolved(
    ESTree::IdentifierNode *identifier) {
  // If identifier already resolved or unresolvable,
  // pick the resolved declaration.
  if (LLVM_UNLIKELY(identifier->isUnresolvable()))
    return nullptr;
  if (identifier->getDecl())
    return identifier->getDecl();

  // If we find the binding, assign the associated declaration and return it.
  if (Binding *binding = bindingTable_.find(identifier->_name)) {
    identifier->setDecl(binding->decl);
    return binding->decl;
  }

  // Failed to resolve.
  return nullptr;
}

void SemanticResolver::processCollectedDeclarations(ESTree::Node *scopeNode) {
  if (hermes::OptValue<llvh::ArrayRef<ESTree::Node *>> declsOpt =
          functionContext()->decls.getScopeDeclsForNode(scopeNode)) {
    processDeclarations(*declsOpt);
  }
}

void SemanticResolver::processDeclarations(const ScopeDecls &decls) {
  for (ESTree::Node *decl : decls) {
    llvh::SmallVector<ESTree::IdentifierNode *, 4> idents{};
    Decl::Kind kind = extractIdentsFromDecl(decl, idents);

    for (ESTree::IdentifierNode *ident : idents) {
      validateAndDeclareIdentifier(kind, ident);
    }
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
    if (functionContext()->isGlobalScope()) {
      return Decl::Kind::GlobalProperty;
    } else {
      return Decl::Kind::ScopedFunction;
    }
  }

  if (auto *classDecl = llvh::dyn_cast<ClassDeclarationNode>(node)) {
    extractDeclaredIdentsFromID(classDecl->_id, idents);
    return Decl::Kind::Class;
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

void SemanticResolver::extractDeclaredIdentsFromID(
    ESTree::Node *node,
    llvh::SmallVectorImpl<ESTree::IdentifierNode *> &idents) {
  // The identifier is sometimes optional, in which case it is valid.
  if (!node)
    return;

  if (auto *idNode = llvh::dyn_cast<IdentifierNode>(node)) {
    idents.push_back(idNode);
    return;
  }

  if (llvh::isa<EmptyNode>(node))
    return;

  if (auto *assign = llvh::dyn_cast<AssignmentPatternNode>(node))
    return extractDeclaredIdentsFromID(assign->_left, idents);

  if (auto *array = llvh::dyn_cast<ArrayPatternNode>(node)) {
    for (auto &elem : array->_elements) {
      extractDeclaredIdentsFromID(&elem, idents);
    }
    return;
  }

  if (auto *restElem = llvh::dyn_cast<RestElementNode>(node)) {
    return extractDeclaredIdentsFromID(restElem->_argument, idents);
  }

  if (auto *obj = llvh::dyn_cast<ObjectPatternNode>(node)) {
    for (auto &propNode : obj->_properties) {
      if (auto *prop = llvh::dyn_cast<PropertyNode>(&propNode)) {
        extractDeclaredIdentsFromID(prop->_value, idents);
      } else {
        auto *rest = cast<RestElementNode>(&propNode);
        extractDeclaredIdentsFromID(rest->_argument, idents);
      }
    }
    return;
  }

  sm_.error(node->getSourceRange(), "invalid destructuring target");
}

void SemanticResolver::validateAndDeclareIdentifier(
    Decl::Kind kind,
    ESTree::IdentifierNode *ident) {
  if (!validateDeclarationName(kind, ident))
    return;

  Binding prevName = bindingTable_.lookup(ident->_name);

  // Redeclaration of `arguments` in non-strict mode is allowed at the function
  // level, so we don't need to declare a new variable. We do need to check that
  // this isn't the global function, because `arguments` is a valid variable
  // name in the global function in non-strict mode.
  if (!curFunctionInfo()->strict && ident->_name == kw_.identArguments &&
      Decl::isKindVarLike(kind) && !functionContext()->isGlobalScope()) {
    return;
  }

  // Ignore declarations in enclosing functions.
  if (prevName.isValid() && !declInCurFunction(prevName.decl)) {
    prevName.invalidate();
  }

  Decl *decl = nullptr;

  // Handle re-declarations, ignoring ambient properties.
  if (prevName.isValid() &&
      prevName.decl->kind != Decl::Kind::UndeclaredGlobalProperty) {
    const Decl::Kind prevKind = prevName.decl->kind;
    const bool sameScope = prevName.decl->scope == curScope_;

    // Check whether the redeclaration is invalid.
    // Note that since "var" declarations have been hoisted to the function
    // scope, we cannot catch cases where "var" follows something declared in a
    // surrounding lexical scope.
    //
    // ES5Catch, var
    //          -> valid, special case ES10 B.3.5, but we can't catch it here.
    // var|scopedFunction, var|scopedFunction
    //          -> always valid
    // let, var
    //          -> always invalid
    // let, scopedFunction
    //          -> invalid if same scope
    // var|scopedFunction|let, let
    //          -> invalid if the same scope

    if ((Decl::isKindLetLike(prevKind) && Decl::isKindVarLike(kind)) ||
        (Decl::isKindLetLike(prevKind) && kind == Decl::Kind::ScopedFunction &&
         sameScope) ||
        (Decl::isKindLetLike(kind) && sameScope)) {
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
    // Var, ScopedFunc -> if non-strict or same scope, then use prev,
    //                    else declare new
    else if (
        Decl::isKindVarLike(prevKind) &&
        Decl::isKindVarLikeOrScopedFunction(kind)) {
      if (sameScope || !curFunctionInfo()->strict)
        decl = prevName.decl;
      else
        decl = nullptr;
    }
    // ScopedFunc, ScopedFunc same scope -> use prev
    // ScopedFunc, ScopedFunc new scope -> declare new
    else if (
        prevKind == Decl::Kind::ScopedFunction &&
        kind == Decl::Kind::ScopedFunction) {
      if (sameScope) {
        decl = prevName.decl;
      } else {
        decl = nullptr;
      }
    }
    // ScopedFunc, Var -> convert to var
    else if (
        prevKind == Decl::Kind::ScopedFunction && Decl::isKindVarLike(kind)) {
      assert(
          sameScope &&
          "we can only encounter Var after ScopedFunction in the same scope");
      // Since they are in the same scope, we can simply convert the existing
      // ScopedFunction to Var.
      decl = prevName.decl;
      decl->kind = Decl::Kind::Var;
    } else {
      decl = nullptr;
    }
  }

  // Create new decl.
  if (!decl) {
    if (Decl::isKindGlobal(kind))
      decl = semCtx_.newGlobal(ident->_name, kind);
    else
      decl = semCtx_.newDecl(ident->_name, kind, curScope_);
    bindingTable_.insert(ident->_name, Binding{decl, ident});
  }

  ident->setDecl(decl);
}

bool SemanticResolver::validateDeclarationName(
    Decl::Kind declKind,
    const ESTree::IdentifierNode *idNode) const {
  if (curFunctionInfo()->strict) {
    // - 'arguments' cannot be redeclared in strict mode.
    // - 'eval' cannot be redeclared in strict mode. If it is disabled we
    // we don't report an error because it will be reported separately.
    if (idNode->_name == kw_.identArguments ||
        (idNode->_name == kw_.identEval && astContext_.getEnableEval())) {
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

void SemanticResolver::validateAssignmentTarget(const Node *node) {
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

bool SemanticResolver::isLValue(const ESTree::Node *node) {
  if (llvh::isa<MemberExpressionNode>(node))
    return true;

  if (auto *id = llvh::dyn_cast<IdentifierNode>(node)) {
    if (LLVM_UNLIKELY(id->_name == kw_.identArguments)) {
      // "arguments" is only valid assignment in loose mode.
      return !curFunctionInfo()->strict;
    }
    if (LLVM_UNLIKELY(id->_name == kw_.identEval)) {
      // "eval" is only valid assignment in loose mode.
      return !curFunctionInfo()->strict;
    }
    return true;
  }

  return false;
}

void SemanticResolver::recursionDepthExceeded(ESTree::Node *n) {
  sm_.error(
      n->getEndLoc(), "Too many nested expressions/statements/declarations");
}

ESTree::Node *SemanticResolver::findUseStrict(ESTree::NodeList &body) const {
  for (auto &node : body) {
    if (auto *expr = llvh::dyn_cast<ExpressionStatementNode>(&node)) {
      if (expr->_directive == kw_.identUseStrict) {
        return &node;
      }
    }
  }
  return nullptr;
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
  // Associate the scope with the node.
  scopeNode->setScope(scope);
}
SemanticResolver::ScopeRAII::~ScopeRAII() {
  resolver_.curScope_ = oldScope_;
}

//===----------------------------------------------------------------------===//
// FunctionContext

FunctionContext::FunctionContext(
    SemanticResolver &resolver,
    ESTree::FunctionLikeNode *node,
    FunctionInfo *semInfo,
    bool strict)
    : resolver_(resolver),
      semInfo(resolver.semCtx_
                  .newFunction(node, semInfo, resolver.curScope_, strict)),
      node(node),
      decls(DeclCollector::run(node, resolver.keywords())) {
  resolver.functionStack_.push_back(this);
}

FunctionContext::~FunctionContext() {
  assert(
      resolver_.curFunctionInfo() == semInfo &&
      "FunctionContext out of sync with SemContext");
  // If not the global function, pop it.
  resolver_.functionStack_.pop_back();
}

UniqueString *FunctionContext::getFunctionName() const {
  if (node) {
    if (auto *idNode =
            llvh::dyn_cast_or_null<IdentifierNode>(getIdentifier(node)))
      return idNode->_name;
  }
  return nullptr;
}

} // namespace sema
} // namespace hermes
