/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::hbc;

namespace {
using Loc = DebugSourceLocation;

void checkAddress(
    DebugInfo *info,
    uint32_t offset,
    uint32_t address,
    uint32_t file,
    uint32_t line,
    uint32_t column,
    uint32_t statement) {
  auto location = info->getLocationForAddress(offset, address);
  ASSERT_TRUE(location.hasValue());
  EXPECT_EQ(file, location->filenameId);
  EXPECT_EQ(line, location->line);
  EXPECT_EQ(column, location->column);
  EXPECT_EQ(statement, location->statement);
}

static DebugInfoGenerator makeGenerator() {
  UniquingFilenameTable files;
  files.addFilename("file1.js");
  files.addFilename("file2.js");
  return DebugInfoGenerator{std::move(files)};
}

TEST(DebugInfo, TestBasicInfo) {
  auto dbg = makeGenerator();

  auto debugOffset = dbg.appendSourceLocations(
      Loc{0, 1, 1, 1, 0}, // Method starts in file1:1,1
      0,
      {
          Loc{0, 1, 2, 1, 1}, // Opcode at address 0 is file1:2,1
          Loc{2, 1, 3, 1, 1} // Opcode at address 2 is file1:3,1
      });

  DebugInfo info = dbg.serializeWithMove();

  checkAddress(&info, debugOffset, 0, 1, 2, 1, 1);
  checkAddress(&info, debugOffset, 2, 1, 3, 1, 1);
}

TEST(DebugInfo, TestMultipleMethods) {
  auto dbg = makeGenerator();

  auto offset1 = dbg.appendSourceLocations(
      Loc{0, 1, 1, 1, 0}, 0, {Loc{2, 1, 1, 4, 1}, Loc{4, 1, 3, 4, 1}});

  auto offset2 = dbg.appendSourceLocations(
      Loc{0, 1, 100, 1, 0},
      0,
      {Loc{2, 1, 101, 4, 1}, Loc{8, 1, 102, 4, 1}, Loc{16, 1, 103, 4, 1}});

  DebugInfo info = dbg.serializeWithMove();

  checkAddress(&info, offset1, 2, 1, 1, 4, 1);
  checkAddress(&info, offset2, 8, 1, 102, 4, 1);

  checkAddress(&info, offset1, 4, 1, 3, 4, 1);
  checkAddress(&info, offset2, 16, 1, 103, 4, 1);
}

TEST(DebugInfo, TestMultipleFiles) {
  auto dbg = makeGenerator();

  auto offset = dbg.appendSourceLocations(
      Loc{0, 1111, 1, 1, 0}, // Method starts in file #1111
      0, // 0th function
      {
          Loc{2, 2222, 1, 1, 1}, // Continues in #2222
          Loc{4, 1111, 1, 2, 1}, // Back to 1111
          Loc{6, 2222, 1, 2, 1} // Back to 2222
      });

  DebugInfo info = dbg.serializeWithMove();

  checkAddress(&info, offset, 0, 1111, 1, 1, 0);
  checkAddress(&info, offset, 2, 2222, 1, 1, 1);
  checkAddress(&info, offset, 4, 1111, 1, 2, 1);
  checkAddress(&info, offset, 6, 2222, 1, 2, 1);
}

TEST(DebugInfo, TestLargeDeltas) {
  for (uint32_t i = 0; i < INT32_MAX; i += 123457) {
    auto dbg = makeGenerator();
    auto offset = dbg.appendSourceLocations(
        Loc{0, 1, 1, 1, 0}, 0, {Loc{2, i, i, i, 1}, Loc{4, 1, 2, 2, 1}});

    DebugInfo info = dbg.serializeWithMove();

    checkAddress(&info, offset, 0, 1, 1, 1, 0);
    checkAddress(&info, offset, 2, i, i, i, 1);
    checkAddress(&info, offset, 4, 1, 2, 2, 1);
  }
}

TEST(DebugInfo, TestGetAddress) {
  // Smoke test to make sure that the getAddressForLocation works.
  auto dbg = makeGenerator();

  dbg.appendSourceLocations(
      Loc{0, 42, 1, 1, 0}, // Function is in file 42
      3, // function 3
      {
          Loc{0, 42, 2, 1, 1}, // opcode 0 is at file42:2:1
          Loc{2, 42, 3, 1, 1}, // opcode 2 is at file42:3:1
      });

  dbg.appendSourceLocations(
      Loc{0, 12, 1, 1, 0}, // Function is in file 12
      0, // function 0
      {
          Loc{0, 12, 6, 1, 1}, // opcode 0 is at file12:6:1
          Loc{4, 12, 8, 1, 1}, // opcode 4 is at file12:8:1
      });
  dbg.appendSourceLocations(
      Loc{0, 12, 1, 1, 0}, // Function is in file 12
      3, // function 3
      {
          Loc{0, 12, 2, 1, 1}, // opcode 0 is at file12:2:1
          Loc{2, 12, 3, 1, 1}, // opcode 2 is at file12:3:1
      });

  DebugInfo info = dbg.serializeWithMove();

  OptValue<DebugSearchResult> result;

  result = info.getAddressForLocation(42, 3, 1);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(3u, result->functionIndex);
  EXPECT_EQ(2u, result->bytecodeOffset);

  result = info.getAddressForLocation(42, 6, 1);
  ASSERT_FALSE(result.hasValue());

  result = info.getAddressForLocation(12, 8, 1);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(0u, result->functionIndex);
  EXPECT_EQ(4u, result->bytecodeOffset);

  result = info.getAddressForLocation(12, 3, 1);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(3u, result->functionIndex);
  EXPECT_EQ(2u, result->bytecodeOffset);
}
} // end anonymous namespace
