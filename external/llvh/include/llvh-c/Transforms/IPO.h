/*===-- IPO.h - Interprocedural Transformations C Interface -----*- C++ -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header declares the C interface to libLLVMIPO.a, which implements     *|
|* various interprocedural transformations of the LLVM IR.                    *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_TRANSFORMS_IPO_H
#define LLVM_C_TRANSFORMS_IPO_H

#include "llvh-c/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup LLVMCTransformsIPO Interprocedural transformations
 * @ingroup LLVMCTransforms
 *
 * @{
 */

/** See llvh::createArgumentPromotionPass function. */
void LLVMAddArgumentPromotionPass(LLVMPassManagerRef PM);

/** See llvh::createConstantMergePass function. */
void LLVMAddConstantMergePass(LLVMPassManagerRef PM);

/** See llvh::createCalledValuePropagationPass function. */
void LLVMAddCalledValuePropagationPass(LLVMPassManagerRef PM);

/** See llvh::createDeadArgEliminationPass function. */
void LLVMAddDeadArgEliminationPass(LLVMPassManagerRef PM);

/** See llvh::createFunctionAttrsPass function. */
void LLVMAddFunctionAttrsPass(LLVMPassManagerRef PM);

/** See llvh::createFunctionInliningPass function. */
void LLVMAddFunctionInliningPass(LLVMPassManagerRef PM);

/** See llvh::createAlwaysInlinerPass function. */
void LLVMAddAlwaysInlinerPass(LLVMPassManagerRef PM);

/** See llvh::createGlobalDCEPass function. */
void LLVMAddGlobalDCEPass(LLVMPassManagerRef PM);

/** See llvh::createGlobalOptimizerPass function. */
void LLVMAddGlobalOptimizerPass(LLVMPassManagerRef PM);

/** See llvh::createIPConstantPropagationPass function. */
void LLVMAddIPConstantPropagationPass(LLVMPassManagerRef PM);

/** See llvh::createPruneEHPass function. */
void LLVMAddPruneEHPass(LLVMPassManagerRef PM);

/** See llvh::createIPSCCPPass function. */
void LLVMAddIPSCCPPass(LLVMPassManagerRef PM);

/** See llvh::createInternalizePass function. */
void LLVMAddInternalizePass(LLVMPassManagerRef, unsigned AllButMain);

/** See llvh::createStripDeadPrototypesPass function. */
void LLVMAddStripDeadPrototypesPass(LLVMPassManagerRef PM);

/** See llvh::createStripSymbolsPass function. */
void LLVMAddStripSymbolsPass(LLVMPassManagerRef PM);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif
