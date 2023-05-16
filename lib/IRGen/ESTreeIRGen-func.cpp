/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

#include "llvh/ADT/SmallString.h"

namespace hermes {
namespace irgen {

//===----------------------------------------------------------------------===//
// FunctionContext

FunctionContext::FunctionContext(
    ESTreeIRGen *irGen,
    Function *function,
    sema::FunctionInfo *semInfo)
    : irGen_(irGen),
      semInfo_(semInfo),
      oldContext_(irGen->functionContext_),
      builderSaveState_(irGen->Builder),
      function(function) {
  irGen->functionContext_ = this;

  // Initialize it to LiteralUndefined by default to avoid corner cases.
  this->capturedNewTarget = irGen->Builder.getLiteralUndefined();

  if (semInfo_) {
    // Allocate the label table. Each label definition will be encountered in
    // the AST before it is referenced (because of the nature of JavaScript), at
    // which point we will initialize the GotoLabel structure with basic blocks
    // targets.
    labels_.resize(semInfo_->numLabels);
  }
}

FunctionContext::~FunctionContext() {
  irGen_->functionContext_ = oldContext_;
}

Identifier FunctionContext::genAnonymousLabelName(llvh::StringRef hint) {
  llvh::SmallString<16> buf;
  llvh::raw_svector_ostream nameBuilder{buf};
  nameBuilder << "?anon_" << anonymousLabelCounter++ << "_" << hint;
  return function->getContext().getIdentifier(nameBuilder.str());
}

//===----------------------------------------------------------------------===//
// ESTreeIRGen

void ESTreeIRGen::genFunctionDeclaration(
    ESTree::FunctionDeclarationNode *func) {
  if (func->_async && func->_generator) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        func->getSourceRange(), Twine("async generators are unsupported"));
    return;
  }

  // Find the name of the function.
  auto *id = llvh::cast<ESTree::IdentifierNode>(func->_id);
  Identifier functionName = Identifier::getFromPointer(id->_name);
  LLVM_DEBUG(llvh::dbgs() << "IRGen function \"" << functionName << "\".\n");

  Value *funcStorage = resolveIdentifier(id);
  assert(funcStorage && "Function declaration storage must have been resolved");

  Function *newFunc = func->_async ? genAsyncFunction(functionName, func)
      : func->_generator           ? genGeneratorFunction(functionName, func)
                                   : genES5Function(functionName, func);

  // Store the newly created closure into a frame variable with the same name.
  auto *newClosure = Builder.createCreateFunctionInst(newFunc);

  emitStore(newClosure, funcStorage, true);
}

Value *ESTreeIRGen::genFunctionExpression(
    ESTree::FunctionExpressionNode *FE,
    Identifier nameHint,
    ESTree::Node *superClassNode) {
  if (FE->_async && FE->_generator) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        FE->getSourceRange(), Twine("async generators are unsupported"));
    return Builder.getLiteralUndefined();
  }

  // This is the possibly empty scope containing the function expression name.
  emitScopeDeclarations(FE->getScope());

  auto *id = llvh::cast_or_null<ESTree::IdentifierNode>(FE->_id);
  Identifier originalNameIden =
      id ? Identifier::getFromPointer(id->_name) : nameHint;

  Function *newFunc = FE->_async ? genAsyncFunction(originalNameIden, FE)
      : FE->_generator           ? genGeneratorFunction(originalNameIden, FE)
                       : genES5Function(originalNameIden, FE, superClassNode);

  Value *closure = Builder.createCreateFunctionInst(newFunc);

  if (id)
    emitStore(closure, resolveIdentifier(id), true);

  return closure;
}

