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
STATISTIC(NumPT, "Number of parameters type inferred");
STATISTIC(
    UniquePropertyValue,
    "Number of instances of loads where there is a "
    "unique store(own) to that value");

namespace {

static Type inferUnaryArith(UnaryOperatorInst *UOI, Type numberResultType) {
  Value *op = UOI->getSingleOperand();

  if (op->getType().isNumberType()) {
    return numberResultType;
  }

  if (op->getType().isBigIntType()) {
    return Type::createBigInt();
  }

  Type mayBeBigInt =
      op->getType().canBeBigInt() ? Type::createBigInt() : Type::createNoType();

  // - ?? => Number|?BigInt. BigInt is only possible if op.Type canBeBigInt.
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

    if (first && !storedType.isNoType()) {
      // This is the first value that we are encountering.
      T = storedType;
      first = false;
      continue;
    }

    T = Type::unionTy(T, storedType);
  }

  return T;
}

/// Attempt to infer the type of a variable stored in memory.
/// \return true if the type changed.
static bool inferMemoryType(Value *V) {
  Type T = inferMemoryLocationType(V);

  // We were able to identify the type of the value. Record this info.
  if (T != V->getType()) {
    V->setType(T);
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

  Type mayBeBigInt = LeftTy.canBeBigInt() && RightTy.canBeBigInt()
      ? Type::createBigInt()
      : Type::createNoType();

  // ?? - ?? => Number|?BigInt. BigInt is only possible if both operands can be
  // BigInt due to the no automatic BigInt conversion.
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

/// Infer the return type of \p F and register it.
/// \return true if the return type was changed.
static bool inferFunctionReturnType(Function *F) {
  Type originalTy = F->getType();
  Type returnTy;
  bool first = true;

  if (llvh::isa<GeneratorInnerFunction>(F)) {
    // GeneratorInnerFunctions may be called with `.return()` at the start,
    // with any value of any type.
    returnTy = Type::createAnyType();
    if (returnTy != originalTy) {
      F->setType(returnTy);
      return true;
    }
    return false;
  }

  for (auto &bbit : *F) {
    for (auto &it : bbit) {
      Instruction *I = &it;
      if (auto *RI = llvh::dyn_cast<ReturnInst>(I)) {
        Type T = RI->getType();
        if (first && !T.isNoType()) {
          returnTy = T;
          first = false;
        } else {
          returnTy = Type::unionTy(returnTy, T);
        }
      }
    }
  }
  if (returnTy != originalTy) {
    F->setType(returnTy);
    return true;
  }
  return false;
}

/// Propagate type information from call sites of F to formals of F.
/// This assumes that all call sites of F are known.
static bool propagateArgs(
    llvh::DenseSet<BaseCallInst *> &callSites,
    Function *F) {
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

    if (first) {
      // No information retrieved from call sites, bail.
      P->setType(Type::createAnyType());
      changed = true;
    } else {
      // Update the type if we have new information.
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

/// Propagate return type from potential callees to a CallInst.
/// Assumes that each funcs' type has been inferred, in F->getType().
static Type propagateReturn(llvh::DenseSet<Function *> &funcs, CallInst *CI) {
  bool first = true;
  Type retTy;

  for (auto *F : funcs) {
    if (first && !F->getType().isNoType()) {
      retTy = F->getType();
      first = false;
    } else {
      retTy = Type::unionTy(retTy, F->getType());
    }
  }

  if (!first) {
    LLVM_DEBUG(dbgs() << CI->getName().str() << " changed to ");
    LLVM_DEBUG(retTy.print(dbgs()));
    LLVM_DEBUG(dbgs() << "\n");
    return retTy;
  }

  return Type::createAnyType();
}

/// Does a given prop belong in the owned set?
static bool isOwnedProperty(AllocObjectInst *I, Value *prop) {
  for (auto *J : I->getUsers()) {
    if (auto *SOPI = llvh::dyn_cast<BaseStoreOwnPropertyInst>(J)) {
      if (SOPI->getObject() == I) {
        if (prop == SOPI->getProperty())
          return true;
      }
    }
  }
  return false;
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
  /// Call graph provider to use. There could be different implementations
  /// of the call graph provider.
  CallGraphProvider *cgp_;

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
    LLVM_DEBUG(dbgs() << "Inferring " << inst->getName() << "\n");
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

    Type inferredTy;

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

    assert(
        (inferredTy.isNoType() ^ inst->hasOutput()) &&
        "Instructions are NoType iff they have no outputs");

    // Only return true if the type actually changed.
    bool changed = inferredTy != originalTy;

    // For debugging, only output if things changed.
    if (changed) {
      ++NumTI;
      inst->setType(inferredTy);
      LLVM_DEBUG(dbgs() << "Inferred " << inst->getName() << "\n");
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

  bool runOnFunction(Function *F);

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
    return *inst->getInherentType();
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
  Type inferLoadFrameInst(LoadFrameInst *inst) {
    Type T = inst->getSingleOperand()->getType();
    return T;
  }
  Type inferHBCLoadConstInst(HBCLoadConstInst *inst) {
    return inst->getSingleOperand()->getType();
  }
  Type inferHBCLoadParamInst(HBCLoadParamInst *inst) {
    return Type::createAnyType();
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
  Type inferDeletePropertyInst(DeletePropertyInst *inst) {
    return Type::createBoolean();
  }
  Type inferLoadPropertyInst(LoadPropertyInst *inst) {
    bool first = true;
    Type retTy;
    bool unique = true;

    // Bail out if there are unknown receivers.
    if (cgp_->hasUnknownReceivers(inst)) {
      return Type::createAnyType();
    }

    // Go over each known receiver R (can be empty)
    for (auto *R : cgp_->getKnownReceivers(inst)) {
      assert(llvh::isa<AllocObjectInst>(R));
      // Note: currently Array analysis is purposely disabled.

      // Bail out if there are unknown stores.
      if (cgp_->hasUnknownStores(R)) {
        return Type::createAnyType();
      }

      Value *prop = inst->getProperty();

      // If the property being requested is NOT an owned prop, Bail out
      if (llvh::isa<AllocObjectInst>(R)) {
        if (!isOwnedProperty(cast<AllocObjectInst>(R), prop)) {
          return Type::createAnyType();
        }
      }

      // Go over each store of R (can be empty)
      for (auto *S : cgp_->getKnownStores(R)) {
        assert(
            llvh::isa<BaseStoreOwnPropertyInst>(S) ||
            llvh::isa<BaseStorePropertyInst>(S));
        Value *storeVal = nullptr;

        if (llvh::isa<AllocObjectInst>(R)) {
          // If the property in the store is what this inst wants, skip the
          // store.
          if (auto *SS = llvh::dyn_cast<BaseStoreOwnPropertyInst>(S)) {
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
    if (!first) {
      return retTy;
    }
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
    return inferMemoryLocationType(inst);
  }
  Type inferAllocObjectInst(AllocObjectInst *inst) {
    return Type::createObject();
  }
  Type inferAllocArrayInst(AllocArrayInst *inst) {
    return *inst->getInherentType();
  }
  Type inferAllocObjectLiteralInst(AllocObjectLiteralInst *inst) {
    return *inst->getInherentType();
  }
  Type inferCreateArgumentsInst(CreateArgumentsInst *inst) {
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
    return Type::createAnyType();
  }
  Type inferThrowIfEmptyInst(ThrowIfEmptyInst *inst) {
    // TODO(T134361858): This can remove "Empty" from the possible types of
    // inst, but that could result in a "NoType" (e.g. if the TDZ is always
    // going to throw), so we avoid doing that for now.
    return inst->getCheckedValue()->getType();
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
    return Type::createAnyType();
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
#ifdef HERMES_RUN_WASM
  Type inferCallIntrinsicInst(CallIntrinsicInst *inst) {
    // unimplemented
    return false;
  }
#endif

  Type inferTerminatorInst(TerminatorInst *inst) {
    hermes_fatal("This is not a concrete instruction");
  }
  Type inferBranchInst(BranchInst *inst) {
    return Type::createNoType();
  }
  Type inferReturnInst(ReturnInst *inst) {
    return inst->getOperand(0)->getType();
  }
  Type inferThrowInst(ThrowInst *inst) {
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
  Type inferCheckHasInstanceInst(CheckHasInstanceInst *inst) {
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
    if (cgp_->hasUnknownCallees(inst)) {
      LLVM_DEBUG(
          dbgs() << "Unknown callees for : " << inst->getName().str() << "\n");
      return Type::createAnyType();
    }
    llvh::DenseSet<Function *> &callees = cgp_->getKnownCallees(inst);
    LLVM_DEBUG(
        dbgs() << "Found " << callees.size()
               << " callees for : " << inst->getName().str() << "\n");
    return propagateReturn(callees, inst);
  }
  Type inferConstructInst(ConstructInst *inst) {
    return Type::createObject();
  }
  Type inferCallBuiltinInst(CallBuiltinInst *inst) {
    // unimplemented
    return Type::createAnyType();
  }
  Type inferHBCConstructInst(HBCConstructInst *inst) {
    return Type::createObject();
  }
  Type inferHBCCallDirectInst(HBCCallDirectInst *inst) {
    // unimplemented
    return Type::createAnyType();
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
  Type inferHBCGetThisNSInst(HBCGetThisNSInst *inst) {
    return Type::createObject();
  }
  Type inferHBCCreateThisInst(HBCCreateThisInst *inst) {
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
  Type inferHBCGetConstructedObjectInst(HBCGetConstructedObjectInst *inst) {
    return Type::createObject();
  }
  Type inferHBCAllocObjectFromBufferInst(HBCAllocObjectFromBufferInst *inst) {
    return *inst->getInherentType();
  }
  Type inferHBCProfilePointInst(HBCProfilePointInst *inst) {
    return Type::createNoType();
  }

  /// If all call sites of this Function are known, propagate
  /// information from actuals to formals.
  bool inferParams(Function *F) {
    bool changed;
    if (cgp_->hasUnknownCallsites(F)) {
      LLVM_DEBUG(
          dbgs() << F->getInternalName().str() << " has unknown call sites.\n");
      // If there are unknown call sites, we can't infer anything about the
      // parameters.
      for (Parameter *param : F->getParameters()) {
        param->setType(Type::createAnyType());
      }
      return true;
    }
    llvh::DenseSet<BaseCallInst *> &callsites = cgp_->getKnownCallsites(F);
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
};

/// Clear every type for instructions, return types, parameters and variables
/// in the function provided.
static void clearTypesInFunction(Function *f) {
  // Instructions
  for (auto &bbit : *f) {
    for (auto &it : bbit) {
      Instruction *inst = &it;
      OptValue<Type> inherent = inst->getInherentType();
      // Clear to the inherent type if possible.
      inst->setType(inherent ? *inherent : Type::createNoType());
    }
  }
  // Parameters
  for (Value *param : f->getParameters()) {
    param->setType(Type::createNoType());
  }
  // Variables
  if (!f->isGlobalScope()) {
    for (auto *V : f->getFunctionScope()->getVariables()) {
      V->setType(Type::createNoType());
    }
  }
  // Return type
  f->setType(Type::createNoType());
}

bool TypeInferenceImpl::runOnFunction(Function *F) {
  bool changed = false;
  bool localChanged = false;

  LLVM_DEBUG(
      dbgs() << "\nStart Type Inference on " << F->getInternalName().c_str()
             << "\n");

  // Begin by clearing the existing types.
  // This prevents us from relying on the previous inference pass's type info,
  // which can be too loose (if things have been simplified, etc.).
  clearTypesInFunction(F);

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
    bool inferredInst = false;
    for (auto &bbit : *F) {
      for (auto &it : bbit) {
        Instruction *I = &it;
        inferredInst |= inferInstruction(I);
      }
    }
    if (inferredInst)
      LLVM_DEBUG(dbgs() << "Inferred an instruction\n");
    localChanged |= inferredInst;

    // Infer the return type of the function based on the type of return
    // instructions in the function.
    bool inferredRetType = inferFunctionReturnType(F);
    if (inferredRetType)
      LLVM_DEBUG(dbgs() << "Inferred function return type\n");
    localChanged |= inferredRetType;

    // Infer type of F's variables, except if F is in global scope
    bool inferredVarType = false;
    if (!F->isGlobalScope()) {
      for (auto *V : F->getFunctionScope()->getVariables()) {
        inferredVarType |= inferMemoryType(V);
      }
    }
    if (inferredVarType)
      LLVM_DEBUG(dbgs() << "Inferred variable type\n");
    localChanged |= inferredVarType;

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

} // anonymous namespace

bool TypeInference::runOnModule(Module *M) {
  TypeInferenceImpl impl{};
  return impl.runOnModule(M);
}

Pass *hermes::createTypeInference() {
  return new TypeInference();
}

#undef DEBUG_TYPE
