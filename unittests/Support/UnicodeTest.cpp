/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/UTF8.h"

#include "gtest/gtest.h"

#include <vector>

using namespace hermes;

namespace {

// Verify correct behavior of convertUTF16ToUTF8StringWithSingleSurrogates().
TEST(StringTest, UTF16ToUTF8StringWithSingleSurrogates) {
  auto convert = [](std::initializer_list<char16_t> cs) -> std::vector<char> {
    std::string out;
    std::vector<char16_t> vec(cs);
    convertUTF16ToUTF8WithSingleSurrogates(out, cs);
    return std::vector<char>(out.begin(), out.end());
  };
  auto bytes =
      [](std::initializer_list<unsigned char> cs) -> std::vector<char> {
    return std::vector<char>(cs.begin(), cs.end());
  };
  EXPECT_EQ(bytes({}), convert({}));
  EXPECT_EQ(bytes({'a'}), convert({'a'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'e', 0xCC, 0x81}), convert({'e', 0x0301}));
  EXPECT_EQ(bytes({0xE2, 0x98, 0x83}), convert({0x2603}));

  // UTF-16 encoded U+1F639
  EXPECT_EQ(
      bytes({0xED, 0xA0, 0xBD, 0xED, 0xB8, 0xB9}), convert({0xD83D, 0xDE39}));

  // Unpaired surrogate halves
  EXPECT_EQ(bytes({0xED, 0xA0, 0xBD}), convert({0xD83D}));
  EXPECT_EQ(bytes({'a', 0xED, 0xA0, 0xBD, 'b'}), convert({'a', 0xD83D, 'b'}));
  EXPECT_EQ(bytes({'a', 0xED, 0xB8, 0xB9, 'b'}), convert({'a', 0xDE39, 'b'}));

  // Unpaired trailing surrogate halves
  EXPECT_EQ(bytes({'a', 0xED, 0xA0, 0xBD}), convert({'a', 0xD83D}));
  EXPECT_EQ(bytes({'a', 0xED, 0xB8, 0xB9}), convert({'a', 0xDE39}));
}

// Verify correct behavior of convertUTF16ToUTF8StringWithReplacements().
TEST(StringTest, UTF16ToUTF8StringWithReplacements) {
  auto convert = [](std::initializer_list<char16_t> cs) -> std::vector<char> {
    std::string out;
    std::vector<char16_t> vec(cs);
    convertUTF16ToUTF8WithReplacements(out, cs);
    return std::vector<char>(out.begin(), out.end());
  };
  auto bytes =
      [](std::initializer_list<unsigned char> cs) -> std::vector<char> {
    return std::vector<char>(cs.begin(), cs.end());
  };
  EXPECT_EQ(bytes({}), convert({}));
  EXPECT_EQ(bytes({'a'}), convert({'a'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'e', 0xCC, 0x81}), convert({'e', 0x0301}));
  EXPECT_EQ(bytes({0xE2, 0x98, 0x83}), convert({0x2603}));

  // UTF-16 encoded U+1F639
  EXPECT_EQ(bytes({0xF0, 0x9F, 0x98, 0xB9}), convert({0xD83D, 0xDE39}));

  // Unpaired surrogate halves
  EXPECT_EQ(bytes({0xEF, 0xBF, 0xBD}), convert({0xD83D}));
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD, 'b'}), convert({'a', 0xD83D, 'b'}));
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD, 'b'}), convert({'a', 0xDE39, 'b'}));

  // Unpaired trailing surrogate halves
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD}), convert({'a', 0xD83D}));
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD}), convert({'a', 0xDE39}));
}

} // end anonymous namespace
