/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

namespace hermes {
namespace irgen {

Value *ESTreeIRGen::genExpression(ESTree::Node *expr, Identifier nameHint) {
  LLVM_DEBUG(
      dbgs() << "IRGen expression of type " << expr->getNodeName() << "\n");
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
    LLVM_DEBUG(dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralString(Lit->_value->str());
  }

  // Handle Regexp Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-regular-expression-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::RegExpLiteralNode>(expr)) {
    LLVM_DEBUG(
        dbgs() << "Loading regexp Literal \"" << Lit->_pattern->str() << " / "
               << Lit->_flags->str() << "\"\n");

    return Builder.createRegExpInst(
        Identifier::getFromPointer(Lit->_pattern),
        Identifier::getFromPointer(Lit->_flags));
  }

  // Handle Boolean Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-boolean-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::BooleanLiteralNode>(expr)) {
    LLVM_DEBUG(dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralBool(Lit->_value);
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::NumericLiteralNode>(expr)) {
    LLVM_DEBUG(dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralNumber(Lit->_value);
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
    Value *LHS = genExpression(Bin->_left);
    Value *RHS = genExpression(Bin->_right);
    auto cookie = instrumentIR_.preBinaryExpression(Bin, LHS, RHS);

    auto Kind = BinaryOperatorInst::parseOperator(Bin->_operator->str());

    BinaryOperatorInst *result =
        Builder.createBinaryOperatorInst(LHS, RHS, Kind);
    return instrumentIR_.postBinaryExpression(Bin, cookie, result, LHS, RHS);
  }

  // Handle Unary operator Expressions.
  if (auto *U = llvh::dyn_cast<ESTree::UnaryExpressionNode>(expr)) {
    return genUnaryExpression(U);
  }

  // Handle the 'this' keyword.
  if (llvh::isa<ESTree::ThisExpressionNode>(expr)) {
    if (curFunction()->function->getDefinitionKind() ==
        Function::DefinitionKind::ES6Arrow) {
      assert(
          curFunction()->capturedThis &&
          "arrow function must have a captured this");
      return Builder.createLoadFrameInst(curFunction()->capturedThis);
    }
    return curFunction()->function->getThisParameter();
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
      switch (UnaryOperatorInst::parseOperator(e->_operator->str())) {
        case UnaryOperatorInst::OpKind::BangKind:
          // Do not propagate onNullish here because !expr cannot be nullish.
          return genExpressionBranch(e->_argument, onFalse, onTrue, nullptr);
        default:
          break;
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
        condVal,
        Builder.getLiteralNull(),
        BinaryOperatorInst::OpKind::EqualKind);
    BasicBlock *notNullishBB = Builder.createBasicBlock(Builder.getFunction());
    Builder.createCondBranchInst(isNullish, onNullish, notNullishBB);
    Builder.setInsertionBlock(notNullishBB);
  }
  Builder.createCondBranchInst(condVal, onTrue, onFalse);
}

Value *ESTreeIRGen::genArrayFromElements(ESTree::NodeList &list) {
  LLVM_DEBUG(dbgs() << "Initializing a new array\n");
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
    nextIndex = Builder.createAllocStackInst("nextIndex");
    Builder.createStoreStackInst(Builder.getLiteralPositiveZero(), nextIndex);
  }

