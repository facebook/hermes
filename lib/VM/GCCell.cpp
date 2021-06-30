/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/GCCell.h"
#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifndef NDEBUG
GCCell::GCCell(const VTable *vtp) : GCCell(vtp->kind, vtp->size) {
  assert(vtp->size != 0 && "Fixed size cells should have non-zero size.");
}

GCCell::GCCell(CellKind kind, size_t sz)
    : kindAndSize_(kind, sz), _debugAllocationId_(0) {
  assert(getVT()->size == 0 || getVT()->size == sz && "Invalid size");
  assert(getVT()->kind == kind && "VTable does not match kind.");
}

GCCell::GCCell(GC *gc, const VTable *vtp)
    : kindAndSize_(vtp->kind, vtp->size),
      _debugAllocationId_(gc->nextObjectID()) {
  assert(getVT() == vtp && "VTable does not match kind.");
  // If the vtp has a finalizer, then it should be the most recent thing
  // added to the finalizer list.
  assert(
      (vtp->finalize_ == nullptr || gc->isMostRecentFinalizableObj(this)) &&
      "If the vtp has a finalizer, then the obj should be on the "
      "finalizer list");
  if (gc->lastAllocationWasFixedSize() == GCBase::FixedSizeValue::Yes) {
    assert(
        vtp->size > 0 &&
        "Fixed size allocation must specify object size in vtable.");
  } else if (gc->lastAllocationWasFixedSize() == GCBase::FixedSizeValue::No) {
    assert(
        vtp->size == 0 &&
        "Variable size allocation must have object size = 0 in vtable.");
  }
  assert(
      (!vtp->mallocSize_ || vtp->finalize_) &&
      "If a cell uses malloc, then it needs a finalizer");
}
#endif

} // namespace vm
} // namespace hermes
