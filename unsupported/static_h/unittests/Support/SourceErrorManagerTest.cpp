/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SourceErrorManager.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(SourceErrorManagerTest, testFindSMLocFromCoords) {
  SourceErrorManager mgr{};
  auto buf =
      llvh::MemoryBuffer::getMemBuffer("01\n34567\n\n\n\n", "TEST", false);
  auto *start = buf->getBufferStart();
  auto *end = buf->getBufferEnd();

  mgr.addNewSourceBuffer(std::move(buf));

  // Test just a normal position first.
  SourceErrorManager::SourceCoords coords;
  auto loc1 = SMLoc::getFromPointer(start + 5);
  mgr.findBufferLineAndLoc(loc1, coords);
  ASSERT_TRUE(coords.isValid());
  ASSERT_EQ(2, coords.line);
  ASSERT_EQ(3, coords.col);

  auto loc2 = mgr.findSMLocFromCoords(coords);
  ASSERT_EQ(loc1, loc2);

  // Test column 1 in an empty line.
  loc1 = SMLoc::getFromPointer(start + 10);
  mgr.findBufferLineAndLoc(loc1, coords);
  ASSERT_TRUE(coords.isValid());
  ASSERT_EQ(4, coords.line);
  ASSERT_EQ(1, coords.col);

  loc2 = mgr.findSMLocFromCoords(coords);
  ASSERT_EQ(loc1, loc2);

  // Make it the last line.
  loc1 = SMLoc::getFromPointer(start + 11);
  mgr.findBufferLineAndLoc(loc1, coords);
  ASSERT_TRUE(coords.isValid());
  ASSERT_EQ(5, coords.line);
  ASSERT_EQ(1, coords.col);

  loc2 = mgr.findSMLocFromCoords(coords);
  ASSERT_EQ(loc1, loc2);

  // Some errors show unexpected EOF at one past the last character.
  loc1 = SMLoc::getFromPointer(end);
  mgr.findBufferLineAndLoc(loc1, coords);
  ASSERT_TRUE(coords.isValid());
  ASSERT_EQ(6, coords.line);
  ASSERT_EQ(1, coords.col);

  loc2 = mgr.findSMLocFromCoords(coords);
  ASSERT_EQ(loc1, loc2);
}

} // end anonymous namespace
