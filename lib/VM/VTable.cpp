/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/VTable.h"

#include "hermes/VM/GCCell.h"

namespace hermes {
namespace vm {

std::string VTable::HeapSnapshotMetadata::nameForNode(GCCell *cell, GC *gc)
    const {
  if (name_) {
    return name_(cell, gc);
  }
  return cellKindStr(cell->getKind());
}

void VTable::HeapSnapshotMetadata::addEdges(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) const {
  if (addEdges_) {
    addEdges_(cell, gc, snap);
  }
}

void VTable::HeapSnapshotMetadata::addNodes(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) const {
  if (addNodes_) {
    addNodes_(cell, gc, snap);
  }
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const VTable &vt) {
  return os << "VTable: {\n\tsize: " << vt.size
            << ", finalize: " << reinterpret_cast<void *>(vt.finalize_)
            << ", markWeak: " << reinterpret_cast<void *>(vt.markWeak_) << "}";
}

} // namespace vm
} // namespace hermes
