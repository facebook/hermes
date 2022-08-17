/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/FillerCell.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

const VTable FillerCell::vt{CellKind::FillerCellKind, 0};

void UninitializedBuildMeta(const GCCell *, Metadata::Builder &mb) {
  static VTable vt{CellKind::UninitializedKind, 0};
  mb.setVTable(&vt);
}
void FillerCellBuildMeta(const GCCell *, Metadata::Builder &mb) {
  mb.setVTable(&FillerCell::vt);
}

} // namespace vm
} // namespace hermes
