/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

#include "llvh/ADT/SetVector.h"
#include "llvh/ADT/StringSet.h"
#include "llvh/Support/Debug.h"
#include "llvh/Support/SaveAndRestore.h"

namespace hermes {
namespace irgen {

//===----------------------------------------------------------------------===//
// Free standing helpers.

/// \returns true if \p node is a constant expression.
bool isConstantExpr(ESTree::Node *node) {
  switch (node->getKind()) {
    case ESTree::NodeKind::StringLiteral:
    case ESTree::NodeKind::NumericLiteral:
    case ESTree::NodeKind::NullLiteral:
    case ESTree::NodeKind::BooleanLiteral:
      return true;
    default:
      return false;
  }
}

//===----------------------------------------------------------------------===//
// LReference

IRBuilder &LReference::getBuilder() {
  return irgen_->Builder;
}

Value *LReference::emitLoad() {
  auto &builder = getBuilder();
  IRBuilder::ScopedLocationChange slc(builder, loadLoc_);

  switch (kind_) {
    case Kind::Empty:
      assert(false && "empty cannot be loaded");
      return builder.getLiteralUndefined();
    case Kind::Member:
      return irgen_
          ->emitMemberLoad(
              llvh::cast<ESTree::MemberExpressionNode>(ast_), base_, property_)
          .result;
    case Kind::VarOrGlobal:
      return irgen_->emitLoad(base_, false);
    case Kind::Destructuring:
      assert(false && "destructuring cannot be loaded");
      return builder.getLiteralUndefined();
    case Kind::Error:
      return builder.getLiteralUndefined();
  }

  llvm_unreachable("invalid LReference kind");
}

void LReference::emitStore(Value *value) {
  switch (kind_) {
    case Kind::Empty:
      return;
    case Kind::Member:
      return irgen_->emitMemberStore(
          llvh::cast<ESTree::MemberExpressionNode>(ast_),
          value,
          base_,
          property_);
    case Kind::VarOrGlobal:
      irgen_->emitStore(value, base_, declInit_);
      return;
    case Kind::Error:
      return;
    case Kind::Destructuring:
      return irgen_->emitDestructuringAssignment(
          declInit_, destructuringTarget_, value);
  }

  llvm_unreachable("invalid LReference kind");
}

bool LReference::canStoreWithoutSideEffects() const {
  return kind_ == Kind::VarOrGlobal && llvh::isa<Variable>(base_);
}

Variable *LReference::castAsVariable() const {
  return kind_ == Kind::VarOrGlobal ? dyn_cast_or_null<Variable>(base_)
                                    : nullptr;
}
GlobalObjectProperty *LReference::castAsGlobalObjectProperty() const {
  return kind_ == Kind::VarOrGlobal
      ? dyn_cast_or_null<GlobalObjectProperty>(base_)
      : nullptr;
}

//===----------------------------------------------------------------------===//
// ESTreeIRGen

ESTreeIRGen::ESTreeIRGen(
    Module *M,
    sema::SemContext &semCtx,
    flow::FlowContext &flowContext,
    ESTree::Node *root)
    : Mod(M),
      semCtx_(semCtx),
      kw_(semCtx.kw),
      flowContext_(flowContext),
      Root(root),
      Builder(Mod),
      identDefaultExport_(Builder.createIdentifier("?default")) {}

void ESTreeIRGen::doIt() {
  LLVM_DEBUG(llvh::dbgs() << "Processing top level program.\n");

  ESTree::ProgramNode *Program;

  Program = llvh::dyn_cast<ESTree::ProgramNode>(Root);

  if (!Program) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        SMLoc{}, "missing 'Program' AST node");
    return;
  }

  LLVM_DEBUG(llvh::dbgs() << "Found Program decl.\n");

  // The function which will "execute" the module.
  Function *topLevelFunction;

  topLevelFunction = Builder.createTopLevelFunction(
      ESTree::isStrict(Program->strictness),
      Program->getSemInfo()->customDirectives,
      Program->getSourceRange());

  // Function context for topLevelFunction.
  FunctionContext topLevelFunctionContext{
      this, topLevelFunction, Program->getSemInfo()};

  emitFunctionPrologue(
      Program,
      Builder.createBasicBlock(topLevelFunction),
      InitES5CaptureState::Yes,
      DoEmitDeclarations::Yes);

  Value *retVal;
  {
    // Allocate the return register, initialize it to undefined.
    curFunction()->globalReturnRegister = Builder.createAllocStackInst(
        genAnonymousLabelName("ret"), Type::createAnyType());
    Builder.createStoreStackInst(
        Builder.getLiteralUndefined(), curFunction()->globalReturnRegister);

    genBody(Program->_body);

    // Terminate the top-level scope with a return statement.
    retVal = Builder.createLoadStackInst(curFunction()->globalReturnRegister);
  }

  emitFunctionEpilogue(retVal);

  drainCompilationQueue();
}

