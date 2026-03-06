/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSFinalizationRegistry.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/JSFinalizationRegistry.h"
#include "hermes/VM/Runtime-inline.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

const VTable FinalizationRecord::vt{VTable(
    CellKind::FinalizationRecordKind,
    cellSize<FinalizationRecord>(),
    false,
    FinalizationRecord::_finalizeImpl,
    nullptr,
    nullptr)};

/* static */
void FinalizationRecord::_finalizeImpl(GCCell *cell, GC &) {
  auto *self = static_cast<FinalizationRecord *>(cell);
  self->target.releaseSlot();
  if (!self->unregisterToken.isEmpty())
    self->unregisterToken.releaseSlot();
}

void FinalizationRecordBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const FinalizationRecord *>(cell);
  mb.setVTable(&FinalizationRecord::vt);
  mb.addField(&self->heldValue);
}

/* static */
HermesValue FinalizationRecord::create(
    Runtime &runtime,
    Handle<> target,
    Handle<> heldValue,
    Handle<> unregisterToken) {
  struct : Locals {
    PinnedValue<SmallHermesValue> targetSHV;
    PinnedValue<SmallHermesValue> unregisterTokenSHV;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  // This may allocate when Handle Sanitizer is ON and target is a Symbol.
  lv.targetSHV = SmallHermesValue::encodeHermesValue(*target, runtime);
  lv.unregisterTokenSHV =
      SmallHermesValue::encodeHermesValue(*unregisterToken, runtime);
  auto *record = runtime.makeAFixed<FinalizationRecord, HasFinalizer::Yes>(
      runtime, lv.targetSHV, heldValue, lv.unregisterTokenSHV);
  return HermesValue::encodeObjectValue(record);
}

const ObjectVTable JSFinalizationRegistry::vt{
    VTable(
        CellKind::JSFinalizationRegistryKind,
        cellSize<JSFinalizationRegistry>()),
    JSFinalizationRegistry::_getOwnIndexedRangeImpl,
    JSFinalizationRegistry::_haveOwnIndexedImpl,
    JSFinalizationRegistry::_getOwnIndexedPropertyFlagsImpl,
    JSFinalizationRegistry::_getOwnIndexedImpl,
    JSFinalizationRegistry::_setOwnIndexedImpl,
    JSFinalizationRegistry::_deleteOwnIndexedImpl,
    JSFinalizationRegistry::_checkAllOwnIndexedImpl,
};

void JSFinalizationRegistryBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<JSFinalizationRegistry>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSFinalizationRegistry *>(cell);
  mb.setVTable(&JSFinalizationRegistry::vt);
  mb.addField(&self->cleanupCallback_);
  mb.addField(&self->cells_);
}

/* static */
PseudoHandle<JSFinalizationRegistry> JSFinalizationRegistry::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    Handle<Callable> cleanupCallback) {
  auto *cell = runtime.makeAFixed<JSFinalizationRegistry>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSFinalizationRegistry>()),
      cleanupCallback);
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

