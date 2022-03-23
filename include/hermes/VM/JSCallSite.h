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
/// inside a JSError object. Hermes' CallSite objects do not have their own
/// JSObject subclass, cell kind, metadata builder etc. Rather, CallSites are
/// plain JSObjects with internal CallSite properties. The implementation will
/// ensure that any receivers passed to the CallSite methods below are proper
/// CallSite objects (e.g., have all CallSite internal properties defined as
/// expected) and raise a TypeError in case it isn't.
namespace JSCallSite {
/// Create a CallSite object that references the frame at index
/// \p stackFrameIndex in the stack trace captured in \p error.
/// \return a JSObject that is a CallSite object -- i.e., it has the CallSite
/// internal properties.
CallResult<HermesValue>
create(Runtime &runtime, Handle<JSError> error, uint32_t stackFrameIndex);

/// \return the function name of this call site, or null if it is anonymous or
/// unavailable.
CallResult<HermesValue> getFunctionName(Runtime &runtime, Handle<> selfHandle);

/// \return the file name of this call site, or null if it is
/// unavailable.
CallResult<HermesValue> getFileName(Runtime &runtime, Handle<> selfHandle);

/// \return the 1-based line number of this call site, or null if it is
/// unavailable.
CallResult<HermesValue> getLineNumber(Runtime &runtime, Handle<> selfHandle);

/// \return the 1-based column number of this call site, or null if it is
/// unavailable.
CallResult<HermesValue> getColumnNumber(Runtime &runtime, Handle<> selfHandle);

/// This method is Hermes-specific.
/// \return the 0-based virtual offset of this call site, or null if it is
/// unavailable.
CallResult<HermesValue> getBytecodeAddress(
    Runtime &runtime,
    Handle<> selfHandle);

/// \return true if this call site is a native function, false otherwise.
CallResult<HermesValue> isNative(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c undefined.
CallResult<HermesValue> getThis(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> getTypeName(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c undefined.
CallResult<HermesValue> getFunction(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> getMethodName(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> getEvalOrigin(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> isToplevel(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> isEval(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> isConstructor(Runtime &runtime, Handle<> selfHandle);

/// Is this an async call (i.e. await or Promise.all())?
/// \return \c false since Hermes has no native support for async calls.
CallResult<HermesValue> isAsync(Runtime &runtime, Handle<> selfHandle);

/// Is this an async call to Promise.all()?
/// \return \c false since Hermes has no native support for async calls.
CallResult<HermesValue> isPromiseAll(Runtime &runtime, Handle<> selfHandle);

/// Not implemented.
/// \return \c null.
CallResult<HermesValue> getPromiseIndex(Runtime &runtime, Handle<> selfHandle);
}; // namespace JSCallSite

} // namespace vm
} // namespace hermes
#endif