void ESTreeIRGen::doCJSModule(
    sema::SemContext &semContext,
    uint32_t segmentID,
    uint32_t id,
    llvh::StringRef filename) {
  assert(Root && "no root in ESTreeIRGen");
  auto *func = cast<ESTree::FunctionExpressionNode>(Root);

  // Take care of the additions to the global scope that this module could
  // have done. A module can only add ambient global properties. Look for new
  // ones (customData == nullptr) and declare them.
  for (sema::Decl *decl : semContext.getGlobalScope()->decls) {
    if (decl->kind == sema::Decl::Kind::UndeclaredGlobalProperty &&
        !decl->customData) {
      setDeclData(decl, Builder.createGlobalObjectProperty(decl->name, false));
    }
  }

  Identifier functionName = Builder.createIdentifier("cjs_module");
  Function *newFunc = genES5Function(functionName, func);

  drainCompilationQueue();

  Builder.getModule()->addCJSModule(
      segmentID, id, Builder.createIdentifier(filename), newFunc);
}

Value *ESTreeIRGen::genMemberExpressionProperty(
    ESTree::MemberExpressionLikeNode *Mem) {
  // If computed is true, the node corresponds to a computed (a[b]) member
  // lookup and '_property' is an Expression. Otherwise, the node
  // corresponds to a static (a.b) member lookup and '_property' is an
  // Identifier.
  // Details of the computed field are available here:
  // https://github.com/estree/estree/blob/master/spec.md#memberexpression

  if (getComputed(Mem)) {
    return genExpression(getProperty(Mem));
  }

  // Arrays and objects may be accessed with integer indices.
  if (auto N = llvh::dyn_cast<ESTree::NumericLiteralNode>(getProperty(Mem))) {
    return Builder.getLiteralNumber(N->_value);
  }

  // ESTree encodes property access as MemberExpression -> Identifier.
  auto Id = cast<ESTree::IdentifierNode>(getProperty(Mem));

  Identifier fieldName = getNameFieldFromID(Id);
  LLVM_DEBUG(
      llvh::dbgs() << "Emitting direct label access to field '" << fieldName
                   << "'\n");
  return Builder.getLiteralString(fieldName);
}

bool ESTreeIRGen::canCreateLRefWithoutSideEffects(
    hermes::ESTree::Node *target) {
  // Check for an identifier bound to an existing local variable.
  if (auto *iden = llvh::dyn_cast<ESTree::IdentifierNode>(target)) {
    return llvh::isa<Variable>(resolveIdentifier(iden));
  }

  return false;
}

LReference ESTreeIRGen::createLRef(ESTree::Node *node, bool declInit) {
  SMLoc sourceLoc = node->getDebugLoc();
  IRBuilder::ScopedLocationChange slc(Builder, sourceLoc);

  if (llvh::isa<ESTree::EmptyNode>(node)) {
    LLVM_DEBUG(llvh::dbgs() << "Creating an LRef for EmptyNode.\n");
    return LReference(
        LReference::Kind::Empty,
        this,
        false,
        nullptr,
        nullptr,
        nullptr,
        sourceLoc);
  }

  /// Create lref for member expression (ex: o.f).
  if (auto *ME = llvh::dyn_cast<ESTree::MemberExpressionNode>(node)) {
    LLVM_DEBUG(llvh::dbgs() << "Creating an LRef for member expression.\n");
    Value *obj = genExpression(ME->_object);
    Value *prop = genMemberExpressionProperty(ME);
    return LReference(
        LReference::Kind::Member, this, false, ME, obj, prop, sourceLoc);
  }

  /// Create lref for identifiers  (ex: a).
  if (auto *iden = llvh::dyn_cast<ESTree::IdentifierNode>(node)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Creating an LRef for identifier \""
                     << getNameFieldFromID(iden) << "\"\n");
    auto *var = resolveIdentifier(iden);
    return LReference(
        LReference::Kind::VarOrGlobal,
        this,
        declInit,
        nullptr,
        var,
        nullptr,
        sourceLoc);
  }

  /// Create lref for variable decls (ex: var a).
  if (auto *V = llvh::dyn_cast<ESTree::VariableDeclarationNode>(node)) {
    LLVM_DEBUG(llvh::dbgs() << "Creating an LRef for variable declaration.\n");

    assert(V->_declarations.size() == 1 && "Malformed variable declaration");
    auto *decl =
        cast<ESTree::VariableDeclaratorNode>(&V->_declarations.front());

    return createLRef(decl->_id, true);
  }

  // Destructuring assignment.
  if (auto *pat = llvh::dyn_cast<ESTree::PatternNode>(node)) {
    return LReference(this, declInit, pat);
  }

  Builder.getModule()->getContext().getSourceErrorManager().error(
      node->getSourceRange(), "unsupported assignment target");

  return LReference(
      LReference::Kind::Error,
      this,
      false,
      nullptr,
      nullptr,
      nullptr,
      sourceLoc);
}

