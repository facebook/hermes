/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/SynthTrace.h>

#include <hermes/Parser/JSONParser.h>
#include <hermes/Support/Algorithms.h>
#include <hermes/TraceInterpreter.h>
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

class SynthTraceSerializationTest : public ::testing::Test {
 protected:
  SynthTrace::TimeSinceStart dummyTime{SynthTrace::TimeSinceStart::zero()};

  std::string to_string(const SynthTrace::Record &rec) {
    std::string result;
    llvh::raw_string_ostream resultStream{result};
    ::hermes::JSONEmitter json{resultStream};
    rec.toJSON(json);
    resultStream.flush();
    return result;
  }

 private:
  std::string traceFilename_;
};

TEST_F(SynthTraceSerializationTest, EncodeNumber) {
  EXPECT_EQ(
      "number:0x3ff0000000000000",
      SynthTrace::encode(SynthTrace::encodeNumber(1)));
}

TEST_F(SynthTraceSerializationTest, EncodeNaN) {
  EXPECT_EQ(
      "number:0x7ff8000000000000",
      SynthTrace::encode(
          SynthTrace::encodeNumber(std::numeric_limits<double>::quiet_NaN())));
}

TEST_F(SynthTraceSerializationTest, EncodeInfinity) {
  EXPECT_EQ(
      "number:0x7ff0000000000000",
      SynthTrace::encode(
          SynthTrace::encodeNumber(std::numeric_limits<double>::infinity())));
}

TEST_F(SynthTraceSerializationTest, EncodeNegativeInfinity) {
  EXPECT_EQ(
      "number:0xfff0000000000000",
      SynthTrace::encode(
          SynthTrace::encodeNumber(-std::numeric_limits<double>::infinity())));
}

TEST_F(SynthTraceSerializationTest, EncodeReallyBigInteger) {
  EXPECT_EQ(
      "number:0x423d2729eec71f97",
      SynthTrace::encode(SynthTrace::encodeNumber(125211111111.1234)));
}

TEST_F(SynthTraceSerializationTest, EncodeString) {
  const SynthTrace::ObjectID stringId = 1111;
  EXPECT_EQ(
      std::string("string:") + std::to_string(stringId),
      SynthTrace::encode(SynthTrace::encodeString(stringId)));
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
  EXPECT_EQ(
      R"({"type":"ReturnFromNativeRecord","time":0,"retval":"string:1111"})",
      to_string(SynthTrace::ReturnFromNativeRecord{
          dummyTime, SynthTrace::encodeString(1111)}));
}

TEST_F(SynthTraceSerializationTest, GetProperty) {
  const std::string ex =
      std::string(
          R"({"type":"GetPropertyRecord","time":0,"objID":1,"propID":1111,)") +
#ifdef HERMESVM_API_TRACE_DEBUG
      R"("propName":"x",)" +
#endif
      R"("value":"undefined:"})";
  auto testRec = SynthTrace::GetPropertyRecord(
      dummyTime,
      1,
      1111,
#ifdef HERMESVM_API_TRACE_DEBUG
      "x",
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ(ex, to_string(testRec));
}

TEST_F(SynthTraceSerializationTest, SetProperty) {
  const std::string ex =
      std::string(
          R"({"type":"SetPropertyRecord","time":0,"objID":1,"propID":1111,)") +
#ifdef HERMESVM_API_TRACE_DEBUG
      R"("propName":"x",)" +
#endif
      R"("value":"string:1112"})";
  auto testRec = SynthTrace::SetPropertyRecord(
      dummyTime,
      1,
      1111,
#ifdef HERMESVM_API_TRACE_DEBUG
      "x",
#endif
      SynthTrace::encodeString(1112));
  EXPECT_EQ(ex, to_string(testRec));
}

TEST_F(SynthTraceSerializationTest, HasProperty) {
  const std::string ex =
      std::string(
          R"({"type":"HasPropertyRecord","time":0,"objID":1,"propID":1111)") +
#ifdef HERMESVM_API_TRACE_DEBUG
      R"(,"propName":"a")" +
#endif
      "}";
  auto testRec = SynthTrace::HasPropertyRecord(
      dummyTime,
      1,
      1111
#ifdef HERMESVM_API_TRACE_DEBUG
      ,
      "a"
#endif
  );
  EXPECT_EQ(ex, to_string(testRec));
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
      R"({"type":"ArrayWriteRecord","time":0,"objID":1,"index":0,"value":"string:1111"})",
      to_string(SynthTrace::ArrayWriteRecord(
          dummyTime, 1, 0, SynthTrace::encodeString(1111))));
}

TEST_F(SynthTraceSerializationTest, MarkerRecord) {
  EXPECT_EQ(
      R"({"type":"MarkerRecord","time":0,"tag":"foo"})",
      to_string(SynthTrace::MarkerRecord(dummyTime, "foo")));
}

