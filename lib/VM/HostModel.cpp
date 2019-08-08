/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/HostModel.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class FinalizableNativeFunction

CallableVTable FinalizableNativeFunction::vt{
    {
        VTable(
            CellKind::FinalizableNativeFunctionKind,
            sizeof(FinalizableNativeFunction),
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
  NativeFunctionBuildMeta(cell, mb);
}

void FinalizableNativeFunctionSerialize(Serializer &s, const GCCell *cell) {}

void FinalizableNativeFunctionDeserialize(Deserializer &d, CellKind kind) {}

CallResult<HermesValue> FinalizableNativeFunction::createWithoutPrototype(
    Runtime *runtime,
    void *context,
    NativeFunctionPtr functionPtr,
    FinalizeNativeFunctionPtr finalizePtr,
    SymbolID name,
    unsigned paramCount) {
  auto parentHandle = Handle<JSObject>::vmcast(&runtime->functionPrototype);

  void *mem = runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      sizeof(FinalizableNativeFunction));
  auto selfHandle = runtime->makeHandle(new (mem) FinalizableNativeFunction(
      runtime,
      parentHandle,
      runtime->getHiddenClassForPrototype(parentHandle),
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
        sizeof(HostObject),
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
  ObjectBuildMeta(cell, mb);
}

void HostObjectSerialize(Serializer &s, const GCCell *cell) {}

void HostObjectDeserialize(Deserializer &d, CellKind kind) {}

CallResult<HermesValue> HostObject::createWithoutPrototype(
    Runtime *runtime,
    std::shared_ptr<HostObjectProxy> proxy) {
  auto parentHandle = Handle<JSObject>::vmcast(&runtime->objectPrototype);

  void *mem =
      runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(sizeof(HostObject));
  HostObject *hostObj = new (mem) HostObject(
      runtime,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(*parentHandle),
      proxy);

  hostObj->flags_.hostObject = true;

  return HermesValue::encodeObjectValue(hostObj);
}

void HostObject::_finalizeImpl(GCCell *cell, GC *) {
  auto *self = vmcast<HostObject>(cell);
  // Destruct the object.
  self->~HostObject();
}

} // namespace vm
} // namespace hermes
