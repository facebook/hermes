/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
// This test only makes sense if the tracing mode is turned on.
#ifdef HERMESVM_API_TRACE

#include <hermes/SynthTrace.h>
#include <hermes/Parser/JSONParser.h>
#include <hermes/TracingRuntime.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <hermes/hermes.h>

#include <limits>
#include <memory>

using namespace facebook::hermes::tracing;
using namespace facebook::hermes;
using namespace ::hermes::parser;
namespace jsi = facebook::jsi;

namespace {

struct SynthTraceTest : public ::testing::Test {
  std::unique_ptr<TracingHermesRuntime> rt;
  SynthTrace::TimeSinceStart dummyTime{SynthTrace::TimeSinceStart::zero()};

  SynthTraceTest()
      : rt(makeTracingHermesRuntime(
            makeHermesRuntime(::hermes::vm::RuntimeConfig()),
            ::hermes::vm::RuntimeConfig())) {}

  template <typename T>
  void expectEqual(
      T expected,
      const SynthTrace::Record &actual,
      const char *file,
      int line) {
    // gtest doesn't know how to convert a T, so change T to its superclass,
    // Record.
    const SynthTrace::Record &baseExpected = expected;
    if (!(expected == dynamic_cast<const T &>(actual))) {
      ADD_FAILURE_AT(file, line)
          << "expected is: " << ::testing::PrintToString(baseExpected)
          << ", actual is: " << ::testing::PrintToString(actual);
    }
  }
};

#define EXPECT_EQ_RECORD(expected, actual) \
  expectEqual(expected, actual, __FILE__, __LINE__)

/// @name HermesRuntime API tests
/// @{

TEST_F(SynthTraceTest, CreateObject) {
  SynthTrace::ObjectID objID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(1, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
}

TEST_F(SynthTraceTest, CallAndReturn) {
  const SynthTrace::ObjectID globalObjID = rt->getUniqueID(rt->global());
  SynthTrace::ObjectID functionID;
  std::string arg{"foobar"};
  {
    // Choose some function that will take a string and return the same string,
    // it doesn't matter which function it is.
    // This way we don't need to define any extra JS.
    auto func = rt->global().getPropertyAsFunction(*rt, "encodeURI");
    functionID = rt->getUniqueID(func);
    auto ret = func.call(*rt, {jsi::String::createFromAscii(*rt, arg)});
    // Make sure that the return value is correct in case there's some bug in
    // the function that was called.
    ASSERT_EQ(arg, ret.asString(*rt).utf8(*rt));
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(3, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyRecord(
          dummyTime,
          globalObjID,
          "encodeURI",
          SynthTrace::encodeObject(functionID)),
      *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {rt->trace().encodeString(arg)}),
      *records.at(1));
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(
          dummyTime, rt->trace().encodeString(arg)),
      *records.at(2));
}

TEST_F(SynthTraceTest, CallToNative) {
  auto arg = 1;
  SynthTrace::ObjectID functionID;
  {
    // Define a native function that is called from JS.
    auto undefined = [](jsi::Runtime &rt,
                        const jsi::Value &,
                        const jsi::Value *args,
                        size_t argc) -> jsi::Value {
      // Return the argument that was passed in.
      if (argc != 1) {
        throw std::logic_error("Should be exactly one argument");
      }
      return jsi::Value(rt, args[0].asNumber() + 100);
    };
    auto func = jsi::Function::createFromHostFunction(
        *rt, jsi::PropNameID::forAscii(*rt, "foo"), 0, undefined);
    functionID = rt->getUniqueID(func);
    auto ret = func.call(*rt, {jsi::Value(arg)});
    ASSERT_EQ(arg + 100, ret.asNumber());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(5, records.size());
  // The function is called from native, and is defined in native, so it
  // trampolines through the VM.
  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostFunctionRecord(dummyTime, functionID),
      *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNumber(arg)}),
      *records.at(1));
  EXPECT_EQ_RECORD(
      SynthTrace::CallToNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNumber(arg)}),
      *records.at(2));
  // Return once from the call from JS to native, and then again for the call
  // into JS.
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeNumber(arg + 100)),
      *records.at(3));
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(
          dummyTime, SynthTrace::encodeNumber(arg + 100)),
      *records.at(4));
}

