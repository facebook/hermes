/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/SynthTrace.h>

#include <hermes/Parser/JSONParser.h>
#include <hermes/Support/Algorithms.h>
#include <hermes/SynthTraceParser.h>
#include <hermes/TraceInterpreter.h>
#include <hermes/TracingRuntime.h>
#include <hermes/VM/HermesValue.h>

#include "llvh/Support/SHA1.h"

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
  ::hermes::vm::RuntimeConfig config =
      ::hermes::vm::RuntimeConfig::Builder().withTraceEnabled(true).build();
  std::unique_ptr<TracingHermesRuntime> rt;
  SynthTrace::TimeSinceStart dummyTime{SynthTrace::TimeSinceStart::zero()};

  SynthTraceTest()
      // We pass "forReplay = true" below, to prevent the TracingHermesRuntime
      // from interactions it does automatically on non-replay runs.
      // We don't need those for these tests.
      : rt(makeTracingHermesRuntime(
            makeHermesRuntime(config),
            config,
            /* traceStream */ nullptr,
            /* forReplay */ true)) {}

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
  const std::string code = "function identity(x) { return x; }";
  rt->evaluateJavaScript(
      std::unique_ptr<jsi::StringBuffer>(new jsi::StringBuffer(code)), "");
  const SynthTrace::ObjectID globalObjID = rt->getUniqueID(rt->global());
  std::string argStr{"foobar"};
  // StringCreate0
  auto arg = jsi::String::createFromAscii(*rt, argStr);
  SynthTrace::ObjectID argID = rt->getUniqueID(arg);

  std::string identityStr{"identity"};
  jsi::String identity = jsi::String::createFromAscii(*rt, identityStr);
  SynthTrace::ObjectID identityID = rt->getUniqueID(identity);

  auto func =
      rt->global().getProperty(*rt, identity).asObject(*rt).asFunction(*rt);
  SynthTrace::ObjectID functionID = rt->getUniqueID(func);
  auto ret = func.call(*rt, {std::move(arg)});
  // Make sure that the return value is correct in case there's some bug in
  // the function that was called.
  ASSERT_EQ(argStr, ret.asString(*rt).utf8(*rt));

  const auto &records = rt->trace().records();
  // The first two records are for executing the JS for "identity".
  // Then there are two string creations -- one labeled StringCreate0 above,
  // and the other for the string "identity" in StringCreate1.
  EXPECT_EQ(7, records.size());
  auto gprExpect = SynthTrace::GetPropertyRecord(
      dummyTime,
      globalObjID,
      identityID,
#ifdef HERMESVM_API_TRACE_DEBUG
      identityStr,
#endif
      SynthTrace::encodeObject(functionID));
  EXPECT_EQ_RECORD(gprExpect, *records.at(4));
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeString(argID)}),
      *records.at(5));
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(
          dummyTime, SynthTrace::encodeString(argID)),
      *records.at(6));
}

TEST_F(SynthTraceTest, CallToNative) {
  auto arg = 1;
  SynthTrace::ObjectID functionID;
  uint32_t propNameID;
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
    auto propName = jsi::PropNameID::forAscii(*rt, "foo");
    auto func =
        jsi::Function::createFromHostFunction(*rt, propName, 1, undefined);
    propNameID = rt->getUniqueID(propName);
    functionID = rt->getUniqueID(func);
    auto ret = func.call(*rt, {jsi::Value(arg)});
    ASSERT_EQ(arg + 100, ret.asNumber());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(6, records.size());
  // The function is called from native, and is defined in native, so it
  // trampolines through the VM.
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, propNameID, "foo", 3),
      *records.at(0));
  auto chfrExpect = SynthTrace::CreateHostFunctionRecord(
      dummyTime,
      functionID,
      propNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      "foo",
#endif
      1);
  EXPECT_EQ_RECORD(chfrExpect, *records.at(1));
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNumber(arg)}),
      *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CallToNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNumber(arg)}),
      *records.at(3));
  // Return once from the call from JS to native, and then again for the call
  // into JS.
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeNumber(arg + 100)),
      *records.at(4));
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(
          dummyTime, SynthTrace::encodeNumber(arg + 100)),
      *records.at(5));
}