  bool consecutive = true;
  auto codeGenOpts = Mod->getContext().getCodeGenerationSettings();
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
    if (!value ||
        (!llvh::isa<Literal>(value) && !codeGenOpts.unlimitedRegisters) ||
        isSpread) {
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
          Builder.createBinaryOperatorInst(
              Builder.createLoadStackInst(nextIndex),
              Builder.getLiteralNumber(1),
              BinaryOperatorInst::OpKind::AddKind),
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
  if (count > 0 && llvh::isa<ESTree::EmptyNode>(&list.back())) {
    // Last element is an elision, VM cannot derive the length properly.
    // We have to explicitly set it.
    Builder.createStorePropertyInst(
        Builder.getLiteralNumber(count), allocArrayInst, StringRef("length"));
  }
  return allocArrayInst;
}

Value *ESTreeIRGen::genArrayExpr(ESTree::ArrayExpressionNode *Expr) {
  return genArrayFromElements(Expr->_elements);
}

Value *ESTreeIRGen::genCallExpr(ESTree::CallExpressionNode *call) {
  LLVM_DEBUG(dbgs() << "IRGen 'call' statement/expression.\n");

  // Check for a direct call to eval().
  if (auto *identNode = llvh::dyn_cast<ESTree::IdentifierNode>(call->_callee)) {
    if (Identifier::getFromPointer(identNode->_name) == identEval_) {
      auto *evalVar = nameTable_.lookup(identEval_);
      if (!evalVar || llvh::isa<GlobalObjectProperty>(evalVar))
        return genCallEvalExpr(call);
    }
  }

  Value *thisVal;
  Value *callee;

  // Handle MemberExpression expression calls that sets the 'this' property.
  if (auto *Mem = llvh::dyn_cast<ESTree::MemberExpressionNode>(call->_callee)) {
    MemberExpressionResult memResult =
        genMemberExpression(Mem, MemberExpressionOperation::Load);

    // Call the callee with obj as the 'this' pointer.
    thisVal = memResult.base;
    callee = memResult.result;
  } else if (
      auto *Mem =
          llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(call->_callee)) {
    MemberExpressionResult memResult = genOptionalMemberExpression(
        Mem, nullptr, MemberExpressionOperation::Load);

    // Call the callee with obj as the 'this' pointer.
    thisVal = memResult.base;
    callee = memResult.result;
  } else {
    thisVal = Builder.getLiteralUndefined();
    callee = genExpression(call->_callee);
  }

  return emitCall(call, callee, thisVal);
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
            callee,
            Builder.getLiteralNull(),
            BinaryOperatorInst::OpKind::EqualKind),
        shortCircuitBB,
        evalRHSBB);

    // baseValue is not undefined or null.
    Builder.setInsertionBlock(evalRHSBB);
  }

  Value *callResult = emitCall(call, callee, thisVal);

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

Value *ESTreeIRGen::emitCall(
    ESTree::CallExpressionLikeNode *call,
    Value *callee,
    Value *thisVal) {
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

    return Builder.createCallInst(callee, thisVal, args);
  }

  // Otherwise, there exists a spread argument, so the number of arguments
  // is variable.
  // Generate IR for this by creating an array and populating it with the
  // arguments, then calling HermesInternal.apply.
  auto *args = genArrayFromElements(getArguments(call));
  return genBuiltinCall(
      BuiltinMethod::HermesBuiltin_apply, {callee, args, thisVal});
}

ESTreeIRGen::MemberExpressionResult ESTreeIRGen::genMemberExpression(
    ESTree::MemberExpressionNode *mem,
    MemberExpressionOperation op) {
  Value *baseValue = genExpression(mem->_object);
  Value *prop = genMemberExpressionProperty(mem);
  switch (op) {
    case MemberExpressionOperation::Load:
      return MemberExpressionResult{
          Builder.createLoadPropertyInst(baseValue, prop), baseValue};
    case MemberExpressionOperation::Delete:
      return MemberExpressionResult{
          Builder.createDeletePropertyInst(baseValue, prop), baseValue};
  }
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
            BinaryOperatorInst::OpKind::EqualKind),
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
    return {Builder.createPhiInst(values, blocks), baseValue};
  }

  // If this isn't the first optional, no Phi needed, just return the result.
  return {result, baseValue};
}

Value *ESTreeIRGen::genCallEvalExpr(ESTree::CallExpressionNode *call) {
  if (call->_arguments.empty()) {
    Mod->getContext().getSourceErrorManager().warning(
        call->getSourceRange(), "eval() without arguments returns undefined");
    return Builder.getLiteralUndefined();
  }

  Mod->getContext().getSourceErrorManager().warning(
      Warning::DirectEval,
      call->getSourceRange(),
      "Direct call to eval(), but lexical scope is not supported.");

  llvh::SmallVector<Value *, 1> args;
  for (auto &arg : call->_arguments) {
    args.push_back(genExpression(&arg));
  }

  if (args.size() > 1) {
    Mod->getContext().getSourceErrorManager().warning(
        call->getSourceRange(), "Extra eval() arguments are ignored");
  }

  return Builder.createDirectEvalInst(args[0]);
}