TEST_F(SynthTraceSerializationTest, GetPropertyNative) {
  EXPECT_EQ(
      R"({"type":"GetPropertyNativeRecord","time":0,"hostObjectID":1,"propNameID":100,"propName":"foo"})",
      to_string(SynthTrace::GetPropertyNativeRecord(dummyTime, 1, 100, "foo")));
  EXPECT_EQ(
      R"({"type":"GetPropertyNativeReturnRecord","time":0,"retval":"null:"})",
      to_string(SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNull())));
}

TEST_F(SynthTraceSerializationTest, SetPropertyNative) {
  EXPECT_EQ(
      R"({"type":"SetPropertyNativeRecord","time":0,"hostObjectID":1,"propNameID":100,"propName":"foo","value":"string:1111"})",
      to_string(SynthTrace::SetPropertyNativeRecord(
          dummyTime, 1, 100, "foo", SynthTrace::encodeString(1111))));
}

TEST_F(SynthTraceSerializationTest, SetPropertyNativeReturn) {
  EXPECT_EQ(
      R"({"type":"SetPropertyNativeReturnRecord","time":0})",
      to_string(SynthTrace::SetPropertyNativeReturnRecord(dummyTime)));
}

TEST_F(SynthTraceSerializationTest, TimeIsPrinted) {
  hermes::SHA1 hash{{0x64, 0x40, 0xb5, 0x37, 0xaf, 0x26, 0x79,
                     0x5e, 0x5f, 0x45, 0x2b, 0xcd, 0x32, 0x0f,
                     0xac, 0xcb, 0x02, 0x05, 0x5a, 0x4f}};
  // JSON emitters escape forward slashes.
  EXPECT_EQ(
      R"({"type":"BeginExecJSRecord","time":100,"sourceURL":"file:\/\/\/file.js","sourceHash":"6440b537af26795e5f452bcd320faccb02055a4f","sourceIsBytecode":false})",
      to_string(SynthTrace::BeginExecJSRecord(
          std::chrono::milliseconds(100), "file:///file.js", hash, false)));
}

TEST_F(SynthTraceSerializationTest, EndExecHasRetval) {
  EXPECT_EQ(
      R"({"type":"EndExecJSRecord","time":0,"tag":"end_global_code","retval":"null:"})",
      to_string(
          SynthTrace::EndExecJSRecord(dummyTime, SynthTrace::encodeNull())));
}

TEST_F(SynthTraceSerializationTest, TraceHeader) {
  std::string result;
  auto resultStream = std::make_unique<llvh::raw_string_ostream>(result);
  const ::hermes::vm::RuntimeConfig conf;
  std::unique_ptr<TracingHermesRuntime> rt(makeTracingHermesRuntime(
      makeHermesRuntime(conf), conf, std::move(resultStream)));

  SynthTrace::ObjectID globalObjID = rt->getUniqueID(rt->global());

  rt->flushAndDisableTrace();

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hermes::SourceErrorManager sm;
  JSONParser parser{jsonFactory, result, sm};
  auto optTrace = parser.parse();
  ASSERT_TRUE(optTrace) << "Trace file is not valid JSON:\n" << result << "\n";

  JSONObject *root = llvh::cast<JSONObject>(optTrace.getValue());
  EXPECT_EQ(
      SynthTrace::synthVersion(),
      llvh::cast<JSONNumber>(root->at("version"))->getValue());
  EXPECT_EQ(
      globalObjID, llvh::cast<JSONNumber>(root->at("globalObjID"))->getValue());

  JSONObject *rtConfig = llvh::cast<JSONObject>(root->at("runtimeConfig"));

  JSONObject *gcConfig = llvh::cast<JSONObject>(rtConfig->at("gcConfig"));
  EXPECT_EQ(
      conf.getGCConfig().getMinHeapSize(),
      llvh::cast<JSONNumber>(gcConfig->at("minHeapSize"))->getValue());
  EXPECT_EQ(
      conf.getGCConfig().getInitHeapSize(),
      llvh::cast<JSONNumber>(gcConfig->at("initHeapSize"))->getValue());
  EXPECT_EQ(
      conf.getGCConfig().getMaxHeapSize(),
      llvh::cast<JSONNumber>(gcConfig->at("maxHeapSize"))->getValue());
  EXPECT_EQ(
      conf.getGCConfig().getOccupancyTarget(),
      llvh::cast<JSONNumber>(gcConfig->at("occupancyTarget"))->getValue());
  EXPECT_EQ(
      conf.getGCConfig().getEffectiveOOMThreshold(),
      llvh::cast<JSONNumber>(gcConfig->at("effectiveOOMThreshold"))
          ->getValue());
  EXPECT_EQ(
      conf.getGCConfig().getShouldReleaseUnused(),
      SynthTrace::releaseUnusedFromName(
          llvh::cast<JSONString>(gcConfig->at("shouldReleaseUnused"))
              ->c_str()));
  EXPECT_EQ(
      conf.getGCConfig().getName(),
      llvh::cast<JSONString>(gcConfig->at("name"))->str());
  EXPECT_EQ(
      conf.getGCConfig().getAllocInYoung(),
      llvh::cast<JSONBoolean>(gcConfig->at("allocInYoung"))->getValue());

  EXPECT_EQ(
      conf.getMaxNumRegisters(),
      llvh::cast<JSONNumber>(rtConfig->at("maxNumRegisters"))->getValue());
  EXPECT_EQ(
      conf.getES6Promise(),
      llvh::cast<JSONBoolean>(rtConfig->at("ES6Promise"))->getValue());
  EXPECT_EQ(
      conf.getES6Proxy(),
      llvh::cast<JSONBoolean>(rtConfig->at("ES6Proxy"))->getValue());
  EXPECT_EQ(
      conf.getIntl(),
      llvh::cast<JSONBoolean>(rtConfig->at("Intl"))->getValue());
  EXPECT_EQ(
      conf.getEnableSampledStats(),
      llvh::cast<JSONBoolean>(rtConfig->at("enableSampledStats"))->getValue());
  EXPECT_EQ(
      conf.getVMExperimentFlags(),
      llvh::cast<JSONNumber>(rtConfig->at("vmExperimentFlags"))->getValue());
}

