/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <map>
#include <string>

#include "gtest/gtest.h"

#include "hermes/Support/LEB128.h"
#include "llvh/Support/LEB128.h"
#include "llvh/Support/raw_ostream.h"

namespace {

// Strings can contain null bytes, but if they are initialised using string
// literals, then they will treat null bytes as terminators. This function
// avoids this problem by initialising the string using the list of bytes
// directly.
inline std::string ubyte_str(std::initializer_list<uint8_t> bytes) {
  return std::string(bytes.begin(), bytes.end());
}

extern const std::map<uint32_t, std::string> kSignedEncodings{
    {0x80000000, ubyte_str({0x80, 0x80, 0x80, 0x80, 0x78})},
    {0xF7FFFFFF, ubyte_str({0xFF, 0xFF, 0xFF, 0xBF, 0x7F})},
    {0xF8000000, ubyte_str({0x80, 0x80, 0x80, 0x40})},
    {0xFFEFFFFF, ubyte_str({0xFF, 0xFF, 0xBF, 0x7F})},
    {0xFFF00000, ubyte_str({0x80, 0x80, 0x40})},
    {0xFFF6789B, ubyte_str({0x9B, 0xF1, 0x59})},
    {0xFFFFDFFF, ubyte_str({0xFF, 0xBF, 0x7F})},
    {0xFFFFE000, ubyte_str({0x80, 0x40})},
    {0xFFFFFFBF, ubyte_str({0xBF, 0x7F})},
    {0xFFFFFFC0, ubyte_str({0x40})},
    {0xFFFFFFFF, ubyte_str({0x7F})},
    {0x00000000, ubyte_str({0x00})},
    {0x00000001, ubyte_str({0x01})},
    {0x0000002A, ubyte_str({0x2A})},
    {0x0000003F, ubyte_str({0x3F})},
    {0x00000040, ubyte_str({0xC0, 0x00})},
    {0x0000007F, ubyte_str({0xFF, 0x00})},
    {0x000000FF, ubyte_str({0xFF, 0x01})},
    {0x00001FFF, ubyte_str({0xFF, 0x3F})},
    {0x00002000, ubyte_str({0x80, 0xC0, 0x00})},
    {0x0000FFFF, ubyte_str({0xFF, 0xFF, 0x03})},
    {0x00098765, ubyte_str({0xE5, 0x8E, 0x26})},
    {0x000FFFFF, ubyte_str({0xFF, 0xFF, 0x3F})},
    {0x00100000, ubyte_str({0x80, 0x80, 0xC0, 0x00})},
    {0x07FFFFFF, ubyte_str({0xFF, 0xFF, 0xFF, 0x3F})},
    {0x08000000, ubyte_str({0x80, 0x80, 0x80, 0xC0, 0x00})},
};

TEST(LEB128, EncodeSLEB128) {
  for (const auto &kvp : kSignedEncodings) {
    int32_t input = static_cast<int32_t>(kvp.first);
    const std::string &expected = kvp.second;

    std::string buf = "";
    llvh::raw_string_ostream os{buf};

    hermes::encodeSLEB128(input, os);
    EXPECT_EQ(expected, os.str()) << "When writing " << input;
  }
}

TEST(LEB128, EncodeSLEB128Padded) {
  constexpr size_t kMinSize = 4;
  for (const auto &kvp : kSignedEncodings) {
    int32_t input = static_cast<int32_t>(kvp.first);
    const std::string &expected = kvp.second;

    std::string buf = "";
    llvh::raw_string_ostream os{buf};

    hermes::encodeSLEB128(input, os, kMinSize);

    std::string encoded = os.str();

    if (expected.size() <= kMinSize) {
      EXPECT_EQ(kMinSize, encoded.size()) << "Output padding " << input;
    } else {
      EXPECT_LT(kMinSize, encoded.size()) << "Output big enough " << input;
    }

    const uint8_t *cStr = reinterpret_cast<const uint8_t *>(encoded.c_str());

    unsigned written = 0;
    int64_t decoded = llvh::decodeSLEB128(cStr, &written);

    EXPECT_EQ(input, decoded) << "Encode then decode";
    EXPECT_EQ(written, encoded.size()) << "Consumed all of output";
  }
}

} // anonymous namespace
