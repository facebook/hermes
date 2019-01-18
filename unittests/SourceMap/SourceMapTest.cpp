/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMap.h"

#include "llvm/Support/raw_ostream.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

/// Helper to return a Segment.
SourceMapGenerator::Segment
loc(int32_t address, int32_t sourceIndex, int32_t line, int32_t column) {
  return SourceMapGenerator::Segment{address, sourceIndex, line, column, 0};
}

TEST(SourceMap, Basic) {
  SourceMapGenerator map;
  EXPECT_EQ(map.getMappingsLines().size(), 0u);

  map.setSources({"file1"});
  map.addMappingsLine({
      loc(0, 0, 1, 1), // function 0 -> 1:1
      loc(0, 0, 2, 1), // addr 0 -> 2:1
      loc(3, 0, 3, 1), // addr 3 -> 3:1
      loc(5, 0, 3, 2), // addr 5 -> 3:2
  });
  map.addMappingsLine({
      loc(0, 1, 6, 6), // addr 0 -> 6:6
      loc(0, 1, 7, 2), // addr 0 -> 2:1
      loc(3, 1, 7, 3), // addr 3 -> 3:1
      loc(5, 1, 8, 1), // addr 5 -> 3:2
  });

  ASSERT_EQ(map.getMappingsLines().size(), 2u);
  EXPECT_EQ(map.getMappingsLines()[0].size(), 4u);
  EXPECT_EQ(map.getMappingsLines()[1].size(), 4u);

  std::string storage;
  llvm::raw_string_ostream OS(storage);
  map.outputAsJSON(OS);
  EXPECT_EQ(
      OS.str(),
      R"#({"version":3,"sources":["file1"],"mappings":"AAAC,AACA,GACA,EAAC;ACGI,AACJ,GAAC,EACF;"})#");
}

TEST(SourceMap, VLQRandos) {
  // clang-format off
  const std::vector<int32_t> inputs = {0, 1, -1, 2, -2, 5298, -23498,
                                       INT_MIN/2, INT_MAX/2, INT_MIN+1,
                                       INT_MAX-1, INT_MIN, INT_MAX, INT_MIN+1,
                                       0, 100, 0, 100, 42};
  // clang-format on

  // Encode every input into the string.
  std::string storage;
  llvm::raw_string_ostream OS(storage);
  for (int32_t x : inputs)
    base64vlq::encode(OS, x);
  OS.flush();

  // Decode from the string.
  std::vector<int32_t> outputs;
  const char *begin = storage.c_str();
  const char *end = begin + storage.size();
  while (auto decoded = base64vlq::decode(begin, end))
    outputs.push_back(*decoded);
  EXPECT_EQ(outputs, inputs);
}

TEST(SourceMap, VLQDecodeInvalids) {
  auto decode = [](const char *s) -> OptValue<int32_t> {
    return base64vlq::decode(s, s + strlen(s));
  };
  auto opt = [](int32_t x) -> OptValue<int32_t> { return x; };
  const OptValue<int32_t> none{llvm::None};
  EXPECT_EQ(opt(INT32_MAX), decode("+/////D")); // 2**31-1
  EXPECT_EQ(opt(-INT32_MAX), decode("//////D")); // -2**31+1
  // note http://www.murzwin.com/base64vlq.html gets this wrong!
  EXPECT_EQ(opt(INT32_MIN), decode("hgggggE"));
  EXPECT_EQ(none, decode("jgggggE")); // -2**31-1.
  EXPECT_EQ(opt(1 << 30), decode("ggggggC")); // 2**30
  EXPECT_EQ(none, decode("gggggggC")); // Something very big and positive.
  EXPECT_EQ(none, decode("hggggggC")); // Something very big and negative.
  EXPECT_EQ(none, decode(""));
  EXPECT_EQ(none, decode("!"));
  EXPECT_EQ(opt(1024), decode("ggC"));
  EXPECT_EQ(none, decode(" ggC"));
  EXPECT_EQ(none, decode("//")); // too many continuation bits.
  EXPECT_EQ(none, decode("67")); // too many continuation bits.

  EXPECT_EQ(opt(0), decode("A")); // plain old zero
  // SourceMap encoding is one's complement, so this represents an integer
  // "negative zero." Treat it as zero.
  EXPECT_EQ(opt(0), decode("B"));
}

} // end anonymous namespace
