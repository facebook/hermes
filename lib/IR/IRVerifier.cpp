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

#include "hermes/Utils/Dumper.h"
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
class Verifier : public InstructionVisitor<Verifier, bool> {
  class FunctionState; // Forward declaration of inner class.

  const Module &M;
  Context *Ctx{nullptr};
  raw_ostream &OS;
  VerificationMode verificationMode;
  irdumper::IRPrinter &printer;
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
  explicit Verifier(
      const Module &M,
      raw_ostream &OS,
      VerificationMode mode,
      irdumper::IRPrinter &printer)
      : M(M), OS(OS), verificationMode(mode), printer(printer) {}

  bool verify() {
    Ctx = &M.getContext();
    return visitModule(M);
  };

  /// Reimplement in Verifier class for custom behavior.
  LLVM_NODISCARD bool verifyBeforeVisitInstruction(const Instruction &I);

  LLVM_NODISCARD bool visitModule(const Module &M);
  LLVM_NODISCARD bool visitFunction(const Function &F);
  LLVM_NODISCARD bool visitBasicBlock(const BasicBlock &BB);

  LLVM_NODISCARD bool visitBaseStoreOwnPropertyInst(
      const BaseStoreOwnPropertyInst &Inst);
  LLVM_NODISCARD bool visitBaseCreateLexicalChildInst(
      const BaseCreateLexicalChildInst &Inst);
  LLVM_NODISCARD bool visitBaseCreateCallableInst(
      const BaseCreateCallableInst &Inst);

  LLVM_NODISCARD bool visitCreateArgumentsInst(const CreateArgumentsInst &Inst);

#define DEF_VALUE(XX, PARENT) bool visit##XX(const XX &Inst);
#define BEGIN_VALUE(XX, PARENT) DEF_VALUE(XX, PARENT)
#include "hermes/IR/Instrs.def"

 private:
  /// Implement verification for a switch-like instruction.
  template <class T>
  LLVM_NODISCARD bool visitSwitchLikeInst(const T &Inst);
  /// Implement verification for a conditional branch-like instruction.
  template <class T>
  LLVM_NODISCARD bool visitCondBranchLikeInst(const T &Inst);
  /// Implement verification for a binaryoperator-like instruction.
  template <class T>
  LLVM_NODISCARD bool visitBinaryOperatorLikeInst(const T &Inst);

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
  LLVM_NODISCARD bool verifyAttributes(const Value *val);

  /// A helper function for the Assert macros that prints the location of the
  /// the problematic instruction, if available. It also prints the
  /// instruction/BB label.
  /// \param inst the optional instruction to print the
  /// location of.
  void _assertPrintLocation(const Instruction *inst);

  /// Verify that TryStart/EndInsts are balanced. Also, verify all Try bodies
  /// are reachable only through the enclosing TryStartInst.
  LLVM_NODISCARD bool verifyTryStructure(const Function &F);

  /// Helper function to get the label of a BasicBlock.
  llvh::format_object<unsigned> bbLabel(const BasicBlock &block) {
    return llvh::format("%%BB%u", printer.namer_.getBBNumber(&block));
  }

  /// Helper function to get the label of an Instruction.
  llvh::format_object<unsigned> iLabel(const Instruction &I) {
    return llvh::format("%%%u", printer.namer_.getInstNumber(&I));
  }
};

void Verifier::_assertPrintLocation(const Instruction *inst) {
  const Function *f = nullptr;
  if (inst)
    f = inst->getParent()->getParent();
  else if (functionState)
    f = &functionState->function;

  if (f) {
    OS << " in function " << f->getInternalNameStr();
    if (inst) {
      SourceErrorManager::SourceCoords coords{};
      if (Ctx->getSourceErrorManager().findBufferLineAndLoc(
              inst->getLocation(), coords)) {
        OS << ", ";
        Ctx->getSourceErrorManager().dumpCoords(OS, coords);
      }
    }
  }
  if (inst) {
    OS << " in " << bbLabel(*inst->getParent()) << ", " << iLabel(*inst);
    ;
  }
  OS << '\n';
}

#define ReturnIfNot(C) \
  do {                 \
    if (!(C)) {        \
      return false;    \
    }                  \
  } while (0)

/// TODO: Need to make this accept format strings
/// Check if \p C is false. If so, dump the module, forward the given
/// arguments to OS, print function information and return false in the
/// enclosing scope.
#define AssertWithMsg(C, ...)        \
  do {                               \
    if (!(C)) {                      \
      printer.visit(M);              \
      OS << __VA_ARGS__;             \
      _assertPrintLocation(nullptr); \
      return false;                  \
    }                                \
  } while (0)

/// This macro is the same as AssertWithMsg, but it will also append the
/// location information of the given instruction \p inst.
#define AssertIWithMsg(inst, C, ...)                  \
  do {                                                \
    if (!(C)) {                                       \
      printer.visit(M);                               \
      OS << inst.getKindStr() << ": " << __VA_ARGS__; \
      _assertPrintLocation(&inst);                    \
      return false;                                   \
    }                                                 \
  } while (0)

bool Verifier::verifyAttributes(const Value *val) {
  const auto &attrs = val->getAttributes(&M);
#define ATTRIBUTE(valueKind, name, _string) \
  if (attrs.name)                           \
    AssertWithMsg(                          \
        llvh::isa<valueKind>(val), #name " must be set on a " #valueKind);
#include "hermes/IR/Attributes.def"
  return true;
}

bool Verifier::visitModule(const Module &M) {
  // Verify all functions are valid
  for (Module::const_iterator I = M.begin(); I != M.end(); I++) {
    AssertWithMsg(
        I->getParent() == &M, "Function's parent does not match module");
    ReturnIfNot(visitFunction(*I));
  }
  return true;
}

bool Verifier::visitFunction(const Function &F) {
  AssertWithMsg(&F.getContext() == Ctx, "Function has wrong context");

  AssertWithMsg(verifyAttributes(&F), "Invalid function attributes");

  for (Value *newTargetUser : F.getNewTargetParam()->getUsers()) {
    AssertWithMsg(
        llvh::isa<GetNewTargetInst>(newTargetUser),
        "Only GetNewTargetInst may use the newTargetParam");
  }

  FunctionState newFunctionState(this, F);

  // Verify all basic blocks are valid
  for (auto I = F.begin(); I != F.end(); I++) {
    AssertWithMsg(
        I->getParent() == &F,
        bbLabel(*I) << "'s parent does not match function");
    ReturnIfNot(visitBasicBlock(*I));
  }

  // Verify Dominance Tree is valid
  DominanceInfo D(const_cast<Function *>(&F));
  AssertWithMsg(
      D.getRoot() == &*F.begin(),
      "Root node in dominance tree should be the entry basic block");
  AssertWithMsg(
      pred_count(&*F.begin()) == 0,
      "The entry block should have no predecessors");
  for (const auto &I : F) {
    // Check for unreachable blocks.
    AssertWithMsg(
        D.isReachableFromEntry(&I),
        bbLabel(I) << " unreachable from entry in the Dominance Tree");

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
          AssertWithMsg(
              D.dominates(inst->getParent(), block),
              "Incoming PHI value " << bbLabel(*inst->getParent())
                                    << " must dominate incoming "
                                    << bbLabel(*block));
        }
        continue;
      }

      // Check that all instructions are dominated by their operands.
      for (unsigned i = 0; i < II->getNumOperands(); i++) {
        auto Operand = II->getOperand(i);
        if (auto *InstOp = llvh::dyn_cast<Instruction>(Operand)) {
          AssertWithMsg(
              seen.count(InstOp) || D.properlyDominates(InstOp, &*II),
              "Operand " << iLabel(*InstOp) << " must dominate the Instruction "
                         << iLabel(*II));
        }
      }
      seen.insert(&*II);
    }
  }
  return verifyTryStructure(F);
}

bool Verifier::verifyTryStructure(const Function &F) {
  // Stack of basic blocks to visit, and the TryStartInst on entry of that
  // block.
  llvh::SmallVector<std::pair<const BasicBlock *, const TryStartInst *>, 4>
      stack;
  // Holds a mapping from basic block -> innermost enclosing TryStartInst,
  // accounting for Catch/TryEnd.
  llvh::DenseMap<const BasicBlock *, const TryStartInst *> blockToEnclosingTry;

  stack.push_back({&*F.begin(), nullptr});
  blockToEnclosingTry[&*F.begin()] = nullptr;
  while (!stack.empty()) {
    auto [BB, enclosingTry] = stack.pop_back_val();
    if (llvh::isa<ReturnInst>(BB->getTerminator())) {
      AssertWithMsg(
          enclosingTry == nullptr,
          "ReturnInst " << iLabel(*BB->getTerminator()) << " but Try "
                        << iLabel(*enclosingTry) << " has not been closed.");
    } else if (auto *TSI = llvh::dyn_cast<TryStartInst>(BB->getTerminator())) {
      enclosingTry = TSI;
    }

    for (auto *succ : successors(BB)) {
      // If succ starts with a TryEndInst or CatchInst, pop off to the
      // nearest try.
      auto *succEnclosingTry = enclosingTry;
      if (llvh::isa<TryEndInst>(&succ->front()) ||
          llvh::isa<CatchInst>(&succ->front())) {
        AssertIWithMsg(
            succ->front(),
            enclosingTry != nullptr,
            "ending a try outside of any TryStartInst context.");
        assert(
            blockToEnclosingTry.find(enclosingTry->getParent()) !=
                blockToEnclosingTry.end() &&
            "enclosingTry should already be in map.");
        succEnclosingTry = blockToEnclosingTry[enclosingTry->getParent()];
      }

      // If BB already exists in blockToEnclosingTry and has a different
      // enclosing TryStartInst, error out.
      auto [enclosingInfo, success] =
          blockToEnclosingTry.insert({succ, succEnclosingTry});
      if (!success) {
        AssertWithMsg(
            enclosingInfo->second == succEnclosingTry,
            bbLabel(*succ)
                << " is reachable from multiple different TryStartInsts: "
                << iLabel(*succEnclosingTry) << " and "
                << iLabel(*enclosingInfo->second));
      } else {
        // Only expore this BB if we haven't visited it before.
        stack.push_back({succ, succEnclosingTry});
      }
    }
  }
  return true;
}

