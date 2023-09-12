/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IRVerifier.h"

#include "hermes/IR/CFG.h"
#include "hermes/IR/IRVisitor.h"
#include "hermes/IR/Instrs.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/Casting.h"

using llvh::dyn_cast;
using llvh::isa;
using llvh::raw_null_ostream;

using namespace hermes;

#ifdef HERMES_SLOW_DEBUG

namespace {

#define INCLUDE_ALL_INSTRS

/// IR Verifier - the verifier checks some basic properties of the IR to make
/// sure that it is not incorrect. For example, it checks that instructions are
/// dominated by their oparands and that the entry block has no predecessors.
///
/// The verifier also checks if the IR is in optimized form that allows bytecode
/// generation. For example, it checks that there are no unreachable basic
/// blocks.
class Verifier : public InstructionVisitor<Verifier, void> {
  class FunctionState; // Forward declaration of inner class.

  bool valid{true};
  Context *Ctx{nullptr};
  raw_ostream &OS;
  VerificationMode verificationMode;
  /// State for the function currently being verified.
  FunctionState *functionState{};
  /// Cached users of each value, lazily computed.
  llvh::DenseMap<const Value *, llvh::DenseSet<const Value *>> cachedUsers{};

  /// Verifier state per-function is kept here.
  class FunctionState {
    Verifier *verifier;
    /// Save the previous state here so we can restore it in our destructor.
    FunctionState *oldState;

   public:
    const Function &function;
    bool createArgumentsEncountered = false;

    FunctionState(Verifier *verifier, const Function &function)
        : verifier(verifier),
          oldState(verifier->functionState),
          function(function) {
      verifier->functionState = this;
    }

    ~FunctionState() {
      verifier->functionState = oldState;
    }
  };

  bool isVerifyingOptimalIR() {
    return verificationMode == VerificationMode::IR_OPTIMIZED;
  }

 public:
  explicit Verifier(raw_ostream &OS, VerificationMode mode)
      : OS(OS), verificationMode(mode) {}

  bool verify(const Module &M) {
    valid = true;
    Ctx = &M.getContext();

    visitModule(M);

    return valid;
  };

  /// Reimplement in Verifier class for custom behavior.
  void beforeVisitInstruction(const Instruction &I);

  void visitModule(const Module &M);
  void visitFunction(const Function &F);
  void visitBasicBlock(const BasicBlock &BB);

  void visitBaseStoreOwnPropertyInst(const BaseStoreOwnPropertyInst &Inst);
  void visitBaseCreateCallableInst(const BaseCreateCallableInst &Inst);

  void visitCreateArgumentsInst(const CreateArgumentsInst &Inst);

#define DEF_VALUE(XX, PARENT) void visit##XX(const XX &Inst);
#define BEGIN_VALUE(XX, PARENT) DEF_VALUE(XX, PARENT)
#include "hermes/IR/Instrs.def"

 private:
  /// Implement verification for a switch-like instruction.
  template <class T>
  void visitSwitchLikeInst(const T &Inst);
  /// Implement verification for a conditional branch-like instruction.
  template <class T>
  void visitCondBranchLikeInst(const T &Inst);
  /// Implement verification for a binaryoperator-like instruction.
  template <class T>
  void visitBinaryOperatorLikeInst(const T &Inst);

  /// \return a DenseSet of users for a Value, computed lazily.
  const llvh::DenseSet<const Value *> &getUsersSetForValue(const Value *V) {
    auto &users = cachedUsers[V];
    if (users.empty()) {
      users.insert(V->users_begin(), V->users_end());
    }
    return users;
  }