Value *ESTreeIRGen::genArrowFunctionExpression(
    ESTree::ArrowFunctionExpressionNode *AF,
    Identifier nameHint) {
  // Check if already compiled.
  if (Value *compiled = findCompiledEntity(AF)) {
    return Builder.createCreateFunctionInst(llvh::cast<Function>(compiled));
  }

  LLVM_DEBUG(
      llvh::dbgs()
      << "Creating arrow function. "
      << Builder.getInsertionBlock()->getParent()->getInternalName() << ".\n");

  if (AF->_async) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        AF->getSourceRange(), Twine("async functions are unsupported"));
    return Builder.getLiteralUndefined();
  }

  auto *newFunc = Builder.createFunction(
      nameHint,
      Function::DefinitionKind::ES6Arrow,
      ESTree::isStrict(AF->strictness),
      AF->getSemInfo()->customDirectives,
      AF->getSourceRange());

  auto compileFunc = [this,
                      newFunc,
                      AF,
                      capturedThis = curFunction()->capturedThis,
                      capturedNewTarget = curFunction()->capturedNewTarget,
                      capturedArguments = curFunction()->capturedArguments]() {
    FunctionContext newFunctionContext{this, newFunc, AF->getSemInfo()};

    // Propagate captured "this", "new.target" and "arguments" from parents.
    curFunction()->capturedThis = capturedThis;
    curFunction()->capturedNewTarget = capturedNewTarget;
    curFunction()->capturedArguments = capturedArguments;

    emitFunctionPrologue(
        AF,
        Builder.createBasicBlock(newFunc),
        InitES5CaptureState::No,
        DoEmitDeclarations::Yes);

    genStatement(AF->_body);
    emitFunctionEpilogue(Builder.getLiteralUndefined());
  };

  enqueueCompilation(AF, ExtraKey::Normal, newFunc, compileFunc);

  // Emit CreateFunctionInst after we have restored the builder state.
  return Builder.createCreateFunctionInst(newFunc);
}

Function *ESTreeIRGen::genES5Function(
    Identifier originalName,
    ESTree::FunctionLikeNode *functionNode,
    ESTree::Node *superClassNode,
    bool isGeneratorInnerFunction) {
  assert(functionNode && "Function AST cannot be null");

  // Check if already compiled.
  if (Value *compiled = findCompiledEntity(functionNode))
    return llvh::cast<Function>(compiled);

  auto *body = ESTree::getBlockStatement(functionNode);
  assert(body && "body of ES5 function cannot be null");

  Function *newFunction = isGeneratorInnerFunction
      ? Builder.createGeneratorInnerFunction(
            originalName,
            Function::DefinitionKind::ES5Function,
            ESTree::isStrict(functionNode->strictness),
            functionNode->getSourceRange(),
            /* insertBefore */ nullptr)
      : llvh::cast<Function>(Builder.createFunction(
            originalName,
            Function::DefinitionKind::ES5Function,
            ESTree::isStrict(functionNode->strictness),
            functionNode->getSemInfo()->customDirectives,
            functionNode->getSourceRange(),
            /* insertBefore */ nullptr));

  auto compileFunc = [this,
                      newFunction,
                      functionNode,
                      isGeneratorInnerFunction,
                      superClassNode,
                      body]() {
    FunctionContext newFunctionContext{
        this, newFunction, functionNode->getSemInfo()};
    newFunctionContext.superClassNode_ = superClassNode;

    if (isGeneratorInnerFunction) {
      // StartGeneratorInst
      // ResumeGeneratorInst
      // at the beginning of the function, to allow for the first .next()
      // call.
      auto *initGenBB = Builder.createBasicBlock(newFunction);
      Builder.setInsertionBlock(initGenBB);
      Builder.createStartGeneratorInst();
      auto *prologueBB = Builder.createBasicBlock(newFunction);
      auto *prologueResumeIsReturn = Builder.createAllocStackInst(
          genAnonymousLabelName("isReturn_prologue"), Type::createBoolean());
      genResumeGenerator(GenFinally::No, prologueResumeIsReturn, prologueBB);

      if (hasSimpleParams(functionNode)) {
        // If there are simple params, then we don't need an extra
        // yield/resume. They can simply be initialized on the first call to
        // `.next`.
        Builder.setInsertionBlock(prologueBB);
        emitFunctionPrologue(
            functionNode,
            prologueBB,
            InitES5CaptureState::Yes,
            DoEmitDeclarations::Yes);
      } else {
        // If there are non-simple params, then we must add a new
        // yield/resume. The `.next()` call will occur once in the outer
        // function, before the iterator is returned to the caller of the
        // `function*`.
        auto *entryPointBB = Builder.createBasicBlock(newFunction);
        auto *entryPointResumeIsReturn = Builder.createAllocStackInst(
            genAnonymousLabelName("isReturn_entry"), Type::createBoolean());

        // Initialize parameters.
        Builder.setInsertionBlock(prologueBB);
        emitFunctionPrologue(
            functionNode,
            prologueBB,
            InitES5CaptureState::Yes,
            DoEmitDeclarations::Yes);
        Builder.createSaveAndYieldInst(
            Builder.getLiteralUndefined(), entryPointBB);

        // Actual entry point of function from the caller's perspective.
        Builder.setInsertionBlock(entryPointBB);
        genResumeGenerator(
            GenFinally::No,
            entryPointResumeIsReturn,
            Builder.createBasicBlock(newFunction));
      }
    } else {
      emitFunctionPrologue(
          functionNode,
          Builder.createBasicBlock(newFunction),
          InitES5CaptureState::Yes,
          DoEmitDeclarations::Yes);
    }

    genStatement(body);
    emitFunctionEpilogue(Builder.getLiteralUndefined());
  };

  enqueueCompilation(functionNode, ExtraKey::Normal, newFunction, compileFunc);

  return newFunction;
}

