/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "ESTreeIRGen.h"

#include "llvm/Support/SaveAndRestore.h"

namespace hermes {
namespace irgen {

//===----------------------------------------------------------------------===//
// Free standing helpers.

Instruction *emitLoad(IRBuilder &builder, Value *from, bool inhibitThrow) {
  if (auto *var = dyn_cast<Variable>(from)) {
    return builder.createLoadFrameInst(var);
  } else if (auto *globalProp = dyn_cast<GlobalObjectProperty>(from)) {
    if (globalProp->isDeclared() || inhibitThrow)
      return builder.createLoadPropertyInst(
          builder.getGlobalObject(), globalProp->getName());
    else
      return builder.createTryLoadGlobalPropertyInst(globalProp);
  } else {
    llvm_unreachable("unvalid value to load from");
  }
}

Instruction *emitStore(IRBuilder &builder, Value *storedValue, Value *ptr) {
  if (auto *var = dyn_cast<Variable>(ptr)) {
    return builder.createStoreFrameInst(storedValue, var);
  } else if (auto *globalProp = dyn_cast<GlobalObjectProperty>(ptr)) {
    if (globalProp->isDeclared() || !builder.getFunction()->isStrictMode())
      return builder.createStorePropertyInst(
          storedValue, builder.getGlobalObject(), globalProp->getName());
    else
      return builder.createTryStoreGlobalPropertyInst(storedValue, globalProp);
  } else {
    llvm_unreachable("unvalid value to load from");
  }
}

//===----------------------------------------------------------------------===//
// ESTreeIRGen

ESTreeIRGen::ESTreeIRGen(
    ESTree::Node *root,
    const DeclarationFileListTy &declFileList,
    Module *M,
    const ScopeChain &scopeChain)
    : Mod(M),
      Builder(Mod),
      Root(root),
      DeclarationFileList(declFileList),
      lexicalScopeChain(resolveScopeIdentifiers(scopeChain)),
      identEval_(Builder.createIdentifier("eval")) {}

void ESTreeIRGen::doIt() {
  DEBUG(dbgs() << "Processing top level program.\n");

  ESTree::ProgramNode *Program;

  if (auto File = dyn_cast<ESTree::FileNode>(Root)) {
    DEBUG(dbgs() << "Found File decl.\n");
    Program = dyn_cast<ESTree::ProgramNode>(File->_program);
  } else {
    Program = dyn_cast<ESTree::ProgramNode>(Root);
  }

  if (!Program) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        SMLoc{}, "missing 'Program' AST node");
    return;
  }

  DEBUG(dbgs() << "Found Program decl.\n");

  // The function which will "execute" the module.
  Function *topLevelFunction;

  // Function context used only when compiling in an existing lexical scope
  // chain. It is only initialized if we have a lexical scope chain.
  llvm::Optional<FunctionContext> wrapperFunctionContext{};

  if (!lexicalScopeChain) {
    topLevelFunction = Builder.createTopLevelFunction(
        ESTree::isStrict(Program->strictness), Program->getSourceRange());
  } else {
    // If compiling in an existing lexical context, we need to install the
    // scopes in a wrapper function, which represents the "global" code.

    Function *wrapperFunction = Builder.createFunction(
        "",
        Function::DefinitionKind::ES5Function,
        ESTree::isStrict(Program->strictness),
        Program->getSourceRange(),
        true);

    // Initialize the wrapper context.
    wrapperFunctionContext.emplace(this, wrapperFunction, nullptr);

    // Populate it with dummy code so it doesn't crash the back-end.
    genDummyFunction(wrapperFunction);

    // Restore the previously saved parent scopes.
    materializeScopesInChain(wrapperFunction, lexicalScopeChain, 1);

    // Finally create the function which will actually be executed.
    topLevelFunction = Builder.createFunction(
        "eval",
        Function::DefinitionKind::ES5Function,
        ESTree::isStrict(Program->strictness),
        Program->getSourceRange(),
        false);
  }

