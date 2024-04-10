/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRUtils.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {
namespace {
/// Set the insertion point of \p builder to \p I, and set its location to \p I.
static void moveBuilderTo(Instruction *I, IRBuilder &builder) {
  builder.setLocation(I->getLocation());
  builder.setInsertionPoint(I);
}

/// Set the insertion point of \p builder after \p I, and set its location to \p
/// I.
static void moveBuilderAfter(Instruction *I, IRBuilder &builder) {
  builder.setLocation(I->getLocation());
  builder.setInsertionPointAfter(I);
}

/// This class will perform a transformation of the IR for a given pair of inner
/// and outer generator functions. A state machine is created to branch to the
/// correct location inside the inner function. All of the state that needs to
/// be persisted between generator invocations is lifted to the outer function's
/// environment.
class LowerToStateMachine {
  /// Outer generator function.
  GeneratorFunction *outer_;
  /// Inner generator function.
  GeneratorInnerFunction *inner_;
  /// The instruction that creates the inner generator function.
  CreateGeneratorInst *CGI_;
  /// Scope instruction operand passed to the CreateGenerator instruction.
  BaseScopeInst *newOuterScope_;
  /// Instruction to obtain the outer function scope. Should be created in the
  /// inner function.
  BaseScopeInst *getParentOuterScope_;
  /// Builder used to generate IR.
  IRBuilder builder_;

 public:
  explicit LowerToStateMachine(
      Module *M,
      GeneratorFunction *outer,
      CreateGeneratorInst *CGI)
      : outer_(outer),
        inner_(llvh::cast<GeneratorInnerFunction>(CGI->getFunctionCode())),
        CGI_(CGI),
        newOuterScope_(nullptr),
        getParentOuterScope_(nullptr),
        builder_(M) {}

  /// Peform the conversion to the state machine.
  void convert();

 private:
  /// Create a new scope in the outer function that we will store all the needed
  /// variables into.
  void setupScopes();

  /// Move all 'local' values of the inner function to variables in the outer
  /// function. Local here refers to instructions referencing local parameters,
  /// and AllocStackInsts.
  void moveLocalsToOuter();
};

void LowerToStateMachine::convert() {
  setupScopes();
  moveLocalsToOuter();
}

void LowerToStateMachine::setupScopes() {
  moveBuilderTo(CGI_, builder_);
  // Create a new scope in the outer function to store our variables into. This
  // means we must change the operand to the CreateGeneratorInst with this new
  // scope.
  auto *existingScopeOperand = llvh::cast<BaseScopeInst>(CGI_->getScope());
  auto *newScope = builder_.createCreateScopeInst(
      builder_.createVariableScope(existingScopeOperand->getVariableScope()),
      existingScopeOperand);
  CGI_->setOperand(newScope, CreateGeneratorInst::ScopeIdx);
  newOuterScope_ = newScope;
  movePastFirstInBlock(builder_, &inner_->front());
  getParentOuterScope_ = builder_.createGetParentScopeInst(
      newScope->getVariableScope(), inner_->getParentScopeParam());
  IRBuilder::InstructionDestroyer destroyer{};
  for (auto &I : inner_->getParentScopeParam()->getUsers()) {
    // Don't process the GPSI that we created above.
    if (I == getParentOuterScope_)
      continue;
    // We have just inserted a new layer of scope above the inner function. This
    // means that all the existing GPSI's are now inaccurate. What they
    // originally returned can be obtained using a ResolveScopeInst, starting
    // from what the current, new parent is.
    if (auto *GPSI = llvh::dyn_cast<GetParentScopeInst>(I)) {
      moveBuilderTo(GPSI, builder_);
      auto *RSI = builder_.createResolveScopeInst(
          GPSI->getVariableScope(),
          getParentOuterScope_->getVariableScope(),
          getParentOuterScope_);
      GPSI->replaceAllUsesWith(RSI);
      destroyer.add(GPSI);
    }
  }
}

void LowerToStateMachine::moveLocalsToOuter() {
  auto &innerFuncParams = inner_->getJSDynamicParams();
  // Map the parameter in the inner function to the variable declared in the
  // outer_ function scope that is replacing it.
  llvh::DenseMap<JSDynamicParam *, Variable *> paramReplacements;
  builder_.setInsertionPoint(CGI_);
  for (size_t i = 0, e = innerFuncParams.size(); i < e; ++i) {
    auto jsParam = innerFuncParams[i];
    auto outerVar = builder_.createVariable(
        getParentOuterScope_->getVariableScope(),
        jsParam->getName(),
        jsParam->getType());
    paramReplacements.insert({jsParam, outerVar});
    if (i == 0) {
      // The 'this' param already exists in outer_, so no need to
      // create a new JSDynamicParam.
      builder_.createStoreFrameInst(
          newOuterScope_,
          builder_.createLoadParamInst(outer_->getJSDynamicParams()[0]),
          outerVar);
    } else {
      auto *outerJSParam = outer_->addJSDynamicParam(jsParam->getName());
      builder_.createStoreFrameInst(
          newOuterScope_, builder_.createLoadParamInst(outerJSParam), outerVar);
    }
  }

  // We have to handle AllocStackInst separately, because we create new
  // AllocStackInsts in the process. This would lead to an infinite loop if we
  // tried to process AllocStackInst during the iteration of all instructions in
  // each BasicBlock.
  llvh::SmallVector<AllocStackInst *, 4> allocStacks;
  IRBuilder::InstructionDestroyer destroyer{};
  // This builder will be left in the correct spot to insert instructions at the
  // end of outer_. Place it right before CGI_ so we can correctly reference its
  // arguments.
  IRBuilder beforeCGI(outer_);
  beforeCGI.setInsertionPoint(CGI_);
  for (BasicBlock &BB : *inner_) {
    for (auto iter = BB.begin(), end = BB.end(); iter != end;) {
      auto &I = *iter++;
      if (auto *ASI = llvh::dyn_cast<AllocStackInst>(&I)) {
        allocStacks.push_back(ASI);
        continue;
      }

      if (auto *LPI = llvh::dyn_cast<LoadParamInst>(&I)) {
        Variable *v = paramReplacements[LPI->getParam()];
        assert(v && "JSDynamicParam should have had a replacement variable");
        moveBuilderTo(LPI, builder_);
        auto *loadReplacement =
            builder_.createLoadFrameInst(getParentOuterScope_, v);
        LPI->replaceAllUsesWith(loadReplacement);
        destroyer.add(LPI);
        continue;
      }

      if (auto *CBI = llvh::dyn_cast<CallBuiltinInst>(&I)) {
        // copyRestArgs references the local register stack of the inner
        // function, to scan and copy the parameters. However, we have
        // transferred the parameters of the inner function to the outer
        // function. So we should also transfer copyRestArgs to the outer
        // function.
        auto builtinIdx =
            llvh::cast<LiteralNumber>(CBI->getCallee())->getValue();
        if (builtinIdx == BuiltinMethod::HermesBuiltin_copyRestArgs) {
          Variable *outerRestArgs = builder_.createVariable(
              getParentOuterScope_->getVariableScope(),
              CBI->getName(),
              CBI->getType());
          moveBuilderTo(CBI, builder_);
          auto *replacement =
              builder_.createLoadFrameInst(getParentOuterScope_, outerRestArgs);
          CBI->replaceAllUsesWith(replacement);
          auto *outerCBI = beforeCGI.createCallBuiltinInst(
              BuiltinMethod::HermesBuiltin_copyRestArgs,
              beforeCGI.getLiteralNumber(
                  llvh::cast<LiteralNumber>(CBI->getArgument(1))->getValue()));
          beforeCGI.createStoreFrameInst(
              newOuterScope_, outerCBI, outerRestArgs);
          destroyer.add(CBI);
        }
        continue;
      }

      if (auto *CAI = llvh::dyn_cast<CreateArgumentsInst>(&I)) {
        // Create a CreateArgumentsInst in outer_, write it to a variable in
        // outer_, and replace usages here with a read to the variable.
        Variable *argumentsVar = beforeCGI.createVariable(
            getParentOuterScope_->getVariableScope(),
            CAI->getName(),
            Type::createObject());
        auto *argInstInOuter = llvh::isa<CreateArgumentsLooseInst>(CAI)
            ? static_cast<CreateArgumentsInst *>(
                  beforeCGI.createCreateArgumentsLooseInst())
            : beforeCGI.createCreateArgumentsStrictInst();
        beforeCGI.createStoreFrameInst(
            newOuterScope_, argInstInOuter, argumentsVar);
        moveBuilderTo(CAI, builder_);
        CAI->replaceAllUsesWith(
            builder_.createLoadFrameInst(getParentOuterScope_, argumentsVar));
        destroyer.add(CAI);
        continue;
      }
    }
  }

  for (AllocStackInst *ASI : allocStacks) {
    auto *outerVar = builder_.createVariable(
        getParentOuterScope_->getVariableScope(),
        ASI->getVariableName(),
        ASI->getType());
    // Users of AllocStackInst must take AllocStackInst directly as they write
    // to it. To deal with this, create a 'dummy' AllocStackInst for each user
    // of the original AllocStackInst. Once it's done writing to the dummy,
    // propagate the value back to the outer_ variable. Users can also read the
    // stack variable, so initialze the dummy alloc stack to the value of the
    // outer_ variable.
    for (Instruction *user : ASI->getUsers()) {
      moveBuilderTo(user, builder_);
      // We can use simpler code paths for LoadStack and StoreStack, which
      // only read or only write to the stack space, respectively.
      if (auto *LSI = llvh::dyn_cast<LoadStackInst>(user)) {
        auto *loadFrameReplacement =
            builder_.createLoadFrameInst(getParentOuterScope_, outerVar);
        LSI->replaceAllUsesWith(loadFrameReplacement);
        destroyer.add(LSI);
      } else if (auto *SSI = llvh::dyn_cast<StoreStackInst>(user)) {
        auto *storeFrameReplacement = builder_.createStoreFrameInst(
            getParentOuterScope_, SSI->getValue(), outerVar);
        SSI->replaceAllUsesWith(storeFrameReplacement);
        destroyer.add(SSI);
      } else {
        auto *dummyAlloc = builder_.createAllocStackInst(
            ASI->getVariableName(), ASI->getType());
        builder_.createStoreStackInst(
            builder_.createLoadFrameInst(getParentOuterScope_, outerVar),
            dummyAlloc);
        moveBuilderAfter(user, builder_);
        auto *writtenStackVal = builder_.createLoadStackInst(dummyAlloc);
        builder_.createStoreFrameInst(
            getParentOuterScope_, writtenStackVal, outerVar);
        user->replaceFirstOperandWith(ASI, dummyAlloc);
      }
    }
    destroyer.add(ASI);
  }

  // Clear out the original parameters in inner_, except for 'this'.
  innerFuncParams.resize(1);
}

} // namespace

/// \return the corresponding NormalFunction for the given \p outer
/// generator function, or nullptr if there is none. In the case of a generator
/// function which is unreachable, the inner function may have been deleted
/// because it will never be created.
static CreateGeneratorInst *findCreateGenerator(GeneratorFunction *outer) {
  for (BasicBlock &BB : *outer) {
    for (Instruction &I : BB) {
      if (auto *CGI = llvh::dyn_cast<CreateGeneratorInst>(&I)) {
        return CGI;
      }
    }
  }
  return nullptr;
}

Pass *createLowerGeneratorFunction() {
  /// Process all GeneratorFunction and GeneratorInnerFunctions. Turn them into
  /// regular function IR, removing all generator-specific IR.
  class LowerGeneratorFunction : public ModulePass {
   public:
    explicit LowerGeneratorFunction() : ModulePass("LowerGeneratorFunction") {}
    ~LowerGeneratorFunction() override = default;
    bool runOnModule(Module *M) override {
      // This lowering pass may be inserted both in optimization and lowering.
      // In that case, we shoud simply return early.
      if (M->areGeneratorsLowered())
        return false;
      bool changed = false;
      for (auto &F : *M) {
        if (auto *GF = llvh::dyn_cast<GeneratorFunction>(&F)) {
          if (auto *inner = findCreateGenerator(GF)) {
            LowerToStateMachine(M, GF, inner).convert();
            changed = true;
          }
        }
      }
      M->setGeneratorsLowered(true);
      return changed;
    }
  };
  return new LowerGeneratorFunction();
}
} // namespace hermes
