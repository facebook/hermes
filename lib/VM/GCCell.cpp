/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/GCCell.h"
#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifndef NDEBUG
GCCell::GCCell(CellKind, size_t) : _debugAllocationId_(0) {}

GCCell::GCCell(GC *gc, const VTable *)
    : _debugAllocationId_(gc->nextObjectID()) {}
#endif

} // namespace vm
} // namespace hermes