  Mod->setTopLelevFunction(topLevelFunction);

  // Function context for topLevelFunction.
  FunctionContext topLevelFunctionContext{
      this, topLevelFunction, Program->getSemInfo()};

  // IRGen needs a pointer to the outer-most context, which is either
  // topLevelContext or wrapperFunctionContext, depending on whether the latter
  // was created.
  // We want to set the pointer to that outer-most context, but ensure that it
  // doesn't outlive the context it is pointing to.
  llvm::SaveAndRestore<FunctionContext *> saveTopLevelContext(
      topLevelContext,
      !wrapperFunctionContext.hasValue() ? &topLevelFunctionContext
                                         : &wrapperFunctionContext.getValue());

  // Now declare all externally supplied global properties, but only if we don't
  // have a lexical scope chain.
  if (!lexicalScopeChain) {
    for (auto declFile : DeclarationFileList) {
      processDeclarationFile(declFile);
    }
  }

  doGenFunctionLike(
      curFunction()->function,
      Program,
      ESTree::NodeList{},
      Program,
      [this](ESTree::Node *body) {
        // Allocate the return register, initialize it to undefined.
        curFunction()->globalReturnRegister =
            Builder.createAllocStackInst(genAnonymousLabelName("ret"));
        Builder.createStoreStackInst(
            Builder.getLiteralUndefined(), curFunction()->globalReturnRegister);

        genBody(cast<ESTree::ProgramNode>(body)->_body);

        // Terminate the top-level scope with a return statement.
        auto ret_val =
            Builder.createLoadStackInst(curFunction()->globalReturnRegister);
        Builder.createReturnInst(ret_val);
      });
}

void ESTreeIRGen::doCJSModule(
    Function *topLevelFunction,
    sem::FunctionInfo *semInfo,
    llvm::StringRef filename) {
  assert(Root && "no root in ESTreeIRGen");
  auto *func = cast<ESTree::FunctionExpressionNode>(Root);
  assert(func && "doCJSModule without a module");

  FunctionContext topLevelFunctionContext{this, topLevelFunction, semInfo};
  llvm::SaveAndRestore<FunctionContext *> saveTopLevelContext(
      topLevelContext, &topLevelFunctionContext);

  Identifier functionName = Builder.createIdentifier("cjs_module");
  Function *newFunc = genFunctionLike(
      functionName,
      nullptr,
      Function::DefinitionKind::ES5Function,
      ESTree::isStrict(func->strictness),
      func,
      func->_params,
      func->_body,
      Mod->getContext().isLazyCompilation());

  Builder.getModule()->addCJSModule(
      Builder.createIdentifier(filename), newFunc);
}

Function *ESTreeIRGen::doLazyFunction(hbc::LazyCompilationData *lazyData) {
  // Create a dummy top level function so IRGen doesn't think our lazyFunction
  // is in global scope.
  Function *topLevel = Builder.createTopLevelFunction(lazyData->strictMode, {});
  genDummyFunction(topLevel);

  FunctionContext topLevelFunctionContext{this, topLevel, nullptr};

  // Save the top-level context, but ensure it doesn't outlive what it is
  // pointing to.
  llvm::SaveAndRestore<FunctionContext *> saveTopLevelContext(
      topLevelContext, &topLevelFunctionContext);

  auto *node = cast<ESTree::FunctionLikeNode>(Root);

  // Restore the previously saved parent scopes.
  lexicalScopeChain = lazyData->parentScope;
  materializeScopesInChain(topLevel, lexicalScopeChain, 1);

  // If lazyData->closureAlias is specified, we must create an alias binding
  // between originalName (which must be valid) and the variable identified by
  // closureAlias.
  Variable *parentVar = nullptr;
  if (lazyData->closureAlias.isValid()) {
    assert(lazyData->originalName.isValid() && "Original name invalid");
    assert(
        lazyData->originalName != lazyData->closureAlias &&
        "Original name must be different from the alias");

    // NOTE: the closureAlias target must exist and must be a Variable.
    parentVar = cast<Variable>(nameTable_.lookup(lazyData->closureAlias));

    // Re-create the alias.
    nameTable_.insert(lazyData->originalName, parentVar);
  }

  ESTree::NodeList const *params;
  ESTree::NodePtr body;
  Function::DefinitionKind definitionKind =
      Function::DefinitionKind::ES5Function;

  if (auto *FE = dyn_cast<ESTree::FunctionExpressionNode>(node)) {
    params = &FE->_params;
    body = FE->_body;
  } else if (auto *FD = dyn_cast<ESTree::FunctionDeclarationNode>(node)) {
    params = &FD->_params;
    body = FD->_body;
  } else if (auto *AF = dyn_cast<ESTree::ArrowFunctionExpressionNode>(node)) {
    // FIXME: Arrow functions are broken with lazy compilation because of the
    //`this` binding.
    assert(false && "Lazy compilation not supported in ES6");
    params = &FD->_params;
    body = FD->_body;
    definitionKind = Function::DefinitionKind::ES6Arrow;
  } else {
    llvm_unreachable("invalid lazy function AST node");
  }

  return genFunctionLike(
      lazyData->originalName,
      parentVar,
      definitionKind,
      lazyData->strictMode,
      node,
      *params,
      body,
      false);
}

