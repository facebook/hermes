/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/ExecHeap.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {
static void assertDump(ExecHeap &heap, const char *msg) {
  std::string str;
  llvm::raw_string_ostream OS{str};
  heap.dump(OS, true);
  OS.flush();
  // Strip prefix and suffix.
  while (!str.empty() && str.back() == '\n')
    str.pop_back();
  EXPECT_STREQ(msg, str.c_str());
}

TEST(ExecHeapTest, SmokeTest) {
  ExecHeap eh{1024 * 3, 1024, 1024 * 8};

  assertDump(
      eh,
      "== ExecHeap 3072+1024\n"
      "  maxPools:2\n"
      "  numPools:0");

  // No pools yet, this should fail.
  EXPECT_FALSE(eh.alloc({10, 20}));

  // Allocate the first pool.
  ASSERT_TRUE(eh.addPool());

  // Size 0 always should fail.
  EXPECT_FALSE(eh.alloc({0, 0}));
  // Too large size should fail.
  EXPECT_FALSE(eh.alloc({100000, 100000}));

  auto r1 = eh.alloc({10, 20});
  EXPECT_TRUE(r1);

  assertDump(
      eh,
      "== ExecHeap 3072+1024\n"
      "  maxPools:2\n"
      "  numPools:1\n"
      "First == PoolHeap\n"
      "start: 0x000000\n"
      "size : 3072\n"
      "  Used at        0 size 16\n"
      "  Free at       16 size 3056\n"
      "\n"
      "Second == PoolHeap\n"
      "start: 0x000000\n"
      "size : 1024\n"
      "  Used at        0 size 32\n"
      "  Free at       32 size 992");

  // Free the just allocated blocks.
  eh.free(*r1);

  // It should be empty again.
  assertDump(
      eh,
      "== ExecHeap 3072+1024\n"
      "  maxPools:2\n"
      "  numPools:0");

  // Try to allocate block again.
  EXPECT_FALSE(eh.alloc({100, 200}));
  // Allocate the first pool again.
  ASSERT_TRUE(eh.addPool());
  // Allocate again.
  EXPECT_TRUE(eh.alloc({100, 200}));

  // Try to allocate a second one. It should fail.
  EXPECT_FALSE(eh.alloc({1024 * 3, 1024}));

  // Allocate a second pool.
  ASSERT_TRUE(eh.addPool());

  // Try to allocate again.
  EXPECT_TRUE(eh.alloc({1024 * 3, 1024}));

  // Allocate again - it should go into the first pool.
  EXPECT_TRUE(eh.alloc({100, 200}));

  assertDump(
      eh,
      "== ExecHeap 3072+1024\n"
      "  maxPools:2\n"
      "  numPools:2\n"
      "First == PoolHeap\n"
      "start: 0x000000\n"
      "size : 3072\n"
      "  Used at        0 size 112\n"
      "  Used at      112 size 112\n"
      "  Free at      224 size 2848\n"
      "\n"
      "Second == PoolHeap\n"
      "start: 0x000000\n"
      "size : 1024\n"
      "  Used at        0 size 208\n"
      "  Used at      208 size 208\n"
      "  Free at      416 size 608\n"
      "\n"
      "First == PoolHeap\n"
      "start: 0x000000\n"
      "size : 3072\n"
      "  Used at        0 size 3072\n"
      "\n"
      "Second == PoolHeap\n"
      "start: 0x000000\n"
      "size : 1024\n"
      "  Used at        0 size 1024");

  eh.dump(llvm::errs(), true);
}

} // namespace