bool Verifier::visitBasicBlock(const BasicBlock &BB) {
  AssertWithMsg(&BB.getContext() == Ctx, bbLabel(BB) << "has wrong context");

  AssertWithMsg(BB.getTerminator(), bbLabel(BB) << " must have a terminator.");

  ReturnIfNot(verifyAttributes(&BB));

  // Verify the mutual predecessor/successor relationship
  for (auto I = succ_begin(&BB), E = succ_end(&BB); I != E; ++I) {
    AssertWithMsg(
        pred_contains(*I, &BB),
        "Cannot find self " << bbLabel(BB)
                            << "in the predecessors of a successor "
                            << bbLabel(**I));
  }
  for (auto I = pred_begin(&BB), E = pred_end(&BB); I != E; ++I) {
    AssertWithMsg(
        succ_contains(*I, &BB),
        "Cannot find self " << bbLabel(BB)
                            << " in the successors of a predecessor "
                            << bbLabel(**I));
  }

  // Indicates whether all the instructions in the block observed so far had
  // FirstInBlock set.
  bool visitingFirstInBlock = true;
  // Verify each instruction
  for (BasicBlock::const_iterator I = BB.begin(); I != BB.end(); I++) {
    AssertWithMsg(
        I->getParent() == &BB,
        "Instruction " << iLabel(*I) << "'s parent " << bbLabel(*I->getParent())
                       << " does not match " << bbLabel(BB));

    // Check that FirstInBlock instructions are not preceded by other
    // instructions.
    bool firstInBlock = I->getSideEffect().getFirstInBlock();
    visitingFirstInBlock &= firstInBlock;
    AssertIWithMsg(
        (*I),
        visitingFirstInBlock || !firstInBlock,
        "Unexpected FirstInBlock instruction.");

    // Use the instruction using the InstructionVisitor::visit();
    ReturnIfNot(verifyBeforeVisitInstruction(*I));
    ReturnIfNot(visit(*I));
  }
  return true;
}

bool Verifier::verifyBeforeVisitInstruction(const Instruction &Inst) {
  // TODO: Verify the instruction is valid, need switch/case on each
  // actual Instruction type
  AssertIWithMsg(Inst, &Inst.getContext() == Ctx, "Wrong context");

  AssertIWithMsg(
      Inst, Inst.getSideEffect().isWellFormed(), "Ill-formed side effects");

  ReturnIfNot(verifyAttributes(&Inst));

  bool const acceptsEmptyType = Inst.acceptsEmptyType();

  // Verify all operands are valid
  for (unsigned i = 0; i < Inst.getNumOperands(); i++) {
    auto Operand = Inst.getOperand(i);
    AssertIWithMsg(Inst, Operand != nullptr, "Invalid operand");
    AssertIWithMsg(
        Inst,
        getUsersSetForValue(Operand).count(&Inst) == 1,
        "This instruction is not in the User list of the operand");
    if (llvh::isa<Variable>(Operand)) {
      AssertIWithMsg(
          Inst,
          llvh::isa<LoadFrameInst>(Inst) || llvh::isa<StoreFrameInst>(Inst),
          "Variable can only be accessed in "
          "LoadFrame/StoreFrame Inst.");
    }

    // Most instructions that accepts a stack operand must write to it. If it
    // strictly reads from the stack variable, it should go through a
    // LoadStackInst.
    if (llvh::isa<AllocStackInst>(Operand)) {
      AssertIWithMsg(
          Inst,
          llvh::isa<LoadStackInst>(Inst) ||
              Inst.getSideEffect().getWriteStack(),
          "Must write to stack operand.");
    }

    if (Operand->getType().canBeEmpty()) {
      AssertIWithMsg(
          Inst, acceptsEmptyType, "Instruction does not accept empty type");
    } else if (Operand->getType().canBeUninit()) {
      AssertIWithMsg(
          Inst, acceptsEmptyType, "Instruction does not accept uninit type");
    }
  }

  // Verify that terminator instructions never need to return a value.
  if (llvh::isa<TerminatorInst>(Inst)) {
    AssertIWithMsg(
        Inst, Inst.getNumUsers() == 0, "Terminator Inst cannot return value.");
  }

  return true;
}

static bool isTerminator(const Instruction *Inst) {
  return &*Inst->getParent()->rbegin() == Inst;
}

//===----------------------------------------------------------------------===//
// Verifications for each individual instruction.

bool Verifier::visitReturnInst(const ReturnInst &Inst) {
  AssertIWithMsg(
      Inst,
      isTerminator(&Inst),
      "Return Instruction must be the last instruction of a basic block");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 0,
      "ReturnInst should not have successors");
  return true;
}

bool Verifier::visitSaveAndYieldInst(const SaveAndYieldInst &Inst) {
  AssertIWithMsg(
      Inst, isTerminator(&Inst), "SaveAndYield must be a terminator");
  AssertIWithMsg(
      Inst,
      Inst.getParent()->getTerminator() == &Inst,
      "SaveAndYield must be the terminator");
  return true;
}

bool Verifier::visitBranchInst(const BranchInst &Inst) {
  AssertIWithMsg(
      Inst,
      isTerminator(&Inst),
      "Branch Instruction must be the last instruction of a basic block");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 1,
      "Branch Instruction should have only 1 successor");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getBranchDest()),
      "Branch Dest Basic Block does not match with successor pointer");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getBranchDest(), Inst.getParent()),
      "BranchInst Basic Block not in the predecessor list of target block");
  return true;
}

bool Verifier::visitHBCResolveParentEnvironmentInst(
    const HBCResolveParentEnvironmentInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getParentScopeParam() == Inst.getFunction()->getParentScopeParam(),
      "parentScopeParam must be the JSDynamicParam in the Function.");
  return true;
}

