/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <hermes/BCGen/HBC/BytecodeFileFormat.h>
#include <hermes/CompileJS.h>
#include <hermes/Public/JSOutOfMemoryError.h>
#include <hermes/VM/TimeLimitMonitor.h>
#include <hermes/hermes.h>
#include <hermes_sandbox/HermesSandboxRuntime.h>
#include <jsi/instrumentation.h>
#include <jsi/test/testlib.h>

#include <atomic>
#include <tuple>

using namespace facebook::jsi;
using namespace facebook::hermes;

struct HermesTestHelper {
  static size_t rootsListLength(const HermesRuntime &rt) {
    return rt.rootsListLengthForTests();
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

class HermesRuntimeTestBase {
 public:
  HermesRuntimeTestBase(std::unique_ptr<Runtime> rt) : rt(std::move(rt)) {}

 protected:
  Value eval(const char *code) {
    return rt->global().getPropertyAsFunction(*rt, "eval").call(*rt, code);
  }

  std::unique_ptr<Runtime> rt;
};

/// TODO: Run these tests against all jsi::Runtimes implemented on top of
///       Hermes, similar to HermesRuntimeTest below.
class HermesRuntimeCustomConfigTest : public ::testing::Test,
                                      public HermesRuntimeTestBase {
 public:
  HermesRuntimeCustomConfigTest(::hermes::vm::RuntimeConfig runtimeConfig)
      : HermesRuntimeTestBase(makeHermesRuntime(runtimeConfig)) {}
};

class HermesRuntimeTest : public ::testing::TestWithParam<RuntimeFactory>,
                          public HermesRuntimeTestBase {
 public:
  HermesRuntimeTest() : HermesRuntimeTestBase(GetParam()()) {}

  /// Evaluate the given buffer containing either source or bytecode in the
  /// runtime and return the result.
  Value evaluateSourceOrBytecode(
      const std::shared_ptr<const Buffer> &buf,
      const std::string &sourceURL) {
    // HermesSandboxRuntime does not permit bytecode in evaluateJavaScript, so
    // check for it and call the appropriate method.
    if (auto *sbrt = dynamic_cast<HermesSandboxRuntime *>(rt.get()))
      if (HermesSandboxRuntime::isHermesBytecode(buf->data(), buf->size()))
        return sbrt->evaluateHermesBytecode(buf, sourceURL);

    return rt->evaluateJavaScript(buf, sourceURL);
  }
};

// In JSC there's a bug where host functions are always ran with a this in
// nonstrict mode so this must be a hermes only test. See
// https://es5.github.io/#x10.4.3 for more info.
TEST_P(HermesRuntimeTest, StrictHostFunctionBindTest) {
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

TEST_P(HermesRuntimeTest, DescriptionTest) {
  // Minimally, if the description doesn't include "Hermes", something
  // is wrong.
  EXPECT_NE(rt->description().find("Hermes"), std::string::npos);

  std::shared_ptr<facebook::jsi::Runtime> rt2 = makeHermesRuntime(
      ::hermes::vm::RuntimeConfig::Builder()
          .withGCConfig(
              ::hermes::vm::GCConfig::Builder().withName("ForTesting").build())
          .build());
  EXPECT_NE(rt2->description().find("Hermes"), std::string::npos);
}

TEST_P(HermesRuntimeTest, ArrayBufferTest) {
  eval(
      "var buffer = new ArrayBuffer(16);\
        var int32View = new Int32Array(buffer);\
        int32View[0] = 1234;\
        int32View[1] = 5678;");

  Object object = rt->global().getPropertyAsObject(*rt, "buffer");
  EXPECT_TRUE(object.isArrayBuffer(*rt));

  auto arrayBuffer = object.getArrayBuffer(*rt);
  EXPECT_EQ(arrayBuffer.size(*rt), 16);

  // HermesSandboxRuntime does not support getting the ArrayBuffer's data.
  if (!dynamic_cast<HermesSandboxRuntime *>(rt.get())) {
    int32_t *buffer = reinterpret_cast<int32_t *>(arrayBuffer.data(*rt));
    EXPECT_EQ(buffer[0], 1234);
    EXPECT_EQ(buffer[1], 5678);
  }
}

class HermesRuntimeTestMethodsTest : public HermesRuntimeCustomConfigTest {
 public:
  HermesRuntimeTestMethodsTest()
      : HermesRuntimeCustomConfigTest(
            ::hermes::vm::RuntimeConfig::Builder()
                .withEnableHermesInternalTestMethods(true)
                .build()) {}
};

TEST_F(HermesRuntimeTestMethodsTest, ExternalArrayBufferTest) {
  struct FixedBuffer : MutableBuffer {
    size_t size() const override {
      return sizeof(arr);
    }
    uint8_t *data() override {
      return reinterpret_cast<uint8_t *>(arr.data());
    }

    std::array<uint32_t, 256> arr;
  };

  {
    auto buf = std::make_shared<FixedBuffer>();
    for (uint32_t i = 0; i < buf->arr.size(); i++)
      buf->arr[i] = i;
    auto arrayBuffer = ArrayBuffer(*rt, buf);
    auto square = eval(
        R"#(
(function (buf) {
  var view = new Uint32Array(buf);
  for(var i = 0; i < view.length; i++) view[i] = view[i] * view[i];
})
)#");
    square.asObject(*rt).asFunction(*rt).call(*rt, arrayBuffer);
    for (uint32_t i = 0; i < 256; i++)
      EXPECT_EQ(buf->arr[i], i * i);
  }

  {
    auto buf = std::make_shared<FixedBuffer>();
    std::weak_ptr<FixedBuffer> weakBuf(buf);
    auto arrayBuffer = ArrayBuffer(*rt, std::move(buf));
    auto detach = eval(
        R"#(
(function (buf) {
  var view = new Uint32Array(buf);
  HermesInternal.detachArrayBuffer(buf);
  view[0] = 5;
  view[0]
})
)#");
    EXPECT_TRUE(detach.asObject(*rt)
                    .asFunction(*rt)
                    .call(*rt, arrayBuffer)
                    .isUndefined());
    rt->instrumentation().collectGarbage("");
    EXPECT_TRUE(weakBuf.expired());
  }
}

TEST_F(HermesRuntimeTestMethodsTest, DetachedArrayBuffer) {
  auto ab = eval(
                R"(
  var x  = new ArrayBuffer(10);
  HermesInternal.detachArrayBuffer(x);
  x
)")
                .getObject(*rt)
                .getArrayBuffer(*rt);
  EXPECT_THROW(ab.data(*rt), JSINativeException);
}

TEST_P(HermesRuntimeTest, BytecodeTest) {
  const uint8_t shortBytes[] = {1, 2, 3};
  EXPECT_FALSE(HermesRuntime::isHermesBytecode(shortBytes, 0));
  EXPECT_FALSE(HermesRuntime::isHermesBytecode(shortBytes, sizeof(shortBytes)));
  uint8_t longBytes[1024];
  memset(longBytes, 'H', sizeof(longBytes));
  EXPECT_FALSE(HermesRuntime::isHermesBytecode(longBytes, sizeof(longBytes)));

  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("x = 1", bytecode));
  EXPECT_TRUE(HermesRuntime::isHermesBytecode(
      reinterpret_cast<const uint8_t *>(bytecode.data()), bytecode.size()));
  evaluateSourceOrBytecode(
      std::unique_ptr<StringBuffer>(new StringBuffer(bytecode)), "");
  EXPECT_EQ(rt->global().getProperty(*rt, "x").getNumber(), 1);

  EXPECT_EQ(HermesRuntime::getBytecodeVersion(), hermes::hbc::BYTECODE_VERSION);
}

TEST(HermesRuntimePreparedJavaScriptTest, BytecodeTest) {
  auto rt = makeHermesRuntime();
  rt->evaluateJavaScript(std::make_unique<StringBuffer>("var q = 0;"), "");
  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("q++", bytecode));
  auto prep =
      rt->prepareJavaScript(std::make_unique<StringBuffer>(bytecode), "");
  EXPECT_EQ(rt->global().getProperty(*rt, "q").getNumber(), 0);
  rt->evaluatePreparedJavaScript(prep);
  EXPECT_EQ(rt->global().getProperty(*rt, "q").getNumber(), 1);
  rt->evaluatePreparedJavaScript(prep);
  EXPECT_EQ(rt->global().getProperty(*rt, "q").getNumber(), 2);
}

TEST_P(HermesRuntimeTest, CompileWithSourceMapTest) {
  /* original source:
  const a: number = 12;
  class MyClass {
    val: number;
    constructor(val: number) {
      this.val = val;
    }
    doSomething(a: number, b: number) {
      invalidRef.sum = a + b + this.val;
    }
  }
  const c = new MyClass(3);
  c.doSomething(a, 15);*/
  const char *TestSource = R"#('use strict';
var a = 12;
var MyClass = /** @class */ (function () {
    function MyClass(val) {
        this.val = val;
    }
    MyClass.prototype.doSomething = function (a, b) {
        invalidRef.sum = a + b + this.val;
    };
    return MyClass;
}());
var c = new MyClass(3);
c.doSomething(a, 15);
//# sourceMappingURL=script.js.map)#";
  const char *TestSourceMap = R"#({
    "version":3,
    "file":"script.js",
    "sourceRoot":"",
    "sources":["script.ts"],
    "names":[],
    "mappings": ";AAAA,IAAM,CAAC,GAAW,EAAE,CAAC;AACrB;IAEI,iBAAY,GAAW;QACnB,IAAI,CAAC,GAAG,GAAG,GAAG,CAAC;IACnB,CAAC;IACD,6BAAW,GAAX,UAAY,CAAS,EAAE,CAAS;QAC5B,UAAU,CAAC,GAAG,GAAG,CAAC,GAAG,CAAC,GAAG,IAAI,CAAC,GAAG,CAAC;IACtC,CAAC;IACL,cAAC;AAAD,CAAC,AARD,IAQC;AACD,IAAM,CAAC,GAAG,IAAI,OAAO,CAAC,CAAC,CAAC,CAAC;AACzB,CAAC,CAAC,WAAW,CAAC,CAAC,EAAE,EAAE,CAAC,CAAC"
  })#";

  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS(
      TestSource,
      "script.js",
      bytecode,
      true,
      true,
      nullptr,
      std::optional<std::string_view>(TestSourceMap)));
  EXPECT_TRUE(HermesRuntime::isHermesBytecode(
      reinterpret_cast<const uint8_t *>(bytecode.data()), bytecode.size()));
  try {
    evaluateSourceOrBytecode(
        std::unique_ptr<StringBuffer>(new StringBuffer(bytecode)), "");
    FAIL() << "Expected JSIException";
  } catch (const facebook::jsi::JSIException &err) {
    EXPECT_STREQ(
        "Property 'invalidRef' doesn't exist\n\nReferenceError: Property 'invalidRef' doesn't exist\n    at anonymous (script.ts:8:9)\n    at global (script.ts:12:14)",
        err.what());
  }
}