Value *ESTreeIRGen::genHermesInternalCall(
    llvh::StringRef name,
    Value *thisValue,
    ArrayRef<Value *> args) {
  return Builder.createCallInst(
      Builder.createLoadPropertyInst(
          Builder.createTryLoadGlobalPropertyInst("HermesInternal"), name),
      /* newTarget */ Builder.getLiteralUndefined(),
      thisValue,
      args);
}

Value *ESTreeIRGen::genBuiltinCall(
    hermes::BuiltinMethod::Enum builtinIndex,
    ArrayRef<Value *> args) {
  return Builder.createCallBuiltinInst(builtinIndex, args);
}

void ESTreeIRGen::emitEnsureObject(Value *value, llvh::StringRef message) {
  // TODO: use "thisArg" when builtins get fixed to support it.
  genBuiltinCall(
      BuiltinMethod::HermesBuiltin_ensureObject,
      {value, Builder.getLiteralString(message)});
}

Value *ESTreeIRGen::emitIteratorSymbol() {
  return Builder.createLoadPropertyInst(
      Builder.createGetBuiltinClosureInst(BuiltinMethod::globalThis_Symbol),
      "iterator");
}

ESTreeIRGen::IteratorRecordSlow ESTreeIRGen::emitGetIteratorSlow(Value *obj) {
  auto *method = Builder.createLoadPropertyInst(obj, emitIteratorSymbol());
  auto *iterator = Builder.createCallInst(
      method, /* newTarget */ Builder.getLiteralUndefined(), obj, {});

  emitEnsureObject(iterator, "iterator is not an object");
  auto *nextMethod = Builder.createLoadPropertyInst(iterator, "next");

  return {iterator, nextMethod};
}

Value *ESTreeIRGen::emitIteratorNextSlow(IteratorRecordSlow iteratorRecord) {
  auto *nextResult = Builder.createCallInst(
      iteratorRecord.nextMethod,
      /* newTarget */ Builder.getLiteralUndefined(),
      iteratorRecord.iterator,
      {});
  emitEnsureObject(nextResult, "iterator.next() did not return an object");
  return nextResult;
}

Value *ESTreeIRGen::emitIteratorCompleteSlow(Value *iterResult) {
  return Builder.createLoadPropertyInst(iterResult, "done");
}

Value *ESTreeIRGen::emitIteratorValueSlow(Value *iterResult) {
  return Builder.createLoadPropertyInst(iterResult, "value");
}

void ESTreeIRGen::emitIteratorCloseSlow(
    hermes::irgen::ESTreeIRGen::IteratorRecordSlow iteratorRecord,
    bool ignoreInnerException) {
  auto *haveReturn = Builder.createBasicBlock(Builder.getFunction());
  auto *noReturn = Builder.createBasicBlock(Builder.getFunction());

  auto *returnMethod = genBuiltinCall(
      BuiltinMethod::HermesBuiltin_getMethod,
      {iteratorRecord.iterator, Builder.getLiteralString("return")});
  Builder.createCompareBranchInst(
      returnMethod,
      Builder.getLiteralUndefined(),
      ValueKind::CmpBrStrictlyEqualInstKind,
      noReturn,
      haveReturn);

  Builder.setInsertionBlock(haveReturn);
  if (ignoreInnerException) {
    emitTryCatchScaffolding(
        noReturn,
        // emitBody.
        [this, returnMethod, &iteratorRecord]() {
          Builder.createCallInst(
              returnMethod,
              /* newTarget */ Builder.getLiteralUndefined(),
              iteratorRecord.iterator,
              {});
        },
        // emitNormalCleanup.
        []() {},
        // emitHandler.
        [this](BasicBlock *nextBlock) {
          // We need to catch the exception, even if we don't used it.
          Builder.createCatchInst();
          Builder.createBranchInst(nextBlock);
        });
  } else {
    auto *innerResult = Builder.createCallInst(
        returnMethod,
        /* newTarget */ Builder.getLiteralUndefined(),
        iteratorRecord.iterator,
        {});
    emitEnsureObject(innerResult, "iterator.return() did not return an object");
    Builder.createBranchInst(noReturn);
  }

  Builder.setInsertionBlock(noReturn);
}

ESTreeIRGen::IteratorRecord ESTreeIRGen::emitGetIterator(Value *obj) {
  // Each of these will be modified by "next", so we use a stack storage.
  auto *iterStorage = Builder.createAllocStackInst(
      genAnonymousLabelName("iter"), Type::createAnyType());
  auto *sourceOrNext = Builder.createAllocStackInst(
      genAnonymousLabelName("sourceOrNext"), Type::createAnyType());
  Builder.createStoreStackInst(obj, sourceOrNext);
  auto *iter = Builder.createIteratorBeginInst(sourceOrNext);
  Builder.createStoreStackInst(iter, iterStorage);
  return IteratorRecord{iterStorage, sourceOrNext};
}

