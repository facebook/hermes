/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
// The core idea of this pass is to transform the sequence of yield instructions
// within the generator function to one large switch statement that branches to
// the correct spot in the function. Each target block of a SaveAndYieldInst
// will occupy a slot in this switch statement. Each SaveAndYieldInst will then
// turn into an instruction that modifies the switch variable to point to the
// next block that the function should resume from.
//
// Exceptions:
// As a result of this new switch structure, we cannot retain the TryStartInsts
// that existed in the function originally. It would be invalid for the main
// switch to jump immediately into the middle of a block that was enclosed in a
// try. So, we remove all of the user's try instructions, and wrap the execution
// of all user blocks in one single try.  If an exception is thrown, we have to
// figure out which user block would have caught it, had we not removed all the
// try instructions. The exception handler we install needs to modify the switch
// variable to point to the correct user handler, then restart the execution of
// the main switch again. The installed handler recovers the information it
// needs by using another switch and switch variable. All of the try
// instructions (TryStart/Catch/TryEnd) modify the exception handler switch
// variable with a unique number. This is enough information to be able to
// recover which user error handler block we should jump to when an exception is
// thrown.
//
// All 'local' instructions must be removed from the inner function and lifted
// into the outer function as a variable. Local here means any instructions that
// refer to stack values, such as AllocStackInst or instructions that read
// paramters.
//
// The creation of the switch changes the dominance structure between the
// existing blocks. Values that now cross blocks without proper dominance need
// to be lifted into the outer closure.
//
//===----------------------------------------------------------------------===//

#include "hermes/BCGen/GeneratorResumeMethod.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRUtils.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"

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

/// ES6.0 7.4.7
/// \return ReturnInst of object {value: \p val, done: \p done}.
static ReturnInst *
emitReturnIterResultObject(IRBuilder &builder, Value *val, bool done) {
  AllocObjectLiteralInst::ObjectPropertyMap props{};
  props.push_back({builder.getLiteralString("value"), val});
  props.push_back(
      {builder.getLiteralString("done"), builder.getLiteralBool(done)});
  return builder.createReturnInst(builder.createAllocObjectLiteralInst(props));
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
  NormalFunction *inner_;
  /// The instruction that creates the inner generator function.
  CreateGeneratorInst *CGI_;
  /// Scope instruction operand passed to the CreateGenerator instruction.
  BaseScopeInst *newOuterScope_;
  /// Instruction to obtain the outer function scope. Should be created in the
  /// inner function.
  BaseScopeInst *getParentOuterScope_;
  /// Builder used to generate IR.
  IRBuilder builder_;
  /// Set to true if the resulting function contains try blocks. Used to decide
  /// whether to run fixupCatchTargets.
  bool resultContainsTrys_ = false;

  /// Represents the GeneratorState internal slot.
  /// ES6.0 25.3.2.
  enum class State {
    SuspendedStart,
    SuspendedYield,
    Executing,
    Completed,
  };

 public:
  explicit LowerToStateMachine(
      Module *M,
      GeneratorFunction *outer,
      CreateGeneratorInst *CGI)
      : outer_(outer),
        inner_(llvh::cast<NormalFunction>(CGI->getFunctionCode())),
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

  /// Terminators need to be handled differently than other instructions
  /// consuming an AllocStackInst. We cannot insert the IR to do the copying of
  /// the dummy to the outer variable right after the instruction. No
  /// instructions are allowed after a terminator. So, we create a new BB per
  /// successor of the terminator. This new BB does the writeback of the
  /// dummyAlloc to the outer variable, and then branches to the original
  /// successor.
  ///
  /// \p TI the terminator that is taking in an AllocStackInst.
  /// \p dummyAlloc the replacement AllocStackInst for TI.
  /// \p outerVar needs to be updated with the value of dummyAlloc.
  /// \p seenBefore true if \p TI has been encountered already.
  void writebackTerminator(
      TerminatorInst *TI,
      AllocStackInst *dummyAlloc,
      Variable *outerVar,
      bool seenBefore);

  /// Lower ResumeGeneratorInst.
  void lowerResumeGenerator(
      LoadParamInst *actionParam,
      LoadParamInst *valueParam,
      Variable *genState);

  /// Lower the inner function to a switch.
  void lowerToSwitch(
      LoadParamInst *actionParam,
      LoadParamInst *valueParam,
      Variable *genState);

  /// Find all values that are used across basic blocks without proper
  /// dominance. Move these values to the outer function.
  void moveCrossingValuesToOuter();

  /// Find PhiInsts that have an entry block which is not a predecessor of the
  /// PhiInst. Move these instructions to the outer function.
  void moveInnerPhisToOuter(DominanceInfo &D);

  /// Create a BB to handle being in the 'Completed' state of the generator.
  BasicBlock *createCompletedStateBlock(
      LoadParamInst *action,
      LoadParamInst *value);
};

void LowerToStateMachine::convert() {
  setupScopes();
  moveLocalsToOuter();

  // The inner function will take 2 parameters: action, value. Action
  // communicates what method was called on the generator: next, return, or
  // throw. For return and throw actions, value will hold the parameter that was
  // passed.
  auto *actionParam = builder_.createJSDynamicParam(
      inner_, builder_.createIdentifier("action"));
  actionParam->setType(Type::createUint32());
  auto *valueParam =
      builder_.createJSDynamicParam(inner_, builder_.createIdentifier("value"));
  movePastFirstInBlock(builder_, &*inner_->begin());
  auto *loadActionParam = builder_.createLoadParamInst(actionParam);
  auto *loadValueParam = builder_.createLoadParamInst(valueParam);

  // This variable holds the state of the generator, directly corresponding to
  // ES6.0 25.3.2.
  Variable *genState = builder_.createVariable(
      getParentOuterScope_->getVariableScope(),
      "generator_state",
      Type::createInt32());

  lowerResumeGenerator(loadActionParam, loadValueParam, genState);
  lowerToSwitch(loadActionParam, loadValueParam, genState);
  // Creating the switch will break the connection between user BBs in the
  // process. Values that flow between the newly disjointed BBs are illegal and
  // should be moved to the closure. So do the promotion after these
  // operations.
  moveCrossingValuesToOuter();

  // If the result contains trys, fixup the throws.
  if (resultContainsTrys_)
    fixupCatchTargets(inner_);
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
      auto *outerJSParam =
          builder_.createJSDynamicParam(outer_, jsParam->getName());
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
        if (CBI->getBuiltinIndex() ==
            BuiltinMethod::HermesBuiltin_copyRestArgs) {
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

  // This enables us to recognize when we encounter a given TerminatorInst user
  // for the first time.
  llvh::DenseSet<TerminatorInst *> processedTerminators;
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
        if (auto *TI = llvh::dyn_cast<TerminatorInst>(user)) {
          bool seenBefore = !processedTerminators.insert(TI).second;
          writebackTerminator(TI, dummyAlloc, outerVar, seenBefore);
        } else {
          moveBuilderAfter(user, builder_);
          auto *writtenStackVal = builder_.createLoadStackInst(dummyAlloc);
          builder_.createStoreFrameInst(
              getParentOuterScope_, writtenStackVal, outerVar);
        }
        user->replaceFirstOperandWith(ASI, dummyAlloc);
      }
    }
    destroyer.add(ASI);
  }

  // Clear out the original parameters in inner_, except for 'this'.
  innerFuncParams.resize(1);
}

void LowerToStateMachine::writebackTerminator(
    TerminatorInst *TI,
    AllocStackInst *dummyAlloc,
    Variable *outerVar,
    bool seenBefore) {
  // For a given successor BB, find its replacement BB that performs the
  // writeback and branches to the original successor.
  llvh::SmallDenseMap<BasicBlock *, BasicBlock *> replacementBBs;
  for (size_t i = 0, e = TI->getNumSuccessors(); i < e; ++i) {
    BasicBlock *origSuccBB = TI->getSuccessor(i);
    BasicBlock *updateVarBB = origSuccBB;
    if (!seenBefore) {
      // First time this terminator is seen, create the blocks.
      auto &replacementBB = replacementBBs[origSuccBB];
      if (replacementBB) {
        // If the same BB in different successor entries, we want to update the
        // BB used in the terminator for that successor, but we don't need to do
        // anything else since the replacement BB has already been created and
        // initialized with the proper IR.
        TI->setSuccessor(i, replacementBB);
        continue;
      }
      replacementBB = builder_.createBasicBlock(inner_);
      builder_.setInsertionBlock(replacementBB);
      builder_.createBranchInst(origSuccBB);
      TI->setSuccessor(i, replacementBB);
      updateVarBB = replacementBB;
      updateIncomingPhiValues(origSuccBB, TI->getParent(), replacementBB);
    }
    builder_.setInsertionPoint(&*updateVarBB->begin());
    auto *writtenStackVal = builder_.createLoadStackInst(dummyAlloc);
    builder_.createStoreFrameInst(
        getParentOuterScope_, writtenStackVal, outerVar);
  }
}

