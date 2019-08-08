/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/FillerCell.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

const VTable FillerCell::vt{CellKind::FillerCellKind, 0};

// Empty to prevent linker errors, doesn't need to do anything.
void UninitializedBuildMeta(const GCCell *, Metadata::Builder &) {}
void FillerCellBuildMeta(const GCCell *, Metadata::Builder &) {}

void UninitializedSerialize(Serializer &s, const GCCell *cell) {}

void FillerCellSerialize(Serializer &s, const GCCell *cell) {}

void UninitializedDeserialize(Deserializer &d, CellKind kind) {}

void FillerCellDeserialize(Deserializer &d, CellKind kind) {}

} // namespace vm
} // namespace hermes
