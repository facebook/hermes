/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMapGenerator.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/Support/Base64vlq.h"

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

// Empty "sourceRoot" field.
const char *TestMapEmptyLines = R"#({
  "version": 3,
  "file": "min.js",
  "names": ["bar", "baz", "n"],
  "sources": ["one.js", "two.js"],
  "mappings": ";;CAAC,IAAI,IAAM,SAAUA,GAClB,OAAOC,IAAID;CCDb,IAAI,IAAM,SAAUE,GAClB,OAAOA"
})#";

/// Helper to return a Segment.
SourceMap::Segment loc(
    int32_t address /* 0-based */,
    int32_t sourceIndex,
    int32_t line /* 1-based */,
    int32_t column /* 0-based */,
    llvm::Optional<int32_t> nameIndex = llvm::None) {
  return SourceMap::Segment{address, sourceIndex, line - 1, column, nameIndex};
}

/// Helper to return a segment with no represented location.
SourceMap::Segment loc(int32_t address) {
  SourceMap::Segment seg;
  seg.generatedColumn = address;
  return seg;
}

void verifySegment(
    SourceMap &sourceMap,
    int generatedLine,
    const std::vector<std::string> &sources,
    const SourceMap::Segment &segment) {
  llvm::Optional<SourceMapTextLocation> locOpt =
      sourceMap.getLocationForAddress(
          generatedLine, segment.generatedColumn + 1);
  if (segment.representedLocation.hasValue()) {
    EXPECT_TRUE(locOpt.hasValue());
    EXPECT_EQ(
        locOpt.getValue().fileName,
        sources[segment.representedLocation->sourceIndex]);
    EXPECT_EQ(
        locOpt.getValue().line, segment.representedLocation->lineIndex + 1);
    EXPECT_EQ(
        locOpt.getValue().column, segment.representedLocation->columnIndex + 1);
  } else {
    EXPECT_FALSE(locOpt.hasValue());
  }
}

std::unique_ptr<SourceMap> parseSourceMap(llvm::StringRef sourceMapContent) {
  SourceMapParser parser;
  std::unique_ptr<SourceMap> sourceMap = parser.parse(sourceMapContent);
  EXPECT_TRUE(sourceMap != nullptr);
  return sourceMap;
}

TEST(SourceMap, Basic) {
  SourceMapGenerator map;
  EXPECT_EQ(map.getMappingsLines().size(), 0u);

  std::vector<std::string> sources{"file1", "file2"};
  map.setSources(sources);

  std::vector<SourceMap::Segment> segmentsList[] = {
      {
          loc(0, 0, 1, 1), // addr 1:0 -> file1:1:1
          loc(2, 0, 2, 1), // addr 1:2 -> file1:2:1
          loc(3, 0, 3, 1), // addr 1:3 -> file1:3:1
          loc(4), //          addr 1:4 -> unmapped
          loc(5, 0, 3, 2), // addr 1:5 -> file1:3:2
      },
      {
          loc(0, 1, 6, 6), // addr 2:0 -> file2:6:6
          loc(1, 1, 7, 2), // addr 2:1 -> file2:7:2
          loc(3, 1, 7, 3), // addr 2:3 -> file2:7:3
          loc(5, 1, 8, 1), // addr 2:5 -> file2:8:1
      }};

  uint32_t i = 0;
  for (const auto &segments : segmentsList) {
    map.addMappingsLine(segments, i++);
  }

  ASSERT_EQ(map.getMappingsLines().size(), 2u);
  EXPECT_EQ(map.getMappingsLines()[0].size(), 5u);
  EXPECT_EQ(map.getMappingsLines()[1].size(), 4u);

  std::string storage;
  llvm::raw_string_ostream OS(storage);
  map.outputAsJSON(OS);
  EXPECT_EQ(
      OS.str(),
      R"#({"version":3,"sources":["file1","file2"],"mappings":"AAAC,EACA,CACA,C,CAAC;ACGI,CACJ,EAAC,EACF;"})#");

  std::unique_ptr<SourceMap> sourceMap = parseSourceMap(storage);
  for (uint32_t line = 0; line < sizeof(segmentsList) / sizeof(segmentsList[0]);
       ++line) {
    const auto &segments = segmentsList[line];
    for (uint32_t i = 0; i < segments.size(); ++i) {
      verifySegment(
          *sourceMap, /*generatedLine*/ line + 1, sources, segments[i]);
    }
  }
}

/// "test that the `sources` field has the original sources" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, SourcesField) {
  auto verifySources = [](const char *sourceMapContent,
                          const std::vector<std::string> &expected) {
    std::unique_ptr<SourceMap> sourceMap = parseSourceMap(sourceMapContent);
    std::vector<std::string> sources = sourceMap->getAllFullPathSources();
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
  std::unique_ptr<SourceMap> sourceMap = parseSourceMap(TestMap);

  llvm::Optional<SourceMapTextLocation> locOpt =
      sourceMap->getLocationForAddress(2, 2);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "/the/root/two.js");

  locOpt = sourceMap->getLocationForAddress(1, 2);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "/the/root/one.js");

  std::unique_ptr<SourceMap> sourceMap2 = parseSourceMap(TestMapNoSourceRoot);

  locOpt = sourceMap2->getLocationForAddress(2, 2);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "two.js");

  locOpt = sourceMap2->getLocationForAddress(1, 2);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "one.js");

  std::unique_ptr<SourceMap> sourceMap3 =
      parseSourceMap(TestMapEmptySourceRoot);

  locOpt = sourceMap3->getLocationForAddress(2, 2);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "two.js");

  locOpt = sourceMap3->getLocationForAddress(1, 2);
  EXPECT_TRUE(locOpt.hasValue());
  EXPECT_EQ(locOpt.getValue().fileName, "one.js");
};