TEST_P(HermesRuntimeTest, JumpTableBytecodeTest) {
  std::string code = R"xyz(
    (function(){
var i = 0;
    switch (i) {
      case 0:
        return 5;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
    }
})();
)xyz";
  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS(code, bytecode));
  auto ret =
      evaluateSourceOrBytecode(std::make_unique<StringBuffer>(bytecode), "");
  ASSERT_EQ(ret.asNumber(), 5.0);
}

TEST(HermesRuntimePreparedJavaScriptTest, InvalidSourceThrows) {
  auto rt = makeHermesRuntime();
  const char *badSource = "this is definitely not valid javascript";
  EXPECT_THROW(
      rt->prepareJavaScript(std::make_unique<StringBuffer>(badSource), ""),
      facebook::jsi::JSIException)
      << "prepareJavaScript should have thrown an exception";
}

TEST_P(HermesRuntimeTest, NoCorruptionOnJSError) {
  // If the test crashes or infinite loops, the likely cause is that
  // Hermes API library is not built with proper compiler flags
  // (-fexception in GCC/CLANG, /EHsc in MSVC)
  try {
    rt->evaluateJavaScript(std::make_unique<StringBuffer>("foo.bar = 1"), "");
    FAIL() << "Expected JSIException";
  } catch (const facebook::jsi::JSIException &) {
    // expected exception, ignore
  }
  try {
    rt->evaluateJavaScript(std::make_unique<StringBuffer>("foo.baz = 1"), "");
    FAIL() << "Expected JSIException";
  } catch (const facebook::jsi::JSIException &) {
    // expected exception, ignore
  }
  rt->evaluateJavaScript(std::make_unique<StringBuffer>("gc()"), "");
}

// In JSC we use multiple threads in our implementation of JSI so we can't
// use the ASSERT_DEATH macros when testing that implementation.
// Asserts are compiled out of opt builds
#if !defined(NDEBUG) && defined(ASSERT_DEATH)
TEST(HermesRuntimeDeathTest, ValueTest) {
  auto eval = [](Runtime &rt, const char *code) {
    return rt.global().getPropertyAsFunction(rt, "eval").call(rt, code);
  };

  ASSERT_DEATH(
      eval(*makeHermesRuntime(), "'slay'").getNumber(), "Assertion.*isNumber");
  ASSERT_DEATH(
      {
        auto rt = makeHermesRuntime();
        eval(*rt, "123").getString(*rt);
      },
      "Assertion.*isString");
}
#endif

