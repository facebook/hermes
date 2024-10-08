/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LoweringPipelines.h"

#include "BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/Passes/InsertProfilePoint.h"
#include "hermes/BCGen/HBC/Passes/OptParentEnvironment.h"
#include "hermes/BCGen/HBC/Passes/PeepholeLowering.h"
#include "hermes/BCGen/HBC/Passes/ReorderRegisters.h"
#include "hermes/BCGen/LowerBuiltinCalls.h"
#include "hermes/BCGen/LowerScopes.h"
#include "hermes/BCGen/LowerStoreInstrs.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/BCGen/MovElimination.h"
#include "hermes/IR/IRVerifier.h"

namespace hermes {
namespace hbc {

void lowerModuleIR(Module *M, const BytecodeGenerationOptions &options) {
  if (M->isLowered())
    return;

  PassManager PM("HBC Lowering");
  // LowerGeneratorFunction produces ThrowTypeErrorInst, so it should run before
  // PeepholeLowering.
  PM.addLowerGeneratorFunction();
  // Lowering ExponentiationOperator and ThrowTypeError (in PeepholeLowering)
  // needs to run before LowerBuiltinCalls because it introduces calls to
  // HermesInternal.
  PM.addPass(new PeepholeLowering());
  PM.addPass(createLowerScopes());
  // LowerBuilinCalls needs to run before the rest of the lowering.
  PM.addPass(new LowerBuiltinCalls());
  PM.addPass(new LowerCalls());
  // It is important to run LowerNumericProperties before LoadConstants
  // as LowerNumericProperties could generate new constants.
  PM.addPass(new LowerNumericProperties());
  // Lower AllocObjectLiteral into a mixture of HBCAllocObjectFromBufferInst,
  // AllocObjectInst, StoreNewOwnPropertyInst and StorePropertyInst.
  PM.addPass(new LowerAllocObjectLiteral());
  PM.addPass(new LowerArgumentsArray());
  PM.addPass(new LimitAllocArray(UINT16_MAX));
  PM.addPass(new DedupReifyArguments());
  PM.addPass(new LowerSwitchIntoJumpTables());
  PM.addPass(new SwitchLowering());
  if (options.optimizationEnabled) {
    // TODO(T204084366): TypeInference must run before OptEnvironmentInit,
    // because the latter will remove stores that may affect the inferred type.
    PM.addTypeInference();
    // OptEnvironmentInit needs to run before LowerConstants.
    PM.addPass(createOptEnvironmentInit());
  }
  PM.addPass(new LoadConstants());
  if (options.optimizationEnabled) {
    PM.addPass(createOptParentEnvironment());
    // Reduce comparison and conditional jump to single comparison jump
    PM.addPass(new LowerCondBranch());
    // Turn Calls into CallNs.
    // Move loads to child blocks if possible.
    PM.addCodeMotion();
    // Eliminate common HBCLoadConstInsts.
    // TODO(T140823187): Run before CodeMotion too.
    // Avoid pushing HBCLoadConstInsts down into individual blocks,
    // preventing their elimination.
    PM.addCSE();
    // Drop unused LoadParamInsts.
    PM.addDCE();
  }

  // Move StartGenerator instructions to the start of functions.
  PM.addHoistStartGenerator();

  if (!PM.run(M))
    return;
  M->setLowered(true);

  if (options.verifyIR &&
      !verifyModule(*M, &llvh::errs(), VerificationMode::IR_LOWERED)) {
    M->getContext().getSourceErrorManager().error(
        SMLoc{}, "Lowered IR verification failed");
    M->dump(llvh::errs());
    return;
  }
}

void lowerAllocatedFunctionIR(
    Function *F,
    HVMRegisterAllocator &RA,
    const BytecodeGenerationOptions &options) {
  PassManager PM("HBC LowerAllocatedFunctionIR");
  if (options.optimizationEnabled) {
    PM.addPass(new MovElimination(RA));
    PM.addPass(new ReorderRegisters(RA));
  }
  PM.addPass(new LowerStoreInstrs(RA));
  PM.addPass(new InitCallFrame(RA));
  if (options.optimizationEnabled) {
    PM.addPass(new MovElimination(RA));
    PM.addPass(new RecreateCheapValues(RA));
    PM.addPass(new LoadConstantValueNumbering(RA));
  }
  PM.addPass(new SpillRegisters(RA));
  if (options.basicBlockProfiling) {
    // Insert after all other passes so that it sees final basic block
    // list.
    PM.addPass(new InsertProfilePoint());
  }
  PM.run(F);
}

} // namespace hbc
} // namespace hermes