/// "test mapping tokens back exactly" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, ExactMappings) {
  std::unique_ptr<SourceMap> sourceMap = parseSourceMap(TestMap);

  std::vector<std::string> sources = {"/the/root/one.js", "/the/root/two.js"};

  int generatedLine = 1;
  int sourceIndex = 0;
  verifySegment(*sourceMap, generatedLine, sources, loc(1, sourceIndex, 1, 1));
  verifySegment(*sourceMap, generatedLine, sources, loc(5, sourceIndex, 1, 5));
  verifySegment(*sourceMap, generatedLine, sources, loc(9, sourceIndex, 1, 11));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(18, sourceIndex, 1, 21));
  verifySegment(*sourceMap, generatedLine, sources, loc(21, sourceIndex, 2, 3));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(28, sourceIndex, 2, 10));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(32, sourceIndex, 2, 14));

  generatedLine = 2;
  sourceIndex = 1;
  verifySegment(*sourceMap, generatedLine, sources, loc(1, sourceIndex, 1, 1));
  verifySegment(*sourceMap, generatedLine, sources, loc(5, sourceIndex, 1, 5));
  verifySegment(*sourceMap, generatedLine, sources, loc(9, sourceIndex, 1, 11));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(18, sourceIndex, 1, 21));
  verifySegment(*sourceMap, generatedLine, sources, loc(21, sourceIndex, 2, 3));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(28, sourceIndex, 2, 10));
};

/// "test mapping tokens fuzzy" from
/// https://github.com/mozilla/source-map/blob/master/test/test-source-map-consumer.js
TEST(SourceMap, FuzzyMappings) {
  std::unique_ptr<SourceMap> sourceMap = parseSourceMap(TestMap);

  std::vector<std::string> sources = {"/the/root/one.js", "/the/root/two.js"};

  verifySegment(*sourceMap, 1, sources, loc(20, 0, 1, 21));
  verifySegment(*sourceMap, 1, sources, loc(30, 0, 2, 10));
  verifySegment(*sourceMap, 2, sources, loc(12, 1, 1, 11));
};

/// Test to make sure we can parse mappings with no represented location
TEST(SourceMap, NoRepresentedLocation) {
  std::unique_ptr<SourceMap> sourceMap = parseSourceMap(
      R"#({
        "version": 3,
        "sources": ["a.js", "b.js"],
        "mappings": "CACC,E,G;A,A,CCCC"
      })#");

  std::vector<std::string> sources = {"a.js", "b.js"};

  int generatedLine = 1;
  int sourceIndex = 0;
  verifySegment(*sourceMap, generatedLine, sources, loc(1, sourceIndex, 2, 1));
  verifySegment(*sourceMap, generatedLine, sources, loc(3));
  verifySegment(*sourceMap, generatedLine, sources, loc(6));

  generatedLine = 2;
  sourceIndex = 1;
  verifySegment(*sourceMap, generatedLine, sources, loc(0));
  verifySegment(*sourceMap, generatedLine, sources, loc(0));
  verifySegment(*sourceMap, generatedLine, sources, loc(1, sourceIndex, 3, 2));
};

/// Test to make sure we can properly parse empty lines.
TEST(SourceMap, EmptyLines) {
  std::unique_ptr<SourceMap> sourceMap = parseSourceMap(TestMapEmptyLines);

  std::vector<std::string> sources = {"one.js", "two.js"};

  int generatedLine = 3;
  int sourceIndex = 0;
  verifySegment(*sourceMap, generatedLine, sources, loc(1, sourceIndex, 1, 1));
  verifySegment(*sourceMap, generatedLine, sources, loc(5, sourceIndex, 1, 5));
  verifySegment(*sourceMap, generatedLine, sources, loc(9, sourceIndex, 1, 11));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(18, sourceIndex, 1, 21));
  verifySegment(*sourceMap, generatedLine, sources, loc(21, sourceIndex, 2, 3));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(28, sourceIndex, 2, 10));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(32, sourceIndex, 2, 14));

  generatedLine = 4;
  sourceIndex = 1;
  verifySegment(*sourceMap, generatedLine, sources, loc(1, sourceIndex, 1, 1));
  verifySegment(*sourceMap, generatedLine, sources, loc(5, sourceIndex, 1, 5));
  verifySegment(*sourceMap, generatedLine, sources, loc(9, sourceIndex, 1, 11));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(18, sourceIndex, 1, 21));
  verifySegment(*sourceMap, generatedLine, sources, loc(21, sourceIndex, 2, 3));
  verifySegment(
      *sourceMap, generatedLine, sources, loc(28, sourceIndex, 2, 10));
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