std::pair<Value *, bool> ESTreeIRGen::declareVariableOrGlobalProperty(
    Function *inFunc,
    Identifier name) {
  Value *found = nameTable_.lookup(name);

  // If the variable is already declared in this scope, do not create a
  // second instance.
  if (found) {
    if (auto *var = dyn_cast<Variable>(found)) {
      if (var->getParent()->getFunction() == inFunc)
        return {found, false};
    } else {
      assert(
          isa<GlobalObjectProperty>(found) &&
          "Invalid value found in name table");
      if (inFunc->isGlobalScope())
        return {found, false};
    }
  }

  // Create a property if global scope, variable otherwise.
  Value *var;
  if (inFunc->isGlobalScope()) {
    var = Builder.createGlobalObjectProperty(name, true);
  } else {
    var = Builder.createVariable(inFunc->getFunctionScope(), name);
  }

  // Register the variable in the scoped hash table.
  nameTable_.insert(name, var);
  return {var, true};
}

GlobalObjectProperty *ESTreeIRGen::declareAmbientGlobalProperty(
    Identifier name) {
  // Avoid redefining global properties.
  auto *prop = dyn_cast_or_null<GlobalObjectProperty>(nameTable_.lookup(name));
  if (prop)
    return prop;

  DEBUG(
      llvm::dbgs() << "declaring ambient global property " << name << " "
                   << name.getUnderlyingPointer() << "\n");

  prop = Builder.createGlobalObjectProperty(name, false);
  nameTable_.insertIntoScope(&topLevelContext->scope, name, prop);
  return prop;
}

namespace {
/// This visitor structs collects declarations within a single closure without
/// descending into child closures.
struct DeclHoisting {
  /// The list of collected identifiers (variables and functions).
  llvm::SmallVector<ESTree::VariableDeclaratorNode *, 8> decls{};

  /// A list of functions that need to be hoisted and materialized before we
  /// can generate the rest of the function.
  llvm::SmallVector<ESTree::FunctionDeclarationNode *, 8> closures;

  explicit DeclHoisting() = default;
  ~DeclHoisting() = default;

  /// Extract the variable name from the nodes that can define new variables.
  /// The nodes that can define a new variable in the scope are:
  /// VariableDeclarator and FunctionDeclaration>
  void collectDecls(ESTree::Node *V) {
    if (auto VD = dyn_cast<ESTree::VariableDeclaratorNode>(V)) {
      return decls.push_back(VD);
    }

    if (auto FD = dyn_cast<ESTree::FunctionDeclarationNode>(V)) {
      return closures.push_back(FD);
    }
  }

