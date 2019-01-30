/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSDataView.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

ObjectVTable JSDataView::vt{
    VTable(CellKind::DataViewKind, sizeof(JSDataView)),
    JSDataView::_getOwnIndexedRangeImpl,
    JSDataView::_haveOwnIndexedImpl,
    JSDataView::_getOwnIndexedPropertyFlagsImpl,
    JSDataView::_getOwnIndexedImpl,
    JSDataView::_setOwnIndexedImpl,
    JSDataView::_deleteOwnIndexedImpl,
    JSDataView::_checkAllOwnIndexedImpl,
};

void DataViewBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSDataView *>(cell);
  mb.addField("@buffer", &self->buffer_);
}

CallResult<HermesValue> JSDataView::create(
    Runtime *runtime,
    Handle<JSObject> prototype) {
  auto propStorage =
      JSObject::createPropStorage(runtime, NEEDED_PROPERTY_SLOTS);
  if (LLVM_UNLIKELY(propStorage == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  void *mem = runtime->alloc(sizeof(JSDataView));
  return HermesValue::encodeObjectValue(new (mem) JSDataView(
      runtime,
      *prototype,
      runtime->getHiddenClassForPrototypeRaw(*prototype),
      **propStorage));
}

JSDataView::JSDataView(
    Runtime *runtime,
    JSObject *parent,
    HiddenClass *clazz,
    JSObjectPropStorage *propStorage)
    : JSObject(runtime, &vt.base, parent, clazz, propStorage),
      buffer_(nullptr),
      offset_(0),
      length_(0) {}

} // namespace vm
} // namespace hermes