TEST(HermesRootsTest, DontGrowWhenMoveObjectOutOfValue) {
  auto rt = makeHermesRuntime();
  Value val = Object(*rt);
  // Keep the object alive during measurement.
  std::unique_ptr<Object> obj;
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(*rt, [&]() {
    obj = std::make_unique<Object>(std::move(val).getObject(*rt));
  });
  EXPECT_EQ(rootsDelta, 0);
}

TEST(HermesRootsTest, DontGrowWhenCloneObject) {
  auto rt = makeHermesRuntime();
  Value val = Object(*rt);
  constexpr int kCloneCount = 1000;
  // Keep the objects alive during measurement.
  std::vector<Object> objects;
  objects.reserve(kCloneCount);
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(*rt, [&]() {
    for (size_t i = 0; i < kCloneCount; i++) {
      objects.push_back(val.getObject(*rt));
    }
  });
  EXPECT_EQ(rootsDelta, 0);
}

TEST(HermesWatchTimeLimitTest, WatchTimeLimit) {
  // Some code that exercies the async break checks.
  const char *forABit = "var t = Date.now(); while (Date.now() < t + 100) {}";
  const char *forEver = "for (;;){}";
  uint32_t Around20MinsMS = 1234567;
  uint32_t ShortTimeoutMS = 123;
  {
    // Single runtime with ~20 minute limit that will not be reached.
    auto rt = makeHermesRuntime();
    rt->watchTimeLimit(Around20MinsMS);
    rt->evaluateJavaScript(std::make_unique<StringBuffer>(forABit), "");
  }
  {
    // Single runtime with timeout reset -- first a very long time out, then a
    // short one.
    auto rt = makeHermesRuntime();
    rt->watchTimeLimit(Around20MinsMS);
    rt->watchTimeLimit(ShortTimeoutMS);
    ASSERT_THROW(
        rt->evaluateJavaScript(std::make_unique<StringBuffer>(forEver), ""),
        JSIException);
  }
  {
    // Multiple runtimes, but neither will time out.
    auto rt1 = makeHermesRuntime();
    rt1->watchTimeLimit(Around20MinsMS);
    auto rt2 = makeHermesRuntime();
    rt2->watchTimeLimit(Around20MinsMS / 2);
    rt1->evaluateJavaScript(std::make_unique<StringBuffer>(forABit), "");
    rt2->evaluateJavaScript(std::make_unique<StringBuffer>(forABit), "");
  }
  {
    // Timeout in one of the runtimes. Make sure the first watchTimeLimit is the
    // "infinite one" as time TimeLimitMonitor could sleep for the given timeout
    // without properly handle the next timeouts.
    auto rt1 = makeHermesRuntime();
    rt1->watchTimeLimit(Around20MinsMS);
    auto rt2 = makeHermesRuntime();
    rt2->watchTimeLimit(ShortTimeoutMS);
    ASSERT_THROW(
        rt2->evaluateJavaScript(std::make_unique<StringBuffer>(forEver), ""),
        JSIException);
  }
  {
    auto timeLimitMonitor = hermes::vm::TimeLimitMonitor::getOrCreate();
    const auto &watchedRuntimes = timeLimitMonitor->getWatchedRuntimes();

    auto rt1 = makeHermesRuntime();
    rt1->watchTimeLimit(Around20MinsMS);
    auto rt2 = makeHermesRuntime();
    rt2->watchTimeLimit(Around20MinsMS);
    auto rt3 = makeHermesRuntime();
    rt3->watchTimeLimit(Around20MinsMS);
    EXPECT_EQ(watchedRuntimes.size(), 3);

    rt2 = nullptr;
    EXPECT_EQ(watchedRuntimes.size(), 2);
  }
}

TEST_P(HermesRuntimeTest, TriggerAsyncTimeout) {
  auto runTest = [](auto *rt) {
    // Some code that loops forever to exercise the async interrupt.
    const char *forEver = "for (;;){}";
    uint32_t ShortTimeoutMS = 123;
    {
      std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(ShortTimeoutMS));
        rt->asyncTriggerTimeout();
      });
      ASSERT_THROW(
          rt->evaluateJavaScript(std::make_unique<StringBuffer>(forEver), ""),
          JSIException);
      t.join();
    }
  };

  // Only these runtimes support asyncTriggerTimeout.
  if (auto *hrt = dynamic_cast<HermesRuntime *>(rt.get()))
    runTest(hrt);
  else if (auto *hsrt = dynamic_cast<HermesSandboxRuntime *>(rt.get()))
    runTest(hsrt);
}

TEST(HermesRuntimeCrashManagerTest, CrashGetStackTrace) {
  class CrashManagerImpl : public hermes::vm::CrashManager {
   public:
    void registerMemory(void *, size_t) override {}
    void unregisterMemory(void *) override {}
    void setCustomData(const char *, const char *) override {}
    void removeCustomData(const char *) override {}
    void setContextualCustomData(const char *, const char *) override {}
    void removeContextualCustomData(const char *) override {}
    CallbackKey registerCallback(CallbackFunc cb) override {
      auto key = callbacks.size();
      callbacks.push_back(std::move(cb));
      return key;
    }
    void unregisterCallback(CallbackKey /*key*/) override {}
    void setHeapInfo(const HeapInformation & /*heapInfo*/) override {}

    std::vector<CallbackFunc> callbacks;
  };

  auto cm = std::make_shared<CrashManagerImpl>();
  auto rt = makeHermesRuntime(
      hermes::vm::RuntimeConfig::Builder().withCrashMgr(cm).build());
  Function runCrashCallbacks = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "runCrashCallbacks"),
      0,
      [&](Runtime &rt, const Value &, const Value *, size_t) {
        // Create a temporary file to write the crash manager data to.
        FILE *f = tmpfile();
        for (const auto &cb : cm->callbacks)
          cb(fileno(f));
        const auto sz = ftell(f);
        rewind(f);
        // Dump the crash manager data to a std::string.
        std::string out;
        out.resize(sz);
        const auto readsz = fread(&out[0], 1, sz, f);
        EXPECT_EQ(readsz, sz);
        fclose(f);
        return String::createFromUtf8(rt, std::move(out));
      });
  rt->global().setProperty(
      *rt, "runCrashCallbacks", std::move(runCrashCallbacks));
  std::string jsCode = R"(
