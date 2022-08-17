/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/VM/BuildMetadata.h>
#include <hermes/VM/DummyObject.h>
#include <hermes/VM/GC.h>
#include <chrono>
#include <functional>
#include "TestHelpers.h"
#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using testhelpers::DummyObject;

TEST(InstrumentationAPITest, RunCallbackWhenCollecting) {
  bool triggeredTripwire = false;
  auto rt = DummyRuntime::create(
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
  GCScope scope{runtime};
  auto h = runtime.makeHandle(DummyObject::create(runtime.getHeap(), runtime));
  h->acquireExtMem(runtime.getHeap(), 200);
  runtime.collect();
  EXPECT_TRUE(triggeredTripwire);
}

TEST(InstrumentationAPITest, DontRunCallbackAfterAllocatingMemoryUnderLimit) {
  bool triggeredTripwire = false;
  auto rt = DummyRuntime::create(
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
  GCScope scope{runtime};
  runtime.makeHandle(DummyObject::create(runtime.getHeap(), runtime));
  runtime.collect();
  EXPECT_FALSE(triggeredTripwire);
}

} // namespace
