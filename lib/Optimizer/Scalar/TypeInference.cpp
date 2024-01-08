/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// This optimization performs type inference for instructions that do not have
/// isTyped() == true.
/// It infers types for variables and function return types as well,
/// and propagates them through the IR.
///
/// Steps:
/// 1. Partition the functions into groups that use each others variables or
/// call each other. This allows us to process all the instructions that could
/// possibly influence each others' types together. If we were to visit
/// functions one at a time, we might visit the use of a variable prior to its
/// assignment in another function, and be unable to infer its type correctly.
/// 2. Clear the type information from all instructions in the group. This
/// allows us to expand the types to the smallest valid type for each value
/// instead of starting with types that are too loose prior to the pass and not
/// be able to narrow them properly.
/// 3. Infer the type of every instruction, function, and variable in the group.
/// Iterate until no more changes are made.
///
/// The pass never widens types of any value, because it intersects the result
/// types with the types prior to the pass before setting them.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "typeinference"

#include "hermes/Optimizer/Scalar/TypeInference.h"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/EquivalenceClasses.h"
#include "llvh/ADT/MapVector.h"
#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/Support/Debug.h"

using namespace hermes;
using llvh::dbgs;
using llvh::SmallPtrSetImpl;

STATISTIC(NumTI, "Number of instructions type inferred");

namespace {

/// \return if the given \p type is a BigInt|Object, which used to determine if
/// unary/binary operations may have a BigInt result.
static bool isBigIntOrObject(Type type) {
  return type.canBeBigInt() || type.canBeObject();
}

static Type inferUnaryArith(UnaryOperatorInst *UOI, Type numberResultType) {
  Value *op = UOI->getSingleOperand();

  if (op->getType().isNumberType()) {
    return numberResultType;
  }

  if (op->getType().isBigIntType()) {
    return Type::createBigInt();
  }

  Type mayBeBigInt = isBigIntOrObject(op->getType()) ? Type::createBigInt()
                                                     : Type::createNoType();

  // - ?? => Number|?BigInt. BigInt is only possible if op.Type is
  // BigInt|Object.
  return Type::unionTy(numberResultType, mayBeBigInt);
}

static Type inferUnaryArithDefault(UnaryOperatorInst *UOI) {
  // - Number => Number
  // - BigInt => BigInt
  // - ?? => Number|BigInt
  return inferUnaryArith(UOI, Type::createNumber());
}

static Type inferTilde(UnaryOperatorInst *UOI) {
  // ~ Number => Int32
  // ~ BigInt => BigInt
  // ~ ?? => Int32|BigInt
  return inferUnaryArith(UOI, Type::createInt32());
}

/// Try to infer the type of the value that's stored into \p addr. \p addr is
/// either a stack location or a variable.
static Type inferMemoryLocationType(Value *addr) {
  Type T = Type::createNoType();

  for (auto *U : addr->getUsers()) {
    Value *storedVal = nullptr;

    switch (U->getKind()) {
      case ValueKind::StoreFrameInstKind: {
        auto *SF = cast<StoreFrameInst>(U);
        storedVal = SF->getValue();
        break;
      }

      case ValueKind::StoreStackInstKind: {
        auto *SS = cast<StoreStackInst>(U);
        storedVal = SS->getValue();
        break;
      }

      case ValueKind::HBCStoreToEnvironmentInstKind: {
        auto *SE = cast<HBCStoreToEnvironmentInst>(U);
        storedVal = SE->getStoredValue();
        break;
      }

      // Loads do not change the type of the memory location.
      case ValueKind::LoadFrameInstKind:
      case ValueKind::LoadStackInstKind:
      case ValueKind::HBCLoadFromEnvironmentInstKind:
        continue;

      default:
        // Other instructions that may write to alloc stack thwart our analysis.
        return Type::createAnyType();
    }

    if (!storedVal)
      continue;

    Type storedType = storedVal->getType();

    T = Type::unionTy(T, storedType);
  }

  return T;
}

/// Collects all of the values that are used by a tree of PHIs, recursively.
/// Inputs are stored into \p inputs. Visited PHIs are stored into \p visited.
static void collectPHIInputs(
    SmallPtrSetImpl<Value *> &visited,
    SmallPtrSetImpl<Value *> &inputs,
    PhiInst *P) {
  // Return if we already visited this node.
  if (!visited.insert(P).second)
    return;

  // For all operands:
  for (unsigned i = 0, e = P->getNumEntries(); i < e; i++) {
    auto E = P->getEntry(i);

    // Recursively inspect PHI node operands, and insert non-phis into the input
    // list.
    if (auto *PN = llvh::dyn_cast<PhiInst>(E.first)) {
      collectPHIInputs(visited, inputs, PN);
    } else {
      inputs.insert(E.first);
    }
  }
}

static Type inferBinaryArith(
    BinaryOperatorInst *BOI,
    Type numberType = Type::createNumber()) {
  Type LeftTy = BOI->getLeftHandSide()->getType();
  Type RightTy = BOI->getRightHandSide()->getType();

  // Number - Number => Number
  if (LeftTy.isNumberType() && RightTy.isNumberType()) {
    return numberType;
  }

  // BigInt - BigInt => BigInt
  if (LeftTy.isBigIntType() && RightTy.isBigIntType()) {
    return Type::createBigInt();
  }

  Type mayBeBigInt = (isBigIntOrObject(LeftTy) && isBigIntOrObject(RightTy))
      ? Type::createBigInt()
      : Type::createNoType();

  // ?? - ?? => Number|?BigInt. BigInt is only possible if both operands are
  // BigInt|Object due to the no automatic BigInt conversion.
  return Type::unionTy(numberType, mayBeBigInt);
}

static Type inferBinaryBitwise(BinaryOperatorInst *BOI) {
  Type LeftTy = BOI->getLeftHandSide()->getType();
  Type RightTy = BOI->getRightHandSide()->getType();

  Type mayBeBigInt = LeftTy.canBeBigInt() && RightTy.canBeBigInt()
      ? Type::createBigInt()
      : Type::createNoType();

  // ?? - ?? => Int32|?BigInt. BigInt is only possible if both operands can be
  // BigInt due to the no automatic BigInt conversion.
  return Type::unionTy(Type::createInt32(), mayBeBigInt);
}

static Type inferBinaryInst(BinaryOperatorInst *BOI) {
  switch (BOI->getKind()) {
    // The following operations always return a boolean result.
    // They may throw, they may read/write memory, but the result of the
    // operation must be a boolean.
    case ValueKind::BinaryEqualInstKind:
    case ValueKind::BinaryNotEqualInstKind:
    case ValueKind::BinaryStrictlyEqualInstKind:
    case ValueKind::BinaryStrictlyNotEqualInstKind:
    case ValueKind::BinaryLessThanInstKind:
    case ValueKind::BinaryLessThanOrEqualInstKind:
    case ValueKind::BinaryGreaterThanInstKind:
    case ValueKind::BinaryGreaterThanOrEqualInstKind:
    case ValueKind::BinaryInInstKind:
    case ValueKind::BinaryInstanceOfInstKind:
      // Notice that the spec says that comparison of NaN should return
      // "Undefined" but all VMs return 'false'. We decided to conform to the
      // current implementation and not to the spec.
      return Type::createBoolean();

    // These arithmetic operations always return a number or bigint:
    // https://262.ecma-international.org/#sec-multiplicative-operators
    case ValueKind::BinaryDivideInstKind:
    case ValueKind::BinaryMultiplyInstKind:
    // https://262.ecma-international.org/#sec-exp-operator
    case ValueKind::BinaryExponentiationInstKind:
    // https://tc39.es/ecma262/#sec-subtraction-operator-minus
    case ValueKind::BinarySubtractInstKind:
    // https://tc39.es/ecma262/#sec-left-shift-operator
    case ValueKind::BinaryLeftShiftInstKind:
    // https://tc39.es/ecma262/#sec-signed-right-shift-operator
    case ValueKind::BinaryRightShiftInstKind:
      return inferBinaryArith(BOI);

    case ValueKind::BinaryModuloInstKind:
      return inferBinaryArith(BOI, Type::createInt32());

    // https://es5.github.io/#x11.7.3
    case ValueKind::BinaryUnsignedRightShiftInstKind:
      return Type::createUint32();

    // The Add operator is special:
    // https://262.ecma-international.org/#sec-addition-operator-plus
    case ValueKind::BinaryAddInstKind: {
      Type LeftTy = BOI->getLeftHandSide()->getType();
      Type RightTy = BOI->getRightHandSide()->getType();
      // String + String -> String. It is enough for one of the operands to be
      // a string to force the result to be a string.
      if (LeftTy.isStringType() || RightTy.isStringType()) {
        return Type::createString();
      }

      // Number + Number -> Number.
      if (LeftTy.isNumberType() && RightTy.isNumberType()) {
        return Type::createNumber();
      }

      // BigInt + BigInt -> BigInt.
      if (LeftTy.isBigIntType() && RightTy.isBigIntType()) {
        return Type::createBigInt();
      }

      // ?BigInt + ?BigInt => ?BigInt. Both operands need to "may be a BigInt"
      // for a possible BigInt result from this operator. This is true because
      // there's no automative BigInt type conversion.
      Type mayBeBigInt = (isBigIntOrObject(LeftTy) && isBigIntOrObject(RightTy))
          ? Type::createBigInt()
          : Type::createNoType();

      // handy alias for number|maybe(BigInt).
      Type numeric = Type::unionTy(Type::createNumber(), mayBeBigInt);

      // If both sides of the binary operand are known and both sides are known
      // to be non-string (and can't be converted to strings) then the result
      // must be of a numeric type.
      if (LeftTy.isPrimitive() && RightTy.isPrimitive() &&
          !LeftTy.canBeString() && !RightTy.canBeString()) {
        return numeric;
      }

      // The plus operator always returns a number, bigint, or a string.
      return Type::unionTy(numeric, Type::createString());
    }

    // https://tc39.es/ecma262/#sec-binary-bitwise-operators
    case ValueKind::BinaryAndInstKind:
    case ValueKind::BinaryOrInstKind:
    case ValueKind::BinaryXorInstKind:
      return inferBinaryBitwise(BOI);

    default:
      LLVM_DEBUG(
          dbgs() << "Unknown binary operator in TypeInference: "
                 << BOI->getOperatorStr() << '\n');
      return Type::createAnyType();
  }
}

/// Actual implementation of type inference pass.
/// Contains the ability to infer types per-instruction.
///
/// Prior to inferring the type of the instructions, the result type of
/// each instruction is cleared out (set to "NoType"), and the inference
/// is run from first principles on each of the instructions.
///
/// Each of the "inferXXXInst" functions returns a Type.
/// The only instructions which are allowed to return NoType from their infer
/// implementations are those instructions which have no output.
/// Each of these infer functions for Instructions do NOT themselves have to
/// check if the newly inferred type is _different_ - that will be done
/// by the dispatch function. However, other infer functions that are called
/// directly by \c runOnFunction must return false if they aren't changing the
/// type.
///
/// Importantly, the Phi instruction is handled separately from the usual
/// dispatch mechanism.
class TypeInferenceImpl {
  /// Map from various values to their types prior to the pass.
  /// Store types for Instruction, Parameter, Variable, Function.
  llvh::DenseMap<Value *, Type> prePassTypes_{};

