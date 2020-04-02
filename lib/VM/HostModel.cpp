/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HostModel.h"

#include "hermes/VM/BuildMetadata.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class FinalizableNativeFunction

CallableVTable FinalizableNativeFunction::vt{
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
}

#ifdef HERMESVM_SERIALIZE
void FinalizableNativeFunctionSerialize(Serializer &s, const GCCell *cell) {
  llvm::outs()
      << "Serialize function not implemented for FinalizableNativeFunction\n";
}

void FinalizableNativeFunctionDeserialize(Deserializer &d, CellKind kind) {
  llvm::outs()
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

  JSObjectAlloc<FinalizableNativeFunction, HasFinalizer::Yes> mem{runtime};
  auto selfHandle = mem.initToHandle(new (mem) FinalizableNativeFunction(
      runtime,
      parentHandle,
      createPseudoHandle(runtime->getHiddenClassForPrototypeRaw(
          *parentHandle,
          numOverlapSlots<FinalizableNativeFunction>() +
              ANONYMOUS_PROPERTY_SLOTS)),
      context,
      functionPtr,
      finalizePtr));

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

ObjectVTable HostObject::vt{
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
}

#ifdef HERMESVM_SERIALIZE
void HostObjectSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs() << "Serialize function not implemented for HostObject\n");
}

void HostObjectDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs() << "Deserialize function not implemented for HostObject\n");
}
#endif

CallResult<HermesValue> HostObject::createWithoutPrototype(
    Runtime *runtime,
    std::shared_ptr<HostObjectProxy> proxy) {
  auto parentHandle = Handle<JSObject>::vmcast(&runtime->objectPrototype);

  JSObjectAlloc<HostObject, HasFinalizer::Yes> mem{runtime};
  HostObject *hostObj = new (mem) HostObject(
      runtime,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(
          *parentHandle,
          numOverlapSlots<HostObject>() + ANONYMOUS_PROPERTY_SLOTS),
      proxy);

  hostObj->flags_.hostObject = true;

  return mem.initToHermesValue(hostObj);
}

void HostObject::_finalizeImpl(GCCell *cell, GC *) {
  auto *self = vmcast<HostObject>(cell);
  // Destruct the object.
  self->~HostObject();
}

} // namespace vm
} // namespace hermes