TEST_F(SynthTraceSerializationTest, FullTrace) {
  std::string result;
  auto resultStream = std::make_unique<llvh::raw_string_ostream>(result);
  const ::hermes::vm::RuntimeConfig conf;
  std::unique_ptr<TracingHermesRuntime> rt(makeTracingHermesRuntime(
      makeHermesRuntime(conf),
      conf,
      std::move(resultStream),
      /* forReplay */ true));

  SynthTrace::ObjectID objID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    // Property name doesn't matter, just want to record that some property was
    // requested.
    auto value = obj.getProperty(*rt, "a");
    ASSERT_TRUE(value.isUndefined());
  }

  rt->flushAndDisableTrace();

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hermes::SourceErrorManager sm;
  JSONParser parser{jsonFactory, result, sm};
  auto optTrace = parser.parse();
  ASSERT_TRUE(optTrace) << "Trace file is not valid JSON:\n" << result << "\n";

  // Too verbose to check every key, so let llvh::cast do the checks.
  JSONObject *root = llvh::cast<JSONObject>(optTrace.getValue());

  JSONObject *environment = llvh::cast<JSONObject>(root->at("env"));
  EXPECT_TRUE(llvh::isa<JSONNumber>(environment->at("mathRandomSeed")));
  EXPECT_EQ(
      0, llvh::cast<JSONArray>(environment->at("callsToDateNow"))->size());
  EXPECT_EQ(
      0, llvh::cast<JSONArray>(environment->at("callsToNewDate"))->size());
  EXPECT_EQ(
      0,
      llvh::cast<JSONArray>(environment->at("callsToDateAsFunction"))->size());

  JSONArray *records = llvh::cast<JSONArray>(root->at("trace"));

  const JSONObject *record = llvh::cast<JSONObject>(records->at(0));
  EXPECT_EQ(
      "CreateObjectRecord", llvh::cast<JSONString>(record->at("type"))->str());
  EXPECT_TRUE(llvh::isa<JSONNumber>(record->at("time")));
  EXPECT_EQ(objID, llvh::cast<JSONNumber>(record->at("objID"))->getValue());

  // The obj.getProperty(*rt, "a") creates a string primitive for "a".
  record = llvh::cast<JSONObject>(records->at(1));
  EXPECT_EQ(
      "CreateStringRecord", llvh::cast<JSONString>(record->at("type"))->str());
  EXPECT_TRUE(llvh::isa<JSONNumber>(record->at("time")));
  EXPECT_TRUE(llvh::isa<JSONNumber>(record->at("objID")));
  auto stringID = llvh::cast<JSONNumber>(record->at("objID"))->getValue();

  record = llvh::cast<JSONObject>(records->at(2));
  EXPECT_EQ(
      "GetPropertyRecord", llvh::cast<JSONString>(record->at("type"))->str());
  EXPECT_TRUE(llvh::isa<JSONNumber>(record->at("time")));
  EXPECT_EQ(objID, llvh::cast<JSONNumber>(record->at("objID"))->getValue());
  EXPECT_EQ(stringID, llvh::cast<JSONNumber>(record->at("propID"))->getValue());
  EXPECT_EQ("undefined:", llvh::cast<JSONString>(record->at("value"))->str());
}

