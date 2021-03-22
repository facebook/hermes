/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_EXTERNALCALLS_H
#define HERMES_VM_JIT_EXTERNALCALLS_H

#include "hermes/VM/CallResult.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/StackFrame-inline.h"

namespace hermes {
namespace vm {

class Runtime;

CallResult<HermesValue> slowPathToNumber(
    Runtime *runtime,
    PinnedHermesValue *src);

CallResult<HermesValue> slowPathAddEmptyString(
    Runtime *runtime,
    PinnedHermesValue *src);

/// An external call invoked by JIT compiled code to convert \p val to a 32-bit
/// signed integer.
CallResult<HermesValue> externToInt32(Runtime *runtime, PinnedHermesValue *val);

/// An external call invoked by JIT compiled code to declare a global variable
/// by string table index.
/// \param stringID string table index of this global variable
ExecutionStatus externCallDeclareGlobalVar(Runtime *runtime, uint32_t stringID);

/// An external call invoked by JIT compiled code to create a new environment.
/// \param currentFrame the current frame on the stack
/// \param envSize the size of the environment to be created.
/// \return a newly created Environment.
CallResult<HermesValue> externCreateEnvironment(
    Runtime *runtime,
    PinnedHermesValue *currentFrame,
    uint32_t envSize);

/// An external call invoked by JIT compiled code to allocate a closure for the
/// specified function and the specified environment.
/// \param codeBlock code block of the Closure to be created.
/// \param env the environment of the Closure to be created.
/// \return a newly created JSFunction.
CallResult<HermesValue> externCreateClosure(
    Runtime *runtime,
    CodeBlock *codeBlock,
    PinnedHermesValue *env);

/// An external call invoked by JIT compiled code to set an object property by
/// string index.
/// \param opFlags property access flags.
/// \param sid the SymbolID of the property which must already exist in the
///  string id map.
/// \param target the target to set a property in.
/// \param prop the property to be set.
/// \param cacheIdx a cache index to speed up property write and read.
ExecutionStatus externPutById(
    Runtime *runtime,
    PropOpFlags opFlags,
    uint32_t sid,
    PinnedHermesValue *target,
    PinnedHermesValue *prop,
    uint8_t cacheIdx);

/// An external call invoked by JIT compiled code to get an object property by
/// string index.
/// \param opFlags property access flags.
/// \param sid the SymbolID of the property which must already exist in the
///  string id map.
/// \param target the target to get a property from.
/// \param cacheIdx a cache index to speed up property write and read.
/// \param codeBlock the current code block.
CallResult<HermesValue> externGetById(
    Runtime *runtime,
    PropOpFlags opFlags,
    uint32_t sid,
    PinnedHermesValue *target,
    uint8_t cacheIdx,
    CodeBlock *codeBlock);

/// An external call invoked by JIT compiled code to call a Callable entity.
/// \param callable the callable entity, it should be a NativeFunction,
/// JSFunction or BoundFunction, otherwise an exception is returned
/// \param argCount the count of arguments, including the "thisArg"
/// \param stackPointer the runtime stack pointer
/// \param ip the ip in the caller code block to be saved before the call
/// \param previousFrame the previous frame to be saved before the call
CallResult<HermesValue> externCall(
    Runtime *runtime,
    PinnedHermesValue *callable,
    uint32_t argCount,
    PinnedHermesValue *stackPointer,
    Inst const *ip,
    PinnedHermesValue *previousFrame);

/// An external call invoked by JIT compiled code to call a constructor.
/// \param callable the callable entity, it should be a NativeFunction,
/// JSFunction or BoundFunction, otherwise an exception is returned
/// \param argCount the count of arguments, including the "thisArg"
/// \param stackPointer the runtime stack pointer
/// \param ip the ip in the caller code block to be saved before the call
/// \param previousFrame the previous frame to be saved before the call
CallResult<HermesValue> externConstruct(
    Runtime *runtime,
    PinnedHermesValue *callable,
    uint32_t argCount,
    PinnedHermesValue *stackPointer,
    Inst const *ip,
    PinnedHermesValue *previousFrame);

/// An slow path invoked by JIT compiled code to convert operands to number
/// and do subtraction (op1 - op2)
CallResult<HermesValue>
slowPathSub(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An slow path invoked by JIT compiled code to do numeric or string addition
CallResult<HermesValue>
slowPathAdd(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An slow path invoked by JIT compiled code to convert operands to number
/// and do multiplication
CallResult<HermesValue>
slowPathMul(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An slow path invoked by JIT compiled code to convert operands to number
/// and do division
CallResult<HermesValue>
slowPathDiv(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to load a constant string,
/// which is allocated in the GC heap and cannot be pre-fetched at compile time.
/// \param stringID the string table index of the constant string
/// \param runtimeModule the runtime module of the current code block.
HermesValue externLoadConstStringMayAllocate(
    uint32_t stringID,
    RuntimeModule *runtimeModule);

/// An external call invoked by JIT compiled code to \return the JS type of a
/// given hermes register \p src.
HermesValue externTypeOf(Runtime *runtime, PinnedHermesValue *src);

/// An slow path invoked by JIT compiled code to call Operation::lessOp
CallResult<HermesValue>
slowPathLess(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An slow path invoked by JIT compiled code to call Operation::greaterOp
CallResult<HermesValue> slowPathGreater(
    Runtime *runtime,
    PinnedHermesValue *op1,
    PinnedHermesValue *op2);

/// An slow path invoked by JIT compiled code to call Operation::lessEqualOp
CallResult<HermesValue> slowPathLessEq(
    Runtime *runtime,
    PinnedHermesValue *op1,
    PinnedHermesValue *op2);

/// An slow path invoked by JIT compiled code to call Operation::greaterEqualOp
CallResult<HermesValue> slowPathGreaterEq(
    Runtime *runtime,
    PinnedHermesValue *op1,
    PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to call
/// Operation::abstractEqualityTest
CallResult<HermesValue> externAbstractEqualityTest(
    Runtime *runtime,
    PinnedHermesValue *op1,
    PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to call
/// JSObject::create
HermesValue externNewObject(Runtime *runtime);

/// An external call invoked by JIT compiled code to call
/// Callable::newObject
/// \param proto prototype of the object to be created
/// \param closure the 'this' object
CallResult<HermesValue> externCreateThis(
    Runtime *runtime,
    PinnedHermesValue *proto,
    PinnedHermesValue *closure);

/// An external call invoked by JIT compiled code to call
/// JSArray::create
/// \param size size/capacity hint of the array
CallResult<HermesValue> externNewArray(Runtime *runtime, uint32_t size);

/// An external call invoked by JIT compiled code to call
/// JSObject::defineOwnComputedPrimitive
/// \param obj the target to set a property in.
/// \param prop the property value to be set.
/// \param idVal the index of the property
ExecutionStatus externPutOwnByIndex(
    Runtime *runtime,
    PinnedHermesValue *obj,
    PinnedHermesValue *prop,
    uint32_t idVal);

/// An external call invoked by JIT compiled code to call
/// JSObject::defineNewOwnProperty
/// \param target the target to put a property in.
/// \param prop the property to be put.
/// \param sid the SymbolID of the property which must already exist in the map.
ExecutionStatus externPutNewOwnById(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *prop,
    uint32_t sid);

/// An slow path invoked by JIT compiled code to coerce \p thisVal assumed to
/// contain 'this' to an object
CallResult<HermesValue> slowPathCoerceThis(
    Runtime *runtime,
    PinnedHermesValue *thisVal);

/// An external call invoked by JIT compiled code to get a property by \p
/// nameVal from the object \p target
CallResult<HermesValue> externGetByVal(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *nameVal);

/// An external call invoked by JIT compiled code to set a property to the \p
/// value by the index \p nameVal in the object \p target
/// \param flags property access flags
ExecutionStatus externPutByVal(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *nameVal,
    PinnedHermesValue *value,
    PropOpFlags flags);

/// An external call invoked by JIT compiled code to delete a property by \p
/// nameVal from the object \p target
/// \param flags property access flags
CallResult<HermesValue> externDelByVal(
    Runtime *runtime,
    PinnedHermesValue *target,
    PinnedHermesValue *nameVal,
    PropOpFlags flags);

CallResult<HermesValue> externDelById(
    Runtime *runtime,
    PinnedHermesValue *target,
    uint32_t sid,
    PropOpFlags flags);

/// An external call invoked by JIT compiled code to store a value \p val to an
/// environment \p env, by the index slot number \p idx
void externStoreToEnvironment(
    PinnedHermesValue *env,
    uint32_t idx,
    PinnedHermesValue *val,
    Runtime *runtime);

/// An external call invoked by JIT compiled code to store a non-pointer value
/// \p val to an environment \p env, by the index slot number \p idx
void externStoreNPToEnvironment(
    PinnedHermesValue *env,
    uint32_t idx,
    PinnedHermesValue *val,
    Runtime *runtime);

/// An external call invoked by JIT compiled code to \return a value from an
/// environment \p env, by the index slot number \p idx
HermesValue externLoadFromEnvironment(PinnedHermesValue *env, uint32_t idx);

/// An external call invoked by JIT compiled code to do mod (op1 % op2)
CallResult<HermesValue>
externMod(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to do JS bitshift left (op1 <<
/// op2)
CallResult<HermesValue>
externLShift(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);
/// An external call invoked by JIT compiled code to do JS signed bitshift right
/// (op1 >> op2)
CallResult<HermesValue>
externRShift(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);
/// An external call invoked by JIT compiled code to do JS unsigned bitshift
/// right (op1 >>> op2)
CallResult<HermesValue>
externURshift(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to do JS bitwise AND (op1 &
/// op2)
CallResult<HermesValue>
externBitAnd(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to do JS bitwise OR (op1 |
/// op2)
CallResult<HermesValue>
externBitOr(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// An external call invoked by JIT compiled code to do JS bitwise XOR (op1 ^
/// op2)
CallResult<HermesValue>
externBitXor(Runtime *runtime, PinnedHermesValue *op1, PinnedHermesValue *op2);

/// \return the environment from \p numLevel levels up the stack.
/// 0 is the current environment, 1 is the caller's environment, etc
/// \param frame the current frame
HermesValue externGetEnvironment(
    Runtime *runtime,
    PinnedHermesValue *frame,
    uint32_t numLevel);

/// A wrapper to call Interpreter::createObjectFromBuffer, so that we could add
/// GC scope marker before the call.
CallResult<HermesValue> externNewObjectWithBuffer(
    Runtime *runtime,
    CodeBlock *curCodeBlock,
    uint32_t numLiterals,
    uint32_t keyBufferIndex,
    uint32_t valBufferIndex);

/// A wrapper to call Interpreter::createArrayFromBuffer, so that we could add
/// GC scope marker before the call.
CallResult<HermesValue> externNewArrayWithBuffer(
    Runtime *runtime,
    CodeBlock *curCodeBlock,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t bufferIndex);

/// A slow path invoked by JIT compiled code to do unary minus
/// \return -op1
CallResult<HermesValue> slowPathNegate(
    Runtime *runtime,
    PinnedHermesValue *op1);

/// An external call invoked by JIT compiled code to \return the next property
/// in the for..in iterator.
/// \param arrReg the array of properties
/// \param objReg the object containing the properties
/// \param iterReg the iterating index, it will be incremented by 1 every time
///         GetNextPName is called.
/// \param sizeReg the size of property list.
CallResult<HermesValue> externGetNextPName(
    Runtime *runtime,
    PinnedHermesValue *arrReg,
    PinnedHermesValue *objReg,
    PinnedHermesValue *iterReg,
    PinnedHermesValue *sizeReg);

/// An external call invoked by JIT compiled code to create an actual
/// 'arguments' array.
/// \param currentFrame the current frame in stack
/// \param isStrict is the current code block in strict mode
CallResult<HermesValue> externSlowPathReifyArguments(
    Runtime *runtime,
    PinnedHermesValue *currentFrame,
    bool isStrict);

CallResult<HermesValue> externSlowPathGetArgumentsPropByVal(
    Runtime *runtime,
    PinnedHermesValue *lazyReg,
    PinnedHermesValue *valueReg,
    PinnedHermesValue *currentFrame,
    bool isStrict);

/// A slow path invoked by JIT compiled code to do bitwise not
/// \return ~op
CallResult<HermesValue> externSlowPathBitNot(
    Runtime *runtime,
    PinnedHermesValue *op);

/// A slow path invoked by JIT compiled code to \return the arguments length of
/// \p obj
CallResult<HermesValue> slowPathGetArgumentsLength(
    Runtime *runtime,
    PinnedHermesValue *obj);

/// An external call invoked by JIT compiled code to \return if \p propName is
/// 'in' \p obj. (JS relational 'in')
CallResult<HermesValue> externIsIn(
    Runtime *runtime,
    PinnedHermesValue *propName,
    PinnedHermesValue *obj);

/// An external call invoked by JIT compiled code to \return if \p obj is in \p
/// constructor's prototype chain.
CallResult<HermesValue> externInstanceOf(
    Runtime *runtime,
    PinnedHermesValue *obj,
    PinnedHermesValue *constructor);

/// An external call invoked by JIT compiled code to create a regular
/// expression.
/// \param patternIdx the string index of the pattern.
/// \param flagsIdx the string index of the flags.
/// \param bytecodeIdx the regexp bytecode index in the regexp table.
/// \param codeBlock the current code block.
CallResult<HermesValue> externCreateRegExpMayAllocate(
    Runtime *runtime,
    uint32_t patternIdx,
    uint32_t flagsIdx,
    uint32_t bytecodeIdx,
    CodeBlock *codeBlock);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_EXTERNALCALLS_H
