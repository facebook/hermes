/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/BoxedDouble.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

const VTable BoxedDouble::vt{
    CellKind::BoxedDoubleKind,
    cellSize<BoxedDouble>()};
void BoxedDoubleBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&BoxedDouble::vt);
}

} // namespace vm
} // namespace hermes