Function *ESTreeIRGen::genGeneratorFunction(
    Identifier originalName,
    ESTree::FunctionLikeNode *functionNode) {
  assert(functionNode && "Function AST cannot be null");

  if (Value *compiled =
          findCompiledEntity(functionNode, ExtraKey::GeneratorOuter))
    return llvh::cast<Function>(compiled);

  if (!Builder.getModule()->getContext().isGeneratorEnabled()) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        functionNode->getSourceRange(), "generator compilation is disabled");
  }

  // Build the outer function which creates the generator.
  // Does not have an associated source range.
  auto *outerFn = Builder.createGeneratorFunction(
      originalName,
      Function::DefinitionKind::ES5Function,
      ESTree::isStrict(functionNode->strictness),
      functionNode->getSemInfo()->customDirectives,
      functionNode->getSourceRange(),
      /* insertBefore */ nullptr);

  auto compileFunc = [this, outerFn, functionNode, originalName]() {
    FunctionContext outerFnContext{this, outerFn, functionNode->getSemInfo()};

    // Build the inner function. This must be done in the outerFnContext
    // since it's lexically considered a child function.
    auto *innerFn = genES5Function(
        genAnonymousLabelName(originalName.isValid() ? originalName.str() : ""),
        functionNode,
        /* classNode */ nullptr,
        true);

    emitFunctionPrologue(
        functionNode,
        Builder.createBasicBlock(outerFn),
        InitES5CaptureState::Yes,
        DoEmitDeclarations::No);

    // Create a generator function, which will store the arguments.
    auto *gen = Builder.createCreateGeneratorInst(
        llvh::cast<GeneratorInnerFunction>(innerFn));

    if (!hasSimpleParams(functionNode)) {
      // If there are non-simple params, step the inner function once to
      // initialize them.
      Value *next = Builder.createLoadPropertyInst(gen, "next");
      Builder.createCallInst(next, gen, {});
    }

    emitFunctionEpilogue(gen);
  };

  enqueueCompilation(
      functionNode, ExtraKey::GeneratorOuter, outerFn, compileFunc);

  return outerFn;
}

Function *ESTreeIRGen::genAsyncFunction(
    Identifier originalName,
    ESTree::FunctionLikeNode *functionNode) {
  assert(functionNode && "Function AST cannot be null");

  if (auto *compiled = findCompiledEntity(functionNode, ExtraKey::AsyncOuter))
    return llvh::cast<Function>(compiled);

  if (!Builder.getModule()->getContext().isGeneratorEnabled()) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        functionNode->getSourceRange(),
        "async function compilation requires enabling generator");
  }

  auto *asyncFn = Builder.createAsyncFunction(
      originalName,
      Function::DefinitionKind::ES5Function,
      ESTree::isStrict(functionNode->strictness),
      functionNode->getSemInfo()->customDirectives,
      functionNode->getSourceRange(),
      /* insertBefore */ nullptr);

  auto compileFunc = [this, asyncFn, functionNode, originalName]() {
    FunctionContext asyncFnContext{this, asyncFn, functionNode->getSemInfo()};

    // Build the inner generator. This must be done in the outerFnContext
    // since it's lexically considered a child function.
    auto *gen = genGeneratorFunction(
        genAnonymousLabelName(originalName.isValid() ? originalName.str() : ""),
        functionNode);

    // The outer async function need not emit code for parameters.
    // It would simply delegate `arguments` object down to inner generator.
    // This avoid emitting code e.g. destructuring parameters twice.
    emitFunctionPrologue(
        functionNode,
        Builder.createBasicBlock(asyncFn),
        InitES5CaptureState::Yes,
        DoEmitDeclarations::No);

    auto *genClosure = Builder.createCreateFunctionInst(gen);
    auto *thisArg = curFunction()->jsParams[0];
    auto *argumentsList = curFunction()->createArgumentsInst;

    auto *spawnAsyncClosure = Builder.createGetBuiltinClosureInst(
        BuiltinMethod::HermesBuiltin_spawnAsync);

    auto *res = Builder.createCallInst(
        spawnAsyncClosure,
        Builder.getLiteralUndefined(),
        {genClosure, thisArg, argumentsList});

    emitFunctionEpilogue(res);
  };

  enqueueCompilation(functionNode, ExtraKey::AsyncOuter, asyncFn, compileFunc);

  return asyncFn;
}

