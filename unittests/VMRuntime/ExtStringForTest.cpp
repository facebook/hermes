/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ExtStringForTest.h"

#include "TestHelpers.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/StringPrimitive.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

namespace hermes {
namespace vm {

const VTable ExtStringForTest::vt{
    ExternalStringPrimitive<char>::getCellKind(),
    0,
    ExtStringForTest::_finalizeImpl,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    _externalMemorySizeImpl,
};

void ExtStringForTest::_finalizeImpl(GCCell *cell, GC *gc) {
  ExtStringForTest *self = vmcast<ExtStringForTest>(cell);
  gc->debitExternalMemory(self, self->length);
  self->~ExtStringForTest();
}
gcheapsize_t ExtStringForTest::_externalMemorySizeImpl(const GCCell *cell) {
  auto *self = vmcast<ExtStringForTest>(cell);
  return self->length;
}

void ExtStringForTest::releaseMem(GC *gc) {
  gc->debitExternalMemory(this, length);
  length = 0;
}

/*static*/
ExtStringForTest *ExtStringForTest::create(
    DummyRuntime &runtime,
    unsigned length) {
  auto res = runtime.makeAVariable<ExtStringForTest, HasFinalizer::Yes>(
      sizeof(ExtStringForTest), &runtime.getHeap(), length);
  runtime.getHeap().creditExternalMemory(res, length);
  return res;
}

/*static*/
ExtStringForTest *ExtStringForTest::createLongLived(
    DummyRuntime &runtime,
    unsigned length) {
  auto res =
      runtime
          .makeAVariable<ExtStringForTest, HasFinalizer::Yes, LongLived::Yes>(
              sizeof(ExtStringForTest), &runtime.getHeap(), length);
  runtime.getHeap().creditExternalMemory(res, length);
  return res;
}

void ExtStringForTestBuildMeta(const GCCell *cell, Metadata::Builder &mb) {}

} // namespace vm
} // namespace hermes

#pragma GCC diagnostic pop
