/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/JSONEmitter.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::parser;

namespace {

TEST(JSONParserTest, SmokeTest1) {
  static const char json[] =
      "{\n"
      "  '6': null,\n"
      "  '1': null,\n"
      "  '2': null,\n"
      "  '3': null,\n"
      "  '4': null,\n"
      "  '5': null\n"
      "}";

  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  JSONParser parser(factory, json, sm);

  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
}

TEST(JSONParserTest, SmokeTest2) {
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  JSONParser parser(
      factory,
      "{"
      " 'key1' : 1,"
      " 'key2' : 'value2',"
      " 'key3' : {'nested1': true},"
      " \"key4\" : [false, null, 'value2']"
      "}",
      sm);

  // Check basic assumptions.
  ASSERT_STREQ("key4", factory.getString("key4")->c_str());
  ASSERT_STREQ("key3", factory.getString("key3")->c_str());
  ASSERT_STREQ("key2", factory.getString("key2")->c_str());
  ASSERT_STREQ("key1", factory.getString("key1")->c_str());
  ASSERT_STREQ("key0", factory.getString("key0")->c_str());
  ASSERT_EQ(factory.getNumber(1.0)->getValue(), 1.0);
  ASSERT_EQ(factory.getString("key2"), factory.getString("key2"));

  // Parse an object.
  auto t1 = parser.parse();
  ASSERT_TRUE(t1.hasValue());
  auto *o1 = llvh::dyn_cast<JSONObject>(t1.getValue());
  ASSERT_TRUE(nullptr != o1);

  ASSERT_EQ(4u, o1->size());

  ASSERT_EQ(0u, o1->count("key0"));
  ASSERT_EQ(o1->end(), o1->find("key0"));

  ASSERT_EQ(1u, o1->count("key1"));
  auto kv1 = o1->find("key1");
  ASSERT_NE(o1->end(), kv1);
  ASSERT_EQ((*kv1).first, factory.getString("key1"));
  ASSERT_EQ((*kv1).second, factory.getNumber(1.0));
  ASSERT_EQ(o1->at("key1"), (*kv1).second);
  ASSERT_EQ(o1->operator[]("key1"), (*kv1).second);

  ASSERT_EQ(1u, o1->count("key2"));
  auto kv2 = o1->find("key2");
  ASSERT_NE(o1->end(), kv2);
  ASSERT_EQ((*kv2).first, factory.getString("key2"));
  ASSERT_EQ((*kv2).second, factory.getString("value2"));
  JSONValue *value2 = (*kv2).second;

  ASSERT_EQ(1u, o1->count("key3"));

  ASSERT_EQ(1u, o1->count("key4"));

  // Keys are ordered in insertion order.
  auto it = o1->begin();
  ASSERT_STREQ("key1", (*it).first->c_str());
  ASSERT_EQ(o1->find("key1"), it);
  ++it;
  ASSERT_STREQ("key2", (*it).first->c_str());
  ASSERT_EQ(o1->find("key2"), it);
  ++it;
  ASSERT_STREQ("key3", (*it).first->c_str());
  ASSERT_EQ(o1->find("key3"), it);
  ++it;
  ASSERT_STREQ("key4", (*it).first->c_str());
  ASSERT_EQ(o1->find("key4"), it);
  ++it;
  ASSERT_EQ(o1->end(), it);

  // Check the nested object
  auto *o2 = llvh::dyn_cast<JSONObject>((*o1)["key3"]);
  ASSERT_TRUE(nullptr != o2);
  auto it2 = o2->begin();
  ASSERT_STREQ("nested1", (*it2).first->c_str());
  ASSERT_EQ(factory.getBoolean(true), (*it2).second);
  ++it2;
  ASSERT_EQ(o2->end(), it2);

  // Check the array
  auto *a1 = llvh::dyn_cast<JSONArray>(o1->at("key4"));
  ASSERT_TRUE(nullptr != a1);
  ASSERT_EQ(3u, a1->size());

  auto it3 = a1->begin();
  ASSERT_EQ(*it3, a1->at(0));
  ASSERT_EQ(*it3, (*a1)[0]);
  ASSERT_EQ(*it3, factory.getBoolean(false));
  ++it3;
  ASSERT_EQ(*it3, a1->at(1));
  ASSERT_EQ(*it3, factory.getNull());
  ++it3;
  ASSERT_EQ(*it3, a1->at(2));
  ASSERT_EQ(value2, *it3); // This also checks the uniquing
  ++it3;
  ASSERT_EQ(a1->end(), it3);
}

TEST(JSONParserTest, NegativeNumbers) {
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  const std::vector<double> expectedValues{-1.0, -1.0, -0.0};
  JSONParser parser(factory, "[-1.0, -1, -0]", sm);
  auto t1 = parser.parse();
  ASSERT_TRUE(t1.hasValue());
  auto a1 = llvh::dyn_cast<JSONArray>(*t1);
  ASSERT_TRUE(nullptr != a1);
  ASSERT_EQ(a1->size(), expectedValues.size());

  auto it = a1->begin();
  for (double expected : expectedValues) {
    auto actual = llvh::dyn_cast<JSONNumber>(*it++);
    ASSERT_TRUE(nullptr != actual);
    ASSERT_EQ(actual->getValue(), expected);
  }

  SourceErrorManager sm1;
  JSONParser parser1(factory, "-", sm1);
  auto t2 = parser1.parse();
  ASSERT_FALSE(t2.hasValue());
  ASSERT_EQ(sm1.getErrorCount(), 1);
}

TEST(JSONParserTest, HiddenClassTest) {
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  JSONParser parser(
      factory,
      "[ {'key1': 1, 'key2': {'key2': 5, 'key1': 6}}, "
      "{'key2': 10, 'key1': 20}]",
      sm);

  auto t1 = parser.parse();
  ASSERT_TRUE(t1.hasValue());
  auto array = llvh::dyn_cast<JSONArray>(t1.getValue());
  ASSERT_TRUE(nullptr != array);

  ASSERT_EQ(2u, array->size());

  auto o1 = llvh::dyn_cast<JSONObject>(array->at(0));
  ASSERT_TRUE(nullptr != o1);

  auto o2 = llvh::dyn_cast<JSONObject>(o1->at("key2"));
  ASSERT_TRUE(nullptr != o2);
  ASSERT_EQ(o1->getHiddenClass(), o2->getHiddenClass());

  auto o3 = llvh::dyn_cast<JSONObject>(array->at(1));
  ASSERT_TRUE(nullptr != o3);
  ASSERT_EQ(o1->getHiddenClass(), o3->getHiddenClass());
}

TEST(JSONParserTest, EmitTest) {
  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  SourceErrorManager sm;
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  JSONEmitter emitter(OS);
  JSONParser parser(
      factory,
      "{"
      " 'key1' : 1,"
      " 'key2' : 'value2',"
      " 'key3' : {'nested1': true},"
      " \"key4\" : [false, null, 'value2']"
      "}",
      sm);

  auto t1 = parser.parse();
  ASSERT_TRUE(t1.hasValue());
  t1.getValue()->emitInto(emitter);

  // This intermediate variable is necessary because
  // MSVC's macro preprocessor does not behave as expected with R-literals.
  const char *expected =
      R"#({"key1":1,"key2":"value2","key3":{"nested1":true},)#"
      R"#("key4":[false,null,"value2"]})#";
  EXPECT_EQ(OS.str(), expected);
}

}; // anonymous namespace