void ESTreeIRGen::initCaptureStateInES5FunctionHelper() {
  // Capture "this", "new.target" and "arguments" if there are inner arrows.
  if (!curFunction()->getSemInfo()->containsArrowFunctions)
    return;

  auto *scope = curFunction()->function->getFunctionScope();

  // "this".
  auto *th = Builder.createVariable(
      scope, genAnonymousLabelName("this"), Type::createAnyType());
  curFunction()->capturedThis = th;
  emitStore(curFunction()->jsParams[0], th, true);

  // "new.target".
  curFunction()->capturedNewTarget = Builder.createVariable(
      scope,
      genAnonymousLabelName("new.target"),
      curFunction()->function->getNewTargetParam()->getType());
  emitStore(
      Builder.createGetNewTargetInst(
          curFunction()->function->getNewTargetParam()),
      curFunction()->capturedNewTarget,
      true);

  // "arguments".
  if (curFunction()->getSemInfo()->containsArrowFunctionsUsingArguments) {
    auto *args = Builder.createVariable(
        scope, genAnonymousLabelName("arguments"), Type::createObject());
    curFunction()->capturedArguments = args;
    emitStore(curFunction()->createArgumentsInst, args, true);
  }
}

void ESTreeIRGen::emitFunctionPrologue(
    ESTree::FunctionLikeNode *funcNode,
    BasicBlock *entry,
    InitES5CaptureState doInitES5CaptureState,
    DoEmitDeclarations doEmitDeclarations) {
  auto *newFunc = curFunction()->function;
  auto *semInfo = curFunction()->getSemInfo();

  Builder.setLocation(newFunc->getSourceRange().Start);

  // Start pumping instructions into the entry basic block.
  Builder.setInsertionBlock(entry);

  // Always insert a CreateArgumentsInst. We will delete it later if it is
  // unused.
  curFunction()->createArgumentsInst = newFunc->isStrictMode()
      ? (CreateArgumentsInst *)Builder.createCreateArgumentsStrictInst()
      : Builder.createCreateArgumentsLooseInst();

  // If "arguments" is declared in the current function, bind it to its value.
  if (semInfo->argumentsDecl.hasValue()) {
    setDeclData(
        semInfo->argumentsDecl.getValue(), curFunction()->createArgumentsInst);
  }

  // Always create the "this" parameter. It needs to be created before we
  // initialized the ES5 capture state.
  JSDynamicParam *thisParam = newFunc->addJSThisParam();

  // Save the "this" parameter. We will delete it later if unused.
  // In strict mode just use param 0 directly. In non-strict, we must coerce
  // it to an object.
  {
    Instruction *thisVal = Builder.createLoadParamInst(thisParam);
    assert(
        curFunction()->jsParams.empty() &&
        "jsParams must be empty in new function");
    curFunction()->jsParams.push_back(
        newFunc->isStrictMode() ? thisVal
                                : Builder.createCoerceThisNSInst(thisVal));
  }

  if (doInitES5CaptureState != InitES5CaptureState::No)
    initCaptureStateInES5FunctionHelper();

  // Construct the parameter list. Create function parameters and register
  // them in the scope.
  newFunc->setExpectedParamCountIncludingThis(
      countExpectedArgumentsIncludingThis(funcNode));

  if (doEmitDeclarations == DoEmitDeclarations::No)
    return;

  emitParameters(funcNode);
  emitScopeDeclarations(semInfo->getFunctionScope());

  // Generate the code for import declarations before generating the rest of the
  // body.
  for (auto importDecl : semInfo->imports) {
    genImportDeclaration(importDecl);
  }
}