void LowerToStateMachine::lowerResumeGenerator(
    LoadParamInst *actionParam,
    LoadParamInst *valueParam,
    Variable *genState) {
  IRBuilder::InstructionDestroyer destroyer{};
  // This variable holds the value to return if the user executed .return().
  // Store this in a variable because it potentially needs to persist across
  // multiple generator invocations.
  Variable *valueToReturn = builder_.createVariable(
      getParentOuterScope_->getVariableScope(),
      "return_value",
      Type::createAnyType());
  for (BasicBlock &BB : *inner_) {
    for (auto iter = BB.begin(), end = BB.end(); iter != end; ++iter) {
      Instruction *I = &*iter;
      auto *RGI = llvh::dyn_cast<ResumeGeneratorInst>(I);
      if (!RGI)
        continue;
      // All of the instructions we generate here should contain the same
      // location information as RGI.
      builder_.setLocation(RGI->getLocation());
      // RGI checks: if .throw(valueParam) was called, then throw valueParam.
      // We are going to split the current basic block at this RGI, and put
      // the check for .throw at the end of this block.
      ++iter;
      auto *throwBlockBB = builder_.createBasicBlock(inner_);
      auto *restOfInstsBB = splitBasicBlock(&BB, iter);

      // Now put the check at the end of this block.
      builder_.setInsertionBlock(&BB);
      builder_.createCompareBranchInst(
          actionParam,
          builder_.getLiteralNumber((uint8_t)Action::Throw),
          ValueKind::CmpBrStrictlyEqualInstKind,
          throwBlockBB,
          restOfInstsBB);

      builder_.setInsertionBlock(throwBlockBB);
      builder_.createStoreFrameInst(
          getParentOuterScope_,
          builder_.getLiteralNumber((uint8_t)State::Completed),
          genState);
      builder_.createThrowInst(valueParam);

      // Else, RGI takes an AllocStackInst and writes a boolean value to it.
      // Write true iff the method called on the generator was
      // .return(valueParam). The return value of RGI is valueParam.
      builder_.setInsertionPoint(&*restOfInstsBB->begin());
      builder_.createStoreFrameInst(
          getParentOuterScope_, valueParam, valueToReturn);
      auto *loadReturnVar =
          builder_.createLoadFrameInst(getParentOuterScope_, valueToReturn);
      RGI->replaceAllUsesWith(loadReturnVar);
      auto *isReturn = builder_.createFCompareInst(
          ValueKind::FEqualInstKind,
          actionParam,
          builder_.getLiteralNumber((uint8_t)Action::Return));
      builder_.createStoreStackInst(
          isReturn, llvh::cast<AllocStackInst>(RGI->getIsReturn()));
      destroyer.add(RGI);
      // There can't be anymore ResumeGeneratorInsts in this BB, so skip to
      // the next one.
      break;
    }
  }
}

/// Helper class for creating a SwitchInst.
class SwitchBuilder {
  IRBuilder &builder_;
  /// Values the switch index can take.
  SwitchInst::ValueListType values_{};
  /// Branches of the switch.
  SwitchInst::BasicBlockListType switchTargets_{};
  /// BB -> value in the switch that leads to it.
  llvh::DenseMap<BasicBlock *, size_t> bbMapping_{};

 public:
  explicit SwitchBuilder(IRBuilder &builder) : builder_(builder) {}

