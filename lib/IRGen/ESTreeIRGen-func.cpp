/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

#include "llvm/ADT/SmallString.h"

namespace hermes {
namespace irgen {

//===----------------------------------------------------------------------===//
// FunctionContext

FunctionContext::FunctionContext(
    ESTreeIRGen *irGen,
    Function *function,
    sem::FunctionInfo *semInfo)
    : irGen_(irGen),
      semInfo_(semInfo),
      oldContext_(irGen->functionContext_),
      builderSaveState_(irGen->Builder),
      function(function),
      scope(irGen->nameTable_) {
  irGen->functionContext_ = this;

  // Initialize it to LiteraUndefined by default to avoid corner cases.
  this->capturedNewTarget = irGen->Builder.getLiteralUndefined();

  if (semInfo_) {
    // Allocate the label table. Each label definition will be encountered in
    // the AST before it is referenced (because of the nature of JavaScript), at
    // which point we will initialize the GotoLabel structure with basic blocks
    // targets.
    labels_.resize(semInfo_->labelCount);
  }
}

FunctionContext::~FunctionContext() {
  irGen_->functionContext_ = oldContext_;
}

Identifier FunctionContext::genAnonymousLabelName(StringRef hint) {
  llvm::SmallString<16> buf;
  llvm::raw_svector_ostream nameBuilder{buf};
  nameBuilder << "?anon_" << anonymousLabelCounter++ << "_" << hint;
  return function->getContext().getIdentifier(nameBuilder.str());
}

//===----------------------------------------------------------------------===//
// ESTreeIRGen

void ESTreeIRGen::genFunctionDeclaration(
    ESTree::FunctionDeclarationNode *func) {
  // Find the name of the function.
  Identifier functionName = getNameFieldFromID(func->_id);
  LLVM_DEBUG(dbgs() << "IRGen function \"" << functionName << "\".\n");

  auto *funcStorage = nameTable_.lookup(functionName);
  assert(
      funcStorage && "function declaration variable should have been hoisted");

  Function *newFunc = func->_generator
      ? genGeneratorFunction(functionName, nullptr, func)
      : genES5Function(functionName, nullptr, func);

  // Store the newly created closure into a frame variable with the same name.
  auto *newClosure = Builder.createCreateFunctionInst(newFunc);

  emitStore(Builder, newClosure, funcStorage, true);
}

Value *ESTreeIRGen::genFunctionExpression(
    ESTree::FunctionExpressionNode *FE,
    Identifier nameHint) {
  LLVM_DEBUG(
      dbgs() << "Creating anonymous closure. "
             << Builder.getInsertionBlock()->getParent()->getInternalName()
             << ".\n");

  NameTableScopeTy newScope(nameTable_);
  Variable *tempClosureVar = nullptr;

  Identifier originalNameIden = nameHint;
  if (FE->_id) {
    auto closureName = genAnonymousLabelName("closure");
    tempClosureVar = Builder.createVariable(
        curFunction()->function->getFunctionScope(),
        Variable::DeclKind::Var,
        closureName);

    // Insert the synthesized variable into the name table, so it can be
    // looked up internally as well.
    nameTable_.insertIntoScope(
        &curFunction()->scope, tempClosureVar->getName(), tempClosureVar);

    // Alias the lexical name to the synthesized variable.
    originalNameIden = getNameFieldFromID(FE->_id);
    nameTable_.insert(originalNameIden, tempClosureVar);
  }

  Function *newFunc = FE->_generator
      ? genGeneratorFunction(originalNameIden, tempClosureVar, FE)
      : genES5Function(originalNameIden, tempClosureVar, FE);

  Value *closure = Builder.createCreateFunctionInst(newFunc);

  if (tempClosureVar)
    emitStore(Builder, closure, tempClosureVar, true);

  return closure;
}

Value *ESTreeIRGen::genArrowFunctionExpression(
    ESTree::ArrowFunctionExpressionNode *AF,
    Identifier nameHint) {
  LLVM_DEBUG(
      dbgs() << "Creating arrow function. "
             << Builder.getInsertionBlock()->getParent()->getInternalName()
             << ".\n");

  auto *newFunc = Builder.createFunction(
      nameHint,
      Function::DefinitionKind::ES6Arrow,
      ESTree::isStrict(AF->strictness),
      AF->getSourceRange());

  {
    FunctionContext newFunctionContext{this, newFunc, AF->getSemInfo()};

    // Propagate captured "this", "new.target" and "arguments" from parents.
    auto *prev = curFunction()->getPreviousContext();
    curFunction()->capturedThis = prev->capturedThis;
    curFunction()->capturedNewTarget = prev->capturedNewTarget;
    curFunction()->capturedArguments = prev->capturedArguments;

    emitFunctionPrologue(
        AF,
        Builder.createBasicBlock(newFunc),
        InitES5CaptureState::No,
        DoEmitParameters::Yes);

    genStatement(AF->_body);
    emitFunctionEpilogue(Builder.getLiteralUndefined());
  }

  // Emit CreateFunctionInst after we have restored the builder state.
  return Builder.createCreateFunctionInst(newFunc);
}

#ifndef HERMESVM_LEAN
namespace {
ESTree::NodeKind getLazyFunctionKind(ESTree::FunctionLikeNode *node) {
  if (node->isMethodDefinition) {
    // This is not a regular function expression but getter/setter.
    // If we want to reparse it later, we have to start from an
    // identifier and not from a 'function' keyword.
    return ESTree::NodeKind::Property;
  }
  return node->getKind();
}
} // namespace
Function *ESTreeIRGen::genES5Function(
    Identifier originalName,
    Variable *lazyClosureAlias,
    ESTree::FunctionLikeNode *functionNode,
    bool isGeneratorInnerFunction) {
  assert(functionNode && "Function AST cannot be null");

  auto *body = ESTree::getBlockStatement(functionNode);
  assert(body && "body of ES5 function cannot be null");

  Function *newFunction = isGeneratorInnerFunction
      ? Builder.createGeneratorInnerFunction(
            originalName,
            Function::DefinitionKind::ES5Function,
            ESTree::isStrict(functionNode->strictness),
            functionNode->getSourceRange(),
            /* insertBefore */ nullptr)
      : Builder.createFunction(
            originalName,
            Function::DefinitionKind::ES5Function,
            ESTree::isStrict(functionNode->strictness),
            functionNode->getSourceRange(),
            /* isGlobal */ false,
            /* insertBefore */ nullptr);

  newFunction->setLazyClosureAlias(lazyClosureAlias);

  if (auto *bodyBlock = llvm::dyn_cast<ESTree::BlockStatementNode>(body)) {
    if (bodyBlock->isLazyFunctionBody) {
      // Set the AST position and variable context so we can continue later.
      newFunction->setLazyScope(saveCurrentScope());
      auto &lazySource = newFunction->getLazySource();
      lazySource.bufferId = bodyBlock->bufferId;
      lazySource.nodeKind = getLazyFunctionKind(functionNode);
      lazySource.functionRange = functionNode->getSourceRange();

      // Set the function's .length.
      newFunction->setExpectedParamCountIncludingThis(
          countExpectedArgumentsIncludingThis(functionNode));
      return newFunction;
    }
  }

  FunctionContext newFunctionContext{
      this, newFunction, functionNode->getSemInfo()};

  if (isGeneratorInnerFunction) {
    // StartGeneratorInst
    // ResumeGeneratorInst
    // at the beginning of the function, to allow for the first .next() call.
    auto *initGenBB = Builder.createBasicBlock(newFunction);
    Builder.setInsertionBlock(initGenBB);
    Builder.createStartGeneratorInst();
    auto *prologueBB = Builder.createBasicBlock(newFunction);
    auto *prologueResumeIsReturn = Builder.createAllocStackInst(
        genAnonymousLabelName("isReturn_prologue"));
    genResumeGenerator(nullptr, prologueResumeIsReturn, prologueBB);

    if (hasSimpleParams(functionNode)) {
      // If there are simple params, then we don't need an extra yield/resume.
      // They can simply be initialized on the first call to `.next`.
      Builder.setInsertionBlock(prologueBB);
      emitFunctionPrologue(
          functionNode,
          prologueBB,
          InitES5CaptureState::Yes,
          DoEmitParameters::Yes);
    } else {
      // If there are non-simple params, then we must add a new yield/resume.
      // The `.next()` call will occur once in the outer function, before
      // the iterator is returned to the caller of the `function*`.
      auto *entryPointBB = Builder.createBasicBlock(newFunction);
      auto *entryPointResumeIsReturn =
          Builder.createAllocStackInst(genAnonymousLabelName("isReturn_entry"));

      // Initialize parameters.
      Builder.setInsertionBlock(prologueBB);
      emitFunctionPrologue(
          functionNode,
          prologueBB,
          InitES5CaptureState::Yes,
          DoEmitParameters::Yes);
      Builder.createSaveAndYieldInst(
          Builder.getLiteralUndefined(), entryPointBB);

      // Actual entry point of function from the caller's perspective.
      Builder.setInsertionBlock(entryPointBB);
      genResumeGenerator(
          nullptr,
          entryPointResumeIsReturn,
          Builder.createBasicBlock(newFunction));
    }
  } else {
    emitFunctionPrologue(
        functionNode,
        Builder.createBasicBlock(newFunction),
        InitES5CaptureState::Yes,
        DoEmitParameters::Yes);
  }

  genStatement(body);
  emitFunctionEpilogue(Builder.getLiteralUndefined());

  return curFunction()->function;
}
#endif

Function *ESTreeIRGen::genGeneratorFunction(
    Identifier originalName,
    Variable *lazyClosureAlias,
    ESTree::FunctionLikeNode *functionNode) {
  assert(functionNode && "Function AST cannot be null");

  // Build the outer function which creates the generator.
  // Does not have an associated source range.
  auto *outerFn = Builder.createGeneratorFunction(
      originalName,
      Function::DefinitionKind::ES5Function,
      ESTree::isStrict(functionNode->strictness),
      /* insertBefore */ nullptr);

  auto *innerFn = genES5Function(
      genAnonymousLabelName(originalName.isValid() ? originalName.str() : ""),
      lazyClosureAlias,
      functionNode,
      true);

  {
    FunctionContext outerFnContext{this, outerFn, functionNode->getSemInfo()};
    emitFunctionPrologue(
        functionNode,
        Builder.createBasicBlock(outerFn),
        InitES5CaptureState::Yes,
        DoEmitParameters::No);

    // Create a generator function, which will store the arguments.
    auto *gen = Builder.createCreateGeneratorInst(innerFn);

    if (!hasSimpleParams(functionNode)) {
      // If there are non-simple params, step the inner function once to
      // initialize them.
      Value *next = Builder.createLoadPropertyInst(gen, "next");
      Builder.createCallInst(next, gen, {});
    }

    emitFunctionEpilogue(gen);
  }

  return outerFn;
}

void ESTreeIRGen::initCaptureStateInES5FunctionHelper() {
  // Capture "this", "new.target" and "arguments" if there are inner arrows.
  if (!curFunction()->getSemInfo()->containsArrowFunctions)
    return;

  auto *scope = curFunction()->function->getFunctionScope();

  // "this".
  curFunction()->capturedThis = Builder.createVariable(
      scope, Variable::DeclKind::Var, genAnonymousLabelName("this"));
  emitStore(
      Builder,
      Builder.getFunction()->getThisParameter(),
      curFunction()->capturedThis,
      true);

  // "new.target".
  curFunction()->capturedNewTarget = Builder.createVariable(
      scope, Variable::DeclKind::Var, genAnonymousLabelName("new.target"));
  emitStore(
      Builder,
      Builder.createGetNewTargetInst(),
      curFunction()->capturedNewTarget,
      true);

  // "arguments".
  if (curFunction()->getSemInfo()->containsArrowFunctionsUsingArguments) {
    curFunction()->capturedArguments = Builder.createVariable(
        scope, Variable::DeclKind::Var, genAnonymousLabelName("arguments"));
    emitStore(
        Builder,
        curFunction()->createArgumentsInst,
        curFunction()->capturedArguments,
        true);
  }
}

void ESTreeIRGen::emitFunctionPrologue(
    ESTree::FunctionLikeNode *funcNode,
    BasicBlock *entry,
    InitES5CaptureState doInitES5CaptureState,
    DoEmitParameters doEmitParameters) {
  auto *newFunc = curFunction()->function;
  auto *semInfo = curFunction()->getSemInfo();
  LLVM_DEBUG(
      dbgs() << "Hoisting "
             << (semInfo->varDecls.size() + semInfo->closures.size())
             << " variable decls.\n");

  Builder.setLocation(newFunc->getSourceRange().Start);

  // Start pumping instructions into the entry basic block.
  Builder.setInsertionBlock(entry);

  // Always insert a CreateArgumentsInst. We will delete it later if it is
  // unused.
  curFunction()->createArgumentsInst = Builder.createCreateArgumentsInst();

  // Create variable declarations for each of the hoisted variables and
  // functions. Initialize only the variables to undefined.
  for (auto decl : semInfo->varDecls) {
    auto res = declareVariableOrGlobalProperty(
        newFunc, decl.kind, getNameFieldFromID(decl.identifier));
    // If this is not a frame variable or it was already declared, skip.
    auto *var = llvm::dyn_cast<Variable>(res.first);
    if (!var || !res.second)
      continue;

    // Otherwise, initialize it to undefined.
    Builder.createStoreFrameInst(Builder.getLiteralUndefined(), var);
    if (var->getRelatedVariable()) {
      Builder.createStoreFrameInst(
          Builder.getLiteralUndefined(), var->getRelatedVariable());
    }
  }
  for (auto *fd : semInfo->closures) {
    declareVariableOrGlobalProperty(
        newFunc, VarDecl::Kind::Var, getNameFieldFromID(fd->_id));
  }

  // Always create the "this" parameter. It needs to be created before we
  // initialized the ES5 capture state.
  Builder.createParameter(newFunc, "this");

  if (doInitES5CaptureState != InitES5CaptureState::No)
    initCaptureStateInES5FunctionHelper();

  // Construct the parameter list. Create function parameters and register
  // them in the scope.
  if (doEmitParameters == DoEmitParameters::Yes) {
    emitParameters(funcNode);
  } else {
    newFunc->setExpectedParamCountIncludingThis(
        countExpectedArgumentsIncludingThis(funcNode));
  }

  // Generate the code for import declarations before generating the rest of the
  // body.
  for (auto importDecl : semInfo->imports) {
    genImportDeclaration(importDecl);
  }

  // Generate and initialize the code for the hoisted function declarations
  // before generating the rest of the body.
  for (auto funcDecl : semInfo->closures) {
    genFunctionDeclaration(funcDecl);
  }
}

void ESTreeIRGen::emitParameters(ESTree::FunctionLikeNode *funcNode) {
  auto *newFunc = curFunction()->function;

  LLVM_DEBUG(dbgs() << "IRGen function parameters.\n");

  // Create a variable for every parameter.
  for (auto paramDecl : funcNode->getSemInfo()->paramNames) {
    Identifier paramName = getNameFieldFromID(paramDecl.identifier);
    LLVM_DEBUG(dbgs() << "Adding parameter: " << paramName << "\n");
    auto *paramStorage = Builder.createVariable(
        newFunc->getFunctionScope(), Variable::DeclKind::Var, paramName);
    // Register the storage for the parameter.
    nameTable_.insert(paramName, paramStorage);
  }

  // FIXME: T42569352 TDZ for parameters used in initializer expressions.
  uint32_t paramIndex = uint32_t{0} - 1;
  for (auto &elem : ESTree::getParams(funcNode)) {
    ESTree::Node *param = &elem;
    ESTree::Node *init = nullptr;
    ++paramIndex;

    if (auto *rest = llvm::dyn_cast<ESTree::RestElementNode>(param)) {
      createLRef(rest->_argument, true)
          .emitStore(genBuiltinCall(
              BuiltinMethod::HermesBuiltin_copyRestArgs,
              Builder.getLiteralNumber(paramIndex)));
      break;
    }

    // Unpack the optional initialization.
    if (auto *assign = llvm::dyn_cast<ESTree::AssignmentPatternNode>(param)) {
      param = assign->_left;
      init = assign->_right;
    }

    Identifier formalParamName = llvm::isa<ESTree::IdentifierNode>(param)
        ? getNameFieldFromID(param)
        : genAnonymousLabelName("param");

    auto *formalParam = Builder.createParameter(newFunc, formalParamName);
    createLRef(param, true)
        .emitStore(
            emitOptionalInitialization(formalParam, init, formalParamName));
  }

  newFunc->setExpectedParamCountIncludingThis(
      countExpectedArgumentsIncludingThis(funcNode));
}

uint32_t ESTreeIRGen::countExpectedArgumentsIncludingThis(
    ESTree::FunctionLikeNode *funcNode) {
  // Start at 1 to account for "this".
  uint32_t count = 1;
  for (auto &param : ESTree::getParams(funcNode)) {
    if (llvm::isa<ESTree::AssignmentPatternNode>(param)) {
      // Found an initializer, stop counting expected arguments.
      break;
    }
    ++count;
  }
  return count;
}

void ESTreeIRGen::emitFunctionEpilogue(Value *returnValue) {
  if (returnValue) {
    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        Builder.getFunction()->getSourceRange()));
    Builder.createReturnInst(returnValue);
  }