 public:
  /// Run type inference on every instruction in the module.
  /// \return true when some types were changed.
  bool runOnModule(Module *M);

 private:
  /// Run type inference on an instruction.
  /// This just does a case-based dispatch.
  /// \return true when another iteration will be required to fully infer the
  ///   type of this instruction (either the type has changed or it hasn't been
  ///   fully resolved yet).
  bool inferInstruction(Instruction *inst) {
    if (inst->isTyped()) {
      // Typed instructions are excluded from TypeInference.
      return false;
    }

    Type originalTy = inst->getType();

    // Handle Phi instructions separately by invoking inferPhi directly.
    // Phi instructions can have a NoType operand (e.g. in a loop) that would
    // be unresolvable due to a cycle if we didn't visit it anyway.
    // If we didn't special-case this, we would have a cycle that would cause
    // an infinite loop due to always returning true from this function (below).
    if (auto *phi = llvh::dyn_cast<PhiInst>(inst)) {
      return inferPhi(phi);
    }

    // If one of the operands hasn't had its type inferred yet,
    // skip it and come back later, returning true to signify that we're not
    // done yet.
    for (unsigned int i = 0, e = inst->getNumOperands(); i < e; ++i) {
      Value *operand = inst->getOperand(i);
      if (operand->getType().isNoType()) {
        LLVM_DEBUG(
            dbgs() << llvh::format("Missing type for operand %u of ", i)
                   << inst->getName() << "(" << operand->getKindStr() << ")\n");
        return true;
      }
    }

    Type inferredTy = Type::createAnyType();

    // Attempt inference for the given instruction.
    // It's possible that inference will result in the same type being
    // assigned.
    switch (inst->getKind()) {
#define INCLUDE_HBC_INSTRS
#define DEF_VALUE(CLASS, PARENT)                  \
  case ValueKind::CLASS##Kind:                    \
    inferredTy = infer##CLASS(cast<CLASS>(inst)); \
    break;
#define DEF_TAG(NAME, PARENT)                       \
  case ValueKind::NAME##Kind:                       \
    inferredTy = infer##PARENT(cast<PARENT>(inst)); \
    break;
#include "hermes/IR/Instrs.def"
      default:
        llvm_unreachable("Invalid kind");
    }

    // Only return true if the type actually changed.
    inst->setType(inferredTy);
    checkAndSetPrePassType(inst);
    inferredTy = inst->getType();
    bool changed = inferredTy != originalTy;

    // For debugging, only output if things changed.
    if (changed) {
      ++NumTI;
      LLVM_DEBUG(
          dbgs() << "Inferred " << inst->getName() << ": " << inst->getType()
                 << "\n");
    }

    return changed;
  }

  /// Phi instructions are to be treated specially by the inference algorithm,
  /// so we put the logic for handling them directly in this function.
  /// \return true if the type changed or we need another iteration of
  ///   inference.
  bool inferPhi(PhiInst *inst) {
    // Check if the types of all incoming values match and if they do set the
    // value of the PHI to match the incoming values.
    unsigned numEntries = inst->getNumEntries();
    if (numEntries < 1)
      return false;

    llvh::SmallPtrSet<Value *, 8> visited;
    llvh::SmallPtrSet<Value *, 8> values;
    collectPHIInputs(visited, values, inst);

    Type originalTy = inst->getType();

    Type newTy = Type::createNoType();

    bool changed = false;

    // For all possible incoming values into this phi:
    for (auto *input : values) {
      Type T = input->getType();

      // If any phi input has no type inferred, set the changed flag.
      if (T.isNoType())
        changed = true;

      // If we already have the first type stored, make a union.
      newTy = Type::unionTy(T, newTy);
    }

    inst->setType(newTy);
    return newTy != originalTy || changed;
  }

  /// Run type inference on a the provided set of functions to a fixed point.
  bool runOnFunctions(llvh::ArrayRef<Function *> functions);

