/*===-- Scalar.h - Scalar Transformation Library C Interface ----*- C++ -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header declares the C interface to libLLVMScalarOpts.a, which         *|
|* implements various scalar transformations of the LLVM IR.                  *|
|*                                                                            *|
|* Many exotic languages can interoperate with C code but have a harder time  *|
|* with C++ due to name mangling. So in addition to C, this interface enables *|
|* tools written in such languages.                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_TRANSFORMS_SCALAR_H
#define LLVM_C_TRANSFORMS_SCALAR_H

#include "llvh-c/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup LLVMCTransformsScalar Scalar transformations
 * @ingroup LLVMCTransforms
 *
 * @{
 */

/** See llvh::createAggressiveDCEPass function. */
void LLVMAddAggressiveDCEPass(LLVMPassManagerRef PM);

/** See llvh::createBitTrackingDCEPass function. */
void LLVMAddBitTrackingDCEPass(LLVMPassManagerRef PM);

/** See llvh::createAlignmentFromAssumptionsPass function. */
void LLVMAddAlignmentFromAssumptionsPass(LLVMPassManagerRef PM);

/** See llvh::createCFGSimplificationPass function. */
void LLVMAddCFGSimplificationPass(LLVMPassManagerRef PM);

/** See llvh::createDeadStoreEliminationPass function. */
void LLVMAddDeadStoreEliminationPass(LLVMPassManagerRef PM);

/** See llvh::createScalarizerPass function. */
void LLVMAddScalarizerPass(LLVMPassManagerRef PM);

/** See llvh::createMergedLoadStoreMotionPass function. */
void LLVMAddMergedLoadStoreMotionPass(LLVMPassManagerRef PM);

/** See llvh::createGVNPass function. */
void LLVMAddGVNPass(LLVMPassManagerRef PM);

/** See llvh::createGVNPass function. */
void LLVMAddNewGVNPass(LLVMPassManagerRef PM);

/** See llvh::createIndVarSimplifyPass function. */
void LLVMAddIndVarSimplifyPass(LLVMPassManagerRef PM);

/** See llvh::createInstructionCombiningPass function. */
void LLVMAddInstructionCombiningPass(LLVMPassManagerRef PM);

/** See llvh::createJumpThreadingPass function. */
void LLVMAddJumpThreadingPass(LLVMPassManagerRef PM);

/** See llvh::createLICMPass function. */
void LLVMAddLICMPass(LLVMPassManagerRef PM);

/** See llvh::createLoopDeletionPass function. */
void LLVMAddLoopDeletionPass(LLVMPassManagerRef PM);

/** See llvh::createLoopIdiomPass function */
void LLVMAddLoopIdiomPass(LLVMPassManagerRef PM);

/** See llvh::createLoopRotatePass function. */
void LLVMAddLoopRotatePass(LLVMPassManagerRef PM);

/** See llvh::createLoopRerollPass function. */
void LLVMAddLoopRerollPass(LLVMPassManagerRef PM);

/** See llvh::createLoopUnrollPass function. */
void LLVMAddLoopUnrollPass(LLVMPassManagerRef PM);

/** See llvh::createLoopUnrollAndJamPass function. */
void LLVMAddLoopUnrollAndJamPass(LLVMPassManagerRef PM);

/** See llvh::createLoopUnswitchPass function. */
void LLVMAddLoopUnswitchPass(LLVMPassManagerRef PM);

/** See llvh::createLowerAtomicPass function. */
void LLVMAddLowerAtomicPass(LLVMPassManagerRef PM);

/** See llvh::createMemCpyOptPass function. */
void LLVMAddMemCpyOptPass(LLVMPassManagerRef PM);

/** See llvh::createPartiallyInlineLibCallsPass function. */
void LLVMAddPartiallyInlineLibCallsPass(LLVMPassManagerRef PM);

/** See llvh::createReassociatePass function. */
void LLVMAddReassociatePass(LLVMPassManagerRef PM);

/** See llvh::createSCCPPass function. */
void LLVMAddSCCPPass(LLVMPassManagerRef PM);

/** See llvh::createSROAPass function. */
void LLVMAddScalarReplAggregatesPass(LLVMPassManagerRef PM);

/** See llvh::createSROAPass function. */
void LLVMAddScalarReplAggregatesPassSSA(LLVMPassManagerRef PM);

/** See llvh::createSROAPass function. */
void LLVMAddScalarReplAggregatesPassWithThreshold(LLVMPassManagerRef PM,
                                                  int Threshold);

/** See llvh::createSimplifyLibCallsPass function. */
void LLVMAddSimplifyLibCallsPass(LLVMPassManagerRef PM);

/** See llvh::createTailCallEliminationPass function. */
void LLVMAddTailCallEliminationPass(LLVMPassManagerRef PM);

/** See llvh::createConstantPropagationPass function. */
void LLVMAddConstantPropagationPass(LLVMPassManagerRef PM);

/** See llvh::demotePromoteMemoryToRegisterPass function. */
void LLVMAddDemoteMemoryToRegisterPass(LLVMPassManagerRef PM);

/** See llvh::createVerifierPass function. */
void LLVMAddVerifierPass(LLVMPassManagerRef PM);

/** See llvh::createCorrelatedValuePropagationPass function */
void LLVMAddCorrelatedValuePropagationPass(LLVMPassManagerRef PM);

/** See llvh::createEarlyCSEPass function */
void LLVMAddEarlyCSEPass(LLVMPassManagerRef PM);

/** See llvh::createEarlyCSEPass function */
void LLVMAddEarlyCSEMemSSAPass(LLVMPassManagerRef PM);

/** See llvh::createLowerExpectIntrinsicPass function */
void LLVMAddLowerExpectIntrinsicPass(LLVMPassManagerRef PM);

/** See llvh::createTypeBasedAliasAnalysisPass function */
void LLVMAddTypeBasedAliasAnalysisPass(LLVMPassManagerRef PM);

/** See llvh::createScopedNoAliasAAPass function */
void LLVMAddScopedNoAliasAAPass(LLVMPassManagerRef PM);

/** See llvh::createBasicAliasAnalysisPass function */
void LLVMAddBasicAliasAnalysisPass(LLVMPassManagerRef PM);

/** See llvh::createUnifyFunctionExitNodesPass function */
void LLVMAddUnifyFunctionExitNodesPass(LLVMPassManagerRef PM);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif
