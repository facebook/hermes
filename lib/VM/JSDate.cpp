/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSDate.h"

#include "hermes/VM/BuildMetadata.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSDate

ObjectVTable JSDate::vt{
    VTable(CellKind::DateKind, sizeof(JSDate)),
    JSDate::_getOwnIndexedRangeImpl,
    JSDate::_haveOwnIndexedImpl,
    JSDate::_getOwnIndexedPropertyFlagsImpl,
    JSDate::_getOwnIndexedImpl,
    JSDate::_setOwnIndexedImpl,
    JSDate::_deleteOwnIndexedImpl,
    JSDate::_checkAllOwnIndexedImpl,
};

void DateBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
void DateSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(llvm::dbgs() << "Serialize function not implemented for Date\n");
}

void DateDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(llvm::dbgs() << "Deserialize function not implemented for Date\n");
}
#endif

CallResult<HermesValue>
JSDate::create(Runtime *runtime, double value, Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSDate));
  auto selfHandle = runtime->makeHandle(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSDate(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));
  JSObject::addInternalProperties(
      selfHandle,
      runtime,
      1,
      runtime->makeHandle(HermesValue::encodeDoubleValue(value)));
  return selfHandle.getHermesValue();
}

} // namespace vm
} // namespace hermes