  /// \return the index \p BB occupies in the switch. If \p BB is not in the
  /// switch, then add it.
  size_t getBBIdx(BasicBlock *BB) {
    auto idx = bbMapping_.size();
    auto [pair, didInsert] = bbMapping_.try_emplace(BB, idx);
    if (didInsert) {
      values_.push_back(builder_.getLiteralNumber(idx));
      switchTargets_.push_back(BB);
    }
    return pair->second;
  }

  /// Construct and \return the SwitchInst.
  SwitchInst *generate(Value *operand, BasicBlock *defaultBlock) {
    return builder_.createSwitchInst(
        operand, defaultBlock, values_, switchTargets_);
  }

  /// \return number of cases in the switch.
  size_t size() const {
    return values_.size();
  }
};

void LowerToStateMachine::lowerToSwitch(
    LoadParamInst *actionParam,
    LoadParamInst *valueParam,
    Variable *genState) {
  // Collect the original user blocks, before we created any.
  llvh::SmallVector<BasicBlock *, 6> userBBs;
  for (auto &BB : *inner_) {
    userBBs.push_back(&BB);
  }

  // Before we modify anything, record the relationship of each block to its
  // enclosing try.
  auto blockToEnclosingTry = findEnclosingTrysPerBlock(inner_);
  // If there are no existing trys, then we will not create a try of our own.
  // Instead we simply let all exceptions bubble up out of the inner function,
  // which is what should happen since there are no trys anywhere.
  bool hasExistingTrys = blockToEnclosingTry.hasValue();

  // This is the main switch that will contain all target blocks of
  // SaveAndYieldInsts and all catch handlers of a try-catch.
  SwitchBuilder mainSwitch(builder_);
  Variable *switchIdx = builder_.createVariable(
      getParentOuterScope_->getVariableScope(), "idx", Type::createInt32());

  // Original entry point to the function.
  auto *oldBeginBB = &*inner_->begin();
  // New entry point to the function.
  auto *newBeginBB = builder_.createBasicBlock(inner_);
  inner_->moveBlockToEntry(newBeginBB);
  // Execute the switch, potentially wrapped in a try.
  auto *executeSwitchBB = builder_.createBasicBlock(inner_);
  // Holds the switch that jumps to the correct user block after yields.
  auto *userCodeSwitchBB = builder_.createBasicBlock(inner_);

  // We begin by checking the current state of the generator.
  // If executing, throw.
  // Else, if the generator is completed, go to completedStateBB
  // Else, dispatch to switch.
  auto *throwBecauseExecutingBB = builder_.createBasicBlock(inner_);
  auto *checkIfCompletedBB = builder_.createBasicBlock(inner_);
  builder_.setInsertionBlock(newBeginBB);

  auto *loadState =
      builder_.createLoadFrameInst(getParentOuterScope_, genState);
  // Move these instructions to the new beginning block so that
  // they don't cross any blocks without dominance.
  valueParam->moveBefore(loadState);
  actionParam->moveBefore(loadState);

  // Note also that this must be before genState since it is an operand.
  getParentOuterScope_->moveBefore(loadState);

  builder_.createCompareBranchInst(
      loadState,
      builder_.getLiteralNumber((uint8_t)State::Executing),
      ValueKind::CmpBrStrictlyEqualInstKind,
      throwBecauseExecutingBB,
      checkIfCompletedBB);

  builder_.setInsertionBlock(throwBecauseExecutingBB);
  builder_.createStoreFrameInst(
      getParentOuterScope_,
      builder_.getLiteralNumber((uint8_t)State::Completed),
      genState);
  builder_.createThrowTypeErrorInst(builder_.getLiteralString(
      "Generator functions may not be called on executing generators"));

  auto *completedStateBB = createCompletedStateBlock(actionParam, valueParam);
  // If the state of the generator is completed, go to completedStateBB. Else,
  // try to execute the main switch.
  builder_.setInsertionBlock(checkIfCompletedBB);
  builder_.createCompareBranchInst(
      builder_.createLoadFrameInst(getParentOuterScope_, genState),
      builder_.getLiteralNumber((uint8_t)State::Completed),
      ValueKind::CmpBrStrictlyEqualInstKind,
      completedStateBB,
      executeSwitchBB);

  // Initialize the main switch index and generator state variables.
  builder_.setInsertionPoint(CGI_);
  // oldBeginBB is where the original first BB instructions are. This should
  // be the first case that is executed, so initialize the switch index
  // variable to the correct index.
  // Use createGeneratorScopeArg_ here since we are emitting this IR in the
  // outer function.
  builder_.createStoreFrameInst(
      newOuterScope_,
      builder_.getLiteralNumber(mainSwitch.getBBIdx(oldBeginBB)),
      switchIdx);
  builder_.createStoreFrameInst(
      newOuterScope_,
      builder_.getLiteralNumber((uint8_t)State::SuspendedStart),
      genState);

  // This switch will be executed when an exception is thrown and is responsible
  // for setting up switchIdx to point to the correct block for the user catch
  // handler.
  SwitchBuilder exceptionSwitch(builder_);
  Variable *exceptionSwitchIdx = nullptr;
  // This BB guards the entire execution of user blocks.
  BasicBlock *surroundingCatchBB = nullptr;
  // The top level catch will propagate the caught error to the user
  // handlers via this value.
  Variable *thrownValPlaceholder = nullptr;
  // The default case in the exception handler switch is to simply re-throw the
  // exception. Also mark the generator as completed.
  BasicBlock *rethrowBB = nullptr;
  size_t rethrowBBIdx = 0;
  if (hasExistingTrys) {
    thrownValPlaceholder = builder_.createVariable(
        getParentOuterScope_->getVariableScope(),
        "catchVal",
        Type::createAnyType());
    rethrowBB = builder_.createBasicBlock(inner_);
    builder_.setInsertionBlock(rethrowBB);
    builder_.createStoreFrameInst(
        getParentOuterScope_,
        builder_.getLiteralNumber((uint8_t)State::Completed),
        genState);
    auto loadVal = builder_.createLoadFrameInst(
        getParentOuterScope_, thrownValPlaceholder);
    builder_.createThrowInst(loadVal);
    rethrowBBIdx = exceptionSwitch.getBBIdx(rethrowBB);
    builder_.setInsertionPoint(CGI_);
    // Initialize the exception index to rethrow idx, which is equivalent to
    // being outside of any try body.
    exceptionSwitchIdx = builder_.createVariable(
        getParentOuterScope_->getVariableScope(),
        "exception_handler_idx",
        Type::createInt32());
    builder_.createStoreFrameInst(
        newOuterScope_,
        builder_.getLiteralNumber(rethrowBBIdx),
        exceptionSwitchIdx);
    // Wrap execution of switch in a try.
    builder_.setInsertionBlock(executeSwitchBB);
    surroundingCatchBB = builder_.createBasicBlock(inner_);
    builder_.createTryStartInst(userCodeSwitchBB, surroundingCatchBB);
  } else {
    // Don't wrap execution of switch in a try.
    builder_.setInsertionBlock(executeSwitchBB);
    builder_.createBranchInst(userCodeSwitchBB);
  }

  // This holds the mapping of user error handler to the exception switch index
  // that will lead to the execution of that handler.
  llvh::DenseMap<BasicBlock *, size_t> handlersToExceptionSwitchIdx;
  // Get the index in handlersToExceptionSwitchIdx that would execute \p BB, or
  // return rethrowBBIdx if \p BB is not enclosed in any try.
  auto getEnclosingHandlerIdx = [rethrowBBIdx,
                                 &blockToEnclosingTry,
                                 &handlersToExceptionSwitchIdx](
                                    BasicBlock *BB) {
    // Check to see if BB is enclosed in a try. If so, the index is the user
    // handler of the enclosing TryStartInst. If not, use the index for the
    // re-throwing block.
    size_t idx = rethrowBBIdx;
    auto enclosing = blockToEnclosingTry->find(BB);
    if (enclosing != blockToEnclosingTry->end() && enclosing->second) {
      idx = handlersToExceptionSwitchIdx[enclosing->second->getCatchTarget()];
    }
    return idx;
  };

  // Emit IR to return an iteration result object, potentially wrapped in a
  // block with a TryEnd. builder_ should be set in the desired location for the
  // return to be emitted before calling.
  auto emitInnerReturn = [this, surroundingCatchBB](
                             Value *val, bool isDelegated, bool done) {
    // If there are existing trys, then end the try in a separate BB before we
    // emit the return instruction.
    if (surroundingCatchBB) {
      auto *nextBlock = builder_.createBasicBlock(inner_);
      builder_.createTryEndInst(surroundingCatchBB, nextBlock);
      builder_.setInsertionBlock(nextBlock);
    }
    if (isDelegated) {
      // Delegated yields (yield*) should not wrap the given value in an
      // iteration object.
      builder_.createReturnInst(val);
    } else {
      emitReturnIterResultObject(builder_, val, done);
    }
  };

  IRBuilder::InstructionDestroyer destroyer{};
  for (auto *BB : userBBs) {
    for (Instruction &I : *BB) {
      if (auto *SGI = llvh::dyn_cast<StartGeneratorInst>(&I)) {
        // We've already replicated the semantics of StartGeneratorInst, so
        // remove it.
        destroyer.add(SGI);
      } else if (auto *YI = llvh::dyn_cast<SaveAndYieldInst>(&I)) {
        // Return a value, and set switchIdx to jump to the next block
        // specified in the parameter of SaveAndYieldInst.
        auto *nextBB = YI->getNextBlock();
        // We want to execute nextBB via the switch on the next function
        // invocation. Add nextBB to the switch, and set the switch index
        // variable correctly.
        size_t idx = mainSwitch.getBBIdx(nextBB);
        moveBuilderTo(YI, builder_);
        builder_.createStoreFrameInst(
            getParentOuterScope_, builder_.getLiteralNumber(idx), switchIdx);
        builder_.createStoreFrameInst(
            getParentOuterScope_,
            builder_.getLiteralNumber((uint8_t)State::SuspendedYield),
            genState);
        emitInnerReturn(
            YI->getResult(),
            /*isDelegated*/ YI->getIsDelegated(),
            /*done*/ false);
        destroyer.add(YI);
      } else if (auto *TSI = llvh::dyn_cast<TryStartInst>(&I)) {
        auto *userCatchBB = TSI->getCatchTarget();
        // Replace the catch with a read from the variable holding the thrown
        // value.
        auto *catchInst = llvh::cast<CatchInst>(&*userCatchBB->begin());
        moveBuilderTo(catchInst, builder_);
        catchInst->replaceAllUsesWith(builder_.createLoadFrameInst(
            getParentOuterScope_, thrownValPlaceholder));
        destroyer.add(catchInst);
        // Entering into the catch handler changes what handler should be
        // called if an exception is thrown.
        builder_.createStoreFrameInst(
            getParentOuterScope_,
            builder_.getLiteralNumber(getEnclosingHandlerIdx(userCatchBB)),
            exceptionSwitchIdx);

        // This block will be dispatched to from the top level catch switch. It
        // is responsible for setting up the main switch index to point to the
        // correct user catch handler.
        auto *setupUserHandlerBB = builder_.createBasicBlock(inner_);
        builder_.setInsertionBlock(setupUserHandlerBB);
        builder_.createStoreFrameInst(
            getParentOuterScope_,
            builder_.getLiteralNumber(mainSwitch.getBBIdx(userCatchBB)),
            switchIdx);
        builder_.createBranchInst(executeSwitchBB);
        size_t setupBBIdx = exceptionSwitch.getBBIdx(setupUserHandlerBB);
        handlersToExceptionSwitchIdx.insert({userCatchBB, setupBBIdx});

        // The try body should update the exception switch index to point at
        // setupUserHandlerBB.
        auto *tryBody = TSI->getTryBody();
        movePastFirstInBlock(builder_, tryBody);
        builder_.createStoreFrameInst(
            getParentOuterScope_,
            builder_.getLiteralNumber(setupBBIdx),
            exceptionSwitchIdx);

        // Turn the TryStartInst into an unconditional branch to the try body.
        moveBuilderTo(TSI, builder_);
        auto *branch = builder_.createBranchInst(tryBody);
        TSI->replaceAllUsesWith(branch);
        destroyer.add(TSI);
      } else if (auto *TEI = llvh::dyn_cast<TryEndInst>(&I)) {
        // TryEndInst needs to update the exception switch index because it
        // changes what handler should be invoked if an exception were to be
        // thrown.
        moveBuilderTo(TEI, builder_);
        builder_.createStoreFrameInst(
            getParentOuterScope_,
            builder_.getLiteralNumber(
                getEnclosingHandlerIdx(TEI->getBranchDest())),
            exceptionSwitchIdx);
        builder_.createBranchInst(TEI->getBranchDest());
        destroyer.add(TEI);
      } else if (auto *RI = llvh::dyn_cast<ReturnInst>(&I)) {
        // ReturnInsts from IRGen have special semantics: they are meant to
        // complete the generator.
        moveBuilderTo(RI, builder_);
        builder_.createStoreFrameInst(
            getParentOuterScope_,
            builder_.getLiteralNumber((uint8_t)State::Completed),
            genState);
        emitInnerReturn(
            RI->getValue(),
            /*isDelegated*/ false,
            /*done*/ true);
        destroyer.add(RI);
      }
    }
  }

  auto *defaultMainSwitchBB = builder_.createBasicBlock(inner_);
  builder_.setInsertionBlock(defaultMainSwitchBB);
  // The switch index should always be hitting a value we explicitly defined, so
  // this should never execute.
  builder_.createUnreachableInst();

  builder_.setInsertionBlock(userCodeSwitchBB);
  builder_.createStoreFrameInst(
      getParentOuterScope_,
      builder_.getLiteralNumber((uint8_t)State::Executing),
      genState);
  mainSwitch.generate(
      builder_.createLoadFrameInst(getParentOuterScope_, switchIdx),
      defaultMainSwitchBB);

  if (hasExistingTrys) {
    auto *defaultExceptionSwitchBB = builder_.createBasicBlock(inner_);
    builder_.setInsertionBlock(defaultExceptionSwitchBB);
    // The switch index should always be hitting a value we explicitly defined,
    // so this should never execute.
    builder_.createUnreachableInst();

    // Set the thrown value placeholder variable and execute the switch.
    builder_.setInsertionBlock(surroundingCatchBB);
    auto *catchVal = builder_.createCatchInst();
    builder_.createStoreFrameInst(
        getParentOuterScope_, catchVal, thrownValPlaceholder);
    exceptionSwitch.generate(
        builder_.createLoadFrameInst(getParentOuterScope_, exceptionSwitchIdx),
        defaultExceptionSwitchBB);
  }

  resultContainsTrys_ = hasExistingTrys;
}

