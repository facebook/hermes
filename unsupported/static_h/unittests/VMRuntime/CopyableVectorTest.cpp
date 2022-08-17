/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/CopyableVector.h"

#include "TestHelpers.h"
#include "hermes/Support/Compiler.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using CopyableVectorTest = RuntimeTestFixture;

TEST_F(CopyableVectorTest, PushBackInt) {
  {
    CopyableVector<int> v;
    EXPECT_TRUE(v.empty());

    v.push_back(42, runtime.getHeap());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(1, v.size());
    EXPECT_EQ(42, v[0]);

    v.push_back(43, runtime.getHeap());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(2, v.size());
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(43, v[1]);

    v.push_back(44, runtime.getHeap());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(3, v.size());
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(43, v[1]);
    EXPECT_EQ(44, v[2]);
  }

  {
    CopyableVector<int> v;
    v.reserve(10);
    EXPECT_TRUE(v.empty());

    v.push_back(42, runtime.getHeap());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(1, v.size());
    EXPECT_EQ(42, v[0]);

    v.push_back(43, runtime.getHeap());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(2, v.size());
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(43, v[1]);

    v.push_back(44, runtime.getHeap());
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(3, v.size());
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(43, v[1]);
    EXPECT_EQ(44, v[2]);
  }
}

TEST_F(CopyableVectorTest, ArrayRef) {
  using llvh::ArrayRef;
  CopyableVector<char> v;
  auto arr1 = ArrayRef<char>("1234", 4);
  auto arr2 = ArrayRef<char>("12345678", 8);
  EXPECT_EQ(0, v.size());
  v = arr1;
  EXPECT_EQ(arr1, ArrayRef<char>(v));
  EXPECT_NE(arr2, ArrayRef<char>(v));
  v = arr2;
  EXPECT_NE(arr1, ArrayRef<char>(v));
  EXPECT_EQ(arr2, ArrayRef<char>(v));
}

TEST_F(CopyableVectorTest, ArrayRefNonTrivial) {
  using llvh::ArrayRef;
  CopyableVector<std::string> v;
  std::string strings1[] = {"12", "345678", "91011", "121314151617181920"};
  std::string strings2[] = {
      "abc",
      "defhi",
      "jklmonpqrst",
      "uvwxyzABCDEF",
      "HIGHJKLMNOP",
      "Q",
      "",
      "RSTU"};
  auto arr1 = llvh::makeArrayRef(strings1);
  auto arr2 = llvh::makeArrayRef(strings2);
  EXPECT_EQ(0, v.size());
  v = arr1;
  EXPECT_EQ(arr1, ArrayRef<std::string>(v));
  EXPECT_NE(arr2, ArrayRef<std::string>(v));
  v = arr2;
  EXPECT_NE(arr1, ArrayRef<std::string>(v));
  EXPECT_EQ(arr2, ArrayRef<std::string>(v));
}

TEST_F(CopyableVectorTest, PushBackStruct) {
  struct Elem {
    int x;
    int y;
  };

  CopyableVector<Elem> v;
  v.push_back({100, 200}, runtime.getHeap());
  EXPECT_EQ(1, v.size());
  EXPECT_EQ(100, v[0].x);
  EXPECT_EQ(200, v[0].y);
  v.push_back({101, 201}, runtime.getHeap());
  EXPECT_EQ(2, v.size());
  EXPECT_EQ(100, v[0].x);
  EXPECT_EQ(200, v[0].y);
  EXPECT_EQ(101, v[1].x);
  EXPECT_EQ(201, v[1].y);
}

TEST_F(CopyableVectorTest, Pop) {
  CopyableVector<int> v;
  v.push_back(10, runtime.getHeap());
  v.push_back(11, runtime.getHeap());
  v.push_back(12, runtime.getHeap());
  v.push_back(13, runtime.getHeap());
  EXPECT_EQ(4, v.size());

  v.pop_back();
  EXPECT_EQ(3, v.size());
  EXPECT_EQ(10, v[0]);
  EXPECT_EQ(11, v[1]);
  EXPECT_EQ(12, v[2]);

  v.pop_back();
  EXPECT_EQ(2, v.size());
  EXPECT_EQ(10, v[0]);
  EXPECT_EQ(11, v[1]);

  v.pop_back();
  EXPECT_EQ(1, v.size());
  EXPECT_EQ(10, v[0]);

  v.pop_back();
  EXPECT_TRUE(v.empty());
}

TEST_F(CopyableVectorTest, Iteration) {
  CopyableVector<int> v;
  EXPECT_EQ(v.begin(), v.end());
  EXPECT_TRUE(v.empty());

  v.push_back(10, runtime.getHeap());
  v.push_back(11, runtime.getHeap());
  v.push_back(12, runtime.getHeap());
  v.push_back(13, runtime.getHeap());

  {
    int sum = 0;
    for (int i : v) {
      sum += i;
    }
    EXPECT_EQ(46, sum);
  }

  {
    int sum = 0;
    for (const int &i : v) {
      sum += i;
    }
    EXPECT_EQ(46, sum);
  }

  {
    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
      sum += *it;
    }
    EXPECT_EQ(46, sum);
  }
}

TEST_F(CopyableVectorTest, PushPop) {
  CopyableVector<int> v(1);
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(1, v.capacity());
  for (int i = 0; i < 1000; ++i) {
    v.push_back(i, runtime.getHeap());
    EXPECT_EQ(i, v[0]);
    EXPECT_EQ(1, v.size());
    EXPECT_EQ(1, v.capacity());
    v.pop_back();
    EXPECT_TRUE(v.empty());
  }
  EXPECT_EQ(1, v.capacity());
  EXPECT_TRUE(v.empty());
}

TEST_F(CopyableVectorTest, MovePush) {
  CopyableVector<std::unique_ptr<int>> v(1);
  for (uint32_t i = 0; i < 10; ++i) {
    std::unique_ptr<int> ref = std::unique_ptr<int>(new int(i));
    v.push_back(std::move(ref), runtime.getHeap());
  }
  EXPECT_EQ(10, v.size());
  for (uint32_t i = 0; i < 10; ++i) {
    EXPECT_EQ(i, *v[i]);
  }
}

TEST_F(CopyableVectorTest, NonTrivialTest) {
  std::shared_ptr<int> ref = std::make_shared<int>(0);

  {
    CopyableVector<std::shared_ptr<int>> v(1);

    // Ensure that pushing back and growing has proper memory management.
    for (uint32_t i = 0; i < 10; ++i) {
      v.push_back(ref, runtime.getHeap());
      EXPECT_EQ(i + 2, ref.use_count());
    }

    // Ensure popping destructs objects.
    v.pop_back();
    EXPECT_EQ(10, ref.use_count());
    v.pop_back();
    EXPECT_EQ(9, ref.use_count());
  }

  // Ensure destructing the vector calls all destructors.
  EXPECT_EQ(1, ref.use_count());
}

} // namespace
