/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/SynthTrace.h>
#include "llvh/ADT/ArrayRef.h"

#include <hermes/TraceInterpreter.h>
#include <hermes/TracingRuntime.h>

#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/VMExperiments.h"
#include "llvh/Support/SHA1.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <hermes/hermes.h>

#include <memory>
#include <variant>

using namespace facebook::hermes::tracing;
using namespace facebook::hermes;
namespace jsi = facebook::jsi;

namespace {

struct SynthTraceTest : public ::testing::Test {
  std::unique_ptr<TracingHermesRuntime> rt;
  SynthTrace::TimeSinceStart dummyTime{SynthTrace::TimeSinceStart::zero()};

  static std::unique_ptr<TracingHermesRuntime> makeRuntime() {
    ::hermes::vm::RuntimeConfig config =
        ::hermes::vm::RuntimeConfig::Builder()
            .withSynthTraceMode(
                ::hermes::vm::SynthTraceMode::TracingAndReplaying)
            .withMicrotaskQueue(true)
            .build();
    // We pass "forReplay = true" below, to prevent the TracingHermesRuntime
    // from interactions it does automatically on non-replay runs.
    // We don't need those for these tests.
    return makeTracingHermesRuntime(
        makeHermesRuntime(config),
        config,
        /* traceStream */ nullptr,
        /* forReplay */ true);
  }

  SynthTraceTest() : rt(makeRuntime()) {}

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

/// @name Synth trace tests
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
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records[0]);
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
      SynthTrace::encodeString(identityID),
#ifdef HERMESVM_API_TRACE_DEBUG
      identityStr,
#endif
      SynthTrace::encodeObject(functionID));
  EXPECT_EQ_RECORD(gprExpect, *records[4]);
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeString(argID)}),
      *records[5]);
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(
          dummyTime, SynthTrace::encodeString(argID)),
      *records[6]);
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
      *records[0]);
  auto chfrExpect = SynthTrace::CreateHostFunctionRecord(
      dummyTime,
      functionID,
      propNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      "foo",
#endif
      1);
  EXPECT_EQ_RECORD(chfrExpect, *records[1]);
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNumber(arg)}),
      *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CallToNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeNumber(arg)}),
      *records[3]);
  // Return once from the call from JS to native, and then again for the call
  // into JS.
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeNumber(arg + 100)),
      *records[4]);
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(
          dummyTime, SynthTrace::encodeNumber(arg + 100)),
      *records[5]);
}

TEST_F(SynthTraceTest, GetProperty) {
  std::string a{"a"};
  std::string b{"b"};
  SynthTrace::ObjectID objID;
  SynthTrace::ObjectID aStringID;
  SynthTrace::ObjectID aPropID;
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

    // Now get using a PropNameID created from aStr.
    auto aProp = jsi::PropNameID::forString(*rt, aStr);
    aPropID = rt->getUniqueID(aProp);
    aValue = obj.getProperty(*rt, aProp);
    ASSERT_TRUE(aValue.isUndefined());

    // Now get using a PropNameID created from b.
    auto bProp = jsi::PropNameID::forAscii(*rt, b);
    bPropID = rt->getUniqueID(bProp);
    auto bValue = obj.getProperty(*rt, bProp);
    ASSERT_TRUE(bValue.isUndefined());
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(7, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records[1]);
  auto gprExpect0 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodeString(aStringID),
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect0, *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, aPropID, SynthTrace::encodeString(aStringID)),
      *records[3]);
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(aPropID),
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect1, *records[4]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, bPropID, b.c_str(), 1),
      *records[5]);
  auto gprExpect2 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(bPropID),
#ifdef HERMESVM_API_TRACE_DEBUG
      b,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect2, *records[6]);
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
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records[1]);
  auto sprExpect0 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodeString(aStringID),
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeNumber(1));
  EXPECT_EQ_RECORD(sprExpect0, *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, bPropID, b.c_str(), 1),
      *records[3]);
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(bPropID),
#ifdef HERMESVM_API_TRACE_DEBUG
      b,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect1, *records[4]);
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
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateStringRecord(dummyTime, aStringID, a.c_str(), 1),
      *records[1]);
  auto hprExpect0 = SynthTrace::HasPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodeString(aStringID)
#ifdef HERMESVM_API_TRACE_DEBUG
          ,
      a
#endif
  );
  EXPECT_EQ_RECORD(hprExpect0, *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(dummyTime, bPropID, b.c_str(), 1),
      *records[3]);
  auto hprExpect1 = SynthTrace::HasPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(bPropID)
#ifdef HERMESVM_API_TRACE_DEBUG
          ,
      b
#endif
  );
  EXPECT_EQ_RECORD(hprExpect1, *records[4]);
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
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNamesRecord(dummyTime, objID, propNamesID),
      *records[1]);
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
      SynthTrace::CreateArrayRecord(dummyTime, objID, 10), *records[0]);
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
      SynthTrace::CreateArrayRecord(dummyTime, objID, 10), *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::ArrayWriteRecord(
          dummyTime, objID, 0, SynthTrace::encodeNumber(1)),
      *records[1]);
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
      *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          propNameID,
          getObjectPropStr.c_str(),
          getObjectPropStr.size()),
      *records[1]);
  // The function was called with one argument, the object.
  auto chfrExpect = SynthTrace::CreateHostFunctionRecord(
      dummyTime,
      functionID,
      propNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      "getObjectProp",
#endif
      1);
  EXPECT_EQ_RECORD(chfrExpect, *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, objID), *records[3]);
  EXPECT_EQ_RECORD(
      SynthTrace::CallFromNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeObject(objID)}),
      *records[4]);
  // The function (which is called from JS into native) reads one property of
  // the passed in object.
  EXPECT_EQ_RECORD(
      SynthTrace::CallToNativeRecord(
          dummyTime,
          functionID,
          SynthTrace::encodeUndefined(),
          {SynthTrace::encodeObject(objID)}),
      *records[5]);
  auto gprExpect = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodeString(aStringID),
#ifdef HERMESVM_API_TRACE_DEBUG
      a,
#endif
      SynthTrace::encodeUndefined());
  EXPECT_EQ_RECORD(gprExpect, *records[6]);
  // The function returned a number (it also trampolined through JS and back so
  // there's two returns).
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnFromNativeRecord(
          dummyTime, SynthTrace::encodeNumber(1)),
      *records[7]);
  EXPECT_EQ_RECORD(
      SynthTrace::ReturnToNativeRecord(dummyTime, SynthTrace::encodeNumber(1)),
      *records[8]);
}

TEST_F(SynthTraceTest, DrainMicrotasks) {
  {
    rt->drainMicrotasks();
    rt->drainMicrotasks(5);
  }
  const auto &records = rt->trace().records();
  EXPECT_EQ(2, records.size());
  EXPECT_EQ_RECORD(SynthTrace::DrainMicrotasksRecord(dummyTime), *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::DrainMicrotasksRecord(dummyTime, 5), *records[1]);
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
      *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          getHappenedPropNameID,
          cs.getHappened.c_str(),
          cs.getHappened.size()),
      *records[1]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          setHappenedPropNameID,
          cs.setHappened.c_str(),
          cs.setHappened.size()),
      *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime,
          getPropertyNamesHappenedPropNameID,
          cs.getPropertyNamesHappened.c_str(),
          cs.getPropertyNamesHappened.size()),
      *records[3]);

  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostObjectRecord(dummyTime, objID), *records[4]);
  // Called getProperty on the proxy. This first calls getProperty on the proxy,
  // then on the host object itself.
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeRecord(dummyTime, objID, xPropNameID, cs.x),
      *records[5]);
  auto sprExpect0 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(getHappenedPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.getHappened,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect0, *records[6]);
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(0)),
      *records[7]);
  auto gprExpect0 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(xPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(0));
  EXPECT_EQ_RECORD(gprExpect0, *records[8]);
  // Called setProperty on the proxy.
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(xPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(insertValue));
  EXPECT_EQ_RECORD(sprExpect1, *records[9]);
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeRecord(
          dummyTime,
          objID,
          xPropNameID,
          cs.x,
          SynthTrace::encodeNumber(insertValue)),
      *records[10]);
  auto sprExpect2 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(setHappenedPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.setHappened,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect2, *records[11]);
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeReturnRecord(dummyTime), *records[12]);
  // Called getProperty one last time.
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeRecord(dummyTime, objID, xPropNameID, cs.x),
      *records[13]);
  auto sprExpect4 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(getHappenedPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.getHappened,
#endif
      SynthTrace::encodeBool(true));
  EXPECT_EQ_RECORD(sprExpect4, *records[14]);
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(insertValue)),
      *records[15]);
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      objID,
      SynthTrace::encodePropNameID(xPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(insertValue));
  EXPECT_EQ_RECORD(gprExpect1, *records[16]);
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
      *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateObjectRecord(dummyTime, oObjID), *records[1]);
  auto sprExpect0 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(oPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(sprExpect0, *records[2]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateHostObjectRecord(dummyTime, hoObjID), *records[3]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, hoPropNameID, ho.c_str(), ho.size()),
      *records[4]);
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(hoPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      ho,
#endif
      SynthTrace::encodeObject(hoObjID));
  EXPECT_EQ_RECORD(sprExpect1, *records[5]);
  EXPECT_EQ_RECORD(
      SynthTrace::BeginExecJSRecord(dummyTime, "", codeHash, false),
      *records[6]);
  // Called getProperty on the host object.
  // We can't create the expected record, since we don't know the unique ID for
  // the PropNameID for "x".  So test the fields individually.
  EXPECT_EQ(SynthTrace::RecordType::GetPropertyNative, records[7]->getType());
  auto rec7AsGPN =
      dynamic_cast<const SynthTrace::GetPropertyNativeRecord &>(*records[7]);
  EXPECT_EQ(hoObjID, rec7AsGPN.hostObjectID_);
  uint32_t observedXPropNameUID = rec7AsGPN.propNameID_;
  EXPECT_EQ(x, rec7AsGPN.propName_);
  // Now we're in in the body of the HostObject getter.
  auto gprExpect0 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(oPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(gprExpect0, *records[8]);
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      oObjID,
      SynthTrace::encodePropNameID(observedXPropNameUID),
#ifdef HERMESVM_API_TRACE_DEBUG
      x,
#endif
      SynthTrace::encodeNumber(7));
  EXPECT_EQ_RECORD(gprExpect1, *records[9]);
  EXPECT_EQ_RECORD(
      SynthTrace::GetPropertyNativeReturnRecord(
          dummyTime, SynthTrace::encodeNumber(7)),
      *records[10]);
  // Called setProperty on the host object.
  // We can't create the expected record, since we don't know the unique ID for
  // the PropNameID for "y".  So test the fields individually.
  EXPECT_EQ(SynthTrace::RecordType::SetPropertyNative, records[11]->getType());
  auto rec11AsGPN =
      dynamic_cast<const SynthTrace::SetPropertyNativeRecord &>(*records[11]);
  EXPECT_EQ(hoObjID, rec11AsGPN.hostObjectID_);
  uint32_t observedYPropNameUID = rec11AsGPN.propNameID_;
  EXPECT_EQ(y, rec11AsGPN.propName_);
  EXPECT_FALSE(rec11AsGPN.value_.getBool());
  auto gprExpect2 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(oPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(gprExpect2, *records[12]);
  auto sprExpect = SynthTrace::SetPropertyRecord(
      dummyTime,
      oObjID,
      SynthTrace::encodePropNameID(observedYPropNameUID),
#ifdef HERMESVM_API_TRACE_DEBUG
      y,
#endif
      SynthTrace::encodeBool(false));
  EXPECT_EQ_RECORD(sprExpect, *records[13]);
  EXPECT_EQ_RECORD(
      SynthTrace::SetPropertyNativeReturnRecord(dummyTime), *records[14]);
  EXPECT_EQ_RECORD(
      SynthTrace::EndExecJSRecord(dummyTime, SynthTrace::encodeUndefined()),
      *records[15]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, xResPropNameID, xRes.c_str(), xRes.size()),
      *records[16]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreatePropNameIDRecord(
          dummyTime, yResPropNameID, yRes.c_str(), yRes.size()),
      *records[17]);
  auto gprExpect3 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(xResPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      xRes,
#endif
      SynthTrace::encodeNumber(7));
  EXPECT_EQ_RECORD(gprExpect3, *records[18]);
  auto gprExpect4 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      SynthTrace::encodePropNameID(yResPropNameID),
#ifdef HERMESVM_API_TRACE_DEBUG
      yRes,
#endif
      SynthTrace::encodeBool(false));
  EXPECT_EQ_RECORD(gprExpect4, *records[19]);
}

TEST_F(SynthTraceTest, CreateBigInt) {
  SynthTrace::ObjectID fromInt64ID =
      rt->getUniqueID(jsi::BigInt::fromInt64(*rt, 0xffffffffffffffff));
  SynthTrace::ObjectID fromUint64ID =
      rt->getUniqueID(jsi::BigInt::fromUint64(*rt, 0xffffffffffffffff));

  const auto &records = rt->trace().records();
  ASSERT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateBigIntRecord(
          dummyTime,
          fromInt64ID,
          SynthTrace::CreateBigIntRecord::Method::FromInt64,
          0xffffffffffffffff),
      *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::CreateBigIntRecord(
          dummyTime,
          fromUint64ID,
          SynthTrace::CreateBigIntRecord::Method::FromUint64,
          0xffffffffffffffff),
      *records[1]);
}

TEST_F(SynthTraceTest, BigIntToString) {
  jsi::BigInt b = jsi::BigInt::fromInt64(*rt, -42);
  SynthTrace::ObjectID bID = rt->getUniqueID(b);
  jsi::String str = b.toString(*rt, 16);
  SynthTrace::ObjectID strID = rt->getUniqueID(str);

  const auto &records = rt->trace().records();
  ASSERT_EQ(2, records.size());
  EXPECT_EQ_RECORD(
      SynthTrace::CreateBigIntRecord(
          dummyTime,
          bID,
          SynthTrace::CreateBigIntRecord::Method::FromInt64,
          -42),
      *records[0]);
  EXPECT_EQ_RECORD(
      SynthTrace::BigIntToStringRecord(dummyTime, strID, bID, 16), *records[1]);
}

// These tests fail on Windows.
#if defined(EXPECT_DEATH_IF_SUPPORTED) && !defined(_WINDOWS)
TEST(SynthTraceDeathTest, HostFunctionThrowsExceptionFails) {
  auto fn = [] {
    auto rt = SynthTraceTest::makeRuntime();
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
    throwingFunc.call(*rt);
  };
  EXPECT_DEATH_IF_SUPPORTED(fn(), "");
}

TEST(SynthTraceDeathTest, HostObjectThrowsExceptionFails) {
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

  auto fn = [] {
    auto rt = SynthTraceTest::makeRuntime();
    jsi::Object thro = jsi::Object::createFromHostObject(
        *rt, std::make_shared<ThrowingHostObject>());
    ASSERT_TRUE(thro.isHostObject(*rt));
    thro.getProperty(*rt, "foo");
  };
  EXPECT_DEATH_IF_SUPPORTED(fn(), "");
}
#endif

/// @}

struct SynthTraceRuntimeTest : public ::testing::Test {
  ::hermes::vm::RuntimeConfig config;
  std::string traceResult;
  std::unique_ptr<TracingHermesRuntime> traceRt;

  SynthTraceRuntimeTest()
      : config(::hermes::vm::RuntimeConfig::Builder()
                   .withSynthTraceMode(
                       ::hermes::vm::SynthTraceMode::TracingAndReplaying)
                   .withMicrotaskQueue(true)
                   .build()),
        traceRt(makeTracingHermesRuntime(
            makeHermesRuntime(config),
            config,
            /* traceStream */
            std::make_unique<llvh::raw_string_ostream>(traceResult),
            /* forReplay */ false)) {}

  jsi::Value eval(jsi::Runtime &rt, std::string code) {
    return rt.global().getPropertyAsFunction(rt, "eval").call(rt, code);
  }
};

/// @name Synth trace environment tests
/// @{

/// These tests validate that parts of the environment that were modified by the
/// TracingRuntime still appear the same to JS.
struct SynthTraceEnvironmentTest : public SynthTraceRuntimeTest {
  std::unique_ptr<jsi::Runtime> plainRt;

  SynthTraceEnvironmentTest() : plainRt(makeHermesRuntime(config)) {}

  std::vector<jsi::Runtime *> runtimes() {
    return {plainRt.get(), traceRt.get()};
  }
};

TEST_F(SynthTraceEnvironmentTest, NonDeterministicFunctionNames) {
  for (auto *rt : runtimes()) {
    auto getName = [&](std::string fn) {
      return eval(*rt, fn + ".name").toString(*rt).utf8(*rt);
    };
    EXPECT_EQ(getName("Math.random"), "random");
    EXPECT_EQ(getName("WeakRef"), "WeakRef");
    EXPECT_EQ(getName("WeakRef.prototype.deref"), "deref");
    EXPECT_EQ(getName("Date"), "Date");
    EXPECT_EQ(getName("Date.now"), "now");
  }
}
/// @}

/// @name Synth trace replay tests
/// @{
struct SynthTraceReplayTest : public SynthTraceRuntimeTest {
  std::unique_ptr<jsi::Runtime> replayRt;

  void replay() {
    traceRt.reset();

    tracing::TraceInterpreter::ExecuteOptions options;
    options.useTraceConfig = true;
    auto [_, rt] = tracing::TraceInterpreter::execFromMemoryBuffer(
        llvh::MemoryBuffer::getMemBuffer(traceResult), // traceBuf
        {}, // codeBufs
        options, // ExecuteOptions
        makeHermesRuntime);
    replayRt = std::move(rt);
  }
};

TEST_F(SynthTraceReplayTest, CreateObjectReplay) {
  {
    auto &rt = *traceRt;
    auto obj = jsi::Object(rt);
    obj.setProperty(rt, "bar", 5);
    rt.global().setProperty(rt, "foo", obj);
  }
  replay();
  {
    auto &rt = *replayRt;
    EXPECT_EQ(
        rt.global()
            .getPropertyAsObject(rt, "foo")
            .getProperty(rt, "bar")
            .asNumber(),
        5);
  }
}

TEST_F(SynthTraceRuntimeTest, WarmUpAndRepeatReplay) {
  {
    auto &rt = *traceRt;
    auto obj = jsi::Object(rt);
    obj.setProperty(rt, "bar", 5);
    rt.global().setProperty(rt, "foo", obj);
  }

  traceRt.reset();

  // ExecuteOptions has warmupReps and reps to run replay multiple times. It
  // should not have different results from replaying only once.
  tracing::TraceInterpreter::ExecuteOptions options;
  options.warmupReps = 1;
  options.reps = 3;
  options.useTraceConfig = true;
  auto [_, rt] = tracing::TraceInterpreter::execFromMemoryBuffer(
      llvh::MemoryBuffer::getMemBuffer(traceResult), // traceBuf
      {}, // codeBufs
      options, // ExecuteOptions
      makeHermesRuntime);

  {
    EXPECT_EQ(
        rt->global()
            .getPropertyAsObject(*rt, "foo")
            .getProperty(*rt, "bar")
            .asNumber(),
        5);
  }
}

// Replay the trace and generate a new one while doing so. They should produce
// the same results in the runtime regardless how many times we repeat.
TEST_F(SynthTraceRuntimeTest, TraceWhileReplaying) {
  // Generate the initial trace.
  {
    auto &rt = *traceRt;
    auto obj = jsi::Object(rt);
    obj.setProperty(rt, "bar", 5);
    rt.global().setProperty(rt, "foo", obj);
  }

  traceRt.reset();

  std::string previousResult = traceResult;

  for (int i = 0; i < 5; i++) {
    tracing::TraceInterpreter::ExecuteOptions options;
    options.useTraceConfig = true;
    options.traceEnabled = true;
    std::string newResult;

    auto [_, rt] = tracing::TraceInterpreter::execFromMemoryBuffer(
        llvh::MemoryBuffer::getMemBuffer(previousResult),
        {}, // codeBufs
        options, // ExecuteOptions
        [&newResult](const ::hermes::vm::RuntimeConfig &config) {
          return makeTracingHermesRuntime(
              makeHermesRuntime(config),
              config,
              std::make_unique<llvh::raw_string_ostream>(newResult),
              true // forReplay
          );
        }

    );

    EXPECT_EQ(
        rt->global()
            .getPropertyAsObject(*rt, "foo")
            .getProperty(*rt, "bar")
            .asNumber(),
        5);

    rt.reset(); // flush the result
    previousResult = newResult;
  }
}

TEST_F(SynthTraceReplayTest, SetPropertyReplay) {
  {
    auto &rt = *traceRt;
    auto asciiProp = jsi::PropNameID::forAscii(rt, "a");
    auto utf8Prop = jsi::PropNameID::forUtf8(rt, "b");
    auto strProp =
        jsi::PropNameID::forString(rt, jsi::String::createFromAscii(rt, "c"));
    rt.global().setProperty(rt, "symD", eval(rt, "Symbol('d')"));
    rt.global().setProperty(rt, "symE", eval(rt, "Symbol('e')"));
    auto symProp = jsi::PropNameID::forSymbol(
        rt, rt.global().getProperty(rt, "symE").asSymbol(rt));

    jsi::Object x(rt);
    x.setProperty(rt, asciiProp, "apple");
    x.setProperty(rt, utf8Prop, "banana");
    x.setProperty(rt, strProp, "coconut");
    x.setProperty(rt, symProp, "eggplant");
    rt.global().setProperty(rt, "x", x);
    eval(rt, "x[symD] = 'durian'");
  }
  replay();
  {
    auto &rt = *replayRt;
    EXPECT_EQ(eval(rt, "x.a").asString(rt).utf8(rt), "apple");
    EXPECT_EQ(eval(rt, "x.b").asString(rt).utf8(rt), "banana");
    EXPECT_EQ(eval(rt, "x.c").asString(rt).utf8(rt), "coconut");
    EXPECT_EQ(eval(rt, "x[symD]").asString(rt).utf8(rt), "durian");
    EXPECT_EQ(eval(rt, "x[symE]").asString(rt).utf8(rt), "eggplant");
  }
}

TEST_F(SynthTraceReplayTest, BigIntCreate) {
  {
    auto &rt = *traceRt;

    auto fromInt64 = jsi::BigInt::fromInt64(rt, -1ll);
    rt.global().setProperty(rt, "int64BigInt", fromInt64);
    rt.global().setProperty(rt, "int64String", fromInt64.toString(rt, 16));
    auto fromUint64 = jsi::BigInt::fromUint64(rt, ~0ull);
    rt.global().setProperty(rt, "uint64BigInt", fromUint64);
    rt.global().setProperty(rt, "uint64String", fromUint64.toString(rt, 8));
  }
  replay();
  {
    auto &rt = *replayRt;
    auto int64BigInt = rt.global().getProperty(rt, "int64BigInt").asBigInt(rt);
    auto int64String = rt.global().getProperty(rt, "int64String").asString(rt);
    auto uint64BigInt =
        rt.global().getProperty(rt, "uint64BigInt").asBigInt(rt);
    auto uint64String =
        rt.global().getProperty(rt, "uint64String").asString(rt);
    EXPECT_EQ(int64BigInt.getInt64(rt), -1ll);
    EXPECT_EQ(int64String.utf8(rt), "-1");
    EXPECT_EQ(uint64BigInt.getUint64(rt), ~0ull);
    EXPECT_EQ(uint64String.utf8(rt), "1777777777777777777777");
  }
}