function baz(){ return runCrashCallbacks(); }
function bar(){ return baz(); }
function foo(){ return bar(); }
var out = foo();
// Extract just the source locations of the call stack.
JSON.stringify(JSON.parse(out).callstack.map(x => x.SourceLocation));
)";
  auto buf = std::make_shared<StringBuffer>(std::move(jsCode));
  std::string callstack =
      rt->evaluateJavaScript(buf, "crashCode.js").asString(*rt).utf8(*rt);
  const char *expected =
      "[\"crashCode.js:2:41\",\"crashCode.js:3:27\",\"crashCode.js:4:27\",\"crashCode.js:5:14\",null]";
  EXPECT_EQ(callstack, expected);
}

TEST_P(HermesRuntimeTest, SpreadHostObjectWithOwnProperties) {
  // TODO(T174477667): Understand why this test fails for the sandbox under
  // MSVC.
#ifdef _MSC_VER
  if (dynamic_cast<HermesSandboxRuntime *>(rt.get()))
    return;
#endif

  class HostObjectWithPropertyNames : public HostObject {
    std::vector<PropNameID> getPropertyNames(Runtime &rt) override {
      return PropNameID::names(rt, "prop1", "1", "2", "prop2", "3");
    }
    Value get(Runtime &runtime, const PropNameID &name) override {
      return Value();
    }
  };

  Object ho = Object::createFromHostObject(
      *rt, std::make_shared<HostObjectWithPropertyNames>());
  rt->global().setProperty(*rt, "ho", ho);

  auto res = eval(R"###(
var spreaded = {...ho};
var props = Object.getOwnPropertyNames(spreaded);
props.toString();
)###")
                 .getString(*rt)
                 .utf8(*rt);
  EXPECT_EQ(res, "1,2,3,prop1,prop2");
}