  Type inferSingleOperandInst(SingleOperandInst *inst) {
    hermes_fatal("This is not a concrete instruction");
  }
  Type inferAddEmptyStringInst(AddEmptyStringInst *inst) {
    return *inst->getInherentType();
  }
  Type inferAsNumberInst(AsNumberInst *inst) {
    return *inst->getInherentType();
  }
  Type inferAsNumericInst(AsNumericInst *inst) {
    if (inst->getSingleOperand()->getType().isNumberType()) {
      return Type::createNumber();
    }
    if (inst->getSingleOperand()->getType().isBigIntType()) {
      return Type::createBigInt();
    }
    return Type::createNumeric();
  }
  Type inferAsInt32Inst(AsInt32Inst *inst) {
    return *inst->getInherentType();
  }
  Type inferLoadStackInst(LoadStackInst *inst) {
    return inst->getSingleOperand()->getType();
  }
  Type inferMovInst(MovInst *inst) {
    Type srcType = inst->getSingleOperand()->getType();
    return srcType;
  }
  Type inferImplicitMovInst(ImplicitMovInst *inst) {
    Type srcType = inst->getSingleOperand()->getType();
    return srcType;
  }
  Type inferCoerceThisNSInst(CoerceThisNSInst *inst) {
    return *inst->getInherentType();
  }
  Type inferUnaryOperatorInst(UnaryOperatorInst *inst) {
    switch (inst->getKind()) {
      case ValueKind::UnaryVoidInstKind: // void
        return Type::createUndefined();
      case ValueKind::UnaryTypeofInstKind: // typeof
        return Type::createString();
      // https://tc39.es/ecma262/#sec-prefix-increment-operator
      // https://tc39.es/ecma262/#sec-postfix-increment-operator
      case ValueKind::UnaryIncInstKind: // ++
      // https://tc39.es/ecma262/#sec-prefix-decrement-operator
      // https://tc39.es/ecma262/#sec-postfix-decrement-operator
      case ValueKind::UnaryDecInstKind: // --
      // https://tc39.es/ecma262/#sec-unary-minus-operator
      case ValueKind::UnaryMinusInstKind: // -
        return inferUnaryArithDefault(inst);
      // https://tc39.es/ecma262/#sec-bitwise-not-operator
      case ValueKind::UnaryTildeInstKind: // ~
        return inferTilde(inst);
      case ValueKind::UnaryBangInstKind: // !
        return Type::createBoolean();
      default:
        hermes_fatal("Invalid unary operator");
        break;
    }
  }
  Type inferDirectEvalInst(DirectEvalInst *inst) {
    return Type::createAnyType();
  }
  Type inferDeclareGlobalVarInst(DeclareGlobalVarInst *inst) {
    return Type::createNoType();
  }
  Type inferLoadFrameInst(LoadFrameInst *inst) {
    Type T = inst->getLoadVariable()->getType();
    return T;
  }
  Type inferHBCLoadConstInst(HBCLoadConstInst *inst) {
    return inst->getSingleOperand()->getType();
  }
  Type inferLoadParamInst(LoadParamInst *inst) {
    // Return the type that has been inferred for the parameter.
    return inst->getParam()->getType();
  }
  Type inferHBCResolveEnvironment(HBCResolveEnvironment *inst) {
    return Type::createEnvironment();
  }
  Type inferHBCGetArgumentsLengthInst(HBCGetArgumentsLengthInst *inst) {
    return Type::createNumber();
  }
  Type inferHBCReifyArgumentsInst(HBCReifyArgumentsInst *inst) {
    hermes_fatal("This is not a concrete instruction");
  }
  Type inferHBCReifyArgumentsLooseInst(HBCReifyArgumentsLooseInst *inst) {
    // Does not return a value, uses a lazy register instead.
    return Type::createNoType();
  }
  Type inferHBCReifyArgumentsStrictInst(HBCReifyArgumentsStrictInst *inst) {
    // Does not return a value, uses a lazy register instead.
    return Type::createNoType();
  }
  Type inferHBCSpillMovInst(HBCSpillMovInst *inst) {
    return inst->getSingleOperand()->getType();
  }

  Type inferPhiInst(PhiInst *inst) {
    // This is a dummy function that shouldn't be called.
    hermes_fatal("Phis are to be handled specially by inferPhi()");
  }
  Type inferBinaryOperatorInst(BinaryOperatorInst *inst) {
    return inferBinaryInst(inst);
  }
  Type inferStorePropertyLooseInst(StorePropertyLooseInst *inst) {
    return Type::createNoType();
  }
  Type inferStorePropertyStrictInst(StorePropertyStrictInst *inst) {
    return Type::createNoType();
  }
  Type inferTryStoreGlobalPropertyLooseInst(
      TryStoreGlobalPropertyLooseInst *inst) {
    return Type::createNoType();
  }
  Type inferTryStoreGlobalPropertyStrictInst(
      TryStoreGlobalPropertyStrictInst *inst) {
    return Type::createNoType();
  }

  Type inferStoreOwnPropertyInst(StoreOwnPropertyInst *inst) {
    return Type::createNoType();
  }
  Type inferStoreNewOwnPropertyInst(StoreNewOwnPropertyInst *inst) {
    return Type::createNoType();
  }