BasicBlock *LowerToStateMachine::createCompletedStateBlock(
    LoadParamInst *actionParam,
    LoadParamInst *valueParam) {
  // The done state functionally replicates ResumeGeneratorInst.
  // 1. If the method called was .throw(X), then throw X
  // 2. Else if the method called was .return(X), then return X
  // 3. Else, return undefined

  auto *checkIsThrowBB = builder_.createBasicBlock(inner_);
  auto *throwValueBB = builder_.createBasicBlock(inner_);
  auto *checkIsReturnBB = builder_.createBasicBlock(inner_);
  auto *returnValueBB = builder_.createBasicBlock(inner_);
  auto *returnUndefBB = builder_.createBasicBlock(inner_);

  builder_.setInsertionBlock(checkIsThrowBB);
  builder_.createCompareBranchInst(
      actionParam,
      builder_.getLiteralNumber((uint8_t)Action::Throw),
      ValueKind::CmpBrStrictlyEqualInstKind,
      throwValueBB,
      checkIsReturnBB);

  builder_.setInsertionBlock(checkIsReturnBB);
  builder_.createCompareBranchInst(
      actionParam,
      builder_.getLiteralNumber((uint8_t)Action::Return),
      ValueKind::CmpBrStrictlyEqualInstKind,
      returnValueBB,
      returnUndefBB);

  builder_.setInsertionBlock(returnUndefBB);
  emitReturnIterResultObject(builder_, builder_.getLiteralUndefined(), true);

  builder_.setInsertionBlock(returnValueBB);
  emitReturnIterResultObject(builder_, valueParam, true);

  builder_.setInsertionBlock(throwValueBB);
  builder_.createThrowInst(valueParam);

  return checkIsThrowBB;
}