TEST_P(HermesRuntimeTest, HostObjectWithOwnProperties) {
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

// TODO mhorowitz: move this to jsi/testlib.cpp once we have impls for all VMs
TEST_P(HermesRuntimeTest, WeakReferences) {
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

TEST_P(HermesRuntimeTest, SourceURLAppearsInBacktraceTest) {
  std::string sourceURL = "//SourceURLAppearsInBacktraceTest/Test/URL";
  std::string sourceCode = R"(
function thrower() { throw new Error('Test Error Message')}
function throws1() { thrower(); }
throws1();
)";
  std::string bytecode;
  bool compiled = hermes::compileJS(sourceCode, sourceURL.c_str(), bytecode);
  ASSERT_TRUE(compiled) << "JS source should have compiled";

  for (const std::string &code : {sourceCode, bytecode}) {
    bool caught = false;
    try {
      evaluateSourceOrBytecode(std::make_unique<StringBuffer>(code), sourceURL);
    } catch (facebook::jsi::JSError &err) {
      caught = true;
      EXPECT_TRUE(err.getStack().find(sourceURL) != std::string::npos)
          << "Backtrace should contain source URL";
    }
    EXPECT_TRUE(caught) << "JS should have thrown an exception";
  }
}

TEST_P(HermesRuntimeTest, HostObjectAsParentTest) {
  class HostObjectWithProp : public HostObject {
    Value get(Runtime &runtime, const PropNameID &name) override {
      if (PropNameID::compare(
              runtime, name, PropNameID::forAscii(runtime, "prop1")))
        return 10;
      return Value();
    }
  };

  Object ho =
      Object::createFromHostObject(*rt, std::make_shared<HostObjectWithProp>());
  rt->global().setProperty(*rt, "ho", ho);

  EXPECT_TRUE(
      eval("var subClass = {__proto__: ho}; subClass.prop1 == 10;").getBool());
}

TEST_P(HermesRuntimeTest, NativeStateTest) {
  class C : public facebook::jsi::NativeState {
   public:
    int *dtors;
    C(int *_dtors) : dtors(_dtors) {}
    virtual ~C() override {
      ++*dtors;
    }
  };
  int dtors1 = 0;
  int dtors2 = 0;
  {
    Object obj = eval("({one: 1})").getObject(*rt);
    ASSERT_FALSE(obj.hasNativeState<C>(*rt));
    {
      // Set some state.
      obj.setNativeState(*rt, std::make_shared<C>(&dtors1));
      ASSERT_TRUE(obj.hasNativeState<C>(*rt));
      auto ptr = obj.getNativeState<C>(*rt);
      EXPECT_EQ(ptr->dtors, &dtors1);
    }
    {
      // Overwrite the state.
      obj.setNativeState(*rt, std::make_shared<C>(&dtors2));
      ASSERT_TRUE(obj.hasNativeState<C>(*rt));
      auto ptr = obj.getNativeState<C>(*rt);
      EXPECT_EQ(ptr->dtors, &dtors2);
    }
  } // closing scope -> obj unreachable
  // should finalize both
  eval("gc()");
  EXPECT_EQ(1, dtors1);
  EXPECT_EQ(1, dtors2);

  // Trying to set native state on frozen object should throw.
  {
    Object frozen = eval("Object.freeze({one: 1})").getObject(*rt);
    ASSERT_THROW(
        frozen.setNativeState(*rt, std::make_shared<C>(&dtors1)), JSIException);
  }
  // Make sure any NativeState cells are finalized before leaving, since they
  // point to local variables. Otherwise ASAN will complain.
  eval("gc()");
}

TEST_P(HermesRuntimeTest, ExternalMemoryTest) {
  // Keep track of the number of NativeState instances to make sure they are
  // being freed by the GC when there is memory pressure associated with the
  // object. This needs to be atomic because the destructor of NativeState may
  // be invoked on any thread.
  static std::atomic<size_t> numAllocs = 0;

  class Counter : public HostObject, public NativeState {
   public:
    Counter() {
      // Check that we haven't accumulated too many CountNativeStates. MallocGC
      // does not deal with external memory correctly so it is excluded.

#ifndef HERMESVM_GC_MALLOC
      EXPECT_LT(numAllocs++, 50);
#endif
    }
    ~Counter() {
      numAllocs--;
    }
  };

  for (size_t i = 0; i < 200; i++) {
    auto o = Object::createFromHostObject(*rt, std::make_shared<Counter>());
    o.setExternalMemoryPressure(*rt, 1024 * 1024);
  }

  // Test that we can adjust memory pressure even on a frozen object.
  auto freeze = eval("Object.freeze").getObject(*rt).getFunction(*rt);
  for (size_t i = 0; i < 200; i++) {
    Object o{*rt};
    o.setNativeState(*rt, std::make_shared<Counter>());
    freeze.call(*rt, o);
    o.setExternalMemoryPressure(*rt, 1024 * 1024);
  }

  // Try setting a series of values on the same object.
  Object o{*rt};
  o.setExternalMemoryPressure(*rt, 5);
  o.setExternalMemoryPressure(*rt, 5);
  o.setExternalMemoryPressure(*rt, 0);
  o.setExternalMemoryPressure(*rt, 1024 * 1024);
}

TEST_P(HermesRuntimeTest, PropNameIDFromSymbol) {
  auto strProp = PropNameID::forAscii(*rt, "a");
  auto secretProp = PropNameID::forSymbol(
      *rt, eval("var secret = Symbol('a'); secret;").getSymbol(*rt));
  auto globalProp =
      PropNameID::forSymbol(*rt, eval("Symbol.for('a');").getSymbol(*rt));
  auto x =
      eval("({a : 'str', [secret] : 'secret', [Symbol.for('a')] : 'global'});")
          .getObject(*rt);

  EXPECT_EQ(x.getProperty(*rt, strProp).getString(*rt).utf8(*rt), "str");
  EXPECT_EQ(x.getProperty(*rt, secretProp).getString(*rt).utf8(*rt), "secret");
  EXPECT_EQ(x.getProperty(*rt, globalProp).getString(*rt).utf8(*rt), "global");
}

TEST_P(HermesRuntimeTest, HostObjectSymbolTest) {
  class DummyHO : public HostObject {
   public:
    Value get(Runtime &, const PropNameID &name) {
      return Value();
    }
    void set(Runtime &rt, const PropNameID &name, const Value &value) {
      props.emplace_back(rt, name);
    }
    std::vector<PropNameID> getPropertyNames(Runtime &rt) {
      std::vector<PropNameID> propsRet;
      for (const auto &prop : props)
        propsRet.emplace_back(rt, prop);
      return propsRet;
    }

    std::vector<PropNameID> props;
  };
  auto hoObj = Object::createFromHostObject(*rt, std::make_shared<DummyHO>());
  rt->global().setProperty(*rt, "ho", hoObj);
  eval(R"(
var abcSym = Symbol("abc");
ho[abcSym] = 1;
var defSym = Symbol("def");
ho[defSym] = 2;
// Add a number symbol to test that we don't treat it as integer index.
var numberSym = Symbol("5");
ho[numberSym] = 55;
ho.xyz = 5;
ho.qwerty = 6;
// Create some duplicate properties to test deduplication.
Object.defineProperty(ho, "xyz", {
  value: 42,
});
Object.defineProperty(ho, "foo", {
  value: 42,
});
Object.defineProperty(ho, abcSym, {
  value: 78,
});
var hoDefSym = Symbol("def");
Object.defineProperty(ho, hoDefSym, {
  value: 26,
});

let assert = function(cond, msg) {
  if (!cond) {
    throw new Error(msg);
  }
};

// Ensure that the values are the same, and have the same order.
let arrayEqual = function(arr1, arr2) {
  if (arr1.length !== arr2.length) {
    return false;
  }
  for (var i = 0; i < arr1.length; i++) {
    if (arr1[i] !== arr2[i]) {
      return false;
    }
  }
  return true;
};

let strArr = Object.getOwnPropertyNames(ho);
// Hidden class properties should come before HO properties.
assert(arrayEqual(strArr, ["xyz", "foo", "qwerty"]),
  "getOwnPropertyNames() returns incorrectly.");
let symArr = Object.getOwnPropertySymbols(ho);
assert(arrayEqual(symArr, [abcSym, hoDefSym, defSym, numberSym]),
  "getOwnPropertySymbols() returns incorrectly.");
)");
}

TEST_P(HermesRuntimeTest, ArrayTest) {
  auto array = eval("[1, 2, 3]").getObject(*rt);
  EXPECT_TRUE(array.isArray(*rt));
  auto jsiArray = array.getArray(*rt);
  EXPECT_EQ(jsiArray.size(*rt), 3);
  EXPECT_EQ(jsiArray.getValueAtIndex(*rt, 0).asNumber(), 1);
  jsiArray.setValueAtIndex(*rt, 1, 0);
  EXPECT_EQ(jsiArray.getValueAtIndex(*rt, 1).asNumber(), 0);

  array = eval("new Proxy([4, 5, 6], {})").getObject(*rt);
  EXPECT_TRUE(array.isArray(*rt));
  jsiArray = array.getArray(*rt);
  EXPECT_EQ(jsiArray.size(*rt), 3);
  EXPECT_EQ(jsiArray.getValueAtIndex(*rt, 0).asNumber(), 4);
  jsiArray.setValueAtIndex(*rt, 1, 0);
  EXPECT_EQ(jsiArray.getValueAtIndex(*rt, 1).asNumber(), 0);
}

TEST_P(HermesRuntimeTest, HasComputedTest) {
  // The only use of JSObject::hasComputed() is in HermesRuntimeImpl,
  // so we test its Proxy support here, instead of from JS.

  EXPECT_FALSE(eval("'prop' in new Proxy({}, {})").getBool());
  EXPECT_TRUE(eval("'prop' in new Proxy({prop:1}, {})").getBool());
  EXPECT_FALSE(
      eval("'prop' in new Proxy({}, {has() { return false; }})").getBool());
  EXPECT_TRUE(
      eval("'prop' in new Proxy({}, {has() { return true; }})").getBool());

  // While we're here, test that a HostFunction can be used as a proxy
  // trap.  This could be very powerful in the right hands.
  Function returnTrue = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "returnTrue"),
      0,
      [](Runtime &rt, const Value &, const Value *args, size_t count) {
        EXPECT_EQ(count, 2);
        EXPECT_EQ(args[1].toString(rt).utf8(rt), "prop");
        return true;
      });
  rt->global().setProperty(*rt, "returnTrue", returnTrue);
  EXPECT_TRUE(eval("'prop' in new Proxy({}, {has: returnTrue})").getBool());
}

TEST_P(HermesRuntimeTest, GlobalObjectTest) {
  rt->global().setProperty(*rt, "a", 5);
  eval("f = function(b) { return a + b; }");
  eval("gc()");
  EXPECT_EQ(eval("f(10)").getNumber(), 15);
}

class HermesRuntimeTestWithDisableGenerator
    : public HermesRuntimeCustomConfigTest {
 public:
  HermesRuntimeTestWithDisableGenerator()
      : HermesRuntimeCustomConfigTest(::hermes::vm::RuntimeConfig::Builder()
                                          .withEnableGenerator(false)
                                          .build()) {}
};

TEST_F(HermesRuntimeTestWithDisableGenerator, WithDisableGenerator) {
  EXPECT_THROW(
      rt->evaluateJavaScript(
          std::make_unique<StringBuffer>("function* foo() {}"), ""),
      facebook::jsi::JSIException)
      << "Expected JSIException";

  EXPECT_THROW(
      rt->evaluateJavaScript(
          std::make_unique<StringBuffer>("obj = {*foo() {}}"), ""),
      facebook::jsi::JSIException)
      << "Expected JSIException";

  // async function depends on generator.
  EXPECT_THROW(
      rt->evaluateJavaScript(
          std::make_unique<StringBuffer>("async function foo() {}"), ""),
      facebook::jsi::JSIException)
      << "Expected JSIException";
}

