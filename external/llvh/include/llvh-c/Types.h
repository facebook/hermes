/*===-- llvm-c/Support.h - C Interface Types declarations ---------*- C -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This file defines types used by the C interface to LLVM.                   *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_TYPES_H
#define LLVM_C_TYPES_H

#include "llvh-c/DataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup LLVMCSupportTypes Types and Enumerations
 *
 * @{
 */

typedef int LLVMBool;

/* Opaque types. */

/**
 * LLVM uses a polymorphic type hierarchy which C cannot represent, therefore
 * parameters must be passed as base types. Despite the declared types, most
 * of the functions provided operate only on branches of the type hierarchy.
 * The declared parameter names are descriptive and specify which type is
 * required. Additionally, each type hierarchy is documented along with the
 * functions that operate upon it. For more detail, refer to LLVM's C++ code.
 * If in doubt, refer to Core.cpp, which performs parameter downcasts in the
 * form unwrap<RequiredType>(Param).
 */

/**
 * Used to pass regions of memory through LLVM interfaces.
 *
 * @see llvh::MemoryBuffer
 */
typedef struct LLVMOpaqueMemoryBuffer *LLVMMemoryBufferRef;

/**
 * The top-level container for all LLVM global data. See the LLVMContext class.
 */
typedef struct LLVMOpaqueContext *LLVMContextRef;

/**
 * The top-level container for all other LLVM Intermediate Representation (IR)
 * objects.
 *
 * @see llvh::Module
 */
typedef struct LLVMOpaqueModule *LLVMModuleRef;

/**
 * Each value in the LLVM IR has a type, an LLVMTypeRef.
 *
 * @see llvh::Type
 */
typedef struct LLVMOpaqueType *LLVMTypeRef;

/**
 * Represents an individual value in LLVM IR.
 *
 * This models llvh::Value.
 */
typedef struct LLVMOpaqueValue *LLVMValueRef;

/**
 * Represents a basic block of instructions in LLVM IR.
 *
 * This models llvh::BasicBlock.
 */
typedef struct LLVMOpaqueBasicBlock *LLVMBasicBlockRef;

/**
 * Represents an LLVM Metadata.
 *
 * This models llvh::Metadata.
 */
typedef struct LLVMOpaqueMetadata *LLVMMetadataRef;

/**
 * Represents an LLVM Named Metadata Node.
 *
 * This models llvh::NamedMDNode.
 */
typedef struct LLVMOpaqueNamedMDNode *LLVMNamedMDNodeRef;

/**
 * Represents an entry in a Global Object's metadata attachments.
 *
 * This models std::pair<unsigned, MDNode *>
 */
typedef struct LLVMOpaqueValueMetadataEntry LLVMValueMetadataEntry;

/**
 * Represents an LLVM basic block builder.
 *
 * This models llvh::IRBuilder.
 */
typedef struct LLVMOpaqueBuilder *LLVMBuilderRef;

/**
 * Represents an LLVM debug info builder.
 *
 * This models llvh::DIBuilder.
 */
typedef struct LLVMOpaqueDIBuilder *LLVMDIBuilderRef;

/**
 * Interface used to provide a module to JIT or interpreter.
 * This is now just a synonym for llvh::Module, but we have to keep using the
 * different type to keep binary compatibility.
 */
typedef struct LLVMOpaqueModuleProvider *LLVMModuleProviderRef;

/** @see llvh::PassManagerBase */
typedef struct LLVMOpaquePassManager *LLVMPassManagerRef;

/** @see llvh::PassRegistry */
typedef struct LLVMOpaquePassRegistry *LLVMPassRegistryRef;

/**
 * Used to get the users and usees of a Value.
 *
 * @see llvh::Use */
typedef struct LLVMOpaqueUse *LLVMUseRef;

/**
 * Used to represent an attributes.
 *
 * @see llvh::Attribute
 */
typedef struct LLVMOpaqueAttributeRef *LLVMAttributeRef;

/**
 * @see llvh::DiagnosticInfo
 */
typedef struct LLVMOpaqueDiagnosticInfo *LLVMDiagnosticInfoRef;

/**
 * @see llvh::Comdat
 */
typedef struct LLVMComdat *LLVMComdatRef;

/**
 * @see llvh::Module::ModuleFlagEntry
 */
typedef struct LLVMOpaqueModuleFlagEntry LLVMModuleFlagEntry;

/**
 * @see llvh::JITEventListener
 */
typedef struct LLVMOpaqueJITEventListener *LLVMJITEventListenerRef;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