TEST_F(SynthTraceTest, GetProperty) {
  SynthTrace::ObjectID objID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    // Property name doesn't matter, just want to record that some property was
    // requested.
    auto value = obj.getProperty(*rt, "a");
    ASSERT_TRUE(value.isUndefined());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyRecord(
          dummyTime, objID, "a", SynthTrace::encodeUndefined()),
      *records.at(1));
}

TEST_F(SynthTraceTest, SetProperty) {
  SynthTrace::ObjectID objID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    obj.setProperty(*rt, "a", 1);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyRecord(
          dummyTime, objID, "a", SynthTrace::encodeNumber(1)),
      *records.at(1));
}

TEST_F(SynthTraceTest, HasProperty) {
  SynthTrace::ObjectID objID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    bool hasA = obj.hasProperty(*rt, "a");
    // Whether or not A exists is irrelevant in this test.
    (void)hasA;
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::HasPropertyRecord(dummyTime, objID, "a"), *records.at(1));
}

TEST_F(SynthTraceTest, GetPropertyNames) {
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID propNamesID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    jsi::Array names = obj.getPropertyNames(*rt);
    propNamesID = rt->getUniqueID(names);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNamesRecord(dummyTime, objID, propNamesID),
      *records.at(1));
}

TEST_F(SynthTraceTest, CreateArray) {
  SynthTrace::ObjectID objID;
  {
    auto arr = jsi::Array(*rt, 10);
    objID = rt->getUniqueID(arr);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(1, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateArrayRecord(dummyTime, objID, 10), *records.at(0));
}

TEST_F(SynthTraceTest, ArrayWrite) {
  SynthTrace::ObjectID objID;
  {
    auto arr = jsi::Array(*rt, 10);
    objID = rt->getUniqueID(arr);
    arr.setValueAtIndex(*rt, 0, 1);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateArrayRecord(dummyTime, objID, 10), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::ArrayWriteRecord(
          dummyTime, objID, 0, SynthTrace::encodeNumber(1)),
      *records.at(1));
}

TEST_F(SynthTraceTest, CallObjectGetProp) {
  // Test to see if the GetPropertyRecord properly associates with the passed
  // in object.
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID functionID;
  {
    auto getObjectProp = [](jsi::Runtime &rt,
                            const jsi::Value &,
                            const jsi::Value *args,
                            size_t argc) {
      if (argc != 1) {
        throw std::logic_error("Should be exactly one argument");
      }
      args[0].asObject(rt).getProperty(rt, "a");
      return jsi::Value(1);
    };
    auto func = jsi::Function::createFromHostFunction(
        *rt, jsi::PropNameID::forAscii(*rt, "getObjectProp"), 1, getObjectProp);
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    functionID = rt->getUniqueID(func);
    auto value = func.call(*rt, obj);
    // Make sure the right value was returned.
    ASSERT_EQ(1, value.asNumber());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(7, records.size());
  // The function was called with one argument, the object.
  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostFunctionRecord(dummyTime, functionID),
      *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(1));
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeObject(objID)}),
      *records.at(2));
  // The function (which is called from JS into native) reads one property of
  // the passed in object.
  EXPECT_EQ_RECORD(
      SynthTrace::CallToNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeObject(objID)}),
      *records.at(3));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyRecord(
          dummyTime, objID, "a", SynthTrace::encodeUndefined()),
      *records.at(4));
  // The function returned a number (it also trampolined through JS and back so
  // there's two returns).
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeNumber(1)),
      *records.at(5));
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(dummyTime, SynthTrace::encodeNumber(1)),
      *records.at(6));
}