TEST_P(HermesRuntimeTest, DiagnosticHandlerTestError) {
  using DiagnosticHandler = hermes::DiagnosticHandler;

  struct BufferingDiagnosticHandler : DiagnosticHandler {
    void handle(const DiagnosticHandler::Diagnostic &d) {
      ds.push_back(d);
    }
    std::vector<DiagnosticHandler::Diagnostic> ds;
  } diagHandler;
  std::string bytecode;
  ASSERT_FALSE(
      hermes::compileJS("x++1", "", bytecode, true, true, &diagHandler));
  ASSERT_EQ(1, diagHandler.ds.size());
  EXPECT_EQ(DiagnosticHandler::Error, diagHandler.ds[0].kind);
  EXPECT_EQ(1, diagHandler.ds[0].line);
  EXPECT_EQ(4, diagHandler.ds[0].column);
}

TEST_P(HermesRuntimeTest, DiagnosticHandlerTestWarning) {
  using DiagnosticHandler = hermes::DiagnosticHandler;

  struct BufferingDiagnosticHandler : DiagnosticHandler {
    void handle(const DiagnosticHandler::Diagnostic &d) {
      ds.push_back(d);
    }
    std::vector<DiagnosticHandler::Diagnostic> ds;
  } diagHandler;
  std::string bytecode;
  // Succeeds with a warning + associated note.
  ASSERT_TRUE(
      hermes::compileJS("({a:1,a:2})", "", bytecode, true, true, &diagHandler));
  ASSERT_EQ(2, diagHandler.ds.size());

  // warning: the property "a" was set multiple times in the object definition.
  EXPECT_EQ(DiagnosticHandler::Warning, diagHandler.ds[0].kind);
  EXPECT_EQ(1, diagHandler.ds[0].line);
  EXPECT_EQ(7, diagHandler.ds[0].column);
  ASSERT_EQ(1, diagHandler.ds[0].ranges.size());
  EXPECT_EQ(6, diagHandler.ds[0].ranges[0].first);
  EXPECT_EQ(9, diagHandler.ds[0].ranges[0].second);

  // The first definition was here.
  EXPECT_EQ(DiagnosticHandler::Note, diagHandler.ds[1].kind);
  EXPECT_EQ(1, diagHandler.ds[1].line);
  EXPECT_EQ(3, diagHandler.ds[1].column);
  ASSERT_EQ(1, diagHandler.ds[1].ranges.size());
  EXPECT_EQ(2, diagHandler.ds[1].ranges[0].first);
  EXPECT_EQ(5, diagHandler.ds[1].ranges[0].second);
}

TEST_P(HermesRuntimeTest, BigIntJSI) {
  Function bigintCtor = rt->global().getPropertyAsFunction(*rt, "BigInt");
  auto BigInt = [&](const char *v) { return bigintCtor.call(*rt, eval(v)); };

  auto v0 = BigInt("0");
  auto b0 = v0.asBigInt(*rt);
  EXPECT_EQ(v0.toString(*rt).utf8(*rt), "0");
  EXPECT_EQ(b0.toString(*rt).utf8(*rt), "0");

  auto vffffffffffffffff = BigInt("0xffffffffffffffffn");
  auto bffffffffffffffff = vffffffffffffffff.asBigInt(*rt);
  EXPECT_EQ(vffffffffffffffff.toString(*rt).utf8(*rt), "18446744073709551615");
  EXPECT_EQ(bffffffffffffffff.toString(*rt, 16).utf8(*rt), "ffffffffffffffff");
  EXPECT_EQ(bffffffffffffffff.toString(*rt, 36).utf8(*rt), "3w5e11264sgsf");

  auto vNeg1 = BigInt("-1");
  auto bNeg1 = vNeg1.asBigInt(*rt);
  EXPECT_EQ(vNeg1.toString(*rt).utf8(*rt), "-1");
  EXPECT_EQ(bNeg1.toString(*rt, 16).utf8(*rt), "-1");
  EXPECT_EQ(bNeg1.toString(*rt, 36).utf8(*rt), "-1");

  EXPECT_TRUE(BigInt::strictEquals(*rt, b0, b0));
  EXPECT_TRUE(BigInt::strictEquals(*rt, bffffffffffffffff, bffffffffffffffff));
  EXPECT_FALSE(BigInt::strictEquals(*rt, bNeg1, bffffffffffffffff));
}

TEST_P(HermesRuntimeTest, BigIntJSIFromScalar) {
  Function bigintCtor = rt->global().getPropertyAsFunction(*rt, "BigInt");
  auto BigInt = [&](const char *v) {
    return bigintCtor.call(*rt, eval(v)).asBigInt(*rt);
  };

  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt("0"), BigInt::fromUint64(*rt, 0)));
  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt("0"), BigInt::fromInt64(*rt, 0)));
  EXPECT_TRUE(BigInt::strictEquals(
      *rt, BigInt("0xdeadbeef"), BigInt::fromUint64(*rt, 0xdeadbeef)));
  EXPECT_TRUE(BigInt::strictEquals(
      *rt, BigInt("0xc0ffee"), BigInt::fromInt64(*rt, 0xc0ffee)));
  EXPECT_TRUE(BigInt::strictEquals(
      *rt, BigInt("0xffffffffffffffffn"), BigInt::fromUint64(*rt, ~0ull)));
  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt("-1"), BigInt::fromInt64(*rt, ~0ull)));
}

TEST_P(HermesRuntimeTest, BigIntJSIToString) {
  auto b = BigInt::fromUint64(*rt, 1);
  // Test all possible radixes.
  for (int radix = 2; radix <= 36; ++radix) {
    EXPECT_EQ(b.toString(*rt, radix).utf8(*rt), "1") << radix;
  }

  // Test some invaild radixes.
  EXPECT_THROW(b.toString(*rt, -1), JSIException);
  EXPECT_THROW(b.toString(*rt, 0), JSIException);
  EXPECT_THROW(b.toString(*rt, 1), JSIException);
  EXPECT_THROW(b.toString(*rt, 37), JSIException);
  EXPECT_THROW(b.toString(*rt, 100), JSIException);

  Function bigintCtor = rt->global().getPropertyAsFunction(*rt, "BigInt");
  auto BigInt = [&](int value) {
    return bigintCtor.call(*rt, value).asBigInt(*rt);
  };

  // Now test that the radix is being passed to the VM.
  for (int radix = 2; radix <= 36; ++radix) {
    EXPECT_EQ(BigInt(radix + 1).toString(*rt, radix).utf8(*rt), "11") << radix;
    EXPECT_EQ(BigInt(-(radix + 1)).toString(*rt, radix).utf8(*rt), "-11")
        << radix;
  }
}

