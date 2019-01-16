#include <gtest/gtest.h>
#include <hermes/CompileJS.h>
#include <hermes/hermes.h>

using namespace facebook::jsi;
using namespace facebook::hermes;

struct HermesTestHelper {
  static size_t rootsListLength(const HermesRuntime &rt) {
    return rt.rootsListLength();
  }

  static int64_t calculateRootsListChange(
      const HermesRuntime &rt,
      std::function<void(void)> f) {
    auto before = rootsListLength(rt);
    f();
    return rootsListLength(rt) - before;
  }
};

namespace {

class HermesRuntimeTest : public ::testing::Test {
 public:
  HermesRuntimeTest() : rt(makeHermesRuntime()) {}

 protected:
  Value eval(const char *code) {
    return rt->global().getPropertyAsFunction(*rt, "eval").call(*rt, code);
  }

  std::shared_ptr<HermesRuntime> rt;
};

using HermesRuntimeDeathTest = HermesRuntimeTest;

// In JSC there's a bug where host functions are always ran with a this in
// nonstrict mode so this must be a hermes only test. See
// https://es5.github.io/#x10.4.3 for more info.
TEST_F(HermesRuntimeTest, StrictHostFunctionBindTest) {
  Function coolify = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "coolify"),
      0,
      [](Runtime &, const Value &thisVal, const Value *args, size_t count) {
        EXPECT_TRUE(thisVal.isUndefined());
        return thisVal.isUndefined();
      });
  rt->global().setProperty(*rt, "coolify", coolify);
  EXPECT_TRUE(eval("(function() {"
                   "  \"use strict\";"
                   "  return coolify.bind(undefined)();"
                   "})()")
                  .getBool());
}

TEST_F(HermesRuntimeTest, ArrayBufferTest) {
  eval(
      "var buffer = new ArrayBuffer(16);\
        var int32View = new Int32Array(buffer);\
        int32View[0] = 1234;\
        int32View[1] = 5678;");

  Object object = rt->global().getPropertyAsObject(*rt, "buffer");
  EXPECT_TRUE(object.isArrayBuffer(*rt));

  auto arrayBuffer = object.getArrayBuffer(*rt);
  EXPECT_EQ(arrayBuffer.size(*rt), 16);

#ifndef HERMESVM_API_TRACE
  // Since synthetic benchmarks disallow raw memory access, don't do this test.
  int32_t *buffer = reinterpret_cast<int32_t *>(arrayBuffer.data(*rt));
  EXPECT_EQ(buffer[0], 1234);
  EXPECT_EQ(buffer[1], 5678);
#endif
}

TEST_F(HermesRuntimeTest, BytecodeTest) {
  const uint8_t empty[] = {};
  EXPECT_FALSE(HermesRuntime::isHermesBytecode(empty, sizeof(empty)));
  const uint8_t shortBytes[] = {1, 2, 3};
  EXPECT_FALSE(HermesRuntime::isHermesBytecode(shortBytes, sizeof(shortBytes)));
  uint8_t longBytes[1024];
  memset(longBytes, 'H', sizeof(longBytes));
  EXPECT_FALSE(HermesRuntime::isHermesBytecode(longBytes, sizeof(longBytes)));

  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("x = 1", bytecode));
  EXPECT_TRUE(HermesRuntime::isHermesBytecode(
      reinterpret_cast<const uint8_t *>(bytecode.data()), bytecode.size()));
  rt->evaluateJavaScript(
      std::unique_ptr<StringBuffer>(new StringBuffer(bytecode)), "");
  EXPECT_EQ(rt->global().getProperty(*rt, "x").getNumber(), 1);
}

// In JSC we use multiple threads in our implementation of JSI so we can't
// use the ASSERT_DEATH macros when testing that implementation.
// Asserts are compiled out of opt builds
#ifndef NDEBUG
#ifdef ASSERT_DEATH
TEST_F(HermesRuntimeDeathTest, ValueTest) {
  ASSERT_DEATH(eval("'slay'").getNumber(), "Assertion.*isNumber");
  ASSERT_DEATH(eval("123").getString(*rt), "Assertion.*isString");
}
#endif
#endif

TEST_F(HermesRuntimeTest, DontGrowWhenMoveObjectOutOfValue) {
  Value val = Object(*rt);
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(*rt, [&]() {
    Object obj = std::move(val).getObject(*rt);
    (void)obj;
  });
  EXPECT_EQ(rootsDelta, 0);
}

