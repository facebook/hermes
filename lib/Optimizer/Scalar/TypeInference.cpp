/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "typeinference"

#include "hermes/Optimizer/Scalar/TypeInference.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/Support/Debug.h"

using namespace hermes;
using llvh::dbgs;
using llvh::SmallPtrSetImpl;

STATISTIC(NumTI, "Number of instructions type inferred");
STATISTIC(NumRT, "Number of call sites type inferred");
STATISTIC(NumPT, "Number of parameters type inferred");
STATISTIC(
    UniquePropertyValue,
    "Number of instances of loads where there is a "
    "unique store(own) to that value");

namespace {

class TypeInferenceImpl {
  /// Call graph provider to use. There could be different implementations
  /// of the call graph provider.
  CallGraphProvider *cgp_;

  /// Run type inference on a CallInst, with the purpose of identifying the
  /// type of the return value produced by the call.
  bool inferCallInst(CallInst *CI);

  /// Run type inference on an Instruction; this just does a case-based
  /// dispatch.
  bool inferType(Instruction *I);

  /// Infer type information for formal parameters of a Function, based on the
  /// types of actual parameters at call sites.
  bool inferParams(Function *F);

  /// Infer type information for a property load (or an array index too)
  bool inferLoadPropertyInst(LoadPropertyInst *LPI);

  bool runOnFunction(Function *F);

 public:
  bool runOnModule(Module *M);
};

static bool inferUnaryArith(UnaryOperatorInst *UOI, Type numberResultType) {
  Value *op = UOI->getSingleOperand();

  if (op->getType().isNumberType()) {
    UOI->setType(numberResultType);
    return true;
  }

  if (op->getType().isBigIntType()) {
    UOI->setType(Type::createBigInt());
    return true;
  }

  Type mayBeBigInt =
      op->getType().canBeBigInt() ? Type::createBigInt() : Type::createNoType();

  // - ?? => Number|?BigInt. BigInt is only possible if op.Type canBeBigInt.
  UOI->setType(Type::unionTy(numberResultType, mayBeBigInt));
  return true;
}

static bool inferUnaryArithDefault(UnaryOperatorInst *UOI) {
  // - Number => Number
  // - BigInt => BigInt
  // - ?? => Number|BigInt
  return inferUnaryArith(UOI, Type::createNumber());
}

static bool inferTilde(UnaryOperatorInst *UOI) {
  // ~ Number => Int32
  // ~ BigInt => BigInt
  // ~ ?? => Int32|BigInt
  return inferUnaryArith(UOI, Type::createInt32());
}
} // anonymous namespace.

static bool inferUnaryInst(UnaryOperatorInst *UOI) {
  using OpKind = UnaryOperatorInst::OpKind;

  switch (UOI->getOperatorKind()) {
    case OpKind::DeleteKind: // delete:
      UOI->setType(Type::createBoolean());
      return true;
    case OpKind::VoidKind: // void
      UOI->setType(Type::createUndefined());
      return true;
    case OpKind::TypeofKind: // typeof
      UOI->setType(Type::createString());
      return true;
    // https://tc39.es/ecma262/#sec-prefix-increment-operator
    // https://tc39.es/ecma262/#sec-postfix-increment-operator
    case OpKind::IncKind: // ++
    // https://tc39.es/ecma262/#sec-prefix-decrement-operator
    // https://tc39.es/ecma262/#sec-postfix-decrement-operator
    case OpKind::DecKind: // --
    // https://tc39.es/ecma262/#sec-unary-minus-operator
    case OpKind::MinusKind: // -
      return inferUnaryArithDefault(UOI);
    // https://tc39.es/ecma262/#sec-unary-plus-operator
    case OpKind::PlusKind: // +
      UOI->setType(Type::createNumber());
      return true;
    // https://tc39.es/ecma262/#sec-bitwise-not-operator
    case OpKind::TildeKind: // ~
      return inferTilde(UOI);
    case OpKind::BangKind: // !
      UOI->setType(Type::createBoolean());
      return true;
    default:
      break;
  }

  return false;
}