  Type inferStoreGetterSetterInst(StoreGetterSetterInst *inst) {
    return Type::createNoType();
  }
  Type inferDeletePropertyLooseInst(DeletePropertyLooseInst *inst) {
    return Type::createBoolean();
  }
  Type inferDeletePropertyStrictInst(DeletePropertyStrictInst *inst) {
    return Type::createBoolean();
  }
  Type inferLoadPropertyInst(LoadPropertyInst *inst) {
    return Type::createAnyType();
  }
  Type inferTryLoadGlobalPropertyInst(TryLoadGlobalPropertyInst *inst) {
    return Type::createAnyType();
  }
  Type inferStoreStackInst(StoreStackInst *inst) {
    return Type::createNoType();
  }
  Type inferStoreFrameInst(StoreFrameInst *inst) {
    return Type::createNoType();
  }
  Type inferAllocStackInst(AllocStackInst *inst) {
    // AllocStackInst is an exceptional case, since as a convenience we have
    // decided that it assumes the type of the allocated value (instead if
    // "pointer to the type of the allocated value"). So, if it is never used,
    // we can't infer anything, ending up with "notype". But we can't allow an
    // instruction with an output to have type "notype". So, if there are no
    // users, just assume the type is "any" as a convenience.
    return inst->hasUsers() ? inferMemoryLocationType(inst)
                            : Type::createAnyType();
  }
  Type inferAllocObjectInst(AllocObjectInst *inst) {
    return Type::createObject();
  }
  Type inferAllocArrayInst(AllocArrayInst *inst) {
    return *inst->getInherentType();
  }
  Type inferAllocFastArrayInst(AllocFastArrayInst *inst) {
    return *inst->getInherentType();
  }
  Type inferGetTemplateObjectInst(GetTemplateObjectInst *inst) {
    return *inst->getInherentType();
  }
  Type inferAllocObjectLiteralInst(AllocObjectLiteralInst *inst) {
    return *inst->getInherentType();
  }
  Type inferCreateArgumentsLooseInst(CreateArgumentsLooseInst *inst) {
    return *inst->getInherentType();
  }
  Type inferCreateArgumentsStrictInst(CreateArgumentsStrictInst *inst) {
    return *inst->getInherentType();
  }
  Type inferCatchInst(CatchInst *inst) {
    return Type::createAnyType();
  }
  Type inferDebuggerInst(DebuggerInst *inst) {
    return Type::createNoType();
  }
  Type inferCreateRegExpInst(CreateRegExpInst *inst) {
    return *inst->getInherentType();
  }
  Type inferTryEndInst(TryEndInst *inst) {
    return Type::createNoType();
  }
  Type inferGetNewTargetInst(GetNewTargetInst *inst) {
    return inst->getOperand(GetNewTargetInst::GetNewTargetParamIdx)->getType();
  }
  Type inferThrowIfInst(ThrowIfInst *inst) {
    Type type = inst->getCheckedValue()->getType();
    assert(!type.isNoType() && "input to throwIfEmpty cannot be NoType");

    if (LLVM_UNLIKELY(type.isEmptyType())) {
      // We remove "Empty" from the possible types of inst, which would result
      // in a "NoType" (i.e. TDZ is always going to throw), so instead we use
      // the last "valid" type we know about. While this might seem "hacky", it
      // eliminates a lot of complexity that would result from having to deal
      // with unreachable code expressed via the type system (T134361858).
      return inst->getSavedResultType();
    }

    return Type::subtractTy(type, Type::createEmpty());
  }
  Type inferIteratorBeginInst(IteratorBeginInst *inst) {
    return Type::createAnyType();
  }
  Type inferIteratorNextInst(IteratorNextInst *inst) {
    return Type::createAnyType();
  }
  Type inferIteratorCloseInst(IteratorCloseInst *inst) {
    return Type::createAnyType();
  }
  Type inferHBCStoreToEnvironmentInst(HBCStoreToEnvironmentInst *inst) {
    return Type::createNoType();
  }
  Type inferHBCLoadFromEnvironmentInst(HBCLoadFromEnvironmentInst *inst) {
    return inst->getResolvedName()->getType();
  }
  Type inferUnreachableInst(UnreachableInst *inst) {
    return Type::createNoType();
  }

  Type inferCreateFunctionInst(CreateFunctionInst *inst) {
    return *inst->getInherentType();
  }
  Type inferCreateGeneratorInst(CreateGeneratorInst *inst) {
    return *inst->getInherentType();
  }
  Type inferHBCCreateFunctionInst(HBCCreateFunctionInst *inst) {
    return *inst->getInherentType();
  }
  Type inferHBCCreateGeneratorInst(HBCCreateGeneratorInst *inst) {
    return *inst->getInherentType();
  }

  Type inferTerminatorInst(TerminatorInst *inst) {
    hermes_fatal("This is not a concrete instruction");
  }
  Type inferBranchInst(BranchInst *inst) {
    return Type::createNoType();
  }
  Type inferReturnInst(ReturnInst *inst) {
    return Type::createNoType();
  }
  Type inferThrowInst(ThrowInst *inst) {
    return Type::createNoType();
  }
  Type inferThrowTypeErrorInst(ThrowTypeErrorInst *inst) {
    return Type::createNoType();
  }
  Type inferSwitchInst(SwitchInst *inst) {
    return Type::createNoType();
  }
  Type inferCondBranchInst(CondBranchInst *inst) {
    return Type::createNoType();
  }
  Type inferGetPNamesInst(GetPNamesInst *inst) {
    return Type::createNoType();
  }
  Type inferGetNextPNameInst(GetNextPNameInst *inst) {
    return Type::createNoType();
  }
  Type inferTryStartInst(TryStartInst *inst) {
    return Type::createNoType();
  }
  Type inferCompareBranchInst(CompareBranchInst *inst) {
    return Type::createNoType();
  }
  Type inferSwitchImmInst(SwitchImmInst *inst) {
    return Type::createNoType();
  }
  Type inferSaveAndYieldInst(SaveAndYieldInst *inst) {
    return Type::createNoType();
  }

