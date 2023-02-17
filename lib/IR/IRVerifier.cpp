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
    ScopeCreationInst *functionBodyScopeDescCreator = nullptr;

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
  void visitScope(const ScopeDesc &S);
  void visitFunction(const Function &F);
  void visitBasicBlock(const BasicBlock &BB);

#define DEF_VALUE(XX, PARENT) void visit##XX(const XX &Inst);
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

void Verifier::visitScope(const ScopeDesc &S) {
  Assert(S.getParent(), "All scopes should have a parent");

  const auto &parentsChildren = S.getParent()->getInnerScopes();
  Assert(
      std::find(parentsChildren.begin(), parentsChildren.end(), &S) !=
          parentsChildren.end(),
      "Scope is not in parent's inner scope list");

  Assert(S.hasFunction(), "Scope is not bound to a function");
  for (ScopeDesc *i : S.getInnerScopes()) {
    visitScope(*i);
  }
}

void Verifier::visitModule(const Module &M) {
  for (ScopeDesc *S : M.getInitialScope()->getInnerScopes()) {
    visitScope(*S);
  }

  // Verify all functions are valid
  for (Module::const_iterator I = M.begin(); I != M.end(); I++) {
    Assert(I->getParent() == &M, "Function's parent does not match module");
    visitFunction(*I);
  }
}

