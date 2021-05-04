/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Casting.h"
#include "TestHelpers.h"
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

struct Type1 : public GCCell {
  static VTable vt;
  int tag1;
  // Need to meet minimum allocation size requirements.
  uint64_t dummy_{0};
  uint64_t dummy2_{0};

  explicit Type1(GC *gc, int64_t tag) : GCCell(gc, &vt), tag1(tag) {}

  static Type1 *create(DummyRuntime *runtime, int64_t tag) {
    return runtime->makeAFixed<Type1>(&runtime->getHeap(), tag);
  }
  static bool classof(const GCCell *cell) {
    return cell->getVT() == &vt;
  }
};
VTable Type1::vt(CellKind::UninitializedKind, sizeof(Type1));

struct Type2 : public GCCell {
  static VTable vt;
  int tag2;
  // Need to meet minimum allocation size requirements.
  uint64_t dummy_{0};
  uint64_t dummy2_{0};

  explicit Type2(GC *gc, int64_t tag) : GCCell(gc, &vt), tag2(tag) {}
  static Type2 *create(DummyRuntime *runtime, int64_t tag) {
    return runtime->makeAFixed<Type2>(&runtime->getHeap(), tag);
  }
  static bool classof(const GCCell *cell) {
    return cell->getVT() == &vt;
  }
};
VTable Type2::vt(CellKind::UninitializedKind, sizeof(Type2));

TEST(CastingTest, SmokeTest) {
  auto rt = DummyRuntime::create(getMetadataTable(), kTestGCConfigSmall);
  DummyRuntime &runtime = *rt;
  GCScope gcScope(&runtime);

  const int TAG1 = 1234;
  const int TAG2 = 5678;

  auto h1 = runtime.makeHandle(
      HermesValue::encodeObjectValue(Type1::create(&runtime, TAG1)));
  auto h2 = runtime.makeHandle(
      HermesValue::encodeObjectValue(Type2::create(&runtime, TAG2)));

  GCCell *p1 = (GCCell *)h1.get().getObject();
  GCCell *p2 = (GCCell *)h2.get().getObject();

  auto v1 = HermesValue::encodeObjectValue(p1);
  auto v2 = HermesValue::encodeObjectValue(p2);
  auto nullv = HermesValue::encodeObjectValue(nullptr);

  EXPECT_EQ(TAG1, vmcast<Type1>(p1)->tag1);
  EXPECT_EQ(TAG2, vmcast<Type2>(p2)->tag2);
  EXPECT_EQ(TAG1, vmcast<Type1>(v1)->tag1);
  EXPECT_EQ(TAG2, vmcast<Type2>(v2)->tag2);

  EXPECT_EQ(TAG1, vmcast_or_null<Type1>(p1)->tag1);
  EXPECT_EQ(TAG2, vmcast_or_null<Type2>(p2)->tag2);
  EXPECT_EQ(TAG1, vmcast_or_null<Type1>(v1)->tag1);
  EXPECT_EQ(TAG2, vmcast_or_null<Type2>(v2)->tag2);

  EXPECT_EQ(nullptr, vmcast_or_null<Type1>(nullptr));
  EXPECT_EQ(nullptr, vmcast_or_null<Type1>(nullv));

  EXPECT_NE(nullptr, dyn_vmcast<Type1>(p1));
  EXPECT_NE(nullptr, dyn_vmcast<Type2>(p2));
  EXPECT_NE(nullptr, dyn_vmcast<Type1>(v1));
  EXPECT_NE(nullptr, dyn_vmcast<Type2>(v2));

  EXPECT_EQ(nullptr, dyn_vmcast<Type1>(p2));
  EXPECT_EQ(nullptr, dyn_vmcast<Type2>(p1));
  EXPECT_EQ(nullptr, dyn_vmcast<Type1>(v2));
  EXPECT_EQ(nullptr, dyn_vmcast<Type2>(v1));

  EXPECT_EQ(TAG1, dyn_vmcast<Type1>(p1)->tag1);
  EXPECT_EQ(TAG2, dyn_vmcast<Type2>(p2)->tag2);
  EXPECT_EQ(TAG1, dyn_vmcast<Type1>(v1)->tag1);
  EXPECT_EQ(TAG2, dyn_vmcast<Type2>(v2)->tag2);

  EXPECT_NE(nullptr, dyn_vmcast_or_null<Type1>(p1));
  EXPECT_NE(nullptr, dyn_vmcast_or_null<Type2>(p2));
  EXPECT_NE(nullptr, dyn_vmcast_or_null<Type1>(v1));
  EXPECT_NE(nullptr, dyn_vmcast_or_null<Type2>(v2));

  EXPECT_EQ(nullptr, dyn_vmcast_or_null<Type1>(p2));
  EXPECT_EQ(nullptr, dyn_vmcast_or_null<Type2>(p1));
  EXPECT_EQ(nullptr, dyn_vmcast_or_null<Type1>(v2));
  EXPECT_EQ(nullptr, dyn_vmcast_or_null<Type2>(v1));
  EXPECT_EQ(nullptr, dyn_vmcast_or_null<Type1>(nullptr));
  EXPECT_EQ(nullptr, dyn_vmcast_or_null<Type1>(nullv));

  EXPECT_EQ(TAG1, dyn_vmcast_or_null<Type1>(p1)->tag1);
  EXPECT_EQ(TAG2, dyn_vmcast_or_null<Type2>(p2)->tag2);
  EXPECT_EQ(TAG1, dyn_vmcast_or_null<Type1>(v1)->tag1);
  EXPECT_EQ(TAG2, dyn_vmcast_or_null<Type2>(v2)->tag2);
}
} // namespace
