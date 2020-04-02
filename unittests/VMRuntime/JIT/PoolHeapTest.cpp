/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/PoolHeap.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

static void assertDump(PoolHeap &pool, const char *msg) {
  std::string str;
  llvm::raw_string_ostream OS{str};
  pool.dump(OS, true);
  OS.flush();
  // Strip prefix and suffix.
  if (str.compare(0, 28, "== PoolHeap\nstart: 0x000000\n") == 0)
    str.erase(0, 28);
  while (!str.empty() && str.back() == '\n')
    str.pop_back();
  EXPECT_STREQ(msg, str.c_str());
}

TEST(PoolHeapTest, SmokeTest) {
  char buf[64];
  PoolHeap pool(buf, sizeof(buf));

  assertDump(
      pool,
      "size : 64\n"
      "  Free at        0 size 64");

  void *b0 = pool.alloc(10);
  EXPECT_EQ(buf, b0);
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 16\n"
      "  Free at       16 size 48");

  EXPECT_FALSE(pool.alloc(100));
  void *b1 = pool.alloc(20);
  EXPECT_EQ(buf + 16, b1);
  void *b2 = pool.alloc(16);
  EXPECT_EQ(buf + 48, b2);
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 16\n"
      "  Used at       16 size 32\n"
      "  Used at       48 size 16");

  pool.freeRemaining(b1, 10);
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 16\n"
      "  Used at       16 size 16\n"
      "  Free at       32 size 16\n"
      "  Used at       48 size 16");

  void *b1_1 = pool.alloc(10);
  EXPECT_EQ(buf + 32, b1_1);
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 16\n"
      "  Used at       16 size 16\n"
      "  Used at       32 size 16\n"
      "  Used at       48 size 16");

  pool.free(b1);
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 16\n"
      "  Free at       16 size 16\n"
      "  Used at       32 size 16\n"
      "  Used at       48 size 16");

  pool.free(b1_1);
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 16\n"
      "  Free at       16 size 32\n"
      "  Used at       48 size 16");

  pool.free(b0);
  assertDump(
      pool,
      "size : 64\n"
      "  Free at        0 size 48\n"
      "  Used at       48 size 16");

  pool.free(b2);
  assertDump(
      pool,
      "size : 64\n"
      "  Free at        0 size 64");

  EXPECT_EQ(buf, pool.alloc(64));
  assertDump(
      pool,
      "size : 64\n"
      "  Used at        0 size 64");
}

} // namespace