  Type inferCallInst(CallInst *inst) {
    // If the target of this call is known, propagate its return type.
    if (auto *F = llvh::dyn_cast<Function>(inst->getTarget()))
      return F->getType();
    return Type::createAnyType();
  }
  Type inferCallBuiltinInst(CallBuiltinInst *inst) {
    switch (inst->getBuiltinIndex()) {
      case BuiltinMethod::Math_abs:
      case BuiltinMethod::Math_acos:
      case BuiltinMethod::Math_asin:
      case BuiltinMethod::Math_atan:
      case BuiltinMethod::Math_atan2:
      case BuiltinMethod::Math_ceil:
      case BuiltinMethod::Math_cos:
      case BuiltinMethod::Math_exp:
      case BuiltinMethod::Math_floor:
      case BuiltinMethod::Math_hypot:
      case BuiltinMethod::Math_imul:
      case BuiltinMethod::Math_log:
      case BuiltinMethod::Math_max:
      case BuiltinMethod::Math_min:
      case BuiltinMethod::Math_pow:
      case BuiltinMethod::Math_round:
      case BuiltinMethod::Math_sin:
      case BuiltinMethod::Math_sqrt:
      case BuiltinMethod::Math_tan:
      case BuiltinMethod::Math_trunc:
        return Type::createNumber();
      default:
        return Type::createAnyType();
    }
  }
  Type inferHBCCallNInst(HBCCallNInst *inst) {
    // unimplemented
    return Type::createAnyType();
  }

  Type inferGetBuiltinClosureInst(GetBuiltinClosureInst *inst) {
    return *inst->getInherentType();
  }
  Type inferStartGeneratorInst(StartGeneratorInst *inst) {
    return Type::createNoType();
  }
  Type inferResumeGeneratorInst(ResumeGeneratorInst *inst) {
    // Result of ResumeGeneratorInst is whatever the user passes to .next()
    // or .throw() to resume the generator, which we don't yet support
    // understanding the types of.
    return Type::createAnyType();
  }

  // These are target dependent instructions:

