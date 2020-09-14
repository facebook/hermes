/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_ARRAY_H
#define HERMES_UNITTESTS_VMRUNTIME_ARRAY_H

#include "TestHelpers.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GC.h"

#include "llvh/Support/TrailingObjects.h"

using namespace hermes::vm;

namespace hermes {
namespace unittest {

/// An Array class suitable for use in unit tests.
class Array final : public VariableSizeRuntimeCell,
                    private llvh::TrailingObjects<Array, GCHermesValue> {
  friend TrailingObjects;

 public:
  static const VTable vt;
  AtomicIfConcurrentGC<uint32_t> length;

  Array(GC *gc, uint32_t length)
      : VariableSizeRuntimeCell(gc, &vt, allocSize(length)), length(length) {}

  static bool classof(const GCCell *cell) {
    return cell->getVT() == &vt;
  }

  GCHermesValue *values() {
    return getTrailingObjects<GCHermesValue>();
  }
  const GCHermesValue *values() const {
    return getTrailingObjects<GCHermesValue>();
  }

  static Array *create(DummyRuntime &runtime, uint32_t length) {
    auto *self =
        new (runtime.allocWithFinalizer</*fixedSize*/ false>(allocSize(length)))
            Array(&runtime.getHeap(), length);
    GCHermesValue::fill(
        self->values(),
        self->values() + length,
        HermesValue::encodeEmptyValue(),
        &runtime.getHeap());
    return self;
  }

  static uint32_t allocSize(uint32_t length) {
    return totalSizeToAlloc<GCHermesValue>(length);
  }
};

void ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb);

} // namespace unittest
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_ARRAY_H
