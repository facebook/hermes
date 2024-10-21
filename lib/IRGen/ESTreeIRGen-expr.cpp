/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/Support/UTF8.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/ADT/StringSwitch.h"

namespace hermes {
namespace irgen {

Value *ESTreeIRGen::enforceExprType(hermes::Value *value, ESTree::Node *expr) {
  flow::Type *exprFlowType = flowContext_.findNodeType(expr);
  if (!exprFlowType)
    return value;

  Type exprIRType = flowTypeToIRType(exprFlowType);
  Type valueType = value->getType();
  // NOTE: equal sets are subsets of each other, but we want to do the fast
  // check first. In theory, it should catsh 100% of the cases when IRGen is
  // fully typed.
  if (exprIRType == valueType || valueType.isSubsetOf(exprIRType))
    return value;

  if (!exprIRType.isSubsetOf(valueType)) {
    Mod->getContext().getSourceErrorManager().error(
        expr->getSourceRange(),
        "Internal error: Flow expr type is not a subset of value type");
    return value;
  }

  Instruction *cast = Builder.createCheckedTypeCastInst(value, exprIRType);
  cast->setLocation(expr->getDebugLoc());
  return cast;
}

Value *ESTreeIRGen::genExpression(ESTree::Node *expr, Identifier nameHint) {
  return enforceExprType(_genExpressionImpl(expr, nameHint), expr);
}

Value *ESTreeIRGen::_genExpressionImpl(
    ESTree::Node *expr,
    Identifier nameHint) {
  LLVM_DEBUG(
      llvh::dbgs() << "IRGen expression of type " << expr->getNodeName()
                   << "\n");
  IRBuilder::ScopedLocationChange slc(Builder, expr->getDebugLoc());

  // Handle identifiers.
  if (auto *Iden = llvh::dyn_cast<ESTree::IdentifierNode>(expr)) {
    return genIdentifierExpression(Iden, false);
  }

  // Handle Null Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-null-literals
  if (llvh::isa<ESTree::NullLiteralNode>(expr)) {
    return Builder.getLiteralNull();
  }

  // Handle String Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-string-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::StringLiteralNode>(expr)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralString(Lit->_value->str());
  }

  // Handle Regexp Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-regular-expression-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::RegExpLiteralNode>(expr)) {
    return genRegExpLiteral(Lit);
  }

  // Handle Boolean Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-boolean-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::BooleanLiteralNode>(expr)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralBool(Lit->_value);
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::NumericLiteralNode>(expr)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralNumber(Lit->_value);
  }

  // Handle BigInt Literals.
  // https://262.ecma-international.org/#sec-ecmascript-language-types-bigint-type
  if (auto *Lit = llvh::dyn_cast<ESTree::BigIntLiteralNode>(expr)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading BitInt Literal \"" << Lit->_bigint->str()
                     << "\"\n");
    return Builder.getLiteralBigInt(Lit->_bigint);
  }

  // Handle the assignment expression.
  if (auto Assign = llvh::dyn_cast<ESTree::AssignmentExpressionNode>(expr)) {
    return genAssignmentExpr(Assign);
  }

  // Handle Call expressions.
  if (auto *call = llvh::dyn_cast<ESTree::CallExpressionNode>(expr)) {
    return genCallExpr(call);
  }

  // Handle Call expressions.
  if (auto *call = llvh::dyn_cast<ESTree::OptionalCallExpressionNode>(expr)) {
    return genOptionalCallExpr(call, nullptr);
  }

  // Handle the 'new' expressions.
  if (auto *newExp = llvh::dyn_cast<ESTree::NewExpressionNode>(expr)) {
    return genNewExpr(newExp);
  }

  // Handle MemberExpression expressions for access property.
  if (auto *Mem = llvh::dyn_cast<ESTree::MemberExpressionNode>(expr)) {
    return genMemberExpression(Mem, MemberExpressionOperation::Load).result;
  }

  // Handle MemberExpression expressions for access property.
  if (auto *mem = llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(expr)) {
    return genOptionalMemberExpression(
               mem, nullptr, MemberExpressionOperation::Load)
        .result;
  }

  // Handle Array expressions (syntax: [1,2,3]).
  if (auto *Arr = llvh::dyn_cast<ESTree::ArrayExpressionNode>(expr)) {
    return genArrayExpr(Arr);
  }

  // Handle object expressions (syntax: {"1" : "2"}).
  if (auto *Obj = llvh::dyn_cast<ESTree::ObjectExpressionNode>(expr)) {
    return genObjectExpr(Obj);
  }

  // Handle logical expressions (short circuiting).
  if (auto *L = llvh::dyn_cast<ESTree::LogicalExpressionNode>(expr)) {
    return genLogicalExpression(L);
  }

  // Handle Binary Expressions.
  if (auto *Bin = llvh::dyn_cast<ESTree::BinaryExpressionNode>(expr)) {
    return genBinaryExpression(Bin);
  }

  // Handle Unary operator Expressions.
  if (auto *U = llvh::dyn_cast<ESTree::UnaryExpressionNode>(expr)) {
    return genUnaryExpression(U);
  }

  // Handle the 'this' keyword.
  if (llvh::isa<ESTree::ThisExpressionNode>(expr)) {
    return genThisExpression();
  }

  if (auto *MP = llvh::dyn_cast<ESTree::MetaPropertyNode>(expr)) {
    return genMetaProperty(MP);
  }

  // Handle function expressions.
  if (auto *FE = llvh::dyn_cast<ESTree::FunctionExpressionNode>(expr)) {
    return genFunctionExpression(FE, nameHint);
  }

  if (auto *AF = llvh::dyn_cast<ESTree::ArrowFunctionExpressionNode>(expr)) {
    return genArrowFunctionExpression(AF, nameHint);
  }

  if (auto *U = llvh::dyn_cast<ESTree::UpdateExpressionNode>(expr)) {
    return genUpdateExpr(U);
  }

  if (auto *C = llvh::dyn_cast<ESTree::ConditionalExpressionNode>(expr)) {
    return genConditionalExpr(C);
  }

  if (auto *Sq = llvh::dyn_cast<ESTree::SequenceExpressionNode>(expr)) {
    return genSequenceExpr(Sq);
  }

  if (auto *Tl = llvh::dyn_cast<ESTree::TemplateLiteralNode>(expr)) {
    return genTemplateLiteralExpr(Tl);
  }

  if (auto *Tt = llvh::dyn_cast<ESTree::TaggedTemplateExpressionNode>(expr)) {
    return genTaggedTemplateExpr(Tt);
  }

  if (auto *Y = llvh::dyn_cast<ESTree::YieldExpressionNode>(expr)) {
    return Y->_delegate ? genYieldStarExpr(Y) : genYieldExpr(Y);
  }

  if (auto *A = llvh::dyn_cast<ESTree::AwaitExpressionNode>(expr)) {
    return genAwaitExpr(A);
  }

  if (auto *ICK = llvh::dyn_cast<ESTree::ImplicitCheckedCastNode>(expr)) {
    // FIXME: emit something.
    return Builder.createCheckedTypeCastInst(
        genExpression(ICK->_argument, nameHint),
        flowTypeToIRType(flowContext_.getNodeTypeOrAny(ICK)));
  }

  if (auto *TC = llvh::dyn_cast<ESTree::TypeCastExpressionNode>(expr)) {
    return Builder.createCheckedTypeCastInst(
        genExpression(TC->_expression, nameHint),
        flowTypeToIRType(flowContext_.getNodeTypeOrAny(TC)));
  }
  if (auto *TC = llvh::dyn_cast<ESTree::AsExpressionNode>(expr)) {
    return Builder.createCheckedTypeCastInst(
        genExpression(TC->_expression, nameHint),
        flowTypeToIRType(flowContext_.getNodeTypeOrAny(TC)));
  }

  Builder.getModule()->getContext().getSourceErrorManager().error(
      expr->getSourceRange(), Twine("Invalid expression encountered"));
  return Builder.getLiteralUndefined();
}

void ESTreeIRGen::genExpressionBranch(
    ESTree::Node *expr,
    BasicBlock *onTrue,
    BasicBlock *onFalse,
    BasicBlock *onNullish) {
  switch (expr->getKind()) {
    case ESTree::NodeKind::LogicalExpression:
      return genLogicalExpressionBranch(
          cast<ESTree::LogicalExpressionNode>(expr),
          onTrue,
          onFalse,
          onNullish);

    case ESTree::NodeKind::UnaryExpression: {
      auto *e = cast<ESTree::UnaryExpressionNode>(expr);
      // Handle `!expr`
      if (e->_operator == kw_.identBang) {
        // Do not propagate onNullish here because !expr cannot be nullish.
        return genExpressionBranch(e->_argument, onFalse, onTrue, nullptr);
      }
      break;
    }

    case ESTree::NodeKind::SequenceExpression: {
      auto *e = cast<ESTree::SequenceExpressionNode>(expr);

      ESTree::NodePtr last = nullptr;
      for (auto &ex : e->_expressions) {
        if (last)
          genExpression(last);
        last = &ex;
      }
      if (last)
        genExpressionBranch(last, onTrue, onFalse, onNullish);
      return;
    }

    default:
      break;
  }

  Value *condVal = genExpression(expr);
  if (onNullish) {
    Value *isNullish = Builder.createBinaryOperatorInst(
        condVal, Builder.getLiteralNull(), ValueKind::BinaryEqualInstKind);
    BasicBlock *notNullishBB = Builder.createBasicBlock(Builder.getFunction());
    Builder.createCondBranchInst(isNullish, onNullish, notNullishBB);
    Builder.setInsertionBlock(notNullishBB);
  }
  Builder.createCondBranchInst(condVal, onTrue, onFalse);
}

Value *ESTreeIRGen::genArrayFromElements(ESTree::NodeList &list) {
  LLVM_DEBUG(llvh::dbgs() << "Initializing a new array\n");
  AllocArrayInst::ArrayValueList elements;

  // Precalculate the minimum number of elements in case we need to call
  // AllocArrayInst at some point, as well as find out whether we have a spread
  // element (which will result in the final array having variable length).
  unsigned minElements = 0;
  bool variableLength = false;
  for (auto &E : list) {
    if (llvh::isa<ESTree::SpreadElementNode>(&E)) {
      variableLength = true;
      continue;
    }
    ++minElements;
  }

  // We store consecutive elements until we encounter elision,
  // or we enounter a non-literal in limited-register mode.
  // The rest of them has to be initialized through property sets.

  // If we have a variable length array, then we store the next index in
  // a stack location `nextIndex`, to be updated when we encounter spread
  // elements. Otherwise, we simply count them in `count`.
  unsigned count = 0;
  AllocStackInst *nextIndex = nullptr;
  if (variableLength) {
    // Avoid emitting the extra instructions unless we actually need to,
    // to simplify tests and because it's easy.
    nextIndex = Builder.createAllocStackInst("nextIndex", Type::createNumber());
    Builder.createStoreStackInst(Builder.getLiteralPositiveZero(), nextIndex);
  }

  bool consecutive = true;
  AllocArrayInst *allocArrayInst = nullptr;
  for (auto &E : list) {
    Value *value{nullptr};
    bool isSpread = false;
    if (!llvh::isa<ESTree::EmptyNode>(&E)) {
      if (auto *spread = llvh::dyn_cast<ESTree::SpreadElementNode>(&E)) {
        isSpread = true;
        value = genExpression(spread->_argument);
      } else {
        value = genExpression(&E);
      }
    }
    if (!value || !llvh::isa<Literal>(value) || isSpread) {
      // This is either an elision,
      // or a non-literal in limited-register mode,
      // or a spread element.
      if (consecutive) {
        // So far we have been storing elements consecutively,
        // but not anymore, time to create the array.
        allocArrayInst = Builder.createAllocArrayInst(elements, minElements);
        consecutive = false;
      }
    }
    if (isSpread) {
      // Spread the SpreadElement argument into the array.
      // HermesInternal.arraySpread returns the new value of nextIndex,
      // so update nextIndex accordingly and finish this iteration of the loop.
      auto *newNextIndex = genBuiltinCall(
          BuiltinMethod::HermesBuiltin_arraySpread,
          {allocArrayInst, value, Builder.createLoadStackInst(nextIndex)});
      newNextIndex->setType(Type::createNumber());
      Builder.createStoreStackInst(newNextIndex, nextIndex);
      continue;
    }
    // The element is not a spread element, so perform the store here.
    if (value) {
      if (consecutive) {
        elements.push_back(value);
      } else {
        Builder.createStoreOwnPropertyInst(
            value,
            allocArrayInst,
            variableLength ? cast<Value>(Builder.createLoadStackInst(nextIndex))
                           : cast<Value>(Builder.getLiteralNumber(count)),
            IRBuilder::PropEnumerable::Yes);
      }
    }
    // Update the next index or the count depending on if it's a variable length
    // array.
    if (variableLength) {
      // We perform this update on any leading elements before the first spread
      // element as well, but the optimizer will eliminate the extra adds
      // because we know the initial value (0) and how incrementing works.
      Builder.createStoreStackInst(
          Builder.createFBinaryMathInst(
              ValueKind::FAddInstKind,
              Builder.createLoadStackInst(nextIndex),
              Builder.getLiteralNumber(1)),
          nextIndex);
    } else {
      count++;
    }
  }

  if (!allocArrayInst) {
    assert(
        !variableLength &&
        "variable length arrays must allocate their own arrays");
    allocArrayInst = Builder.createAllocArrayInst(elements, list.size());
  }
  if (!list.empty() && llvh::isa<ESTree::EmptyNode>(&list.back())) {
    // Last element is an elision, VM cannot derive the length properly.
    // We have to explicitly set it.
    Value *newLength;
    if (variableLength)
      newLength = Builder.createLoadStackInst(nextIndex);
    else
      newLength = Builder.getLiteralNumber(count);
    Builder.createStorePropertyInst(
        newLength, allocArrayInst, llvh::StringRef("length"));
  }
  return allocArrayInst;
}

