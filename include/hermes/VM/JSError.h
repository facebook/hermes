/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSERROR_H
#define HERMES_VM_JSERROR_H

#include "hermes/VM/JSObject.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/SmallXString.h"

namespace hermes {
namespace vm {

/// StackTraceInfo holds information of an entry in the stacktrace upon
/// exceptions. We only need to store the CodeBlock and bytecode offset
/// to obtain the full function name/file name/position later when we
/// need to generate the stacktrace string.
/// We store the domains for the CodeBlocks within the JSError to ensure that
/// the CodeBlocks never get freed, and thus every StackTraceInfo is still
/// valid.
struct StackTraceInfo {
  /// The code block of the function.
  CodeBlock *codeBlock;

  /// The bytecode offset where exception was thrown.
  uint32_t bytecodeOffset;

  StackTraceInfo(CodeBlock *codeBlock, uint32_t bytecodeOffset)
      : codeBlock(codeBlock), bytecodeOffset(bytecodeOffset) {}

  StackTraceInfo(const StackTraceInfo &) = default;

  StackTraceInfo(StackTraceInfo &&) = default;
};
using StackTrace = std::vector<StackTraceInfo>;
using StackTracePtr = std::unique_ptr<StackTrace>;

/// Error Object.
class JSError final : public JSObject {
 public:
  using Super = JSObject;
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSErrorKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSErrorKind;
  }

  /// Create an Error Object.
  static PseudoHandle<JSError> create(
      Runtime &runtime,
      Handle<JSObject> prototype);
  /// Create an uncatchable Error Object. If this object is thrown, no catch
  /// handlers or finally handlers are called.
  /// NOTE: This should be used only in very specific circumstances where it is
  /// impossible or undesirable to continue running the VM. The error should be
  /// considered a fatal abort, but one that cleans up internal VM resources.
  static PseudoHandle<JSError> createUncatchable(
      Runtime &runtime,
      Handle<JSObject> prototype);

  /// If the stack trace is not set, attempt to record it by walking the runtime
  /// stack. If the top call frame indicates a JS callee, but the codeBlock and
  /// ip are not supplied, return without doing anything. This handles the case
  /// when an exception is thrown from within the current code block.
  ///
  /// \param skipTopFrame don't record the topmost frame. This is used when
  ///   we want to skip the Error() constructor itself.
  /// \param codeBlock optional current CodeBlock.
  /// \param ip if \c codeBlock is not \c nullptr, the instruction in the
  ///   current CodeBlock.
  static ExecutionStatus recordStackTrace(
      Handle<JSError> selfHandle,
      Runtime &runtime,
      bool skipTopFrame = false,
      CodeBlock *codeBlock = nullptr,
      const Inst *ip = nullptr);

  /// Define the stack setter and getter, for later stack trace creation.
  /// May be used on JSError instances, or on any JSObject that has a
  /// \c CapturedError property.
  static ExecutionStatus setupStack(
      Handle<JSObject> selfHandle,
      Runtime &runtime);

  /// Set the message property.
  static ExecutionStatus
  setMessage(Handle<JSError> selfHandle, Runtime &runtime, Handle<> message);

  /// \return a pointer to the stack trace, or NULL if the stack trace has been
  /// cleared or not been set.
  const StackTrace *getStackTrace() const {
    return stacktrace_.get();
  }

  bool catchable() const {
    return catchable_;
  }

  /// When called, construct the stacktrace string based on the value of
  /// stacktrace_, and reset the stack property to the stacktrace string.
  friend CallResult<HermesValue>
  errorStackGetter(void *, Runtime &runtime, NativeArgs args);

  /// This is called when someone manually set the stack property to
  /// an error object, which should happen rarely. It destroys the
  /// stack access and replace it with a regular property.
  friend CallResult<HermesValue>
  errorStackSetter(void *, Runtime &runtime, NativeArgs args);

