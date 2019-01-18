/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifdef HERMESVM_GCCELL_ID

GCCell::GCCell(GC *gc, const VTable *vtp)
    : vtp_(vtp), _debugAllocationId_(gc->nextObjectID()) {}

#endif

} // namespace vm
} // namespace hermes