  /// Assert that the attributes of \p val are allowed to be set, based on the
  /// kind of \p val.
  void verifyAttributes(const Value *val, Module *M);
};

// TODO: Need to make this accept format strings
#define Assert(C, ...)                                              \
  do {                                                              \
    if (!(C)) {                                                     \
      valid = false;                                                \
      if (functionState)                                            \
        OS << (__VA_ARGS__) << " in function "                      \
           << functionState->function.getInternalNameStr() << '\n'; \
      else                                                          \
        OS << (__VA_ARGS__) << '\n';                                \
      return;                                                       \
    }                                                               \
  } while (0)

void Verifier::verifyAttributes(const Value *val, Module *M) {
  const auto &attrs = val->getAttributes(M);
#define ATTRIBUTE(valueKind, name, _string) \
  if (attrs.name)                           \
    Assert(llvh::isa<valueKind>(val), #name " must be set on a " #valueKind);
#include "hermes/IR/Attributes.def"
}

void Verifier::visitModule(const Module &M) {
  // Verify all functions are valid
  for (Module::const_iterator I = M.begin(); I != M.end(); I++) {
    Assert(I->getParent() == &M, "Function's parent does not match module");
    visitFunction(*I);
  }
}

void Verifier::visitFunction(const Function &F) {
  Assert(&F.getContext() == Ctx, "Function has wrong context");

  verifyAttributes(&F, F.getParent());

  for (Value *newTargetUser : F.getNewTargetParam()->getUsers()) {
    Assert(
        llvh::isa<GetNewTargetInst>(newTargetUser),
        "Only GetNewTargetInst may use the newTargetParam");
  }

  FunctionState newFunctionState(this, F);

  // Verify all basic blocks are valid
  for (auto I = F.begin(); I != F.end(); I++) {
    Assert(
        I->getParent() == &F, "Basic Block's parent does not match functiion");
    visitBasicBlock(*I);
  }

  // Verify Dominance Tree is valid
  DominanceInfo D(const_cast<Function *>(&F));
  Assert(
      D.getRoot() == &*F.begin(),
      "Root node in dominance tree should be the entry basic block");
  for (const auto &I : F) {
    if (isVerifyingOptimalIR()) {
      // Check for unreachable blocks.
      Assert(
          D.isReachableFromEntry(&I),
          "Basic Block unreachable from entry in the Dominance Tree");
    }

    // Domination checks within a block are linear, so for huge blocks we get
    // quadratic runtime. To avoid this, we store a set of instructions we've
    // seen so far in the current block.
    llvh::SmallPtrSet<const Instruction *, 32> seen;

    // Instruction dominance check
    for (BasicBlock::const_iterator II = I.begin(); II != I.end(); II++) {
      // Check that incoming phi node values are dominated in their incoming
      // blocks.
      if (auto *Phi = llvh::dyn_cast<PhiInst>(&*II)) {
        for (int i = 0, e = Phi->getNumEntries(); i < e; ++i) {
          auto pair = Phi->getEntry(i);
          BasicBlock *block = pair.second;
          auto *inst = llvh::dyn_cast<Instruction>(pair.first);

          // Non-instructions always dominate everything. Move on.
          if (!inst) {
            continue;
          }

          // Make sure that the incoming value dominates the incoming block.
          Assert(
              D.dominates(inst->getParent(), block),
              "Incoming PHI value must dominate incoming basic block");
        }
        continue;
      }

      if (llvh::isa<UnreachableInst>(&*II)) {
        Assert(
            !D.isReachableFromEntry(II->getParent()),
            "UnreachableInst must not be reachable from entry");
        continue;
      }

      // Check that all instructions are dominated by their operands.
      for (unsigned i = 0; i < II->getNumOperands(); i++) {
        auto Operand = II->getOperand(i);
        if (auto *InstOp = llvh::dyn_cast<Instruction>(Operand)) {
          Assert(
              seen.count(InstOp) || D.properlyDominates(InstOp, &*II),
              "Operand must dominates the Instruction");
        }
      }
      seen.insert(&*II);
    }
  }
}

void Verifier::visitBasicBlock(const BasicBlock &BB) {
  Assert(&BB.getContext() == Ctx, "Basic Block has wrong context");

  Assert(BB.getTerminator(), "Basic block must have a terminator.");

  verifyAttributes(&BB, BB.getParent()->getParent());

  // Verify the mutual predecessor/successor relationship
  for (auto I = succ_begin(&BB), E = succ_end(&BB); I != E; ++I) {
    Assert(
        pred_contains(*I, &BB),
        "Cannot find self in the predecessors of a successor");
  }
  for (auto I = pred_begin(&BB), E = pred_end(&BB); I != E; ++I) {
    Assert(
        succ_contains(*I, &BB),
        "Cannot find self in the successors of a predecessor");
  }

  // Indicates whether all the instructions in the block observed so far had
  // FirstInBlock set.
  bool visitingFirstInBlock = true;
  // Verify each instruction
  for (BasicBlock::const_iterator I = BB.begin(); I != BB.end(); I++) {
    Assert(
        I->getParent() == &BB,
        "Instruction's parent does not match basic block");

    // Check that FirstInBlock instructions are not preceded by other
    // instructions.
    bool firstInBlock = I->getSideEffect().getFirstInBlock();
    visitingFirstInBlock &= firstInBlock;
    Assert(
        visitingFirstInBlock || !firstInBlock,
        "Unexpected FirstInBlock instruction.");

    // Use the instruction using the InstructionVisitor::visit();
    visit(*I);
  }
}

void Verifier::beforeVisitInstruction(const Instruction &Inst) {
  // TODO: Verify the instruction is valid, need switch/case on each
  // actual Instruction type
  Assert(&Inst.getContext() == Ctx, "Instruction has wrong context");

  Assert(Inst.getSideEffect().isWellFormed(), "Ill-formed side effects");

  verifyAttributes(&Inst, Inst.getModule());

  bool const acceptsEmptyType = Inst.acceptsEmptyType();

  // Verify all operands are valid
  for (unsigned i = 0; i < Inst.getNumOperands(); i++) {
    auto Operand = Inst.getOperand(i);
    Assert(Operand != nullptr, "Invalid operand");
    Assert(
        getUsersSetForValue(Operand).count(&Inst) == 1,
        "This instruction is not in the User list of the operand");
    if (llvh::isa<Variable>(Operand)) {
      Assert(
          llvh::isa<LoadFrameInst>(Inst) || llvh::isa<StoreFrameInst>(Inst) ||
              llvh::isa<HBCLoadFromEnvironmentInst>(Inst) ||
              llvh::isa<HBCStoreToEnvironmentInst>(Inst),
          "Variable can only be accessed in "
          "LoadFrame/StoreFrame/HBCLoadFromEnvironmentInst/HBCStoreToEnvironmentInst Inst.");
    }

    // Most instructions that accepts a stack operand must write to it. If it
    // strictly reads from the stack variable, it should go through a
    // LoadStackInst.
    if (llvh::isa<AllocStackInst>(Operand)) {
      Assert(
          llvh::isa<LoadStackInst>(Inst) ||
              Inst.getSideEffect().getWriteStack(),
          "Must write to stack operand.");
    }

    if (Operand->getType().canBeEmpty()) {
      Assert(acceptsEmptyType, "Instruction does not accept empty type");
    }
  }

  // Verify that terminator instructions never need to return a value.
  if (llvh::isa<TerminatorInst>(Inst)) {
    Assert(Inst.getNumUsers() == 0, "Terminator Inst cannot return value.");
  }
}

static bool isTerminator(const Instruction *Inst) {
  return &*Inst->getParent()->rbegin() == Inst;
}

void Verifier::visitReturnInst(const ReturnInst &Inst) {
  Assert(
      isTerminator(&Inst),
      "Return Instruction must be the last instruction of a basic block");
  Assert(Inst.getNumSuccessors() == 0, "ReturnInst should not have successors");
}

void Verifier::visitSaveAndYieldInst(const SaveAndYieldInst &Inst) {
  Assert(isTerminator(&Inst), "SaveAndYield must be a terminator");
  Assert(
      Inst.getParent()->getTerminator() == &Inst,
      "SaveAndYield must be the terminator");
}

void Verifier::visitBranchInst(const BranchInst &Inst) {
  Assert(
      isTerminator(&Inst),
      "Branch Instruction must be the last instruction of a basic block");
  Assert(
      Inst.getNumSuccessors() == 1,
      "Branch Instruction should have only 1 successor");
  Assert(
      succ_contains(Inst.getParent(), Inst.getBranchDest()),
      "Branch Dest Basic Block does not match with successor pointer");
  Assert(
      pred_contains(Inst.getBranchDest(), Inst.getParent()),
      "BranchInst Basic Block not in the predecessor list of target block");
}

void Verifier::visitHBCStoreToEnvironmentInst(
    const HBCStoreToEnvironmentInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCLoadFromEnvironmentInst(
    const HBCLoadFromEnvironmentInst &Inst) {
  Assert(
      Inst.getType() == Inst.getResolvedName()->getType(),
      "HBCLoadFromEnvironment type must be the same as the variable type");
}
void Verifier::visitHBCResolveEnvironment(const HBCResolveEnvironment &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitAsNumberInst(const AsNumberInst &Inst) {
  Assert(
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  Assert(
      Inst.getType() == Type::createNumber(),
      "AsNumberInst must return a number type");
}

void Verifier::visitAsNumericInst(const AsNumericInst &Inst) {
  Assert(
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  Assert(
      Inst.getType().isSubsetOf(Type::createNumeric()),
      "AsNumericInst must return a numeric type");
}

void Verifier::visitAsInt32Inst(const AsInt32Inst &Inst) {
  Assert(
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  Assert(Inst.getType().isInt32Type(), "AsInt32Inst must return an Int32 type");
}

void Verifier::visitAddEmptyStringInst(const AddEmptyStringInst &Inst) {
  Assert(
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  Assert(
      Inst.getType() == Type::createString(),
      "AddEmptyStringInst must return a string type");
}

void Verifier::visitAllocStackInst(const AllocStackInst &Inst) {
  Assert(
      &(Inst.getParent()->back()) != &Inst,
      "Alloca Instruction cannot be the last instruction of a basic block");
}

template <class T>
void Verifier::visitCondBranchLikeInst(const T &Inst) {
  Assert(
      isTerminator(&Inst),
      "CondBranchInst must be the last instruction of a basic block");
  Assert(
      Inst.getNumSuccessors() == 2, "CondBranchInst should have 2 successors");
  Assert(
      succ_contains(Inst.getParent(), Inst.getTrueDest()),
      "True dest should be a successor of CondBranchInst block");
  Assert(
      pred_contains(Inst.getTrueDest(), Inst.getParent()),
      "CondBranchInst block should be a predecessor of true dest");
  Assert(
      succ_contains(Inst.getParent(), Inst.getFalseDest()),
      "False dest should be a successor of CondBranchInst block");
  Assert(
      pred_contains(Inst.getFalseDest(), Inst.getParent()),
      "CondBranchInst block should be a predecessor of false dest");
}

void Verifier::visitCondBranchInst(const CondBranchInst &Inst) {
  visitCondBranchLikeInst(Inst);
}

void Verifier::visitUnaryOperatorInst(const UnaryOperatorInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitTryStartInst(const TryStartInst &Inst) {
  Assert(isTerminator(&Inst), "TryStartInst must be a TermInst");
  Assert(Inst.getNumSuccessors() == 2, "TryStartInst must have 2 successors");
  Assert(
      Inst.getCatchTarget()->front().getKind() == ValueKind::CatchInstKind,
      "Catch Target of TryStartInst must begin with a CatchInst");
}

void Verifier::visitTryEndInst(const TryEndInst &Inst) {
  Assert(
      &Inst == &Inst.getParent()->front(),
      "TryEndInst must be the first instruction of a block");
  Assert(
      pred_count(Inst.getParent()) == 1,
      "TryEndInst must have only one predecessor.");
}

void Verifier::visitStoreStackInst(const StoreStackInst &Inst) {
  Assert(
      !llvh::isa<AllocStackInst>(Inst.getValue()),
      "Storing the address of stack location");
  Assert(!Inst.hasUsers(), "Store Instructions must not have users");
}

void Verifier::visitStoreFrameInst(const StoreFrameInst &Inst) {
  Assert(!Inst.hasUsers(), "Store Instructions must not have users");
}

void Verifier::visitLoadFrameInst(const LoadFrameInst &Inst) {
  Assert(
      Inst.getType() == Inst.getLoadVariable()->getType(),
      "LoadFrameInst type must be the same as the variable type");
}

void Verifier::visitLoadStackInst(const LoadStackInst &Inst) {
  Assert(
      Inst.getType() == Inst.getPtr()->getType(),
      "LoadStackInst type must be the same as the AllocStackInst type");
}

void Verifier::visitBaseCreateCallableInst(const BaseCreateCallableInst &Inst) {
  Assert(
      llvh::isa<NormalFunction>(Inst.getFunctionCode()) ||
          llvh::isa<GeneratorFunction>(Inst.getFunctionCode()) ||
          llvh::isa<AsyncFunction>(Inst.getFunctionCode()),
      "BaseCreateCallableInst only accepts NormalFunction/GeneratorFunction");
}
void Verifier::visitCreateFunctionInst(const CreateFunctionInst &Inst) {
  visitBaseCreateCallableInst(Inst);
}

void Verifier::visitCallInst(const CallInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitHBCCallNInst(const HBCCallNInst &Inst) {
  Assert(
      HBCCallNInst::kMinArgs <= Inst.getNumArguments() &&
          Inst.getNumArguments() <= HBCCallNInst::kMaxArgs,
      "CallNInst has too many args");
}

void Verifier::visitCallBuiltinInst(CallBuiltinInst const &Inst) {
  assert(
      isNativeBuiltin(Inst.getBuiltinIndex()) &&
      "CallBuiltin must take a native builtin.");
}

void Verifier::visitGetBuiltinClosureInst(GetBuiltinClosureInst const &Inst) {
  assert(
      Inst.getBuiltinIndex() < BuiltinMethod::_count &&
      "Out of bound BuiltinMethod index.");
}

void Verifier::visitLoadPropertyInst(const LoadPropertyInst &Inst) {}
void Verifier::visitTryLoadGlobalPropertyInst(
    const TryLoadGlobalPropertyInst &Inst) {}

void Verifier::visitDeletePropertyLooseInst(
    const DeletePropertyLooseInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitDeletePropertyStrictInst(
    const DeletePropertyStrictInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitStorePropertyLooseInst(const StorePropertyLooseInst &Inst) {
}
void Verifier::visitStorePropertyStrictInst(
    const StorePropertyStrictInst &Inst) {}

void Verifier::visitTryStoreGlobalPropertyLooseInst(
    const TryStoreGlobalPropertyLooseInst &Inst) {}
void Verifier::visitTryStoreGlobalPropertyStrictInst(
    const TryStoreGlobalPropertyStrictInst &Inst) {}

void Verifier::visitBaseStoreOwnPropertyInst(
    const BaseStoreOwnPropertyInst &Inst) {
  Assert(
      llvh::isa<LiteralBool>(
          Inst.getOperand(StoreOwnPropertyInst::IsEnumerableIdx)),
      "BaseStoreOwnPropertyInst::IsEnumerable must be a boolean literal");
}
void Verifier::visitStoreOwnPropertyInst(const StoreOwnPropertyInst &Inst) {
  visitBaseStoreOwnPropertyInst(Inst);
}
void Verifier::visitStoreNewOwnPropertyInst(
    const StoreNewOwnPropertyInst &Inst) {
  visitBaseStoreOwnPropertyInst(Inst);
  Assert(
      Inst.getObject()->getType().isObjectType(),
      "StoreNewOwnPropertyInst::Object must be known to be an object");
  if (auto *LN = llvh::dyn_cast<LiteralNumber>(Inst.getProperty())) {
    Assert(
        LN->convertToArrayIndex().hasValue(),
        "StoreNewOwnPropertyInst::Property can only be an index-like number");
  } else {
    Assert(
        llvh::isa<LiteralString>(Inst.getProperty()),
        "StoreNewOwnPropertyInst::Property must be a string or number literal");
  }
}

void Verifier::visitStoreGetterSetterInst(const StoreGetterSetterInst &Inst) {
  Assert(
      llvh::isa<LiteralBool>(
          Inst.getOperand(StoreGetterSetterInst::IsEnumerableIdx)),
      "StoreGetterSetterInsr::IsEnumerable must be a boolean constant");
}

void Verifier::visitAllocObjectInst(const hermes::AllocObjectInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitAllocFastArrayInst(const hermes::AllocFastArrayInst &Inst) {
  LiteralNumber *size = Inst.getCapacity();
  Assert(size->isUInt32Representible(), "Invalid AllocArrayInst size hint");
}

void Verifier::visitAllocArrayInst(const hermes::AllocArrayInst &Inst) {
  LiteralNumber *size = Inst.getSizeHint();
  Assert(size->isUInt32Representible(), "Invalid AllocArrayInst size hint");
  Assert(
      Inst.isLiteralArray(),
      "Array elements must be literal when registers are limited");
}

void Verifier::visitGetTemplateObjectInst(
    const hermes::GetTemplateObjectInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitCreateArgumentsInst(const CreateArgumentsInst &Inst) {
  Assert(functionState, "functionState cannot be null");
  Assert(
      !functionState->createArgumentsEncountered,
      "There should be only one CreateArgumentsInst in a function");
  functionState->createArgumentsEncountered = true;

  BasicBlock *BB = Inst.getParent();
  Function *F = BB->getParent();
  if (llvh::isa<GeneratorInnerFunction>(F)) {
    auto secondBB = F->begin();
    ++secondBB;
    Assert(
        BB == &*secondBB,
        "CreateArgumentsInst must be in the second basic block in generators");
  } else {
    Assert(
        BB == &*F->begin(),
        "CreateArgumentsInst must be in the first basic block");
  }
}

void Verifier::visitCreateArgumentsLooseInst(
    const CreateArgumentsLooseInst &Inst) {
  visitCreateArgumentsInst(Inst);
}
void Verifier::visitCreateArgumentsStrictInst(
    const CreateArgumentsStrictInst &Inst) {
  visitCreateArgumentsInst(Inst);
}

void Verifier::visitCreateRegExpInst(CreateRegExpInst const &Inst) {
  // Nothing to verify at this point.
}

template <class T>
void Verifier::visitBinaryOperatorLikeInst(const T &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitBinaryOperatorInst(const hermes::BinaryOperatorInst &Inst) {
  visitBinaryOperatorLikeInst(Inst);
}

void Verifier::visitCatchInst(const CatchInst &Inst) {
  Assert(
      &Inst.getParent()->front() == &Inst,
      "Catch instruction must be the first in a basic block");
  Assert(
      pred_count(Inst.getParent()) == 1,
      "CatchInst must have only 1 predecessor.");
}

void Verifier::visitThrowInst(const ThrowInst &Inst) {
  Assert(isTerminator(&Inst), "ThrowInst must be a terminator");
  Assert(Inst.getNumSuccessors() == 0, "ThrowInst should not have successors");
}

void Verifier::visitThrowTypeErrorInst(const ThrowTypeErrorInst &Inst) {
  Assert(isTerminator(&Inst), "ThrowTypeErrorInst must be a terminator");
  Assert(
      Inst.getNumSuccessors() == 0,
      "ThrowTypeErrorInst should not have successors");
}

void Verifier::visitGetNextPNameInst(const GetNextPNameInst &Inst) {
  Assert(isTerminator(&Inst), "GetNextPNameInst must terminate the block");
  Assert(
      Inst.getNumSuccessors() == 2,
      "GetNextPNameInst should have 2 successors");
  Assert(
      succ_contains(Inst.getParent(), Inst.getOnSomeDest()),
      "OnSome dest should be a successor of GetNextPNameInst block");
  Assert(
      pred_contains(Inst.getOnSomeDest(), Inst.getParent()),
      "GetNextPNameInst block should be a predecessor of OnSome dest");
  Assert(
      succ_contains(Inst.getParent(), Inst.getOnLastDest()),
      "OnLast dest should be a successor of GetNextPNameInst block");
  Assert(
      pred_contains(Inst.getOnLastDest(), Inst.getParent()),
      "GetNextPNameInst block should be a predecessor of OnLast dest");
}

void Verifier::visitGetPNamesInst(const GetPNamesInst &Inst) {
  Assert(isTerminator(&Inst), "GetPNamesInst must terminate the block");
  Assert(
      Inst.getNumSuccessors() == 2, "GetPNamesInst should have 2 successors");
  Assert(
      succ_contains(Inst.getParent(), Inst.getOnSomeDest()),
      "OnSome dest should be a successor of GetPNamesInst block");
  Assert(
      pred_contains(Inst.getOnSomeDest(), Inst.getParent()),
      "GetPNamesInst block should be a predecessor of OnSome dest");
  Assert(
      succ_contains(Inst.getParent(), Inst.getOnEmptyDest()),
      "OnEmpty dest should be a successor of GetPNamesInst block");
  Assert(
      pred_contains(Inst.getOnEmptyDest(), Inst.getParent()),
      "GetPNamesInst block should be a predecessor of OnEmpty dest");
}

void Verifier::visitMovInst(const hermes::MovInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitImplicitMovInst(const hermes::ImplicitMovInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitCoerceThisNSInst(CoerceThisNSInst const &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitPhiInst(const hermes::PhiInst &Inst) {
  // We verify the dominance property when we scan the whole function. In here
  // we only verify local properties.

  llvh::SmallDenseSet<BasicBlock *, 8> entries;

  // Check that every input block enters only once:
  for (int i = 0, e = Inst.getNumEntries(); i < e; ++i) {
    auto pair = Inst.getEntry(i);
    BasicBlock *block = pair.second;
    Assert(
        pred_contains(Inst.getParent(), block),
        "Predecessor not covered by phi node!");
    Assert(
        succ_contains(block, Inst.getParent()),
        "Phi node should be Successor!");

    // In theory, it is legitimate for a Phi to have multiple entries for a
    // block as long as all the associated values are the same. However, such an
    // invariant would be more complicated, and requires care to maintain when
    // updating operands of the Phi.
    auto [it, first] = entries.insert(block);
    Assert(first, "Phi node has multiple entries for the same block");
  }

  Assert(
      entries.size() == pred_count_unique(Inst.getParent()),
      "number of predecessors does not match phi inputs");
}

template <class T>
void Verifier::visitSwitchLikeInst(const T &Inst) {
  Assert(isTerminator(&Inst), "SwitchInst must be a terminator");

  Assert(
      Inst.getNumCasePair() > 0, "SwitchInst must have some case destinations");

  Assert(Inst.getDefaultDestination(), "Invalid destination block");

  Assert(Inst.getInputValue(), "Invalid input value");

  Assert(
      Inst.getNumSuccessors() == Inst.getNumCasePair() + 1,
      "Number of successors of SwitchInst does not match.");
  Assert(
      succ_contains(Inst.getParent(), Inst.getDefaultDestination()),
      "Default destination must be a successor of SwitchInst block");
  Assert(
      pred_contains(Inst.getDefaultDestination(), Inst.getParent()),
      "SwitchInst block must be a predecessor of default destination.");
  for (unsigned idx = 0, e = Inst.getNumCasePair(); idx < e; ++idx) {
    Assert(
        succ_contains(Inst.getParent(), Inst.getCasePair(idx).second),
        "Case target must be a successor of SwitchInst block");
    Assert(
        pred_contains(Inst.getCasePair(idx).second, Inst.getParent()),
        "SwitchInst block must be a predecessor of case target");
  }

  // Verify that each case is unique.
  llvh::SmallPtrSet<Literal *, 8> values;
  for (unsigned idx = 0, e = Inst.getNumCasePair(); idx < e; ++idx) {
    Assert(
        values.insert(Inst.getCasePair(idx).first).second,
        "switch values must be unique");
  }
}

void Verifier::visitSwitchInst(const hermes::SwitchInst &Inst) {
  visitSwitchLikeInst(Inst);
}

void Verifier::visitSwitchImmInst(const hermes::SwitchImmInst &Inst) {
  visitSwitchLikeInst(Inst);
  for (unsigned idx = 0, e = Inst.getNumCasePair(); idx < e; ++idx) {
    Assert(
        Inst.getCasePair(idx).first->isInt32Representible(),
        "case value must be a int32");
  }
}

void Verifier::visitDebuggerInst(DebuggerInst const &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitDirectEvalInst(DirectEvalInst const &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitDeclareGlobalVarInst(const DeclareGlobalVarInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitHBCCreateEnvironmentInst(
    const HBCCreateEnvironmentInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitHBCProfilePointInst(const HBCProfilePointInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitHBCAllocObjectFromBufferInst(
    const hermes::HBCAllocObjectFromBufferInst &Inst) {
  LiteralNumber *size = Inst.getSizeHint();
  Assert(
      size->isUInt32Representible(),
      "Invalid HBCAllocObjectFromBufferInst size hint");
  Assert(
      Inst.getKeyValuePairCount() > 0,
      "Cannot allocate an empty HBCAllocObjectFromBufferInst");
}

void Verifier::visitAllocObjectLiteralInst(
    const hermes::AllocObjectLiteralInst &Inst) {
  Assert(
      Inst.getKeyValuePairCount() > 0,
      "Cannot allocate an empty AllocObjectLiteralInst");
}

void Verifier::visitHBCGetGlobalObjectInst(const HBCGetGlobalObjectInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitHBCLoadConstInst(hermes::HBCLoadConstInst const &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitLoadParamInst(hermes::LoadParamInst const &Inst) {}
void Verifier::visitCompareBranchInst(const CompareBranchInst &Inst) {
  visitCondBranchLikeInst(Inst);
  visitBinaryOperatorLikeInst(Inst);
}

void Verifier::visitCreateGeneratorInst(const CreateGeneratorInst &Inst) {
  Assert(
      llvh::isa<GeneratorInnerFunction>(Inst.getFunctionCode()),
      "CreateGeneratorInst must take a GeneratorInnerFunction");
}
void Verifier::visitStartGeneratorInst(const StartGeneratorInst &Inst) {
  Assert(
      &Inst == &Inst.getParent()->front() &&
          Inst.getParent() == &Inst.getParent()->getParent()->front(),
      "StartGeneratorInst must be the first instruction of a function");
}
void Verifier::visitResumeGeneratorInst(const ResumeGeneratorInst &Inst) {}

void Verifier::visitHBCCreateGeneratorInst(const HBCCreateGeneratorInst &Inst) {
  Assert(
      llvh::isa<GeneratorInnerFunction>(Inst.getFunctionCode()),
      "HBCCreateGeneratorInst must take a GeneratorInnerFunction");
}

void Verifier::visitLIRGetThisNSInst(const LIRGetThisNSInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCGetArgumentsPropByValLooseInst(
    const HBCGetArgumentsPropByValLooseInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCGetArgumentsPropByValStrictInst(
    const HBCGetArgumentsPropByValStrictInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCGetArgumentsLengthInst(
    const HBCGetArgumentsLengthInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCReifyArgumentsLooseInst(
    const HBCReifyArgumentsLooseInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCReifyArgumentsStrictInst(
    const HBCReifyArgumentsStrictInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitCreateThisInst(const CreateThisInst &Inst) {}
void Verifier::visitGetConstructedObjectInst(
    const GetConstructedObjectInst &Inst) {}

void Verifier::visitHBCCreateFunctionInst(const HBCCreateFunctionInst &Inst) {
  visitBaseCreateCallableInst(Inst);
}
void Verifier::visitHBCSpillMovInst(const HBCSpillMovInst &Inst) {}
void Verifier::visitUnreachableInst(const UnreachableInst &Inst) {}

void Verifier::visitIteratorBeginInst(const IteratorBeginInst &Inst) {
  Assert(
      llvh::isa<AllocStackInst>(Inst.getSourceOrNext()),
      "SourceOrNext must be an AllocStackInst");
}
void Verifier::visitIteratorNextInst(const IteratorNextInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitIteratorCloseInst(const IteratorCloseInst &Inst) {
  Assert(
      llvh::isa<LiteralBool>(
          Inst.getOperand(IteratorCloseInst::IgnoreInnerExceptionIdx)),
      "IgnoreInnerException must be a LiteralBool in IteratorCloseInst");
}

void Verifier::visitGetNewTargetInst(GetNewTargetInst const &Inst) {
  auto definitionKind = Inst.getParent()->getParent()->getDefinitionKind();
  Assert(
      definitionKind == Function::DefinitionKind::ES5Function ||
          definitionKind == Function::DefinitionKind::ES6Constructor,
      "GetNewTargetInst can only be used in ES6 constructors and ES5 functions");
  Assert(
      Inst.getParent()->getParent()->getNewTargetParam() ==
          Inst.getOperand(GetNewTargetInst::GetNewTargetParamIdx),
      "GetNewTargetInst must use correct getNewTargetParam");
}

void Verifier::visitThrowIfEmptyInst(const ThrowIfEmptyInst &Inst) {}

void Verifier::visitPrLoadInst(const PrLoadInst &Inst) {}
void Verifier::visitPrStoreInst(const PrStoreInst &Inst) {}

void Verifier::visitFastArrayLoadInst(const FastArrayLoadInst &Inst) {}
void Verifier::visitFastArrayStoreInst(const FastArrayStoreInst &Inst) {}
void Verifier::visitFastArrayPushInst(const FastArrayPushInst &Inst) {}
void Verifier::visitFastArrayAppendInst(const FastArrayAppendInst &Inst) {}
void Verifier::visitFastArrayLengthInst(const FastArrayLengthInst &Inst) {}

void Verifier::visitLoadParentInst(const LoadParentInst &Inst) {}
void Verifier::visitStoreParentInst(const StoreParentInst &Inst) {}

void Verifier::visitFUnaryMathInst(const FUnaryMathInst &Inst) {
  Assert(
      Inst.getArg()->getType().isNumberType() && Inst.getType().isNumberType(),
      "FUnaryMathInst wrong type");
}
void Verifier::visitFBinaryMathInst(const FBinaryMathInst &Inst) {
  Assert(
      Inst.getLeft()->getType().isNumberType() &&
          Inst.getRight()->getType().isNumberType() &&
          Inst.getType().isNumberType(),
      "FBinaryMathInst wrong type");
}
void Verifier::visitFCompareInst(const FCompareInst &Inst) {
  Assert(
      Inst.getLeft()->getType().isNumberType() &&
          Inst.getRight()->getType().isNumberType() &&
          Inst.getType().isBooleanType(),
      "FCompare wrong type");
}

void Verifier::visitUnionNarrowTrustedInst(const UnionNarrowTrustedInst &Inst) {
}
void Verifier::visitCheckedTypeCastInst(const CheckedTypeCastInst &Inst) {}
void Verifier::visitLIRDeadValueInst(const LIRDeadValueInst &Inst) {}

void Verifier::visitNativeCallInst(const hermes::NativeCallInst &Inst) {
  // FIXM: this should be added when types are propagated.
  //  Assert(
  //      Inst.getCallee()->getType().isNumberType(),
  //      "NativeCallInst callee must be a number");
}

} // namespace

#endif

bool hermes::verifyModule(
    const Module &M,
    raw_ostream *OS,
    VerificationMode mode) {
#ifdef HERMES_SLOW_DEBUG
  raw_null_ostream NullStr;
  NullStr.SetUnbuffered();
  Verifier V(OS ? *OS : NullStr, mode);
  return V.verify(M);
#else
  return true;
#endif
}
