/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "instsimplify"

#include "hermes/Optimizer/Scalar/InstSimplify.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IREval.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseSet.h"

using namespace hermes;

STATISTIC(NumSimp, "Number of instructions simplified");

namespace {

constexpr Type kNullOrUndef =
    Type::unionTy(Type::createUndefined(), Type::createNull());

bool canBeNaN(Value *value) {
  if (!value->getType().canBeNumber()) {
    return false;
  }

  if (auto *literalNumber = llvh::dyn_cast<LiteralNumber>(value)) {
    return std::isnan(literalNumber->getValue());
  }

  return true;
}

template <typename T>
Value *reduceAsNumberLike(T *asNumber) {
  IRBuilder builder(asNumber->getParent()->getParent());
  auto *op = asNumber->getSingleOperand();

  // If operand is a literal, try to evaluate ToNumber(operand).
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto *result = evalToNumber(builder, lit)) {
      return result;
    }
  }

  // No need to convert if the value is a number already.
  if (op->getType().isNumberType()) {
    return op;
  }

  // If it can't be converted, return the original instruction.
  return asNumber;
}

Value *reduceAsNumber(AsNumberInst *asNumber) {
  return reduceAsNumberLike(asNumber);
}

Value *reduceAsNumeric(AsNumericInst *asNumeric) {
  auto *op = asNumeric->getSingleOperand();
  if (op->getType().isBigIntType()) {
    return op;
  }

  return reduceAsNumberLike(asNumeric);
}

Value *reduceAsInt32(AsInt32Inst *asInt32) {
  IRBuilder builder(asInt32->getParent()->getParent());
  auto *op = asInt32->getSingleOperand();

  // If operand is a literal, try to evaluate ToNumber(operand).
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto *result = evalToInt32(builder, lit)) {
      return result;
    }
  }

#ifdef HERMES_RUN_WASM
  if (op->getType().isInt32Type()) {
    return op;
  }
#endif

  // Nothing can be done to simplify, return it as-is.
  return asInt32;
}

Value *simplifyUnOp(UnaryOperatorInst *unary) {
  IRBuilder builder(unary->getParent()->getParent());

  auto kind = unary->getOperatorKind();
  auto *op = unary->getSingleOperand();
  Type t = op->getType();

  // If the operand is a literal, try to evaluate the expression.
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto result = evalUnaryOperator(kind, builder, lit)) {
      return result;
    }
  }

  using OpKind = UnaryOperatorInst::OpKind;

  // Try to simplify based on type information.
  switch (kind) {
    case OpKind::TypeofKind:
      if (t.isNullType()) {
        return builder.getLiteralString("object");
      }
      if (t.isNumberType()) {
        return builder.getLiteralString("number");
      }
      if (t.isUndefinedType()) {
        return builder.getLiteralString("undefined");
      }
      if (t.isBooleanType()) {
        return builder.getLiteralString("boolean");
      }
      if (t.isStringType()) {
        return builder.getLiteralString("string");
      }
      if (t.isRegExpType()) {
        return builder.getLiteralString("object");
      }
      if (t.isClosureType()) {
        return builder.getLiteralString("function");
      }
      break;

    case OpKind::BangKind:
      if (op->getType().isSubsetOf(kNullOrUndef)) {
        return builder.getLiteralBool(true);
      }
      break;

    case OpKind::PlusKind:
      // Convert +x to AsNumber(x).
      builder.setInsertionPoint(unary);
      return reduceAsNumber(builder.createAsNumberInst(op));

    default:
      break;
  }

  return nullptr;
}