bool Verifier::visitAsNumberInst(const AsNumberInst &Inst) {
  AssertIWithMsg(
      Inst,
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  AssertIWithMsg(
      Inst,
      Inst.getType() == Type::createNumber(),
      "AsNumberInst must return a number type");
  return true;
}

bool Verifier::visitAsNumericInst(const AsNumericInst &Inst) {
  AssertIWithMsg(
      Inst,
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  AssertIWithMsg(
      Inst,
      Inst.getType().isSubsetOf(Type::createNumeric()),
      "AsNumericInst must return a numeric type");
  return true;
}

bool Verifier::visitAsInt32Inst(const AsInt32Inst &Inst) {
  AssertIWithMsg(
      Inst,
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  return true;
}

bool Verifier::visitAddEmptyStringInst(const AddEmptyStringInst &Inst) {
  AssertIWithMsg(
      Inst,
      !isTerminator(&Inst),
      "Non-terminator cannot be the last instruction of a basic block");
  AssertIWithMsg(
      Inst,
      Inst.getType() == Type::createString(),
      "AddEmptyStringInst must return a string type");
  return true;
}

bool Verifier::visitAllocStackInst(const AllocStackInst &Inst) {
  AssertIWithMsg(
      Inst,
      &(Inst.getParent()->back()) != &Inst,
      "Alloca Instruction cannot be the last instruction of a basic block");
  return true;
}

template <class T>
bool Verifier::visitCondBranchLikeInst(const T &Inst) {
  AssertIWithMsg(
      Inst,
      isTerminator(&Inst),
      "CondBranchInst must be the last instruction of a basic block");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 2,
      "CondBranchInst should have 2 successors");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getTrueDest()),
      "True dest should be a successor of CondBranchInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getTrueDest(), Inst.getParent()),
      "CondBranchInst block should be a predecessor of true dest");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getFalseDest()),
      "False dest should be a successor of CondBranchInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getFalseDest(), Inst.getParent()),
      "CondBranchInst block should be a predecessor of false dest");
  return true;
}

bool Verifier::visitCondBranchInst(const CondBranchInst &Inst) {
  ReturnIfNot(visitCondBranchLikeInst(Inst));
  return true;
}

bool Verifier::visitUnaryOperatorInst(const UnaryOperatorInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitTryStartInst(const TryStartInst &Inst) {
  AssertIWithMsg(Inst, isTerminator(&Inst), "TryStartInst must be a TermInst");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 2,
      "TryStartInst must have 2 successors");
  AssertIWithMsg(
      Inst,
      Inst.getCatchTarget()->front().getKind() == ValueKind::CatchInstKind,
      "Catch Target of TryStartInst must begin with a CatchInst");
  return true;
}

bool Verifier::visitTryEndInst(const TryEndInst &Inst) {
  AssertIWithMsg(
      Inst,
      &Inst == &Inst.getParent()->front(),
      "TryEndInst must be the first instruction of a block");
  AssertIWithMsg(
      Inst,
      pred_count(Inst.getParent()) == 1,
      "TryEndInst must have only one predecessor.");
  return true;
}

bool Verifier::visitStoreStackInst(const StoreStackInst &Inst) {
  AssertIWithMsg(
      Inst,
      !llvh::isa<AllocStackInst>(Inst.getValue()),
      "Storing the address of stack location");
  AssertIWithMsg(
      Inst, !Inst.hasUsers(), "Store Instructions must not have users");
  return true;
}

bool Verifier::visitStoreFrameInst(const StoreFrameInst &Inst) {
  AssertIWithMsg(
      Inst, !Inst.hasUsers(), "Store Instructions must not have users");
  return true;
}

bool Verifier::visitLoadFrameInst(const LoadFrameInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getType() == Inst.getLoadVariable()->getType(),
      "LoadFrameInst type must be the same as the variable type");
  return true;
}

bool Verifier::visitLoadStackInst(const LoadStackInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getType() == Inst.getPtr()->getType(),
      "LoadStackInst type must be the same as the AllocStackInst type");
  return true;
}

bool Verifier::visitBaseCreateLexicalChildInst(
    const hermes::BaseCreateLexicalChildInst &Inst) {
  auto *VS = llvh::cast<BaseScopeInst>(Inst.getScope())->getVariableScope();

  // Verify that any GetParentScope inside the function produces the same
  // VariableScope that the function is being created with.
  for (auto *U : Inst.getFunctionCode()->getParentScopeParam()->getUsers())
    if (auto *GPSI = llvh::dyn_cast<GetParentScopeInst>(U))
      AssertIWithMsg(
          Inst, GPSI->getVariableScope() == VS, "Parent scope mismatch.");
  return true;
}

bool Verifier::visitBaseCreateCallableInst(const BaseCreateCallableInst &Inst) {
  ReturnIfNot(visitBaseCreateLexicalChildInst(Inst));
  AssertIWithMsg(
      Inst,
      llvh::isa<NormalFunction>(Inst.getFunctionCode()) ||
          llvh::isa<GeneratorFunction>(Inst.getFunctionCode()) ||
          llvh::isa<AsyncFunction>(Inst.getFunctionCode()),
      "BaseCreateCallableInst only accepts NormalFunction/GeneratorFunction");
  return true;
}
bool Verifier::visitCreateFunctionInst(const CreateFunctionInst &Inst) {
  ReturnIfNot(visitBaseCreateCallableInst(Inst));
  return true;
}