void ESTreeIRGen::emitScopeDeclarations(sema::LexicalScope *scope) {
  if (!scope)
    return;

  Function *func = curFunction()->function;

  for (sema::Decl *decl : scope->decls) {
    Variable *var = nullptr;
    bool init = false;
    switch (decl->kind) {
      case sema::Decl::Kind::Let:
      case sema::Decl::Kind::Const:
      case sema::Decl::Kind::Class:
        assert(
            ((curFunction()->debugAllowRecompileCounter != 0) ^
             (decl->customData == nullptr)) &&
            "customData can be bound only if recompiling AST");

        if (!decl->customData) {
          bool tdz = Mod->getContext().getCodeGenerationSettings().enableTDZ;
          var = Builder.createVariable(
              func->getFunctionScope(),
              decl->name,
              tdz ? Type::createAnyOrEmpty() : Type::createAnyType());
          var->setObeysTDZ(tdz);
          setDeclData(decl, var);
        } else {
          var = llvh::cast<Variable>(getDeclData(decl));
        }
        init = true;
        break;

      case sema::Decl::Kind::Var:
        // 'arguments' must have already been bound in the function prologue.
        if (decl->special == sema::Decl::Special::Arguments) {
          assert(
              decl->customData &&
              "'arguments', if it exists, must be bound in the function prologue");
          continue;
        }
      case sema::Decl::Kind::Import:
      case sema::Decl::Kind::ES5Catch:
      case sema::Decl::Kind::FunctionExprName:
      case sema::Decl::Kind::ClassExprName:
      case sema::Decl::Kind::ScopedFunction:
        // NOTE: we are overwriting the decl's customData, even if it is already
        // set. Ordinarily we shouldn't be evaluating the same declarations
        // twice, except when we are compiling the body of a "finally" handler.
        assert(
            ((curFunction()->debugAllowRecompileCounter != 0) ^
             (decl->customData == nullptr)) &&
            "customData can be bound only if recompiling AST");

        if (!decl->customData) {
          var = Builder.createVariable(
              func->getFunctionScope(), decl->name, Type::createAnyType());
          setDeclData(decl, var);
        } else {
          var = llvh::cast<Variable>(getDeclData(decl));
        }
        // Var declarations must be initialized to undefined at the beginning
        // of the scope.
        //
        // Loose mode scoped function decls also need to be initialized. In
        // strict mode, the scoped function is declared and created in the
        // same scope, so there is no need to initialize before that. In loose
        // mode however, the declaration may be promoted to function scope while
        // the function creation happens later in the scope. The variable needs
        // to be initialized meanwhile.
        init = decl->kind == sema::Decl::Kind::Var ||
            (decl->kind == sema::Decl::Kind::ScopedFunction &&
             !curFunction()->function->isStrictMode());
        break;

      case sema::Decl::Kind::Parameter:
        // Skip parameters, they are handled separately.
        continue;

      case sema::Decl::Kind::GlobalProperty:
      case sema::Decl::Kind::UndeclaredGlobalProperty: {
        assert(
            ((curFunction()->debugAllowRecompileCounter != 0) ^
             (decl->customData == nullptr)) &&
            "customData can be bound only if recompiling AST");

        if (!decl->customData) {
          bool declared = decl->kind == sema::Decl::Kind::GlobalProperty;
          auto *prop = Builder.createGlobalObjectProperty(decl->name, declared);
          setDeclData(decl, prop);
          if (declared)
            Builder.createDeclareGlobalVarInst(prop->getName());
        }
      } break;
    }

    if (init) {
      assert(var);
      Builder.createStoreFrameInst(
          var->getObeysTDZ() ? (Literal *)Builder.getLiteralEmpty()
                             : (Literal *)Builder.getLiteralUndefined(),
          var);
    }
  }

  // Generate and initialize the code for the hoisted function declarations
  // before generating the rest of the body.
  for (auto funcDecl : scope->hoistedFunctions) {
    genFunctionDeclaration(funcDecl);
  }
}

