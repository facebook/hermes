/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValue.h"

namespace hermes::vm::testhelpers {

/// A GCCell type supports large allocation. Used in test only.
struct LargeDummyObject final : public VariableSizeRuntimeCell {
  static const VTable vt;

  GCSmallHermesValueInLargeObj hv;
  GCPointerInLargeObj<DummyObject> ptrToNormalObj;

  static constexpr CellKind getCellKind() {
    return CellKind::LargeDummyObjectKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::LargeDummyObjectKind;
  }

  LargeDummyObject() = default;

  static LargeDummyObject *create(uint32_t size, GC &gc);
};
} // namespace hermes::vm::testhelpers