bool Verifier::visitCallInst(const CallInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitHBCCallWithArgCountInst(
    const HBCCallWithArgCountInst &Inst) {
  // NumArgumentsLiteral is not always a number literal. For example, it will be
  // lowered into HBCLoadConstant.
  if (auto *LN = llvh::dyn_cast<LiteralNumber>(Inst.getNumArgumentsLiteral())) {
    AssertIWithMsg(
        Inst,
        LN->getValue() == Inst.getNumArguments(),
        "HBCCallWithArgCountInst mismatched arg count");
  }
  return true;
}

bool Verifier::visitHBCCallNInst(const HBCCallNInst &Inst) {
  AssertIWithMsg(
      Inst,
      HBCCallNInst::kMinArgs <= Inst.getNumArguments() &&
          Inst.getNumArguments() <= HBCCallNInst::kMaxArgs,
      "CallNInst has too many args");
  return true;
}

bool Verifier::visitCallBuiltinInst(CallBuiltinInst const &Inst) {
  assert(
      isNativeBuiltin(Inst.getBuiltinIndex()) &&
      "CallBuiltin must take a native builtin.");
  return true;
}

bool Verifier::visitGetBuiltinClosureInst(GetBuiltinClosureInst const &Inst) {
  assert(
      Inst.getBuiltinIndex() < BuiltinMethod::_count &&
      "Out of bound BuiltinMethod index.");
  return true;
}

bool Verifier::visitLoadPropertyInst(const LoadPropertyInst &Inst) {
  return true;
}
bool Verifier::visitTryLoadGlobalPropertyInst(
    const TryLoadGlobalPropertyInst &Inst) {
  return true;
}

bool Verifier::visitDeletePropertyLooseInst(
    const DeletePropertyLooseInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitDeletePropertyStrictInst(
    const DeletePropertyStrictInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitStorePropertyLooseInst(const StorePropertyLooseInst &Inst) {
  return true;
}
bool Verifier::visitStorePropertyStrictInst(
    const StorePropertyStrictInst &Inst) {
  return true;
}

bool Verifier::visitTryStoreGlobalPropertyLooseInst(
    const TryStoreGlobalPropertyLooseInst &Inst) {
  return true;
}
bool Verifier::visitTryStoreGlobalPropertyStrictInst(
    const TryStoreGlobalPropertyStrictInst &Inst) {
  return true;
}

bool Verifier::visitBaseStoreOwnPropertyInst(
    const BaseStoreOwnPropertyInst &Inst) {
  AssertIWithMsg(
      Inst,
      llvh::isa<LiteralBool>(
          Inst.getOperand(StoreOwnPropertyInst::IsEnumerableIdx)),
      "BaseStoreOwnPropertyInst::IsEnumerable must be a boolean literal");
  return true;
}
bool Verifier::visitStoreOwnPropertyInst(const StoreOwnPropertyInst &Inst) {
  return visitBaseStoreOwnPropertyInst(Inst);
}
bool Verifier::visitStoreNewOwnPropertyInst(
    const StoreNewOwnPropertyInst &Inst) {
  ReturnIfNot(visitBaseStoreOwnPropertyInst(Inst));
  AssertIWithMsg(
      Inst,
      Inst.getObject()->getType().isObjectType(),
      "StoreNewOwnPropertyInst::Object must be known to be an object");
  if (auto *LN = llvh::dyn_cast<LiteralNumber>(Inst.getProperty())) {
    AssertIWithMsg(
        Inst,
        LN->convertToArrayIndex().hasValue(),
        "StoreNewOwnPropertyInst::Property can only be an index-like number");
  } else {
    AssertIWithMsg(
        Inst,
        llvh::isa<LiteralString>(Inst.getProperty()),
        "StoreNewOwnPropertyInst::Property must be a string or number literal");
  }
  return true;
}

bool Verifier::visitStoreGetterSetterInst(const StoreGetterSetterInst &Inst) {
  AssertIWithMsg(
      Inst,
      llvh::isa<LiteralBool>(
          Inst.getOperand(StoreGetterSetterInst::IsEnumerableIdx)),
      "StoreGetterSetterInsr::IsEnumerable must be a boolean constant");
  return true;
}

bool Verifier::visitAllocObjectInst(const hermes::AllocObjectInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitAllocFastArrayInst(const hermes::AllocFastArrayInst &Inst) {
  LiteralNumber *size = Inst.getCapacity();
  AssertIWithMsg(
      Inst, size->isUInt32Representible(), "Invalid AllocArrayInst size hint");
  return true;
}

bool Verifier::visitAllocArrayInst(const hermes::AllocArrayInst &Inst) {
  LiteralNumber *size = Inst.getSizeHint();
  AssertIWithMsg(
      Inst, size->isUInt32Representible(), "Invalid AllocArrayInst size hint");
  AssertIWithMsg(
      Inst,
      Inst.isLiteralArray(),
      "Array elements must be literal when registers are limited");
  return true;
}

bool Verifier::visitGetTemplateObjectInst(
    const hermes::GetTemplateObjectInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitCreateArgumentsInst(const CreateArgumentsInst &Inst) {
  AssertIWithMsg(Inst, functionState, "functionState cannot be null");
  AssertIWithMsg(
      Inst,
      !functionState->createArgumentsEncountered,
      "There should be only one CreateArgumentsInst in a function");
  functionState->createArgumentsEncountered = true;

  BasicBlock *BB = Inst.getParent();
  Function *F = BB->getParent();
  if (llvh::isa<GeneratorInnerFunction>(F)) {
    auto secondBB = F->begin();
    ++secondBB;
    AssertIWithMsg(
        Inst,
        BB == &*secondBB,
        "CreateArgumentsInst must be in the second basic block in generators");
  } else {
    AssertIWithMsg(
        Inst,
        BB == &*F->begin(),
        "CreateArgumentsInst must be in the first basic block");
  }
  return true;
}

bool Verifier::visitCreateArgumentsLooseInst(
    const CreateArgumentsLooseInst &Inst) {
  return visitCreateArgumentsInst(Inst);
}
bool Verifier::visitCreateArgumentsStrictInst(
    const CreateArgumentsStrictInst &Inst) {
  return visitCreateArgumentsInst(Inst);
}

bool Verifier::visitCreateRegExpInst(CreateRegExpInst const &Inst) {
  // Nothing to verify at this point.
  return true;
}

template <class T>
bool Verifier::visitBinaryOperatorLikeInst(const T &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitBinaryOperatorInst(const hermes::BinaryOperatorInst &Inst) {
  return visitBinaryOperatorLikeInst(Inst);
}

bool Verifier::visitCatchInst(const CatchInst &Inst) {
  AssertIWithMsg(
      Inst,
      &Inst.getParent()->front() == &Inst,
      "Catch instruction must be the first in a basic block");
  AssertIWithMsg(
      Inst,
      pred_count(Inst.getParent()) == 1,
      "CatchInst must have only 1 predecessor.");
  return true;
}

bool Verifier::visitThrowInst(const ThrowInst &Inst) {
  AssertIWithMsg(Inst, isTerminator(&Inst), "ThrowInst must be a terminator");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 0,
      "ThrowInst should not have successors");
  return true;
}

bool Verifier::visitThrowTypeErrorInst(const ThrowTypeErrorInst &Inst) {
  AssertIWithMsg(
      Inst, isTerminator(&Inst), "ThrowTypeErrorInst must be a terminator");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 0,
      "ThrowTypeErrorInst should not have successors");
  return true;
}

bool Verifier::visitGetNextPNameInst(const GetNextPNameInst &Inst) {
  AssertIWithMsg(
      Inst, isTerminator(&Inst), "GetNextPNameInst must terminate the block");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 2,
      "GetNextPNameInst should have 2 successors");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getOnSomeDest()),
      "OnSome dest should be a successor of GetNextPNameInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getOnSomeDest(), Inst.getParent()),
      "GetNextPNameInst block should be a predecessor of OnSome dest");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getOnLastDest()),
      "OnLast dest should be a successor of GetNextPNameInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getOnLastDest(), Inst.getParent()),
      "GetNextPNameInst block should be a predecessor of OnLast dest");
  return true;
}

