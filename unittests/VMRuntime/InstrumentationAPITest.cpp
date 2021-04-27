/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/VM/BuildMetadata.h>
#include <hermes/VM/GC.h>
#include <chrono>
#include <functional>
#include "TestHelpers.h"
#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

const MetadataTableForTests getMetadataTable() {
  static const Metadata table[] = {Metadata()};
  return MetadataTableForTests(table);
}

struct Dummy final : public GCCell {
  static const VTable vt;

  static Dummy *create(DummyRuntime &runtime) {
    return runtime.makeAFixed<Dummy>(&runtime.getHeap());
  }
  static bool classof(const GCCell *cell) {
    return cell->getVT() == &vt;
  }

  template <class C>
  static constexpr uint32_t cellSizeImpl() {
    static_assert(std::is_convertible<C *, Dummy *>::value, "must be a Dummy");
    return 128;
  }

  Dummy(GC *gc) : GCCell(gc, &vt) {}
};

const VTable Dummy::vt{CellKind::UninitializedKind, cellSize<Dummy>()};

TEST(InstrumentationAPITest, RunCallbackWhenCollecting) {
  bool triggeredTripwire = false;
  auto rt = DummyRuntime::create(
      getMetadataTable(),
      kTestGCConfigSmall.rebuild()
          .withTripwireConfig(
              GCTripwireConfig::Builder()
                  .withLimit(0)
                  .withCallback([&triggeredTripwire](GCTripwireContext &) {
                    triggeredTripwire = true;
                  })
                  .build())
          .build());
  DummyRuntime &runtime = *rt;
  runtime.collect();
  EXPECT_TRUE(triggeredTripwire);
}

TEST(InstrumentationAPITest, DontRunCallbackWhenCollecting_underSizeLimit) {
  bool triggeredTripwire = false;
  auto rt = DummyRuntime::create(
      getMetadataTable(),
      kTestGCConfigSmall.rebuild()
          .withTripwireConfig(
              GCTripwireConfig::Builder()
                  .withLimit(100)
                  .withCallback([&triggeredTripwire](GCTripwireContext &) {
                    triggeredTripwire = true;
                  })
                  .build())
          .build());
  DummyRuntime &runtime = *rt;
  runtime.collect();
  EXPECT_FALSE(triggeredTripwire);
}

TEST(InstrumentationAPITest, RunCallbackOnlyOnce_UnderCooldownTime) {
  int timesTriggeredTripwire = 0;
  auto rt = DummyRuntime::create(
      getMetadataTable(),
      kTestGCConfigSmall.rebuild()
          .withTripwireConfig(
              GCTripwireConfig::Builder()
                  .withLimit(0)
                  .withCallback([&timesTriggeredTripwire](GCTripwireContext &) {
                    timesTriggeredTripwire++;
                  })
                  .build())
          .build());
  DummyRuntime &runtime = *rt;

  runtime.getHeap().checkTripwire(100);
  runtime.getHeap().checkTripwire(100);
  EXPECT_EQ(timesTriggeredTripwire, 1);
}

TEST(InstrumentationAPITest, RunCallbackAfterAllocatingMemoryOverLimit) {
  bool triggeredTripwire = false;
  auto rt = DummyRuntime::create(
      getMetadataTable(),
      kTestGCConfigSmall.rebuild()
          .withTripwireConfig(
              GCTripwireConfig::Builder()
                  .withLimit(32)
                  .withCallback([&triggeredTripwire](GCTripwireContext &) {
                    triggeredTripwire = true;
                  })
                  .build())
          .build());
  DummyRuntime &runtime = *rt;
  runtime.collect();
  EXPECT_FALSE(triggeredTripwire);
  GCCell *cell = Dummy::create(runtime);
  runtime.pointerRoots.push_back(&cell);
  runtime.collect();
  EXPECT_TRUE(triggeredTripwire);
}

TEST(InstrumentationAPITest, DontRunCallbackAfterAllocatingMemoryUnderLimit) {
  bool triggeredTripwire = false;
  auto rt = DummyRuntime::create(
      getMetadataTable(),
      kTestGCConfigSmall.rebuild()
          .withTripwireConfig(
              GCTripwireConfig::Builder()
                  .withLimit(256)
                  .withCallback([&triggeredTripwire](GCTripwireContext &) {
                    triggeredTripwire = true;
                  })
                  .build())
          .build());
  DummyRuntime &runtime = *rt;
  runtime.collect();
  EXPECT_FALSE(triggeredTripwire);
  GCCell *cell = Dummy::create(runtime);
  runtime.pointerRoots.push_back(&cell);
  runtime.collect();
  EXPECT_FALSE(triggeredTripwire);
}

} // namespace
