/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/JSONEmitter.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(JSONEmitterTest, SmokeTest) {
  std::string storage;
  llvm::raw_string_ostream OS(storage);
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
  llvm::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openArray();
  json.closeArray();
  EXPECT_EQ(OS.str(), "[]");
}

TEST(JSONEmitterTest, EmptyDict) {
  std::string storage;
  llvm::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openDict();
  json.closeDict();
  OS.flush();
  EXPECT_EQ(OS.str(), "{}");
}

TEST(JSONEmitterTest, Sample) {
  std::string storage;
  llvm::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("name", "hermes");
  json.emitKeyValue("age", 2);
  json.emitKeyValue("hot", true);
  json.emitKeyValue("cold", false);
  json.emitKey("platforms");
  json.openArray();
  json.emitValues({"ios", "android"});
  json.closeArray();
  json.closeDict();
  EXPECT_EQ(
      OS.str(),
      R"#({"name":"hermes","age":2,"hot":true,"cold":false,"platforms":["ios","android"]})#");
}

TEST(JSONEmitterTest, Escapes) {
  std::string storage;
  llvm::raw_string_ostream OS(storage);
  JSONEmitter json(OS);
  json.emitValue("x\b\f\n\r\tx");
  EXPECT_EQ(OS.str(), R"#("x\b\f\n\r\tx")#");
}

TEST(JSONEmitterTest, JSONL) {
  std::string storage;
  llvm::raw_string_ostream OS(storage);
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
  llvm::raw_string_ostream OS(storage);
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

}; // anonymous namespace
