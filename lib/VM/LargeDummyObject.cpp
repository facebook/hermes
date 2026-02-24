/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/LargeDummyObject.h"

namespace hermes::vm {
namespace testhelpers {
const VTable LargeDummyObject::vt{
    CellKind::LargeDummyObjectKind,
    0,
    /*allowLargeAlloc=*/true};

/* static */ LargeDummyObject *LargeDummyObject::create(uint32_t size, GC &gc) {
  return gc.makeAVariable<
      LargeDummyObject,
      HasFinalizer::No,
      LongLived::No,
      CanBeLarge::Yes,
      MayFail::Yes>(size);
}
} // namespace testhelpers

void LargeDummyObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&testhelpers::LargeDummyObject::vt);
  const auto *self = static_cast<const testhelpers::LargeDummyObject *>(cell);
  mb.addField("HermesValue", &self->hv);
  mb.addField("ptrToNormalObj", &self->ptrToNormalObj);
}

} // namespace hermes::vm