  /// Pop frames from the stack trace until we encounter a frame attributed to
  /// \p callable, and pop that frame too. No frames are skipped if a matching
  /// frame isn't found.
  /// \pre \p selfHandle's stacktrace_ is non-null.
  static void popFramesUntilInclusive(
      Runtime &runtime,
      Handle<JSError> selfHandle,
      Handle<Callable> callableHandle);

  /// Given a codeblock and opcode offset, \returns the debug information.
  static OptValue<hbc::DebugSourceLocation> getDebugInfo(
      CodeBlock *codeBlock,
      uint32_t bytecodeOffset);

  /// \return the name of the function at \p index.
  static Handle<StringPrimitive> getFunctionNameAtIndex(
      Runtime &runtime,
      Handle<JSError> selfHandle,
      size_t index);

  JSError(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      bool catchable)
      : JSObject(runtime, *parent, *clazz), catchable_{catchable} {}

 private:
  friend void JSErrorBuildMeta(const GCCell *cell, Metadata::Builder &mb);
  static void _finalizeImpl(GCCell *cell, GC &gc);
  static size_t _mallocSizeImpl(GCCell *cell);

  static PseudoHandle<JSError>
  create(Runtime &runtime, Handle<JSObject> prototype, bool catchable);

  /// A pointer to the stack trace, or nullptr if it has not been set.
  StackTracePtr stacktrace_;

  /// The index of the stack frame to start from when converting the stack
  /// trace to a string or otherwise exposing it to user code.
  /// This can only be changed while stacktrace_ is non-null.
  size_t firstExposedFrameIndex_{0};

  /// A list of Domains which are referenced by the stacktrace_.
  GCPointer<ArrayStorageSmall> domains_;

  /// If not null, an array of function names as the 'name' property of the
  /// Callables. This is parallel to the stack trace array.
  GCPointer<PropStorage> funcNames_;

  /// If true, JS catch and finally blocks will be run after this error is
  /// thrown. Else, there will be no more JS executed after this error is
  /// thrown. This is most useful in scenarios where an abrupt abort is desired,
  /// or when the VM is not in a state where it can execute JS usefully, for
  /// example if an OOM has occurred.
  bool catchable_{true};

  /// Construct the stacktrace string, append to \p stack.
  /// If the construction of the stack throws an uncatchable error, this
  /// function returns prematurely.
  ///
  /// \param selfHandle supplies the call stack for the trace.
  /// \param targetHandle supplies the error name and message for the trace.
  ///
  /// In the `(new Error).stack` case, selfHandle and targetHandle will both
  /// refer to the same object.
  /// In the `target = {}; Error.captureErrorStack(target); target.stack` case,
  /// selfHandle will be the value of `target`'s [[CapturedError]] slot.
  static ExecutionStatus constructStackTraceString(
      Runtime &runtime,
      Handle<JSError> selfHandle,
      Handle<JSObject> targetHandle,
      SmallU16String<32> &stack);

  /// Construct the callSites array for Error.prepareStackTrace.
  static CallResult<HermesValue> constructCallSitesArray(
      Runtime &runtime,
      Handle<JSError> selfHandle);

  /// Append the name of the function at \p index to the given \p str.
  /// \return true on success, false if the name was missing, invalid or empty.
  static bool appendFunctionNameAtIndex(
      Runtime &runtime,
      Handle<JSError> selfHandle,
      size_t index,
      llvh::SmallVectorImpl<char16_t> &str);

  /// Given an object \p targetHandle:
  /// 1. If its [[CapturedError]] slot has a non-null handle, return it as a
  ///    JSError.
  /// 2. Otherwise, return \t targetHandle cast to JSError.
  /// Throws if any cast or property access fails.
  static CallResult<Handle<JSError>> getErrorFromStackTarget_RJS(
      Runtime &runtime,
      Handle<JSObject> targetHandle);
};

} // namespace vm
} // namespace hermes
#endif
