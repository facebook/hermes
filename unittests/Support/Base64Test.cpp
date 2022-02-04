/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/Base64.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(Base64Test, DecodePadded) {
  EXPECT_EQ(base64Decode("YQ==").getValue(), "a");
  EXPECT_EQ(base64Decode("YWI=").getValue(), "ab");
  EXPECT_EQ(base64Decode("YWJj").getValue(), "abc");
  EXPECT_EQ(base64Decode("YWJjZA==").getValue(), "abcd");
  EXPECT_EQ(base64Decode("YWJjZGU=").getValue(), "abcde");
  EXPECT_EQ(base64Decode("YWJjZGVm").getValue(), "abcdef");
}

TEST(Base64Test, DecodeUnpadded) {
  // Only 2 or 3 chars can remain
  EXPECT_EQ(base64Decode("Y"), llvh::None);
  EXPECT_EQ(base64Decode("YQ").getValue(), "a");
  EXPECT_EQ(base64Decode("YWI").getValue(), "ab");
  EXPECT_EQ(base64Decode("YWJj").getValue(), "abc");
  EXPECT_EQ(base64Decode("YWJjZ"), llvh::None);
  EXPECT_EQ(base64Decode("YWJjZA").getValue(), "abcd");
  EXPECT_EQ(base64Decode("YWJjZGU").getValue(), "abcde");
  EXPECT_EQ(base64Decode("YWJjZGVm").getValue(), "abcdef");
}

TEST(Base64Test, DecodeInvalid) {
  // Non-Base64 Ascii, i.e., not [a-z]|[A-Z]|[0-p]|\+|\/
  EXPECT_EQ(base64Decode("*"), llvh::None);
  EXPECT_EQ(base64Decode("YQ*"), llvh::None);

  // Out of Ascii range.
  EXPECT_EQ(base64Decode("\xFF"), llvh::None);

  // Padding in the middle
  EXPECT_EQ(base64Decode("YQ==YQ=="), llvh::None);

  // Extra padding
  EXPECT_EQ(base64Decode("YQ==="), llvh::None);
}

TEST(DataURLTst, ParseDataURL) {
  EXPECT_EQ(
      parseJSONBase64DataURL("data:application/json;base64,YQ==").getValue(),
      "a");

  // Invalid data
  EXPECT_EQ(
      parseJSONBase64DataURL("data:application/json;base64,Y"), llvh::None);

  // Missing [;base64]
  EXPECT_EQ(parseJSONBase64DataURL("data:,YQ=="), llvh::None);
  EXPECT_EQ(parseJSONBase64DataURL("data:text/plain,YQ=="), llvh::None);

  // Invalid mediatype
  EXPECT_EQ(parseJSONBase64DataURL("data:html,YQ"), llvh::None);
  EXPECT_EQ(parseJSONBase64DataURL("data:text/plain,YQ=="), llvh::None);
}

} // end anonymous namespace