Value *simplifyBinOp(BinaryOperatorInst *binary) {
  IRBuilder builder(binary->getParent()->getParent());

  Value *lhs = binary->getLeftHandSide();
  Value *rhs = binary->getRightHandSide();
  auto kind = binary->getOperatorKind();

  // Try simplifying without replacing the instruction.
  auto *litLhs = llvh::dyn_cast<Literal>(lhs);
  auto *litRhs = llvh::dyn_cast<Literal>(rhs);
  if (litLhs && litRhs) {
    if (auto result = evalBinaryOperator(kind, builder, litLhs, litRhs)) {
      return result;
    }
  }

  // In some cases, the instruction can be replaced with a faster version
  // or type information can be used to simplify some non-constant expressions.
  Type leftTy = lhs->getType();
  Type rightTy = rhs->getType();
  bool safeTypes = isSideEffectFree(leftTy) && isSideEffectFree(rightTy);

  // Verify that the operands are identical and can be replaced. NaN is never
  // identical to itself, and comparisons with side effects can't be replaced.
  bool identicalOperands = safeTypes && lhs == rhs && !canBeNaN(lhs);

  using OpKind = BinaryOperatorInst::OpKind;

  switch (kind) {
    case OpKind::EqualKind: // ==
      // Identical operands must be equal.
      if (identicalOperands) {
        return builder.getLiteralBool(true);
      }

      // Promote equality to strict equality if we know that the types are
      // identical primitive types.
      if (leftTy.isKnownPrimitiveType() && rightTy == leftTy) {
        builder.setInsertionPoint(binary);
        return builder.createBinaryOperatorInst(
            lhs, rhs, OpKind::StrictlyEqualKind);
      }
      break;

    case OpKind::NotEqualKind: // !=
      // Identical operands can't be non-equal.
      if (identicalOperands) {
        return builder.getLiteralBool(false);
      }

      // Promote inequality to strict inequality if we know that the types are
      // identical primitive types.
      if (leftTy.isKnownPrimitiveType() && rightTy == leftTy) {
        builder.setInsertionPoint(binary);
        return builder.createBinaryOperatorInst(
            lhs, rhs, OpKind::StrictlyNotEqualKind);
      }
      break;

    case OpKind::StrictlyEqualKind: // ===
      // Identical operand must be strictly equal.
      if (identicalOperands) {
        return builder.getLiteralBool(true);
      }

      // Operands of different types can't be strictly equal.
      if (leftTy.isPrimitive() && rightTy.isPrimitive()) {
        if (Type::intersectTy(leftTy, rightTy).isNoType()) {
          return builder.getLiteralBool(false);
        }
      }
      break;

    case OpKind::StrictlyNotEqualKind: // !===
      // Identical operands can't be non-equal.
      if (identicalOperands) {
        return builder.getLiteralBool(false);
      }
      break;

    case OpKind::LessThanKind: // <
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(false);
      }
      break;

    case OpKind::LessThanOrEqualKind: // <=
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(true);
      }
      break;

    case OpKind::GreaterThanKind: // >
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(false);
      }
      break;

    case OpKind::GreaterThanOrEqualKind: // >=
      // Handle comparison to self:
      if (identicalOperands && !leftTy.isUndefinedType()) {
        return builder.getLiteralBool(true);
      }
      break;

    case OpKind::AddKind:
      // Convert ("" + x) or (x + "") as AsString(x).
      if (llvh::isa<LiteralString>(lhs) &&
          cast<LiteralString>(lhs)->getValue().str() == "") {
        builder.setInsertionPoint(binary);
        return builder.createAddEmptyStringInst(rhs);
      } else if (
          llvh::isa<LiteralString>(rhs) &&
          cast<LiteralString>(rhs)->getValue().str() == "") {
        builder.setInsertionPoint(binary);
        return builder.createAddEmptyStringInst(lhs);
      }
      break;

    case OpKind::OrKind: { // |
      // Convert (x | 0) of (0 | x) to AsInt32.
      Value *nonzeroOp = nullptr;
      if (llvh::isa<LiteralNumber>(lhs) &&
          cast<LiteralNumber>(lhs)->getValue() == 0) {
        nonzeroOp = rhs;
      } else if (
          llvh::isa<LiteralNumber>(rhs) &&
          cast<LiteralNumber>(rhs)->getValue() == 0) {
        nonzeroOp = lhs;
      }
      if (nonzeroOp) {
        builder.setInsertionPoint(binary);
        return reduceAsInt32(builder.createAsInt32Inst(nonzeroOp));
      }
      break;
    }

    default:
      break;
  }

  return nullptr;
}

Value *simplifyPhiInst(PhiInst *P) {
  unsigned numEntries = P->getNumEntries();
  if (!numEntries) {
    // This Phi has no incoming entries. This means that the entire block is
    // unreachable and will be removed by DCE.
    return nullptr;
  }

  // Optimize away PHI nodes with a single entry.
  if (1 == numEntries) {
    auto E = P->getEntry(0);
    P->replaceAllUsesWith(E.first);
    P->eraseFromParent();
    return nullptr;
  }

  Value *incoming = nullptr;
  // Optimize PHI nodes that accept the same input from all directions:
  for (int i = 0, e = P->getNumEntries(); i < e; i++) {
    auto E = P->getEntry(i);
    // Ignore self edges.
    if (E.first == P)
      continue;

    // Record the first valid input.
    if (!incoming) {
      incoming = E.first;
      continue;
    }

    // Found another unique value. Bail out.
    if (incoming != E.first)
      return nullptr;
  }

  // The PHI has a single incoming value. Replace all uses of the PHI with
  // the incoming value.
  if (incoming) {
    P->replaceAllUsesWith(incoming);
    P->eraseFromParent();
  }

  return nullptr;
}

