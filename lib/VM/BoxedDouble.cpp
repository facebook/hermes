/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/BoxedDouble.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/sh_mirror.h"

namespace hermes {
namespace vm {

const VTable BoxedDouble::vt{
    CellKind::BoxedDoubleKind,
    cellSize<BoxedDouble>()};
void BoxedDoubleBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&BoxedDouble::vt);
}

void BoxedDouble::staticAsserts() {
  static_assert(
      sizeof(SHBoxedDouble) == sizeof(BoxedDouble),
      "SHBoxedDouble size must match BoxedDouble");
  static_assert(
      offsetof(SHBoxedDouble, value_) == offsetof(BoxedDouble, value_),
      "SHBoxedDouble value_ offset must match BoxedDouble");
}

} // namespace vm
} // namespace hermes
