/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "gtest/gtest.h"

#include <unordered_set>

namespace {

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