Value *simplifyCondBranchInst(CondBranchInst *CBI) {
  auto *cond = CBI->getCondition();

  // Replace branches where the condition is negation of some expr with the
  // expr itself. Example:
  //
  // %1 = NOT %0
  // COND_BR %1, %BB1, %BB2
  //
  // into:
  //
  // %1 = NOT %0
  // COND_BR %0, %BB2, %BB1
  //
  if (auto *U = llvh::dyn_cast<UnaryOperatorInst>(cond)) {
    if (U->getSideEffect() == SideEffectKind::None &&
        U->getOperatorKind() == UnaryOperatorInst::OpKind::BangKind) {
      // Strip the negation.
      CBI->setOperand(U->getSingleOperand(), 0);
      // Swap the destination blocks:
      Value *BB1 = CBI->getOperand(1);
      Value *BB2 = CBI->getOperand(2);
      CBI->setOperand(BB1, 2);
      CBI->setOperand(BB2, 1);
      return CBI;
    }
  }
  return nullptr;
}

/// \return the one possible return value for the function \p F
/// for callsite \p callSite, or nullptr if none found.
Value *getKnownReturnValue(Function *F, CallInst *callSite) {
  IRBuilder builder(F);
  llvh::DenseSet<Value *> returnValues;

  for (auto &BB : F->getBasicBlockList()) {
    auto *term = BB.getTerminator();
    auto *ret = llvh::dyn_cast<ReturnInst>(term);
    if (!ret)
      continue;

    returnValues.insert(ret->getValue());
  }

  // Found too many return values, or not enough. Notice that
  // in some cases the function may not return anything and just
  // throw and stay in an infinite loop, and this code is legal.
  if (returnValues.size() != 1)
    return nullptr;

  Value *v = *returnValues.begin();

  // The function always returns a single literal.
  if (llvh::isa<Literal>(v))
    return v;

  // The function returns a parameter, let's inspect the call
  // site and find the value.
  if (auto *P = llvh::dyn_cast<Parameter>(v)) {
    // Don't handle the 'this' parameter.
    if (P->isThisParameter())
      return nullptr;

    unsigned idx = P->getIndexInParamList();

    // Returning unpassed parameter results in undef.
    if (idx >= callSite->getNumArguments() - 1)
      return builder.getLiteralUndefined();

    // return the argument that the user passed in (ignore the
    // 'this' argument).
    return callSite->getArgument(idx + 1);
  }

  // Returning some locally computed value.
  return nullptr;
}

Value *simplifyCallInst(CallInst *CI) {
  if (!CI->hasUsers() || llvh::isa<ConstructInst>(CI))
    return nullptr;

  if (Function *F = getCallee(CI->getCallee())) {
    if (Value *V = getKnownReturnValue(F, CI)) {
      CI->replaceAllUsesWith(V);
      return CI;
    }
  }

  return nullptr;
}

Value *simplifySwitchInst(SwitchInst *SI) {
  auto *thisBlock = SI->getParent();
  IRBuilder builder(thisBlock->getParent());
  builder.setInsertionBlock(thisBlock);

  Value *input = SI->getInputValue();
  auto *litInput = llvh::dyn_cast<Literal>(input);

  // If input of switch is not literal, nothing can be done.
  if (!litInput) {
    return nullptr;
  }

  auto *destination = SI->getDefaultDestination();

  for (unsigned i = 0, e = SI->getNumCasePair(); i < e; i++) {
    auto switchCase = SI->getCasePair(i);

    // Look for a case which matches input.
    if (switchCase.first == litInput) {
      destination = switchCase.second;
      break;
    }
  }

  // Rewrite all phi nodes that no longer have incoming arrows from this block.
  for (unsigned i = 0, e = SI->getNumSuccessors(); i < e; i++) {
    auto *successor = SI->getSuccessor(i);
    if (successor == destination)
      continue;
    deleteIncomingBlockFromPhis(successor, thisBlock);
  }

  return builder.createBranchInst(destination);
}

Value *simplifyAsNumber(AsNumberInst *asNumber) {
  Value *reduced = reduceAsNumber(asNumber);
  return reduced == asNumber ? nullptr : reduced;
}

bool isUnaryIncOrDec(Value *value) {
  if (auto *unOp = llvh::dyn_cast<UnaryOperatorInst>(value)) {
    switch (unOp->getOperatorKind()) {
      default:
        break;
      case UnaryOperatorInst::OpKind::IncKind:
      case UnaryOperatorInst::OpKind::DecKind:
        return true;
    }
  }

  return false;
}

Value *simplifyAsNumeric(AsNumericInst *asNumeric) {
  // reduced == asNumeric means that it wasn't possible to optimize asNumeric.
  Value *reduced = asNumeric;

  if (asNumeric->hasOneUser()) {
    if (isUnaryIncOrDec(*asNumeric->users_begin())) {
      // Single-use ToNumeric that feeds Inc/Dec can be optimized out as Inc/Dec
      // will perform ToNumeric internally.
      reduced = asNumeric->getSingleOperand();
    }
  }

  if (reduced == asNumeric) {
    // asNumeric can be optimized out if its (single) argument is provably
    // numeric.
    reduced = reduceAsNumeric(asNumeric);
  }

  return reduced == asNumeric ? nullptr : reduced;
}