  Type inferHBCGetGlobalObjectInst(HBCGetGlobalObjectInst *inst) {
    return *inst->getInherentType();
  }
  Type inferHBCCreateEnvironmentInst(HBCCreateEnvironmentInst *inst) {
    return Type::createEnvironment();
  }
  Type inferLIRGetThisNSInst(LIRGetThisNSInst *inst) {
    return Type::createObject();
  }
  Type inferCreateThisInst(CreateThisInst *inst) {
    return Type::createObject();
  }
  Type inferHBCGetArgumentsPropByValInst(HBCGetArgumentsPropByValInst *inst) {
    hermes_fatal("This is not a concrete instruction");
  }
  Type inferHBCGetArgumentsPropByValLooseInst(
      HBCGetArgumentsPropByValLooseInst *inst) {
    return Type::createAnyType();
  }
  Type inferHBCGetArgumentsPropByValStrictInst(
      HBCGetArgumentsPropByValStrictInst *inst) {
    return Type::createAnyType();
  }
  Type inferGetConstructedObjectInst(GetConstructedObjectInst *inst) {
    return Type::createObject();
  }
  Type inferHBCAllocObjectFromBufferInst(HBCAllocObjectFromBufferInst *inst) {
    return *inst->getInherentType();
  }
  Type inferHBCProfilePointInst(HBCProfilePointInst *inst) {
    return Type::createNoType();
  }
  Type inferPrLoadInst(PrLoadInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferPrStoreInst(PrStoreInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFastArrayLoadInst(FastArrayLoadInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFastArrayStoreInst(FastArrayStoreInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFastArrayPushInst(FastArrayPushInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFastArrayAppendInst(FastArrayAppendInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFastArrayLengthInst(FastArrayLengthInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferLoadParentInst(LoadParentInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferStoreParentInst(StoreParentInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferNativeCallInst(NativeCallInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFUnaryMathInst(FUnaryMathInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFBinaryMathInst(FBinaryMathInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferFCompareInst(FCompareInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferStringConcatInst(StringConcatInst *inst) {
    hermes_fatal("typed instruction");
  }
  Type inferUnionNarrowTrustedInst(UnionNarrowTrustedInst *inst) {
    auto res = Type::intersectTy(
        inst->getSavedResultType(), inst->getSingleOperand()->getType());

    // It may be possible that the input value would be proven to always be
    // empty and this code is unreachable. Similarly to ThrowIf, in that
    // case we simply return something to avoid breaking invariants.
    if (LLVM_UNLIKELY(res.isNoType())) {
      return inst->getSavedResultType();
    }
    return res;
  }
  Type inferCheckedTypeCastInst(CheckedTypeCastInst *inst) {
    Type inputType = inst->getCheckedValue()->getType();
    assert(
        !inputType.isNoType() && "input to CheckedTypeCast cannot be NoType");

    Type resultType =
        Type::intersectTy(inst->getSpecifiedType()->getData(), inputType);

    if (LLVM_UNLIKELY(resultType.isNoType())) {
      // Some checked casts can be proven at compile time to always throw. In
      // that case, the result type of CheckedTypeCastInst would theoretically
      // need to be something that represents an "unreachable" type. Handling
      // this "unreachable" type everywhere would require a lot of complexity.
      // Instead, when we get to that point, we simply return the known type.
      return inst->getSpecifiedType()->getData();
    }

    return resultType;
  }
  Type inferLIRDeadValueInst(LIRDeadValueInst *inst) {
    return inst->getSavedResultType();
  }

  /// If all call sites of this Function are known, propagate
  /// information from actuals to formals.
  void inferParams(Function *F) {
    if (!F->allCallsitesKnown()) {
      LLVM_DEBUG(
          dbgs() << F->getInternalName().str() << " has unknown call sites.\n");
      // If there are unknown call sites, we can't infer anything about the
      // parameters.
      for (auto *param : F->getJSDynamicParams()) {
        param->setType(Type::createAnyType());
        checkAndSetPrePassType(param);
      }
      return;
    }
    auto callsites = getKnownCallsites(F);
    LLVM_DEBUG(
        dbgs() << F->getInternalName().str() << " has " << callsites.size()
               << " call sites.\n");
    propagateArgs(callsites, F);
  }

  /// Propagate type information from call sites of F to formals of F.
  /// This assumes that all call sites of F are known.
  void propagateArgs(llvh::ArrayRef<BaseCallInst *> callSites, Function *F) {
    // Hermes does not support using 'arguments' to modify the arguments to a
    // function in loose mode. Therefore, we can safely propagate the parameter
    // types to their usage regardless of the function's strictness.
    IRBuilder builder(F);
    for (uint32_t i = 0, e = F->getJSDynamicParams().size(); i < e; ++i) {
      auto *P = F->getJSDynamicParam(i);
      Type paramTy = Type::createAnyType();
      bool first = true;

      // For each call sites.
      for (auto *call : callSites) {
        // The argument default value is undefined.
        Value *arg = builder.getLiteralUndefined();

        // Load the argument that's passed in.
        if (i < call->getNumArguments()) {
          arg = call->getArgument(i);
        }

        LLVM_DEBUG(
            dbgs() << F->getInternalName().c_str()
                   << "::" << P->getName().c_str()
                   << " found arg of type: " << arg->getType() << '\n');
        if (first) {
          paramTy = arg->getType();
          first = false;
        } else {
          paramTy = Type::unionTy(paramTy, arg->getType());
        }
      }

      Type originalTy = P->getType();

      if (first || paramTy.isNoType()) {
        // No information retrieved from call sites, bail.
        LLVM_DEBUG(
            dbgs()
            << F->getInternalName().c_str() << "::" << P->getName().c_str()
            << " failed to get info from callsites: defaulting to 'any'\n");
        P->setType(Type::createAnyType());
      } else {
        // Types should only be widened.
        P->setType(Type::unionTy(originalTy, paramTy));
      }

      checkAndSetPrePassType(P);

      if (P->getType() != originalTy) {
        LLVM_DEBUG(
            dbgs() << F->getInternalName().c_str()
                   << "::" << P->getName().c_str() << " changed to ");
        LLVM_DEBUG(P->getType().print(dbgs()));
        LLVM_DEBUG(dbgs() << "\n");
      }
    }
  }

  /// Infer the return type of \p F and register it.
  /// \return true if the return type was changed.
  bool inferFunctionReturnType(Function *F) {
    Type originalTy = F->getType();
    Type returnTy = Type::createAnyType();
    bool first = true;

    if (llvh::isa<GeneratorInnerFunction>(F)) {
      // GeneratorInnerFunctions may be called with `.return()` at the start,
      // with any value of any type.
      F->setType(Type::createAnyType());
      checkAndSetPrePassType(F);
      return F->getType() != originalTy;
    }

    for (auto &bbit : *F) {
      if (auto *returnInst =
              llvh::dyn_cast_or_null<ReturnInst>(bbit.getTerminator())) {
        Type T = returnInst->getValue()->getType();
        if (first && !T.isNoType()) {
          returnTy = T;
          first = false;
        } else {
          returnTy = Type::unionTy(returnTy, T);
        }
      }
    }
    F->setType(returnTy);
    checkAndSetPrePassType(F);
    return F->getType() != originalTy;
  }

  /// Attempt to infer the type of a variable stored in memory.
  /// \return true if the type changed.
  bool inferMemoryType(Value *V) {
    Type originalTy = V->getType();
    Type T = inferMemoryLocationType(V);

    // We were able to identify the type of the value. Record this info.
    if (T != V->getType()) {
      V->setType(T);
      checkAndSetPrePassType(V);
      return V->getType() != originalTy;
    }
    return false;
  }

  /// Clear every type for instructions, return types, parameters and variables
  /// in the function provided.
  /// Store the pre-pass types in prePassTypes_.
  void clearTypesInFunction(Function *f) {
    // Instructions
    for (auto &bbit : *f) {
      for (auto &it : bbit) {
        Instruction *inst = &it;
        if (inst->isTyped()) {
          // Typed instructions preserve their types in TypeInference.
          // Don't modify them.
          continue;
        }
        llvh::Optional<Type> inherent = inst->getInherentType();
        prePassTypes_.try_emplace(inst, inst->getType());
        // Clear to the inherent type if possible.
        inst->setType(inherent ? *inherent : Type::createNoType());
      }
    }
    // Parameters
    for (auto *P : f->getJSDynamicParams()) {
      prePassTypes_.try_emplace(P, P->getType());
      P->setType(Type::createNoType());
    }
    // Variables
    for (auto *V : f->getFunctionScope()->getVariables()) {
      prePassTypes_.try_emplace(V, V->getType());
      V->setType(Type::createNoType());
    }
    // Return type
    prePassTypes_.try_emplace(f, f->getType());
    f->setType(Type::createNoType());
  }

  /// Ensure that the type of \p val is not wider than its type prior to the
  /// pass by checking against the pre-pass type and intersecting the type with
  /// it when the pre-pass type is different than \p val's type.
  /// \return true when the type of \p val was changed.
  bool checkAndSetPrePassType(Value *val) {
    auto it = prePassTypes_.find(val);
    if (it == prePassTypes_.end()) {
      return false;
    }
    if (it->second != val->getType()) {
      Type intersection = Type::intersectTy(it->second, val->getType());
      // Narrow the type to include what we knew before the pass.
      LLVM_DEBUG(
          llvh::errs() << "Intersecting type of " << val->getKindStr()
                       << " from " << val->getType() << " with " << it->second
                       << " to " << intersection << '\n');
      val->setType(intersection);
      return true;
    }
    return false;
  }
};

bool TypeInferenceImpl::runOnFunctions(llvh::ArrayRef<Function *> functions) {
  LLVM_DEBUG(
      dbgs() << "\nStart Type Inference on " << functions.size()
             << " functions.\n");

  for (Function *F : functions) {
    // Begin by clearing the existing types and storing pre-pass types.
    // This prevents us from relying on the previous inference pass's type info,
    // which can be too loose (if things have been simplified, etc.).
    clearTypesInFunction(F);
  }

  // Inferring the types of instructions can help us figure out the types of
  // variables. Typed variables can help us deduce the types of loads and other
  // values. This means that we need to iterate until we reach convergence.
  bool localChanged = false;
  do {
    LLVM_DEBUG(dbgs() << "\nStart TypeInference pass:\n");

    localChanged = false;

    // Infer the type of formal parameters, based on knowing the (full) set
    // of call sites from which this function may be invoked.
    for (Function *F : functions) {
      inferParams(F);
    }

    // Infer types of instructions.
    bool inferredInst = false;
    for (Function *F : functions) {
      for (auto &bbit : *F) {
        for (auto &it : bbit) {
          Instruction *I = &it;
          inferredInst |= inferInstruction(I);
        }
      }
    }
    if (inferredInst)
      LLVM_DEBUG(dbgs() << ">> Inferred an instruction\n");
    localChanged |= inferredInst;

    // Infer the return type of the function based on the type of return
    // instructions in the function.
    bool inferredRetType = false;
    for (Function *F : functions) {
      inferredRetType |= inferFunctionReturnType(F);
    }
    if (inferredRetType)
      LLVM_DEBUG(dbgs() << ">> Inferred function return type\n");
    localChanged |= inferredRetType;

    // Infer type of F's variables.
    bool inferredVarType = false;
    for (Function *F : functions) {
      for (auto *V : F->getFunctionScope()->getVariables()) {
        inferredVarType |= inferMemoryType(V);
      }
    }
    if (inferredVarType)
      LLVM_DEBUG(dbgs() << ">> Inferred variable type\n");
    localChanged |= inferredVarType;
  } while (localChanged);

#ifndef NDEBUG
  // Validate that all instructions that need to have types do.
  for (Function *F : functions) {
    for (auto &BB : *F) {
      for (auto &I : BB) {
        assert(
            (I.getType().isNoType() ^ I.hasOutput()) &&
            "Instructions are NoType iff they have no outputs");
      }
    }
  }
#endif

  // Since we always infer from scratch, the inference has always "changed".
  return true;
}

/// Partition the functions in \p M into groups such that each group contains
/// all functions that have usages (either direct or via their variables) within
/// the same group, and no usages outside the group.
static std::vector<std::vector<Function *>> partitionFunctions(Module *M) {
  // EquivalenceClasses basically implements union-find (disjoint-set).
  // This is convenient for finding all the functions that share variables
  // or use each other.
  llvh::EquivalenceClasses<const Function *> funcGroups{};
  for (Function &F : *M) {
    // Add the function to the equivalence class, in case it doesn't have any
    // users or captured vars, we will create a new group.
    funcGroups.insert(&F);

    // NOTE: unionSets automatically inserts both arguments if they don't exist
    // before unioning.

    // Include any users of the function, to account for known callsites
    // as well as closure creation.
    for (const Instruction *user : F.getUsers()) {
      funcGroups.unionSets(&F, user->getFunction());
    }

    // Include any functions that capture the function's variables,
    // to allow those variables to be inferred.
    for (const Variable *V : F.getFunctionScope()->getVariables()) {
      for (const Instruction *user : V->getUsers()) {
        funcGroups.unionSets(&F, user->getFunction());
      }
    }
  }

  // Convert the EquivalenceClasses into a vector of vectors for faster
  // iteration.
  // Can't iterate the EquivalenceClasses directly because it uses pointers as
  // keys and we want a deterministic ordering.

  // Map from leader to index so we can use findLeader.
  llvh::DenseMap<const Function *, unsigned> funcGroupIndices;
  // List of the groups, where group i has a leader and
  // funcGroupIndices[leader] == i.
  std::vector<std::vector<Function *>> funcGroupsVec;
  for (Function &F : *M) {
    const Function *leader = *funcGroups.findLeader(&F);
    auto [it, inserted] =
        funcGroupIndices.try_emplace(leader, funcGroupsVec.size());
    if (inserted) {
      funcGroupsVec.emplace_back(1, &F);
    } else {
      funcGroupsVec[it->second].push_back(&F);
    }
  }
  return funcGroupsVec;
}

bool TypeInferenceImpl::runOnModule(Module *M) {
  bool changed = false;
  LLVM_DEBUG(dbgs() << "\nStart Type Inference on Module\n");

  auto partitionedFuncs = partitionFunctions(M);
  for (const auto &funcGroup : partitionedFuncs) {
    changed |= runOnFunctions(funcGroup);
  }
  return changed;
}

} // anonymous namespace

bool TypeInference::runOnModule(Module *M) {
  TypeInferenceImpl impl{};
  return impl.runOnModule(M);
}

Pass *hermes::createTypeInference() {
  return new TypeInference();
}

#undef DEBUG_TYPE
