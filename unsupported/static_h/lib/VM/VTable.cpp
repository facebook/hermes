/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/VTable.h"

#include "hermes/VM/GCCell.h"

namespace hermes {
namespace vm {

std::array<const VTable *, kNumCellKinds> VTable::vtableArray;

#ifdef HERMES_MEMORY_INSTRUMENTATION
std::string VTable::HeapSnapshotMetadata::nameForNode(GCCell *cell, GC &gc)
    const {
  std::string name;
  if (name_) {
    name = name_(cell, gc);
  }
  if (!name.empty()) {
    return name;
  }
  return defaultNameForNode(cell);
}

std::string VTable::HeapSnapshotMetadata::defaultNameForNode(
    GCCell *cell) const {
  return cellKindStr(cell->getKind());
}

void VTable::HeapSnapshotMetadata::addEdges(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) const {
  if (addEdges_) {
    addEdges_(cell, gc, snap);
  }
}

void VTable::HeapSnapshotMetadata::addNodes(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) const {
  if (addNodes_) {
    addNodes_(cell, gc, snap);
  }
}

void VTable::HeapSnapshotMetadata::addLocations(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) const {
  if (addLocations_) {
    addLocations_(cell, gc, snap);
  }
}
#endif // HERMES_MEMORY_INSTRUMENTATION

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const VTable &vt) {
  return os << "VTable: {\n\tsize: " << vt.size
            << ", finalize: " << reinterpret_cast<void *>(vt.finalize_)
            << ", markWeak: " << reinterpret_cast<void *>(vt.markWeak_) << "}";
}

} // namespace vm
} // namespace hermes