void ESTreeIRGen::emitParameters(ESTree::FunctionLikeNode *funcNode) {
  auto *newFunc = curFunction()->function;
  sema::FunctionInfo *semInfo = funcNode->getSemInfo();

  LLVM_DEBUG(llvh::dbgs() << "IRGen function parameters.\n");

  // Create a variable for every parameter.
  for (sema::Decl *decl : semInfo->getFunctionScope()->decls) {
    if (decl->kind != sema::Decl::Kind::Parameter)
      break;

    LLVM_DEBUG(llvh::dbgs() << "Adding parameter: " << decl->name << "\n");

    // If not simple parameter list, enable TDZ and init every param.
    bool tdz = !semInfo->simpleParameterList &&
        Mod->getContext().getCodeGenerationSettings().enableTDZ;
    Variable *var = Builder.createVariable(
        newFunc->getFunctionScope(),
        decl->name,
        tdz ? Type::createAnyOrEmpty() : Type::createAnyType());
    setDeclData(decl, var);

    // If not simple parameter list, enable TDZ and init every param.
    if (!semInfo->simpleParameterList) {
      var->setObeysTDZ(tdz);
      Builder.createStoreFrameInst(
          tdz ? (Literal *)Builder.getLiteralEmpty()
              : (Literal *)Builder.getLiteralUndefined(),
          var);
    }
  }

  uint32_t paramIndex = uint32_t{0} - 1;
  for (auto &elem : ESTree::getParams(funcNode)) {
    ESTree::Node *param = &elem;
    ESTree::Node *init = nullptr;
    ++paramIndex;

    if (auto *rest = llvh::dyn_cast<ESTree::RestElementNode>(param)) {
      createLRef(rest->_argument, true)
          .emitStore(genBuiltinCall(
              BuiltinMethod::HermesBuiltin_copyRestArgs,
              Builder.getLiteralNumber(paramIndex)));
      break;
    }

    // Unpack the optional initialization.
    if (auto *assign = llvh::dyn_cast<ESTree::AssignmentPatternNode>(param)) {
      param = assign->_left;
      init = assign->_right;
    }

    Identifier formalParamName = llvh::isa<ESTree::IdentifierNode>(param)
        ? getNameFieldFromID(param)
        : genAnonymousLabelName("param");

    size_t jsParamIndex = newFunc->getJSDynamicParams().size();
    if (jsParamIndex > UINT32_MAX) {
      Mod->getContext().getSourceErrorManager().error(
          param->getSourceRange(), "too many parameters");
      break;
    }
    Instruction *formalParam = Builder.createLoadParamInst(
        newFunc->addJSDynamicParam(formalParamName));
    curFunction()->jsParams.push_back(formalParam);
    createLRef(param, true)
        .emitStore(
            emitOptionalInitialization(formalParam, init, formalParamName));
  }
}

uint32_t ESTreeIRGen::countExpectedArgumentsIncludingThis(
    ESTree::FunctionLikeNode *funcNode) {
  // Start at 1 to account for "this".
  uint32_t count = 1;
  for (auto &param : ESTree::getParams(funcNode)) {
    if (llvh::isa<ESTree::AssignmentPatternNode>(param)) {
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

  // Delete the load of "this" if unused.
  if (!curFunction()->jsParams.empty()) {
    Instruction *I = curFunction()->jsParams[0];
    if (!I->hasUsers()) {
      // If the instruction is CoerceThisNSInst, we may have to delete its
      // operand too.
      Instruction *load = nullptr;
      if (auto *CT = llvh::dyn_cast<CoerceThisNSInst>(I)) {
        load = llvh::dyn_cast<Instruction>(CT->getSingleOperand());
      }
      I->eraseFromParent();
      if (load && !load->hasUsers())
        load->eraseFromParent();
    }
  }

  curFunction()->function->clearStatementCount();
}

void ESTreeIRGen::genDummyFunction(Function *dummy) {
  IRBuilder builder{dummy};

  dummy->addJSThisParam();
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
    llvh::StringRef error) {
  IRBuilder::SaveRestore saveRestore(Builder);

  Function *function = Builder.createFunction(
      originalName,
      Function::DefinitionKind::ES5Function,
      true,
      CustomDirectives{
          .sourceVisibility = SourceVisibility::Sensitive,
          .alwaysInline = false,
      },
      sourceRange);

  function->addJSThisParam();
  BasicBlock *firstBlock = Builder.createBasicBlock(function);
  Builder.setInsertionBlock(firstBlock);

  Builder.createThrowInst(Builder.createCallInst(
      emitLoad(Builder.createGlobalObjectProperty("SyntaxError", false), false),
      Builder.getLiteralUndefined(),
      Builder.getLiteralString(error)));

  return function;
}

} // namespace irgen
} // namespace hermes
