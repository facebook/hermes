/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSDataView.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Runtime-inline.h"

namespace hermes {
namespace vm {

const ObjectVTable JSDataView::vt{
    VTable(CellKind::JSDataViewKind, cellSize<JSDataView>()),
    JSDataView::_getOwnIndexedRangeImpl,
    JSDataView::_haveOwnIndexedImpl,
    JSDataView::_getOwnIndexedPropertyFlagsImpl,
    JSDataView::_getOwnIndexedImpl,
    JSDataView::_setOwnIndexedImpl,
    JSDataView::_deleteOwnIndexedImpl,
    JSDataView::_checkAllOwnIndexedImpl,
};

void JSDataViewBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSDataView>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSDataView *>(cell);
  mb.setVTable(&JSDataView::vt);
  mb.addField("buffer", &self->buffer_);
}

PseudoHandle<JSDataView> JSDataView::create(
    Runtime &runtime,
    Handle<JSObject> prototype) {
  auto *cell = runtime.makeAFixed<JSDataView>(
      runtime,
      prototype,
      runtime.getHiddenClassForPrototype(
          *prototype, numOverlapSlots<JSDataView>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

JSDataView::JSDataView(
    Runtime &runtime,
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz)
    : JSObject(runtime, *parent, *clazz),
      buffer_(nullptr),
      offset_(0),
      length_(0) {}

} // namespace vm
} // namespace hermes