TEST_F(SynthTraceReplayTest, HostObjectManipulation) {
  // HostObject for testing.
  // It allows to set either number or string property.
  class TestHostObject : public jsi::HostObject {
    using NumOrString = std::variant<double, std::string>;
    std::unordered_map<std::string, NumOrString> values_;

   public:
    jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
      auto it = values_.find(name.utf8(rt));
      if (it != values_.end()) {
        const auto &val = it->second;
        if (const auto *d = std::get_if<double>(&val)) {
          return jsi::Value(*d);
        } else if (const auto *s = std::get_if<std::string>(&val)) {
          return jsi::String::createFromUtf8(rt, *s);
        } else {
          return jsi::Value::undefined();
        }
      }
      return jsi::Value::undefined();
    }

    void set(
        jsi::Runtime &rt,
        const jsi::PropNameID &name,
        const jsi::Value &value) override {
      if (value.isNumber()) {
        values_[name.utf8(rt)] = value.asNumber();
      } else if (value.isString()) {
        values_[name.utf8(rt)] = value.asString(rt).utf8(rt);
      } else {
        throw std::runtime_error("Unsupported type");
      }
    }

    std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override {
      std::vector<jsi::PropNameID> ret;
      for (const auto &kv : values_) {
        ret.emplace_back(jsi::PropNameID::forUtf8(rt, kv.first));
      }
      return ret;
    }
  };

  //
  // Tracing
  //
  {
    auto &rt = *traceRt;
    auto o = jsi::Object::createFromHostObject(
        rt, std::make_shared<TestHostObject>());

    // Put "o" in global scope.
    rt.global().setProperty(rt, "o", o);

    // Add a new property with number
    o.setProperty(rt, "foo", 5);
    // Store the value of o.foo to foo1 so that we can use for verification
    eval(rt, "var foo1 = o.foo;");

    // Add another property with number
    o.setProperty(rt, "bar", 3);
    // Store the value of o.bar to bar so that we can use for verification
    eval(rt, "var bar = o.bar;");

    // Override o.foo with string
    o.setProperty(rt, "foo", "name");
    // Store the value of o.foo to foo2 so that we can use for verification
    eval(rt, "var foo2 = o.foo;");

    // Below verification is rather checking that TestHostObject is working as
    // expected.
    auto foo1 = eval(rt, "foo1;");
    auto foo2 = eval(rt, "foo2;");
    auto bar = eval(rt, "bar;");
    EXPECT_TRUE(foo1.isNumber());
    EXPECT_EQ(foo1.asNumber(), 5);
    EXPECT_TRUE(foo2.isString());
    EXPECT_EQ(foo2.asString(rt).utf8(rt), "name");
    EXPECT_TRUE(bar.isNumber());
    EXPECT_EQ(bar.asNumber(), 3);

    // Store the result of Object.getOwnPropertyNames(o) to "props" so that we
    // can use for verification
    eval(rt, "var props = Object.getOwnPropertyNames(o);");

    // Below verification is rather checking that TestHostObject is working as
    // expected.
    auto props = eval(rt, "props;");
    EXPECT_TRUE(props.isObject());
    EXPECT_TRUE(props.asObject(rt).isArray(rt));
    auto propsArray = props.asObject(rt).asArray(rt);
    for (size_t i = 0; i < propsArray.size(rt); ++i) {
      auto prop = propsArray.getValueAtIndex(rt, i);
      EXPECT_TRUE(prop.isString());
      EXPECT_TRUE(
          prop.getString(rt).utf8(rt) == "foo" ||
          prop.getString(rt).utf8(rt) == "bar");
    }
  }

  replay();

  //
  // Verification
  //
  {
    auto &rt = *replayRt;

    // We replayed the code above, that means we should have "foo1", "foo2",
    // "bar", "props" in global scope. Use them to verify that the replay
    // reproduced the same result for the host object manipulation.
    auto foo1 = eval(rt, "foo1;");
    auto foo2 = eval(rt, "foo2;");
    auto bar = eval(rt, "bar;");
    EXPECT_TRUE(foo1.isNumber());
    EXPECT_EQ(foo1.asNumber(), 5);
    EXPECT_TRUE(foo2.isString());
    EXPECT_EQ(foo2.asString(rt).utf8(rt), "name");
    EXPECT_TRUE(bar.isNumber());
    EXPECT_EQ(bar.asNumber(), 3);

    auto props = eval(rt, "props;");
    EXPECT_TRUE(props.isObject());
    EXPECT_TRUE(props.asObject(rt).isArray(rt));
    auto propsArray = props.asObject(rt).asArray(rt);
    for (size_t i = 0; i < propsArray.size(rt); ++i) {
      auto prop = propsArray.getValueAtIndex(rt, i);
      EXPECT_TRUE(prop.isString());
      EXPECT_TRUE(
          prop.getString(rt).utf8(rt) == "foo" ||
          prop.getString(rt).utf8(rt) == "bar");
    }

    // Note: Keeping below as commented out to note that we can't test like
    // below because it will fail. TraceInterpreter's FakeHostObject (see
    // TraceInterpreter::createHostObject()) does not work beyond just replaying
    // what was recorded.

    // auto o = rt.global().getPropertyAsObject(rt, "o");
    // EXPECT_TRUE(o.getProperty(rt, "foo").isString());
    // EXPECT_EQ(o.getProperty(rt, "foo").asString(rt).utf8(rt), "hello");
    // EXPECT_EQ(o.getProperty(rt, "bar").asNumber(), 5);
  }
}

// This test mainly verifies that SynthTrace records HostFunction calls and
// returns properly, and the replay to return the same values for each calls.
TEST_F(SynthTraceReplayTest, HostFunctionTraceAndReplayCallCount) {
  //
  // Tracing
  //
  {
    auto &rt = *traceRt;
    auto propName = jsi::PropNameID::forAscii(rt, "foo");
    const unsigned int paramCount = 0;
    int callCount = 0;

    // A HostFunction that returns different type of Value based on the call
    // count.
    auto func = jsi::Function::createFromHostFunction(
        rt,
        propName,
        paramCount,
        [callCount](
            jsi::Runtime &rt,
            const jsi::Value &thisVal,
            const jsi::Value *args,
            size_t count) mutable -> jsi::Value {
          ++callCount;
          if (callCount == 1) {
            return jsi::Value::undefined();
          } else if (callCount == 2) {
            return jsi::Value::null();
          } else if (callCount == 3) {
            return jsi::Value(3);
          } else if (callCount == 4) {
            return jsi::Value(true);
          } else if (callCount == 5) {
            return jsi::String::createFromUtf8(rt, "foobarbaz");
          }
          return jsi::Value::undefined();
        });

    rt.global().setProperty(rt, "foo", func);

    // Call foo() and store results to variables so that we can use them for
    // verification after replay.
    eval(rt, "var ret1 = foo()"); // undefined
    eval(rt, "var ret2 = foo()"); // null
    eval(rt, "var ret3 = foo()"); // 3
    eval(rt, "var ret4 = foo()"); // true
    eval(rt, "var ret5 = foo()"); // "foobarbaz"
    eval(rt, "var ret6 = foo()"); // undefined
  }

  replay();

  //
  // Verification
  //
  {
    auto &rt = *replayRt;
    auto ret1 = eval(rt, "ret1;");
    EXPECT_TRUE(ret1.isUndefined());

    auto ret2 = eval(rt, "ret2;");
    EXPECT_TRUE(ret2.isNull());

    auto ret3 = eval(rt, "ret3;");
    EXPECT_TRUE(ret3.isNumber());
    EXPECT_EQ(ret3.getNumber(), 3);

    auto ret4 = eval(rt, "ret4;");
    EXPECT_TRUE(ret4.isBool());
    EXPECT_EQ(ret4.getBool(), true);

    auto ret5 = eval(rt, "ret5;");
    EXPECT_TRUE(ret5.isString());
    EXPECT_EQ(ret5.getString(rt).utf8(rt), "foobarbaz");

    auto ret6 = eval(rt, "ret6;");
    EXPECT_TRUE(ret6.isUndefined());
  }
}

