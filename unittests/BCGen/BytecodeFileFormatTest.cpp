/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/Regex/RegexBytecode.h"
#include "hermes/Support/StringTableEntry.h"

#include "llvh/Support/Endian.h"

#include "gtest/gtest.h"

#include <sstream>

// Structs with bit fields can have different layouts between GCC, CLANG,
// and MSVC. With experiments, it is observed that GCC and CLANG always
// agree, but MSVC can behave differently.
//
// With experiments, it is observed that the layout will agree if the
// following rules is observed:
// * LLVM_PACKED should be used. Otherwise, MSVC has additional alignment
//   constraints that GCC, CLANG does not have.
// * Continuous fields with the same type must use a multiple of
//   the number of bytes the type would use without bit field.
//   Holes less than 1 byte (in other words, holes up to 7 bit) is ok.
//   For example, continuous fields of type uint32_t must use 25-32,
//   57-64, 89-96, ... bits total.
// * Each field should fall entirely within the boundary of its size.
//   For example, `uint16_t a:8; uint16_t b:10; uint16_t a:14` is not ok
//   because b should either fall entirely within the first two bytes,
//   or entirely within the second two bytes.
//
// The static_assert(sizeof(X)==Y) in BytecodeFileFormat.h serves as an
// early indicator.

using namespace hermes;
using namespace hermes::hbc;
using namespace hermes::regex;
using namespace llvh::support;
using namespace llvh::support::endian;

namespace {

template <typename T>
std::string toHexString(T t) {
  std::stringstream ss;
  uint8_t bytes[sizeof(T)];
  memcpy(&bytes, &t, sizeof(T));
  for (size_t i = 0; i < sizeof(T); i++) {
    ss << std::setfill('0') << std::setw(2) << std::hex
       << static_cast<uint32_t>(bytes[i]) << " ";
  }
  std::string result = ss.str();
  result.pop_back();
  return result;
}

TEST(BytecodeFileFormatTest, BytecodeOptionsLayout) {
  BytecodeOptions o;
  o.staticBuiltins = 1;
  EXPECT_EQ(o._flags, 1);
}

TEST(BytecodeFileFormatTest, SmallStringTableEntryLayout) {
  if (system_endianness() != endianness::little) {
    // This test assumes little endianness
    return;
  }

  SmallStringTableEntry e(StringTableEntry(0, 0, 0), 0);

  e.isUTF16 = 0x1;
  EXPECT_EQ(toHexString(e), "01 00 00 00");
  e.offset = 0xFF;
  EXPECT_EQ(toHexString(e), "ff 01 00 00");
  e.offset = 0x7FFFFF;
  EXPECT_EQ(toHexString(e), "ff ff ff 00");
  e.length = 0xF;
  EXPECT_EQ(toHexString(e), "ff ff ff 0f");
  e.length = 0xFF;
  EXPECT_EQ(toHexString(e), "ff ff ff ff");
}

TEST(BytecodeFileFormatTest, FunctionHeaderFlagLayout) {
  FunctionHeaderFlag f;

  f.prohibitInvoke = 1;
  EXPECT_EQ(f.flags, 0x01);
  f.prohibitInvoke = 3;
  EXPECT_EQ(f.flags, 0x03);
  f.strictMode = 1;
  EXPECT_EQ(f.flags, 0x07);
  f.hasExceptionHandler = 1;
  EXPECT_EQ(f.flags, 0x0f);
  f.hasDebugInfo = 1;
  EXPECT_EQ(f.flags, 0x1f);
  f.overflowed = 1;
  EXPECT_EQ(f.flags, 0x3f);
}

TEST(BytecodeFileFormatTest, BracketInsnLayout) {
  if (system_endianness() != endianness::little) {
    // This test assumes little endianness
    return;
  }

  BracketInsn bi;
  memset(&bi, 0, sizeof(BracketInsn));

  bi.opcode = static_cast<Opcode>(0xf);
  EXPECT_EQ(toHexString(bi), "0f 00 00 00 00 00");
  bi.opcode = static_cast<Opcode>(0xff);
  EXPECT_EQ(toHexString(bi), "ff 00 00 00 00 00");
  bi.rangeCount = 0xfff;
  EXPECT_EQ(toHexString(bi), "ff ff 0f 00 00 00");
  bi.rangeCount = 0xffffffffu;
  EXPECT_EQ(toHexString(bi), "ff ff ff ff ff 00");
  bi.negate = 1;
  EXPECT_EQ(toHexString(bi), "ff ff ff ff ff 01");
  bi.positiveCharClasses = 1;
  EXPECT_EQ(toHexString(bi), "ff ff ff ff ff 03");
  bi.positiveCharClasses = 7;
  EXPECT_EQ(toHexString(bi), "ff ff ff ff ff 0f");
  bi.negativeCharClasses = 1;
  EXPECT_EQ(toHexString(bi), "ff ff ff ff ff 1f");
  bi.negativeCharClasses = 7;
  EXPECT_EQ(toHexString(bi), "ff ff ff ff ff 7f");
}

} // end anonymous namespace
