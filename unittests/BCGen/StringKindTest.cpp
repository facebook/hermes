/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "hermes/BCGen/HBC/StringKind.h"

namespace {

using namespace hermes;

TEST(StringKindTest, Accumulator) {
  StringKind::Accumulator acc;

  acc.push_back(StringKind::String);
  acc.push_back(StringKind::String);
  acc.push_back(StringKind::String);
  acc.push_back(StringKind::Identifier);
  acc.push_back(StringKind::Identifier);

  std::vector<StringKind::Entry> expected{
      {StringKind::String, 3},
      {StringKind::Identifier, 2},
  };

  auto entries = acc.entries();

  EXPECT_EQ(expected.size(), entries.size());
  for (size_t i = 0; i < entries.size(); ++i) {
    EXPECT_EQ(entries[i].kind(), expected[i].kind()) << "at index " << i;
    EXPECT_EQ(entries[i].count(), expected[i].count()) << "at index " << i;
  }
}

} // namespace