void ESTreeIRGen::emitDestructuringAssignment(
    bool declInit,
    ESTree::PatternNode *target,
    Value *source) {
  if (auto *APN = llvh::dyn_cast<ESTree::ArrayPatternNode>(target))
    return emitDestructuringArray(declInit, APN, source);
  else if (auto *OPN = llvh::dyn_cast<ESTree::ObjectPatternNode>(target))
    return emitDestructuringObject(declInit, OPN, source);
  else {
    Mod->getContext().getSourceErrorManager().error(
        target->getSourceRange(), "unsupported destructuring target");
  }
}

void ESTreeIRGen::emitDestructuringArray(
    bool declInit,
    ESTree::ArrayPatternNode *targetPat,
    Value *source) {
  if (auto *tuple = llvh::dyn_cast<flow::TupleType>(
          flowContext_.getNodeTypeOrAny(targetPat)->info)) {
    emitDestructuringTypedTuple(declInit, targetPat, tuple, source);
    return;
  }

  const IteratorRecord iteratorRecord = emitGetIterator(source);

  /// iteratorDone = undefined.
  auto *iteratorDone = Builder.createAllocStackInst(
      genAnonymousLabelName("iterDone"), Type::createAnyType());
  Builder.createStoreStackInst(Builder.getLiteralUndefined(), iteratorDone);

  auto *value = Builder.createAllocStackInst(
      genAnonymousLabelName("iterValue"), Type::createAnyType());

  SharedExceptionHandler handler{};
  handler.exc = Builder.createAllocStackInst(
      genAnonymousLabelName("exc"), Type::createAnyType());
  // All exception handlers branch to this block.
  handler.exceptionBlock = Builder.createBasicBlock(Builder.getFunction());

  bool first = true;
  bool emittedRest = false;
  // The LReference created in the previous iteration of the destructuring
  // loop. We need it because we want to put the previous store and the creation
  // of the next LReference under one try block.
  llvh::Optional<LReference> lref;

  /// If the previous LReference is valid and non-empty, store "value" into
  /// it and reset the LReference.
  auto storePreviousValue = [&lref, &handler, this, value]() {
    if (lref && !lref->isEmpty()) {
      if (lref->canStoreWithoutSideEffects()) {
        lref->emitStore(Builder.createLoadStackInst(value));
      } else {
        // If we can't store without side effects, wrap the store in try/catch.
        emitTryWithSharedHandler(&handler, [this, &lref, value]() {
          lref->emitStore(Builder.createLoadStackInst(value));
        });
      }
      lref.reset();
    }
  };

  for (auto &elem : targetPat->_elements) {
    ESTree::Node *target = &elem;
    ESTree::Node *init = nullptr;

    if (auto *rest = llvh::dyn_cast<ESTree::RestElementNode>(target)) {
      storePreviousValue();
      emitRestElement(declInit, rest, iteratorRecord, iteratorDone, &handler);
      emittedRest = true;
      break;
    }

    // If we have an initializer, unwrap it.
    if (auto *assign = llvh::dyn_cast<ESTree::AssignmentPatternNode>(target)) {
      target = assign->_left;
      init = assign->_right;
    }

    // Can we create the new LReference without side effects and avoid a
    // try/catch. The complexity comes from having to check whether the last
    // LReference also can avoid a try/catch or not.
    if (canCreateLRefWithoutSideEffects(target)) {
      // We don't need a try/catch, but last lref might. Just let the routine
      // do the right thing.
      storePreviousValue();
      lref = createLRef(target, declInit);
    } else {
      // We need a try/catch, but last lref might not. If it doesn't, emit it
      // directly and clear it, so we won't do anything inside our try/catch.
      if (lref && lref->canStoreWithoutSideEffects()) {
        lref->emitStore(Builder.createLoadStackInst(value));
        lref.reset();
      }
      emitTryWithSharedHandler(
          &handler, [this, &lref, value, target, declInit]() {
            // Store the previous value, if we have one.
            if (lref && !lref->isEmpty())
              lref->emitStore(Builder.createLoadStackInst(value));
            lref = createLRef(target, declInit);
          });
    }

    // Pseudocode of the algorithm for a step:
    //
    //   value = undefined;
    //   if (iteratorDone) goto nextBlock
    // notDoneBlock:
    //   stepResult = IteratorNext(iteratorRecord)
    //   stepDone = IteratorComplete(stepResult)
    //   iteratorDone = stepDone
    //   if (stepDone) goto nextBlock
    // newValueBlock:
    //   value = IteratorValue(stepResult)
    // nextBlock:
    //   if (value !== undefined) goto storeBlock    [if initializer present]
    //   value = initializer                         [if initializer present]
    // storeBlock:
    //   lref.emitStore(value)

    auto *notDoneBlock = Builder.createBasicBlock(Builder.getFunction());
    auto *newValueBlock = Builder.createBasicBlock(Builder.getFunction());
    auto *nextBlock = Builder.createBasicBlock(Builder.getFunction());
    auto *getDefaultBlock =
        init ? Builder.createBasicBlock(Builder.getFunction()) : nullptr;
    auto *storeBlock =
        init ? Builder.createBasicBlock(Builder.getFunction()) : nullptr;

    Builder.createStoreStackInst(Builder.getLiteralUndefined(), value);

    // In the first iteration we know that "done" is false.
    if (first) {
      first = false;
      Builder.createBranchInst(notDoneBlock);
    } else {
      Builder.createCondBranchInst(
          Builder.createLoadStackInst(iteratorDone), nextBlock, notDoneBlock);
    }

    // notDoneBlock:
    Builder.setInsertionBlock(notDoneBlock);
    auto *stepValue = emitIteratorNext(iteratorRecord);
    auto *stepDone = emitIteratorComplete(iteratorRecord);
    Builder.createStoreStackInst(stepDone, iteratorDone);
    Builder.createCondBranchInst(
        stepDone, init ? getDefaultBlock : nextBlock, newValueBlock);

    // newValueBlock:
    Builder.setInsertionBlock(newValueBlock);
    Builder.createStoreStackInst(stepValue, value);
    Builder.createBranchInst(nextBlock);

    // nextBlock:
    Builder.setInsertionBlock(nextBlock);

    // NOTE: we can't use emitOptionalInitializationHere() because we want to
    // be able to jump directly to getDefaultBlock.
    if (init) {
      //    if (value !== undefined) goto storeBlock    [if initializer present]
      //    value = initializer                         [if initializer present]
      //  storeBlock:
      Builder.createCondBranchInst(
          Builder.createBinaryOperatorInst(
              Builder.createLoadStackInst(value),
              Builder.getLiteralUndefined(),
              ValueKind::BinaryStrictlyNotEqualInstKind),
          storeBlock,
          getDefaultBlock);

      Identifier nameHint = llvh::isa<ESTree::IdentifierNode>(target)
          ? getNameFieldFromID(target)
          : Identifier{};

      // getDefaultBlock:
      Builder.setInsertionBlock(getDefaultBlock);
      Builder.createStoreStackInst(genExpression(init, nameHint), value);
      Builder.createBranchInst(storeBlock);

      // storeBlock:
      Builder.setInsertionBlock(storeBlock);
    }
  }

  storePreviousValue();

  // If in the end the iterator is not done, close it. We only need to do
  // that if we didn't end with a rest element because it would have exhausted
  // the iterator.
  if (!emittedRest) {
    auto *notDoneBlock = Builder.createBasicBlock(Builder.getFunction());
    auto *doneBlock = Builder.createBasicBlock(Builder.getFunction());
    Builder.createCondBranchInst(
        Builder.createLoadStackInst(iteratorDone), doneBlock, notDoneBlock);

    Builder.setInsertionBlock(notDoneBlock);
    emitIteratorClose(iteratorRecord, false);
    Builder.createBranchInst(doneBlock);

    Builder.setInsertionBlock(doneBlock);
  }

  // If we emitted at least one try block, generate the exception handler.
  if (handler.emittedTry) {
    IRBuilder::SaveRestore saveRestore{Builder};
    Builder.setInsertionBlock(handler.exceptionBlock);

    auto *notDoneBlock = Builder.createBasicBlock(Builder.getFunction());
    auto *doneBlock = Builder.createBasicBlock(Builder.getFunction());

    Builder.createCondBranchInst(
        Builder.createLoadStackInst(iteratorDone), doneBlock, notDoneBlock);

    Builder.setInsertionBlock(notDoneBlock);
    emitIteratorClose(iteratorRecord, true);
    Builder.createBranchInst(doneBlock);

    Builder.setInsertionBlock(doneBlock);
    Builder.createThrowInst(Builder.createLoadStackInst(handler.exc));
  } else {
    // If we didn't use the exception block, we need to delete it, otherwise
    // it fails IR validation even though it will be never executed.
    handler.exceptionBlock->eraseFromParent();

    // Delete the not needed exception stack allocation. It would be optimized
    // out later, but it is nice to produce cleaner non-optimized IR, if it is
    // easy to do so.
    assert(
        !handler.exc->hasUsers() &&
        "should not have any users if no try/catch was emitted");
    handler.exc->eraseFromParent();
  }
}