TEST_F(SynthTraceTest, HostObjectProxy) {
  SynthTrace::ObjectID objID;
  auto insertValue = 5;
  {
    class TestHostObject : public jsi::HostObject {
      double x;
      jsi::PropNameID propName;

     public:
      TestHostObject(jsi::Runtime &rt)
          : x(0.0), propName(jsi::PropNameID::forAscii(rt, "x")) {}
      jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
        if (jsi::PropNameID::compare(rt, name, propName)) {
          return jsi::Value(x);
        } else {
          return jsi::Value::undefined();
        }
      }
      void set(
          jsi::Runtime &rt,
          const jsi::PropNameID &name,
          const jsi::Value &value) override {
        if (jsi::PropNameID::compare(rt, name, propName)) {
          x = value.asNumber();
        }
      }
      std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override {
        // Can't re-use propName due to deleted copy constructor.
        return jsi::PropNameID::names(rt, "x");
      }
    };

    jsi::Object ho = jsi::Object::createFromHostObject(
        *rt, std::make_shared<TestHostObject>(*rt));
    objID = rt->getUniqueID(ho);
    // Access the property
    ASSERT_EQ(0, ho.getProperty(*rt, "x").asNumber());
    // Write to the property
    ho.setProperty(*rt, "x", jsi::Value(insertValue));
    // Check that it was written just in case.
    ASSERT_EQ(insertValue, ho.getProperty(*rt, "x").asNumber());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(10, records.size());
  // Created a proxy host object.
  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostObjectRecord(dummyTime, objID), *records.at(0));
  // Called getProperty on the proxy. This first calls getProperty on the proxy,
  // then on the host object itself.
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeRecord(dummyTime, objID, "x"),
      *records.at(1));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(0)),
      *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyRecord(
          dummyTime, objID, "x", SynthTrace::encodeNumber(0)),
      *records.at(3));
  // Called setProperty on the proxy.
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyRecord(
          dummyTime, objID, "x", SynthTrace::encodeNumber(insertValue)),
      *records.at(4));
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeRecord(
          dummyTime, objID, "x", SynthTrace::encodeNumber(insertValue)),
      *records.at(5));
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeReturnRecord(dummyTime), *records.at(6));
  // Called getProperty one last time.
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeRecord(dummyTime, objID, "x"),
      *records.at(7));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(insertValue)),
      *records.at(8));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyRecord(
          dummyTime, objID, "x", SynthTrace::encodeNumber(insertValue)),
      *records.at(9));
}

#ifdef EXPECT_DEATH
TEST_F(SynthTraceTest, HostFunctionThrowsExceptionFails) {
  // TODO (T28293178) Remove this once exceptions are supported.
  jsi::Function throwingFunc = jsi::Function::createFromHostFunction(
      *rt,
      jsi::PropNameID::forAscii(*rt, "thrower"),
      0,
      [](jsi::Runtime &rt,
         const jsi::Value &thisVal,
         const jsi::Value *args,
         size_t count) -> jsi::Value {
        throw std::runtime_error("Cannot call");
      });
  EXPECT_DEATH({ throwingFunc.call(*rt); }, "");
}

TEST_F(SynthTraceTest, HostObjectThrowsExceptionFails) {
  // TODO (T28293178) Remove this once exceptions are supported.
  class ThrowingHostObject : public jsi::HostObject {
    jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &sym) override {
      throw std::runtime_error("Cannot get");
    }

    void set(
        jsi::Runtime &rt,
        const jsi::PropNameID &sym,
        const jsi::Value &val) override {
      throw std::runtime_error("Cannot set");
    }
  };

  jsi::Object thro = jsi::Object::createFromHostObject(
      *rt, std::make_shared<ThrowingHostObject>());
  ASSERT_TRUE(thro.isHostObject(*rt));
  EXPECT_DEATH({ thro.getProperty(*rt, "foo"); }, "");
  EXPECT_DEATH({ thro.setProperty(*rt, "foo", jsi::Value::undefined()); }, "");
}
#endif

/// @}

/// @name Serialization tests
/// @{

struct SynthTraceSerializationTest : public ::testing::Test {
  SynthTrace trace{0};
  SynthTrace::TimeSinceStart dummyTime{SynthTrace::TimeSinceStart::zero()};

  std::string to_string(const SynthTrace::Record &rec) {
    std::string result;
    llvm::raw_string_ostream resultStream{result};
    ::hermes::JSONEmitter json{resultStream};
    rec.toJSON(json, trace);
    resultStream.flush();
    return result;
  }
};

TEST_F(SynthTraceSerializationTest, EncodeNumber) {
  EXPECT_EQ(
      "number:0x3ff0000000000000", trace.encode(SynthTrace::encodeNumber(1)));
}

TEST_F(SynthTraceSerializationTest, EncodeNaN) {
  EXPECT_EQ(
      "number:0x7ff8000000000000",
      trace.encode(
          SynthTrace::encodeNumber(std::numeric_limits<double>::quiet_NaN())));
}

TEST_F(SynthTraceSerializationTest, EncodeInfinity) {
  EXPECT_EQ(
      "number:0x7ff0000000000000",
      trace.encode(
          SynthTrace::encodeNumber(std::numeric_limits<double>::infinity())));
}

TEST_F(SynthTraceSerializationTest, EncodeNegativeInfinity) {
  EXPECT_EQ(
      "number:0xfff0000000000000",
      trace.encode(
          SynthTrace::encodeNumber(-std::numeric_limits<double>::infinity())));
}

