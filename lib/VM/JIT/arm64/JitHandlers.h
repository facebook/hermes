/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/HermesValue.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/sh_legacy_value.h"
#include "hermes/VM/static_h.h"

typedef struct SHRuntime SHRuntime;
typedef struct SHRuntimeModule SHRuntimeModule;
typedef struct SHCodeBlock SHCodeBlock;

namespace hermes::vm {

class Runtime;
class CodeBlock;
class JSObject;
class PinnedHermesValue;

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
SHLegacyValue _interpreter_create_object_from_buffer_with_parent(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    SHLegacyValue *parent,
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

/// Wrapper around Interpreter::caseCreateClass
void _interpreter_create_class(SHRuntime *shr, SHLegacyValue *frameRegs);

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

/// Create a new empty object for the given shape table index.
/// The properties will have to be populated by the caller afterwards.
/// \param tmp a FR pointer on the JS stack used to avoid having to allocate
///   Locals internally. May be overwritten.
JSObject *_jit_new_empty_object_for_buffer(
    Runtime &runtime,
    CodeBlock *codeBlock,
    uint32_t shapeTableIndex,
    PinnedHermesValue *tmp);

#ifdef HERMESVM_PROFILER_BB
/// Register BB execution for BB profiler based on the current callee CodeBlock.
void _interpreter_register_bb_execution(SHRuntime *shr, uint16_t pointIndex);
#endif

/// Run the direct eval on \p text and return the result.
HermesValue
_jit_direct_eval(Runtime &runtime, PinnedHermesValue *text, bool strictCaller);

/// Only valid to call from the longjmp catch handler in the JIT,
/// prior to running the Catch instruction itself.
/// At this point the C++ stack has been restored, but the Runtime register
/// stack has not.
///
/// \return a pointer to the instruction to branch to.
void *_jit_find_catch_target(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    SHLegacyValue *frame,
    SHJmpBuf *jmpBuf,
    SHLocals *savedLocals,
    int32_t *addressTable);

/// Throw an exception that the current function was incorrectly called as a
/// constructor/non-constructor.
[[noreturn]] void _sh_throw_invalid_construct(SHRuntime *shr);
[[noreturn]] void _sh_throw_invalid_call(SHRuntime *shr);

/// Raise an exception that the target of a call is not callable. The outgoing
/// registers must already have been set up.
[[noreturn]] void _jit_throw_non_object_call(SHRuntime *shr);

/// Calls the builtin with index \p builtinMethodID. The new frame is at the top
/// of the stack. The arguments (excluding 'this') must be populated.
SHLegacyValue _jit_call_builtin(
    SHRuntime *shr,
    SHLegacyValue *frame,
    uint32_t argCount,
    uint32_t builtinMethodID);

void _jit_put_by_id(
    SHRuntime *shr,
    SHCodeBlock *codeBlock,
    SHLegacyValue *base,
    SHLegacyValue *value,
    uint8_t cacheIdx,
    SHSymbolID symID,
    bool strictMode,
    bool tryProp);

/// Assumes that \p table is an initialized string switch runtime table.
/// If \p switchValue is found as a case in that table, returns the
/// corresponding JIT code target for that case.  Otherwise, returns nullptr.
/// (This assumes that nullptr is not a valid branch target.)
void *_jit_string_switch_imm_table_lookup(
    StringSwitchDenseMap *table,
    SHLegacyValue *switchValue);

} // namespace hermes::vm
