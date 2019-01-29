/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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
      oldContext_(irGen->functionContext),
      builderSaveState_(irGen->Builder),
      function(function),
      scope(irGen->nameTable_) {
  irGen->functionContext = this;

  // Initialize it to LiteraUndefined by default to avoid corner cases.
  this->capturedNewTarget = irGen->Builder.getLiteralUndefined();

  if (semInfo_) {
    // Allocate the label table. Each label definition will be encountered in
    // the AST before it is referenced (because of the nature of JavaScript), at
    // which point we will initialize the GotoLabel structure with basic blocks
    // targets.
    labels.resize(semInfo_->labels.size());
  }
}

FunctionContext::~FunctionContext() {
  irGen_->functionContext = oldContext_;
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
  DEBUG(dbgs() << "IRGen function \"" << functionName << "\".\n");

  auto *funcStorage = nameTable_.lookup(functionName);
  assert(
      funcStorage && "function declaration variable should have been hoisted");

  Function *newFunc = genFunctionLike(
      functionName,
      nullptr,
      Function::DefinitionKind::ES5Function,
      ESTree::isStrict(func->strictness),
      func,
      func->_params,
      func->_body,
      Mod->getContext().isLazyCompilation());

  // Store the newly created closure into a frame variable with the same name.
  auto *newClosure = Builder.createCreateFunctionInst(newFunc);

  emitStore(Builder, newClosure, funcStorage);
}

Value *ESTreeIRGen::genFunctionExpression(
    ESTree::FunctionExpressionNode *FE,
    Identifier nameHint) {
  DEBUG(
      dbgs() << "Creating anonymous closure. "
             << Builder.getInsertionBlock()->getParent()->getInternalName()
             << ".\n");

  NameTableScopeTy newScope(nameTable_);
  Variable *tempClosureVar = nullptr;

  Identifier originalNameIden = nameHint;
  if (FE->_id) {
    auto closureName = genAnonymousLabelName("closure");
    tempClosureVar = Builder.createVariable(
        functionContext->function->getFunctionScope(), closureName);

    // Insert the synthesized variable into the name table, so it can be
    // looked up internally as well.
    nameTable_.insertIntoScope(
        &functionContext->scope, tempClosureVar->getName(), tempClosureVar);

    // Alias the lexical name to the synthesized variable.
    originalNameIden = getNameFieldFromID(FE->_id);
    nameTable_.insert(originalNameIden, tempClosureVar);
  }

  Function *newFunc = genFunctionLike(
      originalNameIden,
      tempClosureVar,
      Function::DefinitionKind::ES5Function,
      ESTree::isStrict(FE->strictness),
      FE,
      FE->_params,
      FE->_body,
      Mod->getContext().isLazyCompilation());

  Value *closure = Builder.createCreateFunctionInst(newFunc);

  if (tempClosureVar)
    emitStore(Builder, closure, tempClosureVar);

  return closure;
}

Value *ESTreeIRGen::genArrowFunctionExpression(
    ESTree::ArrowFunctionExpressionNode *AF,
    Identifier nameHint) {
  DEBUG(
      dbgs() << "Creating arrow function. "
             << Builder.getInsertionBlock()->getParent()->getInternalName()
             << ".\n");

  Function *newFunc = genFunctionLike(
      nameHint,
      nullptr,
      Function::DefinitionKind::ES6Arrow,
      ESTree::isStrict(AF->strictness),
      AF,
      AF->_params,
      AF->_body,
      Mod->getContext().isLazyCompilation());

  return Builder.createCreateFunctionInst(newFunc);
}

#ifndef HERMESVM_LEAN
Function *ESTreeIRGen::genFunctionLike(
    Identifier originalName,
    Variable *lazyClosureAlias,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    ESTree::FunctionLikeNode *functionNode,
    const ESTree::NodeList &params,
    ESTree::Node *body,
    bool lazy) {
  assert(functionNode && "Function AST cannot be null");

  auto *newFunction = Builder.createFunction(
      originalName, definitionKind, strictMode, body->getSourceRange());
  newFunction->setLazyClosureAlias(lazyClosureAlias);

  if (auto *bodyBlock = dyn_cast<ESTree::BlockStatementNode>(body)) {
    if (bodyBlock->isLazyFunctionBody) {
      // Set the AST position and variable context so we can continue later.
      newFunction->setLazyScope(saveCurrentScope());
      auto &lazySource = newFunction->getLazySource();
      lazySource.bufferId = bodyBlock->bufferId;
      lazySource.nodeKind = functionNode->getKind();
      lazySource.functionRange = functionNode->getSourceRange();

      // Give the stub parameters so that we'll know the function's .length .
      Builder.createParameter(newFunction, "this");
      for (auto &param : params) {
        auto idenNode = cast<ESTree::IdentifierNode>(&param);
        Identifier paramName = getNameFieldFromID(idenNode);
        Builder.createParameter(newFunction, paramName);
      }

      return newFunction;
    }
  }

  FunctionContext newFunctionContext{
      this, newFunction, functionNode->getSemInfo()};

  doGenFunctionLike(
      newFunctionContext.function,
      functionNode,
      params,
      body,
      [this](ESTree::Node *body) {
        DEBUG(dbgs() << "IRGen function body.\n");
        // irgen the rest of the body.
        genStatement(body);

        Builder.setLocation(Builder.getFunction()->getSourceRange().End);
        Builder.createReturnInst(Builder.getLiteralUndefined());
      });

  return newFunctionContext.function;
}
#endif

