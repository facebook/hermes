/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSCallSite.h"

#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/Support/OptValue.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/RuntimeModule-inline.h"

namespace hermes {
namespace vm {
namespace {
/// A helper class with all the fields stored in a CallSite. It is populated by
/// callSiteFromSelfHandle below.
struct JSCallSiteInfo {
  Handle<JSError> error;
  size_t stackFrameIndex;
};

/// Raises a TypeError when an incompatible receiver is used for any of the
/// CallSite methods below.
static ExecutionStatus raiseIncompatibleReceiverError(Runtime &runtime) {
  return runtime.raiseTypeError(
      "CallSite method called on an incompatible receiver");
}

/// Looks up property \p iProp in \p selfHandle raising a type error if one
/// isn't found.
/// \return A pseudo-handle to \p iProp.
static CallResult<PseudoHandle<>> getCallSiteProp(
    Runtime &runtime,
    Handle<JSObject> selfHandle,
    Predefined::IProp iProp) {
  auto pof = PropOpFlags().plusMustExist().plusThrowOnError();
  CallResult<PseudoHandle<>> res = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(iProp), pof);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    runtime.clearThrownValue();
    return raiseIncompatibleReceiverError(runtime);
  }
  return res;
}

/// Extracts the CallSite information from \p selfHandle. Raises TypeError if
/// any CallSite property isn't found.
static CallResult<JSCallSiteInfo> callSiteFromSelfHandle(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto selfObjHandle = Handle<JSObject>::dyn_vmcast(selfHandle);
  if (LLVM_UNLIKELY(!selfObjHandle)) {
    return raiseIncompatibleReceiverError(runtime);
  }

  auto errorRes = getCallSiteProp(
      runtime, selfObjHandle, Predefined::InternalPropertyCallSiteError);
  if (LLVM_UNLIKELY(errorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto error = runtime.makeHandle<JSError>(std::move(*errorRes));

  auto frameIndexRes = getCallSiteProp(
      runtime,
      selfObjHandle,
      Predefined::InternalPropertyCallSiteStackFrameIndex);
  if (LLVM_UNLIKELY(frameIndexRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const size_t frameIndex = frameIndexRes->getHermesValue().getNumber();

  return JSCallSiteInfo{std::move(error), frameIndex};
}

/// \return a reference to the stack frame into which this CallSite object is
/// a view.
CallResult<const StackTraceInfo *> getStackTraceInfo(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto callSiteRes = callSiteFromSelfHandle(runtime, selfHandle);
  if (LLVM_UNLIKELY(callSiteRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const StackTrace *stacktrace = callSiteRes->error->getStackTrace();
  assert(
      stacktrace &&
      "The Error associated with this CallSite has already released its "
      "stack trace vector");

  // Returning a pointer to stacktrace elements is safe because:
  //   1. stacktrace's ownership is managed (indirectly) by selfHandle (i.e.,
  //      the CallSite object); and
  //   2. stacktrace is not modified after it is created.
  return &stacktrace->at(callSiteRes->stackFrameIndex);
}

Handle<JSObject> createJSCallSite(Runtime &runtime) {
  auto parentHandle = Handle<JSObject>::vmcast(&runtime.callSitePrototype);
  auto *cell = runtime.makeAFixed<JSObject>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, JSObject::numOverlapSlots<JSObject>()),
      GCPointerBase::NoBarriers());
  return JSObjectInit::initToHandle(runtime, cell);
}
} // namespace

CallResult<HermesValue> JSCallSite::create(
    Runtime &runtime,
    Handle<JSError> errorHandle,
    uint32_t stackFrameIndex) {
  Handle<JSObject> selfHandle = createJSCallSite(runtime);

  assert(
      errorHandle->getStackTrace() &&
      "Error passed to CallSite must have a stack trace");
  assert(
      stackFrameIndex < errorHandle->getStackTrace()->size() &&
      "Stack frame index out of bounds");

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  auto addCallSiteProp = [&](Predefined::IProp iProp, Handle<> value) {
    auto res = JSObject::defineOwnProperty(
        selfHandle,
        runtime,
        Predefined::getSymbolID(iProp),
        dpf,
        std::move(value));
    assert(
        res != ExecutionStatus::EXCEPTION && *res &&
        "defineOwnProperty() failed");
    return res;
  };

  auto res = addCallSiteProp(
      Predefined::InternalPropertyCallSiteError, std::move(errorHandle));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto frameIndexHV = HermesValue::encodeNumberValue(stackFrameIndex);
  res = addCallSiteProp(
      Predefined::InternalPropertyCallSiteStackFrameIndex,
      runtime.makeHandle(std::move(frameIndexHV)));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  assert(
      callSiteFromSelfHandle(runtime, selfHandle) !=
          ExecutionStatus::EXCEPTION &&
      "JSCallSite should **be** a CallSite by now");
  return selfHandle.getHermesValue();
}

CallResult<HermesValue> JSCallSite::getFunctionName(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto callSiteRes = callSiteFromSelfHandle(runtime, selfHandle);
  if (LLVM_UNLIKELY(callSiteRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto functionName = JSError::getFunctionNameAtIndex(
      runtime, callSiteRes->error, callSiteRes->stackFrameIndex);
  return functionName ? functionName.getHermesValue()
                      : HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getFileName(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto stiRes = getStackTraceInfo(runtime, selfHandle);
  if (LLVM_UNLIKELY(stiRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const StackTraceInfo *sti = *stiRes;
  if (sti->codeBlock) {
    OptValue<hbc::DebugSourceLocation> location =
        JSError::getDebugInfo(sti->codeBlock, sti->bytecodeOffset);

    RuntimeModule *runtimeModule = sti->codeBlock->getRuntimeModule();
    auto makeUTF8Ref = [](llvh::StringRef ref) {
      const uint8_t *utf8 = reinterpret_cast<const uint8_t *>(ref.data());
      return llvh::makeArrayRef(utf8, ref.size());
    };

    if (location) {
      auto debugInfo = runtimeModule->getBytecode()->getDebugInfo();

      std::string utf8Storage;
      llvh::StringRef fileName = hbc::getStringFromEntry(
          debugInfo->getFilenameTable()[location->filenameId],
          debugInfo->getFilenameStorage(),
          utf8Storage);
      return StringPrimitive::createEfficient(runtime, makeUTF8Ref(fileName));
    } else {
      llvh::StringRef sourceURL = runtimeModule->getSourceURL();
      if (!sourceURL.empty()) {
        return StringPrimitive::createEfficient(
            runtime, makeUTF8Ref(sourceURL));
      }
    }
  }
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getLineNumber(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto stiRes = getStackTraceInfo(runtime, selfHandle);
  if (LLVM_UNLIKELY(stiRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const StackTraceInfo *sti = *stiRes;
  if (sti->codeBlock) {
    OptValue<hbc::DebugSourceLocation> location =
        JSError::getDebugInfo(sti->codeBlock, sti->bytecodeOffset);
    if (location) {
      return HermesValue::encodeNumberValue(location->line);
    } else {
      // Add 1 to the CJSModuleOffset to account for 1-based indexing of
      // symbolication tools.
      return HermesValue::encodeNumberValue(
          sti->codeBlock->getRuntimeModule()->getBytecode()->getSegmentID() +
          1);
    }
  }
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getColumnNumber(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto stiRes = getStackTraceInfo(runtime, selfHandle);
  if (LLVM_UNLIKELY(stiRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const StackTraceInfo *sti = *stiRes;
  if (sti->codeBlock) {
    OptValue<hbc::DebugSourceLocation> location =
        JSError::getDebugInfo(sti->codeBlock, sti->bytecodeOffset);
    if (location) {
      return HermesValue::encodeNumberValue(location->column);
    }
  }
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getBytecodeAddress(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto stiRes = getStackTraceInfo(runtime, selfHandle);
  if (LLVM_UNLIKELY(stiRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const StackTraceInfo *sti = *stiRes;
  if (sti->codeBlock) {
    return HermesValue::encodeNumberValue(
        sti->bytecodeOffset + sti->codeBlock->getVirtualOffset());
  }
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::isNative(
    Runtime &runtime,
    Handle<> selfHandle) {
  auto stiRes = getStackTraceInfo(runtime, selfHandle);
  if (LLVM_UNLIKELY(stiRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  const StackTraceInfo *sti = *stiRes;
  return HermesValue::encodeBoolValue(!sti->codeBlock);
}

namespace {
/// Ensures that \p selfHandle is a CallSite (raising a TypeError if not) before
/// returning the default value.
CallResult<HermesValue> HandleUnimplemented(
    Runtime &runtime,
    Handle<> selfHandle,
    HermesValue (*createReturnHV)()) {
  if (LLVM_UNLIKELY(
          callSiteFromSelfHandle(runtime, selfHandle) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return (*createReturnHV)();
}
} // namespace

CallResult<HermesValue> JSCallSite::getThis(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeUndefinedValue);
}

CallResult<HermesValue> JSCallSite::getTypeName(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

CallResult<HermesValue> JSCallSite::getFunction(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeUndefinedValue);
}

CallResult<HermesValue> JSCallSite::getMethodName(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

CallResult<HermesValue> JSCallSite::getEvalOrigin(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

CallResult<HermesValue> JSCallSite::isToplevel(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

CallResult<HermesValue> JSCallSite::isEval(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

CallResult<HermesValue> JSCallSite::isConstructor(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

CallResult<HermesValue> JSCallSite::isAsync(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, [] { return HermesValue::encodeBoolValue(false); });
}

CallResult<HermesValue> JSCallSite::isPromiseAll(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, [] { return HermesValue::encodeBoolValue(false); });
}

CallResult<HermesValue> JSCallSite::getPromiseIndex(
    Runtime &runtime,
    Handle<> selfHandle) {
  return HandleUnimplemented(
      runtime, selfHandle, &HermesValue::encodeNullValue);
}

} // namespace vm
} // namespace hermes