void ESTreeIRGen::emitDestructuringTypedTuple(
    bool declInit,
    ESTree::ArrayPatternNode *targetPat,
    flow::TupleType *type,
    Value *source) {
  size_t i = 0;
  for (ESTree::Node &elem : targetPat->_elements) {
    // It's possible that getting the LRef will have side effects,
    // so we must emit it before the PrLoad.
    // We don't need a try/catch here because there's no iterator to close
    // in typed mode.
    // The PrLoad itself won't have side effects.
    LReference elemLRef = createLRef(&elem, declInit);
    assert(
        i < type->getTypes().size() &&
        "index out of bounds on typechecked tuple");
    Value *val = Builder.createPrLoadInst(
        source,
        i,
        Builder.getLiteralString(llvh::Twine(i)),
        flowTypeToIRType(type->getTypes()[i]));
    elemLRef.emitStore(val);
    ++i;
  }
}

void ESTreeIRGen::emitRestElement(
    bool declInit,
    ESTree::RestElementNode *rest,
    hermes::irgen::ESTreeIRGen::IteratorRecord iteratorRecord,
    hermes::AllocStackInst *iteratorDone,
    SharedExceptionHandler *handler) {
  // 13.3.3.8 BindingRestElement:...BindingIdentifier

  auto *notDoneBlock = Builder.createBasicBlock(Builder.getFunction());
  auto *newValueBlock = Builder.createBasicBlock(Builder.getFunction());
  auto *doneBlock = Builder.createBasicBlock(Builder.getFunction());

  llvh::Optional<LReference> lref;
  if (canCreateLRefWithoutSideEffects(rest->_argument)) {
    lref = createLRef(rest->_argument, declInit);
  } else {
    emitTryWithSharedHandler(handler, [this, &lref, rest, declInit]() {
      lref = createLRef(rest->_argument, declInit);
    });
  }

  auto *A = Builder.createAllocArrayInst({}, 0);
  auto *n = Builder.createAllocStackInst(
      genAnonymousLabelName("n"), Type::createNumber());

  // n = 0.
  Builder.createStoreStackInst(Builder.getLiteralPositiveZero(), n);

  Builder.createCondBranchInst(
      Builder.createLoadStackInst(iteratorDone), doneBlock, notDoneBlock);

  // notDoneBlock:
  Builder.setInsertionBlock(notDoneBlock);
  auto *stepValue = emitIteratorNext(iteratorRecord);
  auto *stepDone = emitIteratorComplete(iteratorRecord);
  Builder.createStoreStackInst(stepDone, iteratorDone);
  Builder.createCondBranchInst(stepDone, doneBlock, newValueBlock);

  // newValueBlock:
  Builder.setInsertionBlock(newValueBlock);
  auto *nVal = Builder.createLoadStackInst(n);
  // A[n] = stepValue;
  // Unfortunately this can throw because our arrays can have limited range.
  // The spec doesn't specify what to do in this case, but the reasonable thing
  // to do is to what we would if this was a for-of loop doing the same thing.
  // See section BindingRestElement:...BindingIdentifier, step f and g:
  // https://www.ecma-international.org/ecma-262/9.0/index.html#sec-destructuring-binding-patterns-runtime-semantics-iteratorbindinginitialization
  emitTryWithSharedHandler(handler, [this, stepValue, A, nVal]() {
    Builder.createStorePropertyInst(stepValue, A, nVal);
  });
  // ++n;
  auto add = Builder.createBinaryOperatorInst(
      nVal, Builder.getLiteralNumber(1), ValueKind::BinaryAddInstKind);
  add->setType(Type::createNumber());
  Builder.createStoreStackInst(add, n);
  Builder.createBranchInst(notDoneBlock);

  // doneBlock:
  Builder.setInsertionBlock(doneBlock);
  if (lref->canStoreWithoutSideEffects()) {
    lref->emitStore(A);
  } else {
    emitTryWithSharedHandler(handler, [&lref, A]() { lref->emitStore(A); });
  }
}

