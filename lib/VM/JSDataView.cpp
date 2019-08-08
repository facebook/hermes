/*
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

void DataViewSerialize(Serializer &s, const GCCell *cell) {}

void DataViewDeserialize(Deserializer &d, CellKind kind) {}

CallResult<HermesValue> JSDataView::create(
    Runtime *runtime,
    Handle<JSObject> prototype) {
  void *mem = runtime->alloc(sizeof(JSDataView));
  return HermesValue::encodeObjectValue(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSDataView(
              runtime,
              *prototype,
              runtime->getHiddenClassForPrototypeRaw(*prototype))));
}

JSDataView::JSDataView(Runtime *runtime, JSObject *parent, HiddenClass *clazz)
    : JSObject(runtime, &vt.base, parent, clazz),
      buffer_(nullptr),
      offset_(0),
      length_(0) {}

} // namespace vm
} // namespace hermes
