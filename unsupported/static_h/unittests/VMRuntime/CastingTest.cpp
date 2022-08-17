/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

TEST(CastingTest, SmokeTest) {
  auto rt = DummyRuntime::create(kTestGCConfigSmall);
  DummyRuntime &runtime = *rt;
  GCScope gcScope(runtime);

  const int TAG1 = 1234;
  const SymbolID TAG2 = SymbolID::unsafeCreate(4567);

  const auto HVTAG1 = HermesValue::encodeNumberValue(1234);
  // Using a Symbol here gives us a completely different internal representation
  // than the HV (even when HV32 is disabled), and works around the fact that
  // SmallHermesValue does not support DummyRuntime.
  const auto SHVTAG2 = SmallHermesValue::encodeSymbolValue(TAG2);

  auto h1 =
      runtime.makeHandle(ArrayStorage::createForTest(runtime.getHeap(), 1));
  h1->set(0, HVTAG1, runtime.getHeap());
  auto h2 = runtime.makeHandle(
      ArrayStorageSmall::createForTest(runtime.getHeap(), 1));
  h2->set(0, SHVTAG2, runtime.getHeap());

  GCCell *p1 = h1.get();
  GCCell *p2 = h2.get();

  auto v1 = HermesValue::encodeObjectValue(p1);
  auto v2 = HermesValue::encodeObjectValue(p2);
  auto nullv = HermesValue::encodeNullptrObjectValueUnsafe();

  EXPECT_EQ(TAG1, vmcast<ArrayStorage>(p1)->at(0).getNumber());
  EXPECT_EQ(TAG2, vmcast<ArrayStorageSmall>(p2)->at(0).getSymbol());
  EXPECT_EQ(TAG1, vmcast<ArrayStorage>(v1)->at(0).getNumber());
  EXPECT_EQ(TAG2, vmcast<ArrayStorageSmall>(v2)->at(0).getSymbol());

  EXPECT_EQ(TAG1, vmcast_or_null<ArrayStorage>(p1)->at(0).getNumber());
  EXPECT_EQ(TAG2, vmcast_or_null<ArrayStorageSmall>(p2)->at(0).getSymbol());
  EXPECT_EQ(TAG1, vmcast_or_null<ArrayStorage>(v1)->at(0).getNumber());
  EXPECT_EQ(TAG2, vmcast_or_null<ArrayStorageSmall>(v2)->at(0).getSymbol());

  EXPECT_TRUE(nullptr == vmcast_or_null<ArrayStorage>(nullptr));
  EXPECT_TRUE(nullptr == vmcast_or_null<ArrayStorage>(nullv));

  EXPECT_TRUE(nullptr != dyn_vmcast<ArrayStorage>(p1));
  EXPECT_TRUE(nullptr != dyn_vmcast<ArrayStorageSmall>(p2));
  EXPECT_TRUE(nullptr != dyn_vmcast<ArrayStorage>(v1));
  EXPECT_TRUE(nullptr != dyn_vmcast<ArrayStorageSmall>(v2));

  EXPECT_TRUE(nullptr == dyn_vmcast<ArrayStorage>(p2));
  EXPECT_TRUE(nullptr == dyn_vmcast<ArrayStorageSmall>(p1));
  EXPECT_TRUE(nullptr == dyn_vmcast<ArrayStorage>(v2));
  EXPECT_TRUE(nullptr == dyn_vmcast<ArrayStorageSmall>(v1));

  EXPECT_EQ(TAG1, dyn_vmcast<ArrayStorage>(p1)->at(0).getNumber());
  EXPECT_EQ(TAG2, dyn_vmcast<ArrayStorageSmall>(p2)->at(0).getSymbol());
  EXPECT_EQ(TAG1, dyn_vmcast<ArrayStorage>(v1)->at(0).getNumber());
  EXPECT_EQ(TAG2, dyn_vmcast<ArrayStorageSmall>(v2)->at(0).getSymbol());

  EXPECT_TRUE(nullptr != dyn_vmcast_or_null<ArrayStorage>(p1));
  EXPECT_TRUE(nullptr != dyn_vmcast_or_null<ArrayStorageSmall>(p2));
  EXPECT_TRUE(nullptr != dyn_vmcast_or_null<ArrayStorage>(v1));
  EXPECT_TRUE(nullptr != dyn_vmcast_or_null<ArrayStorageSmall>(v2));

  EXPECT_TRUE(nullptr == dyn_vmcast_or_null<ArrayStorage>(p2));
  EXPECT_TRUE(nullptr == dyn_vmcast_or_null<ArrayStorageSmall>(p1));
  EXPECT_TRUE(nullptr == dyn_vmcast_or_null<ArrayStorage>(v2));
  EXPECT_TRUE(nullptr == dyn_vmcast_or_null<ArrayStorageSmall>(v1));
  EXPECT_TRUE(nullptr == dyn_vmcast_or_null<ArrayStorage>(nullptr));
  EXPECT_TRUE(nullptr == dyn_vmcast_or_null<ArrayStorage>(nullv));

  EXPECT_EQ(TAG1, dyn_vmcast_or_null<ArrayStorage>(p1)->at(0).getNumber());
  EXPECT_EQ(TAG2, dyn_vmcast_or_null<ArrayStorageSmall>(p2)->at(0).getSymbol());
  EXPECT_EQ(TAG1, dyn_vmcast_or_null<ArrayStorage>(v1)->at(0).getNumber());
  EXPECT_EQ(TAG2, dyn_vmcast_or_null<ArrayStorageSmall>(v2)->at(0).getSymbol());
}
} // namespace
