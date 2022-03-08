/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INTERPRETER_H
#define HERMES_VM_INTERPRETER_H
#include <cstdint>

#include "hermes/VM/Runtime.h"

class CodeBlock;

namespace hermes {
namespace vm {
/// This class is a convenience wrapper for the interpreter implementation that
/// needs access to the private fields of Runtime, but doesn't belong in
/// Runtime.
class Interpreter {
 public:
  /// Allocate a generator for the specified function and the specified
  /// environment. \param funcIndex function index in the global function table.
  static CallResult<PseudoHandle<JSGenerator>> createGenerator_RJS(
      Runtime &runtime,
      RuntimeModule *runtimeModule,
      unsigned funcIndex,
      Handle<Environment> envHandle,
      NativeArgs args);

  /// Suspend the generator function and yield to the caller.
  /// \param resumeIP Is the IP where the generator should resume from when it
  ///   is resumed.
  static void saveGenerator(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const Inst *resumeIP);

  /// Slow path for ReifyArguments resReg, lazyReg
  /// It assumes that the fast path has handled the case when 'lazyReg' is
  /// already initialized. It creates a new 'arguments' object and populates it
  /// with the argument values.
  static CallResult<Handle<Arguments>> reifyArgumentsSlowPath(
      Runtime &runtime,
      Handle<Callable> curFunction,
      bool strictMode);

  /// Slow path for GetArgumentsPropByVal resReg, propNameReg, lazyReg.
  ///
  /// It assumes that the "fast path" has already taken care of the case when
  /// the 'lazyReg' is still uninitialized and 'propNameReg' is a valid integer
  /// index less than 'argCount'. So we arrive here when either of these is
  /// true:
  /// - 'lazyReg' is initialized.
  /// - index is >= argCount
  /// - index is not an integer
  /// In the first case we simply perform a normal property get. In the latter
  /// we ultimately need to reify the arguments object, but we try to avoid
  /// doing that by:
  /// - checking if the property name is "length". In that case we can just
  ///   return the value.
  /// - checking if the property name in the prototype is not an accessor. In
  /// that
  ///   case we can also just return the value read from the prototype.
  /// Only if all else fails, we reify.
  /// The FRAME in question is obtained from \p runtime, and the registers
  /// \p lazyReg and \p valueReg are passed directly to make this function
  /// easier to use outside the interpeter.
  static CallResult<PseudoHandle<>> getArgumentsPropByValSlowPath_RJS(
      Runtime &runtime,
      PinnedHermesValue *lazyReg,
      PinnedHermesValue *valueReg,
      Handle<Callable> curFunction,
      bool strictMode);

  /// Implement the slow path of OpCode::Call/CallLong/Construct/ConstructLong.
  /// The callee frame must have been initialized already and the fast path
  /// (calling a \c JSFunction) must have been handled.
  /// This handles the rest of the cases (native function, bound funcation, and
  /// not even a function).
  /// \param callTarget the register containing the function object
  /// \return ExecutionStatus::EXCEPTION if the call threw.
  static CallResult<PseudoHandle<>> handleCallSlowPath(
      Runtime &runtime,
      PinnedHermesValue *callTarget);

  /// Fast path to get primitive value \p base's own properties by name \p id
  /// without boxing.
  /// Primitive own properties are properties fetching values from primitive
  /// value itself.
  /// Currently the only primitive own property is String.prototype.length.
  /// If the fast path property does not exist, return Empty.
  static PseudoHandle<>
  tryGetPrimitiveOwnPropertyById(Runtime &runtime, Handle<> base, SymbolID id);

  /// Implement OpCode::GetById/TryGetById when the base is not an object.
  static CallResult<PseudoHandle<>>
  getByIdTransient_RJS(Runtime &runtime, Handle<> base, SymbolID id);

  /// Fast path for getByValTransient() -- avoid boxing for \p base if it is
  /// string primitive and \p nameHandle is an array index.
  /// If the property does not exist, return Empty.
  static PseudoHandle<>
  getByValTransientFast(Runtime &runtime, Handle<> base, Handle<> nameHandle);

  /// Implement OpCode::GetByVal when the base is not an object.
  static CallResult<PseudoHandle<>>
  getByValTransient_RJS(Runtime &runtime, Handle<> base, Handle<> name);

  /// Implement OpCode::PutById/TryPutById when the base is not an object.
  static ExecutionStatus putByIdTransient_RJS(
      Runtime &runtime,
      Handle<> base,
      SymbolID id,
      Handle<> value,
      bool strictMode);

  /// Implement OpCode::PutByVal when the base is not an object.
  static ExecutionStatus putByValTransient_RJS(
      Runtime &runtime,
      Handle<> base,
      Handle<> name,
      Handle<> value,
      bool strictMode);