using JobQueueReplayTest = SynthTraceReplayTest;

TEST_F(JobQueueReplayTest, DrainSingleMicrotask) {
  {
    auto &rt = *traceRt;
    eval(rt, "var x = 3; HermesInternal.enqueueJob(function(){x = 4;})");
    rt.drainMicrotasks();
  }
  replay();
  {
    auto &rt = *replayRt;
    EXPECT_EQ(eval(rt, "x").asNumber(), 4);
  }
}

TEST_F(JobQueueReplayTest, DrainMultipleMicrotasks) {
  {
    auto &rt = *traceRt;
    eval(rt, R""""(
var x = 0;
function inc(){
  x += 1;
}
HermesInternal.enqueueJob(inc);
HermesInternal.enqueueJob(inc);
HermesInternal.enqueueJob(inc);
)"""");
    rt.drainMicrotasks();
  }
  replay();
  {
    auto &rt = *replayRt;
    EXPECT_EQ(eval(rt, "x").asNumber(), 3);
  }
}

TEST_F(JobQueueReplayTest, QueueMicrotask) {
  {
    auto &rt = *traceRt;
    auto microtask =
        eval(rt, "var x = 3; function updateX() { x = 4; }; updateX")
            .asObject(rt)
            .asFunction(rt);
    rt.queueMicrotask(microtask);
    rt.drainMicrotasks();
    EXPECT_EQ(eval(rt, "x").asNumber(), 4);
  }
  replay();
  {
    auto &rt = *replayRt;
    EXPECT_EQ(eval(rt, "x").asNumber(), 4);
  }
}

using NonDeterminismReplayTest = SynthTraceReplayTest;

TEST_F(NonDeterminismReplayTest, DateNowTest) {
  eval(*traceRt, "var x = Date.now();");
  auto dateTime = eval(*traceRt, "x").asNumber();

  replay();

  auto replayedTime = eval(*replayRt, "x").asNumber();
  EXPECT_EQ(dateTime, replayedTime);
}

TEST_F(NonDeterminismReplayTest, DateFuncTest) {
  eval(*traceRt, "var x = Date();");
  auto dateTime = eval(*traceRt, "x").asString(*traceRt).utf8(*traceRt);

  replay();

  auto replayedTime = eval(*replayRt, "x").asString(*replayRt).utf8(*replayRt);
  EXPECT_EQ(dateTime, replayedTime);
}

TEST_F(NonDeterminismReplayTest, DateNewTest) {
  eval(*traceRt, "var x = new Date();");
  auto dateTime = eval(*traceRt, "x.getTime()").asNumber();

  replay();

  auto replayedTime = eval(*replayRt, "x.getTime()").asNumber();
  EXPECT_EQ(dateTime, replayedTime);
}

TEST_F(NonDeterminismReplayTest, DateNewWithArgsTest) {
  eval(*traceRt, "var x = new Date(1, 2, 3, 4, 5, 6, 7).getTime();");
  auto dateTime = eval(*traceRt, "x").asNumber();

  replay();

  auto replayedTime = eval(*replayRt, "x").asNumber();
  EXPECT_EQ(dateTime, replayedTime);
}

TEST_F(NonDeterminismReplayTest, DateReplacedTest) {
  eval(*traceRt, R"(
var oldDate = Date;
Date = undefined;
var x = new oldDate();
)");
  auto traceTime = eval(*traceRt, "x.getTime()").asNumber();

  replay();

  auto replayedTime = eval(*replayRt, "x.getTime()").asNumber();
  EXPECT_EQ(traceTime, replayedTime);
}

TEST_F(NonDeterminismReplayTest, MathRandomTest) {
  eval(*traceRt, "var x = Math.random();");
  auto randVal = eval(*traceRt, "x").asNumber();

  replay();

  auto replayedVal = eval(*replayRt, "x").asNumber();
  EXPECT_EQ(randVal, replayedVal);
}

TEST_F(NonDeterminismReplayTest, SimpleWeakRefTest) {
  eval(*traceRt, R"(
var obj = {x: 5};
var ref = new WeakRef(obj);
var x = ref.deref().x;
)");

  auto traceX = eval(*traceRt, "x").asNumber();

  replay();

  auto replayX = eval(*replayRt, "x").asNumber();
  EXPECT_EQ(traceX, replayX);
}

TEST_F(NonDeterminismReplayTest, WeakRefTest) {
  eval(*traceRt, R"(
var obj = {x: 5};
var ref = new WeakRef(obj);
obj = null;
var firstDeref = ref.deref();
)");
  traceRt->drainMicrotasks();
  eval(*traceRt, R"(
gc();
var secondDeref = ref.deref();
)");
  auto secondDeref = eval(*traceRt, "secondDeref").isUndefined();

  replay();

  auto replayedFirst = eval(*replayRt, "firstDeref.x").asNumber();
  auto replayedSecond = eval(*replayRt, "secondDeref").isUndefined();
  EXPECT_EQ(replayedFirst, 5);
  EXPECT_EQ(secondDeref, replayedSecond);
}

TEST_F(NonDeterminismReplayTest, HermesInternalGetInstrumentedStatsTest) {
  // Use JSON.stringify to serialize stats object to verify the result easily.
  auto jsonStringify = [](jsi::Runtime &runtime,
                          const jsi::Value &val) -> std::string {
    auto JSON = runtime.global().getPropertyAsObject(runtime, "JSON");
    auto stringify = JSON.getPropertyAsFunction(runtime, "stringify");

    facebook::jsi::Value stringifyRes = stringify.call(runtime, val);
    assert(stringifyRes.isString());
    const auto statsStr = stringifyRes.getString(runtime).utf8(runtime);
    return statsStr;
  };

  // Create some objects
  eval(*traceRt, R"(
    var s512 = " ".repeat(128);
    var arr = [];
  )");

  // Run GC to get meaningful stats
  traceRt->instrumentation().collectGarbage("forced for stats");

  {
    // Store current stats to var "stats1"
    const jsi::Value stats1 = eval(*traceRt, R"(
      var stats1 = HermesInternal.getInstrumentedStats();
      stats1;
    )");

    // Run GC again to get different stats
    traceRt->instrumentation().collectGarbage("forced for stats");

    // Store second stats to var "stats2"
    const jsi::Value stats2 = eval(*traceRt, R"(
      var stats2 = HermesInternal.getInstrumentedStats();
      stats2;
    )");

    // Return type of getInstrumentedStats should be an object
    EXPECT_TRUE(stats1.isObject());
    EXPECT_TRUE(stats2.isObject());
  }

  {
    const std::string stats1Str =
        jsonStringify(*traceRt, eval(*traceRt, "stats1"));
    const std::string stats2Str =
        jsonStringify(*traceRt, eval(*traceRt, "stats2"));

    //
    // replay
    //
    replay();

    // Return type of getInstrumentedStats should be an object
    EXPECT_TRUE(eval(*replayRt, "stats1").isObject());
    EXPECT_TRUE(eval(*replayRt, "stats2").isObject());

    const std::string replayStats1Str =
        jsonStringify(*replayRt, eval(*replayRt, "stats1"));
    const std::string replayStats2Str =
        jsonStringify(*replayRt, eval(*replayRt, "stats2"));

    // Compare the results between the original and the replayed run.
    EXPECT_EQ(stats1Str, replayStats1Str);
    EXPECT_EQ(stats2Str, replayStats2Str);
  }
}

// Verify that jsi::Runtime::setExternalMemoryPressure() is properly traced and
// replayed
TEST(SynthTraceReplayTestNoFixture, ExternalMemoryTest) {
  std::string traceResult;
  std::vector<size_t>
      amounts; // Record the "amount" passed to setExternalMemoryPressure()

  // Tracing
  {
    const ::hermes::vm::RuntimeConfig config(
        ::hermes::vm::RuntimeConfig::Builder()
            .withSynthTraceMode(::hermes::vm::SynthTraceMode::Tracing)
            .build());
    std::unique_ptr<TracingHermesRuntime> traceRt = makeTracingHermesRuntime(
        makeHermesRuntime(config),
        config,
        std::make_unique<llvh::raw_string_ostream>(traceResult),
        /* forReplay */ false);

    for (size_t i = 0; i < 200; i++) {
      auto o = jsi::Object::createFromHostObject(
          *traceRt, std::make_shared<jsi::HostObject>());

      size_t amount = (i + 1) * 1024;
      o.setExternalMemoryPressure(*traceRt, amount);
      amounts.push_back(amount);
    }

    traceRt.reset();
  }

  // Just to hook setExternalMemoryPressure() and record the invokations of it
  // so that we can verify the function calls were traced and replayed
  // correctly.
  class ReplayRuntime : public jsi::RuntimeDecorator<jsi::Runtime> {
   public:
    using RD = RuntimeDecorator<jsi::Runtime>;
    ReplayRuntime(
        std::unique_ptr<jsi::Runtime> runtime,
        const ::hermes::vm::RuntimeConfig &conf)
        : jsi::RuntimeDecorator<jsi::Runtime>(*runtime),
          runtime_(std::move(runtime)) {}

    void setExternalMemoryPressure(const jsi::Object &obj, size_t amount)
        override {
      RD::setExternalMemoryPressure(obj, amount);
      amounts.push_back(amount);
    }

    std::vector<size_t> amounts;

   private:
    std::unique_ptr<jsi::Runtime> runtime_;
  };

  // Replaying

  tracing::TraceInterpreter::ExecuteOptions options;
  options.useTraceConfig = true;
  auto [_, rt] = tracing::TraceInterpreter::execFromMemoryBuffer(
      llvh::MemoryBuffer::getMemBuffer(traceResult),
      {}, // codeBufs
      options,
      [](const ::hermes::vm::RuntimeConfig &config)
          -> std::unique_ptr<jsi::Runtime> {
        return std::make_unique<ReplayRuntime>(
            makeHermesRuntime(config), config);
      });

  auto replayRt = dynamic_cast<ReplayRuntime *>(rt.get());

  // Verify that the call counts of setExternalMemoryPressure() are the same
  EXPECT_EQ(amounts.size(), replayRt->amounts.size());

  // Verify that the arguments passed to setExternalMemoryPressure() are the
  // same
  for (size_t i = 0; i < amounts.size(); i++) {
    EXPECT_EQ(amounts[i], replayRt->amounts[i]);
  }
}

/// @}

} // namespace