void ESTreeIRGen::emitDestructuringObject(
    bool declInit,
    ESTree::ObjectPatternNode *target,
    Value *source) {
  // Keep track of which keys have been destructured.
  llvh::SmallVector<Value *, 4> excludedItems{};

  if (target->_properties.empty() ||
      llvh::isa<ESTree::RestElementNode>(target->_properties.front())) {
    // ES10.0 13.3.3.5
    // 1. Perform ? RequireObjectCoercible(value).

    // The extremely unlikely case that the user is attempting to destructure
    // into {} or {...rest}. Any other object destructuring will fail upon
    // attempting to retrieve a real property from `source`.
    // We must check that the source can be destructured,
    // and the only time this will throw is if source is undefined or null.
    auto *throwBB = Builder.createBasicBlock(Builder.getFunction());
    auto *doneBB = Builder.createBasicBlock(Builder.getFunction());

    // Use == instead of === to account for both undefined and null.
    Builder.createCondBranchInst(
        Builder.createBinaryOperatorInst(
            source, Builder.getLiteralNull(), ValueKind::BinaryEqualInstKind),
        throwBB,
        doneBB);

    Builder.setInsertionBlock(throwBB);
    Builder.createThrowTypeErrorInst(
        Builder.getLiteralString("Cannot destructure 'undefined' or 'null'."));

    Builder.setInsertionBlock(doneBB);
  }

  for (auto &elem : target->_properties) {
    if (auto *rest = llvh::dyn_cast<ESTree::RestElementNode>(&elem)) {
      emitRestProperty(declInit, rest, excludedItems, source);
      break;
    }
    auto *propNode = cast<ESTree::PropertyNode>(&elem);

    ESTree::Node *valueNode = propNode->_value;
    ESTree::Node *init = nullptr;

    // If we have an initializer, unwrap it.
    if (auto *assign =
            llvh::dyn_cast<ESTree::AssignmentPatternNode>(valueNode)) {
      valueNode = assign->_left;
      init = assign->_right;
    }

    Identifier nameHint = llvh::isa<ESTree::IdentifierNode>(valueNode)
        ? getNameFieldFromID(valueNode)
        : Identifier{};

    if (llvh::isa<ESTree::IdentifierNode>(propNode->_key) &&
        !propNode->_computed) {
      Identifier key = getNameFieldFromID(propNode->_key);
      excludedItems.push_back(Builder.getLiteralString(key));
      LReference lref = createLRef(valueNode, declInit);
      Value *propLoad = Builder.createLoadPropertyInst(source, key);
      Value *optInit = emitOptionalInitialization(propLoad, init, nameHint);
      lref.emitStore(optInit);
    } else {
      Value *key = genExpression(propNode->_key);
      excludedItems.push_back(key);
      LReference lref = createLRef(valueNode, declInit);
      Value *propLoad = Builder.createLoadPropertyInst(source, key);
      Value *optInit = emitOptionalInitialization(propLoad, init, nameHint);
      lref.emitStore(optInit);
    }
  }
}