/* static */
ExecutionStatus JSFinalizationRegistry::cleanup(
    Handle<JSFinalizationRegistry> self,
    Runtime &runtime) {
  // If no cells to clean up, return.
  if (!self->cells_)
    return ExecutionStatus::RETURNED;

  struct : Locals {
    PinnedValue<Callable> cleanupCallback;
    PinnedValue<StorageType> cells;
    PinnedValue<FinalizationRecord> record;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.cleanupCallback = self->cleanupCallback_.getNonNull(runtime);
  lv.cells = self->cells_.getNonNull(runtime);
  // We have to create a GCScope here since the caller may not be in a GCScope.
  // The below loop may create too many Handles, it's better to have its own
  // GCScope and flush in every iteration.
  GCScope gcScope(runtime);
  // The size of lv.cells may change during the loop, since the cleanup callback
  // may register/unregister cells.
  for (size_t i = 0; i < lv.cells->size(); ++i) {
    auto element = lv.cells->at(i);
    // This element may be unregistered or cleaned up before.
    if (element.isEmpty())
      continue;
    lv.record = vmcast<FinalizationRecord>(element);

    // ES16 9.12.a Find out dead targets to iterate on.
    if (lv.record->target.isValid())
      continue;

    // ES16 9.12.b Free this record since the target is dead.
    lv.cells->set(i, HermesValue::encodeEmptyValue(), runtime.getHeap());

    GCScopeMarkerRAII gcScopeMarker{gcScope};
    // ES16 9.12.c Run the callback with the held value.
    auto result = Callable::executeCall1(
        lv.cleanupCallback,
        runtime,
        Runtime::getUndefinedValue(),
        lv.record->heldValue);
    if (result == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
  }
  return ExecutionStatus::RETURNED;
}

/* static */
ExecutionStatus JSFinalizationRegistry::registerCell(
    Handle<JSFinalizationRegistry> self,
    Runtime &runtime,
    Handle<> target,
    Handle<> heldValue,
    Handle<> unregisterToken) {
  assert(
      canBeHeldWeakly(runtime, *target) &&
      "target must be Object or non-registered Symbol");
  assert(
      (unregisterToken->isUndefined() ||
       canBeHeldWeakly(runtime, *unregisterToken)) &&
      "unregisterToken must be Undefined, or Object/non-registered Symbol");
  assert(
      target->getRaw() != heldValue->getRaw() &&
      "heldValue can't be the same as target");
  if (!self->cells_) {
    // This should not fail unless OOM.
    auto cells = StorageType::create(runtime, kInitCapacity);
    if (LLVM_UNLIKELY(cells == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    self->cells_.setNonNull(
        runtime, vmcast<StorageType>(*cells), runtime.getHeap());
  }

  struct : Locals {
    PinnedValue<StorageType> cells;
    PinnedValue<> record;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  lv.cells = self->cells_.getNonNull(runtime);
  lv.record =
      FinalizationRecord::create(runtime, target, heldValue, unregisterToken);
  MutableHandle<StorageType> cellsHandle{lv.cells};
  // Add a new FinalizationRecord to the vector.
  if (LLVM_UNLIKELY(
          StorageType::push_back(cellsHandle, runtime, lv.record) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  // Write back the array pointer.
  self->cells_.setNonNull(runtime, *lv.cells, runtime.getHeap());
  return ExecutionStatus::RETURNED;
}

bool JSFinalizationRegistry::unregisterCells(
    Runtime &runtime,
    Handle<> unregisterToken) {
  // No target has been registered yet.
  if (!cells_)
    return false;
  assert(
      canBeHeldWeakly(runtime, *unregisterToken) &&
      "unregisterToken must be Object or non-registered Symbol");
  NoAllocScope noAlloc{runtime};
  // ES16 26.2.3.3
  // 4. Let removed be false.
  // 5. For each Record { [[WeakRefTarget]], [[HeldValue]], [[UnregisterToken]]
  //    } cell of finalizationRegistry.[[Cells]], do
  //    a. If cell.[[UnregisterToken]] is not empty and
  //       SameValue(cell.[[UnregisterToken]], unregisterToken) is true, then
  //       i. Remove cell from finalizationRegistry.[[Cells]].
  //       ii. Set removed to true.
  bool removed = false;
  StorageType &cellsRef = *cells_.get(runtime);
  for (size_t i = 0, e = cellsRef.size(); i < e; ++i) {
    HermesValue element = cellsRef.at(i);
    // This element may be unregistered or cleaned up before.
    if (element.isEmpty())
      continue;
    auto *record = vmcast<FinalizationRecord>(element);
    // Check if the unregisterToken on this record is Empty (not specified)
    // or invalid (freed since it's weakly held).
    if (record->unregisterToken.isEmpty() || !record->unregisterToken.isValid())
      continue;
    // Since unregister token can only be Object or non-registered Symbol,
    // we can directly compare raw bits.
    if (unregisterToken->getRaw() ==
        record->unregisterToken.getNoBarrierUnsafe(runtime).getRaw()) {
      cellsRef.set(i, HermesValue::encodeEmptyValue(), runtime.getHeap());
      removed = true;
    }
  }
  return removed;
}

} // namespace vm
} // namespace hermes