TEST_F(SynthTraceSerializationTest, EncodeReallyBigInteger) {
  EXPECT_EQ(
      "number:0x423d2729eec71f97",
      trace.encode(SynthTrace::encodeNumber(125211111111.1234)));
}

TEST_F(SynthTraceSerializationTest, EncodeString) {
  EXPECT_EQ("string:hello", trace.encode(trace.encodeString("hello")));
}

TEST_F(SynthTraceSerializationTest, EncodeEmptyString) {
  EXPECT_EQ("string:", trace.encode(trace.encodeString("")));
}

TEST_F(SynthTraceSerializationTest, EncodeStringWithQuotes) {
  EXPECT_EQ(
      "string:this string contains \"quotes\"",
      trace.encode(trace.encodeString("this string contains \"quotes\"")));
}

TEST_F(SynthTraceSerializationTest, CallNoArgs) {
  EXPECT_EQ(
      R"({"type":"CallFromNativeRecord","time":0,"functionID":1,"thisArg":"undefined:","args":[]})",
      to_string(SynthTrace::CallFromNativeRecord(
          dummyTime, 1, SynthTrace::encodeUndefined(), {})));
}

TEST_F(SynthTraceSerializationTest, Call) {
  EXPECT_EQ(
      R"({"type":"CallFromNativeRecord","time":0,"functionID":1,"thisArg":"object:1","args":["undefined:","bool:true"]})",
      to_string(SynthTrace::CallFromNativeRecord(
          dummyTime,
          1,
          SynthTrace::encodeObject(1),
          {SynthTrace::encodeUndefined(), SynthTrace::encodeBool(true)})));
}

TEST_F(SynthTraceSerializationTest, Construct) {
  EXPECT_EQ(
      R"({"type":"ConstructFromNativeRecord","time":0,"functionID":1,"thisArg":"undefined:","args":["null:"]})",
      to_string(SynthTrace::ConstructFromNativeRecord(
          dummyTime,
          1,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNull()})));
}

TEST_F(SynthTraceSerializationTest, Return) {
  EXPECT_EQ(
      R"({"type":"ReturnFromNativeRecord","time":0,"retval":"bool:true"})",
      to_string(SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeBool(true))));
  EXPECT_EQ(
      R"({"type":"ReturnToNativeRecord","time":0,"retval":"bool:false"})",
      to_string(SynthTrace::ReturnToNativeRecord(
          dummyTime, SynthTrace::encodeBool(false))));
}

TEST_F(SynthTraceSerializationTest, ReturnEncodeUTF8String) {
  // "namaste" in Hindi, encoded as UTF-8.
  std::string namaste = u8"नमस्ते";
  EXPECT_EQ(
      R"({"type":"ReturnFromNativeRecord","time":0,"retval":"string:\u0928\u092e\u0938\u094d\u0924\u0947"})",
      to_string(SynthTrace::ReturnFromNativeRecord{
          dummyTime, trace.encodeString(namaste)}));
}

TEST_F(SynthTraceSerializationTest, GetProperty) {
  EXPECT_EQ(
      R"({"type":"GetPropertyRecord","time":0,"objID":1,"propName":"a","value":"undefined:"})",
      to_string(SynthTrace::GetPropertyRecord(
          dummyTime, 1, "a", SynthTrace::encodeUndefined())));
}

TEST_F(SynthTraceSerializationTest, SetProperty) {
  EXPECT_EQ(
      R"({"type":"SetPropertyRecord","time":0,"objID":1,"propName":"a","value":"string:b"})",
      to_string(SynthTrace::SetPropertyRecord(
          dummyTime, 1, "a", trace.encodeString("b"))));
}

TEST_F(SynthTraceSerializationTest, GetPropertyNames) {
  EXPECT_EQ(
      R"({"type":"GetPropertyNamesRecord","time":0,"objID":1,"propNamesID":2})",
      to_string(SynthTrace::GetPropertyNamesRecord(dummyTime, 1, 2)));
}

TEST_F(SynthTraceSerializationTest, CreateArray) {
  EXPECT_EQ(
      R"({"type":"CreateArrayRecord","time":0,"objID":1,"length":10})",
      to_string(SynthTrace::CreateArrayRecord(dummyTime, 1, 10)));
}