void ESTreeIRGen::emitRestProperty(
    bool declInit,
    ESTree::RestElementNode *rest,
    const llvh::SmallVectorImpl<Value *> &excludedItems,
    hermes::Value *source) {
  auto lref = createLRef(rest->_argument, declInit);

  // Construct the excluded items.
  llvh::SmallVector<Value *, 4> computedExcludedItems{};
  // Keys must be deduplicated so we don't create StoreNewOwnProperty with the
  // same key twice. Use a SetVector for deterministic ordering.
  llvh::SetVector<Literal *> literalExcludedItems;
  auto *zeroValue = Builder.getLiteralPositiveZero();

  for (Value *key : excludedItems) {
    if (auto *lit = llvh::dyn_cast<LiteralString>(key)) {
      // If the key is a literal string, we can use StoreNewOwnProperty which
      // will allow us to create the object with a single instruction after
      // optimization.
      literalExcludedItems.insert(lit);
    } else {
      // If the key is not a supported literal, then we have to dynamically
      // populate the excluded object with regular stores.
      computedExcludedItems.push_back(key);
    }
  }

  Value *excludedObj;
  if (excludedItems.empty()) {
    excludedObj = Builder.getLiteralUndefined();
  } else {
    // This size is only a hint as the true size may change if there are
    // duplicates when computedExcludedItems is processed at run-time.
    auto excludedSizeHint =
        literalExcludedItems.size() + computedExcludedItems.size();
    // Explicitly set the prototype for the object created here so it isn't
    // initialized to Object.prototype, which may be modified by the user.
    excludedObj = Builder.createAllocObjectInst(
        excludedSizeHint, Builder.getLiteralNull());

    for (Literal *key : literalExcludedItems)
      Builder.createStoreNewOwnPropertyInst(
          zeroValue, excludedObj, key, IRBuilder::PropEnumerable::Yes);

    for (Value *key : computedExcludedItems) {
      Builder.createStorePropertyInst(zeroValue, excludedObj, key);
    }
  }

  auto *restValue = genBuiltinCall(
      BuiltinMethod::HermesBuiltin_copyDataProperties,
      {Builder.createAllocObjectInst(0), source, excludedObj});

  lref.emitStore(restValue);
}

Value *ESTreeIRGen::emitOptionalInitialization(
    Value *value,
    ESTree::Node *init,
    Identifier nameHint) {
  if (!init)
    return value;

  auto *currentBlock = Builder.getInsertionBlock();
  auto *getDefaultBlock = Builder.createBasicBlock(Builder.getFunction());
  auto *storeBlock = Builder.createBasicBlock(Builder.getFunction());

  //    if (value !== undefined) goto storeBlock    [if initializer present]
  //    value = initializer                         [if initializer present]
  //  storeBlock:
  Builder.createCondBranchInst(
      Builder.createBinaryOperatorInst(
          value,
          Builder.getLiteralUndefined(),
          ValueKind::BinaryStrictlyNotEqualInstKind),
      storeBlock,
      getDefaultBlock);

  // getDefaultBlock:
  Builder.setInsertionBlock(getDefaultBlock);
  auto *defaultValue = genExpression(init, nameHint);
  auto *defaultResultBlock = Builder.getInsertionBlock();
  Builder.createBranchInst(storeBlock);

  // storeBlock:
  Builder.setInsertionBlock(storeBlock);
  return Builder.createPhiInst(
      {value, defaultValue}, {currentBlock, defaultResultBlock});
}

