/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSCALLSITE_H
#define HERMES_VM_JSCALLSITE_H

#include "hermes/VM/JSError.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// A CallSite object exposes a read-only view of a particular stack frame
/// inside a JSError object.
class JSCallSite final : public JSObject {
 public:
  using Super = JSObject;
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSCallSiteKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSCallSiteKind;
  }

  /// Create a CallSite object that references the frame at index
  /// \p stackFrameIndex in the stack trace captured in \p error.
  /// \return a JSObject that is a CallSite object -- i.e., it has the CallSite
  /// internal properties.
  static CallResult<HermesValue>
  create(Runtime &runtime, Handle<JSError> error, uint32_t stackFrameIndex);

  /// \return the function name of this call site, or null if it is anonymous or
  /// unavailable.
  static CallResult<HermesValue> getFunctionName(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// \return the file name of this call site, or null if it is
  /// unavailable.
  static CallResult<HermesValue> getFileName(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// \return the 1-based line number of this call site, or null if it is
  /// unavailable.
  static CallResult<HermesValue> getLineNumber(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// \return the 1-based column number of this call site, or null if it is
  /// unavailable.
  static CallResult<HermesValue> getColumnNumber(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// This method is Hermes-specific.
  /// \return the 0-based virtual offset of this call site, or null if it is
  /// unavailable.
  static CallResult<HermesValue> getBytecodeAddress(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// \return true if this call site is a native function, false otherwise.
  static CallResult<HermesValue> isNative(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c undefined.
  static CallResult<HermesValue> getThis(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> getTypeName(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c undefined.
  static CallResult<HermesValue> getFunction(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> getMethodName(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> getEvalOrigin(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> isToplevel(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> isEval(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> isConstructor(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Is this an async call (i.e. await or Promise.all())?
  /// \return \c false since Hermes has no native support for async calls.
  static CallResult<HermesValue> isAsync(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Is this an async call to Promise.all()?
  /// \return \c false since Hermes has no native support for async calls.
  static CallResult<HermesValue> isPromiseAll(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  /// Not implemented.
  /// \return \c null.
  static CallResult<HermesValue> getPromiseIndex(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  JSCallSite(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<JSError> error,
      size_t stackFrameIndex);

 private:
  friend void JSCallSiteBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// \return the StrackTraceInfo related to this JSCallSite.
  static const StackTraceInfo *getStackTraceInfo(
      Runtime &runtime,
      Handle<JSCallSite> selfHandle);

  GCPointer<JSError> error_;
  size_t stackFrameIndex_;
};

} // namespace vm
} // namespace hermes
#endif