TEST_P(HermesRuntimeTest, BigIntJSITruncation) {
  auto lossless = [](uint64_t value) { return std::make_tuple(value, true); };
  auto lossy = [](uint64_t value) { return std::make_tuple(value, false); };

  auto toInt64 = [this](const BigInt &b) {
    return std::make_tuple(b.getInt64(*rt), b.isInt64(*rt));
  };

  auto toUint64 = [this](const BigInt &b) {
    return std::make_tuple(b.getUint64(*rt), b.isUint64(*rt));
  };

  Function bigintCtor = rt->global().getPropertyAsFunction(*rt, "BigInt");
  auto BigInt = [&](const char *v) {
    return bigintCtor.call(*rt, eval(v)).asBigInt(*rt);
  };

  // 0n can be truncated losslessly to either int64_t and uint64_t
  auto b = BigInt::fromUint64(*rt, 0);
  EXPECT_EQ(toUint64(b), lossless(0));
  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt::fromUint64(*rt, b.getUint64(*rt)), b));
  EXPECT_EQ(toInt64(b), lossless(0));
  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt::fromInt64(*rt, b.getInt64(*rt)), b));

  // Creating BigInt from an ~0ull. This value can't be truncated losslessly to
  // int64_t.
  b = BigInt::fromUint64(*rt, ~0ull);
  EXPECT_EQ(toUint64(b), lossless(~0ull));
  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt::fromUint64(*rt, b.getUint64(*rt)), b));
  EXPECT_EQ(toInt64(b), lossy(~0ull));

  // Creating BigInt from an -1ull. This value can't be truncated losslessly to
  // int64_t.
  b = BigInt::fromInt64(*rt, -1ull);
  EXPECT_EQ(toUint64(b), lossy(-1ull));
  EXPECT_EQ(toInt64(b), lossless(-1ull));
  EXPECT_TRUE(
      BigInt::strictEquals(*rt, BigInt::fromInt64(*rt, b.getInt64(*rt)), b));

  // 0x10000000000000000n can't be truncated to int64_t nor uint64_t.
  b = BigInt("0x10000000000000000n");
  EXPECT_EQ(toUint64(b), lossy(0));
  EXPECT_EQ(toInt64(b), lossy(0));

  // -0x10000000000000000n can't be truncated to int64_t nor uint64_t.
  b = BigInt("-0x10000000000000000n");
  EXPECT_EQ(toUint64(b), lossy(0));
  EXPECT_EQ(toInt64(b), lossy(0));

  // (1n << 65n) - 1n can't be truncated to int64_t nor uint64_t.
  b = BigInt("(1n << 65n) - 1n");
  EXPECT_EQ(toUint64(b), lossy(~0ull));
  EXPECT_EQ(toInt64(b), lossy(~0ull));
}

#ifdef HERMESVM_EXCEPTION_ON_OOM
class HermesRuntimeTestSmallHeap : public HermesRuntimeCustomConfigTest {
 public:
  HermesRuntimeTestSmallHeap()
      : HermesRuntimeCustomConfigTest(
            ::hermes::vm::RuntimeConfig::Builder()
                .withGCConfig(::hermes::vm::GCConfig::Builder()
                                  .withInitHeapSize(8 << 20)
                                  .withMaxHeapSize(8 << 20)
                                  .build())
                .build()) {}
};

TEST_F(HermesRuntimeTestSmallHeap, HostFunctionPropagatesOOMExceptionTest) {
  auto func = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, ""),
      1,
      [](Runtime &rt, const Value &, const Value *args, unsigned long count) {
        assert(count > 0);
        auto func = args[0].asObject(rt).asFunction(rt);
        return func.call(rt, args + 1, count - 1);
      });

  auto makeOOM = eval(R"#(
(function (){
  var outer = [];
  while(true){
    var inner = [];
    for (var i = 0; i < 10000; i++) inner.push({});
    outer.push(inner);
  }
})
)#");
  EXPECT_THROW(func.call(*rt, makeOOM), ::hermes::vm::JSOutOfMemoryError);
}

TEST_F(HermesRuntimeTestSmallHeap, CreateJSErrorPropagatesOOMExceptionTest) {
  eval(R"#(
globalThis.Error = function (){
  var outer = [];
  while(true){
    var inner = [];
    for (var i = 0; i < 10000; i++) inner.push({});
    outer.push(inner);
  }
};
)#");
  EXPECT_THROW(throw JSError(*rt, "Foo"), ::hermes::vm::JSOutOfMemoryError);
}
#endif

TEST_P(HermesRuntimeTest, NativeExceptionDoesNotUseGlobalError) {
  Function alwaysThrows = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "alwaysThrows"),
      0,
      [](Runtime &, const Value &, const Value *, size_t) -> Value {
        throw std::logic_error(
            "Native std::logic_error C++ exception in Host Function");
      });
  rt->global().setProperty(*rt, "alwaysThrows", alwaysThrows);
  rt->global().setProperty(*rt, "Error", 10);

  auto test = eval(
                  R"#((function(val) {
                          'use strict';
                          try {
                            alwaysThrows(val);
                          } catch(e) {
                            return 'typeof Error is ' + typeof(Error) + '; ' + e.message;
                          }
                          throw new Error('Unreachable statement');
                       }))#")
                  .getObject(*rt)
                  .getFunction(*rt);
  EXPECT_EQ(
      "typeof Error is number; Exception in HostFunction: Native "
      "std::logic_error C++ exception in Host Function",
      test.call(*rt).getString(*rt).utf8(*rt));
}