Value *simplifyAsInt32(AsInt32Inst *asInt32) {
  Value *reduced = reduceAsInt32(asInt32);
  return (reduced == asInt32) ? nullptr : reduced;
}

Value *simplifyAddEmptyString(AddEmptyStringInst *AES) {
  IRBuilder builder(AES->getParent()->getParent());
  auto *op = AES->getSingleOperand();

  // If operand is a literal, try to evaluate ToString(operand).
  if (auto *lit = llvh::dyn_cast<Literal>(op)) {
    if (auto *result = evalToString(builder, lit)) {
      return result;
    }
  }

  // No need to convert if the value is a string already.
  if (op->getType().isStringType()) {
    return op;
  }

  return nullptr;
}

Value *simplifyCoerceThisNS(CoerceThisNSInst *coerce) {
  auto *operand = coerce->getSingleOperand();

  // null or undefined produce global scope.
  if (operand->getType().isSubsetOf(kNullOrUndef)) {
    IRBuilder builder(coerce->getParent()->getParent());
    builder.setInsertionPoint(coerce);
    return builder.getGlobalObject();
  }

  // Objects are unchanged.
  if (operand->getType().isObjectType())
    return operand;

  return nullptr;
}

/// Try to simplify ThrowIfEmptyInst
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyThrowIfEmpty(ThrowIfEmptyInst *TIE) {
  // If the operand does not contain the "poison" type, it can be safely
  // eliminated.
  if (!TIE->getCheckedValue()->getType().canBeEmpty())
    return TIE->getCheckedValue();
  return nullptr;
}

/// Try to simplify the instruction \p I.
/// \returns one of:
///   - nullptr if the instruction cannot be simplified.
///   - the instruction itself, if it was changed inplace.
///   - a new instruction to replace the original one
///   - llvh::None if the instruction should be deleted.
OptValue<Value *> simplifyInstruction(Instruction *I) {
  // Dispatch the different simplification kinds:
  switch (I->getKind()) {
    case ValueKind::BinaryOperatorInstKind:
      return simplifyBinOp(cast<BinaryOperatorInst>(I));
    case ValueKind::UnaryOperatorInstKind:
      return simplifyUnOp(cast<UnaryOperatorInst>(I));
    case ValueKind::AsNumberInstKind:
      return simplifyAsNumber(cast<AsNumberInst>(I));
    case ValueKind::AsNumericInstKind:
      return simplifyAsNumeric(cast<AsNumericInst>(I));
    case ValueKind::AsInt32InstKind:
      return simplifyAsInt32(cast<AsInt32Inst>(I));
    case ValueKind::AddEmptyStringInstKind:
      return simplifyAddEmptyString(cast<AddEmptyStringInst>(I));
    case ValueKind::PhiInstKind:
      return simplifyPhiInst(cast<PhiInst>(I));
    case ValueKind::CondBranchInstKind:
      return simplifyCondBranchInst(cast<CondBranchInst>(I));
    case ValueKind::SwitchInstKind:
      return simplifySwitchInst(cast<SwitchInst>(I));
    case ValueKind::CallInstKind:
      return simplifyCallInst(cast<CallInst>(I));
    case ValueKind::CoerceThisNSInstKind:
      return simplifyCoerceThisNS(cast<CoerceThisNSInst>(I));
    case ValueKind::ThrowIfEmptyInstKind:
      return simplifyThrowIfEmpty(cast<ThrowIfEmptyInst>(I));

    default:
      // TODO: handle other kinds of instructions.
      return nullptr;
  }
}

} // namespace

bool InstSimplify::runOnFunction(Function *F) {
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer;

  // For all blocks in the function:
  for (auto &blockIter : *F) {
    BasicBlock *BB = &blockIter;

    // For all instructions:
    for (auto instIter = BB->begin(), e = BB->end(); instIter != e;) {
      Instruction *II = &*instIter;
      ++instIter;

      auto optNewVal = simplifyInstruction(II);
      if (optNewVal.hasValue()) {
        auto newVal = optNewVal.getValue();
        if (!newVal)
          continue;

        changed = true;
        NumSimp++;

        // Instruction changed inplace.
        if (II == newVal)
          continue;

        // We have a better and simpler instruction. Replace the original
        // instruction and mark it for deletion.
        II->replaceAllUsesWith(newVal);
      } else {
        changed = true;
        NumSimp++;
      }
      destroyer.add(II);
    }
  }

  return changed;
}

Pass *hermes::createInstSimplify() {
  return new InstSimplify();
}

#undef DEBUG_TYPE