TEST_F(SynthTraceSerializationTest, ArrayWrite) {
  EXPECT_EQ(
      R"({"type":"ArrayWriteRecord","time":0,"objID":1,"index":0,"value":"string:a"})",
      to_string(SynthTrace::ArrayWriteRecord(
          dummyTime, 1, 0, trace.encodeString("a"))));
}

TEST_F(SynthTraceSerializationTest, MarkerRecord) {
  EXPECT_EQ(
      R"({"type":"MarkerRecord","time":0,"tag":"foo"})",
      to_string(SynthTrace::MarkerRecord(dummyTime, "foo")));
}

TEST_F(SynthTraceSerializationTest, GetPropertyNative) {
  EXPECT_EQ(
      R"({"type":"GetPropertyNativeRecord","time":0,"hostObjectID":1,"propName":"foo"})",
      to_string(SynthTrace::GetPropertyNativeRecord(dummyTime, 1, "foo")));
  EXPECT_EQ(
      R"({"type":"GetPropertyNativeReturnRecord","time":0,"retval":"null:"})",
      to_string(SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNull())));
}

TEST_F(SynthTraceSerializationTest, SetPropertyNative) {
  EXPECT_EQ(
      R"({"type":"SetPropertyNativeRecord","time":0,"hostObjectID":1,"propName":"foo","value":"string:bar"})",
      to_string(SynthTrace::SetPropertyNativeRecord(
          dummyTime, 1, "foo", trace.encodeString("bar"))));
}

TEST_F(SynthTraceSerializationTest, SetPropertyNativeReturn) {
  EXPECT_EQ(
      R"({"type":"SetPropertyNativeReturnRecord","time":0})",
      to_string(SynthTrace::SetPropertyNativeReturnRecord(dummyTime)));
}

TEST_F(SynthTraceSerializationTest, TimeIsPrinted) {
  EXPECT_EQ(
      R"({"type":"BeginExecJSRecord","time":100})",
      to_string(SynthTrace::BeginExecJSRecord(std::chrono::milliseconds(100))));
}

TEST_F(SynthTraceSerializationTest, EndExecHasRetval) {
  EXPECT_EQ(
      R"({"type":"EndExecJSRecord","time":0,"tag":"end_global_code","retval":"null:"})",
      to_string(
          SynthTrace::EndExecJSRecord(dummyTime, SynthTrace::encodeNull())));
}

TEST_F(SynthTraceSerializationTest, FullTrace) {
  const ::hermes::vm::RuntimeConfig conf;
  std::unique_ptr<TracingHermesRuntime> rt(
      makeTracingHermesRuntime(makeHermesRuntime(conf), conf));

  SynthTrace::ObjectID globalObjID = rt->getUniqueID(rt->global());
  SynthTrace::ObjectID objID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    // Property name doesn't matter, just want to record that some property was
    // requested.
    auto value = obj.getProperty(*rt, "a");
    ASSERT_TRUE(value.isUndefined());
  }

  std::string result;
  llvm::raw_string_ostream resultStream{result};
  rt->writeTrace(resultStream);
  resultStream.flush();

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hermes::SourceErrorManager sm;
  JSONParser parser{jsonFactory, result, sm};
  auto optTrace = parser.parse();
  ASSERT_TRUE(optTrace) << "Trace file is not valid JSON:\n" << result << "\n";

  // Too verbose to check every key, so let llvm::cast do the checks.
  JSONObject *root = llvm::cast<JSONObject>(optTrace.getValue());
  EXPECT_EQ(2, llvm::cast<JSONNumber>(root->at("version"))->getValue());
  EXPECT_EQ(
      globalObjID, llvm::cast<JSONNumber>(root->at("globalObjID"))->getValue());
  EXPECT_THAT(
      llvm::cast<JSONString>(root->at("sourceHash"))->str(),
      ::testing::MatchesRegex("[0-9]{40}"));

  JSONObject *gcConfig = llvm::cast<JSONObject>(root->at("gcConfig"));
  EXPECT_TRUE(llvm::isa<JSONNumber>(gcConfig->at("initHeapSize")));
  EXPECT_TRUE(llvm::isa<JSONNumber>(gcConfig->at("maxHeapSize")));

  JSONObject *environment = llvm::cast<JSONObject>(root->at("env"));
  EXPECT_TRUE(llvm::isa<JSONNumber>(environment->at("mathRandomSeed")));
  EXPECT_EQ(
      0, llvm::cast<JSONArray>(environment->at("callsToDateNow"))->size());
  EXPECT_EQ(
      0, llvm::cast<JSONArray>(environment->at("callsToNewDate"))->size());
  EXPECT_EQ(
      0,
      llvm::cast<JSONArray>(environment->at("callsToDateAsFunction"))->size());

  JSONArray *records = llvm::cast<JSONArray>(root->at("trace"));

  const JSONObject *record = llvm::cast<JSONObject>(records->at(0));
  EXPECT_EQ(
      "CreateObjectRecord", llvm::cast<JSONString>(record->at("type"))->str());
  EXPECT_TRUE(llvm::isa<JSONNumber>(record->at("time")));
  EXPECT_EQ(objID, llvm::cast<JSONNumber>(record->at("objID"))->getValue());

  record = llvm::cast<JSONObject>(records->at(1));
  EXPECT_EQ(
      "GetPropertyRecord", llvm::cast<JSONString>(record->at("type"))->str());
  EXPECT_TRUE(llvm::isa<JSONNumber>(record->at("time")));
  EXPECT_EQ(objID, llvm::cast<JSONNumber>(record->at("objID"))->getValue());
  EXPECT_EQ("a", llvm::cast<JSONString>(record->at("propName"))->str());
  EXPECT_EQ("undefined:", llvm::cast<JSONString>(record->at("value"))->str());
}