  // Delete CreateArgumentsInst if it is unused.
  if (!curFunction()->createArgumentsInst->hasUsers())
    curFunction()->createArgumentsInst->eraseFromParent();

  curFunction()->function->clearStatementCount();
}

void ESTreeIRGen::genDummyFunction(Function *dummy) {
  IRBuilder builder{dummy};

  builder.createParameter(dummy, "this");
  BasicBlock *firstBlock = builder.createBasicBlock(dummy);
  builder.setInsertionBlock(firstBlock);
  builder.createUnreachableInst();
  builder.createReturnInst(builder.getLiteralUndefined());
}

/// Generate a function which immediately throws the specified SyntaxError
/// message.
Function *ESTreeIRGen::genSyntaxErrorFunction(
    Module *M,
    Identifier originalName,
    SMRange sourceRange,
    StringRef error) {
  IRBuilder builder{M};

  Function *function = builder.createFunction(
      originalName,
      Function::DefinitionKind::ES5Function,
      true,
      sourceRange,
      false);

  builder.createParameter(function, "this");
  BasicBlock *firstBlock = builder.createBasicBlock(function);
  builder.setInsertionBlock(firstBlock);

  builder.createThrowInst(builder.createCallInst(
      emitLoad(
          builder, builder.createGlobalObjectProperty("SyntaxError", false)),
      builder.getLiteralUndefined(),
      builder.getLiteralString(error)));

  return function;
}

} // namespace irgen
} // namespace hermes