TEST_P(HermesRuntimeTest, UTF16ConversionTest) {
  String ascii = String::createFromUtf8(*rt, "z");
  EXPECT_EQ(ascii.utf16(*rt), u"z");

  String foobar = String::createFromUtf8(*rt, "foobar");
  EXPECT_EQ(foobar.utf16(*rt), u"foobar");

  // 你 in UTF-8 encoding is 0xe4 0xbd 0xa0 and 好 is 0xe5 0xa5 0xbd
  // 你 in UTF-16 encoding is 0x4f60 and 好 is 0x597d
  String chineseHello = String::createFromUtf8(*rt, "\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_EQ(chineseHello.utf16(*rt), u"\x4f60\x597d");

  // 👍 in UTF-8 encoding is 0xf0 0x9f 0x91 0x8d
  // 👍 in UTF-16 encoding is 0xd83d 0xdc4d
  String thumbsUpEmoji = String::createFromUtf8(*rt, "\xf0\x9f\x91\x8d");
  EXPECT_EQ(thumbsUpEmoji.utf16(*rt), u"\xd83d\xdc4d");

  // String is foobar👍你好
  String combined = String::createFromUtf8(
      *rt, "foobar\xf0\x9f\x91\x8d\xe4\xbd\xa0\xe5\xa5\xbd");
  EXPECT_EQ(combined.utf16(*rt), u"foobar\xd83d\xdc4d\x4f60\x597d");

  // Thumbs up emoji is encoded as 0xd83d 0xdc4d. These test UTF16 with lone
  // high and low surrogates.
  String loneHighSurrogate = eval("'\\ud83d'").getString(*rt);
  EXPECT_EQ(loneHighSurrogate.utf16(*rt), std::u16string(u"\xd83d"));

  String loneLowSurrogate = eval("'\\udc4d'").getString(*rt);
  EXPECT_EQ(loneLowSurrogate.utf16(*rt), std::u16string(u"\xdc4d"));
}

TEST_P(HermesRuntimeTest, CreateFromUtf16Test) {
  std::u16string utf16 = u"foobar";

  auto jsString = String::createFromUtf16(*rt, utf16);
  EXPECT_EQ(jsString.utf16(*rt), utf16);
  auto prop = PropNameID::forUtf16(*rt, utf16);
  EXPECT_EQ(prop.utf16(*rt), utf16);

  // 👋 in UTF-16 encoding is 0xd83d 0xdc4b
  utf16 = u"hello!\xd83d\xdc4b";
  jsString = String::createFromUtf16(*rt, utf16.data(), utf16.length());
  EXPECT_EQ(jsString.utf16(*rt), utf16);
  prop = PropNameID::forUtf16(*rt, utf16);
  EXPECT_EQ(prop.utf16(*rt), utf16);

  // Thumbs up emoji is encoded as 0xd83d 0xdc4d. The following tests String
  // creation with a lone surrogate.
  utf16 = u"\xd83d";
  jsString = String::createFromUtf16(*rt, utf16.data(), utf16.length());
  EXPECT_EQ(jsString.utf16(*rt), utf16);
  prop = PropNameID::forUtf16(*rt, utf16);
  EXPECT_EQ(prop.utf16(*rt), utf16);
}

TEST_P(HermesRuntimeTest, GetStringDataTest) {
  std::u16string buf;
  auto cb = [&buf](bool ascii, const void *data, size_t num) {
    // this callback copies the string content, but removes every 'o' character
    if (ascii) {
      const char *begin = (const char *)data;
      const char *end = (const char *)data + num;
      while (begin < end) {
        char curr = begin[0];
        if (curr != 'o') {
          buf.push_back((char16_t)curr);
        }
        begin++;
      }
    } else {
      const char16_t *begin = (const char16_t *)data;
      const char16_t *end = (const char16_t *)data + num;
      while (begin < end) {
        char16_t curr = begin[0];
        if (curr != 'o') {
          buf.push_back(curr);
        }
        begin++;
      }
    }
  };

  String asciiString = String::createFromUtf8(*rt, "foobar");
  asciiString.getStringData(*rt, cb);
  EXPECT_EQ(buf, u"fbar");
  buf.clear();

  // String is foobar👍你好
  String utf16Str = String::createFromUtf8(
      *rt, "foobar\xf0\x9f\x91\x8d\xe4\xbd\xa0\xe5\xa5\xbd");
  utf16Str.getStringData(*rt, cb);
  EXPECT_EQ(buf, u"fbar\xd83d\xdc4d\x4f60\x597d");
  buf.clear();
}

TEST_P(HermesRuntimeTest, GetPropNameIdDataTest) {
  std::u16string buf;
  auto cb = [&buf](bool ascii, const void *data, size_t num) {
    // this callback copies the string content, but removes every 'o' character
    if (ascii) {
      const char *begin = (const char *)data;
      const char *end = (const char *)data + num;
      while (begin < end) {
        char curr = begin[0];
        if (curr != 'o') {
          buf.push_back((char16_t)curr);
        }
        begin++;
      }
    } else {
      const char16_t *begin = (const char16_t *)data;
      const char16_t *end = (const char16_t *)data + num;
      while (begin < end) {
        char16_t curr = begin[0];
        if (curr != 'o') {
          buf.push_back(curr);
        }
        begin++;
      }
    }
  };

  PropNameID ascii = PropNameID::forAscii(*rt, "foobar");
  ascii.getPropNameIdData(*rt, cb);
  EXPECT_EQ(buf, u"fbar");
  buf.clear();

  // String is foobar👍你好
  PropNameID utf16 = PropNameID::forUtf8(
      *rt, "foobar\xf0\x9f\x91\x8d\xe4\xbd\xa0\xe5\xa5\xbd");
  utf16.getPropNameIdData(*rt, cb);
  EXPECT_EQ(buf, u"fbar\xd83d\xdc4d\x4f60\x597d");
  buf.clear();
}

TEST_P(HermesRuntimeTest, SetPrototypeOf) {
  Object prototypeObj(*rt);
  prototypeObj.setProperty(*rt, "someProperty", 123);
  Value prototype(*rt, prototypeObj);

  Object child(*rt);
  child.setPrototype(*rt, prototype);
  EXPECT_EQ(child.getProperty(*rt, "someProperty").getNumber(), 123);

  auto getPrototypeRes = child.getPrototype(*rt).asObject(*rt);
  EXPECT_EQ(getPrototypeRes.getProperty(*rt, "someProperty").getNumber(), 123);

  // Tests null value as prototype
  child.setPrototype(*rt, Value::null());
  EXPECT_TRUE(child.getPrototype(*rt).isNull());

  // Throw when prototype is neither an Object nor null
  EXPECT_THROW(child.setPrototype(*rt, Value(1)), JSError);
}

TEST_P(HermesRuntimeTest, CreateObjectWithPrototype) {
  Object prototypeObj(*rt);
  prototypeObj.setProperty(*rt, "someProperty", 123);
  Value prototype(*rt, prototypeObj);

  Object child = Object::create(*rt, prototype);
  EXPECT_EQ(child.getProperty(*rt, "someProperty").getNumber(), 123);

  // Tests null value as prototype
  child = Object::create(*rt, Value::null());
  EXPECT_TRUE(child.getPrototype(*rt).isNull());

  // Throw when prototype is neither an Object nor null
  EXPECT_THROW(Object::create(*rt, Value(1)), JSError);
}

INSTANTIATE_TEST_CASE_P(
    Runtimes,
    HermesRuntimeTest,
    ::testing::ValuesIn(runtimeGenerators()));

} // namespace
