/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValueTraits.h"

#include <vector>

using namespace hermes::vm;

/// Testing finalizer support in whichever GC implementation the tests are
/// configured with.

namespace {

struct FinalizerCell final : public GCCell {
  static const VTable vt;
  int *numFinalized;
  // Some padding to meet the minimum cell size.
  uint64_t padding1_{0};
  uint64_t padding2_{0};

  static void finalize(GCCell *cell, GC *) {
    auto *self = static_cast<FinalizerCell *>(cell);
    ++(*self->numFinalized);
  }

  static FinalizerCell *create(DummyRuntime &runtime, int *numFinalized) {
    return runtime.makeAFixed<FinalizerCell, HasFinalizer::Yes>(
        &runtime.getHeap(), numFinalized);
  }

  static FinalizerCell *create(Runtime &runtime, int *numFinalized) {
    return runtime.makeAFixed<FinalizerCell, HasFinalizer::Yes>(
        &runtime.getHeap(), numFinalized);
  }

  FinalizerCell(GC *gc, int *numFinalized)
      : GCCell(gc, &vt), numFinalized(numFinalized) {}
};

struct DummyCell final : public GCCell {
  static const VTable vt;
  // Some padding to meet the minimum cell size.
  uint64_t padding1_{0};
  uint64_t padding2_{0};

  static DummyCell *create(DummyRuntime &runtime) {
    return runtime.makeAFixed<DummyCell>(&runtime.getHeap());
  }

  DummyCell(GC *gc) : GCCell(gc, &vt) {}
};

const VTable FinalizerCell::vt{
    CellKind::FillerCellKind,
    sizeof(FinalizerCell),
    FinalizerCell::finalize};

const VTable DummyCell::vt{CellKind::UninitializedKind, sizeof(DummyCell)};

MetadataTableForTests getMetadataTable() {
  // Nothing to mark for either of them, leave a blank metadata.
  static const Metadata storage[] = {Metadata(), Metadata()};
  return MetadataTableForTests(storage);
}

TEST(GCFinalizerTest, NoDeadFinalizables) {
  int finalized = 0;
  auto runtime = DummyRuntime::create(getMetadataTable(), kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;

  DummyCell::create(rt);
  GCCell *r = FinalizerCell::create(rt, &finalized);
  rt.pointerRoots.push_back(&r);
  rt.collect();

  ASSERT_EQ(0, finalized);
}

TEST(GCFinalizerTest, FinalizablesOnly) {
  int finalized = 0;
  auto runtime = DummyRuntime::create(getMetadataTable(), kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;

  FinalizerCell::create(rt, &finalized);
  GCCell *r = FinalizerCell::create(rt, &finalized);
  rt.pointerRoots.push_back(&r);
  rt.collect();

  ASSERT_EQ(1, finalized);
}

TEST(GCFinalizerTest, MultipleCollect) {
  int finalized = 0;
  auto runtime = DummyRuntime::create(getMetadataTable(), kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;

  FinalizerCell::create(rt, &finalized);
  DummyCell::create(rt);
  FinalizerCell::create(rt, &finalized);
  GCCell *r1 = FinalizerCell::create(rt, &finalized);
  GCCell *r2 = DummyCell::create(rt);
  rt.pointerRoots.push_back(&r1);
  rt.pointerRoots.push_back(&r2);
  rt.collect();

  ASSERT_EQ(2, finalized);

  rt.pointerRoots.clear();
  rt.collect();

  ASSERT_EQ(3, finalized);
}

TEST(GCFinalizerTest, FinalizeAllOnRuntimeDestructDummyRuntime) {
  int finalized = 0;
  {
    auto rt = DummyRuntime::create(getMetadataTable(), kTestGCConfigSmall);

    GCCell *r1 = FinalizerCell::create(*rt, &finalized);
    GCCell *r2 = FinalizerCell::create(*rt, &finalized);
    rt->pointerRoots.push_back(&r1);
    rt->pointerRoots.push_back(&r2);

    // Collect once to get the objects into the old gen, then a second time
    // to get their mark bits set in their stable locations.
    rt->collect();
    rt->collect();
    ASSERT_EQ(0, finalized);

    // Now: does destructing the runtime with set mark bits run all the
    // finalizers?
  }
  ASSERT_EQ(2, finalized);
}

TEST(GCFinalizerTest, FinalizeAllOnRuntimeDestructRealRuntime) {
  int finalized = 0;
  std::shared_ptr<Runtime> rt{Runtime::create(kTestRTConfig)};
  {
    GCScope gcScope(rt.get());

    auto r1 = rt->makeHandle(
        HermesValue::encodeObjectValue(FinalizerCell::create(*rt, &finalized)));
    auto r2 = rt->makeHandle(
        HermesValue::encodeObjectValue(FinalizerCell::create(*rt, &finalized)));

    // Collect once to get the objects into the old gen, then a second time
    // to get their mark bits set in their stable locations.
    rt->collect("test");
    rt->collect("test");
    ASSERT_EQ(0, finalized);
    (void)r1;
    (void)r2;
  }

  // Now: does destructing the runtime with set mark bits run all the
  // finalizers?
  rt.reset();

  ASSERT_EQ(2, finalized);
}

} // namespace