/// Convert a property key node to its JavaScript string representation.
static StringRef propertyKeyAsString(
    llvh::SmallVectorImpl<char> &storage,
    ESTree::Node *Key) {
  // Handle String Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-string-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::StringLiteralNode>(Key)) {
    LLVM_DEBUG(dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Lit->_value->str();
  }

  // Handle identifiers as if they are String Literals.
  if (auto *Iden = llvh::dyn_cast<ESTree::IdentifierNode>(Key)) {
    LLVM_DEBUG(dbgs() << "Loading String Literal \"" << Iden->_name << "\"\n");
    return Iden->_name->str();
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::NumericLiteralNode>(Key)) {
    LLVM_DEBUG(dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    storage.resize(NUMBER_TO_STRING_BUF_SIZE);
    auto len = numberToString(Lit->_value, storage.data(), storage.size());
    return StringRef(storage.begin(), len);
  }

  llvm_unreachable("Don't know this kind of property key");
  return StringRef();
}

Value *ESTreeIRGen::genObjectExpr(ESTree::ObjectExpressionNode *Expr) {
  LLVM_DEBUG(dbgs() << "Initializing a new object\n");

  /// Store information about a property. Is it an accessor (getter/setter) or
  /// a value, and the actual value.
  class PropertyValue {
   public:
    /// Is this a getter/setter value.
    bool isAccessor = false;
    /// Did we already generate IR to set this property.
    bool isIRGenerated = false;
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
  llvh::SmallVector<char, 32> stringStorage;
  /// The optional __proto__ property.
  ESTree::PropertyNode *protoProperty = nullptr;

  uint32_t numComputed = 0;

  for (auto &P : Expr->_properties) {
    if (llvh::isa<ESTree::SpreadElementNode>(&P))
      continue;

    // We are reusing the storage, so make sure it is cleared at every
    // iteration.
    stringStorage.clear();

    auto *prop = cast<ESTree::PropertyNode>(&P);
    if (prop->_computed) {
      // Can't store any useful information if the name is computed.
      // Just generate the code in the next loop.
      ++numComputed;
      continue;
    }

    auto propName = propertyKeyAsString(stringStorage, prop->_key);
    PropertyValue *propValue = &propMap[propName];

    if (prop->_kind->str() == "get") {
      propValue->setGetter(cast<ESTree::FunctionExpressionNode>(prop->_value));
    } else if (prop->_kind->str() == "set") {
      propValue->setSetter(cast<ESTree::FunctionExpressionNode>(prop->_value));
    } else {
      assert(prop->_kind->str() == "init" && "invalid PropertyNode kind");
      propValue->setValue(prop->_value);
      if (!protoProperty && propName == "__proto__")
        protoProperty = prop;
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
  auto Obj =
      Builder.createAllocObjectInst(propMap.size() + numComputed, objectParent);

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
      auto *value = genExpression(prop->_value);
      if (prop->_kind->str() == "get" || prop->_kind->str() == "set") {
        if (prop->_kind->str() == "get") {
          Builder.createStoreGetterSetterInst(
              value,
              Builder.getLiteralUndefined(),
              Obj,
              key,
              IRBuilder::PropEnumerable::Yes);
        } else {
          Builder.createStoreGetterSetterInst(
              Builder.getLiteralUndefined(),
              value,
              Obj,
              key,
              IRBuilder::PropEnumerable::Yes);
        }
      } else {
        Builder.createStoreOwnPropertyInst(
            value, Obj, key, IRBuilder::PropEnumerable::Yes);
      }
      haveSeenComputedProp = true;
      continue;
    }

    StringRef keyStr = propertyKeyAsString(stringStorage, prop->_key);
    PropertyValue *propValue = &propMap[keyStr];
    auto *Key = Builder.getLiteralString(keyStr);

    if (prop->_kind->str() == "get" || prop->_kind->str() == "set") {
      // If  we already generated it, skip.
      if (propValue->isIRGenerated)
        continue;

      if (!propValue->isAccessor) {
        // This property will be redefined in the end as non-accessor.
        // We need to store this property now otherwise we would break
        // the order of the properties. The only choice we have is to
        // store a placeholder first here.
        if (haveSeenComputedProp) {
          Builder.createStoreOwnPropertyInst(
              Builder.getLiteralUndefined(),
              Obj,
              Key,
              IRBuilder::PropEnumerable::Yes);
        } else {
          Builder.createStoreNewOwnPropertyInst(
              Builder.getLiteralUndefined(),
              Obj,
              Key,
              IRBuilder::PropEnumerable::Yes);
        }
        propValue->isIRGenerated = true;
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

      propValue->isIRGenerated = true;
    } else {
      // The __proto__ property requires special handling.
      if (keyStr == "__proto__") {
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
                BuiltinMethod::HermesBuiltin_silentSetPrototypeOf,
                {Obj, parent});
          }
        } else {
          // __proto__ was defined more than once, which is an error.
          Builder.getModule()->getContext().getSourceErrorManager().error(
              prop->getSourceRange(),
              "__proto__ was set multiple times in the object definition.");
          Builder.getModule()->getContext().getSourceErrorManager().note(
              protoProperty->getSourceRange(),
              "The first definition was here.");
        }

        continue;
      }

      // Always generate the values, even if we don't need it, for the side
      // effects.
      auto value =
          genExpression(prop->_value, Builder.createIdentifier(keyStr));

      // Only store the value if it won't be overwritten.
      if (propMap[keyStr].valueNode == prop->_value) {
        if (haveSeenComputedProp || propValue->isIRGenerated) {
          Builder.createStoreOwnPropertyInst(
              value, Obj, Key, IRBuilder::PropEnumerable::Yes);
        } else {
          Builder.createStoreNewOwnPropertyInst(
              value, Obj, Key, IRBuilder::PropEnumerable::Yes);
        }
        propValue->isIRGenerated = true;
      } else {
        Builder.getModule()->getContext().getSourceErrorManager().warning(
            propMap[keyStr].getSourceRange(),
            Twine("the property \"") + keyStr +
                "\" was set multiple times in the object definition.");

        Builder.getModule()->getContext().getSourceErrorManager().warning(
            prop->getSourceRange(), "The first definition was here.");
      }
    }
  }

  // Return the newly allocated object (because this is an expression, not a
  // statement).
  return Obj;
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

  auto *bb = Builder.getInsertionBlock();
  auto *next = Builder.createBasicBlock(bb->getParent());
  Value *value = Y->_argument ? genExpression(Y->_argument)
                              : Builder.getLiteralUndefined();

  auto *resumeIsReturn =
      Builder.createAllocStackInst(genAnonymousLabelName("isReturn"));

  Builder.createSaveAndYieldInst(value, next);
  Builder.setInsertionBlock(next);
  return genResumeGenerator(
      Y, resumeIsReturn, Builder.createBasicBlock(bb->getParent()));
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
/// - Otherwise, go to body
///
/// resume: Runs the ResumeGenerator instruction.
/// - Code for `finally` is also emitted here.
///
/// body: Yield the result of the next() call
/// - Calls HermesInternal.generatorSetDelegated so that the result is not
///   wrapped by the VM in an IterResult object.
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
  auto *bodyBlock = Builder.createBasicBlock(function);
  auto *exitBlock = Builder.createBasicBlock(function);

  // Calls ResumeGenerator and returns or throws if requested.
  auto *resumeBB = Builder.createBasicBlock(function);

  auto *exprValue = genExpression(Y->_argument);
  IteratorRecordSlow iteratorRecord = emitGetIteratorSlow(exprValue);

  // The "received" value when the user resumes the generator.
  // Initialized to undefined on the first run, then stored to immediately
  // following any genResumeGenerator.
  auto *received =
      Builder.createAllocStackInst(genAnonymousLabelName("received"));
  Builder.createStoreStackInst(Builder.getLiteralUndefined(), received);

  // The "isReturn" value when the user resumes the generator.
  // Stored to immediately following any genResumeGenerator.
  auto *resumeIsReturn =
      Builder.createAllocStackInst(genAnonymousLabelName("isReturn"));

  // The final result of the `yield*` expression.
  // This can be set from either the body or the handler, so it is placed
  // in the stack to allow populating it from anywhere.
  auto *result = Builder.createAllocStackInst(genAnonymousLabelName("result"));

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
      iteratorRecord.iterator,
      {Builder.createLoadStackInst(received)});
  emitEnsureObject(nextResult, "iterator.next() did not return an object");

  Builder.createStoreStackInst(nextResult, result);
  auto *done = emitIteratorCompleteSlow(nextResult);
  Builder.createCondBranchInst(done, exitBlock, bodyBlock);

  Builder.setInsertionBlock(bodyBlock);

  emitTryCatchScaffolding(
      getNextBlock,
      // emitBody.
      [this,
       Y,
       resumeIsReturn,
       getNextBlock,
       resumeBB,
       nextResult,
       received,
       &iteratorRecord]() {
        // Generate IR for the body of Try
        SurroundingTry thisTry{
            curFunction(),
            Y,
            {},
            [this, resumeBB, received, &iteratorRecord](
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
                Builder.createCompareBranchInst(
                    returnMethod,
                    Builder.getLiteralUndefined(),
                    BinaryOperatorInst::OpKind::StrictlyEqualKind,
                    noReturnBB,
                    haveReturnBB);

                Builder.setInsertionBlock(haveReturnBB);
                // iv. Let innerReturnResult be
                // ? Call(return, iterator, received.[[Value]]).
                auto *innerReturnResult = Builder.createCallInst(
                    returnMethod,
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
                genBuiltinCall(
                    BuiltinMethod::HermesBuiltin_generatorSetDelegated, {});
                Builder.createSaveAndYieldInst(innerReturnResult, resumeBB);

                // If return is undefined, return Completion(received).
                Builder.setInsertionBlock(noReturnBB);
              }
            }};

        // The primary call path for yielding the next result.
        genBuiltinCall(BuiltinMethod::HermesBuiltin_generatorSetDelegated, {});
        Builder.createSaveAndYieldInst(nextResult, resumeBB);

        // Note that resumeBB was created above to allow all SaveAndYield insts
        // to have the same resume point (including SaveAndYield in the catch
        // handler), but we must populate it inside the scaffolding so that the
        // SurroundingTry is correct for the genFinallyBeforeControlChange
        // call emitted by genResumeGenerator.
        Builder.setInsertionBlock(resumeBB);
        genResumeGenerator(Y, resumeIsReturn, getNextBlock, received);

        // SaveAndYieldInst is a Terminator, but emitTryCatchScaffolding
        // needs a block from which to Branch to the TryEnd instruction.
        // Make a dummy block which can do that.
        Builder.setInsertionBlock(
            Builder.createBasicBlock(Builder.getFunction()));
      },
      // emitNormalCleanup.
      []() {},
      // emitHandler.
      [this, resumeBB, exitBlock, result, &iteratorRecord](
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
        Builder.createCompareBranchInst(
            throwMethod,
            Builder.getLiteralUndefined(),
            BinaryOperatorInst::OpKind::StrictlyEqualKind,
            noThrowMethodBB,
            hasThrowMethodBB);

        // ii. If throw is not undefined, then
        Builder.setInsertionBlock(hasThrowMethodBB);
        // ii. 1. Let innerResult be
        //        ? Call(throw, iterator, « received.[[Value]] »).
        // ii. 3. NOTE: Exceptions from the inner iterator throw method are
        // propagated. Normal completions from an inner throw method are
        // processed similarly to an inner next.
        auto *innerResult = Builder.createCallInst(
            throwMethod, iteratorRecord.iterator, {catchReg});
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
        genBuiltinCall(BuiltinMethod::HermesBuiltin_generatorSetDelegated, {});
        Builder.createSaveAndYieldInst(innerResult, resumeBB);

        // NOTE: If iterator does not have a throw method, this throw is
        // going to terminate the yield* loop. But first we need to give
        // iterator a chance to clean up.
        Builder.setInsertionBlock(noThrowMethodBB);
        emitIteratorCloseSlow(iteratorRecord, false);
        genBuiltinCall(
            BuiltinMethod::HermesBuiltin_throwTypeError,
            {Builder.getLiteralString(
                "yield* delegate must have a .throw() method")});
        // HermesInternal.throwTypeError will necessarily throw, but we need to
        // have a terminator on this BB to allow proper optimization.
        Builder.createReturnInst(Builder.getLiteralUndefined());
      });

  Builder.setInsertionBlock(exitBlock);

  return emitIteratorValueSlow(Builder.createLoadStackInst(result));
}