bool Verifier::visitGetPNamesInst(const GetPNamesInst &Inst) {
  AssertIWithMsg(
      Inst, isTerminator(&Inst), "GetPNamesInst must terminate the block");
  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == 2,
      "GetPNamesInst should have 2 successors");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getOnSomeDest()),
      "OnSome dest should be a successor of GetPNamesInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getOnSomeDest(), Inst.getParent()),
      "GetPNamesInst block should be a predecessor of OnSome dest");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getOnEmptyDest()),
      "OnEmpty dest should be a successor of GetPNamesInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getOnEmptyDest(), Inst.getParent()),
      "GetPNamesInst block should be a predecessor of OnEmpty dest");
  return true;
}

bool Verifier::visitMovInst(const hermes::MovInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitImplicitMovInst(const hermes::ImplicitMovInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitCoerceThisNSInst(CoerceThisNSInst const &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitPhiInst(const hermes::PhiInst &Inst) {
  // We verify the dominance property when we scan the whole function. In here
  // we only verify local properties.

  llvh::SmallDenseSet<BasicBlock *, 8> entries;

  // Check that every input block enters only once:
  for (int i = 0, e = Inst.getNumEntries(); i < e; ++i) {
    auto pair = Inst.getEntry(i);
    BasicBlock *block = pair.second;
    AssertIWithMsg(
        Inst,
        pred_contains(Inst.getParent(), block),
        "Predecessor not covered by phi node!");
    AssertIWithMsg(
        Inst,
        succ_contains(block, Inst.getParent()),
        "Phi node should be Successor!");

    // In theory, it is legitimate for a Phi to have multiple entries for a
    // block as long as all the associated values are the same. However, such an
    // invariant would be more complicated, and requires care to maintain when
    // updating operands of the Phi.
    auto [it, first] = entries.insert(block);
    AssertIWithMsg(
        Inst, first, "Phi node has multiple entries for the same block");
  }

  AssertIWithMsg(
      Inst,
      entries.size() == pred_count_unique(Inst.getParent()),
      "number of predecessors does not match phi inputs");
  return true;
}

template <class T>
bool Verifier::visitSwitchLikeInst(const T &Inst) {
  AssertIWithMsg(Inst, isTerminator(&Inst), "SwitchInst must be a terminator");

  AssertIWithMsg(
      Inst,
      Inst.getNumCasePair() > 0,
      "SwitchInst must have some case destinations");

  AssertIWithMsg(
      Inst, Inst.getDefaultDestination(), "Invalid destination block");

  AssertIWithMsg(Inst, Inst.getInputValue(), "Invalid input value");

  AssertIWithMsg(
      Inst,
      Inst.getNumSuccessors() == Inst.getNumCasePair() + 1,
      "Number of successors of SwitchInst does not match.");
  AssertIWithMsg(
      Inst,
      succ_contains(Inst.getParent(), Inst.getDefaultDestination()),
      "Default destination must be a successor of SwitchInst block");
  AssertIWithMsg(
      Inst,
      pred_contains(Inst.getDefaultDestination(), Inst.getParent()),
      "SwitchInst block must be a predecessor of default destination.");
  for (unsigned idx = 0, e = Inst.getNumCasePair(); idx < e; ++idx) {
    AssertIWithMsg(
        Inst,
        succ_contains(Inst.getParent(), Inst.getCasePair(idx).second),
        "Case target must be a successor of SwitchInst block");
    AssertIWithMsg(
        Inst,
        pred_contains(Inst.getCasePair(idx).second, Inst.getParent()),
        "SwitchInst block must be a predecessor of case target");
  }

  // Verify that each case is unique.
  llvh::SmallPtrSet<Literal *, 8> values;
  for (unsigned idx = 0, e = Inst.getNumCasePair(); idx < e; ++idx) {
    AssertIWithMsg(
        Inst,
        values.insert(Inst.getCasePair(idx).first).second,
        "switch values must be unique");
  }
  return true;
}

bool Verifier::visitSwitchInst(const hermes::SwitchInst &Inst) {
  return visitSwitchLikeInst(Inst);
}

bool Verifier::visitSwitchImmInst(const hermes::SwitchImmInst &Inst) {
  ReturnIfNot(visitSwitchLikeInst(Inst));
  for (unsigned idx = 0, e = Inst.getNumCasePair(); idx < e; ++idx) {
    AssertIWithMsg(
        Inst,
        Inst.getCasePair(idx).first->isUInt32Representible(),
        "case value must be a uint32");
  }
  return true;
}

bool Verifier::visitDebuggerInst(DebuggerInst const &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitDirectEvalInst(DirectEvalInst const &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitDeclareGlobalVarInst(const DeclareGlobalVarInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitHBCCreateFunctionEnvironmentInst(
    const HBCCreateFunctionEnvironmentInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getFunction()->getFunctionScope() == Inst.getVariableScope(),
      "HBCCreateFunctionEnvironmentInst must take an ES5Function");
  AssertIWithMsg(
      Inst,
      Inst.getParentScopeParam() == Inst.getFunction()->getParentScopeParam(),
      "Using incorect parent scope parameter.");
  return true;
}
bool Verifier::visitGetParentScopeInst(const GetParentScopeInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getParentScopeParam() == Inst.getFunction()->getParentScopeParam(),
      "Using incorect parent scope parameter.");
  return true;
}
bool Verifier::visitCreateScopeInst(const CreateScopeInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getFunction()->getFunctionScope() == Inst.getVariableScope(),
      "Can only create scope for the enclosing function.");
  return true;
}
bool Verifier::visitResolveScopeInst(const ResolveScopeInst &Inst) {
  return true;
}
bool Verifier::visitLIRResolveScopeInst(
    const hermes::LIRResolveScopeInst &Inst) {
  return true;
}
bool Verifier::visitGetClosureScopeInst(
    const hermes::GetClosureScopeInst &Inst) {
  return true;
}

bool Verifier::visitHBCProfilePointInst(const HBCProfilePointInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitHBCAllocObjectFromBufferInst(
    const hermes::HBCAllocObjectFromBufferInst &Inst) {
  LiteralNumber *size = Inst.getSizeHint();
  AssertIWithMsg(
      Inst,
      size->isUInt32Representible(),
      "Invalid HBCAllocObjectFromBufferInst size hint");
  AssertIWithMsg(
      Inst,
      Inst.getKeyValuePairCount() > 0,
      "Cannot allocate an empty HBCAllocObjectFromBufferInst");
  return true;
}

bool Verifier::visitAllocObjectLiteralInst(
    const hermes::AllocObjectLiteralInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getKeyValuePairCount() > 0,
      "Cannot allocate an empty AllocObjectLiteralInst");
  return true;
}

bool Verifier::visitHBCGetGlobalObjectInst(const HBCGetGlobalObjectInst &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitHBCLoadConstInst(hermes::HBCLoadConstInst const &Inst) {
  // Nothing to verify at this point.
  return true;
}

bool Verifier::visitLoadParamInst(hermes::LoadParamInst const &Inst) {
  return true;
}
bool Verifier::visitCompareBranchInst(const CompareBranchInst &Inst) {
  return visitCondBranchLikeInst(Inst) && visitBinaryOperatorLikeInst(Inst);
}

bool Verifier::visitCreateGeneratorInst(const CreateGeneratorInst &Inst) {
  ReturnIfNot(visitBaseCreateLexicalChildInst(Inst));
  AssertIWithMsg(
      Inst,
      llvh::isa<GeneratorInnerFunction>(Inst.getFunctionCode()),
      "CreateGeneratorInst must take a GeneratorInnerFunction");
  return true;
}
bool Verifier::visitStartGeneratorInst(const StartGeneratorInst &Inst) {
  AssertIWithMsg(
      Inst,
      &Inst == &Inst.getParent()->front() &&
          Inst.getParent() == &Inst.getParent()->getParent()->front(),
      "StartGeneratorInst must be the first instruction of a function");
  return true;
}
bool Verifier::visitResumeGeneratorInst(const ResumeGeneratorInst &Inst) {
  return true;
}

bool Verifier::visitLIRGetThisNSInst(const LIRGetThisNSInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitHBCGetArgumentsPropByValLooseInst(
    const HBCGetArgumentsPropByValLooseInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitHBCGetArgumentsPropByValStrictInst(
    const HBCGetArgumentsPropByValStrictInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitHBCGetArgumentsLengthInst(
    const HBCGetArgumentsLengthInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitHBCReifyArgumentsLooseInst(
    const HBCReifyArgumentsLooseInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitHBCReifyArgumentsStrictInst(
    const HBCReifyArgumentsStrictInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitCreateThisInst(const CreateThisInst &Inst) {
  return true;
}
bool Verifier::visitGetConstructedObjectInst(
    const GetConstructedObjectInst &Inst) {
  return true;
}

bool Verifier::visitHBCSpillMovInst(const HBCSpillMovInst &Inst) {
  return true;
}
bool Verifier::visitUnreachableInst(const UnreachableInst &Inst) {
  return true;
}

bool Verifier::visitIteratorBeginInst(const IteratorBeginInst &Inst) {
  AssertIWithMsg(
      Inst,
      llvh::isa<AllocStackInst>(Inst.getSourceOrNext()),
      "SourceOrNext must be an AllocStackInst");
  return true;
}
bool Verifier::visitIteratorNextInst(const IteratorNextInst &Inst) {
  // Nothing to verify at this point.
  return true;
}
bool Verifier::visitIteratorCloseInst(const IteratorCloseInst &Inst) {
  AssertIWithMsg(
      Inst,
      llvh::isa<LiteralBool>(
          Inst.getOperand(IteratorCloseInst::IgnoreInnerExceptionIdx)),
      "IgnoreInnerException must be a LiteralBool in IteratorCloseInst");
  return true;
}

bool Verifier::visitGetNewTargetInst(GetNewTargetInst const &Inst) {
  auto definitionKind = Inst.getParent()->getParent()->getDefinitionKind();
  AssertIWithMsg(
      Inst,
      definitionKind == Function::DefinitionKind::ES5Function ||
          definitionKind == Function::DefinitionKind::ES6Constructor,
      "GetNewTargetInst can only be used in ES6 constructors and ES5 functions");
  AssertIWithMsg(
      Inst,
      Inst.getParent()->getParent()->getNewTargetParam() ==
          Inst.getOperand(GetNewTargetInst::GetNewTargetParamIdx),
      "GetNewTargetInst must use correct getNewTargetParam");
  return true;
}

bool Verifier::visitThrowIfInst(const ThrowIfInst &Inst) {
  const Type invTypes = Inst.getInvalidTypes()->getData();
  AssertIWithMsg(
      Inst,
      !invTypes.isNoType() &&
          invTypes.isSubsetOf(
              Type::unionTy(Type::createEmpty(), Type::createUninit())),
      "ThrowIfInst invalid types set can only contain Empty or Uninit");

  // Note: we are not performing a subtraction and equality check here, because
  // if the invalid types and the input types are proven disjoint after
  // TypeInference, we deliberately don't return NoType.
  AssertIWithMsg(
      Inst,
      Type::intersectTy(Inst.getType(), invTypes).isNoType(),
      "ThrowIfInst must throw away all invalid types");

  // A variable may only perform the following state transitions:
  // empty -> uninit -> initialized
  // empty -> initialized
  // uninit -> initialized
  // So, it can never be empty after it has been checked for uninit.
  AssertIWithMsg(
      Inst,
      !Inst.getType().canBeEmpty(),
      "ThrowIfInst can never return type Empty");
  return true;
}

bool Verifier::visitPrLoadInst(const PrLoadInst &Inst) {
  return true;
}
bool Verifier::visitPrStoreInst(const PrStoreInst &Inst) {
  return true;
}

bool Verifier::visitFastArrayLoadInst(const FastArrayLoadInst &Inst) {
  return true;
}
bool Verifier::visitFastArrayStoreInst(const FastArrayStoreInst &Inst) {
  return true;
}
bool Verifier::visitFastArrayPushInst(const FastArrayPushInst &Inst) {
  return true;
}
bool Verifier::visitFastArrayAppendInst(const FastArrayAppendInst &Inst) {
  return true;
}
bool Verifier::visitFastArrayLengthInst(const FastArrayLengthInst &Inst) {
  return true;
}

bool Verifier::visitLoadParentInst(const LoadParentInst &Inst) {
  return true;
}
bool Verifier::visitStoreParentInst(const StoreParentInst &Inst) {
  return true;
}

bool Verifier::visitFUnaryMathInst(const FUnaryMathInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getArg()->getType().isNumberType() && Inst.getType().isNumberType(),
      "FUnaryMathInst wrong type");
  return true;
}
bool Verifier::visitFBinaryMathInst(const FBinaryMathInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getLeft()->getType().isNumberType() &&
          Inst.getRight()->getType().isNumberType() &&
          Inst.getType().isNumberType(),
      "FBinaryMathInst wrong type");
  return true;
}
bool Verifier::visitFCompareInst(const FCompareInst &Inst) {
  AssertIWithMsg(
      Inst,
      Inst.getLeft()->getType().isNumberType() &&
          Inst.getRight()->getType().isNumberType() &&
          Inst.getType().isBooleanType(),
      "FCompare wrong type");
  return true;
}
bool Verifier::visitStringConcatInst(const StringConcatInst &Inst) {
  AssertIWithMsg(
      Inst, Inst.getType().isStringType(), "StringConcat wrong type");
  for (unsigned i = 0, e = Inst.getNumOperands(); i < e; ++i) {
    AssertIWithMsg(
        Inst,
        Inst.getOperand(i)->getType().isStringType(),
        "StringConcat wrong type");
  }
  return true;
}

bool Verifier::visitUnionNarrowTrustedInst(const UnionNarrowTrustedInst &Inst) {
  return true;
}
bool Verifier::visitCheckedTypeCastInst(const CheckedTypeCastInst &Inst) {
  return true;
}
bool Verifier::visitLIRDeadValueInst(const LIRDeadValueInst &Inst) {
  return true;
}

bool Verifier::visitNativeCallInst(const hermes::NativeCallInst &Inst) {
  // FIXM: this should be added when types are propagated.
  //  Assert(
  //      Inst.getCallee()->getType().isNumberType(),
  //      "NativeCallInst callee must be a number");
  return true;
}

bool Verifier::visitGetNativeRuntimeInst(
    const hermes::GetNativeRuntimeInst &Inst) {
  return true;
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
  raw_ostream &stream = OS ? *OS : NullStr;
  Context &Ctx = M.getContext();
  irdumper::IRPrinter printer(
      Ctx, stream, /*escape*/ false, /*labelAllInsts*/ true);
  Verifier V(M, stream, mode, printer);
  bool result = V.verify();
  return result;
#else
  return true;
#endif
}