  bool shouldVisit(ESTree::Node *V) {
    // Collect declared names, even if we don't descend into children nodes.
    collectDecls(V);

    // Do not descend to child closures because the variables they define are
    // not exposed to the outside function.
    if (isa<ESTree::FunctionDeclarationNode>(V) ||
        isa<ESTree::FunctionExpressionNode>(V) ||
        isa<ESTree::ArrowFunctionExpressionNode>(V))
      return false;
    return true;
  }

  void enter(ESTree::Node *V) {}
  void leave(ESTree::Node *V) {}
};

} // anonymous namespace.

void ESTreeIRGen::processDeclarationFile(ESTree::FileNode *fileNode) {
  auto File = dyn_cast_or_null<ESTree::FileNode>(fileNode);
  if (!File)
    return;

  auto Program = dyn_cast_or_null<ESTree::ProgramNode>(File->_program);
  if (!Program)
    return;

  DeclHoisting DH;
  Program->visit(DH);

  // Create variable declarations for each of the hoisted variables.
  for (auto vd : DH.decls)
    declareAmbientGlobalProperty(getNameFieldFromID(vd->_id));
  for (auto fd : DH.closures)
    declareAmbientGlobalProperty(getNameFieldFromID(fd->_id));
}

Value *ESTreeIRGen::ensureVariableExists(ESTree::IdentifierNode *id) {
  assert(id && "id must be a valid Identifier node");
  Identifier name = getNameFieldFromID(id);

  // Check if this is a known variable.
  if (auto *var = nameTable_.lookup(name))
    return var;

  if (curFunction()->function->isStrictMode()) {
    // Report a warning in strict mode.
    auto currentFunc = Builder.getInsertionBlock()->getParent();

    Builder.getModule()->getContext().getSourceErrorManager().warning(
        Warning::UndefinedVariable,
        id->getSourceRange(),
        Twine("the variable \"") + name.str() +
            "\" was not declared in function \"" +
            currentFunc->getInternalNameStr() + "\"");
  }

  // Undeclared variable is an ambient global property.
  return declareAmbientGlobalProperty(name);
}

Value *ESTreeIRGen::genMemberExpressionProperty(
    ESTree::MemberExpressionNode *Mem) {
  // If computed is true, the node corresponds to a computed (a[b]) member
  // lookup and '_property' is an Expression. Otherwise, the node
  // corresponds to a static (a.b) member lookup and '_property' is an
  // Identifier.
  // Details of the computed field are available here:
  // https://github.com/estree/estree/blob/master/spec.md#memberexpression

  if (Mem->_computed) {
    return genExpression(Mem->_property);
  }

  // Arrays and objects may be accessed with integer indices.
  if (auto N = dyn_cast<ESTree::NumericLiteralNode>(Mem->_property)) {
    return Builder.getLiteralNumber(N->_value);
  }

  // ESTree encodes property access as MemberExpression -> Identifier.
  auto Id = cast<ESTree::IdentifierNode>(Mem->_property);

  Identifier fieldName = getNameFieldFromID(Id);
  DEBUG(
      dbgs() << "Emitting direct lable access to field '" << fieldName
             << "'\n");
  return Builder.getLiteralString(fieldName);
}

LReference ESTreeIRGen::createLRef(ESTree::Node *node) {
  SMLoc sourceLoc = node->getDebugLoc();
  IRBuilder::ScopedLocationChange slc(Builder, sourceLoc);

  /// Create lref for member expression (ex: o.f).
  if (auto *ME = dyn_cast<ESTree::MemberExpressionNode>(node)) {
    DEBUG(dbgs() << "Creating an LRef for member expression.\n");
    Value *obj = genExpression(ME->_object);
    Value *prop = genMemberExpressionProperty(ME);
    return LReference(obj, prop, sourceLoc);
  }

  /// Create lref for identifiers  (ex: a).
  if (auto *iden = dyn_cast<ESTree::IdentifierNode>(node)) {
    DEBUG(dbgs() << "Creating an LRef for identifier.\n");
    DEBUG(
        dbgs() << "Looking for identifier \"" << getNameFieldFromID(iden)
               << "\"\n");
    auto *var = ensureVariableExists(iden);
    return LReference(var, nullptr, sourceLoc);
  }

  /// Create lref for variable decls (ex: var a).
  if (auto *V = dyn_cast<ESTree::VariableDeclarationNode>(node)) {
    DEBUG(dbgs() << "Creating an LRef for variable declaration.\n");

    assert(V->_declarations.size() == 1 && "Malformed variable declaration");
    auto decl = cast<ESTree::VariableDeclaratorNode>(&V->_declarations.front());

    DEBUG(
        dbgs() << "Looking for var-identifier \""
               << getNameFieldFromID(decl->_id) << "\"\n");
    auto *var =
        ensureVariableExists(dyn_cast<ESTree::IdentifierNode>(decl->_id));
    return LReference(var, nullptr, sourceLoc);
  }

  llvm_unreachable("Unexpected for-in pattern.");
}