Value *ESTreeIRGen::genResumeGenerator(
    ESTree::YieldExpressionNode *yield,
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
  if (yield) {
    genFinallyBeforeControlChange(
        curFunction()->surroundingTry, nullptr, ControlFlowChange::Break);
  }
  Builder.createReturnInst(resume);

  Builder.setInsertionBlock(nextBB);
  return resume;
}

Value *ESTreeIRGen::genUnaryExpression(ESTree::UnaryExpressionNode *U) {
  auto kind = UnaryOperatorInst::parseOperator(U->_operator->str());

  // Handle the delete unary expression. https://es5.github.io/#x11.4.1
  if (kind == UnaryOperatorInst::OpKind::DeleteKind) {
    if (auto *memberExpr =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(U->_argument)) {
      LLVM_DEBUG(dbgs() << "IRGen delete member expression.\n");

      return genMemberExpression(memberExpr, MemberExpressionOperation::Delete)
          .result;
    }

    if (auto *memberExpr = llvh::dyn_cast<ESTree::OptionalMemberExpressionNode>(
            U->_argument)) {
      LLVM_DEBUG(dbgs() << "IRGen delete optional member expression.\n");

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
      auto *var = nameTable_.lookup(name);

      if (!var || llvh::isa<GlobalObjectProperty>(var)) {
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
  if (kind == UnaryOperatorInst::OpKind::TypeofKind) {
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(U->_argument)) {
      Value *argument = genIdentifierExpression(id, true);
      return Builder.createUnaryOperatorInst(argument, kind);
    }
  }

  // Generate the unary operand:
  Value *argument = genExpression(U->_argument);
  auto *cookie = instrumentIR_.preUnaryExpression(U, argument);

  Value *result;
  if (kind == UnaryOperatorInst::OpKind::PlusKind)
    result = Builder.createAsNumberInst(argument);
  else
    result = Builder.createUnaryOperatorInst(argument, kind);
  return instrumentIR_.postUnaryExpression(U, cookie, result, argument);
}

Value *ESTreeIRGen::genUpdateExpr(ESTree::UpdateExpressionNode *updateExpr) {
  LLVM_DEBUG(dbgs() << "IRGen update expression.\n");
  bool isPrefix = updateExpr->_prefix;

  // The operands ++ and -- are equivalent to adding or subtracting the
  // literal 1.
  // See section 12.4.4.1.
  BinaryOperatorInst::OpKind opKind;
  if (updateExpr->_operator->str() == "++") {
    opKind = BinaryOperatorInst::OpKind::AddKind;
  } else if (updateExpr->_operator->str() == "--") {
    opKind = BinaryOperatorInst::OpKind::SubtractKind;
  } else {
    llvm_unreachable("Invalid update operator");
  }

  LReference lref = createLRef(updateExpr->_argument, false);

  // Load the original value.
  Value *original = lref.emitLoad();

  // Convert the original value to number. Even on suffix operators we return
  // the converted value.
  original = Builder.createAsNumberInst(original);

  // Create the +1 or -1.
  Value *result = Builder.createBinaryOperatorInst(
      original, Builder.getLiteralNumber(1), opKind);

  // Store the result.
  lref.emitStore(result);

  // Depending on the prefixness return the previous value or the modified
  // value.
  return (isPrefix ? result : original);
}

Value *ESTreeIRGen::genAssignmentExpr(ESTree::AssignmentExpressionNode *AE) {
  LLVM_DEBUG(dbgs() << "IRGen assignment operator.\n");

  auto opStr = AE->_operator->str();
  auto AssignmentKind = BinaryOperatorInst::parseAssignmentOperator(opStr);

  LReference lref = createLRef(AE->_left, false);

  Identifier nameHint{};
  if (auto *var = lref.castAsVariable()) {
    nameHint = var->getName();
  } else if (auto *globProp = lref.castAsGlobalObjectProperty()) {
    nameHint = globProp->getName()->getValue();
  }

  Value *result;
  if (AssignmentKind == BinaryOperatorInst::OpKind::AssignShortCircuitOrKind ||
      AssignmentKind == BinaryOperatorInst::OpKind::AssignShortCircuitAndKind ||
      AssignmentKind == BinaryOperatorInst::OpKind::AssignNullishCoalesceKind) {
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

    switch (AssignmentKind) {
      case BinaryOperatorInst::OpKind::AssignShortCircuitOrKind:
        Builder.createCondBranchInst(lhs, continueBB, assignBB);
        break;
      case BinaryOperatorInst::OpKind::AssignShortCircuitAndKind:
        Builder.createCondBranchInst(lhs, assignBB, continueBB);
        break;
      case BinaryOperatorInst::OpKind::AssignNullishCoalesceKind:
        Builder.createCondBranchInst(
            Builder.createBinaryOperatorInst(
                lhs,
                Builder.getLiteralNull(),
                BinaryOperatorInst::OpKind::EqualKind),
            assignBB,
            continueBB);
        break;
      default:
        llvm_unreachable("invalid AssignmentKind in this branch");
    }

    Builder.setInsertionBlock(assignBB);
    auto *rhs = genExpression(AE->_right, nameHint);
    auto *cookie = instrumentIR_.preAssignment(AE, lhs, rhs);
    result = instrumentIR_.postAssignment(AE, cookie, rhs, lhs, rhs);
    lref.emitStore(result);
    values.push_back(result);
    blocks.push_back(Builder.getInsertionBlock());
    Builder.createBranchInst(continueBB);

    Builder.setInsertionBlock(continueBB);
    // Final result is either the original value or the value assigned,
    // depending on which branch was taken.
    return Builder.createPhiInst(std::move(values), std::move(blocks));
  }

  if (AssignmentKind != BinaryOperatorInst::OpKind::IdentityKind) {
    // Section 11.13.1 specifies that we should first load the
    // LHS before materializing the RHS. Unlike in C, this
    // code is well defined: "x+= x++".
    // https://es5.github.io/#x11.13.1
    auto V = lref.emitLoad();
    auto *RHS = genExpression(AE->_right, nameHint);
    auto *cookie = instrumentIR_.preAssignment(AE, V, RHS);
    result = Builder.createBinaryOperatorInst(V, RHS, AssignmentKind);
    result = instrumentIR_.postAssignment(AE, cookie, result, V, RHS);
  } else {
    auto *RHS = genExpression(AE->_right, nameHint);
    auto *cookie = instrumentIR_.preAssignment(AE, nullptr, RHS);
    result = instrumentIR_.postAssignment(AE, cookie, RHS, nullptr, RHS);
  }

  lref.emitStore(result);

  // Return the value that we stored as the result of the expression.
  return result;
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
  LLVM_DEBUG(dbgs() << "Looking for identifier \"" << Iden->_name << "\"\n");

  // 'arguments' is an array-like object holding all function arguments.
  // If one of the parameters is called "arguments" then it shadows the
  // arguments keyword.
  if (Iden->_name->str() == "arguments" &&
      !nameTable_.count(getNameFieldFromID(Iden))) {
    // If it is captured, we must use the captured value.
    if (curFunction()->capturedArguments) {
      return Builder.createLoadFrameInst(curFunction()->capturedArguments);
    }

    return curFunction()->createArgumentsInst;
  }

  // Lookup variable name.
  auto StrName = getNameFieldFromID(Iden);

  auto *Var = ensureVariableExists(Iden);

  // For uses of undefined as the global property, we make an optimization
  // to always return undefined constant.
  if (llvh::isa<GlobalObjectProperty>(Var) && StrName.str() == "undefined") {
    return Builder.getLiteralUndefined();
  }

  LLVM_DEBUG(
      dbgs() << "Found variable " << StrName << " in function \""
             << (llvh::isa<GlobalObjectProperty>(Var)
                     ? StringRef("global")
                     : cast<Variable>(Var)
                           ->getParent()
                           ->getFunction()
                           ->getInternalNameStr())
             << "\"\n");

  // Typeof <variable> does not throw.
  return emitLoad(Builder, Var, afterTypeOf);
}

Value *ESTreeIRGen::genMetaProperty(ESTree::MetaPropertyNode *MP) {
  // Recognize "new.target"
  if (cast<ESTree::IdentifierNode>(MP->_meta)->_name->str() == "new") {
    if (cast<ESTree::IdentifierNode>(MP->_property)->_name->str() == "target") {
      Value *value;

      if (curFunction()->function->getDefinitionKind() ==
              Function::DefinitionKind::ES6Arrow ||
          curFunction()->function->getDefinitionKind() ==
              Function::DefinitionKind::ES6Method) {
        value = curFunction()->capturedNewTarget;
      } else {
        value = Builder.createGetNewTargetInst();
      }

      // If it is a variable, we must issue a load.
      if (auto *V = llvh::dyn_cast<Variable>(value))
        return Builder.createLoadFrameInst(V);

      return value;
    }
  }

  llvm_unreachable("invalid MetaProperty");
}

Value *ESTreeIRGen::genNewExpr(ESTree::NewExpressionNode *N) {
  LLVM_DEBUG(dbgs() << "IRGen 'new' statement/expression.\n");

  Value *callee = genExpression(N->_callee);

  bool hasSpread = false;
  for (auto &arg : N->_arguments) {
    if (llvh::isa<ESTree::SpreadElementNode>(&arg)) {
      hasSpread = true;
    }
  }

  if (!hasSpread) {
    ConstructInst::ArgumentList args;
    for (auto &arg : N->_arguments) {
      args.push_back(genExpression(&arg));
    }

    return Builder.createConstructInst(callee, args);
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
  LLVM_DEBUG(dbgs() << "IRGen of short circuiting: " << opStr << ".\n");

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
  auto tempVar = Builder.createAllocStackInst(tempVarName);

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
              LHS,
              Builder.getLiteralNull(),
              BinaryOperatorInst::OpKind::EqualKind),
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

void ESTreeIRGen::genLogicalExpressionBranch(
    ESTree::LogicalExpressionNode *logical,
    BasicBlock *onTrue,
    BasicBlock *onFalse,
    BasicBlock *onNullish) {
  auto opStr = logical->_operator->str();
  LLVM_DEBUG(dbgs() << "IRGen of short circuiting: " << opStr << " branch.\n");

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
  LLVM_DEBUG(dbgs() << "IRGen 'TemplateLiteral' expression.\n");

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
  CallInst::ArgumentList argList;
  auto exprItr = Expr->_expressions.begin();
  while (strItr != Expr->_quasis.end()) {
    auto *sub = genExpression(&*exprItr);
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

  if (argList.size() == 1 && firstCookedStr->getValue().str().empty()) {
    // If the template literal only has one substitution and the opening
    // string is empty, it looks something like `${expr}` and we can just
    // convert the argument to a string.
    return Builder.createAddEmptyStringInst(argList[0]);
  }

  // Generate a function call to HermesInternal.concat() with these arguments.
  return genHermesInternalCall("concat", firstCookedStr, argList);
}

Value *ESTreeIRGen::genTaggedTemplateExpr(
    ESTree::TaggedTemplateExpressionNode *Expr) {
  LLVM_DEBUG(dbgs() << "IRGen 'TaggedTemplateExpression' expression.\n");
  // Step 1: get the template object.
  auto *templateLit = cast<ESTree::TemplateLiteralNode>(Expr->_quasi);

  // Construct an argument list for calling HermesInternal.getTemplateObject():
  // [template object ID, dup, raw strings, (optional) cooked strings].
  CallInst::ArgumentList argList;
  // Retrieve template object ID.
  Module::RawStringList rawStrings;
  for (auto &n : templateLit->_quasis) {
    auto element = cast<ESTree::TemplateElementNode>(&n);
    rawStrings.push_back(Builder.getLiteralString(element->_raw->str()));
  }
  uint32_t templateObjID = Mod->getTemplateObjectID(std::move(rawStrings));
  argList.push_back(Builder.getLiteralNumber(templateObjID));

  // dup is true if the cooked strings and raw strings are duplicated.
  bool dup = true;
  // Add the argument dup first as a placeholder which we overwrite with
  // the correct value later.
  argList.push_back(Builder.getLiteralBool(dup));
  for (auto &node : templateLit->_quasis) {
    auto *templateElt = cast<ESTree::TemplateElementNode>(&node);
    if (templateElt->_cooked != templateElt->_raw) {
      dup = false;
    }
    argList.push_back(Builder.getLiteralString(templateElt->_raw->str()));
  }
  argList[1] = Builder.getLiteralBool(dup);
  // If the cooked strings are not the same as raw strings, append them to
  // argument list.
  if (!dup) {
    for (auto &node : templateLit->_quasis) {
      auto *templateElt = cast<ESTree::TemplateElementNode>(&node);
      if (templateElt->_cooked) {
        argList.push_back(
            Builder.getLiteralString(templateElt->_cooked->str()));
      } else {
        argList.push_back(Builder.getLiteralUndefined());
      }
    }
  }

  // Generate a function call to HermesInternal.getTemplateObject() with these
  // arguments.
  auto *templateObj =
      genBuiltinCall(BuiltinMethod::HermesBuiltin_getTemplateObject, argList);

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

  return Builder.createCallInst(callee, thisVal, tagFuncArgList);
}

} // namespace irgen
} // namespace hermes
