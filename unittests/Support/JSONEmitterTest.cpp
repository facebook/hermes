/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/JSONEmitter.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(JSONEmitterTest, SmokeTest) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);

  json.openDict();
  json.emitKey("a");
  json.emitValue(123);

  json.emitKeyValue("b", 456.7);

  json.emitKey("dict1");
  json.openDict();

  json.emitKey("dict1_arr1");
  json.openArray();
  json.emitValues({"val1", "val2", "val3"});
  json.closeArray();

  json.emitKey("dict1_empty");
  json.openDict();
  json.closeDict();

  json.emitKey("dict1_empty2");
  json.openArray();
  json.closeArray();

  json.emitKeyValue("str1", "\"ABC\bDEF\\");
  json.closeDict();
  json.closeDict();

  // This intermediate variable is necessary because
  // MSVC's macro preprocessor does not behave as expected with R-literals.
  const char *expected =
      R"#({"a":123,"b":456.7,"dict1":{"dict1_arr1":["val1","val2","val3"],)#"
      R"#("dict1_empty":{},"dict1_empty2":[],"str1":"\"ABC\bDEF\\"}})#";
  EXPECT_EQ(OS.str(), expected);
}

TEST(JSONEmitterTest, EmptyArray) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openArray();
  json.closeArray();
  EXPECT_EQ(OS.str(), "[]");
}

TEST(JSONEmitterTest, EmptyDict) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openDict();
  json.closeDict();
  OS.flush();
  EXPECT_EQ(OS.str(), "{}");
}

TEST(JSONEmitterTest, Sample) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("name", "hermes");
  json.emitKeyValue("age", 2);
  json.emitKeyValue("hot", true);
  json.emitKeyValue("cold", false);
  json.emitKey("tags");
  json.openArray();
  json.emitValues({"small", "light"});
  json.closeArray();
  json.closeDict();
  EXPECT_EQ(
      OS.str(),
      R"#({"name":"hermes","age":2,"hot":true,"cold":false,"tags":["small","light"]})#");
}

TEST(JSONEmitterTest, Escapes) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.emitValue("x\"\\/\b\f\n\r\tx");
  const char *expected = R"#("x\"\\\/\b\f\n\r\tx")#";
  EXPECT_EQ(OS.str(), expected);
}

TEST(JSONEmitterTest, NonAsciiEscapes) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("ha", reinterpret_cast<const char *>(u8"\u54C8"));
  json.emitKeyValue("gClef", reinterpret_cast<const char *>(u8"\U0001D11E"));
  json.closeDict();
  const char *expected = R"#({"ha":"\u54c8","gClef":"\ud834\udd1e"})#";
  EXPECT_EQ(OS.str(), expected);
}

TEST(JSONEmitterTest, JSONL) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openDict();
  json.closeDict();
  json.endJSONL();

  json.openDict();
  json.closeDict();
  json.endJSONL();
  EXPECT_EQ(OS.str(), "{}\n{}\n");
}

TEST(JSONEmitterTest, PrettyPrint) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS, true /*pretty*/);
  json.openDict();
  json.emitKeyValue("artist", "prince");
  json.emitKey("instruments");
  json.openArray();
  json.emitValue("piano");
  json.openDict();
  json.emitKey("guitars");
  json.openArray();
  json.emitValues({"cloud", "love symbol", "telecaster"});
  json.closeArray();
  json.closeDict();
  json.emitValue("drums");
  json.closeArray();
  json.emitKey("songs");
  json.openDict();
  json.emitKeyValue("purple rain", 1984);
  json.emitKeyValue("1999", 1982);
  json.closeDict();
  json.emitKeyValue("color", "purple");
  json.emitKey("emptyDict");
  json.openDict();
  json.closeDict();
  json.emitKey("emptyArray");
  json.openArray();
  json.closeArray();
  json.closeDict();

  const char *expected = R"#({
  "artist": "prince",
  "instruments": [
    "piano",
    {
      "guitars": [
        "cloud",
        "love symbol",
        "telecaster"
      ]
    },
    "drums"
  ],
  "songs": {
    "purple rain": 1984,
    "1999": 1982
  },
  "color": "purple",
  "emptyDict": {},
  "emptyArray": []
})#";

  EXPECT_EQ(OS.str(), expected);
}

TEST(JSONEmitterTest, NonFinite) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);

  json.openArray();
  json.emitValues({INFINITY, -INFINITY, NAN});
  json.closeArray();

  // This intermediate variable is necessary because
  // MSVC's macro preprocessor does not behave as expected with R-literals.
  const char *expected = R"#([null,null,null])#";
  EXPECT_EQ(OS.str(), expected);
}

TEST(JSONEmitterTest, EmitGroupsOfForwardSlashes) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter json(OS);

  json.openDict();
  json.emitKeyValue("url", "http://www.example.com");
  json.closeDict();

  // Expect escaped forward slashes.
  const char *expected = R"#({"url":"http:\/\/www.example.com"})#";
  EXPECT_EQ(OS.str(), expected);
}
}; // anonymous namespace
