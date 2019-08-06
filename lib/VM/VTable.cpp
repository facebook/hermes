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
  std::string name;
  if (name_) {
    name = name_(cell, gc);
  }
  if (!name.empty()) {
    return name;
  }
  return cellKindStr(cell->getKind());
}

void VTable::HeapSnapshotMetadata::addEdges(
    GCCell *cell,
    GC *gc,
    V8HeapSnapshot &snap) const {
  if (addEdges_) {
    addEdges_(cell, gc, snap);
  }
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const VTable &vt) {
  return os << "VTable: {\n\tsize: " << vt.size
            << ", finalize: " << reinterpret_cast<void *>(vt.finalize_)
            << ", markWeak: " << reinterpret_cast<void *>(vt.markWeak_) << "}";
}

} // namespace vm
} // namespace hermes