TEST_F(SynthTraceTest, GetProperty) {
  std::string a{"a"};
  std::string b{"b"};
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID aStringID;
  SynthTrace::ObjectID bPropID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    // Property name doesn't matter, just want to record that some property was
    // requested.
    auto aStr = jsi::String::createFromAscii(*rt, a);
    aStringID = rt->getUniqueID(aStr);
    auto aValue = obj.getProperty(*rt, aStr);
    ASSERT_TRUE(aValue.isUndefined());

    // Now get using a PropNameID.
    auto bProp = jsi::PropNameID::forAscii(*rt, b);
    bPropID = rt->getUniqueID(bProp);
    auto bValue = obj.getProperty(*rt, bProp);
    ASSERT_TRUE(bValue.isUndefined());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(5, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records.at(1));
  auto gprExpect0 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      aStringID,
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect0, *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, bPropID, b.c_str(), 1),
      *records.at(3));
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      bPropID,
#ifdef HERMESVM_API_TRACE_DEBUG
      b,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect1, *records.at(4));
}

TEST_F(SynthTraceTest, SetProperty) {
  std::string a{"a"};
  std::string b{"b"};
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID aStringID;
  SynthTrace::ObjectID bPropID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    auto aStr = jsi::String::createFromAscii(*rt, a);
    aStringID = rt->getUniqueID(aStr);
    obj.setProperty(*rt, aStr, 1);

    // Now set using a PropNameID.
    auto bProp = jsi::PropNameID::forAscii(*rt, b);
    bPropID = rt->getUniqueID(bProp);
    obj.setProperty(*rt, bProp, true);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(5, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records.at(1));
  auto sprExpect0 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      aStringID,
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeNumber(1));
  EXPECT_EQ_RECORD(sprExpect0, *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, bPropID, b.c_str(), 1),
      *records.at(3));
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      bPropID,
#ifdef HERMESVM_API_TRACE_DEBUG
      b,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect1, *records.at(4));
}

TEST_F(SynthTraceTest, HasProperty) {
  std::string a{"a"};
  std::string b{"b"};
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID aStringID;
  SynthTrace::ObjectID bPropID;
  {
    auto obj = jsi::Object(*rt);
    objID = rt->getUniqueID(obj);
    auto aStr = jsi::String::createFromAscii(*rt, a);
    aStringID = rt->getUniqueID(aStr);
    bool hasA = obj.hasProperty(*rt, aStr);
    // Whether or not "a" exists is irrelevant in this test.
    (void)hasA;

    // Now set using a PropNameID.
    auto bProp = jsi::PropNameID::forAscii(*rt, b);
    bPropID = rt->getUniqueID(bProp);
    bool hasB = obj.hasProperty(*rt, bProp);
    // Whether or not "b" exists is irrelevant in this test.
    (void)hasB;
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(5, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records.at(1));
  auto hprExpect0 = SynthTrace::HasPropertyRecord(
      dummyTime,
      objID,
      aStringID
#ifdef HERMESVM_API_TRACE_DEBUG
      ,
      a
#endif
  );
  EXPECT_EQ_RECORD(hprExpect0, *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, bPropID, b.c_str(), 1),
      *records.at(3));
  auto hprExpect1 = SynthTrace::HasPropertyRecord(
      dummyTime,
      objID,
      bPropID
#ifdef HERMESVM_API_TRACE_DEBUG
      ,
      b
#endif
  );
  EXPECT_EQ_RECORD(hprExpect1, *records.at(4));
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
  std::string a{"a"};
  std::string getObjectPropStr{"getObjectProp"};
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID aStringID;
  SynthTrace::ObjectID functionID;
  uint32_t propNameID;
  {
    auto aStr = jsi::String::createFromAscii(*rt, a);
    aStringID = rt->getUniqueID(aStr);
    auto getObjectProp = [&aStr](
                             jsi::Runtime &rt,
                             const jsi::Value &,
                             const jsi::Value *args,
                             size_t argc) {
      if (argc != 1) {
        throw std::logic_error("Should be exactly one argument");
      }
      args[0].asObject(rt).getProperty(rt, aStr);
      return jsi::Value(1);
    };
    auto propName = jsi::PropNameID::forAscii(*rt, getObjectPropStr);
    auto func =
        jsi::Function::createFromHostFunction(*rt, propName, 1, getObjectProp);
    auto obj = jsi::Object(*rt);
    propNameID = rt->getUniqueID(propName);
    objID = rt->getUniqueID(obj);
    functionID = rt->getUniqueID(func);
    auto value = func.call(*rt, obj);
    // Make sure the right value was returned.
    ASSERT_EQ(1, value.asNumber());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(9, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          propNameID,
          getObjectPropStr.c_str(),
          getObjectPropStr.size()),
      *records.at(1));
  // The function was called with one argument, the object.
  auto chfrExpect = SynthTrace::CreateHostFunctionRecord(
      dummyTime,
      functionID,
      propNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      "getObjectProp",
#endif
      1);
  EXPECT_EQ_RECORD(chfrExpect, *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records.at(3));
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeObject(objID)}),
      *records.at(4));
  // The function (which is called from JS into native) reads one property of
  // the passed in object.
  EXPECT_EQ_RECORD(
      SynthTrace::CallToNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeObject(objID)}),
      *records.at(5));
  auto gprExpect = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      aStringID,
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect, *records.at(6));
  // The function returned a number (it also trampolined through JS and back so
  // there's two returns).
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeNumber(1)),
      *records.at(7));
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(dummyTime, SynthTrace::encodeNumber(1)),
      *records.at(8));
}

