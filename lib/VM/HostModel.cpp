/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HostModel.h"

#include "hermes/VM/BuildMetadata.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class FinalizableNativeFunction

const CallableVTable FinalizableNativeFunction::vt{
    {
        VTable(
            CellKind::FinalizableNativeFunctionKind,
            cellSize<FinalizableNativeFunction>(),
            FinalizableNativeFunction::_finalizeImpl),
        FinalizableNativeFunction::_getOwnIndexedRangeImpl,
        FinalizableNativeFunction::_haveOwnIndexedImpl,
        FinalizableNativeFunction::_getOwnIndexedPropertyFlagsImpl,
        FinalizableNativeFunction::_getOwnIndexedImpl,
        FinalizableNativeFunction::_setOwnIndexedImpl,
        FinalizableNativeFunction::_deleteOwnIndexedImpl,
        FinalizableNativeFunction::_checkAllOwnIndexedImpl,
    },
    FinalizableNativeFunction::_newObjectImpl,
    FinalizableNativeFunction::_callImpl};

void FinalizableNativeFunctionBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<FinalizableNativeFunction>());
  NativeFunctionBuildMeta(cell, mb);
  mb.setVTable(&FinalizableNativeFunction::vt.base.base);
}

#ifdef HERMESVM_SERIALIZE
void FinalizableNativeFunctionSerialize(Serializer &s, const GCCell *cell) {
  llvh::outs()
      << "Serialize function not implemented for FinalizableNativeFunction\n";
}

void FinalizableNativeFunctionDeserialize(Deserializer &d, CellKind kind) {
  llvh::outs()
      << "Deserialize function not implemented for FinalizableNativeFunction\n";
}
#endif

CallResult<HermesValue> FinalizableNativeFunction::createWithoutPrototype(
    Runtime *runtime,
    void *context,
    NativeFunctionPtr functionPtr,
    FinalizeNativeFunctionPtr finalizePtr,
    SymbolID name,
    unsigned paramCount) {
  auto parentHandle = Handle<JSObject>::vmcast(&runtime->functionPrototype);

  auto *cell =
      runtime->makeAFixed<FinalizableNativeFunction, HasFinalizer::Yes>(
          runtime,
          parentHandle,
          runtime->getHiddenClassForPrototype(
              *parentHandle, numOverlapSlots<FinalizableNativeFunction>()),
          context,
          functionPtr,
          finalizePtr);
  auto selfHandle = JSObjectInit::initToHandle(runtime, cell);

  auto prototypeObjectHandle = Handle<JSObject>(runtime);

  auto st = defineNameLengthAndPrototype(
      selfHandle,
      runtime,
      name,
      paramCount,
      prototypeObjectHandle,
      Callable::WritablePrototype::Yes,
      false);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && "defineLengthAndPrototype() failed");

  return selfHandle.getHermesValue();
}

HostObjectProxy::~HostObjectProxy() {}

const ObjectVTable HostObject::vt{
    VTable(
        CellKind::HostObjectKind,
        cellSize<HostObject>(),
        HostObject::_finalizeImpl),
    HostObject::_getOwnIndexedRangeImpl,
    HostObject::_haveOwnIndexedImpl,
    HostObject::_getOwnIndexedPropertyFlagsImpl,
    HostObject::_getOwnIndexedImpl,
    HostObject::_setOwnIndexedImpl,
    HostObject::_deleteOwnIndexedImpl,
    HostObject::_checkAllOwnIndexedImpl,
};

void HostObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<HostObject>());
  ObjectBuildMeta(cell, mb);
  mb.setVTable(&HostObject::vt.base);
}

#ifdef HERMESVM_SERIALIZE
void HostObjectSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvh::dbgs() << "Serialize function not implemented for HostObject\n");
}

void HostObjectDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvh::dbgs() << "Deserialize function not implemented for HostObject\n");
}
#endif

CallResult<HermesValue> HostObject::createWithoutPrototype(
    Runtime *runtime,
    std::unique_ptr<HostObjectProxy> proxy) {
  auto parentHandle = Handle<JSObject>::vmcast(&runtime->objectPrototype);

  HostObject *hostObj = runtime->makeAFixed<HostObject, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime->getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<HostObject>()),
      std::move(proxy));

  hostObj->flags_.hostObject = true;

  return JSObjectInit::initToHermesValue(runtime, hostObj);
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
