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

using namespace facebook::hermes::tracing;
using namespace facebook::hermes;
namespace jsi = facebook::jsi;

namespace {

struct SynthTraceTest : public ::testing::Test {
  std::unique_ptr<TracingHermesRuntime> rt;
  SynthTrace::TimeSinceStart dummyTime{SynthTrace::TimeSinceStart::zero()};

  static std::unique_ptr<TracingHermesRuntime> makeRuntime() {
    ::hermes::vm::RuntimeConfig config =
        ::hermes::vm::RuntimeConfig::Builder().withTraceEnabled(true).build();
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
      identityID,
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
      aStringID,
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
      aPropID,
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
      bPropID,
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
      aStringID,
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
      bPropID,
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
      aStringID
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
      bPropID
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
      aStringID,
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
      getHappenedPropNameID,
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
      xPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.x,
#endif
      SynthTrace::encodeNumber(0));
  EXPECT_EQ_RECORD(gprExpect0, *records[8]);
  // Called setProperty on the proxy.
  auto sprExpect1 = SynthTrace::SetPropertyRecord(
      dummyTime,
      objID,
      xPropNameID,
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
      setHappenedPropNameID,
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
      getHappenedPropNameID,
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
      xPropNameID,
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
      oPropNameID,
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
      hoPropNameID,
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
      oPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(gprExpect0, *records[8]);
  auto gprExpect1 = SynthTrace::GetPropertyRecord(
      dummyTime,
      oObjID,
      observedXPropNameUID,
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
      oPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      cs.o,
#endif
      SynthTrace::encodeObject(oObjID));
  EXPECT_EQ_RECORD(gprExpect2, *records[12]);
  auto sprExpect = SynthTrace::SetPropertyRecord(
      dummyTime,
      oObjID,
      observedYPropNameUID,
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
      xResPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      xRes,
#endif
      SynthTrace::encodeNumber(7));
  EXPECT_EQ_RECORD(gprExpect3, *records[18]);
  auto gprExpect4 = SynthTrace::GetPropertyRecord(
      dummyTime,
      globID,
      yResPropNameID,
#ifdef HERMESVM_API_TRACE_DEBUG
      yRes,
#endif
      SynthTrace::encodeBool(false));
  EXPECT_EQ_RECORD(gprExpect4, *records[19]);
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

/// @name Synth trace replay tests
/// @{

struct SynthTraceReplayTest : public ::testing::Test {
  ::hermes::vm::RuntimeConfig config;
  std::string traceResult;
  std::unique_ptr<TracingHermesRuntime> traceRt;
  std::unique_ptr<jsi::Runtime> replayRt;

  SynthTraceReplayTest(::hermes::vm::RuntimeConfig conf)
      : config(conf),
        traceRt(makeTracingHermesRuntime(
            makeHermesRuntime(config),
            config,
            /* traceStream */
            std::make_unique<llvh::raw_string_ostream>(traceResult),
            /* forReplay */ false)) {}

  SynthTraceReplayTest()
      : SynthTraceReplayTest(::hermes::vm::RuntimeConfig::Builder()
                                 .withTraceEnabled(true)
                                 .build()) {}

  void replay() {
    traceRt.reset();
    replayRt = makeHermesRuntime(config);
    tracing::TraceInterpreter::execFromMemoryBuffer(
        llvh::MemoryBuffer::getMemBuffer(traceResult), {}, *replayRt, {});
  }

  jsi::Value eval(jsi::Runtime &rt, const char *code) {
    return rt.global().getPropertyAsFunction(rt, "eval").call(rt, code);
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

struct JobQueueReplayTest : public SynthTraceReplayTest {
  JobQueueReplayTest()
      : SynthTraceReplayTest(::hermes::vm::RuntimeConfig::Builder()
                                 .withTraceEnabled(true)
                                 .withEnableHermesInternal(true)
                                 .withMicrotaskQueue(true)
                                 .build()) {}
};

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

struct NonDeterminismReplayTest : public SynthTraceReplayTest {
  NonDeterminismReplayTest()
      : SynthTraceReplayTest(::hermes::vm::RuntimeConfig::Builder()
                                 .withTraceEnabled(true)
                                 .withEnableHermesInternal(true)
                                 .withMicrotaskQueue(true)
                                 .build()) {}
};

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

/// @}

} // namespace
