/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#ifdef HERMESVM_SERIALIZE
  JSError(Deserializer &d);

  friend void ErrorSerialize(Serializer &s, const GCCell *cell);
#endif

  using Super = JSObject;
  static const ObjectVTable vt;
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ErrorKind;
  }

  /// Create an Error Object.
  static PseudoHandle<JSError> create(
      Runtime *runtime,
      Handle<JSObject> prototype);
  /// Create an uncatchable Error Object. If this object is thrown, no catch
  /// handlers or finally handlers are called.
  /// NOTE: This should be used only in very specific circumstances where it is
  /// impossible or undesirable to continue running the VM. The error should be
  /// considered a fatal abort, but one that cleans up internal VM resources.
  static PseudoHandle<JSError> createUncatchable(
      Runtime *runtime,
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
      Runtime *runtime,
      bool skipTopFrame = false,
      CodeBlock *codeBlock = nullptr,
      const Inst *ip = nullptr);

  /// Define the stack setter and getter, for later stack trace creation.
  static ExecutionStatus setupStack(
      Handle<JSError> selfHandle,
      Runtime *runtime);

  /// Set the message property.
  static ExecutionStatus
  setMessage(Handle<JSError> selfHandle, Runtime *runtime, Handle<> message);

  /// \return a pointer to the stack trace, or NULL if the stack trace has been
  /// cleared or not been set.
  const StackTrace *getStackTrace() const {
    return stacktrace_.get();
  }

  bool catchable() const {
    return catchable_;
  }

  /// Upon called, construct the stacktrace string based on
  /// the value of stacktrace_, and reset the stack property to the
  /// stacktrace string.
  friend CallResult<HermesValue>
  errorStackGetter(void *, Runtime *runtime, NativeArgs args);

  /// This is called when someone manually set the stack property to
  /// an error object, which should happen rarely. It destroys the
  /// stack access and replace it with a regular property.
  friend CallResult<HermesValue>
  errorStackSetter(void *, Runtime *runtime, NativeArgs args);

  JSError(
      Runtime *runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      bool catchable)
      : JSObject(runtime, &vt.base, *parent, *clazz), catchable_{catchable} {}

 private:
  friend void ErrorBuildMeta(const GCCell *cell, Metadata::Builder &mb);
  static void _finalizeImpl(GCCell *cell, GC *gc);
  static size_t _mallocSizeImpl(GCCell *cell);

  static PseudoHandle<JSError>
  create(Runtime *runtime, Handle<JSObject> prototype, bool catchable);

  /// A pointer to the stack trace, or nullptr if it has not been set.
  StackTracePtr stacktrace_;

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
  static ExecutionStatus constructStackTraceString(
      Runtime *runtime,
      Handle<JSError> selfHandle,
      SmallU16String<32> &stack);

  /// Append the name of the function at \p index to the given \p str.
  /// \return true on success, false if the name was missing, invalid or empty.
  static bool appendFunctionNameAtIndex(
      Runtime *runtime,
      Handle<JSError> selfHandle,
      size_t index,
      llvh::SmallVectorImpl<char16_t> &str);
};

} // namespace vm
} // namespace hermes
#endif