TEST_F(SynthTraceTest, HostObjectProxy) {
  // This allows us to share the constant strings between the outer scope
  // and the TestHostObject, below.
  struct ConstStrings {
    const std::string x{"x"};
    const std::string getHappened{"getHappened"};
    const std::string setHappened{"setHappened"};
    const std::string getPropertyNamesHappened{"getPropertyNamesHappened"};
  };
  ConstStrings cs;

  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID xPropNameID;
  SynthTrace::ObjectID getHappenedPropNameID;
  SynthTrace::ObjectID setHappenedPropNameID;
  SynthTrace::ObjectID getPropertyNamesHappenedPropNameID;
  auto insertValue = 5;
  {
    class TestHostObject : public jsi::HostObject {
      double xVal;
      const ConstStrings &cs;

     public:
      jsi::PropNameID xPropName;
      jsi::PropNameID getHappenedPropName;
      jsi::PropNameID setHappenedPropName;
      jsi::PropNameID getPropertyNamesHappenedPropName;

      TestHostObject(jsi::Runtime &rt, const ConstStrings &cs)
          : xVal(0.0),
            cs(cs),
            xPropName(jsi::PropNameID::forAscii(rt, cs.x.c_str())),
            getHappenedPropName(
                jsi::PropNameID::forAscii(rt, cs.getHappened.c_str())),
            setHappenedPropName(
                jsi::PropNameID::forAscii(rt, cs.setHappened.c_str())),
            getPropertyNamesHappenedPropName(jsi::PropNameID::forAscii(
                rt,
                cs.getPropertyNamesHappened.c_str())) {}
      jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
        // Do an operation with the runtime, to ensure that it is traced.
        rt.global().setProperty(rt, getHappenedPropName, jsi::Value(true));
        if (jsi::PropNameID::compare(rt, name, xPropName)) {
          return jsi::Value(xVal);
        } else {
          return jsi::Value::undefined();
        }
      }
      void set(
          jsi::Runtime &rt,
          const jsi::PropNameID &name,
          const jsi::Value &value) override {
        // Do an operation with the runtime, to ensure that it is traced.
        rt.global().setProperty(rt, setHappenedPropName, jsi::Value(true));
        if (jsi::PropNameID::compare(rt, name, xPropName)) {
          xVal = value.asNumber();
        }
      }
      std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override {
        // Do an operation with the runtime, to ensure that it is traced.
        rt.global().setProperty(
            rt, getPropertyNamesHappenedPropName, jsi::Value(true));
        // Can't re-use propName due to deleted copy constructor.
        return jsi::PropNameID::names(rt, cs.x.c_str());
      }
    };

    auto tho = std::make_shared<TestHostObject>(*rt, cs);
    xPropNameID = rt->getUniqueID(tho->xPropName);
    getHappenedPropNameID = rt->getUniqueID(tho->getHappenedPropName);
    setHappenedPropNameID = rt->getUniqueID(tho->setHappenedPropName);
    getPropertyNamesHappenedPropNameID =
        rt->getUniqueID(tho->getPropertyNamesHappenedPropName);

    jsi::Object ho = jsi::Object::createFromHostObject(*rt, tho);
    objID = rt->getUniqueID(ho);
    // Access the property
    ASSERT_EQ(0, ho.getProperty(*rt, tho->xPropName).asNumber());
    // Write to the property
    ho.setProperty(*rt, tho->xPropName, jsi::Value(insertValue));
    // Check that it was written just in case.
    ASSERT_EQ(insertValue, ho.getProperty(*rt, tho->xPropName).asNumber());
  }
  const auto &records = rt->trace().records();
  auto globID = rt->getUniqueID(rt->global());
  EXPECT_EQ(17, records.size());
  // Created a proxy host object.
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, xPropNameID, cs.x.c_str(), cs.x.size()),
      *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          getHappenedPropNameID,
          cs.getHappened.c_str(),
          cs.getHappened.size()),
      *records.at(1));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          setHappenedPropNameID,
          cs.setHappened.c_str(),
          cs.setHappened.size()),
      *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          getPropertyNamesHappenedPropNameID,
          cs.getPropertyNamesHappened.c_str(),
          cs.getPropertyNamesHappened.size()),
      *records.at(3));

  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostObjectRecord(dummyTime, objID), *records.at(4));
  // Called getProperty on the proxy. This first calls getProperty on the proxy,
  // then on the host object itself.
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeRecord(dummyTime, objID, xPropNameID, cs.x),
      *records.at(5));
  auto sprExpect0 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      getHappenedPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.getHappened,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect0, *records.at(6));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(0)),
      *records.at(7));
  auto gprExpect0 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      xPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(0));
  EXPECT_EQ_RECORD(gprExpect0, *records.at(8));
  // Called setProperty on the proxy.
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      xPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(insertValue));
  EXPECT_EQ_RECORD(sprExpect1, *records.at(9));
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeRecord(
          dummyTime,
          objID,
          xPropNameID,
          cs.x,
          SynthTrace::encodeNumber(insertValue)),
      *records.at(10));
  auto sprExpect2 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      setHappenedPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.setHappened,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect2, *records.at(11));
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeReturnRecord(dummyTime), *records.at(12));
  // Called getProperty one last time.
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeRecord(dummyTime, objID, xPropNameID, cs.x),
      *records.at(13));
  auto sprExpect4 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      getHappenedPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.getHappened,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect4, *records.at(14));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(insertValue)),
      *records.at(15));
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      xPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(insertValue));
  EXPECT_EQ_RECORD(gprExpect1, *records.at(16));
}

