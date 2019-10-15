/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace {

const char *updateBytecodeVersionMsg =
    "Any change to the predefined strings (additions, removals, or re-orderings"
    " -- in short any change that would cause this test to fail) should be"
    " accompanied by an update to the bytecode version.  'PredefinedStrings.lock'"
    " should also be updated to reflect the most up-to-date mapping between"
    " strings and predefined IDs, in order to make this test pass again.\n";

TEST(PredefinedStringsTest, UpdateBytecodeVersion) {
  const std::vector<std::string> expected{
#define EXPECT(string) string,
#include "PredefinedStrings.lock"
  };

  const std::vector<std::string> actual{
#define STR(name, string) string,
#include "hermes/VM/PredefinedStrings.def"
  };

  EXPECT_EQ(expected.size(), actual.size())
      << updateBytecodeVersionMsg << "\n"
      << "Number of predefined strings differs.";

  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(expected[i], actual[i]) << updateBytecodeVersionMsg << "\n"
                                      << "First mismatch at index " << i << ".";
  }
}

/// The Hermes Compiler and Runtime need to agree (statically) on the SymbolIDs
/// for Predefined Strings.  To avoid ambiguity, and avoid wasting SymbolIDs,
/// there should be no duplicate strings.
TEST(PredefinedStringsTest, NoDuplicates) {
  static const char *kPredefinedStrings[]{
#define STR(name, string) string,
#include "hermes/VM/PredefinedStrings.def"
  };

  std::unordered_set<std::string> uniqueStrings;
  size_t duplicates = 0;
  for (auto str : kPredefinedStrings) {
    bool isUnique = uniqueStrings.insert(str).second;
    duplicates += !isUnique;
    EXPECT_TRUE(isUnique) << "Duplicate Predefined String: " << str;
  }

  EXPECT_EQ(0, duplicates) << duplicates << " duplicates found";
}

} // namespace