void ESTreeIRGen::genFastArrayPush(Value *array, ESTree::Node &arg) {
  // Pushing a single element, just push the value.
  if (!llvh::isa<ESTree::SpreadElementNode>(&arg)) {
    Builder.createFastArrayPushInst(genExpression(&arg), array);
    return;
  }

  auto *spread = llvh::cast<ESTree::SpreadElementNode>(&arg);
  // TODO: Support spreading non-arrays.
  assert(
      llvh::isa<flow::ArrayType>(
          flowContext_.getNodeTypeOrAny(spread->_argument)->info) &&
      "Spread argument must be an array.");

  auto *elems = genExpression(spread->_argument);
  // Append the elements from the spread argument to the target array.
  Builder.createFastArrayAppendInst(elems, array);
}

Value *ESTreeIRGen::genFastArrayFromElements(ESTree::NodeList &list) {
  // Lower the array literal into creating an empty array followed by a series
  // of push and append.
  auto *array =
      Builder.createAllocFastArrayInst(Builder.getLiteralNumber(list.size()));
  for (auto &node : list)
    genFastArrayPush(array, node);
  return array;
}

Value *ESTreeIRGen::genTupleFromElements(ESTree::NodeList &list) {
  AllocObjectLiteralInst::ObjectPropertyMap propMap{};
  size_t i = 0;
  for (ESTree::Node &node : list) {
    propMap.emplace_back(
        Builder.getLiteralString(llvh::Twine(i)), genExpression(&node));
    ++i;
  }
  return Builder.createAllocObjectLiteralInst(propMap);
}

Value *ESTreeIRGen::genArrayExpr(ESTree::ArrayExpressionNode *Expr) {
  // If the array literal originates in typed code, produce a fast array.
  if (llvh::isa<flow::ArrayType>(flowContext_.getNodeTypeOrAny(Expr)->info))
    return genFastArrayFromElements(Expr->_elements);
  // If the array literal is a tuple, produce a tuple object.
  if (llvh::isa<flow::TupleType>(flowContext_.getNodeTypeOrAny(Expr)->info))
    return genTupleFromElements(Expr->_elements);
  return genArrayFromElements(Expr->_elements);
}

Value *ESTreeIRGen::genCallExpr(ESTree::CallExpressionNode *call) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen 'call' statement/expression.\n");

  // Handle native calls separately.
  if (auto *natFuncType = llvh::dyn_cast<flow::NativeFunctionType>(
          flowContext_.getNodeTypeOrAny(call->_callee)->info)) {
    return emitNativeCall(call, natFuncType);
  }

  // Check for a direct call to eval().
  if (auto *identNode = llvh::dyn_cast<ESTree::IdentifierNode>(call->_callee)) {
    if (identNode->_name == kw_.identEval)
      return genCallEvalExpr(call);
  }

  Value *thisVal;
  Value *callee;
  Value *target = Builder.getEmptySentinel();
  bool calleeIsAlwaysClosure = false;
  Value *newTarget = Builder.getLiteralUndefined();
  // If this is nonnull, the call is a super() call, and
  // we should call the field initialization function for the
  // given class type immediately after.
  flow::ClassType *fieldInitClassType = nullptr;

  // Handle MemberExpression expression calls that sets the 'this' property.
  if (auto *Mem = llvh::dyn_cast<ESTree::MemberExpressionNode>(call->_callee)) {
    // Check for SHBuiltin.
    if (llvh::isa<ESTree::SHBuiltinNode>(Mem->_object)) {
      return genSHBuiltin(
          call, llvh::cast<ESTree::IdentifierNode>(Mem->_property));
    }

    MemberExpressionResult memResult =
        genMemberExpression(Mem, MemberExpressionOperation::Load);

    // Call the callee with obj as the 'this' pointer.
    thisVal = memResult.base;
    callee = memResult.result;

    if (memResult.resultFn) {
      target = memResult.resultFn;
      calleeIsAlwaysClosure = true;
    }
  } else if (
      auto *Mem =
          llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(call->_callee)) {
    MemberExpressionResult memResult = genOptionalMemberExpression(
        Mem, nullptr, MemberExpressionOperation::Load);

    // Call the callee with obj as the 'this' pointer.
    thisVal = memResult.base;
    callee = memResult.result;
  } else if (llvh::isa<ESTree::SuperNode>(call->_callee)) {
    if (curFunction()->calledSuperConstructor_) {
      // Found another super() call than the one that actually initializes the
      // base class.
      Mod->getContext().getSourceErrorManager().error(
          call->getSourceRange(), "multiple super() calls in constructor");
    }
    // Register the super() call so we detect any other usages in the function.
    curFunction()->calledSuperConstructor_ = true;
    // Check for a super() call.
    // Call with the passed-in 'this'.
    thisVal = curFunction()->jsParams[0];
    assert(
        curFunction()->superClassNode_ &&
        "SemanticResolver must check super() is in a class with a superclass");
    callee = genExpression(curFunction()->superClassNode_);
    newTarget = Builder.createGetNewTargetInst(
        curFunction()->function->getNewTargetParam());
    // Must call the field init function immediately after.
    fieldInitClassType = curFunction()->typedClassContext.type;
  } else {
    thisVal = Builder.getLiteralUndefined();
    callee = genExpression(call->_callee);
  }

  Value *res =
      emitCall(call, callee, target, calleeIsAlwaysClosure, thisVal, newTarget);
  if (fieldInitClassType) {
    emitFieldInitCall(fieldInitClassType);
  }
  return res;
}

Value *ESTreeIRGen::genOptionalCallExpr(
    ESTree::OptionalCallExpressionNode *call,
    BasicBlock *shortCircuitBB) {
  PhiInst::ValueListType values;
  PhiInst::BasicBlockListType blocks;

  // true when this is the genOptionalCallExpr call containing
  // the logic for shortCircuitBB.
  bool isFirstOptional = shortCircuitBB == nullptr;

  // If isFirstOptional, the final result will be computed in continueBB and
  // returned.
  BasicBlock *continueBB = nullptr;

  if (!shortCircuitBB) {
    // If shortCircuitBB is null, then this is the outermost in the optional
    // chain, so we must create it here and pass it through to every other
    // OptionalCallExpression and OptionalMemberExpression in the chain.
    continueBB = Builder.createBasicBlock(Builder.getFunction());
    auto *insertionBB = Builder.getInsertionBlock();
    shortCircuitBB = Builder.createBasicBlock(Builder.getFunction());
    Builder.setInsertionBlock(shortCircuitBB);
    values.push_back(Builder.getLiteralUndefined());
    blocks.push_back(shortCircuitBB);
    Builder.createBranchInst(continueBB);
    Builder.setInsertionBlock(insertionBB);
  }

  Value *thisVal;
  Value *callee;

  // Handle MemberExpression expression calls that sets the 'this' property.
  if (auto *me = llvh::dyn_cast<ESTree::MemberExpressionNode>(call->_callee)) {
    MemberExpressionResult memResult =
        genMemberExpression(me, MemberExpressionOperation::Load);

    // Call the callee with obj as the 'this' pointer.
    thisVal = memResult.base;
    callee = memResult.result;
  } else if (
      auto *ome =
          llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(call->_callee)) {
    MemberExpressionResult memResult = genOptionalMemberExpression(
        ome, shortCircuitBB, MemberExpressionOperation::Load);

    // Call the callee with obj as the 'this' pointer.
    thisVal = memResult.base;
    callee = memResult.result;
  } else if (
      auto *oce =
          llvh::dyn_cast<ESTree::OptionalCallExpressionNode>(call->_callee)) {
    thisVal = Builder.getLiteralUndefined();
    callee = genOptionalCallExpr(oce, shortCircuitBB);
  } else {
    thisVal = Builder.getLiteralUndefined();
    callee = genExpression(getCallee(call));
  }

  if (call->_optional) {
    BasicBlock *evalRHSBB = Builder.createBasicBlock(Builder.getFunction());

    // If callee is undefined or null, then return undefined.
    // NOTE: We use `obj == null` to account for both null and undefined.
    Builder.createCondBranchInst(
        Builder.createBinaryOperatorInst(
            callee, Builder.getLiteralNull(), ValueKind::BinaryEqualInstKind),
        shortCircuitBB,
        evalRHSBB);

    // baseValue is not undefined or null.
    Builder.setInsertionBlock(evalRHSBB);
  }

  Value *callResult = emitCall(
      call,
      callee,
      /* target */ Builder.getEmptySentinel(),
      /* calleeIsAlwaysClosure */ false,
      thisVal,
      Builder.getLiteralUndefined());

  if (isFirstOptional) {
    values.push_back(callResult);
    blocks.push_back(Builder.getInsertionBlock());
    Builder.createBranchInst(continueBB);

    Builder.setInsertionBlock(continueBB);
    return Builder.createPhiInst(values, blocks);
  }

  // If this isn't the first optional, no Phi needed, just return the
  // callResult.
  return callResult;
}

Value *ESTreeIRGen::genSHBuiltin(
    ESTree::CallExpressionNode *call,
    ESTree::IdentifierNode *builtin) {
  if (builtin->_name == kw_.identCall) {
    return genSHBuiltinCall(call);
  }
  if (builtin->_name == kw_.identCNull) {
    return Builder.getLiteralPositiveZero();
  }
  if (builtin->_name == kw_.identCNativeRuntime) {
    return Builder.createGetNativeRuntimeInst();
  }
  if (builtin->_name == kw_.identExternC) {
    return genSHBuiltinExternC(call);
  }
  // %SHBuiltin.?fastArrayPush(arr: Array, ...);
  if (builtin->_name == kw_.identPrivFastArrayPush) {
    assert(call->_arguments.size() >= 1 && "?fastArrayPush without an array");
    auto it = call->_arguments.begin();
    Value *array = genExpression(&*it++);
    for (auto end = call->_arguments.end(); it != end; ++it)
      genFastArrayPush(array, *it);
    return Builder.createFastArrayLengthInst(array);
  }

  Mod->getContext().getSourceErrorManager().error(
      call->getSourceRange(), "unknown SH builtin call");
  return Builder.getLiteralUndefined();
}

Value *ESTreeIRGen::genSHBuiltinCall(ESTree::CallExpressionNode *call) {
  llvh::SmallVector<Value *, 4> args{};
  Value *callee = nullptr;
  Value *thisValue = nullptr;

  for (ESTree::Node &arg : call->_arguments) {
    Value *argVal = genExpression(&arg);
    if (!callee) {
      // First, fill the callee (required).
      callee = argVal;
    } else if (!thisValue) {
      // Then, fill the thisValue (required).
      thisValue = argVal;
    } else {
      // Any remaining args are passed through to the function.
      args.push_back(argVal);
    }
  }

  if (!thisValue) {
    Mod->getContext().getSourceErrorManager().error(
        call->getSourceRange(),
        "SH builtin 'call' requires at least 2 arguments");
    return Builder.getLiteralUndefined();
  }

  return Builder.createCallInst(
      callee, /* newTarget */ Builder.getLiteralUndefined(), thisValue, args);
}

Value *ESTreeIRGen::genSHBuiltinExternC(ESTree::CallExpressionNode *call) {
  // $SHBuiltin.externC({}, function fopen(path: c_ptr, mode: c_ptr): c_ptr)

  flow::Type *type = flowContext_.findNodeType(call);
  if (!type) {
    Mod->getContext().getSourceErrorManager().error(
        call->getSourceRange(), "extern_c cannot be used in legacy mode");
    return Builder.getLiteralUndefined();
  }
  // Extract the native function type and signature from the call expression
  // but only in debug mode.
#ifndef NDEBUG
  assert(type && "type not found for extern_c");
  NativeSignature *signature =
      llvh::cast<flow::NativeFunctionType>(type->info)->getSignature();
#else
  NativeSignature *signature = nullptr;
  (void)signature;
#endif

  // Extract the name from the function expression.
  UniqueString *name = llvh::cast<ESTree::IdentifierNode>(
                           llvh::cast<ESTree::FunctionExpressionNode>(
                               &*std::next(call->_arguments.begin(), 1))
                               ->_id)
                           ->_name;

  // Lookup the extern declaration.
  auto *nativeExtern =
      Mod->getContext().getNativeContext().getExistingExtern(name);
  assert(
      nativeExtern->signature() == signature &&
      "extern_c signature mismatch with the expression type");

  return Builder.getLiteralNativeExtern(nativeExtern);
}