void LowerToStateMachine::moveCrossingValuesToOuter() {
  DominanceInfo D(inner_);
  // Move PhiInsts first. That promotion can potentially localize the
  // lifetime of a value to a single BB, obviating the need to lift it
  // additionally into the outer scope in this pass.
  moveInnerPhisToOuter(D);

  for (BasicBlock &BB : *inner_) {
    for (Instruction &I : BB) {
      // Store the illegal users separately as we cannot modify the users list
      // as we iterate it.
      llvh::SmallVector<Instruction *, 2> illegalUsers;
      for (const auto &user : I.getUsers()) {
        if (llvh::isa<PhiInst>(user)) {
          // Phis were already processed.
          continue;
        }
        BasicBlock *userBB = user->getParent();
        if (userBB != &BB && !D.properlyDominates(&BB, userBB)) {
          illegalUsers.push_back(user);
        }
      }
      if (illegalUsers.empty())
        continue;

      // The current instruction is used across a BB it does not dominate. Store
      // the value of the instruction into the environment, and replace the
      // illegal usages with a read.
      Variable *storedValueOfI = builder_.createVariable(
          getParentOuterScope_->getVariableScope(), I.getName(), I.getType());
      // This is a small optimization to reduce the number of reads to the
      // replacement variable we need to create for each illegal user. Illegal
      // users that reside in the same block can all share the same single read
      // from the variable.
      llvh::DenseMap<BasicBlock *, LoadFrameInst *> loadFramePerBlock;
      moveBuilderAfter(&I, builder_);
      builder_.createStoreFrameInst(getParentOuterScope_, &I, storedValueOfI);
      for (const auto &user : illegalUsers) {
        auto *userBB = user->getParent();
        LoadFrameInst *&load = loadFramePerBlock[userBB];
        if (!load) {
          movePastFirstInBlock(builder_, userBB);
          load = builder_.createLoadFrameInst(
              getParentOuterScope_, storedValueOfI);
        }
        moveBuilderTo(user, builder_);
        user->replaceFirstOperandWith(&I, load);
      }
    }
  }
}

