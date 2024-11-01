/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/sh_legacy_value.h"
#include "hermes/VM/static_h.h"

typedef struct SHRuntime SHRuntime;
typedef struct SHRuntimeModule SHRuntimeModule;
typedef struct SHCodeBlock SHCodeBlock;

namespace hermes::vm {

/// Create a JSFunction closure. The properties of the function (name, etc.)
/// will be populated lazily.
SHLegacyValue _sh_ljs_create_bytecode_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHRuntimeModule *shRuntimeModule,
    uint32_t functionID);

/// Create a generator from bytecode.
SHLegacyValue _interpreter_create_generator(
    SHRuntime *shr,
    SHLegacyValue *frame,
    const SHLegacyValue *env,
    SHRuntimeModule *shRuntimeModule,
    uint32_t functionID);

/// Get the string associated with the given RuntimeModule-specific string ID.
/// This may lazily allocate a SymbolID and the string itself.
/// This is used when executing with a bytecode CodeBlock.
SHLegacyValue _sh_ljs_get_bytecode_string(
    SHRuntime *shr,
    SHRuntimeModule *runtimeModule,
    uint32_t stringID);

/// Get the BigInt associated with the given RuntimeModule-specific BigInt ID.
SHLegacyValue _sh_ljs_get_bytecode_bigint(
    SHRuntime *shr,
    SHRuntimeModule *runtimeModule,
    uint32_t bigintID);

/// Wrapper around Interpreter::createObjectFromBuffer.
SHLegacyValue _interpreter_create_object_from_buffer(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    uint32_t shapeTableIndex,
    uint32_t valBufferOffset);

/// Wrapper around Interpreter::createArrayFromBuffer.
SHLegacyValue _interpreter_create_array_from_buffer(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    unsigned numElements,
    unsigned numLiterals,
    unsigned bufferIndex);

/// Alternative to _sh_ljs_create_regexp that allows using the precompiled
/// regexp bytecode.
SHLegacyValue _interpreter_create_regexp(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    uint32_t pattern,
    uint32_t flags,
    uint32_t regexpID);

/// Implementation of createFunctionEnvironment that takes the closure to get
/// the parentEnvironment from.
/// The native backend doesn't use createFunctionEnvironment.
SHLegacyValue _sh_ljs_create_function_environment(
    SHRuntime *shr,
    SHLegacyValue *frame,
    uint32_t size);

/// Print a message about entering or exiting the specified string, with
/// nesting tracking. Used for debugging.
void _sh_print_function_entry_exit(bool enter, const char *msg);

/// Concat two values that are known to be strings.
SHLegacyValue
_sh_ljs_string_add(SHRuntime *shr, SHLegacyValue *left, SHLegacyValue *right);

#ifdef HERMESVM_PROFILER_BB
/// Register BB execution for BB profiler based on the current callee CodeBlock.
void _interpreter_register_bb_execution(SHRuntime *shr, uint16_t pointIndex);
#endif

/// Throw an exception that the current function was incorrectly called as a
/// constructor/non-constructor.
[[noreturn]] void _sh_throw_invalid_construct(SHRuntime *shr);
[[noreturn]] void _sh_throw_invalid_call(SHRuntime *shr);

/// Throw a register stack overflow exception.
[[noreturn]] void _sh_throw_register_stack_overflow(SHRuntime *shr);

/// Call the closure stored in the outgoing registers of the current frame. The
/// caller is responsible for setting up the callee closure, arg count, and
/// new.target registers.
SHLegacyValue _jit_dispatch_call(SHRuntime *, SHLegacyValue *frame);

} // namespace hermes::vm