Value *ESTreeIRGen::emitCall(
    ESTree::CallExpressionLikeNode *call,
    Value *callee,
    Value *target,
    bool calleeIsAlwaysClosure,
    Value *thisVal,
    Value *newTarget) {
  bool hasSpread = false;
  for (auto &arg : getArguments(call)) {
    if (llvh::isa<ESTree::SpreadElementNode>(&arg)) {
      hasSpread = true;
    }
  }

  if (!hasSpread) {
    CallInst::ArgumentList args;
    for (auto &arg : getArguments(call)) {
      args.push_back(genExpression(&arg));
    }

    auto *callInst = Builder.createCallInst(
        callee,
        target,
        calleeIsAlwaysClosure,
        /* env */ Builder.getEmptySentinel(),
        newTarget,
        thisVal,
        args);
    if (auto *functionType = llvh::dyn_cast<flow::BaseFunctionType>(
            flowContext_.getNodeTypeOrAny(getCallee(call))->info)) {
      // Every BaseFunctionType currently is going to be compiled to a
      // NativeJSFunction, so always set this flag.
      // Eventually we will have bytecode, etc.
      callInst->getAttributesRef(Mod).isNativeJSFunction = true;
    }
    return callInst;
  }

  // Otherwise, there exists a spread argument, so the number of arguments
  // is variable.
  // Generate IR for this by creating an array and populating it with the
  // arguments, then calling HermesInternal.apply.
  auto *args = genArrayFromElements(getArguments(call));
  return genBuiltinCall(
      BuiltinMethod::HermesBuiltin_apply, {callee, args, thisVal});
}

Value *ESTreeIRGen::emitNativeCall(
    ESTree::CallExpressionNode *call,
    flow::NativeFunctionType *natFuncType) {
  Value *callee = genExpression(call->_callee);

  llvh::SmallVector<Value *, 4> args{};
  for (auto &arg : getArguments(call)) {
    if (llvh::isa<ESTree::SpreadElementNode>(&arg)) {
      // TODO: this should have been rejected by the typechecker.
      Mod->getContext().getSourceErrorManager().error(
          call->getSourceRange(),
          "spread arguments are not supported for native functions");
      return Builder.getLiteralUndefined();
    }

    args.push_back(genExpression(&arg));
  }

  return Builder.createNativeCallInst(
      flowTypeToIRType(natFuncType->getReturnType()),
      callee,
      natFuncType->getSignature(),
      args);
}

ESTreeIRGen::MemberExpressionResult ESTreeIRGen::genMemberExpression(
    ESTree::MemberExpressionNode *mem,
    MemberExpressionOperation op) {
  if (auto *superNode = llvh::dyn_cast<ESTree::SuperNode>(mem->_object)) {
    if (op == MemberExpressionOperation::Delete) {
      Builder.createCallBuiltinInst(
          BuiltinMethod::HermesBuiltin_throwReferenceError,
          {Builder.getLiteralString("Cannot delete a super property.")});
      return MemberExpressionResult{
          Builder.getLiteralUndefined(),
          nullptr,
          Builder.getLiteralUndefined()};
    }
    if (auto *classType = llvh::dyn_cast<flow::ClassType>(
            flowContext_.getNodeTypeOrAny(superNode)->info)) {
      auto *property = llvh::dyn_cast<ESTree::IdentifierNode>(mem->_property);
      if (!property || mem->_computed) {
        // We can only handle super.foo, where foo is an identifier.
        Mod->getContext().getSourceErrorManager().error(
            mem->getSourceRange(), "unsupported use of 'super'");
        return MemberExpressionResult{
            Builder.getLiteralUndefined(),
            nullptr,
            Builder.getLiteralUndefined()};
      }
      return emitTypedSuperLoad(superNode, property);
    }
    auto *homeObjectVar = curFunction()->capturedState.homeObject;
    assert(homeObjectVar && "homeObjectVar not populated");
    auto *RSI = emitResolveScopeInstIfNeeded(homeObjectVar->getParent());
    Value *homeObjectVal = Builder.createLoadFrameInst(RSI, homeObjectVar);
    // We know that home objects are always ordinary objects.
    Value *superObj = Builder.createLoadParentNoTrapsInst(homeObjectVal);
    Value *propVal = Builder.createLoadPropertyInst(
        superObj, genMemberExpressionProperty(mem));
    Value *thisValue = genThisExpression();
    return MemberExpressionResult{propVal, nullptr, thisValue};
  }

  Value *baseValue = genExpression(mem->_object);
  Value *prop = genMemberExpressionProperty(mem);
  switch (op) {
    case MemberExpressionOperation::Load:
      return emitMemberLoad(mem, baseValue, prop);
    case MemberExpressionOperation::Delete:
      return MemberExpressionResult{
          Builder.createDeletePropertyInst(baseValue, prop),
          nullptr,
          baseValue};
  }
  llvm_unreachable("No other kind of member expression");
}

ESTreeIRGen::MemberExpressionResult ESTreeIRGen::emitMemberLoad(
    ESTree::MemberExpressionNode *mem,
    Value *baseValue,
    Value *propValue) {
  if (auto *classType = llvh::dyn_cast<flow::ClassType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (!mem->_computed) {
      auto propName = Identifier::getFromPointer(
          llvh::cast<ESTree::IdentifierNode>(mem->_property)->_name);
      auto optFieldLookup = classType->findField(propName);
      if (optFieldLookup) {
        size_t fieldIndex = optFieldLookup->getField()->layoutSlotIR;
        return MemberExpressionResult{
            Builder.createPrLoadInst(
                baseValue,
                fieldIndex,
                Builder.getLiteralString(propName),
                flowTypeToIRType(optFieldLookup->getField()->type)),
            nullptr,
            baseValue};
      }
      // Failed to find a class field, check the home object for methods.
      auto optMethodLookup =
          classType->getHomeObjectTypeInfo()->findField(propName);
      assert(
          optMethodLookup && "must have typechecked as either method or field");
      size_t methodIndex = optMethodLookup->getField()->layoutSlotIR;
      // Lookup method on the parent, return baseValue in the result to
      // correctly populate 'this' argument.
      return MemberExpressionResult{
          Builder.createPrLoadInst(
              Builder.createTypedLoadParentInst(baseValue),
              methodIndex,
              Builder.getLiteralString(propName),
              flowTypeToIRType(optMethodLookup->getField()->type)),
          finalMethods_.lookup(optMethodLookup->getField()),
          baseValue};
    }
  }

  if (auto *objType = llvh::dyn_cast<flow::ExactObjectType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (!mem->_computed) {
      auto propName = Identifier::getFromPointer(
          llvh::cast<ESTree::IdentifierNode>(mem->_property)->_name);
      auto optIndex = objType->findField(propName);
      assert(optIndex.hasValue() && "Expected field to exist");
      const auto &field = objType->getFields()[*optIndex];
      return MemberExpressionResult{
          Builder.createPrLoadInst(
              baseValue,
              *optIndex,
              Builder.getLiteralString(propName),
              flowTypeToIRType(field.type)),
          nullptr,
          baseValue};
    }
  }

  // Check if we are loading an array element, and generate the typed IR.
  // NOTE: This is required for correctness, since a regular property load from
  // a FastArray will simply return undefined if it is out-of-bounds.
  if (auto *arrayType = llvh::dyn_cast<flow::ArrayType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (mem->_computed &&
        llvh::isa<flow::NumberType>(
            flowContext_.getNodeTypeOrAny(mem->_property)->info)) {
      return MemberExpressionResult{
          Builder.createFastArrayLoadInst(
              baseValue, propValue, flowTypeToIRType(arrayType->getElement())),
          nullptr,
          baseValue};
    }

    // If we are reading the length from a known FastArray, use the a
    // specialised instruction to load it efficiently.
    auto *ident = llvh::dyn_cast<ESTree::IdentifierNode>(mem->_property);
    if (!mem->_computed && ident && ident->_name == kw_.identLength) {
      return MemberExpressionResult{
          Builder.createFastArrayLengthInst(baseValue), nullptr, baseValue};
    }
  }

  // Check if we are loading a tuple element, and generate the typed IR.
  if (auto *tupleType = llvh::dyn_cast<flow::TupleType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (auto *idx = llvh::dyn_cast<ESTree::NumericLiteralNode>(mem->_property);
        idx && mem->_computed) {
      double d = idx->_value;
      // Safe cast because this has been typechecked.
      uint32_t ulen = ubcastFromDouble<uint32_t>(d);
      return MemberExpressionResult{
          Builder.createPrLoadInst(
              baseValue,
              ulen,
              Builder.getLiteralString(llvh::Twine(ulen)),
              flowTypeToIRType(tupleType->getTypes()[ulen])),
          nullptr,
          baseValue};
    }
    Mod->getContext().getSourceErrorManager().error(
        mem->getSourceRange(), "invalid tuple access");
  }

  return MemberExpressionResult{
      Builder.createLoadPropertyInst(baseValue, propValue), nullptr, baseValue};
}

ESTreeIRGen::MemberExpressionResult ESTreeIRGen::emitTypedSuperLoad(
    ESTree::SuperNode *superNode,
    ESTree::IdentifierNode *property) {
  auto *classType = llvh::cast<flow::ClassType>(
      flowContext_.getNodeTypeOrAny(superNode)->info);
  auto propName = Identifier::getFromPointer(
      llvh::cast<ESTree::IdentifierNode>(property)->_name);
  Value *thisValue = genThisExpression();
  if (auto optFieldLookup = classType->findField(propName)) {
    // Found the field on the class, so load it directly from 'this'.
    size_t fieldIndex = optFieldLookup->getField()->layoutSlotIR;
    return MemberExpressionResult{
        Builder.createPrLoadInst(
            thisValue,
            fieldIndex,
            Builder.getLiteralString(propName),
            flowTypeToIRType(optFieldLookup->getField()->type)),
        nullptr,
        thisValue};
  }
  // Failed to find a class field, check the home object for methods.
  auto optMethodLookup =
      classType->getHomeObjectTypeInfo()->findField(propName);
  assert(optMethodLookup && "must have typechecked as either method or field");
  size_t methodIndex = optMethodLookup->getField()->layoutSlotIR;
  // Lookup method on the parent, return thisValue in the result to
  // correctly populate 'this' argument.
  auto it = classConstructors_.find(classType);
  auto *RSI =
      emitResolveScopeInstIfNeeded(it->second.homeObjectVar->getParent());
  Value *superHomeObject =
      Builder.createLoadFrameInst(RSI, it->second.homeObjectVar);
  return MemberExpressionResult{
      Builder.createPrLoadInst(
          superHomeObject,
          methodIndex,
          Builder.getLiteralString(propName),
          flowTypeToIRType(optMethodLookup->getField()->type)),
      nullptr,
      thisValue};
}

void ESTreeIRGen::emitFieldStore(
    flow::ClassType *classType,
    ESTree::Node *prop,
    Value *object,
    Value *value) {
  auto propName = Identifier::getFromPointer(
      llvh::cast<ESTree::IdentifierNode>(prop)->_name);
  auto optFieldLookup = classType->findField(propName);
  assert(optFieldLookup && "field lookup must succeed after typechecking");
  size_t fieldIndex = optFieldLookup->getField()->layoutSlotIR;
  Builder.createPrStoreInst(
      value,
      object,
      fieldIndex,
      Builder.getLiteralString(propName),
      flowTypeToIRType(optFieldLookup->getField()->type).isNonPtr());
}

void ESTreeIRGen::emitMemberStore(
    ESTree::MemberExpressionNode *mem,
    Value *storedValue,
    Value *baseValue,
    Value *propValue) {
  if (auto *classType = llvh::dyn_cast<flow::ClassType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (!mem->_computed) {
      emitFieldStore(classType, mem->_property, baseValue, storedValue);
      return;
    }
  }

  if (auto *objType = llvh::dyn_cast<flow::ExactObjectType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (!mem->_computed) {
      auto propName = Identifier::getFromPointer(
          llvh::cast<ESTree::IdentifierNode>(mem->_property)->_name);
      auto optIndex = objType->findField(propName);
      assert(optIndex.hasValue() && "Expected field to exist");
      const auto &field = objType->getFields()[*optIndex];
      Builder.createPrStoreInst(
          storedValue,
          baseValue,
          *optIndex,
          Builder.getLiteralString(propName),
          flowTypeToIRType(field.type).isNonPtr());
      return;
    }
  }

  // Check if we are storing to a FastArray, and generate the specialised
  // instruction for it.
  if (auto *arrayType = llvh::dyn_cast<flow::ArrayType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (mem->_computed &&
        llvh::isa<flow::NumberType>(
            flowContext_.getNodeTypeOrAny(mem->_property)->info)) {
      Builder.createFastArrayStoreInst(storedValue, baseValue, propValue);
      return;
    }
  }

  // Check if we are loading a tuple element, and generate the typed IR.
  if (auto *tupleType = llvh::dyn_cast<flow::TupleType>(
          flowContext_.getNodeTypeOrAny(mem->_object)->info)) {
    if (auto *idx = llvh::dyn_cast<ESTree::NumericLiteralNode>(mem->_property);
        idx && mem->_computed) {
      double d = idx->_value;
      // Safe cast because this has been typechecked.
      uint32_t ulen = ubcastFromDouble<uint32_t>(d);
      Builder.createPrStoreInst(
          storedValue,
          baseValue,
          ulen,
          Builder.getLiteralString(llvh::Twine(ulen)),
          flowTypeToIRType(tupleType->getTypes()[ulen]).isNonPtr());
      return;
    }
    Mod->getContext().getSourceErrorManager().error(
        mem->getSourceRange(), "invalid tuple access");
  }

  Builder.createStorePropertyInst(storedValue, baseValue, propValue);
}

ESTreeIRGen::MemberExpressionResult ESTreeIRGen::genOptionalMemberExpression(
    ESTree::OptionalMemberExpressionNode *mem,
    BasicBlock *shortCircuitBB,
    MemberExpressionOperation op) {
  PhiInst::ValueListType values;
  PhiInst::BasicBlockListType blocks;

  // true when this is the genOptionalMemberExpression call containing
  // the logic for shortCircuitBB.
  bool isFirstOptional = shortCircuitBB == nullptr;

  // If isFirstOptional, the final result will be computed in continueBB and
  // returned.
  BasicBlock *continueBB = nullptr;

  if (isFirstOptional) {
    // If shortCircuitBB is null, then this is the outermost in the optional
    // chain, so we must create it here and pass it through to every other
    // OptionalCallExpression and OptionalMemberExpression in the chain.
    continueBB = Builder.createBasicBlock(Builder.getFunction());
    auto *insertionBB = Builder.getInsertionBlock();
    shortCircuitBB = Builder.createBasicBlock(Builder.getFunction());
    Builder.setInsertionBlock(shortCircuitBB);
    values.push_back(Builder.getLiteralUndefined());
    blocks.push_back(shortCircuitBB);
    Builder.createBranchInst(continueBB);
    Builder.setInsertionBlock(insertionBB);
  }

  Value *baseValue = nullptr;
  if (ESTree::OptionalMemberExpressionNode *ome =
          llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(mem->_object)) {
    baseValue = genOptionalMemberExpression(
                    ome, shortCircuitBB, MemberExpressionOperation::Load)
                    .result;
  } else if (
      ESTree::OptionalCallExpressionNode *oce =
          llvh::dyn_cast<ESTree::OptionalCallExpressionNode>(mem->_object)) {
    baseValue = genOptionalCallExpr(oce, shortCircuitBB);
  } else {
    baseValue = genExpression(mem->_object);
  }

  if (mem->_optional) {
    BasicBlock *evalRHSBB = Builder.createBasicBlock(Builder.getFunction());

    // If baseValue is undefined or null, then return undefined.
    // NOTE: We use `obj == null` to account for both null and undefined.
    Builder.createCondBranchInst(
        Builder.createBinaryOperatorInst(
            baseValue,
            Builder.getLiteralNull(),
            ValueKind::BinaryEqualInstKind),
        shortCircuitBB,
        evalRHSBB);

    // baseValue is not undefined, look up the property properly.
    Builder.setInsertionBlock(evalRHSBB);
  }

  Value *prop = genMemberExpressionProperty(mem);
  Value *result = nullptr;
  switch (op) {
    case MemberExpressionOperation::Load:
      result = Builder.createLoadPropertyInst(baseValue, prop);
      break;
    case MemberExpressionOperation::Delete:
      result = Builder.createDeletePropertyInst(baseValue, prop);
      break;
  }
  assert(result && "result must be set");

  if (isFirstOptional) {
    values.push_back(result);
    blocks.push_back(Builder.getInsertionBlock());
    Builder.createBranchInst(continueBB);

    Builder.setInsertionBlock(continueBB);
    return {Builder.createPhiInst(values, blocks), nullptr, baseValue};
  }

  // If this isn't the first optional, no Phi needed, just return the result.
  return {result, nullptr, baseValue};
}

Value *ESTreeIRGen::genCallEvalExpr(ESTree::CallExpressionNode *call) {
  Value *callee = genExpression(call->_callee);

  llvh::SmallVector<Value *, 1> args;
  for (auto &arg : call->_arguments) {
    args.push_back(genExpression(&arg));
  }

  Function *function = Builder.getFunction();
  BasicBlock *evalBlock = Builder.createBasicBlock(function);
  BasicBlock *callBlock = Builder.createBasicBlock(function);
  BasicBlock *nextBlock = Builder.createBasicBlock(function);

  // Check if it is actually calling eval().
  auto *isEval = Builder.createBinaryOperatorInst(
      callee,
      Builder.createGetBuiltinClosureInst(BuiltinMethod::globalThis_eval),
      ValueKind::BinaryStrictlyEqualInstKind);
  Builder.createCondBranchInst(isEval, evalBlock, callBlock);

  // Perform a direct eval.
  Builder.setInsertionBlock(evalBlock);
  Value *evalRes = Builder.createDirectEvalInst(
      args.empty() ? Builder.getLiteralUndefined() : args[0],
      curFunction()->function->isStrictMode());
  Builder.createBranchInst(nextBlock);

  // Perform a normal call.
  Builder.setInsertionBlock(callBlock);
  Value *callRes = Builder.createCallInst(
      callee,
      /* newTarget */ Builder.getLiteralUndefined(),
      /* thisValue */ Builder.getLiteralUndefined(),
      args);
  Builder.createBranchInst(nextBlock);

  Builder.setInsertionBlock(nextBlock);
  PhiInst *result = Builder.createPhiInst();
  result->addEntry(evalRes, evalBlock);
  result->addEntry(callRes, callBlock);
  return result;
}

/// Convert a property key node to its JavaScript string representation.
static llvh::StringRef propertyKeyAsString(
    llvh::SmallVectorImpl<char> &storage,
    ESTree::Node *Key) {
  // Handle String Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-string-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::StringLiteralNode>(Key)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Lit->_value->str();
  }

  // Handle identifiers as if they are String Literals.
  if (auto *Iden = llvh::dyn_cast<ESTree::IdentifierNode>(Key)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading String Literal \"" << Iden->_name << "\"\n");
    return Iden->_name->str();
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::NumericLiteralNode>(Key)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    storage.resize(NUMBER_TO_STRING_BUF_SIZE);
    auto len = numberToString(Lit->_value, storage.data(), storage.size());
    return llvh::StringRef(storage.begin(), len);
  }

  llvm_unreachable("Don't know this kind of property key");
  return llvh::StringRef();
}

Value *ESTreeIRGen::genObjectExpr(ESTree::ObjectExpressionNode *Expr) {
  LLVM_DEBUG(llvh::dbgs() << "Initializing a new object\n");

  if (flow::ExactObjectType *objType = llvh::dyn_cast<flow::ExactObjectType>(
          flowContext_.getNodeTypeOrAny(Expr)->info)) {
    return genTypedObjectExpr(Expr, objType);
  }

  /// Store information about a property. Is it an accessor (getter/setter) or
  /// a value, and the actual value.
  class PropertyValue {
   public:
    /// Is this a getter/setter value.
    bool isAccessor = false;
    /// Tracks the state of generating IR for this value.
    enum { None, Placeholder, IRGenerated } state{None};
    /// The value, if this is a regular property
    ESTree::Node *valueNode{};
    /// Getter accessor, if this is an accessor property.
    ESTree::FunctionExpressionNode *getterNode{};
    /// Setter accessor, if this is an accessor property.
    ESTree::FunctionExpressionNode *setterNode{};

    SMRange getSourceRange() {
      if (valueNode) {
        return valueNode->getSourceRange();
      }

      if (getterNode) {
        return getterNode->getSourceRange();
      }

      if (setterNode) {
        return setterNode->getSourceRange();
      }

      llvm_unreachable("Unset node has no location info");
    }

    void setValue(ESTree::Node *val) {
      isAccessor = false;
      valueNode = val;
      getterNode = setterNode = nullptr;
    }
    void setGetter(ESTree::FunctionExpressionNode *get) {
      if (!isAccessor) {
        valueNode = nullptr;
        setterNode = nullptr;
        isAccessor = true;
      }
      getterNode = get;
    }
    void setSetter(ESTree::FunctionExpressionNode *set) {
      if (!isAccessor) {
        valueNode = nullptr;
        getterNode = nullptr;
        isAccessor = true;
      }
      setterNode = set;
    }
  };

  // First accumulate all getters and setters. We walk all properties, convert
  // them to string and store the value into propMap, possibly overwriting the
  // previous value.
  // Note that computed properties are not stored in the propMap as we do not
  // know the keys at compilation time.
  llvh::StringMap<PropertyValue> propMap;
  // The first location where a given property name is encountered.
  llvh::StringMap<SMRange> firstLocMap;
  llvh::SmallVector<char, 32> stringStorage;
  /// The optional __proto__ property.
  ESTree::PropertyNode *protoProperty = nullptr;
  // Keep track of if we've seen a method property. This is used to decide if we
  // should capture the object literal we are building in a variable.
  bool hasMethodProp = false;

  for (auto &P : Expr->_properties) {
    if (llvh::isa<ESTree::SpreadElementNode>(&P)) {
      continue;
    }

    // We are reusing the storage, so make sure it is cleared at every
    // iteration.
    stringStorage.clear();

    auto *prop = cast<ESTree::PropertyNode>(&P);
    hasMethodProp |= prop->_method;
    if (prop->_computed) {
      // Can't store any useful information if the name is computed.
      // Just generate the code in the next loop.
      continue;
    }

    auto propName = propertyKeyAsString(stringStorage, prop->_key);

    // protoProperty should only be recorded if the property is not a method
    // nor a shorthand value.
    if (prop->_kind->str() == "init" && propName == "__proto__" &&
        !prop->_method && !prop->_shorthand) {
      if (!protoProperty) {
        protoProperty = prop;
      } else {
        Builder.getModule()->getContext().getSourceErrorManager().error(
            prop->getSourceRange(),
            "__proto__ was set multiple times in the object definition.");
        Builder.getModule()->getContext().getSourceErrorManager().note(
            protoProperty->getSourceRange(), "The first definition was here.");
      }
      continue;
    }

    PropertyValue *propValue = &propMap[propName];
    if (prop->_kind->str() == "get") {
      propValue->setGetter(cast<ESTree::FunctionExpressionNode>(prop->_value));
    } else if (prop->_kind->str() == "set") {
      propValue->setSetter(cast<ESTree::FunctionExpressionNode>(prop->_value));
    } else {
      assert(prop->_kind->str() == "init" && "invalid PropertyNode kind");
      // We record the propValue if this is a regular property
      propValue->setValue(prop->_value);
    }

    std::string key = (prop->_kind->str() + propName).str();
    auto iterAndSuccess = firstLocMap.try_emplace(key, prop->getSourceRange());
    if (!iterAndSuccess.second) {
      Builder.getModule()->getContext().getSourceErrorManager().warning(
          prop->getSourceRange(),
          Twine("the property \"") + propName +
              "\" was set multiple times in the object definition.");

      Builder.getModule()->getContext().getSourceErrorManager().note(
          iterAndSuccess.first->second, "The first definition was here.");
    }
  }

  /// Attempt to determine whether we can directly use the value of the
  /// __proto__ property as a parent when creating the object for an object
  /// initializer, instead of setting it later with
  /// HermesInternal.silentSetPrototypeOf(). That is not possible to determine
  /// statically in the general case, but we can check for the simple cases:
  /// - __proto__ property is first.
  /// - the value of __proto__ is constant.
  Value *objectParent = nullptr;
  if (protoProperty &&
      (&Expr->_properties.front() == protoProperty ||
       isConstantExpr(protoProperty->_value))) {
    objectParent = genExpression(protoProperty->_value);
  }

  // Allocate a new javascript object on the heap.
  auto Obj = Builder.createAllocObjectLiteralInst({}, objectParent);
  Variable *capturedObj = nullptr;
  // If we see a method, there may be a super property reference. We need to
  // capture the value of this object literal, which will become the home object
  // for any methods defined in this literal.
  if (hasMethodProp) {
    capturedObj = Builder.createVariable(
        curFunction()->curScope->getVariableScope(),
        "?obj",
        Type::createObject(),
        true);
    Builder.createStoreFrameInst(curFunction()->curScope, Obj, capturedObj);
  }

  // haveSeenComputedProp tracks whether we have processed a computed property.
  // Once we do, for all future properties, we can no longer generate
  // StoreNewOwnPropertyInst because the computed property could have already
  // defined any property.
  bool haveSeenComputedProp = false;

  // Initialize all properties. We check whether the value of each property
  // will be overwritten (by comparing against what we have saved in propMap).
  // In that case we still compute the value (it could have side effects), but
  // we don't store it. The exception to this are accessor functions - there
  // is no need to create them if we don't use them because creating a function
  // has no side effects.
  for (auto &P : Expr->_properties) {
    if (auto *spread = llvh::dyn_cast<ESTree::SpreadElementNode>(&P)) {
      genBuiltinCall(
          BuiltinMethod::HermesBuiltin_copyDataProperties,
          {Obj, genExpression(spread->_argument)});
      haveSeenComputedProp = true;
      continue;
    }

    // We are reusing the storage, so make sure it is cleared at every
    // iteration.
    stringStorage.clear();

    auto *prop = cast<ESTree::PropertyNode>(&P);

    if (prop->_computed) {
      // TODO (T46136220): Set the .name property for anonymous functions that
      // are values for computed property keys.
      auto *key = genExpression(prop->_key);
      auto *value = prop->_method
          ? genFunctionExpression(
                llvh::cast<ESTree::FunctionExpressionNode>(prop->_value),
                Identifier{},
                nullptr,
                Function::DefinitionKind::ES6Method,
                capturedObj)
          : genExpression(prop->_value, Identifier{});
      if (prop->_kind->str() == "get") {
        Builder.createStoreGetterSetterInst(
            value,
            Builder.getLiteralUndefined(),
            Obj,
            key,
            IRBuilder::PropEnumerable::Yes);
      } else if (prop->_kind->str() == "set") {
        Builder.createStoreGetterSetterInst(
            Builder.getLiteralUndefined(),
            value,
            Obj,
            key,
            IRBuilder::PropEnumerable::Yes);
      } else {
        Builder.createStoreOwnPropertyInst(
            value, Obj, key, IRBuilder::PropEnumerable::Yes);
      }
      haveSeenComputedProp = true;
      continue;
    }

    llvh::StringRef keyStr = propertyKeyAsString(stringStorage, prop->_key);

    if (prop == protoProperty) {
      // This is the first definition of __proto__. If we already used it
      // as an object parent we just skip it, but otherwise we must
      // explicitly set the parent now by calling \c
      // HermesInternal.silentSetPrototypeOf().
      if (!objectParent) {
        auto *parent = genExpression(prop->_value);

        IRBuilder::SaveRestore saveState{Builder};
        Builder.setLocation(prop->_key->getDebugLoc());

        genBuiltinCall(
            BuiltinMethod::HermesBuiltin_silentSetPrototypeOf, {Obj, parent});
      }

      continue;
    }

    PropertyValue *propValue = &propMap[keyStr];

    // For any node that has a corresponding propValue, we need to ensure that
    // the we insert either a placeholder or the final IR before the end of this
    // iteration.
    auto checkState = llvh::make_scope_exit(
        [&] { assert(propValue->state != PropertyValue::None); });

    auto *Key = Builder.getLiteralString(keyStr);

    auto maybeInsertPlaceholder = [&] {
      if (propValue->state == PropertyValue::None) {
        // This value is going to be overwritten, but insert a placeholder in
        // order to maintain insertion order.
        if (haveSeenComputedProp) {
          Builder.createStoreOwnPropertyInst(
              Builder.getLiteralNull(),
              Obj,
              Key,
              IRBuilder::PropEnumerable::Yes);
        } else {
          Builder.createStoreNewOwnPropertyInst(
              Builder.getLiteralNull(),
              Obj,
              Key,
              IRBuilder::PropEnumerable::Yes);
        }
        propValue->state = PropertyValue::Placeholder;
      }
    };

    if (prop->_kind->str() == "get" || prop->_kind->str() == "set") {
      // If  we already generated it, skip.
      if (propValue->state == PropertyValue::IRGenerated)
        continue;

      if (!propValue->isAccessor) {
        maybeInsertPlaceholder();
        continue;
      }

      Value *getter = Builder.getLiteralUndefined();
      Value *setter = Builder.getLiteralUndefined();

      if (propValue->getterNode) {
        getter = genExpression(
            propValue->getterNode,
            Builder.createIdentifier("get " + keyStr.str()));
      }

      if (propValue->setterNode) {
        setter = genExpression(
            propValue->setterNode,
            Builder.createIdentifier("set " + keyStr.str()));
      }

      Builder.createStoreGetterSetterInst(
          getter, setter, Obj, Key, IRBuilder::PropEnumerable::Yes);

      propValue->state = PropertyValue::IRGenerated;

      continue;
    }

    // Always generate the values, even if we don't need it, for the side
    // effects.
    auto nameHint = Builder.createIdentifier(keyStr);
    auto *value = prop->_method
        ? genFunctionExpression(
              llvh::cast<ESTree::FunctionExpressionNode>(prop->_value),
              nameHint,
              nullptr,
              Function::DefinitionKind::ES6Method,
              capturedObj)
        : genExpression(prop->_value, nameHint);

    // Only store the value if it won't be overwritten.
    if (propMap[keyStr].valueNode == prop->_value) {
      assert(
          propValue->state != PropertyValue::IRGenerated &&
          "IR can only be generated once");
      if (haveSeenComputedProp ||
          propValue->state == PropertyValue::Placeholder) {
        Builder.createStoreOwnPropertyInst(
            value, Obj, Key, IRBuilder::PropEnumerable::Yes);
      } else {
        Builder.createStoreNewOwnPropertyInst(
            value, Obj, Key, IRBuilder::PropEnumerable::Yes);
      }
      propValue->state = PropertyValue::IRGenerated;
    } else {
      maybeInsertPlaceholder();
    }
  }

  // Return the newly allocated object (because this is an expression, not a
  // statement).
  return Obj;
}

/// Generate IR for the typed object literal \p Expr.
Value *ESTreeIRGen::genTypedObjectExpr(
    ESTree::ObjectExpressionNode *Expr,
    flow::ExactObjectType *type) {
  // Evaluate the value expressions and keep pointers to the vales.
  // Mapping from property name to value and PrLoad index.
  // If Value is nullptr, then the value was already emitted as part of the
  // object literal and shouldn't be PrStored.
  // The order of PrStores doesn't matter because they have no side effects,
  // it just needs to be deterministic, so use MapVector for iteration.
  llvh::SmallMapVector<Identifier, std::pair<Value *, size_t>, 8>
      storedValues{};
  llvh::SmallVector<char, 32> stringStorage{};
  for (auto &node : Expr->_properties) {
    auto *prop = llvh::cast<ESTree::PropertyNode>(&node);
    assert(
        !prop->_computed && !prop->_method && prop->_kind->str() == "init" &&
        "Unexpected property kind in typechecked object");

    stringStorage.clear();
    Identifier propName = Builder.createIdentifier(
        propertyKeyAsString(stringStorage, prop->_key));

    auto optIndex = type->findField(propName);
    assert(optIndex.hasValue() && "Expected field to exist");

    Value *storedValue = genExpression(prop->_value);
    // Overwrite the previous value if there is one.
    // We've already run the expression for side effects.
    storedValues[propName] = {storedValue, *optIndex};
  }

  // TODO: Have a specific instruction for allocating an object
  // that uses the prop map, makes a sealed object, uses uninitialized fields
  // instead of default init value, etc.

  // Allocate the object with the correct property names.
  AllocObjectLiteralInst::ObjectPropertyMap propMap{};
  propMap.reserve(type->getFields().size());
  for (size_t i = 0, e = type->getFields().size(); i < e; ++i) {
    const auto &field = type->getFields()[i];
    auto it = storedValues.find(field.name);
    assert(
        it != storedValues.end() &&
        "Missing stored value in typechecked object literal");
    // Use a literal if possible, otherwise use the default init value.
    // Avoid putting non-literals here because we want to emit PrStore
    // instead of StoreNewOwnPropertyInst.
    Value *initValue = nullptr;
    if (llvh::isa<Literal>(it->second.first)) {
      initValue = it->second.first;
      // Prevent emitting PrStore later for literals.
      it->second.first = nullptr;
      assert(it->second.second == i && "Stored value must have same index");
    } else {
      initValue = getDefaultInitValue(field.type);
    }
    propMap.emplace_back(Builder.getLiteralString(field.name), initValue);
  }

  Value *result = propMap.empty()
      ? Builder.createAllocObjectLiteralInst()
      : Builder.createAllocObjectLiteralInst(propMap);

  // Store the remaining non-literal properties.
  for (const auto &[name, valueAndIdx] : storedValues) {
    const auto &[storedValue, idx] = valueAndIdx;
    if (storedValue) {
      // We only put strings into propMap.
      auto *name = llvh::cast<LiteralString>(propMap[idx].first);
      Builder.createPrStoreInst(
          storedValue,
          result,
          idx,
          name,
          flowTypeToIRType(type->getFields()[idx].type).isNonPtr());
    }
  }

  return result;
}

Value *ESTreeIRGen::genSequenceExpr(ESTree::SequenceExpressionNode *Sq) {
  Value *result = Builder.getLiteralUndefined();

  // Generate all expressions in the sequence, but take only the last one.
  for (auto &Ex : Sq->_expressions) {
    result = genExpression(&Ex);
  }

  return result;
}

Value *ESTreeIRGen::genYieldExpr(ESTree::YieldExpressionNode *Y) {
  assert(!Y->_delegate && "must use genYieldStarExpr for yield*");

  Value *value = Y->_argument ? genExpression(Y->_argument)
                              : Builder.getLiteralUndefined();
  return genYieldOrAwaitExpr(value);
}

Value *ESTreeIRGen::genAwaitExpr(ESTree::AwaitExpressionNode *A) {
  Value *value = genExpression(A->_argument);
  return genYieldOrAwaitExpr(value);
}

Value *ESTreeIRGen::genYieldOrAwaitExpr(Value *value) {
  auto *bb = Builder.getInsertionBlock();
  auto *next = Builder.createBasicBlock(bb->getParent());

  auto *resumeIsReturn = Builder.createAllocStackInst(
      genAnonymousLabelName("isReturn"), Type::createBoolean());

  Builder.createSaveAndYieldInst(value, Builder.getLiteralBool(false), next);
  Builder.setInsertionBlock(next);
  return genResumeGenerator(
      GenFinally::Yes,
      resumeIsReturn,
      Builder.createBasicBlock(bb->getParent()));
}

