/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/FillerCell.h"

#include "hermes/VM/BuildMetadata.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

const VTable FillerCell::vt{CellKind::FillerCellKind, 0};

void UninitializedBuildMeta(const GCCell *, Metadata::Builder &mb) {
  const static VTable vt{CellKind::UninitializedKind, 0};
  mb.setVTable(&vt);
}
void FillerCellBuildMeta(const GCCell *, Metadata::Builder &mb) {
  mb.setVTable(&FillerCell::vt);
}

#ifdef HERMESVM_SERIALIZE
void UninitializedSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvh::dbgs() << "Serialize function not implemented for Uinitialized\n");
}

void UninitializedDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvh::dbgs()
      << "Deserialize function not implemented for Uninitialized\n");
}

void FillerCellSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const FillerCell>(cell);
  s.writeInt<uint32_t>(self->getSize());
  s.endObject(self);
}

void FillerCellDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::FillerCellKind && "Expected FillerCell");
  uint32_t size = d.readInt<uint32_t>();
  FillerCell *cell = FillerCell::create(d.getRuntime(), size);
  d.endObject((void *)cell);
}
#endif

} // namespace vm
} // namespace hermes
#undef DEBUG_TYPE