TEST_F(SynthTraceSerializationTest, FullTraceWithDateAndMath) {
  const ::hermes::vm::RuntimeConfig conf =
      ::hermes::vm::RuntimeConfig::Builder().withTraceEnabled(true).build();
  std::string result;
  auto resultStream = std::make_unique<llvh::raw_string_ostream>(result);
  std::unique_ptr<TracingHermesRuntime> rt(makeTracingHermesRuntime(
      makeHermesRuntime(conf), conf, std::move(resultStream)));

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

  rt->flushAndDisableTrace();

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hermes::SourceErrorManager sm;
  JSONParser parser{jsonFactory, result, sm};
  auto optTrace = parser.parse();
  ASSERT_TRUE(optTrace) << "Trace file is not valid JSON:\n" << result << "\n";

  // Too verbose to check every key, so let llvh::cast do the checks.
  JSONObject *root = llvh::cast<JSONObject>(optTrace.getValue());

  JSONObject *environment = llvh::cast<JSONObject>(root->at("env"));
  EXPECT_TRUE(llvh::isa<JSONNumber>(environment->at("mathRandomSeed")));
  JSONArray *callsToDateNow =
      llvh::cast<JSONArray>(environment->at("callsToDateNow"));
  JSONArray *callsToNewDate =
      llvh::cast<JSONArray>(environment->at("callsToNewDate"));
  JSONArray *callsToDateAsFunction =
      llvh::cast<JSONArray>(environment->at("callsToDateAsFunction"));
  EXPECT_EQ(1, callsToDateNow->size());
  EXPECT_EQ(dateNow, llvh::cast<JSONNumber>(callsToDateNow->at(0))->getValue());
  EXPECT_EQ(1, callsToNewDate->size());
  EXPECT_EQ(newDate, llvh::cast<JSONNumber>(callsToNewDate->at(0))->getValue());
  EXPECT_EQ(1, callsToDateAsFunction->size());
  EXPECT_EQ(
      dateAsFunc, llvh::cast<JSONString>(callsToDateAsFunction->at(0))->str());
  // Ignore the elements inside the trace, those are tested elsewhere.
}

// Handle sanitization does extra "FillerCell" allocations that break this test.
#ifndef HERMESVM_SANITIZE_HANDLES
TEST_F(SynthTraceSerializationTest, TracePreservesStringAllocs) {
  const ::hermes::vm::RuntimeConfig conf =
      ::hermes::vm::RuntimeConfig::Builder().withTraceEnabled(true).build();
  std::string traceResult;
  auto resultStream = std::make_unique<llvh::raw_string_ostream>(traceResult);
  std::unique_ptr<TracingHermesRuntime> rt(makeTracingHermesRuntime(
      makeHermesRuntime(conf), conf, std::move(resultStream)));

  std::string source = R"(
var s1 = "a";
var s2 = "b";
var s3 = s1 + s2;
function f(s) {
  return s == s3;
}
)";

  rt->evaluateJavaScript(std::make_unique<jsi::StringBuffer>(source), "source");
  // Do get property to get "ab" in the string table.
  auto s = rt->global().getProperty(*rt, "s3");

  auto f = rt->global().getProperty(*rt, "f").asObject(*rt).getFunction(*rt);
  auto res = f.call(*rt, {std::move(s)});
  EXPECT_TRUE(res.getBool());

  const auto &heapInfo = rt->instrumentation().getHeapInfo(false);
  // This test is Hermes-specific -- return early if the heapInfo does
  // not have the hermes keys we expect.
  const std::string kNumAllocObjects{"hermes_numAllocatedObjects"};
  auto iter = heapInfo.find(kNumAllocObjects);
  if (iter == heapInfo.end()) {
    return;
  }
  auto allocObjsOrig = iter->second;

  rt->flushAndDisableTrace();

  // Now replay the trace, see if we get extra allocations.
  std::vector<std::unique_ptr<llvh::MemoryBuffer>> sources;
  sources.emplace_back(llvh::MemoryBuffer::getMemBuffer(source));
  tracing::TraceInterpreter::ExecuteOptions options;
  std::string replayTraceStr;
  auto replayTraceStream =
      std::make_unique<llvh::raw_string_ostream>(replayTraceStr);
  std::unique_ptr<TracingHermesRuntime> rt2(makeTracingHermesRuntime(
      makeHermesRuntime(conf),
      conf,
      std::move(replayTraceStream),
      /* forReplay */ true));

  EXPECT_NO_THROW({
    tracing::TraceInterpreter::execFromMemoryBuffer(
        llvh::MemoryBuffer::getMemBuffer(traceResult),
        std::move(sources),
        *rt2,
        options);
  });
  rt2->flushAndDisableTrace();

  auto allocObjsFinal =
      rt2->instrumentation().getHeapInfo(false).at(kNumAllocObjects);
  EXPECT_EQ(allocObjsOrig, allocObjsFinal);
}
#endif

} // namespace