TEST_F(SynthTraceSerializationTest, FullTraceWithDateAndMath) {
  const ::hermes::vm::RuntimeConfig conf =
      ::hermes::vm::RuntimeConfig::Builder()
          .withTraceEnvironmentInteractions(true)
          .build();
  std::unique_ptr<TracingHermesRuntime> rt(
      makeTracingHermesRuntime(makeHermesRuntime(conf), conf));

  SynthTrace::ObjectID globalObjID = rt->getUniqueID(rt->global());
  uint64_t dateNow = 0;
  uint64_t newDate = 0;
  std::string dateAsFunc;
  {
    jsi::Object math = rt->global().getPropertyAsObject(*rt, "Math");
    jsi::Object date = rt->global().getPropertyAsObject(*rt, "Date");
    // Don't need the result, just making sure the seed gets set.
    math.getPropertyAsFunction(*rt, "random").call(*rt).asNumber();
    dateNow = date.getPropertyAsFunction(*rt, "now").call(*rt).asNumber();
    jsi::Function dateFunc = date.asFunction(*rt);
    auto createdDateObj = dateFunc.callAsConstructor(*rt).asObject(*rt);
    newDate = createdDateObj.getPropertyAsFunction(*rt, "getTime")
                  .callWithThis(*rt, createdDateObj)
                  .asNumber();
    dateAsFunc = dateFunc.call(*rt).asString(*rt).utf8(*rt);
  }

  std::string result;
  llvm::raw_string_ostream resultStream{result};
  rt->writeTrace(resultStream);
  resultStream.flush();

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hermes::SourceErrorManager sm;
  JSONParser parser{jsonFactory, result, sm};
  auto optTrace = parser.parse();
  ASSERT_TRUE(optTrace) << "Trace file is not valid JSON:\n" << result << "\n";

  // Too verbose to check every key, so let llvm::cast do the checks.
  JSONObject *root = llvm::cast<JSONObject>(optTrace.getValue());
  EXPECT_EQ(2, llvm::cast<JSONNumber>(root->at("version"))->getValue());
  EXPECT_EQ(
      globalObjID, llvm::cast<JSONNumber>(root->at("globalObjID"))->getValue());
  EXPECT_THAT(
      llvm::cast<JSONString>(root->at("sourceHash"))->str(),
      ::testing::MatchesRegex("[0-9]{40}"));

  JSONObject *gcConfig = llvm::cast<JSONObject>(root->at("gcConfig"));
  EXPECT_EQ(
      conf.getGCConfig().getInitHeapSize(),
      llvm::cast<JSONNumber>(gcConfig->at("initHeapSize"))->getValue());
  EXPECT_EQ(
      conf.getGCConfig().getMaxHeapSize(),
      llvm::cast<JSONNumber>(gcConfig->at("maxHeapSize"))->getValue());

  JSONObject *environment = llvm::cast<JSONObject>(root->at("env"));
  EXPECT_TRUE(llvm::isa<JSONNumber>(environment->at("mathRandomSeed")));
  JSONArray *callsToDateNow =
      llvm::cast<JSONArray>(environment->at("callsToDateNow"));
  JSONArray *callsToNewDate =
      llvm::cast<JSONArray>(environment->at("callsToNewDate"));
  JSONArray *callsToDateAsFunction =
      llvm::cast<JSONArray>(environment->at("callsToDateAsFunction"));
  EXPECT_EQ(1, callsToDateNow->size());
  EXPECT_EQ(dateNow, llvm::cast<JSONNumber>(callsToDateNow->at(0))->getValue());
  EXPECT_EQ(1, callsToNewDate->size());
  EXPECT_EQ(newDate, llvm::cast<JSONNumber>(callsToNewDate->at(0))->getValue());
  EXPECT_EQ(1, callsToDateAsFunction->size());
  EXPECT_EQ(
      dateAsFunc, llvm::cast<JSONString>(callsToDateAsFunction->at(0))->str());
  // Ignore the elements inside the trace, those are tested elsewhere.
}

