/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSWeakRef.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/Runtime-inline.h"

namespace hermes {
namespace vm {

const ObjectVTable JSWeakRef::vt{
    VTable(
        CellKind::JSWeakRefKind,
        cellSize<JSWeakRef>(),
        JSWeakRef::_finalizeImpl,
        nullptr,
        nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
        ,
        VTable::HeapSnapshotMetadata{
            HeapSnapshot::NodeType::Object,
            nullptr,
            JSWeakRef::_snapshotAddEdgesImpl,
            nullptr,
            nullptr}
#endif
        ),
    JSWeakRef::_getOwnIndexedRangeImpl,
    JSWeakRef::_haveOwnIndexedImpl,
    JSWeakRef::_getOwnIndexedPropertyFlagsImpl,
    JSWeakRef::_getOwnIndexedImpl,
    JSWeakRef::_setOwnIndexedImpl,
    JSWeakRef::_deleteOwnIndexedImpl,
    JSWeakRef::_checkAllOwnIndexedImpl,
};

void JSWeakRefBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSWeakRef>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSWeakRef::vt);
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void JSWeakRef::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSWeakRef>(cell);
  // Filter out empty refs from adding edges.
  if (!self->ref_.isValid())
    return;
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Weak,
      "weak",
      gc.getObjectID(self->ref_.getNoBarrierUnsafe(gc.getPointerBase())));
}
#endif

void JSWeakRef::_finalizeImpl(GCCell *cell, GC &) {
  auto *self = vmcast<JSWeakRef>(cell);
  if (!self->ref_.isEmpty())
    self->ref_.releaseSlot();
}

PseudoHandle<JSWeakRef> JSWeakRef::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSWeakRef, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSWeakRef>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

void JSWeakRef::setTarget(Runtime &runtime, Handle<JSObject> target) {
  assert(ref_.isEmpty() && "Should not call setTarget multiple times");
  ref_ = WeakRef<JSObject>(runtime, target);
}

HermesValue JSWeakRef::deref(Runtime &runtime) const {
  return ref_.isValid() ? HermesValue::encodeObjectValue(ref_.get(runtime))
                        : HermesValue::encodeUndefinedValue();
}

} // namespace vm
} // namespace hermes