  /// Inlining this function is forbidden because it stores label values in a
  /// local static variable. Due to a bug in LLVM, it may sometimes be inlined
  /// anyway, so explicitly mark it as noinline.
  template <bool SingleStep, bool EnableCrashTrace>
  LLVM_ATTRIBUTE_NOINLINE static CallResult<HermesValue> interpretFunction(
      Runtime &runtime,
      InterpreterState &state);

  /// Populates an object with literal values from the object buffer.
  /// \param numLiterals the amount of literals to read from the buffer.
  /// \param keyBufferIndex the first element of the key buffer to read.
  /// \param valBufferIndex the first element of the val buffer to read.
  /// \return ExecutionStatus::EXCEPTION if the property definitions throw.
  static CallResult<PseudoHandle<>> createObjectFromBuffer(
      Runtime &runtime,
      CodeBlock *curCodeBlock,
      unsigned numLiterals,
      unsigned keyBufferIndex,
      unsigned valBufferIndex);

  /// Populates an array with literal values from the array buffer.
  /// \param numLiterals the amount of literals to read from the buffer.
  /// \param bufferIndex the first element of the buffer to read.
  /// \return ExecutionStatus::EXCEPTION if the property definitions throw.
  static CallResult<PseudoHandle<>> createArrayFromBuffer(
      Runtime &runtime,
      CodeBlock *curCodeBlock,
      unsigned numElements,
      unsigned numLiterals,
      unsigned bufferIndex);

#ifdef HERMES_ENABLE_DEBUGGER
  /// Wrapper around runDebugger() that reapplies the interpreter state.
  /// Constructs an interpreter state from the given \p codeBlock and \p ip.
  /// It then invokes the debugger and returns the new code block, and offset by
  /// reference, and updates frameRegs to its new value. Note this function is
  /// inline to allow the compiler to verify that the parameters do not escape,
  /// which might otherwise prevent them from being promoted to registers.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  static inline ExecutionStatus runDebuggerUpdatingState(
      Debugger::RunReason reason,
      Runtime &runtime,
      CodeBlock *&codeBlock,
      const Inst *&ip,
      PinnedHermesValue *&frameRegs) {
    // Hack: if we are already debugging, do nothing. TODO: in the event that we
    // are already debugging and we get an async debugger request, abort the
    // current debugging command (e.g. eval something infinite).
    if (runtime.debugger_.isDebugging())
      return ExecutionStatus::RETURNED;
    uint32_t offset = codeBlock->getOffsetOf(ip);
    InterpreterState state(codeBlock, offset);
    ExecutionStatus status = runtime.debugger_.runDebugger(reason, state);
    codeBlock = state.codeBlock;
    ip = state.codeBlock->getOffsetPtr(state.offset);
    frameRegs = &runtime.currentFrame_.getFirstLocalRef();
    return status;
  }
#endif

  //===========================================================================
  // Out-of-line implementations of entire instructions.

  /// Partial implementation of ES6 18.2.1.1
  /// `PerformEval(x, evalRealm, strictCaller=true, direct=true)`.
  /// The difference is that we don't support actual lexical scope, of course.
  static ExecutionStatus caseDirectEval(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const inst::Inst *ip);

  static ExecutionStatus casePutOwnByVal(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const inst::Inst *ip);

  static ExecutionStatus casePutOwnGetterSetterByVal(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const inst::Inst *ip);

  static ExecutionStatus caseIteratorBegin(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const inst::Inst *ip);
  static ExecutionStatus caseIteratorNext(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const inst::Inst *ip);

  static ExecutionStatus caseGetPNameList(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      const Inst *ip);

  /// Evaluate callBuiltin and store the result in the register stack. it must
  /// must be invoked with CallBuiltin or CallBuiltinLong. \p op3 contains the
  /// value of operand3, which is the only difference in encoding between the
  /// two.
  static ExecutionStatus implCallBuiltin(
      Runtime &runtime,
      PinnedHermesValue *frameRegs,
      CodeBlock *curCodeBlock,
      uint32_t op3);
};

#ifndef NDEBUG
/// A tag used to instruct the output stream to dump more details about the
/// HermesValue, like the length of the string, etc.
struct DumpHermesValue {
  const HermesValue hv;
  DumpHermesValue(HermesValue hv) : hv(hv) {}
};

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, DumpHermesValue dhv);

/// Dump the arguments from a callee frame.
void dumpCallArguments(
    llvh::raw_ostream &OS,
    Runtime &runtime,
    StackFramePtr calleeFrame);

#endif

} // namespace vm
} // namespace hermes

#endif