struct SynthTraceParseTest : public ::testing::Test {
  std::unique_ptr<llvm::MemoryBuffer> bufFromStr(const std::string &str) {
    llvm::StringRef ref{str.data(), str.size()};
    return llvm::MemoryBuffer::getMemBufferCopy(ref);
  }
};

TEST_F(SynthTraceParseTest, ParseHeader) {
  const char *src = R"(
{
  "version": 2,
  "globalObjID": 258,
  "sourceHash": "6440b537af26795e5f452bcd320faccb02055a4f",
  "gcConfig": {
    "initHeapSize": 33554432,
    "maxHeapSize": 536870912
  },
  "env": {
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
  },
  "trace": []
}
  )";
  auto result = SynthTrace::parse(bufFromStr(src));
  const SynthTrace &trace = std::get<0>(result);
  const hermes::vm::RuntimeConfig &rtconf = std::get<1>(result);
  const hermes::vm::MockedEnvironment &env = std::get<2>(result);

  hermes::SHA1 expectedHash{{0x64, 0x40, 0xb5, 0x37, 0xaf, 0x26, 0x79,
                             0x5e, 0x5f, 0x45, 0x2b, 0xcd, 0x32, 0x0f,
                             0xac, 0xcb, 0x02, 0x05, 0x5a, 0x4f}};
  EXPECT_EQ(trace.sourceHash(), expectedHash);
  EXPECT_EQ(trace.records().size(), 0);

  EXPECT_EQ(rtconf.getGCConfig().getInitHeapSize(), 33554432);
  EXPECT_EQ(rtconf.getGCConfig().getMaxHeapSize(), 536870912);

  EXPECT_EQ(env.mathRandomSeed, 123);
  EXPECT_EQ(env.callsToDateNow.size(), 0);
  EXPECT_EQ(env.callsToNewDate.size(), 0);
  EXPECT_EQ(env.callsToDateAsFunction.size(), 0);
}

TEST_F(SynthTraceParseTest, SynthVersionMismatch) {
  const char *src = R"(
{
  "version": 0,
  "globalObjID": 258,
  "sourceHash": "6440b537af26795e5f452bcd320faccb02055a4f",
  "gcConfig": {
    "initHeapSize": 33554432,
    "maxHeapSize": 536870912
  },
  "env": {
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
  },
  "trace": []
}
  )";
  ASSERT_THROW(SynthTrace::parse(bufFromStr(src)), std::invalid_argument);
}

TEST_F(SynthTraceParseTest, SynthVersionInvalidKind) {
  const char *src = R"(
{
  "version": true,
  "globalObjID": 258,
  "sourceHash": "6440b537af26795e5f452bcd320faccb02055a4f",
  "gcConfig": {
    "initHeapSize": 33554432,
    "maxHeapSize": 536870912
  },
  "env": {
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
  },
  "trace": []
}
  )";
  ASSERT_THROW(SynthTrace::parse(bufFromStr(src)), std::invalid_argument);
}

TEST_F(SynthTraceParseTest, SynthMissingVersion) {
  const char *src = R"(
{
  "globalObjID": 258,
  "sourceHash": "6440b537af26795e5f452bcd320faccb02055a4f",
  "gcConfig": {
    "initHeapSize": 33554432,
    "maxHeapSize": 536870912
  },
  "env": {
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
  },
  "trace": []
}
  )";
  EXPECT_NO_THROW(SynthTrace::parse(bufFromStr(src)));
}
/// @}

} // namespace

#endif