/// Try to infer the type of the value that's stored into \p addr. \p addr is
/// either a stack location or a variable.
static Type inferMemoryLocationType(Value *addr) {
  bool first = true;
  Type T;

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

      // Loads do not change the type of the memory location.
      case ValueKind::LoadFrameInstKind:
      case ValueKind::LoadStackInstKind:
        continue;

      default:
        // Other instructions that may write to alloc stack thwart our analysis.
        return Type::createAnyType();
    }

    if (!storedVal)
      continue;

    Type storedType = storedVal->getType();

    if (first) {
      // This is the first value that we are encountering.
      T = storedType;
      first = false;
      continue;
    }

    T = Type::unionTy(T, storedType);
  }

  return T;
}

static bool inferMemoryType(Value *V) {
  Type T = inferMemoryLocationType(V);

  // We were able to identify the type of the value. Record this info.
  if (T.isProperSubsetOf(V->getType())) {
    V->setType(T);
    return true;
  }

  return false;
}

static bool inferLoadStackInst(LoadStackInst *LS) {
  Type T = LS->getSingleOperand()->getType();
  if (T.isProperSubsetOf(LS->getType())) {
    LS->setType(T);
    return true;
  }

  return false;
}

static bool inferLoadFrameInst(LoadFrameInst *LF) {
  Type T = LF->getLoadVariable()->getType();
  if (T.isProperSubsetOf(LF->getType())) {
    LF->setType(T);
    return true;
  }

  return false;
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

static bool inferPhiInstInst(PhiInst *P) {
  // Check if the types of all incoming values match and if they do set the
  // value of the PHI to match the incoming values.
  unsigned numEntries = P->getNumEntries();
  if (numEntries < 1)
    return false;

  llvh::SmallPtrSet<Value *, 8> visited;
  llvh::SmallPtrSet<Value *, 8> values;
  collectPHIInputs(visited, values, P);

  Type originalTy = P->getType();

  Type newTy;
  bool foundFirst = false;

  // For all possible incoming values into this phi:
  for (auto *input : values) {
    // If this is the first valid type remember it.
    if (!foundFirst) {
      newTy = input->getType();
      foundFirst = true;
      continue;
    }

    Type T = input->getType();
    newTy = Type::unionTy(T, newTy);
  }

  if (newTy.isProperSubsetOf(originalTy)) {
    P->setType(newTy);
    return true;
  } else {
    return false;
  }
}

namespace {
static bool inferBinaryArith(
    BinaryOperatorInst *BOI,
    Type numberType = Type::createNumber()) {
  Type LeftTy = BOI->getLeftHandSide()->getType();
  Type RightTy = BOI->getRightHandSide()->getType();

  // Number - Number => Number
  if (LeftTy.isNumberType() && RightTy.isNumberType()) {
    BOI->setType(numberType);
    return true;
  }

  // BigInt - BigInt => BigInt
  if (LeftTy.isBigIntType() && RightTy.isBigIntType()) {
    BOI->setType(Type::createBigInt());
    return true;
  }

  Type mayBeBigInt = LeftTy.canBeBigInt() && RightTy.canBeBigInt()
      ? Type::createBigInt()
      : Type::createNoType();

  // ?? - ?? => Number|?BigInt. BigInt is only possible if both operands can be
  // BigInt due to the no automatic BigInt conversion.
  BOI->setType(Type::unionTy(numberType, mayBeBigInt));
  return true;
}

static bool inferBinaryBitwise(BinaryOperatorInst *BOI) {
  Type LeftTy = BOI->getLeftHandSide()->getType();
  Type RightTy = BOI->getRightHandSide()->getType();

  Type mayBeBigInt = LeftTy.canBeBigInt() && RightTy.canBeBigInt()
      ? Type::createBigInt()
      : Type::createNoType();

  // ?? - ?? => Int32|?BigInt. BigInt is only possible if both operands can be
  // BigInt due to the no automatic BigInt conversion.
  BOI->setType(Type::unionTy(Type::createInt32(), mayBeBigInt));
  return true;
}
} // anonymous namespace

static bool inferBinaryInst(BinaryOperatorInst *BOI) {
  switch (BOI->getOperatorKind()) {
    // The following operations always return a boolean result.
    // They may throw, they may read/write memory, but the result of the
    // operation must be a boolean.
    case BinaryOperatorInst::OpKind::EqualKind:
    case BinaryOperatorInst::OpKind::NotEqualKind:
    case BinaryOperatorInst::OpKind::StrictlyEqualKind:
    case BinaryOperatorInst::OpKind::StrictlyNotEqualKind:
    case BinaryOperatorInst::OpKind::LessThanKind:
    case BinaryOperatorInst::OpKind::LessThanOrEqualKind:
    case BinaryOperatorInst::OpKind::GreaterThanKind:
    case BinaryOperatorInst::OpKind::GreaterThanOrEqualKind:
    case BinaryOperatorInst::OpKind::InKind:
    case BinaryOperatorInst::OpKind::InstanceOfKind:
      // Notice that the spec says that comparison of NaN should return
      // "Undefined" but all VMs return 'false'. We decided to conform to the
      // current implementation and not to the spec.
      BOI->setType(Type::createBoolean());
      return true;

    // These arithmetic operations always return a number or bigint:
    // https://262.ecma-international.org/#sec-multiplicative-operators
    case BinaryOperatorInst::OpKind::DivideKind:
    case BinaryOperatorInst::OpKind::MultiplyKind:
    // https://tc39.es/ecma262/#sec-subtraction-operator-minus
    case BinaryOperatorInst::OpKind::SubtractKind:
    // https://tc39.es/ecma262/#sec-left-shift-operator
    case BinaryOperatorInst::OpKind::LeftShiftKind:
    // https://tc39.es/ecma262/#sec-signed-right-shift-operator
    case BinaryOperatorInst::OpKind::RightShiftKind:
      return inferBinaryArith(BOI);

    case BinaryOperatorInst::OpKind::ModuloKind:
      return inferBinaryArith(BOI, Type::createInt32());

    // https://es5.github.io/#x11.7.3
    case BinaryOperatorInst::OpKind::UnsignedRightShiftKind:
      BOI->setType(Type::createUint32());
      return true;

    // The Add operator is special:
    // https://262.ecma-international.org/#sec-addition-operator-plus
    case BinaryOperatorInst::OpKind::AddKind: {
      Type LeftTy = BOI->getLeftHandSide()->getType();
      Type RightTy = BOI->getRightHandSide()->getType();
      // String + String -> String. It is enough for one of the operands to be
      // a string to force the result to be a string.
      if (LeftTy.isStringType() || RightTy.isStringType()) {
        BOI->setType(Type::createString());
        return true;
      }

      // Number + Number -> Number.
      if (LeftTy.isNumberType() && RightTy.isNumberType()) {
        BOI->setType(Type::createNumber());
        return true;
      }

      // BigInt + BigInt -> BigInt.
      if (LeftTy.isBigIntType() && RightTy.isBigIntType()) {
        BOI->setType(Type::createBigInt());
        return true;
      }

      // ?BigInt + ?BigInt => ?BigInt. Both operands need to "may be a BigInt"
      // for a possible BigInt result from this operator. This is true because
      // there's no automative BigInt type conversion.
      Type mayBeBigInt = (LeftTy.canBeBigInt() && RightTy.canBeBigInt())
          ? Type::createBigInt()
          : Type::createNoType();

      // handy alias for number|maybe(BigInt).
      Type numeric = Type::unionTy(Type::createNumber(), mayBeBigInt);

      // If both sides of the binary operand are known and both sides are known
      // to be non-string (and can't be converted to strings) then the result
      // must be of a numeric type.
      if (isSideEffectFree(LeftTy) && isSideEffectFree(RightTy) &&
          !LeftTy.canBeString() && !RightTy.canBeString()) {
        BOI->setType(numeric);
        return true;
      }

      // The plus operator always returns a number, bigint, or a string.
      BOI->setType(Type::unionTy(numeric, Type::createString()));
      return false;
    }

    // https://tc39.es/ecma262/#sec-binary-bitwise-operators
    case BinaryOperatorInst::OpKind::AndKind:
    case BinaryOperatorInst::OpKind::OrKind:
    case BinaryOperatorInst::OpKind::XorKind:
      return inferBinaryBitwise(BOI);

    default:
      break;
  }
  return false;
}

static bool inferFunctionReturnType(Function *F) {
  Type originalTy = F->getType();
  Type returnTy;
  bool first = true;

  if (llvh::isa<GeneratorInnerFunction>(F)) {
    // GeneratorInnerFunctions may be called with `.return()` at the start,
    // with any value of any type.
    return false;
  }

  for (auto &bbit : *F) {
    for (auto &it : bbit) {
      Instruction *I = &it;
      if (auto *RI = llvh::dyn_cast<ReturnInst>(I)) {
        Type T = RI->getType();
        if (first) {
          returnTy = T;
          first = false;
        } else {
          returnTy = Type::unionTy(returnTy, T);
        }
      }
    }
  }
  if (returnTy.isProperSubsetOf(originalTy)) {
    F->setType(returnTy);
    return true;
  }
  return false;
}

/// Propagate type information from call sites of F to formals of F.
/// This assumes that all call sites of F are known.
static bool propagateArgs(llvh::DenseSet<CallInst *> &callSites, Function *F) {
  bool changed = false;

  // In non strict mode a function can escape by accessing arguments.caller.
  // We don't try to infer the types of the parameters in non-strict mode,
  // unless "Hermes non-strict optimizations are enabled". These optimizations
  // allow us to benefit from the fact that Hermes doesn't implement some
  // aspects of non-strict mode, specifically in this case: modifying arguments
  // indirectly, argumentrs.caller, arguments.callee.
  if (!F->isStrictMode() &&
      !F->getContext()
           .getOptimizationSettings()
           .aggressiveNonStrictModeOptimizations) {
    return changed;
  }

  IRBuilder builder(F);
  for (int i = 0, e = F->getParameters().size(); i < e; i++) {
    auto *P = F->getParameters()[i];

    Type paramTy;
    bool first = true;

    // For each call sites.
    for (auto *call : callSites) {
      // The argument default value is undefined.
      Value *arg = builder.getLiteralUndefined();

      // Skip the 'this' argument.
      unsigned argIdx = i + 1;

      // Load the argument that's passed in.
      if (argIdx < call->getNumArguments()) {
        arg = call->getArgument(argIdx);
      }

      if (first) {
        paramTy = arg->getType();
        first = false;
      } else {
        paramTy = Type::unionTy(paramTy, arg->getType());
      }
    }

    // Update the type if we have new information.
    if (!first && paramTy.isProperSubsetOf(P->getType())) {
      P->setType(paramTy);
      LLVM_DEBUG(
          dbgs() << F->getInternalName().c_str() << "::" << P->getName().c_str()
                 << " changed to ");
      LLVM_DEBUG(paramTy.print(dbgs()));
      LLVM_DEBUG(dbgs() << "\n");
      changed = true;
    }
  }

  return changed;
}

/// If all call sites of this Function are known, propagate
/// information from actuals to formals.
bool TypeInferenceImpl::inferParams(Function *F) {
  bool changed;
  if (cgp_->hasUnknownCallsites(F)) {
    LLVM_DEBUG(
        dbgs() << F->getInternalName().str() << " has unknown call sites.\n");
    return false;
  }
  llvh::DenseSet<CallInst *> &callsites = cgp_->getKnownCallsites(F);
  LLVM_DEBUG(
      dbgs() << F->getInternalName().str() << " has " << callsites.size()
             << " call sites.\n");
  changed = propagateArgs(callsites, F);
  if (changed) {
    LLVM_DEBUG(
        dbgs() << "inferParams changed for function "
               << F->getInternalName().str() << "\n");
    NumPT++;
  }
  return changed;
}

/// Propagate return type from potential callees to a CallInst.
/// Assumes that each funcs' type has been inferred, in F->getType().
static bool propagateReturn(llvh::DenseSet<Function *> &funcs, CallInst *CI) {
  bool changed = false;
  bool first = true;
  Type retTy;

  for (auto *F : funcs) {
    if (first) {
      retTy = F->getType();
      first = false;
    } else {
      retTy = Type::unionTy(retTy, F->getType());
    }
  }

  if (!first && retTy.isProperSubsetOf(CI->getType())) {
    CI->setType(retTy);
    LLVM_DEBUG(dbgs() << CI->getName().str() << " changed to ");
    LLVM_DEBUG(retTy.print(dbgs()));
    LLVM_DEBUG(dbgs() << "\n");
    changed = true;
  }

  return changed;
}

/// If all callees are known, propagate information from returns
/// of those callees to this CallInst.
bool TypeInferenceImpl::inferCallInst(CallInst *CI) {
  bool changed = false;
  if (cgp_->hasUnknownCallees(CI)) {
    LLVM_DEBUG(
        dbgs() << "Unknown callees for : " << CI->getName().str() << "\n");
    return false;
  }
  llvh::DenseSet<Function *> &callees = cgp_->getKnownCallees(CI);
  LLVM_DEBUG(
      dbgs() << "Found " << callees.size()
             << " callees for : " << CI->getName().str() << "\n");
  changed = propagateReturn(callees, CI);
  if (changed) {
    LLVM_DEBUG(dbgs() << "inferCallInst changed!\n");
    NumRT++;
  }
  return changed;
}

static bool inferReturnInst(ReturnInst *RI) {
  Type originalTy = RI->getType();
  Value *operand = RI->getOperand(0);
  Type newTy = operand->getType();

  if (newTy.isProperSubsetOf(originalTy)) {
    RI->setType(newTy);
    return true;
  }
  return false;
}

/// Does a given prop belong in the owned set?
static bool isOwnedProperty(AllocObjectInst *I, Value *prop) {
  for (auto *J : I->getUsers()) {
    if (auto *SOPI = llvh::dyn_cast<StoreOwnPropertyInst>(J)) {
      if (SOPI->getObject() == I) {
        if (prop == SOPI->getProperty())
          return true;
      }
    }
  }
  return false;
}

bool TypeInferenceImpl::inferLoadPropertyInst(LoadPropertyInst *LPI) {
  bool changed = false;
  bool first = true;
  Type retTy;
  Type originalTy = LPI->getType();
  bool unique = true;

  // Bail out if there are unknown receivers.
  if (cgp_->hasUnknownReceivers(LPI))
    return false;

  // Go over each known receiver R (can be empty)
  for (auto *R : cgp_->getKnownReceivers(LPI)) {
    assert(llvh::isa<AllocObjectInst>(R));
    // Note: currently Array analysis is purposely disabled.

    // Bail out if there are unknown stores.
    if (cgp_->hasUnknownStores(R))
      return false;

    Value *prop = LPI->getProperty();

    // If the property being requested is NOT an owned prop, Bail out
    if (llvh::isa<AllocObjectInst>(R)) {
      if (!isOwnedProperty(cast<AllocObjectInst>(R), prop))
        return false;
    }

    // Go over each store of R (can be empty)
    for (auto *S : cgp_->getKnownStores(R)) {
      assert(
          llvh::isa<StoreOwnPropertyInst>(S) ||
          llvh::isa<StorePropertyInst>(S));
      Value *storeVal = nullptr;

      if (llvh::isa<AllocObjectInst>(R)) {
        // If the property in the store is what this LPI wants, skip the store.
        if (auto *SS = llvh::dyn_cast<StoreOwnPropertyInst>(S)) {
          storeVal = SS->getStoredValue();
          if (prop != SS->getProperty())
            continue;
        }
        if (auto *SS = llvh::dyn_cast<StorePropertyInst>(S)) {
          storeVal = SS->getStoredValue();
          if (prop != SS->getProperty())
            continue;
        }
      }

      if (llvh::isa<AllocArrayInst>(R)) {
        if (auto *SS = llvh::dyn_cast<StorePropertyInst>(S)) {
          storeVal =
              SS->getStoredValue(); // for arrays, no need to match prop name
        }
      }

      assert(storeVal != nullptr);

      if (first) {
        retTy = storeVal->getType();
        first = false;
      } else {
        retTy = Type::unionTy(retTy, storeVal->getType());
        unique = false;
      }
    }
  }
  if (!first && unique) {
    UniquePropertyValue++;
  }
  if (!first && retTy.isProperSubsetOf(originalTy)) {
    LPI->setType(retTy);
    return true;
  }
  return changed;
}

/// Infer type information for "ThrowIfEmpty", which simply removes the
/// possibility for "empty" from its operand.
static bool inferThrowIfEmptyInst(ThrowIfEmptyInst *TIE) {
  TIE->setType(
      Type::subtractTy(TIE->getCheckedValue()->getType(), Type::createEmpty()));
  return true;
}

/// Attempts to infer the type of instruction \p I based on the environment.
/// \returns true if the type of the instruction was deduced.
/// This method contains a bunch of rules that conform to the JavaScript
/// language.
bool TypeInferenceImpl::inferType(Instruction *I) {
  Type originalTy = I->getType();

  switch (I->getKind()) {
    case ValueKind::BinaryOperatorInstKind:
      NumTI += inferBinaryInst(cast<BinaryOperatorInst>(I));
      return I->getType() != originalTy;

    case ValueKind::UnaryOperatorInstKind:
      NumTI += inferUnaryInst(cast<UnaryOperatorInst>(I));
      return I->getType() != originalTy;

    case ValueKind::PhiInstKind:
      NumTI += inferPhiInstInst(cast<PhiInst>(I));
      return I->getType() != originalTy;

    case ValueKind::AllocStackInstKind:
      NumTI += inferMemoryType(cast<AllocStackInst>(I));
      return I->getType() != originalTy;

    case ValueKind::LoadStackInstKind:
      NumTI += inferLoadStackInst(cast<LoadStackInst>(I));
      return I->getType() != originalTy;

    case ValueKind::LoadFrameInstKind:
      NumTI += inferLoadFrameInst(cast<LoadFrameInst>(I));
      return I->getType() != originalTy;

    case ValueKind::CallInstKind:
      NumTI += inferCallInst(cast<CallInst>(I));
      return I->getType() != originalTy;

    case ValueKind::ReturnInstKind:
      NumTI += inferReturnInst(cast<ReturnInst>(I));
      return I->getType() != originalTy;

    case ValueKind::LoadPropertyInstKind:
      NumTI += inferLoadPropertyInst(cast<LoadPropertyInst>(I));
      return I->getType() != originalTy;

    case ValueKind::ThrowIfEmptyInstKind:
      NumTI += inferThrowIfEmptyInst(cast<ThrowIfEmptyInst>(I));
      return I->getType() != originalTy;

    default:
      // Not sure how to infer the type here.
      return false;
  }
}

bool TypeInferenceImpl::runOnFunction(Function *F) {
  bool changed = false;
  bool localChanged = false;

  LLVM_DEBUG(
      dbgs() << "\nStart Type Inference on " << F->getInternalName().c_str()
             << "\n");

  // Infer the type of formal parameters, based on knowing the (full) set
  // of call sites from which this function may be invoked.
  // This information changes based on call sites that are  in other functions,
  // so we might as well do this outside the loop because the type information
  // for those call sites will not change in the loop (except for recursive
  // functions.)
  changed |= inferParams(F);

  // Inferring the types of instructions can help us figure out the types of
  // variables. Typed variables can help us deduce the types of loads and other
  // values. This means that we need to iterate until we reach convergence.
  do {
    localChanged = false;

    // Infer types of instructions.
    for (auto &bbit : *F) {
      for (auto &it : bbit) {
        Instruction *I = &it;
        localChanged |= inferType(I);
      }
    }

    // Infer the return type of the function based on the type of return
    // instructions in the function.
    localChanged |= inferFunctionReturnType(F);

    // Infer type of F's variables, except if F is in global scope
    if (!F->isGlobalScope()) {
      for (auto *V : F->getFunctionScopeDesc()->getVariables()) {
        localChanged |= inferMemoryType(V);
      }
    }

    changed |= localChanged;
  } while (localChanged);

  return changed;
}

bool TypeInferenceImpl::runOnModule(Module *M) {
  bool changed = false;

  LLVM_DEBUG(dbgs() << "\nStart Type Inference on Module\n");

  for (auto &F : *M) {
    SimpleCallGraphProvider scgp(&F);
    cgp_ = &scgp;
    changed |= runOnFunction(&F);
  }
  return changed;
}

bool TypeInference::runOnModule(Module *M) {
  TypeInferenceImpl impl{};
  return impl.runOnModule(M);
}

Pass *hermes::createTypeInference() {
  return new TypeInference();
}

#undef DEBUG_TYPE