/// \return true if an entry BasicBlock in \p PI is not a predecessor of
/// the BasicBlock \p PI resides in.
static bool shouldMovePhiInst(
    PhiInst *PI,
    const llvh::DenseSet<BasicBlock *> &predBBs) {
  for (size_t i = 0, e = PI->getNumEntries(); i < e; ++i) {
    auto [_, valBB] = PI->getEntry(i);
    if (!predBBs.count(valBB)) {
      return true;
    }
  }
  return false;
}

void LowerToStateMachine::moveInnerPhisToOuter(DominanceInfo &D) {
  IRBuilder::InstructionDestroyer destroyer{};
  for (BasicBlock &BB : *inner_) {
    llvh::DenseSet<BasicBlock *> predBBs;
    predBBs.insert(pred_begin(&BB), pred_end(&BB));
    for (Instruction &I : BB) {
      if (auto *PI = llvh::dyn_cast<PhiInst>(&I)) {
        if (!shouldMovePhiInst(PI, predBBs))
          continue;
        auto outerVar = builder_.createVariable(
            getParentOuterScope_->getVariableScope(),
            PI->getName(),
            PI->getType());
        for (size_t i = 0, e = PI->getNumEntries(); i < e; ++i) {
          // For each possible value that PhiInst can take, create a
          // corresponding storeFrameInst in the BB it needs to come from in
          // order to have that value. The PhiInst will then read from the
          // frame to obtain the value.
          const auto &[val, predBB] = PI->getEntry(i);
          moveBuilderTo(&predBB->back(), builder_);
          builder_.createStoreFrameInst(getParentOuterScope_, val, outerVar);
        }
        moveBuilderTo(PI, builder_);
        auto loadReplacement =
            builder_.createLoadFrameInst(getParentOuterScope_, outerVar);
        PI->replaceAllUsesWith(loadReplacement);
        destroyer.add(PI);
      }
    }
  }
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