TEST_F(HermesRuntimeTest, DontGrowWhenCloneObject) {
  Value val = Object(*rt);
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(*rt, [&]() {
    for (int i = 0; i < 1000; i++) {
      Object obj = val.getObject(*rt);
      (void)obj;
    }
  });
  EXPECT_EQ(rootsDelta, 0);
}

TEST_F(HermesRuntimeTest, ScopeCleansUpObjectReferences) {
  Object o1(*rt);
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(*rt, [&]() {
    Scope s(*rt);
    Object o2(*rt);
    Object o3(*rt);
  });
  EXPECT_EQ(rootsDelta, 0);
}

TEST_F(HermesRuntimeTest, ReferencesCanEscapeScope) {
  Value v;
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(*rt, [&]() {
    Scope s(*rt);
    Object o1(*rt);
    Object o2(*rt);
    Object o3(*rt);
    v = std::move(o2);
  });
  EXPECT_EQ(rootsDelta, 1);
}

TEST_F(HermesRuntimeTest, HostObjectWithOwnProperties) {
  class HostObjectWithPropertyNames : public HostObject {
    std::vector<PropNameID> getPropertyNames(Runtime &rt) override {
      return PropNameID::names(rt, "prop1", "1", "2", "prop2", "3");
    }
    Value get(Runtime &runtime, const PropNameID &name) override {
      if (PropNameID::compare(
              runtime, name, PropNameID::forAscii(runtime, "prop1")))
        return 10;
      return Value();
    }
  };

  Object ho = Object::createFromHostObject(
      *rt, std::make_shared<HostObjectWithPropertyNames>());
  rt->global().setProperty(*rt, "ho", ho);

  EXPECT_TRUE(eval("\"prop1\" in ho").getBool());
  EXPECT_TRUE(eval("1 in ho").getBool());
  EXPECT_TRUE(eval("2 in ho").getBool());
  EXPECT_TRUE(eval("\"prop2\" in ho").getBool());
  EXPECT_TRUE(eval("3 in ho").getBool());
  // HostObjects say they own any property, even if it's not in their property
  // names list.
  // This is an explicit design choice, to avoid the runtime and API costs of
  // handling checking for property existence.
  EXPECT_TRUE(eval("\"foo\" in ho").getBool());

  EXPECT_TRUE(eval("var properties = Object.getOwnPropertyNames(ho);"
                   "properties[0] === '1' && "
                   "properties[1] === '2' && "
                   "properties[2] === '3' && "
                   "properties[3] === 'prop1' && "
                   "properties[4] === 'prop2' && "
                   "properties.length === 5")
                  .getBool());
  EXPECT_TRUE(eval("ho[2] === undefined").getBool());
  EXPECT_TRUE(eval("ho.prop2 === undefined").getBool());

  eval("Object.defineProperty(ho, '0', {value: 'hi there'})");
  eval("Object.defineProperty(ho, '2', {value: 'hi there'})");
  eval("Object.defineProperty(ho, '4', {value: 'hi there'})");
  eval("Object.defineProperty(ho, 'prop2', {value: 'hi there'})");

  EXPECT_TRUE(eval("var properties = Object.getOwnPropertyNames(ho);"
                   "properties[0] === '0' && "
                   "properties[1] === '1' && "
                   "properties[2] === '2' && "
                   "properties[3] === '3' && "
                   "properties[4] === '4' && "
                   "properties[5] === 'prop2' && "
                   "properties[6] === 'prop1' && "
                   "properties.length === 7")
                  .getBool());
  EXPECT_TRUE(eval("ho[2] === 'hi there'").getBool());
  EXPECT_TRUE(eval("ho.prop2 === 'hi there'").getBool());

  // hasOwnProperty() always succeeds on HostObject
  EXPECT_TRUE(
      eval("Object.prototype.hasOwnProperty.call(ho, 'prop1')").getBool());
  EXPECT_TRUE(
      eval("Object.prototype.hasOwnProperty.call(ho, 'any-string')").getBool());

  // getOwnPropertyDescriptor() always succeeds on HostObject
  EXPECT_TRUE(eval("var d = Object.getOwnPropertyDescriptor(ho, 'prop1');"
                   "d != undefined && "
                   "d.value == 10 && "
                   "d.enumerable && "
                   "d.writable ")
                  .getBool());
  EXPECT_TRUE(eval("var d = Object.getOwnPropertyDescriptor(ho, 'any-string');"
                   "d != undefined && "
                   "d.value == undefined && "
                   "d.enumerable && "
                   "d.writable")
                  .getBool());
}