TEST_F(SynthTraceTest, HostObjectPropertyNamesAreDefs) {
  const std::string ho{"ho"};
  const std::string x{"x"};
  const std::string y{"y"};
  const std::string xRes{"xRes"};
  const std::string yRes{"yRes"};
  // This allows us to share the constant strings between the outer scope
  // and the TestHostObject, below.
  struct ConstStrings {
    const std::string o{"o"};
  };
  ConstStrings cs;

  SynthTrace::ObjectID oObjID;
  SynthTrace::ObjectID hoObjID;
  SynthTrace::ObjectID oPropNameID;
  SynthTrace::ObjectID hoPropNameID;
  SynthTrace::ObjectID xResPropNameID;
  SynthTrace::ObjectID yResPropNameID;
  hermes::SHA1 codeHash;
  {
    class TestHostObject : public jsi::HostObject {
     public:
      jsi::PropNameID oPropName;

      TestHostObject(jsi::Runtime &rt, const ConstStrings &cs)
          : oPropName(jsi::PropNameID::forAscii(rt, cs.o.c_str())) {}

      jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
        // Do an operation with the runtime, to ensure that it is traced.
        auto oObj = rt.global().getProperty(rt, oPropName).asObject(rt);
        return oObj.getProperty(rt, name);
      }
      void set(
          jsi::Runtime &rt,
          const jsi::PropNameID &name,
          const jsi::Value &value) override {
        // Do an operation with the runtime, to ensure that it is traced.
        auto oObj = rt.global().getProperty(rt, oPropName).asObject(rt);
        oObj.setProperty(rt, name, value);
      }
    };

    auto tho = std::make_shared<TestHostObject>(*rt, cs);
    oPropNameID = rt->getUniqueID(tho->oPropName);

    jsi::Object o{*rt};
    oObjID = rt->getUniqueID(o);
    rt->global().setProperty(*rt, tho->oPropName, o);

    jsi::Object hoObj = jsi::Object::createFromHostObject(*rt, tho);
    hoObjID = rt->getUniqueID(hoObj);
    auto hoPropName = jsi::PropNameID::forAscii(*rt, ho.c_str());
    hoPropNameID = rt->getUniqueID(hoPropName);
    rt->global().setProperty(*rt, hoPropName, hoObj);

    const std::string code = R"###(
        o.x = 7;
        o.y = true;
        (function () {
          this.xRes = ho.x;
          ho.y = false;
          this.yRes = o.y;
        }) ();
    )###";
    codeHash = llvh::SHA1::hash(llvh::makeArrayRef(
        reinterpret_cast<const uint8_t *>(code.data()), code.size()));

    rt->evaluateJavaScript(
        std::unique_ptr<jsi::StringBuffer>(new jsi::StringBuffer(code)), "");

    auto xResPropName = jsi::PropNameID::forAscii(*rt, xRes.c_str());
    xResPropNameID = rt->getUniqueID(xResPropName);
    auto yResPropName = jsi::PropNameID::forAscii(*rt, yRes.c_str());
    yResPropNameID = rt->getUniqueID(yResPropName);
    // Retrieve the results.
    ASSERT_EQ(7, rt->global().getProperty(*rt, xResPropName).asNumber());
    ASSERT_FALSE(rt->global().getProperty(*rt, yResPropName).getBool());
  }
  const auto &records = rt->trace().records();
  auto globID = rt->getUniqueID(rt->global());
  EXPECT_EQ(20, records.size());
  // Created a proxy host object.
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, oPropNameID, cs.o.c_str(), cs.o.size()),
      *records.at(0));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, oObjID), *records.at(1));
  auto sprExpect0 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      oPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(sprExpect0, *records.at(2));
  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostObjectRecord(dummyTime, hoObjID), *records.at(3));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, hoPropNameID, ho.c_str(), ho.size()),
      *records.at(4));
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      hoPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      ho,
#endif
      SynthTrace::encodeObject(hoObjID));
  EXPECT_EQ_RECORD(sprExpect1, *records.at(5));
  EXPECT_EQ_RECORD(
      SynthTrace::BeginExecJSRecord(dummyTime, "", codeHash, false),
      *records.at(6));
  // Called getProperty on the host object.
  // We can't create the expected record, since we don't know the unique ID for
  // the PropNameID for "x".  So test the fields individually.
  EXPECT_EQ(
      SynthTrace::RecordType::GetPropertyNative, records.at(7)->getType());
  auto rec7AsGPN =
      dynamic_cast<const SynthTrace::GetPropertyNativeRecord &>(*records.at(7));
  EXPECT_EQ(hoObjID, rec7AsGPN.hostObjectID_);
  uint32_t observedXPropNameUID = rec7AsGPN.propNameID_;
  EXPECT_EQ(x, rec7AsGPN.propName_);
  // Now we're in in the body of the HostObject getter.
  auto gprExpect0 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      oPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(gprExpect0, *records.at(8));
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      oObjID,
      observedXPropNameUID,