/// Generate the code for `yield* value`.
/// We use some stack locations to store state while iterating:
/// - received (the value passed by the user to .next(), etc)
/// - result (the final result of the yield* expression)
///
/// iteratorRecord stores the iterator which we are iterating over.
///
/// Final IR has the following basic blocks for normal control flow:
///
/// getNext: Get the next value from the iterator.
/// - Call next() on the iteratorRecord and stores to `result`
/// - If done, go to exit
/// - Otherwise, go to yieldNextRes
///
/// resumeGenTryStartBB: Runs the ResumeGenerator instruction, wrapped in a try.
/// - Code for `finally` is also emitted here.
///
/// yieldNextRes: Yield the result of the next() call
///
/// exit: Returns `result` which should have the final results stored in it.
///
/// When the user calls `.return`, the finalizer is executed to call
/// `iteratorRecord.return` if it exists. The code for that is contained within
/// the SurroundingTry. If the .return function is defined, it is called and the
/// 'done' property of the result of the call is used to either branch back to
/// the 'resume' block or to propagate the return.
///
/// When the user calls '.throw', the code in emitHandler is executed. All
/// generators used as delegates must have a .throw() method, so that is checked
/// for and called. The result is then used to either resume if not done, or to
/// return immediately by branching to the 'exit' block.
Value *ESTreeIRGen::genYieldStarExpr(ESTree::YieldExpressionNode *Y) {
  assert(Y->_delegate && "must use genYieldExpr for yield");
  auto *function = Builder.getInsertionBlock()->getParent();
  auto *getNextBlock = Builder.createBasicBlock(function);
  auto *yieldNextRes = Builder.createBasicBlock(function);
  auto *exitBlock = Builder.createBasicBlock(function);

  // Calls ResumeGenerator and returns or throws if requested.
  auto *resumeGenTryStartBB = Builder.createBasicBlock(function);

  // Ends the opened TryStartInst.
  auto *tryEndBB = Builder.createBasicBlock(function);

  auto *exprValue = genExpression(Y->_argument);
  IteratorRecordSlow iteratorRecord = emitGetIteratorSlow(exprValue);

  // The "received" value when the user resumes the generator.
  // Initialized to undefined on the first run, then stored to immediately
  // following any genResumeGenerator.
  auto *received = Builder.createAllocStackInst(
      genAnonymousLabelName("received"), Type::createAnyType());
  Builder.createStoreStackInst(Builder.getLiteralUndefined(), received);

  // The "isReturn" value when the user resumes the generator.
  // Stored to immediately following any genResumeGenerator.
  auto *resumeIsReturn = Builder.createAllocStackInst(
      genAnonymousLabelName("isReturn"), Type::createBoolean());

  // The final result of the `yield*` expression.
  // This can be set from either the body or the handler, so it is placed
  // in the stack to allow populating it from anywhere.
  auto *result = Builder.createAllocStackInst(
      genAnonymousLabelName("result"), Type::createAnyType());

  Builder.createBranchInst(getNextBlock);

  // 7.a.i.  Let innerResult be ? Call(
  //   iteratorRecord.[[NextMethod]],
  //   iteratorRecord.[[Iterator]],
  //   <received.[[Value]]>
  // )
  // Avoid using emitIteratorNext here because the spec does not.
  Builder.setInsertionBlock(getNextBlock);
  auto *nextResult = Builder.createCallInst(
      iteratorRecord.nextMethod,
      /* newTarget */ Builder.getLiteralUndefined(),
      iteratorRecord.iterator,
      {Builder.createLoadStackInst(received)});
  emitEnsureObject(nextResult, "iterator.next() did not return an object");

  Builder.createStoreStackInst(nextResult, result);
  auto *done = emitIteratorCompleteSlow(nextResult);
  Builder.createCondBranchInst(done, exitBlock, yieldNextRes);

  // The primary call path for yielding the next result.
  // It's important to put the SaveAndYield for the .next() value in a separate
  // BB then ResumeGenerator. This is because other BBs need to jump to
  // ResumeGenerator after completing a different SaveAndYield.
  Builder.setInsertionBlock(yieldNextRes);
  Builder.createSaveAndYieldInst(
      nextResult, Builder.getLiteralBool(true), resumeGenTryStartBB);

  Builder.setInsertionBlock(resumeGenTryStartBB);
  emitTryCatchScaffolding(
      getNextBlock,
      // emitBody.
      [this,
       Y,
       resumeIsReturn,
       received,
       resumeGenTryStartBB,
       &iteratorRecord,
       tryEndBB](BasicBlock *catchBlock) {
        // Generate IR for the body of Try
        SurroundingTry thisTry{
            curFunction(),
            Y,
            catchBlock,
            {},
            [this, &iteratorRecord, received, resumeGenTryStartBB](
                ESTree::Node *, ControlFlowChange cfc, BasicBlock *) {
              if (cfc == ControlFlowChange::Break) {
                // This finalizer block is executed upon early return during
                // the yield*, which happens when the user requests a .return().
                auto *function = Builder.getFunction();
                auto *haveReturnBB = Builder.createBasicBlock(function);
                auto *noReturnBB = Builder.createBasicBlock(function);
                auto *isDoneBB = Builder.createBasicBlock(function);
                auto *isNotDoneBB = Builder.createBasicBlock(function);

                // Check if "returnMethod" is undefined.
                auto *returnMethod = genBuiltinCall(
                    BuiltinMethod::HermesBuiltin_getMethod,
                    {iteratorRecord.iterator,
                     Builder.getLiteralString("return")});
                auto *noReturn = Builder.createBinaryOperatorInst(
                    returnMethod,
                    Builder.getLiteralUndefined(),
                    ValueKind::BinaryStrictlyEqualInstKind);
                Builder.createCondBranchInst(
                    noReturn, noReturnBB, haveReturnBB);

                Builder.setInsertionBlock(haveReturnBB);
                // iv. Let innerReturnResult be
                // ? Call(return, iterator, received.[[Value]]).
                auto *innerReturnResult = Builder.createCallInst(
                    returnMethod,
                    /* newTarget */ Builder.getLiteralUndefined(),
                    iteratorRecord.iterator,
                    {Builder.createLoadStackInst(received)});
                // vi. If Type(innerReturnResult) is not Object,
                // throw a TypeError exception.
                emitEnsureObject(
                    innerReturnResult,
                    "iterator.return() did not return an object");
                // vii. Let done be ? IteratorComplete(innerReturnResult).
                auto *done = emitIteratorCompleteSlow(innerReturnResult);
                Builder.createCondBranchInst(done, isDoneBB, isNotDoneBB);

                Builder.setInsertionBlock(isDoneBB);
                // viii. 1. Let value be ? IteratorValue(innerReturnResult).
                auto *value = emitIteratorValueSlow(innerReturnResult);
                genFinallyBeforeControlChange(
                    curFunction()->surroundingTry,
                    nullptr,
                    ControlFlowChange::Break);
                // viii. 2. Return Completion
                // { [[Type]]: return, [[Value]]: value, [[Target]]: empty }.
                Builder.createReturnInst(value);

                // x. Else, set received to GeneratorYield(innerReturnResult).
                Builder.setInsertionBlock(isNotDoneBB);
                Builder.createSaveAndYieldInst(
                    innerReturnResult,
                    Builder.getLiteralBool(true),
                    resumeGenTryStartBB);

                // If return is undefined, return Completion(received).
                Builder.setInsertionBlock(noReturnBB);
              }
            }};

        // Note that resumeGenTryStartBB was created above to allow all
        // SaveAndYield insts to have the same resume point (including
        // SaveAndYield in the catch handler), but we must populate it inside
        // the scaffolding so that the SurroundingTry is correct for the
        // genFinallyBeforeControlChange call emitted by genResumeGenerator.
        genResumeGenerator(GenFinally::Yes, resumeIsReturn, tryEndBB, received);

        // SaveAndYieldInst is a Terminator, but emitTryCatchScaffolding
        // needs a block from which to Branch to the TryEnd instruction.
        // Make a dummy block which can do that.
        Builder.setInsertionBlock(tryEndBB);
      },
      // emitNormalCleanup.
      []() {},
      // emitHandler.
      [this, exitBlock, result, &iteratorRecord, &resumeGenTryStartBB](
          BasicBlock *getNextBlock) {
        auto *catchReg = Builder.createCatchInst();

        auto *function = Builder.getFunction();
        auto *hasThrowMethodBB = Builder.createBasicBlock(function);
        auto *noThrowMethodBB = Builder.createBasicBlock(function);
        auto *isDoneBB = Builder.createBasicBlock(function);
        auto *isNotDoneBB = Builder.createBasicBlock(function);

        // b.i. Let throw be ? GetMethod(iterator, "throw").
        auto *throwMethod = genBuiltinCall(
            BuiltinMethod::HermesBuiltin_getMethod,
            {iteratorRecord.iterator, Builder.getLiteralString("throw")});
        auto *hasThrowMethod = Builder.createBinaryOperatorInst(
            throwMethod,
            Builder.getLiteralUndefined(),
            ValueKind::BinaryStrictlyEqualInstKind);
        Builder.createCondBranchInst(
            hasThrowMethod, noThrowMethodBB, hasThrowMethodBB);

        // ii. If throw is not undefined, then
        Builder.setInsertionBlock(hasThrowMethodBB);
        // ii. 1. Let innerResult be
        //        ? Call(throw, iterator, « received.[[Value]] »).
        // ii. 3. NOTE: Exceptions from the inner iterator throw method are
        // propagated. Normal completions from an inner throw method are
        // processed similarly to an inner next.
        auto *innerResult = Builder.createCallInst(
            throwMethod,
            /* newTarget */ Builder.getLiteralUndefined(),
            iteratorRecord.iterator,
            {catchReg});
        // ii. 4. If Type(innerResult) is not Object,
        //        throw a TypeError exception.
        emitEnsureObject(
            innerResult, "iterator.throw() did not return an object");
        // ii. 5. Let done be ? IteratorComplete(innerResult).
        auto *done = emitIteratorCompleteSlow(innerResult);
        Builder.createCondBranchInst(done, isDoneBB, isNotDoneBB);

        // ii. 6. If done is true, then return ? IteratorValue(innerResult).
        Builder.setInsertionBlock(isDoneBB);
        Builder.createStoreStackInst(innerResult, result);
        Builder.createBranchInst(exitBlock);

        // ii. 8. Else, set received to GeneratorYield(innerResult).
        Builder.setInsertionBlock(isNotDoneBB);
        Builder.createSaveAndYieldInst(
            innerResult, Builder.getLiteralBool(true), resumeGenTryStartBB);

        // NOTE: If iterator does not have a throw method, this throw is
        // going to terminate the yield* loop. But first we need to give
        // iterator a chance to clean up.
        Builder.setInsertionBlock(noThrowMethodBB);
        emitIteratorCloseSlow(iteratorRecord, false);
        Builder.createThrowTypeErrorInst(Builder.getLiteralString(
            "yield* delegate must have a .throw() method"));
      });

  Builder.setInsertionBlock(exitBlock);

  return emitIteratorValueSlow(Builder.createLoadStackInst(result));
}

Value *ESTreeIRGen::genResumeGenerator(
    GenFinally genFinally,
    AllocStackInst *isReturn,
    BasicBlock *nextBB,
    AllocStackInst *received) {
  auto *resume = Builder.createResumeGeneratorInst(isReturn);
  if (received) {
    Builder.createStoreStackInst(resume, received);
  }
  auto *retBB =
      Builder.createBasicBlock(Builder.getInsertionBlock()->getParent());

  Builder.createCondBranchInst(
      Builder.createLoadStackInst(isReturn), retBB, nextBB);

  Builder.setInsertionBlock(retBB);
  if (received) {
    Builder.createStoreStackInst(resume, received);
  }
  if (genFinally == GenFinally::Yes) {
    genFinallyBeforeControlChange(
        curFunction()->surroundingTry, nullptr, ControlFlowChange::Break);
  }
  Builder.createReturnInst(resume);

  Builder.setInsertionBlock(nextBB);
  return resume;
}

Value *ESTreeIRGen::genBinaryExpression(ESTree::BinaryExpressionNode *bin) {
  // Handle long chains of +/- non-recursively.
  if (bin->_operator->str() == "+" || bin->_operator->str() == "-") {
    auto list = linearizeLeft(bin, {"+", "-"});

    Value *LHS = genExpression(list[0]->_left);
    for (auto *e : list) {
      Value *RHS = genExpression(e->_right);
      Builder.setLocation(e->getDebugLoc());
      auto Kind = BinaryOperatorInst::parseOperator(e->_operator->str());
      BinaryOperatorInst *result =
          Builder.createBinaryOperatorInst(LHS, RHS, Kind);
      LHS = result;
    }

    return LHS;
  }

  Value *LHS = genExpression(bin->_left);
  Value *RHS = genExpression(bin->_right);

  ValueKind kind = BinaryOperatorInst::parseOperator(bin->_operator->str());
  Instruction *res = Builder.createBinaryOperatorInst(LHS, RHS, kind);

  // If the binary operator is a comparison, set the result type to boolean.
  static_assert(
      (int)ValueKind::BinaryGreaterThanOrEqualInstKind -
              (int)ValueKind::BinaryEqualInstKind + 1 ==
          8,
      "Comparison instructions must be consecutive");
  if ((kind >= ValueKind::BinaryEqualInstKind &&
       kind <= ValueKind::BinaryGreaterThanOrEqualInstKind) ||
      kind == ValueKind::BinaryInInstKind ||
      kind == ValueKind::BinaryInstanceOfInstKind) {
    res->setType(Type::createBoolean());
  }

  return res;
}

Value *ESTreeIRGen::genUnaryExpression(ESTree::UnaryExpressionNode *U) {
  UniqueString *oper = U->_operator;

  // Handle the delete unary expression. https://es5.github.io/#x11.4.1
  if (oper == kw_.identDelete) {
    if (auto *memberExpr =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(U->_argument)) {
      LLVM_DEBUG(llvh::dbgs() << "IRGen delete member expression.\n");

      return genMemberExpression(memberExpr, MemberExpressionOperation::Delete)
          .result;
    }

    if (auto *memberExpr = llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(
            U->_argument)) {
      LLVM_DEBUG(llvh::dbgs() << "IRGen delete optional member expression.\n");

      return genOptionalMemberExpression(
                 memberExpr, nullptr, MemberExpressionOperation::Delete)
          .result;
    }

    // Check for "delete identifier". Note that deleting unqualified identifiers
    // is prohibited in strict mode, so that case is handled earlier in the
    // semantic validator. Here we are left to handle the non-strict mode case.
    if (auto *iden = llvh::dyn_cast<ESTree::IdentifierNode>(U->_argument)) {
      assert(
          !curFunction()->function->isStrictMode() &&
          "delete identifier encountered in strict mode");
      // Check if this is a known variable.
      Identifier name = getNameFieldFromID(iden);
      auto *var = resolveIdentifier(iden);

      if (llvh::isa<GlobalObjectProperty>(var)) {
        // If the variable doesn't exist or if it is global, we must generate
        // a delete global property instruction.
        return Builder.createDeletePropertyInst(
            Builder.getGlobalObject(), Builder.getLiteralString(name));
      } else {
        // Otherwise it is a local variable which can't be deleted and we just
        // return false.
        return Builder.getLiteralBool(false);
      }
    }

    // Generate the code for the delete operand.
    genExpression(U->_argument);

    // Deleting any value or a result of an expression returns True.
    return Builder.getLiteralBool(true);
  }

  // Need to handle the special case of "typeof <undefined variable>".
  if (oper == kw_.identTypeof) {
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(U->_argument)) {
      Value *argument = genIdentifierExpression(id, true);
      return Builder.createTypeOfInst(argument);
    }
  }

  // Generate the unary operand:
  Value *argument = genExpression(U->_argument);

  if (oper == kw_.identPlus) {
    return Builder.createAsNumberInst(argument);
  } else if (oper == kw_.identTypeof) {
    return Builder.createTypeOfInst(argument);
  } else {
    ValueKind kind = UnaryOperatorInst::parseOperator(oper->str());

    // Set the type of unary operators that have a fixed result type.
    Type type = Type::createAnyType();
    switch (kind) {
      case ValueKind::UnaryVoidInstKind:
        type = Type::createUndefined();
        break;
      case ValueKind::UnaryBangInstKind:
        type = Type::createBoolean();
        break;
      default:
        break;
    }
    return Builder.createUnaryOperatorInst(argument, kind, type);
  }
}

