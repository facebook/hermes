/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifndef NDEBUG
GCCell::GCCell(GC *gc, const VTable *vtp)
    : GCCell(gc, vtp, GCCell::AllocEventOption::DoNotEmit) {
  trackAlloc(gc, vtp);
}

GCCell::GCCell(GC *gc, const VTable *vtp, AllocEventOption doNotEmit)
    : vtp_(vtp), _debugAllocationId_(gc->nextObjectID()) {
  // If the vtp has a finalizer, then it should be the most recent thing
  // added to the finalizer list.
  assert(
      (vtp->finalize_ == nullptr || gc->isMostRecentFinalizableObj(this)) &&
      "If the vtp has a finalizer, then the obj should be on the "
      "finalizer list");
}
#endif

#ifdef HERMESVM_MEMORY_PROFILER
void GCCell::trackAlloc(GC *gc, const VTable *vtp) {
  if (auto *met = gc->memEventTracker()) {
    met->emitAlloc(static_cast<uint32_t>(vtp->kind), getAllocatedSize());
  }
}
#endif

} // namespace vm
} // namespace hermes