#ifdef HERMESVM_API_TRACE_DEBUG
      x,
#endif
      SynthTrace::encodeNumber(7));
  EXPECT_EQ_RECORD(gprExpect1, *records.at(9));
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(7)),
      *records.at(10));
  // Called setProperty on the host object.
  // We can't create the expected record, since we don't know the unique ID for
  // the PropNameID for "y".  So test the fields individually.
  EXPECT_EQ(
      SynthTrace::RecordType::SetPropertyNative, records.at(11)->getType());
  auto rec11AsGPN = dynamic_cast<const SynthTrace::SetPropertyNativeRecord &>(
      *records.at(11));
  EXPECT_EQ(hoObjID, rec11AsGPN.hostObjectID_);
  uint32_t observedYPropNameUID = rec11AsGPN.propNameID_;
  EXPECT_EQ(y, rec11AsGPN.propName_);
  EXPECT_FALSE(rec11AsGPN.value_.getBool());
  auto gprExpect2 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      oPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(gprExpect2, *records.at(12));
  auto sprExpect = SynthTrace::SetPropertyRecord(
      dummyTime,
      oObjID,
      observedYPropNameUID,
#ifdef HERMESVM_API_TRACE_DEBUG
      y,
#endif
      SynthTrace::encodeBool(false));
  EXPECT_EQ_RECORD(sprExpect, *records.at(13));
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeReturnRecord(dummyTime), *records.at(14));
  EXPECT_EQ_RECORD(
      SynthTrace::EndExecJSRecord(dummyTime, SynthTrace::encodeUndefined()),
      *records.at(15));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, xResPropNameID, xRes.c_str(), xRes.size()),
      *records.at(16));
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, yResPropNameID, yRes.c_str(), yRes.size()),
      *records.at(17));
  auto gprExpect3 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      xResPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      xRes,
#endif
      SynthTrace::encodeNumber(7));
  EXPECT_EQ_RECORD(gprExpect3, *records.at(18));
  auto gprExpect4 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      yResPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      yRes,
#endif
      SynthTrace::encodeBool(false));
  EXPECT_EQ_RECORD(gprExpect4, *records.at(19));
}

// These tests fail on Windows.
#if defined(EXPECT_DEATH_IF_SUPPORTED) && !defined(_WINDOWS)
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
  EXPECT_DEATH_IF_SUPPORTED({ throwingFunc.call(*rt); }, "");
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
  EXPECT_DEATH_IF_SUPPORTED({ thro.getProperty(*rt, "foo"); }, "");
  EXPECT_DEATH_IF_SUPPORTED(
      { thro.setProperty(*rt, "foo", jsi::Value::undefined()); }, "");
}
#endif

/// @}

/// @name Serialization tests
/// @{

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
/// @}

} // namespace