Value *ESTreeIRGen::genUpdateExpr(ESTree::UpdateExpressionNode *updateExpr) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen update expression.\n");
  bool isPrefix = updateExpr->_prefix;

  ValueKind opKind;
  if (updateExpr->_operator->str() == "++") {
    opKind = ValueKind::UnaryIncInstKind;
  } else if (updateExpr->_operator->str() == "--") {
    opKind = ValueKind::UnaryDecInstKind;
  } else {
    llvm_unreachable("Invalid update operator");
  }

  LReference lref = createLRef(updateExpr->_argument, false);

  // Load the original value.
  Value *original = enforceExprType(lref.emitLoad(), updateExpr->_argument);

  // Postfix updates need to convert the original value to numeric before
  // Inc/Dec to ensure the updateExpr has the proper result value.
  if (!isPrefix && !original->getType().isSubsetOf(Type::createNumeric()))
    original = Builder.createAsNumericInst(original);

  // Create the inc or dec.
  Value *result = Builder.createUnaryOperatorInst(original, opKind);
  // Number and BigInt types are preserved, otherwise the result is numeric.
  result->setType(
      original->getType().isNumberType() || original->getType().isBigIntType()
          ? original->getType()
          : Type::createNumeric());

  result = enforceExprType(result, updateExpr);

  // Store the result.
  lref.emitStore(result);

  // Depending on the prefixness return the previous value or the modified
  // value.
  return (isPrefix ? result : original);
}

/// Extract a name hint from a LReference.
static Identifier extractNameHint(const LReference &lref) {
  Identifier nameHint{};
  if (auto *var = lref.castAsVariable()) {
    nameHint = var->getName();
  } else if (auto *globProp = lref.castAsGlobalObjectProperty()) {
    nameHint = globProp->getName()->getValue();
  }
  return nameHint;
}

Value *ESTreeIRGen::genAssignmentExpr(ESTree::AssignmentExpressionNode *AE) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen assignment operator.\n");

  auto opStr = AE->_operator->str();

  // Handle nested normal assignments non-recursively.
  if (opStr == "=") {
    auto list = ESTree::linearizeRight(AE, {"="});

    // Create an LReference for every assignment left side.
    llvh::SmallVector<LReference, 1> lrefs;
    lrefs.reserve(list.size());
    for (auto *e : list) {
      lrefs.push_back(createLRef(e->_left, false));
    }

    Value *RHS = nullptr;
    auto lrefIterator = lrefs.end();
    for (auto *e : llvh::make_range(list.rbegin(), list.rend())) {
      --lrefIterator;
      if (!RHS)
        RHS = genExpression(e->_right, extractNameHint(*lrefIterator));
      Builder.setLocation(e->getDebugLoc());
      lrefIterator->emitStore(RHS);
    }

    return RHS;
  }

  assert(opStr != "=" && "Assignment was already handled");

  // The implicit cast may exist here to signal that the result of the binary
  // expression should be implicitly casted, so skip it when computing the lref.
  auto *optImplicitCast =
      llvh::dyn_cast<ESTree::ImplicitCheckedCastNode>(AE->_left);
  ESTree::Node *left = optImplicitCast ? optImplicitCast->_argument : AE->_left;

  LReference lref = createLRef(left, false);
  Identifier nameHint = extractNameHint(lref);

  auto logicalAssign =
      llvh::StringSwitch<OptValue<LogicalAssignmentOp>>(opStr)
          .Case("||=", LogicalAssignmentOp::ShortCircuitOrKind)
          .Case("&&=", LogicalAssignmentOp::ShortCircuitAndKind)
          .Case("\?\?=", LogicalAssignmentOp::NullishCoalesceKind)
          .Default(llvh::None);
  if (logicalAssign) {
    return genLogicalAssignmentExpr(AE, *logicalAssign, lref, nameHint);
  }

  auto AssignmentKind = BinaryOperatorInst::parseAssignmentOperator(opStr);

  // Section 11.13.1 specifies that we should first load the
  // LHS before materializing the RHS. Unlike in C, this
  // code is well defined: "x+= x++".
  // https://es5.github.io/#x11.13.1
  Value *V = lref.emitLoad();
  V = enforceExprType(V, left);

  Value *RHS = genExpression(AE->_right, nameHint);
  Value *result;
  result = Builder.createBinaryOperatorInst(V, RHS, AssignmentKind);

  // Cast the result of the binary op if it was on the LHS.
  if (optImplicitCast) {
    result = Builder.createCheckedTypeCastInst(
        result,
        flowTypeToIRType(flowContext_.getNodeTypeOrAny(optImplicitCast)));
  }

  result = enforceExprType(result, AE);
  lref.emitStore(result);

  // Return the value that we stored as the result of the expression.
  return result;
}

Value *ESTreeIRGen::genRegExpLiteral(ESTree::RegExpLiteralNode *RE) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen reg exp literal.\n");
  LLVM_DEBUG(
      llvh::dbgs() << "Loading regexp Literal \"" << RE->_pattern->str()
                   << " / " << RE->_flags->str() << "\"\n");
  auto exp = Builder.createRegExpInst(
      Identifier::getFromPointer(RE->_pattern),
      Identifier::getFromPointer(RE->_flags));

  auto &regexp = Builder.getModule()->getContext().getCompiledRegExp(
      RE->_pattern, RE->_flags);

  if (regexp.getMapping().size()) {
    auto &mapping = regexp.getMapping();
    AllocObjectLiteralInst::ObjectPropertyMap propMap;
    for (auto &identifier : regexp.getOrderedGroupNames()) {
      std::string converted;
      convertUTF16ToUTF8WithSingleSurrogates(converted, identifier);
      auto *key = Builder.getLiteralString(converted);
      auto groupIdxRes = mapping.find(identifier);
      assert(
          groupIdxRes != mapping.end() &&
          "identifier not found in named groups");
      auto groupIdx = groupIdxRes->second;
      auto *val = Builder.getLiteralNumber(groupIdx);
      propMap.emplace_back(key, val);
    }
    auto literalObj = Builder.createAllocObjectLiteralInst(propMap);

    Value *params[] = {exp, literalObj};
    Builder.createCallBuiltinInst(
        BuiltinMethod::HermesBuiltin_initRegexNamedGroups, params);
  }

  return exp;
}

Value *ESTreeIRGen::genLogicalAssignmentExpr(
    ESTree::AssignmentExpressionNode *AE,
    LogicalAssignmentOp assignmentKind,
    LReference lref,
    Identifier nameHint) {
  // Logical assignment expressions must use short-circuiting logic.
  // BB which actually performs the assignment.
  BasicBlock *assignBB = Builder.createBasicBlock(Builder.getFunction());
  // BB which simply continues without performing the assignment.
  BasicBlock *continueBB = Builder.createBasicBlock(Builder.getFunction());
  auto *lhs = lref.emitLoad();

  PhiInst::ValueListType values;
  PhiInst::BasicBlockListType blocks;

  values.push_back(lhs);
  blocks.push_back(Builder.getInsertionBlock());

  switch (assignmentKind) {
    case LogicalAssignmentOp::ShortCircuitOrKind:
      Builder.createCondBranchInst(lhs, continueBB, assignBB);
      break;
    case LogicalAssignmentOp::ShortCircuitAndKind:
      Builder.createCondBranchInst(lhs, assignBB, continueBB);
      break;
    case LogicalAssignmentOp::NullishCoalesceKind:
      Builder.createCondBranchInst(
          Builder.createBinaryOperatorInst(
              lhs, Builder.getLiteralNull(), ValueKind::BinaryEqualInstKind),
          assignBB,
          continueBB);
      break;
    default:
      llvm_unreachable("invalid AssignmentKind in this branch");
  }

  Builder.setInsertionBlock(assignBB);
  auto *rhs = genExpression(AE->_right, nameHint);
  auto *result = rhs;
  lref.emitStore(result);
  values.push_back(result);
  blocks.push_back(Builder.getInsertionBlock());
  Builder.createBranchInst(continueBB);

  Builder.setInsertionBlock(continueBB);
  // Final result is either the original value or the value assigned,
  // depending on which branch was taken.
  return Builder.createPhiInst(std::move(values), std::move(blocks));
}

Value *ESTreeIRGen::genConditionalExpr(ESTree::ConditionalExpressionNode *C) {
  auto parentFunc = Builder.getInsertionBlock()->getParent();

  PhiInst::ValueListType values;
  PhiInst::BasicBlockListType blocks;

  auto alternateBlock = Builder.createBasicBlock(parentFunc);
  auto consequentBlock = Builder.createBasicBlock(parentFunc);
  auto continueBlock = Builder.createBasicBlock(parentFunc);

  // Implement the ternary operator using control flow. We must use control
  // flow because the expressions may have side effects.
  genExpressionBranch(C->_test, consequentBlock, alternateBlock, nullptr);

  // The 'then' side:
  Builder.setInsertionBlock(consequentBlock);

  values.push_back(genExpression(C->_consequent));
  blocks.push_back(Builder.getInsertionBlock());
  Builder.createBranchInst(continueBlock);

  // The 'else' side:
  Builder.setInsertionBlock(alternateBlock);
  values.push_back(genExpression(C->_alternate));
  blocks.push_back(Builder.getInsertionBlock());
  Builder.createBranchInst(continueBlock);

  // Continue:
  Builder.setInsertionBlock(continueBlock);
  return Builder.createPhiInst(values, blocks);
}

Value *ESTreeIRGen::genIdentifierExpression(
    ESTree::IdentifierNode *Iden,
    bool afterTypeOf) {
  auto StrName = getNameFieldFromID(Iden);
  auto *decl = getIDDecl(Iden);
  auto *Var = getDeclData(decl);

  if (decl->special == sema::Decl::Special::Arguments) {
    // If it is captured, we must use the captured value.
    if (curFunction()->capturedState.arguments) {
      auto *capturedArguments = curFunction()->capturedState.arguments;
      auto *RSI = emitResolveScopeInstIfNeeded(capturedArguments->getParent());
      return Builder.createLoadFrameInst(RSI, capturedArguments);
    }

    return curFunction()->createArgumentsInst;
  }

  // For uses of undefined/Infinity/NaN as the global property, we make an
  // optimization to always return the constant directly.
  if (llvh::isa<GlobalObjectProperty>(Var)) {
    if (StrName.getUnderlyingPointer() == kw_.identUndefined) {
      return Builder.getLiteralUndefined();
    }
    if (StrName.getUnderlyingPointer() == kw_.identInfinity) {
      return Builder.getLiteralInfinity();
    }
    if (StrName.getUnderlyingPointer() == kw_.identNaN) {
      return Builder.getLiteralNaN();
    }
  }

  LLVM_DEBUG(
      llvh::dbgs() << "Found variable " << StrName << " accessed in function \""
                   << curFunction()->function->getInternalNameStr() << "\"\n");

  // Typeof <variable> does not throw.
  return emitLoad(Var, afterTypeOf);
}

Value *ESTreeIRGen::genMetaProperty(ESTree::MetaPropertyNode *MP) {
  // Recognize "new.target"
  if (cast<ESTree::IdentifierNode>(MP->_meta)->_name == kw_.identNew) {
    if (cast<ESTree::IdentifierNode>(MP->_property)->_name == kw_.identTarget) {
      return genNewTarget();
    }
  }

  llvm_unreachable("invalid MetaProperty");
}

Value *ESTreeIRGen::genNewTarget() {
  Value *value;

  switch (curFunction()->function->getDefinitionKind()) {
    case Function::DefinitionKind::ES5Function:
    case Function::DefinitionKind::ES6Constructor:
      value = Builder.createGetNewTargetInst(
          curFunction()->function->getNewTargetParam());
      break;
    case Function::DefinitionKind::GeneratorInnerArrow:
    case Function::DefinitionKind::ES6Arrow:
      value = curFunction()->capturedState.newTarget;
      break;
    // Generators cannot be invoked with new, so new.target will be
    // undefined.
    case Function::DefinitionKind::GeneratorInner:
    case Function::DefinitionKind::ES6Method:
      value = Builder.getLiteralUndefined();
      break;
  }

  // If it is a variable, we must issue a load.
  if (auto *V = llvh::dyn_cast<Variable>(value)) {
    auto *RSI = emitResolveScopeInstIfNeeded(V->getParent());
    return Builder.createLoadFrameInst(RSI, V);
  }

  return value;
}