void Verifier::visitFunction(const Function &F) {
  Assert(&F.getContext() == Ctx, "Function has wrong context");

  if (F.isLazy())
    return;

  FunctionState newFunctionState(this, F);

  // Verify all basic blocks are valid
  for (auto I = F.begin(); I != F.end(); I++) {
    Assert(
        I->getParent() == &F, "Basic Block's parent does not match functiion");
    visitBasicBlock(*I);
  }

  // Verify all parameters are valid
  for (auto I = F.arg_begin(); I != F.arg_end(); I++) {
    Assert(
        (*I)->getParent() == &F, "Parameter's parent does not match function");
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

  // Verify each instruction
  for (BasicBlock::const_iterator I = BB.begin(); I != BB.end(); I++) {
    Assert(
        I->getParent() == &BB,
        "Instruction's parent does not match basic block");
    // Use the instruction using the InstructionVisitor::visit();
    visit(*I);
  }
}

void Verifier::beforeVisitInstruction(const Instruction &Inst) {
  // TODO: Verify the instruction is valid, need switch/case on each
  // actual Instruction type
  Assert(&Inst.getContext() == Ctx, "Instruction has wrong context");

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
    if (llvh::isa<AllocStackInst>(Operand)) {
      Assert(
          llvh::isa<LoadStackInst>(Inst) || llvh::isa<StoreStackInst>(Inst) ||
              llvh::isa<CatchInst>(Inst) || llvh::isa<GetPNamesInst>(Inst) ||
              llvh::isa<CheckHasInstanceInst>(Inst) ||
              llvh::isa<GetNextPNameInst>(Inst) ||
              llvh::isa<ResumeGeneratorInst>(Inst) ||
              llvh::isa<IteratorBeginInst>(Inst) ||
              llvh::isa<IteratorNextInst>(Inst) ||
              llvh::isa<IteratorCloseInst>(Inst) ||
              llvh::isa<HBCGetArgumentsPropByValInst>(Inst) ||
              llvh::isa<HBCGetArgumentsLengthInst>(Inst) ||
              llvh::isa<HBCReifyArgumentsInst>(Inst),
          "Stack variable can only be accessed in certain instructions.");
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

void Verifier::visitScopeCreationInst(const ScopeCreationInst &Inst) {}

void Verifier::visitSingleOperandInst(const SingleOperandInst &Inst) {}

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
  // Nothing to verify at this point.
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
      Inst.getType() == Type::createNumeric(),
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
  // Nothing to verify at this point.
}

void Verifier::visitLoadStackInst(const LoadStackInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitCreateScopeInst(const CreateScopeInst &Inst) {
  Assert(functionState, "function state cannot be null");
  Assert(
      !functionState->functionBodyScopeDescCreator,
      "multiple functions materializing function's body scope desc");
  functionState->functionBodyScopeDescCreator =
      const_cast<CreateScopeInst *>(&Inst);
  Assert(
      functionState->function.getFunctionScopeDesc() ==
          Inst.getCreatedScopeDesc(),
      "CreateScopeInst is materializing the wrong scope desc");
}

void Verifier::visitCreateFunctionInst(const CreateFunctionInst &Inst) {
  // Nothing to verify at this point.
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
  visitCallInst(Inst);
}

void Verifier::visitGetBuiltinClosureInst(GetBuiltinClosureInst const &Inst) {
  assert(
      Inst.getBuiltinIndex() < BuiltinMethod::_count &&
      "Out of bound BuiltinMethod index.");
}

#ifdef HERMES_RUN_WASM
void Verifier::visitCallIntrinsicInst(CallIntrinsicInst const &Inst) {
  assert(
      Inst.getIntrinsicsIndex() < WasmIntrinsics::_count &&
      "Out of bound Unsafe Compiler Intrinsics Index.");
}
#endif

void Verifier::visitHBCCallDirectInst(HBCCallDirectInst const &Inst) {
  Assert(
      llvh::isa<Function>(Inst.getCallee()),
      "HBCCallDirect callee must be a Function");
  Assert(
      Inst.getNumArguments() <= HBCCallDirectInst::MAX_ARGUMENTS,
      "CallBuiltin too many arguments");
  visitCallInst(Inst);
}

void Verifier::visitLoadPropertyInst(const LoadPropertyInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitTryLoadGlobalPropertyInst(
    const TryLoadGlobalPropertyInst &Inst) {
  visitLoadPropertyInst(Inst);
}

void Verifier::visitDeletePropertyInst(const DeletePropertyInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitStorePropertyInst(const StorePropertyInst &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitTryStoreGlobalPropertyInst(
    const TryStoreGlobalPropertyInst &Inst) {
  visitStorePropertyInst(Inst);
}

void Verifier::visitStoreOwnPropertyInst(const StoreOwnPropertyInst &Inst) {
  Assert(
      llvh::isa<LiteralBool>(
          Inst.getOperand(StoreOwnPropertyInst::IsEnumerableIdx)),
      "StoreOwnPropertyInst::IsEnumerable must be a boolean literal");
}
void Verifier::visitStoreNewOwnPropertyInst(
    const StoreNewOwnPropertyInst &Inst) {
  visitStoreOwnPropertyInst(Inst);
  Assert(
      llvh::isa<LiteralString>(
          Inst.getOperand(StoreOwnPropertyInst::PropertyIdx)),
      "StoreNewOwnPropertyInst::Property must be a string literal");
  Assert(
      Inst.getObject()->getType().isObjectType(),
      "StoreNewOwnPropertyInst::Object must be known to be an object");
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

void Verifier::visitAllocArrayInst(const hermes::AllocArrayInst &Inst) {
  LiteralNumber *size = Inst.getSizeHint();
  Assert(size->isUInt32Representible(), "Invalid AllocArrayInst size hint");
  if (!Ctx->getCodeGenerationSettings().unlimitedRegisters) {
    Assert(
        Inst.isLiteralArray(),
        "Array elements must be literal when registers are limited");
  }
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
  Assert(
      Inst.getNumSuccessors() == 0,
      "visitThrowInst should not have successors");
}

void Verifier::visitConstructInst(const ConstructInst &Inst) {
  // Nothing to verify at this point.
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

  llvh::DenseMap<BasicBlock *, Value *> entries(8);

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

    Value *value = pair.first;
    auto result = entries.find(block);
    if (result == entries.end()) {
      entries[block] = value;
    } else {
      Assert(
          value == result->second,
          "Phi node has different inputs for the same block.");
    }
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

void Verifier::visitCheckHasInstanceInst(const CheckHasInstanceInst &Inst) {
  Assert(isTerminator(&Inst), "CheckHasInstanceInst must be a terminator");
  Assert(
      Inst.getNumSuccessors() == 2,
      "CheckHasInstanceInst should have 2 successors");
}

void Verifier::visitDebuggerInst(DebuggerInst const &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitTerminatorInst(const TerminatorInst &Inst) {
  Assert(false, "TerminatorInst is a virtual class.");
}

void Verifier::visitDirectEvalInst(DirectEvalInst const &Inst) {
  // Nothing to verify at this point.
}

void Verifier::visitHBCCreateEnvironmentInst(
    const HBCCreateEnvironmentInst &Inst) {
  Assert(functionState, "function state cannot be null");
  Assert(
      !functionState->functionBodyScopeDescCreator,
      "multiple functions materializing function's body scope desc");
  functionState->functionBodyScopeDescCreator =
      const_cast<HBCCreateEnvironmentInst *>(&Inst);
  Assert(
      functionState->function.getFunctionScopeDesc() ==
          Inst.getCreatedScopeDesc(),
      "HBCCreateEnvironmentInst is materializing the wrong scope desc");
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

void Verifier::visitHBCLoadParamInst(hermes::HBCLoadParamInst const &Inst) {
  Assert(
      Inst.getIndex()->isUInt32Representible(),
      "HBCLoadParamInst's LiteralNumber is not a uint32.");
}
void Verifier::visitCompareBranchInst(const CompareBranchInst &Inst) {
  visitCondBranchLikeInst(Inst);
  visitBinaryOperatorLikeInst(Inst);
}

void Verifier::visitCreateGeneratorInst(const CreateGeneratorInst &Inst) {}
void Verifier::visitStartGeneratorInst(const StartGeneratorInst &Inst) {
  Assert(
      Inst.getParent() == &Inst.getParent()->getParent()->front(),
      "StartGeneratorInst must be in the function's first basic block.");

  BasicBlock::iterator it = Inst.getParent()->begin();

  if (&Inst == &*it) {
    // First instruction in the basic block, OK.
    return;
  }

  if (llvh::isa<CreateScopeInst>(&*it)) {
    ++it;
    if (&Inst == &*it) {
      return;
    }
  }

  Assert(
      false,
      "StartGeneratorInst must be the first instruction of a function, "
      "or the second instructions following a CreateScopeInst");
}
void Verifier::visitResumeGeneratorInst(const ResumeGeneratorInst &Inst) {}

void Verifier::visitHBCCreateGeneratorInst(const HBCCreateGeneratorInst &Inst) {
  visitCreateGeneratorInst(Inst);
}

void Verifier::visitHBCGetThisNSInst(const HBCGetThisNSInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCGetArgumentsPropByValInst(
    const HBCGetArgumentsPropByValInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCGetArgumentsLengthInst(
    const HBCGetArgumentsLengthInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCReifyArgumentsInst(const HBCReifyArgumentsInst &Inst) {
  // Nothing to verify at this point.
}
void Verifier::visitHBCConstructInst(const HBCConstructInst &Inst) {}
void Verifier::visitHBCCreateThisInst(const HBCCreateThisInst &Inst) {}
void Verifier::visitHBCGetConstructedObjectInst(
    const HBCGetConstructedObjectInst &Inst) {}

void Verifier::visitHBCCreateFunctionInst(const HBCCreateFunctionInst &Inst) {
  visitCreateFunctionInst(Inst);
}
void Verifier::visitHBCSpillMovInst(const HBCSpillMovInst &Inst) {}
void Verifier::visitUnreachableInst(const UnreachableInst &Inst) {}

void Verifier::visitIteratorBeginInst(const IteratorBeginInst &Inst) {
  Assert(
      llvh::isa<AllocStackInst>(Inst.getSourceOrNext()),
      "SourceOrNext must be an AllocStackInst");
}
void Verifier::visitIteratorNextInst(const IteratorNextInst &Inst) {
  Assert(
      llvh::isa<AllocStackInst>(Inst.getSourceOrNext()),
      "SourceOrNext must be an AllocStackInst");
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
}

void Verifier::visitThrowIfEmptyInst(const ThrowIfEmptyInst &Inst) {}

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
  return !V.verify(M);
#else
  return false;
#endif
}