#ifdef HERMESVM_API_TRACE
#ifdef EXPECT_DEATH
TEST_F(HermesRuntimeTest, HostFunctionThrowsExceptionFails) {
  // TODO (T28293178) Remove this once exceptions are supported.
  Function throwingFunc = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "thrower"),
      0,
      [](Runtime &rt, const Value &thisVal, const Value *args, size_t count)
          -> Value { throw std::runtime_error("Cannot call"); });
  EXPECT_DEATH({ throwingFunc.call(*rt); }, "");
}

TEST_F(HermesRuntimeTest, HostObjectThrowsExceptionFails) {
  // TODO (T28293178) Remove this once exceptions are supported.
  class ThrowingHostObject : public HostObject {
    Value get(Runtime &rt, const PropNameID &sym) override {
      throw std::runtime_error("Cannot get");
    }

    void set(Runtime &rt, const PropNameID &sym, const Value &val) override {
      throw std::runtime_error("Cannot set");
    }
  };

  Object thro =
      Object::createFromHostObject(*rt, std::make_shared<ThrowingHostObject>());
  ASSERT_TRUE(thro.isHostObject(*rt));
  EXPECT_DEATH({ thro.getProperty(*rt, "foo"); }, "");
  EXPECT_DEATH({ thro.setProperty(*rt, "foo", Value::undefined()); }, "");
}
#endif
#endif

// TODO mhorowitz: move this to jsi/testlib.cpp once we have impls for all VMs
TEST_F(HermesRuntimeTest, WeakReferences) {
  Object o = eval("({one: 1})").getObject(*rt);
  WeakObject wo = WeakObject(*rt, o);
  rt->global().setProperty(*rt, "obj", o);

  eval("gc()");

  Value v = wo.lock(*rt);

  // At this point, the object has three strong refs (C++ o, v; JS global.obj).

  EXPECT_TRUE(v.isObject());
  EXPECT_EQ(v.getObject(*rt).getProperty(*rt, "one").asNumber(), 1);

  // Now start removing references.

  v = nullptr;

  // Two left

  eval("gc()");
  EXPECT_EQ(wo.lock(*rt).getObject(*rt).getProperty(*rt, "one").asNumber(), 1);

  o = Object(*rt);

  // Now one, only JS

  eval("gc()");
  EXPECT_EQ(wo.lock(*rt).getObject(*rt).getProperty(*rt, "one").asNumber(), 1);

  eval("obj = null");

  // Now none.

  eval("gc()");
  EXPECT_TRUE(wo.lock(*rt).isUndefined());

  // test where the last ref is C++

  o = eval("({two: 2})").getObject(*rt);
  wo = WeakObject(*rt, o);
  v = Value(*rt, o);

  eval("gc()");
  EXPECT_EQ(wo.lock(*rt).getObject(*rt).getProperty(*rt, "two").asNumber(), 2);

  v = nullptr;

  eval("gc()");
  EXPECT_EQ(wo.lock(*rt).getObject(*rt).getProperty(*rt, "two").asNumber(), 2);

  o = Object(*rt);

  eval("gc()");
  EXPECT_TRUE(wo.lock(*rt).isUndefined());
}

TEST_F(HermesRuntimeTest, SourceURLAppearsInBacktraceTest) {
  std::string sourceURL = "//SourceURLAppearsInBacktraceTest/Test/URL";
  std::string sourceCode = R"(
function thrower() { throw new Error('Test Error Message')}
function throws1() { thrower(); }
throws1();
)";
  std::string bytecode;
  bool compiled = hermes::compileJS(sourceCode, bytecode);
  ASSERT_TRUE(compiled) << "JS source should have compiled";

  for (const std::string &code : {sourceCode, bytecode}) {
    bool caught = false;
    try {
      rt->evaluateJavaScript(std::make_unique<StringBuffer>(code), sourceURL);
    } catch (facebook::jsi::JSError err) {
      caught = true;
      EXPECT_TRUE(err.getStack().find(sourceURL) != std::string::npos)
          << "Backtrace should contain source URL";
    }
    EXPECT_TRUE(caught) << "JS should have thrown an exception";
  }
}

} // namespace