Instruction *ESTreeIRGen::emitLoad(Value *from, bool inhibitThrow) {
  if (auto *var = llvh::dyn_cast<Variable>(from)) {
    Instruction *res;
    if (var->getObeysTDZ()) {
      // We don't need to perform a runtime check for TDZ when in the
      // variable's function, since we know whether it has been initialized.
      if (var->getParent()->getFunction() == curFunction()->function) {
        // If not initialized, throw.
        if (curFunction()->initializedTDZVars.count(var) == 0) {
          // Report an error or warning using the builder's location.
          assert(
              Builder.getLocation().isValid() &&
              "emitLoad() with invalid source location");
          if (Builder.getLocation().isValid()) {
            auto &sm = Mod->getContext().getSourceErrorManager();
            sm.message(
                Mod->getContext().getCodeGenerationSettings().test262
                    ? SourceErrorManager::DK_Warning
                    : SourceErrorManager::DK_Error,
                Builder.getLocation(),
                "TDZ violation: reading from uninitialized variable '" +
                    var->getName().str() + "");
          }

          auto *thr = Builder.createThrowIfEmptyInst(Builder.getLiteralEmpty());
          // Pretend that the instruction, which always throws, returns a
          // value with the correct type.
          thr->setType(Type::subtractTy(var->getType(), Type::createEmpty()));
          thr->updateSavedResultType();
          res = thr;
        } else {
          res = Builder.createUnionNarrowTrustedInst(
              Builder.createLoadFrameInst(var),
              Type::subtractTy(var->getType(), Type::createEmpty()));
        }
      } else {
        res = Builder.createThrowIfEmptyInst(Builder.createLoadFrameInst(var));
      }
    } else {
      res = Builder.createLoadFrameInst(var);
    }

    return res;
  } else if (auto *globalProp = llvh::dyn_cast<GlobalObjectProperty>(from)) {
    if (globalProp->isDeclared() || inhibitThrow) {
      return Builder.createLoadPropertyInst(
          Builder.getGlobalObject(), globalProp->getName());
    } else {
      return Builder.createTryLoadGlobalPropertyInst(globalProp);
    }
  } else {
    llvm_unreachable("invalid value to load from");
  }
}

Instruction *
ESTreeIRGen::emitStore(Value *storedValue, Value *ptr, bool declInit) {
  if (auto *var = llvh::dyn_cast<Variable>(ptr)) {
    if (declInit) {
      assert(
          var->getParent()->getFunction() == curFunction()->function &&
          "variable must be initialized in its own function");

      // If this is a TDZ variable, record that it has been initialized.
      if (var->getObeysTDZ()) {
        curFunction()->initializedTDZVars.insert(var);
        // Note that a variable can be initialized more than once, for example
        // when a "var" declaration is shadowed by a catch variable or a
        // parameter.
      }
    } else {
      if (var->getObeysTDZ()) {
        // We don't need to perform a runtime check for TDZ when in the
        // variable's function, since we know whether it has been initialized.
        if (var->getParent()->getFunction() == curFunction()->function) {
          if (curFunction()->initializedTDZVars.count(var) == 0) {
            // Report an error or warning using the builder's location.
            assert(
                Builder.getLocation().isValid() &&
                "emitStore() with invalid source location");
            if (Builder.getLocation().isValid()) {
              auto &sm = Mod->getContext().getSourceErrorManager();
              sm.message(
                  Mod->getContext().getCodeGenerationSettings().test262
                      ? SourceErrorManager::DK_Warning
                      : SourceErrorManager::DK_Error,
                  Builder.getLocation(),
                  "TDZ violation: writing to uninitialized variable '" +
                      var->getName().str() + "");
            }

            auto thr =
                Builder.createThrowIfEmptyInst(Builder.getLiteralEmpty());
            thr->setType(Type::createUndefined());
            thr->updateSavedResultType();
          }
        } else {
          // Must verify whether the variable is initialized.
          Builder.createThrowIfEmptyInst(Builder.createLoadFrameInst(var));
        }
      }
      if (var->getIsConst()) {
        // If this is a const variable being reassigned, throw a TypeError.
        Builder.createThrowTypeErrorInst(
            Builder.getLiteralString("assignment to constant variable."));
        // Create a new block, since ThrowTypeError is a terminator.
        Builder.setInsertionBlock(
            Builder.createBasicBlock(Builder.getFunction()));
      }
    }

    return Builder.createStoreFrameInst(storedValue, var);
  } else if (auto *globalProp = llvh::dyn_cast<GlobalObjectProperty>(ptr)) {
    if (globalProp->isDeclared() || !Builder.getFunction()->isStrictMode()) {
      return Builder.createStorePropertyInst(
          storedValue, Builder.getGlobalObject(), globalProp->getName());
    } else {
      return Builder.createTryStoreGlobalPropertyInst(storedValue, globalProp);
    }
  } else {
    llvm_unreachable("invalid value to load from");
  }
}

void ESTreeIRGen::drainCompilationQueue() {
  while (!compilationQueue_.empty()) {
    compilationQueue_.front()();
    compilationQueue_.pop_front();
  }
}

} // namespace irgen
} // namespace hermes
