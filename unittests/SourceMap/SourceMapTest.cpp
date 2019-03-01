/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMap.h"

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

/// Test data from:
/// https://github.com/mozilla/source-map/blob/master/test/util.js
const char *TestMap = R"#({
  "version": 3,
  "file": "min.js",
  "names": ["bar", "baz", "n"],
  "sources": ["one.js", "two.js"],
  "sourceRoot": "/the/root/",
  "mappings": "CAAC,IAAI,IAAM,SAAUA,GAClB,OAAOC,IAAID;CCDb,IAAI,IAAM,SAAUE,GAClB,OAAOA"
})#";

// No "sourceRoot" field.
const char *TestMapNoSourceRoot = R"#({
  "version": 3,
  "file": "min.js",
  "names": ["bar", "baz", "n"],
  "sources": ["one.js", "two.js"],
  "mappings": "CAAC,IAAI,IAAM,SAAUA,GAClB,OAAOC,IAAID;CCDb,IAAI,IAAM,SAAUE,GAClB,OAAOA"
})#";

// Empty "sourceRoot" field.
const char *TestMapEmptySourceRoot = R"#({
  "version": 3,
  "file": "min.js",
  "names": ["bar", "baz", "n"],
  "sources": ["one.js", "two.js"],
  "sourceRoot": "",
  "mappings": "CAAC,IAAI,IAAM,SAAUA,GAClB,OAAOC,IAAID;CCDb,IAAI,IAAM,SAAUE,GAClB,OAAOA"
})#";

/// Helper to return a Segment.
SourceMapGenerator::Segment
loc(int32_t address, int32_t sourceIndex, int32_t line, int32_t column) {
  return SourceMapGenerator::Segment{address, sourceIndex, line, column, 0};
}

void verifySegment(
    SourceMapParser &parser,
    int generatedLine,
    const std::vector<std::string> &sources,
    const SourceMapGenerator::Segment &segment) {
  llvm::Optional<SourceMapTextLocation> locOpt =
      parser.getLocationForAddress(generatedLine, segment.generatedColumn);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, sources[segment.sourceIndex]);
  EXPECT_EQ(locOpt.getValue().line, segment.representedLine);
  EXPECT_EQ(locOpt.getValue().column, segment.representedColumn);
}

TEST(SourceMap, Basic) {
  SourceMapGenerator map;
  EXPECT_EQ(map.getMappingsLines().size(), 0u);

  std::vector<std::string> sources{"file1", "file2"};
  map.setSources(sources);

  std::vector<SourceMapGenerator::Segment> segmentsList[] = {
      {
          loc(0, 0, 1, 1), // addr 1:0 -> file1:1:1
          loc(2, 0, 2, 1), // addr 1:2 -> file1:2:1
          loc(3, 0, 3, 1), // addr 1:3 -> file1:3:1
          loc(5, 0, 3, 2), // addr 1:5 -> file1:3:2
      },
      {
          loc(0, 1, 6, 6), // addr 2:0 -> file2:6:6
          loc(1, 1, 7, 2), // addr 2:1 -> file2:7:2
          loc(3, 1, 7, 3), // addr 2:3 -> file2:7:3
          loc(5, 1, 8, 1), // addr 2:5 -> file2:8:1
      }};

  for (const auto &segments : segmentsList) {
    map.addMappingsLine(segments);
  }

  ASSERT_EQ(map.getMappingsLines().size(), 2u);
  EXPECT_EQ(map.getMappingsLines()[0].size(), 4u);
  EXPECT_EQ(map.getMappingsLines()[1].size(), 4u);

  std::string storage;
  llvm::raw_string_ostream OS(storage);
  map.outputAsJSON(OS);
  EXPECT_EQ(
      OS.str(),
      R"#({"version":3,"sources":["file1","file2"],"mappings":"AAAC,EACA,CACA,EAAC;ACGI,CACJ,EAAC,EACF;"})#");

  SourceMapParser parser;
  EXPECT_TRUE(parser.parse(storage));

  for (uint32_t line = 0; line < sizeof(segmentsList) / sizeof(segmentsList[0]);
       ++line) {
    const auto &segments = segmentsList[line];
    for (uint32_t i = 0; i < segments.size(); ++i) {
      verifySegment(parser, /*generatedLine*/ line + 1, sources, segments[i]);
    }
  }
}