void ESTreeIRGen::doGenFunctionLike(
    Function *NewFunc,
    ESTree::FunctionLikeNode *functionNode,
    const ESTree::NodeList &params,
    ESTree::Node *body,
    const std::function<void(ESTree::Node *body)> &genBodyCB) {
  auto *semInfo = functionContext->getSemInfo();
  DEBUG(
      dbgs() << "Hoisting "
             << (semInfo->decls.size() + semInfo->closures.size())
             << " variable decls.\n");

  Builder.setLocation(NewFunc->getSourceRange().Start);

  // Generate the code for closure:
  {
    // Create a new function with the right name.
    auto Entry = Builder.createBasicBlock(NewFunc);

    // Start pumping instructions into the entry basic block.
    Builder.setInsertionBlock(Entry);

    // Create variable declarations for each of the hoisted variables and
    // functions. Initialize only the variables to undefined.
    for (auto *vd : semInfo->decls) {
      auto res =
          declareVariableOrGlobalProperty(NewFunc, getNameFieldFromID(vd->_id));
      // If this is not a frame variable or it was already declared, skip.
      auto *var = dyn_cast<Variable>(res.first);
      if (!var || !res.second)
        continue;

      // Otherwise, initialize it to undefined.
      Builder.createStoreFrameInst(Builder.getLiteralUndefined(), var);
    }
    for (auto *fd : semInfo->closures)
      declareVariableOrGlobalProperty(NewFunc, getNameFieldFromID(fd->_id));

    // Construct the parameter list. Create function parameters and register
    // them in the scope.
    DEBUG(dbgs() << "IRGen function parameters.\n");
    // Always create the "this" parameter.
    Builder.createParameter(NewFunc, "this");
    for (auto &param : params) {
      auto idenNode = cast<ESTree::IdentifierNode>(&param);
      Identifier paramName = getNameFieldFromID(idenNode);
      DEBUG(dbgs() << "Adding parameter: " << paramName << "\n");

      auto *P = Builder.createParameter(NewFunc, paramName);
      auto *ParamStorage =
          Builder.createVariable(NewFunc->getFunctionScope(), paramName);

      // Register the storage for the parameter.
      nameTable_.insert(paramName, ParamStorage);

      // Store the parameter into the local scope.
      emitStore(Builder, P, ParamStorage);
    }

    // If we are not in an arrow function, and contain arrow functions,
    // capture our "this" into a new variable.
    if (functionContext->function->getDefinitionKind() !=
            Function::DefinitionKind::ES6Arrow &&
        functionContext->getSemInfo()->containsArrowFunctions) {
      // Capture the current "this" into a new variable. Note that if the
      // variable is never accessed it will be eliminated by the optimizer.
      DEBUG(
          dbgs() << "Capturing `this` of "
                 << functionContext->function->getInternalName() << "\n");

      auto *captureVar = Builder.createVariable(
          functionContext->function->getFunctionScope(),
          genAnonymousLabelName("this"));

      Builder.createStoreFrameInst(
          functionContext->function->getThisParameter(), captureVar);

      functionContext->capturedThis = captureVar;

      // Capture new.target into a new variable.
      captureVar = Builder.createVariable(
          functionContext->function->getFunctionScope(),
          genAnonymousLabelName("new.target"));

      Builder.createStoreFrameInst(
          Builder.createGetNewTargetInst(), captureVar);

      functionContext->capturedNewTarget = captureVar;
    } else if (
        functionContext->function->getDefinitionKind() ==
        Function::DefinitionKind::ES6Arrow) {
      assert(
          functionContext->getPreviousContext() &&
          "arrow function must have a previous context");
      assert(
          functionContext->getPreviousContext()->capturedThis &&
          "arrow function parent must have a captured this");
      functionContext->capturedThis =
          functionContext->getPreviousContext()->capturedThis;

      assert(
          functionContext->getPreviousContext()->capturedNewTarget &&
          "arrow function parent must have a captured new.target");
      functionContext->capturedNewTarget =
          functionContext->getPreviousContext()->capturedNewTarget;
    } else if (
        functionContext->function->getDefinitionKind() ==
        Function::DefinitionKind::ES6Method) {
      // new.target is always undefined in methods.
      functionContext->capturedNewTarget = Builder.getLiteralUndefined();
    }

    // Generate and initialize the code for the hoisted function declarations
    // before generating the rest of the body.
    for (auto funcDecl : semInfo->closures) {
      genFunctionDeclaration(funcDecl);
    }

    // Separate the next block, so we can append instructions to the entry block
    // in the future.
    auto nextBlock = Builder.createBasicBlock(NewFunc);
    functionContext->entryTerminator = Builder.createBranchInst(nextBlock);
    Builder.setInsertionBlock(nextBlock);

    genBodyCB(body);

    // If Entry is the only user of nextBlock, merge Entry and nextBlock, to
    // create less "noise" when optimization is disabled.
    if (nextBlock->getNumUsers() == 1 &&
        nextBlock->hasUser(functionContext->entryTerminator)) {
      DEBUG(dbgs() << "Merging entry and nextBlock.\n");

      // Move all instructions from nextBlock into Entry.
      while (nextBlock->begin() != nextBlock->end())
        nextBlock->begin()->moveBefore(functionContext->entryTerminator);

      // Now we can delete the original terminator;
      functionContext->entryTerminator->eraseFromParent();
      functionContext->entryTerminator = nullptr;

      // Delete the now empty next block
      nextBlock->eraseFromParent();
      nextBlock = nullptr;
    } else {
      DEBUG(dbgs() << "Could not merge entry and nextBlock.\n");
    }
  }

  NewFunc->clearStatementCount();
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
