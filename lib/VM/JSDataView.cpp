/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSDataView.h"

#include "hermes/VM/BuildMetadata.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

ObjectVTable JSDataView::vt{
    VTable(CellKind::DataViewKind, cellSize<JSDataView>()),
    JSDataView::_getOwnIndexedRangeImpl,
    JSDataView::_haveOwnIndexedImpl,
    JSDataView::_getOwnIndexedPropertyFlagsImpl,
    JSDataView::_getOwnIndexedImpl,
    JSDataView::_setOwnIndexedImpl,
    JSDataView::_deleteOwnIndexedImpl,
    JSDataView::_checkAllOwnIndexedImpl,
};

void DataViewBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSDataView>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSDataView *>(cell);
  mb.addField("buffer", &self->buffer_);
}

#ifdef HERMESVM_SERIALIZE
JSDataView::JSDataView(Deserializer &d) : JSObject(d, &vt.base) {
  d.readRelocation(&buffer_, RelocationKind::GCPointer);
  offset_ = d.readInt<size_type>();
  length_ = d.readInt<size_type>();
}

void DataViewSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSDataView>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSDataView>());
  s.writeRelocation(self->buffer_.get(s.getRuntime()));
  s.writeInt<JSDataView::size_type>(self->offset_);
  s.writeInt<JSDataView::size_type>(self->length_);

  s.endObject(cell);
}

void DataViewDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::DataViewKind && "Expected DataView");
  void *mem = d.getRuntime()->alloc(cellSize<JSDataView>());
  auto *cell = new (mem) JSDataView(d);
  d.endObject(cell);
}
#endif

PseudoHandle<JSDataView> JSDataView::create(
    Runtime *runtime,
    Handle<JSObject> prototype) {
  JSObjectAlloc<JSDataView> mem{runtime};
  return mem.initToPseudoHandle(new (mem) JSDataView(
      runtime,
      *prototype,
      runtime->getHiddenClassForPrototypeRaw(
          *prototype,
          numOverlapSlots<JSDataView>() + ANONYMOUS_PROPERTY_SLOTS)));
}

JSDataView::JSDataView(Runtime *runtime, JSObject *parent, HiddenClass *clazz)
    : JSObject(runtime, &vt.base, parent, clazz),
      buffer_(nullptr),
      offset_(0),
      length_(0) {}

} // namespace vm
} // namespace hermes
