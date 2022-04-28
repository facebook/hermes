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
const ObjectVTable JSCallSite::vt{
    VTable(
        CellKind::JSCallSiteKind,
        cellSize<JSCallSite>(),
        nullptr,
        nullptr,
        nullptr),
    JSCallSite::_getOwnIndexedRangeImpl,
    JSCallSite::_haveOwnIndexedImpl,
    JSCallSite::_getOwnIndexedPropertyFlagsImpl,
    JSCallSite::_getOwnIndexedImpl,
    JSCallSite::_setOwnIndexedImpl,
    JSCallSite::_deleteOwnIndexedImpl,
    JSCallSite::_checkAllOwnIndexedImpl,
};

void JSCallSiteBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSCallSite::numOverlapSlots<JSCallSite>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSCallSite *>(cell);
  mb.setVTable(&JSCallSite::vt);
  mb.addField("error", &self->error_);
}

JSCallSite::JSCallSite(
    Runtime &runtime,
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz,
    Handle<JSError> error,
    size_t stackFrameIndex)
    : JSObject(runtime, *parent, *clazz),
      error_(runtime, *error, runtime.getHeap()),
      stackFrameIndex_(stackFrameIndex) {
  assert(
      error_.getNonNull(runtime)->getStackTrace() &&
      "Error passed to CallSite must have a stack trace");
  assert(
      stackFrameIndex < error_.getNonNull(runtime)->getStackTrace()->size() &&
      "Stack frame index out of bounds");
}

CallResult<HermesValue> JSCallSite::create(
    Runtime &runtime,
    Handle<JSError> errorHandle,
    uint32_t stackFrameIndex) {
  auto jsCallSiteProto = Handle<JSObject>::vmcast(&runtime.callSitePrototype);

  auto *jsCallSite = runtime.makeAFixed<JSCallSite>(
      runtime,
      jsCallSiteProto,
      runtime.getHiddenClassForPrototype(
          *jsCallSiteProto, numOverlapSlots<JSCallSite>()),
      errorHandle,
      stackFrameIndex);

  return JSObjectInit::initToHermesValue(runtime, jsCallSite);
}

CallResult<HermesValue> JSCallSite::getFunctionName(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  Handle<JSError> error = runtime.makeHandle(selfHandle->error_);

  auto functionName = JSError::getFunctionNameAtIndex(
      runtime, error, selfHandle->stackFrameIndex_);
  return functionName ? functionName.getHermesValue()
                      : HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getFileName(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  const StackTraceInfo *sti = getStackTraceInfo(runtime, selfHandle);
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
    Handle<JSCallSite> selfHandle) {
  const StackTraceInfo *sti = getStackTraceInfo(runtime, selfHandle);
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
    Handle<JSCallSite> selfHandle) {
  const StackTraceInfo *sti = getStackTraceInfo(runtime, selfHandle);
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
    Handle<JSCallSite> selfHandle) {
  const StackTraceInfo *sti = getStackTraceInfo(runtime, selfHandle);
  if (sti->codeBlock) {
    return HermesValue::encodeNumberValue(
        sti->bytecodeOffset + sti->codeBlock->getVirtualOffset());
  }
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::isNative(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  const StackTraceInfo *sti = getStackTraceInfo(runtime, selfHandle);
  return HermesValue::encodeBoolValue(!sti->codeBlock);
}

CallResult<HermesValue> JSCallSite::getThis(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue> JSCallSite::getTypeName(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getFunction(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue> JSCallSite::getMethodName(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::getEvalOrigin(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::isToplevel(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::isEval(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::isConstructor(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

CallResult<HermesValue> JSCallSite::isAsync(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeBoolValue(false);
}

CallResult<HermesValue> JSCallSite::isPromiseAll(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeBoolValue(false);
}

CallResult<HermesValue> JSCallSite::getPromiseIndex(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  return HermesValue::encodeNullValue();
}

const StackTraceInfo *JSCallSite::getStackTraceInfo(
    Runtime &runtime,
    Handle<JSCallSite> selfHandle) {
  JSError *error = selfHandle->error_.getNonNull(runtime);
  const StackTrace *stacktrace = error->getStackTrace();
  assert(
      stacktrace &&
      "The Error associated with this CallSite has already released its "
      "stack trace vector");
  // Returning a pointer to stacktrace elements is safe because:
  //   1. stacktrace's ownership is managed (indirectly) by selfHandle (i.e.,
  //      the CallSite object); and
  //   2. stacktrace is not modified after it is created.
  return &stacktrace->at(selfHandle->stackFrameIndex_);
}

} // namespace vm
} // namespace hermes