std::shared_ptr<SerializedScope> ESTreeIRGen::resolveScopeIdentifiers(
    const ScopeChain &chain) {
  std::shared_ptr<SerializedScope> current{};
  for (auto it = chain.functions.rbegin(), end = chain.functions.rend();
       it < end;
       it++) {
    auto next = std::make_shared<SerializedScope>();
    next->variables.reserve(it->variables.size());
    for (auto var : it->variables) {
      next->variables.push_back(std::move(Builder.createIdentifier(var)));
    }
    next->parentScope = current;
    current = next;
  }
  return current;
}

void ESTreeIRGen::materializeScopesInChain(
    Function *wrapperFunction,
    const std::shared_ptr<const SerializedScope> &scope,
    int depth) {
  if (!scope)
    return;
  assert(depth < 1000 && "Excessive scope depth");

  // First materialize parent scopes.
  materializeScopesInChain(wrapperFunction, scope->parentScope, depth + 1);

  // If scope->closureAlias is specified, we must create an alias binding
  // between originalName (which must be valid) and the variable identified by
  // closureAlias.
  //
  // We do this *before* inserting the other variables below to reflect that
  // the closure alias is conceptually in an outside scope and also avoid the
  // closure name incorrectly shadowing the same name inside the closure.
  if (scope->closureAlias.isValid()) {
    assert(scope->originalName.isValid() && "Original name invalid");
    assert(
        scope->originalName != scope->closureAlias &&
        "Original name must be different from the alias");

    // NOTE: the closureAlias target must exist and must be a Variable.
    auto *closureVar = cast<Variable>(nameTable_.lookup(scope->closureAlias));

    // Re-create the alias.
    nameTable_.insert(scope->originalName, closureVar);
  }

  // Create an external scope.
  ExternalScope *ES = Builder.createExternalScope(wrapperFunction, -depth);
  for (auto variableId : scope->variables) {
    auto *variable = Builder.createVariable(ES, variableId);
    nameTable_.insert(variableId, variable);
  }
}

#ifndef HERMESVM_LEAN
std::shared_ptr<SerializedScope> ESTreeIRGen::saveCurrentScope() {
  auto *func = curFunction()->function;
  assert(func && "Missing function when saving scope");

  auto scope = std::make_shared<SerializedScope>();

  // We currently only lazy compile a single level at a time. If we later start
  // compiling multiple, this method would need to walk the scopes.
  assert(
      ((func->isGlobalScope() && !curFunction()->getPreviousContext()) ||
       (!func->isGlobalScope() && curFunction()->getPreviousContext() &&
        !curFunction()->getPreviousContext()->getPreviousContext())) &&
      "Expected exactly one function on the stack.");

  scope->parentScope = lexicalScopeChain;
  scope->originalName = func->getOriginalOrInferredName();
  if (auto *closure = func->getLazyClosureAlias()) {
    scope->closureAlias = closure->getName();
  }
  for (auto *var : func->getFunctionScope()->getVariables()) {
    scope->variables.push_back(var->getName());
  }
  return scope;
}
#endif

} // namespace irgen
} // namespace hermes