Value *ESTreeIRGen::genNewExpr(ESTree::NewExpressionNode *N) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen 'new' statement/expression.\n");

  Value *callee = genExpression(N->_callee);

  bool hasSpread = false;
  for (auto &arg : N->_arguments) {
    if (llvh::isa<ESTree::SpreadElementNode>(&arg)) {
      hasSpread = true;
    }
  }

  // Is this a statically typed new?
  if (auto *consType = llvh::dyn_cast<flow::ClassConstructorType>(
          flowContext_.getNodeTypeOrAny(N->_callee)->info)) {
    flow::ClassType *classType = consType->getClassTypeInfo();
    assert(!hasSpread && "statically typed spread is not supported");

    // If we know the ClassType but it hasn't been populated in
    // classConstructors_ yet, then it must occur later in the same function.
    // If it was created in a surrounding function and captured, the function
    // queueing mechanism would result in compiling this CallInst after
    // it anyway.
    // This is an error, since we can statically determine that the constructor
    // will not have been populated.
    // TODO: This error should be raised earlier, when the callee is loaded.
    auto it = classConstructors_.find(classType);
    if (it == classConstructors_.end()) {
      Builder.getModule()->getContext().getSourceErrorManager().error(
          N->_callee->getSourceRange(),
          Twine("Cannot construct class before its definition."));
      return Builder.getLiteralUndefined();
    }

    // Since the callee has already been loaded at this point, we know that the
    // constructor has already beenn created and initialized. This means that we
    // do not need to perform any TDZ check on the home object variable.
    auto *RSI =
        emitResolveScopeInstIfNeeded(it->second.homeObjectVar->getParent());
    auto *proto = Builder.createUnionNarrowTrustedInst(
        Builder.createLoadFrameInst(RSI, it->second.homeObjectVar),
        Type::createObject());
    Value *newInst = emitClassAllocation(classType, proto);

    // Call the constructor, if necessary.  There is always a constructor,
    // either explicit or implicit.  We will load an implicit ctor (for
    // TDZ), but there is no need to invoke it unless it performs field
    // initializations (which is true iff it is present in the
    // classFieldInitInfo_ table).
    if (classType->getConstructorType() ||
        (classFieldInitInfo_.find(classType) != classFieldInitInfo_.end())) {
      CallInst::ArgumentList args;
      for (auto &arg : N->_arguments)
        args.push_back(genExpression(&arg));

      Function *target = it->second.constructorFunc;
      Builder.createCallInst(
          callee,
          target,
          /* calleeIsAlwaysClosure */ true,
          Builder.getEmptySentinel(),
          callee,
          newInst,
          args);
    }
    return newInst;
  }

  if (!hasSpread) {
    CallInst::ArgumentList args;
    for (auto &arg : N->_arguments) {
      args.push_back(genExpression(&arg));
    }
    auto *thisArg =
        Builder.createCreateThisInst(callee, Builder.getEmptySentinel());
    auto *res = Builder.createCallInst(callee, callee, thisArg, args);
    return Builder.createGetConstructedObjectInst(thisArg, res);
  }

  // Otherwise, there exists a spread argument, so the number of arguments
  // is variable.
  // Generate IR for this by creating an array and populating it with the
  // arguments, then calling HermesInternal.apply.
  auto *args = genArrayFromElements(N->_arguments);

  return genBuiltinCall(BuiltinMethod::HermesBuiltin_apply, {callee, args});
}

Value *ESTreeIRGen::genLogicalExpression(
    ESTree::LogicalExpressionNode *logical) {
  auto opStr = logical->_operator->str();
  LLVM_DEBUG(llvh::dbgs() << "IRGen of short circuiting: " << opStr << ".\n");

  enum class Kind {
    And, // &&
    Or, // ||
    Coalesce, // ??
  };

  Kind kind;

  if (opStr == "&&") {
    kind = Kind::And;
  } else if (opStr == "||") {
    kind = Kind::Or;
  } else if (opStr == "??") {
    kind = Kind::Coalesce;
  } else {
    llvm_unreachable("Invalid update operator");
  }

  // Generate a new temporary stack allocation.
  auto tempVarName = genAnonymousLabelName("logical");
  auto parentFunc = Builder.getInsertionBlock()->getParent();
  auto tempVar =
      Builder.createAllocStackInst(tempVarName, Type::createAnyType());

  auto evalRHSBlock = Builder.createBasicBlock(parentFunc);
  auto continueBlock = Builder.createBasicBlock(parentFunc);

  auto LHS = genExpression(logical->_left);

  // Store the LHS value of the expression in preparation for the case where we
  // won't need to evaluate the RHS side of the expression. In that case, we
  // jump to continueBlock, which returns tempVar.
  Builder.createStoreStackInst(LHS, tempVar);

  // Notice that instead of negating the condition we swap the operands of the
  // branch.
  switch (kind) {
    case Kind::And:
      // Evaluate RHS only when the LHS is true.
      Builder.createCondBranchInst(LHS, evalRHSBlock, continueBlock);
      break;
    case Kind::Or:
      // Evaluate RHS only when the LHS is false.
      Builder.createCondBranchInst(LHS, continueBlock, evalRHSBlock);
      break;
    case Kind::Coalesce:
      // Evaluate RHS only if the value is undefined or null.
      // Use == instead of === to account for both values at once.
      Builder.createCondBranchInst(
          Builder.createBinaryOperatorInst(
              LHS, Builder.getLiteralNull(), ValueKind::BinaryEqualInstKind),
          evalRHSBlock,
          continueBlock);

      break;
  }

  // Continue the evaluation of the right-hand-side of the expression.
  Builder.setInsertionBlock(evalRHSBlock);
  auto RHS = genExpression(logical->_right);

  // Evaluate the RHS and store the result into the temporary variable.
  Builder.createStoreStackInst(RHS, tempVar);

  // Finally, jump to the continuation block.
  Builder.createBranchInst(continueBlock);

  // Load the content of the temp variable that was set in one of the branches.
  Builder.setInsertionBlock(continueBlock);
  return Builder.createLoadStackInst(tempVar);
}

Value *ESTreeIRGen::genThisExpression() {
  // Generators may be used as arrows in async arrow functions, in which case
  // they should load from the captured this.
  auto funcDefKind = curFunction()->function->getDefinitionKind();
  if ((funcDefKind == Function::DefinitionKind::ES6Arrow) ||
      (funcDefKind == Function::DefinitionKind::GeneratorInnerArrow)) {
    assert(
        curFunction()->capturedState.thisVal &&
        "arrow function must have a captured this");
    auto *RSI = emitResolveScopeInstIfNeeded(
        curFunction()->capturedState.thisVal->getParent());
    return Builder.createLoadFrameInst(
        RSI, curFunction()->capturedState.thisVal);
  }
  return curFunction()->jsParams[0];
}

void ESTreeIRGen::genLogicalExpressionBranch(
    ESTree::LogicalExpressionNode *logical,
    BasicBlock *onTrue,
    BasicBlock *onFalse,
    BasicBlock *onNullish) {
  auto opStr = logical->_operator->str();
  LLVM_DEBUG(
      llvh::dbgs() << "IRGen of short circuiting: " << opStr << " branch.\n");

  auto parentFunc = Builder.getInsertionBlock()->getParent();
  auto *block = Builder.createBasicBlock(parentFunc);

  if (opStr == "&&") {
    genExpressionBranch(logical->_left, block, onFalse, onNullish);
  } else if (opStr == "||") {
    genExpressionBranch(logical->_left, onTrue, block, onNullish);
  } else {
    assert(opStr == "??" && "invalid logical operator");
    genExpressionBranch(logical->_left, onTrue, onFalse, block);
  }

  Builder.setInsertionBlock(block);
  genExpressionBranch(logical->_right, onTrue, onFalse, onNullish);
}

Value *ESTreeIRGen::genTemplateLiteralExpr(ESTree::TemplateLiteralNode *Expr) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen 'TemplateLiteral' expression.\n");

  assert(
      Expr->_quasis.size() == Expr->_expressions.size() + 1 &&
      "The string count should always be one more than substitution count.");

  // Construct an argument list for calling HermesInternal.concat():
  // cookedStr0, substitution0, cookedStr1, ..., substitutionN, cookedStrN + 1,
  // skipping any empty string, except for the first cooked string, which is
  // going to be the `this` to the concat call.

  // Get the first cooked string.
  auto strItr = Expr->_quasis.begin();
  auto *tempEltNode = cast<ESTree::TemplateElementNode>(&*strItr);
  auto *firstCookedStr = Builder.getLiteralString(tempEltNode->_cooked->str());
  ++strItr;
  // If the template literal is effectively only one string, directly return it.
  if (strItr == Expr->_quasis.end()) {
    return firstCookedStr;
  }
  // argList includes firstCookedStr as the first element.
  CallInst::ArgumentList argList;
  argList.push_back(firstCookedStr);
  // Whether all the expressions are strings.
  // Determines if we can use StringConcatInst.
  bool allStrings = true;
  auto exprItr = Expr->_expressions.begin();
  while (strItr != Expr->_quasis.end()) {
    auto *sub = genExpression(&*exprItr);
    if (!sub->getType().isStringType()) {
      // Uses the IR type instead of the FlowContext type because
      // StringConcat needs to know that its operands are String types in the
      // IR, and not all values have their types correctly brought over from the
      // annotated AST.
      allStrings = false;
    }
    argList.push_back(sub);
    tempEltNode = cast<ESTree::TemplateElementNode>(&*strItr);
    auto cookedStr = tempEltNode->_cooked->str();
    if (!cookedStr.empty()) {
      argList.push_back(Builder.getLiteralString(cookedStr));
    }
    ++strItr;
    ++exprItr;
  }
  assert(
      exprItr == Expr->_expressions.end() &&
      "All the substitutions must have been collected.");

  if (allStrings) {
    // Fast path: all the substitutions are strings, so we can use
    // StringConcatInst.
    return Builder.createStringConcatInst(argList);
  } else {
    // Generate a function call to HermesInternal.concat() with these arguments.
    return genHermesInternalCall(
        "concat", argList[0], llvh::ArrayRef<Value *>(argList).drop_front(1));
  }
}

Value *ESTreeIRGen::genTaggedTemplateExpr(
    ESTree::TaggedTemplateExpressionNode *Expr) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen 'TaggedTemplateExpression' expression.\n");
  // Step 1: get the template object.
  auto *templateLit = cast<ESTree::TemplateLiteralNode>(Expr->_quasi);

  // Empty if same as rawStringArgs.
  llvh::SmallVector<Value *, 2> cookedStrings{};

  Module::RawStringList rawStringsForMap;
  // dup is true if the cooked strings and raw strings are duplicated.
  bool dup = true;
  for (auto &n : templateLit->_quasis) {
    auto element = cast<ESTree::TemplateElementNode>(&n);
    if (element->_cooked != element->_raw) {
      dup = false;
    }
    rawStringsForMap.push_back(Builder.getLiteralString(element->_raw->str()));
  }

  // Retrieve template object ID.
  // rawStringsForMap is moved, rawStrings is a reference to the same array now
  // that it lives in the map.
  auto [rawStrings, templateObjID] =
      Mod->emplaceTemplateObject(std::move(rawStringsForMap));

  // If the cooked strings are not the same as raw strings, append them to
  // argument list.
  if (!dup) {
    for (auto &node : templateLit->_quasis) {
      auto *templateElt = cast<ESTree::TemplateElementNode>(&node);
      if (templateElt->_cooked) {
        cookedStrings.push_back(
            Builder.getLiteralString(templateElt->_cooked->str()));
      } else {
        cookedStrings.push_back(Builder.getLiteralUndefined());
      }
    }
  }

  auto *templateObj = Builder.createGetTemplateObjectInst(
      templateObjID, dup, rawStrings, cookedStrings);

  // Step 2: call the tag function, passing the template object followed by a
  // list of substitutions as arguments.
  CallInst::ArgumentList tagFuncArgList;
  tagFuncArgList.push_back(templateObj);
  for (auto &sub : templateLit->_expressions) {
    tagFuncArgList.push_back(genExpression(&sub));
  }

  Value *callee;
  Value *thisVal;
  // Tag function is a member expression.
  if (auto *Mem = llvh::dyn_cast<ESTree::MemberExpressionNode>(Expr->_tag)) {
    Value *obj = genExpression(Mem->_object);
    Value *prop = genMemberExpressionProperty(Mem);
    // Call the callee with obj as the 'this'.
    thisVal = obj;
    callee = Builder.createLoadPropertyInst(obj, prop);
  } else {
    thisVal = Builder.getLiteralUndefined();
    callee = genExpression(Expr->_tag);
  }

  return Builder.createCallInst(
      callee,
      /* newTarget */ Builder.getLiteralUndefined(),
      thisVal,
      tagFuncArgList);
}

} // namespace irgen
} // namespace hermes