/// "test that the `sources` field has the original sources" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, SourcesField) {
  auto verifySources = [](const char *sourceMap,
                          const std::vector<std::string> &expected) {
    SourceMapParser parser;
    EXPECT_TRUE(parser.parse(sourceMap));

    std::vector<std::string> sources = parser.getAllFullPathSources();
    EXPECT_EQ(sources.size(), expected.size());
    for (uint32_t i = 0; i < expected.size(); ++i) {
      EXPECT_EQ(sources[i], expected[i]);
    }
  };
  verifySources(TestMap, {"/the/root/one.js", "/the/root/two.js"});
  verifySources(TestMapNoSourceRoot, {"one.js", "two.js"});
  verifySources(TestMapEmptySourceRoot, {"one.js", "two.js"});
};

/// "test that the source root is reflected in a mapping's source field" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, SourceRoot) {
  SourceMapParser parser;
  EXPECT_TRUE(parser.parse(TestMap));

  llvm::Optional<SourceMapTextLocation> locOpt =
      parser.getLocationForAddress(2, 1);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "/the/root/two.js");

  locOpt = parser.getLocationForAddress(1, 1);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "/the/root/one.js");

  SourceMapParser parser2;
  EXPECT_TRUE(parser2.parse(TestMapNoSourceRoot));

  locOpt = parser2.getLocationForAddress(2, 1);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "two.js");

  locOpt = parser2.getLocationForAddress(1, 1);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "one.js");

  SourceMapParser parser3;
  EXPECT_TRUE(parser3.parse(TestMapEmptySourceRoot));

  locOpt = parser3.getLocationForAddress(2, 1);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "two.js");

  locOpt = parser3.getLocationForAddress(1, 1);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "one.js");
};

/// "test mapping tokens back exactly" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, ExactMappings) {
  SourceMapParser parser;
  EXPECT_TRUE(parser.parse(TestMap));

  std::vector<std::string> sources = {"/the/root/one.js", "/the/root/two.js"};

  int generatedLine = 1;
  int sourceIndex = 0;
  verifySegment(parser, generatedLine, sources, loc(1, sourceIndex, 1, 1));
  verifySegment(parser, generatedLine, sources, loc(5, sourceIndex, 1, 5));
  verifySegment(parser, generatedLine, sources, loc(9, sourceIndex, 1, 11));
  verifySegment(parser, generatedLine, sources, loc(18, sourceIndex, 1, 21));
  verifySegment(parser, generatedLine, sources, loc(21, sourceIndex, 2, 3));
  verifySegment(parser, generatedLine, sources, loc(28, sourceIndex, 2, 10));
  verifySegment(parser, generatedLine, sources, loc(32, sourceIndex, 2, 14));

  generatedLine = 2;
  sourceIndex = 1;
  verifySegment(parser, generatedLine, sources, loc(1, sourceIndex, 1, 1));
  verifySegment(parser, generatedLine, sources, loc(5, sourceIndex, 1, 5));
  verifySegment(parser, generatedLine, sources, loc(9, sourceIndex, 1, 11));
  verifySegment(parser, generatedLine, sources, loc(18, sourceIndex, 1, 21));
  verifySegment(parser, generatedLine, sources, loc(21, sourceIndex, 2, 3));
  verifySegment(parser, generatedLine, sources, loc(28, sourceIndex, 2, 10));
};

/// "test mapping tokens fuzzy" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, FuzzyMappings) {
  SourceMapParser parser;
  EXPECT_TRUE(parser.parse(TestMap));

  std::vector<std::string> sources = {"/the/root/one.js", "/the/root/two.js"};

  verifySegment(parser, 1, sources, loc(20, 0, 1, 21));
  verifySegment(parser, 1, sources, loc(30, 0, 2, 10));
  verifySegment(parser, 2, sources, loc(12, 1, 1, 11));
};

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
