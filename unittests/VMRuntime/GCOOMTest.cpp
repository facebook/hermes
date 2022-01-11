/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/PointerBase.h"

#include <deque>

using namespace hermes::vm;

namespace {

TEST(GCOOMDeathTest, SuperSegment) {
  using SuperSegmentCell = EmptyCell<GC::maxAllocationSize() * 2>;
  auto runtime = DummyRuntime::create(kTestGCConfig);
  EXPECT_OOM(SuperSegmentCell::create(*runtime));
}

static void exceedMaxHeap(
    GCConfig::Builder baseConfig = kTestGCConfigBaseBuilder) {
  static constexpr size_t kSegments = 10;
  static constexpr size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * kSegments;
  // Only one of these cells will fit into a segment, with the maximum amount of
  // space wasted in the segment.
  using AwkwardCell = EmptyCell<AlignedHeapSegment::maxSize() / 2 + 1>;

  auto runtime =
      DummyRuntime::create(TestGCConfigFixedSize(kHeapSizeHint, baseConfig));
  DummyRuntime &rt = *runtime;
  GCScope scope{&rt};

  // Exceed the maximum size of the heap. Note we need 2 extra segments instead
  // of just one because Hades can sometimes hide the memory for a segment
  // during compaction.
  for (size_t i = 0; i < kSegments + 2; ++i)
    rt.makeHandle(AwkwardCell::create(rt));
}

TEST(GCOOMDeathTest, Fragmentation) {
  EXPECT_OOM(exceedMaxHeap());
}

TEST_F(RuntimeTestFixture, WeakMapMarking) {
  auto mapResult = JSWeakMap::create(
      runtime, Handle<JSObject>::vmcast(&runtime->weakMapPrototype));
  auto map = runtime->makeHandle(std::move(*mapResult));
  MutableHandle<ArrayStorage> keys{runtime};
  keys = vmcast<ArrayStorage>(*ArrayStorage::create(runtime, 5));
  EXPECT_OOM(while (true) {
    GCScopeMarkerRAII marker(runtime);
    auto key = runtime->makeHandle(JSObject::create(runtime));
    ArrayStorage::push_back(keys, runtime, key);
    auto value = runtime->makeHandle(JSObject::create(runtime));
    JSWeakMap::setValue(map, runtime, key, value);
  });
}

} // namespace

#endif
