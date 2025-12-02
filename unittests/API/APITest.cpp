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
  static size_t rootsListLength(const IHermesTestHelpers &rt) {
    return rt.rootsListLengthForTests();
  }

  static int64_t calculateRootsListChange(
      const IHermesTestHelpers &rt,
      std::function<void(void)> f) {
    auto before = rootsListLength(rt);
    f();
    return rootsListLength(rt) - before;
  }
};

namespace {

class HermesRuntimeTestBase {
 public:
  HermesRuntimeTestBase(std::shared_ptr<Runtime> rt) : rt(std::move(rt)) {}

 protected:
  Value eval(const char *code) {
    return rt->global().getPropertyAsFunction(*rt, "eval").call(*rt, code);
  }

  std::shared_ptr<Runtime> rt;
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
  EXPECT_TRUE(eval(
                  "(function() {"
                  "  \"use strict\";"
                  "  return coolify.bind(undefined)();"
                  "})()")
                  .getBool());
}

TEST_P(HermesRuntimeTest, ResetTimezoneCache) {
  if (auto *hrt = castInterface<IHermes>(rt.get())) {
    EXPECT_NO_THROW({ hrt->resetTimezoneCache(); });
  }
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
    EXPECT_EQ(weakBuf.use_count(), 1);
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
  auto *api = castInterface<IHermesRootAPI>(makeHermesRootAPI());
  const uint8_t shortBytes[] = {1, 2, 3};
  EXPECT_FALSE(api->isHermesBytecode(shortBytes, 0));
  EXPECT_FALSE(api->isHermesBytecode(shortBytes, sizeof(shortBytes)));
  uint8_t longBytes[1024];
  memset(longBytes, 'H', sizeof(longBytes));
  EXPECT_FALSE(api->isHermesBytecode(longBytes, sizeof(longBytes)));

  std::string bytecode;
  ASSERT_TRUE(hermes::compileJS("x = 1", bytecode));
  EXPECT_TRUE(api->isHermesBytecode(
      reinterpret_cast<const uint8_t *>(bytecode.data()), bytecode.size()));
  evaluateSourceOrBytecode(
      std::unique_ptr<StringBuffer>(new StringBuffer(bytecode)), "");
  EXPECT_EQ(rt->global().getProperty(*rt, "x").getNumber(), 1);

  EXPECT_EQ(api->getBytecodeVersion(), hermes::hbc::BYTECODE_VERSION);
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

  auto *api = castInterface<IHermesRootAPI>(makeHermesRootAPI());
  std::string bytecode;
  ASSERT_TRUE(
      hermes::compileJS(
          TestSource,
          "script.js",
          bytecode,
          true,
          true,
          nullptr,
          std::optional<std::string_view>(TestSourceMap)));
  EXPECT_TRUE(api->isHermesBytecode(
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
  std::shared_ptr<HermesRuntime> rt = makeHermesRuntime();
  Value val = Object(*rt);
  // Keep the object alive during measurement.
  std::unique_ptr<Object> obj;
  auto helperRt = dynamicInterfaceCast<IHermesTestHelpers>(rt);
  auto rootsDelta = HermesTestHelper::calculateRootsListChange(
      *helperRt,
      [&]() { obj = std::make_unique<Object>(std::move(val).getObject(*rt)); });
  EXPECT_EQ(rootsDelta, 0);
}

TEST(HermesRootsTest, DontGrowWhenCloneObject) {
  std::shared_ptr<HermesRuntime> rt = makeHermesRuntime();
  Value val = Object(*rt);
  constexpr int kCloneCount = 1000;
  // Keep the objects alive during measurement.
  std::vector<Object> objects;
  objects.reserve(kCloneCount);
  auto helperRt = dynamicInterfaceCast<IHermesTestHelpers>(rt);
  auto rootsDelta =
      HermesTestHelper::calculateRootsListChange(*helperRt, [&]() {
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

#ifdef HERMESVM_GC_HADES
TEST_P(HermesRuntimeTest, GetHeapInfo) {
  auto &instrumentation = rt->instrumentation();
  // Make sure we do run some collections.
  instrumentation.collectGarbage("test");
  auto heapInfo = instrumentation.getHeapInfo(false);
  // Let's not assert the exact number of collections, which could be changed
  // in concrete Hades implementation.
  EXPECT_NE(heapInfo["hermes_full_numCollections"], 0);
  EXPECT_NE(heapInfo["hermes_yg_numCollections"], 0);
}
#endif

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

  EXPECT_TRUE(eval(
                  "var properties = Object.getOwnPropertyNames(ho);"
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

  EXPECT_TRUE(eval(
                  "var properties = Object.getOwnPropertyNames(ho);"
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
  EXPECT_TRUE(eval(
                  "var d = Object.getOwnPropertyDescriptor(ho, 'prop1');"
                  "d != undefined && "
                  "d.value == 10 && "
                  "d.enumerable && "
                  "d.writable ")
                  .getBool());
  EXPECT_TRUE(eval(
                  "var d = Object.getOwnPropertyDescriptor(ho, 'any-string');"
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
      : HermesRuntimeCustomConfigTest(
            ::hermes::vm::RuntimeConfig::Builder()
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
  EXPECT_TRUE(
      BigInt::strictEquals(
          *rt, BigInt("0xdeadbeef"), BigInt::fromUint64(*rt, 0xdeadbeef)));
  EXPECT_TRUE(
      BigInt::strictEquals(
          *rt, BigInt("0xc0ffee"), BigInt::fromInt64(*rt, 0xc0ffee)));
  EXPECT_TRUE(
      BigInt::strictEquals(
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
                .withGCConfig(
                    ::hermes::vm::GCConfig::Builder()
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

TEST_F(HermesRuntimeTestSmallHeap, InterpreterUnwindOOM) {
  // Test that the interpreter does not attempt to restore the IP in the runtime
  // during exception unwinding. This would be a unnecessary, and causes
  // assertion failures because the register stack is not unwound.
  // It is important in order to reliably test this that the function where the
  // OOM occurs does not make any calls, or contain any numbers in registers, so
  // that we reliably get a crash if the interpreter tries to access the SavedIP
  // slot during unwinding.
  const char *src = R"#(
function foo(){
  let obj = {};
  let i = "a";
  while(true) obj[ i += "a" ] = ["a", "b", "c", "d", "e", "f"];
}
foo();
)#";
  EXPECT_THROW(eval(src), ::hermes::vm::JSOutOfMemoryError);
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

TEST_P(HermesRuntimeTest, SetRuntimeData) {
  UUID uuid1{0xe67ab3d6, 0x09a0, 0x11f0, 0xa641, 0x325096b39f47};
  auto str = std::make_shared<std::string>("hello world");
  rt->setRuntimeData(uuid1, str);

  UUID uuid2{0xa12f99fc, 0x09a2, 0x11f0, 0x84de, 0x325096b39f47};
  auto obj1 = std::make_shared<Object>(*rt);
  rt->setRuntimeData(uuid2, obj1);

  auto storedStr =
      std::static_pointer_cast<std::string>(rt->getRuntimeData(uuid1));
  auto storedObj = std::static_pointer_cast<Object>(rt->getRuntimeData(uuid2));
  EXPECT_EQ(storedStr, str);
  EXPECT_EQ(storedObj, obj1);

  // Override the existing value at uuid1
  auto weakOldStr = std::weak_ptr<std::string>(str);
  str = std::make_shared<std::string>("goodbye world");
  rt->setRuntimeData(uuid1, str);
  storedStr = std::static_pointer_cast<std::string>(rt->getRuntimeData(uuid1));
  EXPECT_EQ(str, storedStr);
  // Verify that the old data was not held on after it was deleted
  EXPECT_EQ(weakOldStr.use_count(), 0);

  auto rt2 = makeHermesRuntime();
  UUID uuid3{0x16f55892, 0x1034, 0x11f0, 0x8f65, 0x325096b39f47};
  auto obj2 = std::make_shared<Object>(*rt2);
  rt2->setRuntimeData(uuid3, obj2);

  auto storedObj2 =
      std::static_pointer_cast<Object>(rt2->getRuntimeData(uuid3));
  EXPECT_EQ(storedObj2, obj2);

  // UUID 1 is for data in the first hermes runtime, so we expect nullptr here
  EXPECT_FALSE(rt2->getRuntimeData(uuid1));

  // Verify that when the runtime gets destroyed, the custom data is also
  // released.
  auto weakObj2 = std::weak_ptr<Object>(obj2);
  obj2.reset();
  storedObj2.reset();
  rt2.reset();
  EXPECT_EQ(weakObj2.use_count(), 0);
}

TEST_P(HermesRuntimeTest, DeleteProperty) {
  eval("var obj = {1:2, foo: 'bar', 3:4, salt: 'pepper'}");
  auto obj = rt->global().getPropertyAsObject(*rt, "obj");

  auto prop = PropNameID::forAscii(*rt, "1");
  obj.deleteProperty(*rt, prop);
  auto hasRes = obj.hasProperty(*rt, prop);
  EXPECT_FALSE(hasRes);

  auto str = String::createFromAscii(*rt, "foo");
  obj.deleteProperty(*rt, str);
  hasRes = obj.hasProperty(*rt, str);
  EXPECT_FALSE(hasRes);

  auto valProp = Value(3);
  obj.deleteProperty(*rt, valProp);
  auto getRes = obj.getProperty(*rt, "3");
  EXPECT_TRUE(getRes.isUndefined());

  hasRes = obj.hasProperty(*rt, "salt");
  EXPECT_TRUE(hasRes);
  obj.deleteProperty(*rt, "salt");
  hasRes = obj.hasProperty(*rt, "salt");
  EXPECT_FALSE(hasRes);

  obj = eval(
            "const obj = {};"
            "Object.defineProperty(obj, 'prop', {"
            "value: 10,"
            "configurable: false,"
            "}); obj;")
            .getObject(*rt);
  prop = PropNameID::forAscii(*rt, "prop");
  EXPECT_THROW(obj.deleteProperty(*rt, prop), JSError);
  hasRes = obj.hasProperty(*rt, "prop");
  EXPECT_TRUE(hasRes);
}

TEST_P(HermesRuntimeTest, ObjectTest) {
  eval("var obj = {1:2, 3:4}");
  auto obj = rt->global().getPropertyAsObject(*rt, "obj");

  auto propVal = Value(1);
  // Check for and get existing properties
  auto hasRes = obj.hasProperty(*rt, propVal);
  EXPECT_TRUE(hasRes);
  auto getRes = obj.getProperty(*rt, propVal);
  EXPECT_EQ(getRes.getNumber(), 2);
  // Overwrite existing property
  obj.setProperty(*rt, propVal, 3);
  getRes = obj.getProperty(*rt, propVal);
  EXPECT_EQ(getRes.getNumber(), 3);

  // Tests for non-existing properties
  propVal = Value(5);
  hasRes = obj.hasProperty(*rt, propVal);
  EXPECT_FALSE(hasRes);
  getRes = obj.getProperty(*rt, propVal);
  EXPECT_TRUE(getRes.isUndefined());

  // Add new property
  obj.setProperty(*rt, propVal, "bar");
  hasRes = obj.hasProperty(*rt, propVal);
  EXPECT_TRUE(hasRes);
  getRes = obj.getProperty(*rt, propVal);
  EXPECT_EQ(getRes.getString(*rt).utf8(*rt), "bar");

  obj = eval(
            "Object.defineProperty(obj, '456', {"
            "  value: 10,"
            "  writable: false,});")
            .getObject(*rt);
  auto unwritableProp = Value(456);
  EXPECT_THROW(obj.setProperty(*rt, unwritableProp, 1), JSError);

  auto badObjKey = eval(
      "var badObj = {"
      "    toString: function() {"
      "        throw new Error('something went wrong');"
      "    }"
      "};"
      "badObj;");
  EXPECT_THROW(obj.setProperty(*rt, badObjKey, 123), JSError);
  EXPECT_THROW(obj.hasProperty(*rt, badObjKey), JSError);
  EXPECT_THROW(obj.getProperty(*rt, badObjKey), JSError);
}

#ifdef JSI_UNSTABLE
class HermesSerializationTest : public HermesRuntimeTest {
 public:
  HermesSerializationTest() : HermesRuntimeTest() {
    serializationInterface = castInterface<ISerialization>(rt.get());
  }

  // Evaluate the given code and serialize its results
  std::shared_ptr<Serialized> evalAndSerialize(const char *code) {
    Value evalRes = eval(code);
    return serializationInterface->serialize(evalRes);
  }

  // Deserialize the given Serialized object as a jsi::Object
  Object deserializeAsObject(const std::shared_ptr<Serialized> &serialized) {
    return serializationInterface->deserialize(serialized).getObject(*rt);
  }

  ISerialization *serializationInterface;
};

TEST_P(HermesSerializationTest, SerializePrimitives) {
  auto serialized = evalAndSerialize("undefined");
  auto deserialized = serializationInterface->deserialize(serialized);
  EXPECT_TRUE(deserialized.isUndefined());

  serialized = evalAndSerialize("null");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_TRUE(deserialized.isNull());

  serialized = evalAndSerialize("true");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_TRUE(deserialized.getBool());

  serialized = evalAndSerialize("false");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_FALSE(deserialized.getBool());

  serialized = evalAndSerialize("100.99");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_EQ(deserialized.getNumber(), 100.99);

  serialized = evalAndSerialize("18446744073709551615n");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_EQ(
      deserialized.getBigInt(*rt).toString(*rt).utf8(*rt),
      "18446744073709551615");

  serialized = evalAndSerialize("'this is an ascii string!'");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_EQ(deserialized.getString(*rt).utf8(*rt), "this is an ascii string!");

  serialized = evalAndSerialize("'this is a utf16 string!\\ud83d\\udc4d'");
  deserialized = serializationInterface->deserialize(serialized);
  EXPECT_EQ(
      deserialized.getString(*rt).utf16(*rt),
      u"this is a utf16 string!\xd83d\xdc4d");

  auto sym = eval("Symbol('foo')");
  EXPECT_THROW(serializationInterface->serialize(sym), JSError);
}

TEST_P(HermesSerializationTest, SerializeSimpleObjectTypes) {
  auto serialized = evalAndSerialize("new Boolean(true)");
  auto deserializedObj = deserializeAsObject(serialized);
  auto result = deserializedObj.getPropertyAsFunction(*rt, "valueOf")
                    .callWithThis(*rt, deserializedObj);
  EXPECT_TRUE(result.getBool());

  serialized = evalAndSerialize("new Boolean(false)");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "valueOf")
               .callWithThis(*rt, deserializedObj);
  EXPECT_FALSE(result.getBool());

  serialized = evalAndSerialize("new Number(123.456)");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "valueOf")
               .callWithThis(*rt, deserializedObj);
  EXPECT_EQ(result.getNumber(), 123.456);

  serialized = evalAndSerialize("new Number('foobar')");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "valueOf")
               .callWithThis(*rt, deserializedObj);
  EXPECT_TRUE(std::isnan(result.getNumber()));

  serialized = evalAndSerialize("Object(18446744073709551615n)");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "toString")
               .callWithThis(*rt, deserializedObj);
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "18446744073709551615");

  serialized = evalAndSerialize("new String('this is an ascii string!')");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "valueOf")
               .callWithThis(*rt, deserializedObj);
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "this is an ascii string!");

  serialized =
      evalAndSerialize("new String('this is a utf16 string!\\ud83d\\udc4d')");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "valueOf")
               .callWithThis(*rt, deserializedObj);
  EXPECT_EQ(
      result.getString(*rt).utf16(*rt), u"this is a utf16 string!\xd83d\xdc4d");

  serialized = evalAndSerialize("new Date(2025, 5, 4)");
  deserializedObj = deserializeAsObject(serialized);
  result = deserializedObj.getPropertyAsFunction(*rt, "toDateString")
               .callWithThis(*rt, deserializedObj);
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "Wed Jun 04 2025");
}

TEST_P(HermesSerializationTest, SerializeRegExp) {
  // This will search for the word 'salty'. the 'i' flag indicate an
  // case-insensitive search
  auto exp = "/salty/i";
  auto serialized = evalAndSerialize(exp);
  auto deserializedObj = deserializeAsObject(serialized);

  auto testFunc = deserializedObj.getPropertyAsFunction(*rt, "test");
  auto result =
      testFunc.callWithThis(*rt, deserializedObj, "My cat's name is Salty.");

  EXPECT_TRUE(result.getBool());

  // Using named capture groups
  exp = "/(?<word>Foobar)/i";
  serialized = evalAndSerialize(exp);
  deserializedObj = deserializeAsObject(serialized);
  auto execFunc = deserializedObj.getPropertyAsFunction(*rt, "exec");
  auto testStr = "When I write tests, my go-to word is foobar";
  result = execFunc.callWithThis(*rt, deserializedObj, testStr);
  auto resultGroup = result.getObject(*rt).getPropertyAsObject(*rt, "groups");
  auto word = resultGroup.getProperty(*rt, "word");
  EXPECT_EQ(word.getString(*rt).utf8(*rt), "foobar");
}

TEST_P(HermesSerializationTest, SerializeObject) {
  // A simple object with 2 properties
  auto code = R"(
var obj = {1:2, foo: 'bar'};
obj;
)";
  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);
  auto resultValue = deserializedObj.getProperty(*rt, "1");
  EXPECT_EQ(resultValue.getNumber(), 2);
  resultValue = deserializedObj.getProperty(*rt, "foo");
  EXPECT_EQ(resultValue.getString(*rt).utf8(*rt), "bar");

  // An object that uses the same string
  code = R"(
var obj = {foo:'bar', foo2: 'bar'};
obj;
 )";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);
  resultValue = deserializedObj.getProperty(*rt, "foo");
  EXPECT_EQ(resultValue.getString(*rt).utf8(*rt), "bar");
  resultValue = deserializedObj.getProperty(*rt, "foo2");
  EXPECT_EQ(resultValue.getString(*rt).utf8(*rt), "bar");

  // An object that uses symbol for property key. Symbol should not be
  // serialized.
  auto sym = eval("var fooSym = Symbol('foo'); fooSym;").asSymbol(*rt);
  code = R"(
var obj = {1:2};
obj[fooSym] = 'baz';
obj;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);
  // Check the symbol is on the original object
  auto originalObj = rt->global().getPropertyAsObject(*rt, "obj");
  EXPECT_TRUE(originalObj.hasProperty(*rt, PropNameID::forSymbol(*rt, sym)));
  // Check the symbol is not on the cloned object
  resultValue = deserializedObj.getProperty(*rt, "1");
  EXPECT_EQ(resultValue.getNumber(), 2);
  EXPECT_FALSE(
      deserializedObj.hasProperty(*rt, PropNameID::forSymbol(*rt, sym)));

  // An object that contains another object as a property
  code = R"(
  var innerObj = {1: 2};
  var outerObj = {inner: innerObj, 3: 4};
  outerObj;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);
  resultValue = deserializedObj.getProperty(*rt, "3");
  EXPECT_EQ(resultValue.getNumber(), 4);
  auto innerObj = deserializedObj.getPropertyAsObject(*rt, "inner");
  resultValue = innerObj.getProperty(*rt, "1");
  EXPECT_EQ(resultValue.getNumber(), 2);

  // Referencing an object multiple times
  code = R"(
var innerObj = {1: 2};
var outerObj = {
    firstRef: innerObj,
    3: 4,
    nestedObj: {
        5: 6,
        secondRef: innerObj
    }
};
outerObj;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);
  resultValue = deserializedObj.getProperty(*rt, "3");
  EXPECT_EQ(resultValue.getNumber(), 4);
  innerObj = deserializedObj.getPropertyAsObject(*rt, "firstRef");
  resultValue = innerObj.getProperty(*rt, "1");
  EXPECT_EQ(resultValue.getNumber(), 2);
  auto nestedObj = deserializedObj.getPropertyAsObject(*rt, "nestedObj");
  resultValue = nestedObj.getProperty(*rt, "5");
  EXPECT_EQ(resultValue.getNumber(), 6);
  auto secondRef = nestedObj.getPropertyAsObject(*rt, "secondRef");
  EXPECT_TRUE(Object::strictEquals(*rt, innerObj, secondRef));

  // An object that points to itself in one of the properties
  code = R"(
var bar = 'bar';
var obj = {
    1: 2,
    foo: bar,
    foo2: bar
};
obj.myself = obj;
obj;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);
  resultValue = deserializedObj.getProperty(*rt, "1");
  EXPECT_EQ(resultValue.getNumber(), 2);
  innerObj = deserializedObj.getPropertyAsObject(*rt, "myself");
  resultValue = innerObj.getProperty(*rt, "1");
  EXPECT_EQ(resultValue.getNumber(), 2);
  // Changing the "inner" myself object should also be reflected at the high
  // level object
  innerObj.setProperty(*rt, "foo", "bar");
  resultValue = deserializedObj.getProperty(*rt, "foo");
  EXPECT_EQ(resultValue.getString(*rt).utf8(*rt), "bar");
}

TEST_P(HermesSerializationTest, SeriaalizeObjectNonEnumerableProperties) {
  auto code = R"(
var obj = {
  foo: "bar",
  apple: "banana"
};
Object.defineProperty(obj, "salt", {
  value: "pepper",
  enumerable: false
});
obj;
)";

  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);
  auto hasPropertyRes = deserializedObj.hasProperty(*rt, "foo");
  EXPECT_TRUE(hasPropertyRes);
  hasPropertyRes = deserializedObj.hasProperty(*rt, "apple");
  EXPECT_TRUE(hasPropertyRes);

  // 'salt' is a non-enumerable property key on the original object, so it
  // should not be cloned in the new copy
  hasPropertyRes = deserializedObj.hasProperty(*rt, "salt");
  EXPECT_FALSE(hasPropertyRes);
}

TEST_P(HermesSerializationTest, SerializeObjectRunJavaScript) {
  // When serializing `obj`, the `foo` getter will be invoked, deleting the
  // property `salt` from the object and adding the property `hello`. However,
  // the cloned object should only contain `foo`.
  auto code = R"(
var obj = {
    get foo() {
        delete this.salt;
        this.hello = 'world';
        return 'bar';
    },
    salt: 'pepper'
};
obj;
)";

  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);

  // First, check that the property 'salt' was deleted and 'hello' was added
  // to the object in the original runtime.
  auto originalObj = rt->global().getPropertyAsObject(*rt, "obj");
  auto foo = originalObj.getProperty(*rt, "foo");
  EXPECT_EQ(foo.getString(*rt).utf8(*rt), "bar");
  auto hasSalt = originalObj.hasProperty(*rt, "salt");
  EXPECT_FALSE(hasSalt);
  auto hello = originalObj.getProperty(*rt, "hello");
  EXPECT_EQ(hello.getString(*rt).utf8(*rt), "world");

  /// Check that the deserialized object only has the 'foo' property. 'salt'
  /// was deleted by the foo getter, so it was never serialized, and the
  /// algorithm shouldn't serialize any new properties added by getter.
  foo = deserializedObj.getProperty(*rt, "foo");
  EXPECT_EQ(foo.getString(*rt).utf8(*rt), "bar");
  hasSalt = deserializedObj.hasProperty(*rt, "salt");
  EXPECT_FALSE(hasSalt);
  auto hasHello = deserializedObj.hasProperty(*rt, "hello");
  EXPECT_FALSE(hasHello);
}

TEST_P(HermesSerializationTest, SerializeArrayBuffer) {
  auto code = R"(
var buffer = new ArrayBuffer(32);
buffer;
)";
  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);

  auto byteLen = deserializedObj.getProperty(*rt, "byteLength");
  EXPECT_EQ(byteLen.getNumber(), 32);

  code = R"(
var buffer = new ArrayBuffer(0);
buffer;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);

  byteLen = deserializedObj.getProperty(*rt, "byteLength");
  EXPECT_EQ(byteLen.getNumber(), 0);

  // Detached ArrayBuffers are not serializable
  code = R"(
var buffer = new ArrayBuffer(8);
HermesInternal.detachArrayBuffer(buffer);
buffer;
)";
  auto detachedBuffer = eval(code);
  EXPECT_THROW(serializationInterface->serialize(detachedBuffer), JSError);
}

TEST_P(HermesSerializationTest, SerializeDataView) {
  auto code = R"(
var buffer = new ArrayBuffer(32);
var view = new DataView(buffer, 16, 4);
view.setInt16(0, -123);
view.setInt16(2, 456);
view;
)";

  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);

  auto byteLen = deserializedObj.getProperty(*rt, "byteLength").getNumber();
  EXPECT_EQ(byteLen, 4);
  auto byteOffset = deserializedObj.getProperty(*rt, "byteOffset").getNumber();
  EXPECT_EQ(byteOffset, 16);
  auto getInt16Fn = deserializedObj.getPropertyAsFunction(*rt, "getInt16");
  auto getRes = getInt16Fn.callWithThis(*rt, deserializedObj, 0).getNumber();
  EXPECT_EQ(getRes, -123);
  getRes = getInt16Fn.callWithThis(*rt, deserializedObj, 2).getNumber();
  EXPECT_EQ(getRes, 456);

  // Verify that the underlying buffer was copied over completely
  auto buffer = deserializedObj.getPropertyAsObject(*rt, "buffer");
  byteLen = buffer.getProperty(*rt, "byteLength").getNumber();
  EXPECT_EQ(byteLen, 32);

  // The underlying ArrayBuffer is detached
  code = R"(
var buffer = new ArrayBuffer(32);
var view = new DataView(buffer, 16, 4);
HermesInternal.detachArrayBuffer(buffer);
view;
)";
  auto viewWithDetachedBuffer = eval(code);
  EXPECT_THROW(
      serializationInterface->serialize(viewWithDetachedBuffer), JSError);
}

TEST_P(HermesSerializationTest, VerifyTypeSerializedTypeArray) {
  // This test simply checks that we correctly serialize the type of the Array.
  // It does not verify the content of the array.
#define TYPED_ARRAY(name, type)                                  \
  {                                                              \
    auto code = "new " #name "Array();";                         \
    auto serialized = evalAndSerialize(code);                    \
    auto deserializedArr = deserializeAsObject(serialized);      \
    auto constructor =                                           \
        deserializedArr.getPropertyAsObject(*rt, "constructor"); \
    auto name = constructor.getProperty(*rt, "name");            \
    EXPECT_EQ(name.getString(*rt).utf8(*rt), #name "Array");     \
  }
#include "hermes/VM/TypedArrays.def"
} // namespace

TEST_P(HermesSerializationTest, SerializeTypedArray) {
  auto code = R"(
var buffer = new ArrayBuffer(32);
var float64arr = new Float64Array(buffer);
float64arr[0] = 12.34;
float64arr[1] = 34.56;
float64arr[2] = 56.78;
float64arr[3] = 78.90;

float64arr;
)";

  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);

  // Verify that the underlying buffer was copied over completely
  auto buffer = deserializedObj.getPropertyAsObject(*rt, "buffer");
  auto byteLen = buffer.getProperty(*rt, "byteLength").getNumber();
  EXPECT_EQ(byteLen, 32);

  auto bytesPerElem = deserializedObj.getProperty(*rt, "BYTES_PER_ELEMENT");
  EXPECT_EQ(bytesPerElem.getNumber(), 8);
  byteLen = deserializedObj.getProperty(*rt, "byteLength").getNumber();
  EXPECT_EQ(byteLen, 32);
  auto byteOffset = deserializedObj.getProperty(*rt, "byteOffset").getNumber();
  EXPECT_EQ(byteOffset, 0);
  auto len = deserializedObj.getProperty(*rt, "length").getNumber();
  EXPECT_EQ(len, 4);

  auto atFn = deserializedObj.getPropertyAsFunction(*rt, "at");
  auto res = atFn.callWithThis(*rt, deserializedObj, 0).getNumber();
  EXPECT_EQ(res, 12.34);
  res = atFn.callWithThis(*rt, deserializedObj, 1).getNumber();
  EXPECT_EQ(res, 34.56);
  res = atFn.callWithThis(*rt, deserializedObj, 2).getNumber();
  EXPECT_EQ(res, 56.78);
  res = atFn.callWithThis(*rt, deserializedObj, 3).getNumber();
  EXPECT_EQ(res, 78.90);

  code = R"(
var uint32arr = new Int32Array(buffer, 4, 5);
uint32arr[0] = 100;
uint32arr[1] = 200;
uint32arr[2] = 300;
uint32arr[3] = 400;
uint32arr[4] = 500;

uint32arr;
)";

  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);

  bytesPerElem = deserializedObj.getProperty(*rt, "BYTES_PER_ELEMENT");
  EXPECT_EQ(bytesPerElem.getNumber(), 4);
  byteLen = deserializedObj.getProperty(*rt, "byteLength").getNumber();
  EXPECT_EQ(byteLen, 20);
  byteOffset = deserializedObj.getProperty(*rt, "byteOffset").getNumber();
  EXPECT_EQ(byteOffset, 4);
  len = deserializedObj.getProperty(*rt, "length").getNumber();
  EXPECT_EQ(len, 5);

  atFn = deserializedObj.getPropertyAsFunction(*rt, "at");
  bytesPerElem = deserializedObj.getProperty(*rt, "BYTES_PER_ELEMENT");
  EXPECT_EQ(bytesPerElem.getNumber(), 4);
  res = atFn.callWithThis(*rt, deserializedObj, 0).getNumber();
  EXPECT_EQ(res, 100);
  res = atFn.callWithThis(*rt, deserializedObj, 1).getNumber();
  EXPECT_EQ(res, 200);
  res = atFn.callWithThis(*rt, deserializedObj, 2).getNumber();
  EXPECT_EQ(res, 300);
  res = atFn.callWithThis(*rt, deserializedObj, 3).getNumber();
  EXPECT_EQ(res, 400);
  res = atFn.callWithThis(*rt, deserializedObj, 4).getNumber();
  EXPECT_EQ(res, 500);

  // The array is empty
  code = R"(
var arr = new Int32Array(0);
arr;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);
  bytesPerElem = deserializedObj.getProperty(*rt, "BYTES_PER_ELEMENT");
  EXPECT_EQ(bytesPerElem.getNumber(), 4);
  byteLen = deserializedObj.getProperty(*rt, "byteLength").getNumber();
  EXPECT_EQ(byteLen, 0);
  byteOffset = deserializedObj.getProperty(*rt, "byteOffset").getNumber();
  EXPECT_EQ(byteOffset, 0);
  len = deserializedObj.getProperty(*rt, "length").getNumber();
  EXPECT_EQ(len, 0);

  // The underlying ArrayBuffer is detached
  code = R"(
var buffer = new ArrayBuffer(8);
var arr = new Int8Array(buffer);
HermesInternal.detachArrayBuffer(buffer);
arr;
)";
  auto arrWithDetachedBuffer = eval(code);
  EXPECT_THROW(
      serializationInterface->serialize(arrWithDetachedBuffer), JSError);
}

TEST_P(HermesSerializationTest, SerializeMap) {
  auto code = R"(
var m = new Map();
m.set('salt', 'pepper');
m.set('obj', {1: 2});
m;
)";

  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);
  auto getFunc = deserializedObj.getPropertyAsFunction(*rt, "get");
  auto size = deserializedObj.getProperty(*rt, "size").getNumber();
  EXPECT_EQ(size, 2);

  auto result = getFunc.callWithThis(
      *rt, deserializedObj, String::createFromUtf8(*rt, "salt"));
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "pepper");

  result = getFunc.callWithThis(
      *rt, deserializedObj, String::createFromUtf8(*rt, "obj"));
  EXPECT_EQ(result.getObject(*rt).getProperty(*rt, "1").getNumber(), 2);

  // An empty map
  serialized = evalAndSerialize("new Map();");
  deserializedObj = deserializeAsObject(serialized);
  size = deserializedObj.getProperty(*rt, "size").getNumber();
  EXPECT_EQ(size, 0);
}

TEST_P(HermesSerializationTest, SerializeMapRunJavaScript) {
  // When serializing the map, `obj` will also be serialized, the foo getter
  // will be invoked, removing the key `2` from the original map. However,
  // serialized map should still contain the key-value (2, 3)
  auto code = R"(
var map = new Map();
var obj = {
    get foo() {
        map.delete(2);
        return 'bar';
    }
};
map.set(1, obj);
map.set(2, 3);
map;
)";

  auto serialized = evalAndSerialize(code);
  auto mapCloned = deserializeAsObject(serialized);

  // Check the original map is modified by the `foo` getter in `obj`
  auto mapOriginal = rt->global().getPropertyAsObject(*rt, "map");
  auto hasFn = mapOriginal.getPropertyAsFunction(*rt, "has");
  auto result = hasFn.callWithThis(*rt, mapOriginal, 1);
  EXPECT_TRUE(result.getBool());
  result = hasFn.callWithThis(*rt, mapOriginal, 2);
  EXPECT_FALSE(result.getBool());

  // Check the cloned map has cloned both (1, obj) and (2, 3)
  auto getFn = mapCloned.getPropertyAsFunction(*rt, "get");
  result = getFn.callWithThis(*rt, mapCloned, 1);
  auto objCloned = result.getObject(*rt);
  auto foo = objCloned.getProperty(*rt, "foo");
  EXPECT_EQ(foo.getString(*rt).utf8(*rt), "bar");

  result = getFn.callWithThis(*rt, mapCloned, 2);
  EXPECT_EQ(result.getNumber(), 3);
}

TEST_P(HermesSerializationTest, SerializeSet) {
  auto code = R"(
var s = new Set(['salt', 10]);
s;
)";

  auto serialized = evalAndSerialize(code);
  auto deserializedObj = deserializeAsObject(serialized);
  auto size = deserializedObj.getProperty(*rt, "size").getNumber();
  EXPECT_EQ(size, 2);

  auto hasFunc = deserializedObj.getPropertyAsFunction(*rt, "has");
  auto result = hasFunc.callWithThis(
      *rt, deserializedObj, String::createFromUtf8(*rt, "salt"));
  EXPECT_TRUE(result.getBool());

  result = hasFunc.callWithThis(*rt, deserializedObj, Value(10));
  EXPECT_TRUE(result.getBool());

  result = hasFunc.callWithThis(
      *rt, deserializedObj, String::createFromUtf8(*rt, "pepper"));
  EXPECT_FALSE(result.getBool());

  // An empty set
  serialized = evalAndSerialize("new Set();");
  deserializedObj = deserializeAsObject(serialized);
  size = deserializedObj.getProperty(*rt, "size").getNumber();
  EXPECT_EQ(size, 0);
}

TEST_P(HermesSerializationTest, SerializeSetRunJavaScript) {
  // When serializing the set, `obj` will be serialized first and the foo
  // getter will be invoked. The getter will remove the `2` from the original
  // set. However, serialized set should contain both `obj` and `2`.
  auto code = R"(
var set = new Set();
var obj = {
    get foo() {
        set.delete('foobar');
        return 'bar';
    }
};
set.add(obj);
set.add('foobar');
var arr = [set, obj];
arr;
)";

  auto serialized = evalAndSerialize(code);
  auto arrCloned = deserializeAsObject(serialized).getArray(*rt);

  // Check the original set is modified by the `foo` getter in `obj`
  auto setOriginal = rt->global().getPropertyAsObject(*rt, "set");
  auto obj = rt->global().getPropertyAsObject(*rt, "obj");
  auto hasFn = setOriginal.getPropertyAsFunction(*rt, "has");
  auto result = hasFn.callWithThis(*rt, setOriginal, "foobar");
  EXPECT_FALSE(result.getBool());
  result = hasFn.callWithThis(*rt, setOriginal, obj);
  EXPECT_TRUE(result.getBool());

  // Check the cloned set has both obj and "foobar"
  auto setCloned = arrCloned.getValueAtIndex(*rt, 0).getObject(*rt);
  auto objCloned = arrCloned.getValueAtIndex(*rt, 1).getObject(*rt);
  result = hasFn.callWithThis(*rt, setCloned, "foobar");
  EXPECT_TRUE(result.getBool());
  result = hasFn.callWithThis(*rt, setCloned, objCloned);
  EXPECT_TRUE(result.getBool());

  auto foo = objCloned.getProperty(*rt, "foo");
  EXPECT_EQ(foo.getString(*rt).utf8(*rt), "bar");
}

TEST_P(HermesSerializationTest, SerializeError) {
  auto serialized = evalAndSerialize("new Error('some generic error');");
  auto deserializedObj = deserializeAsObject(serialized);

  auto errorName = deserializedObj.getProperty(*rt, "name").getString(*rt);
  EXPECT_EQ(errorName.utf8(*rt), "Error");

  auto msg = deserializedObj.getProperty(*rt, "message").getString(*rt);
  EXPECT_EQ(msg.utf8(*rt), "some generic error");

  serialized = evalAndSerialize("new TypeError('some type error');");
  deserializedObj = deserializeAsObject(serialized);

  errorName = deserializedObj.getProperty(*rt, "name").getString(*rt);
  EXPECT_EQ(errorName.utf8(*rt), "TypeError");

  msg = deserializedObj.getProperty(*rt, "message").getString(*rt);
  EXPECT_EQ(msg.utf8(*rt), "some type error");

  serialized = evalAndSerialize("new ReferenceError(undefined);");
  deserializedObj = deserializeAsObject(serialized);

  errorName = deserializedObj.getProperty(*rt, "name").getString(*rt);
  EXPECT_EQ(errorName.utf8(*rt), "ReferenceError");

  auto hasOwnFn = deserializedObj.getPropertyAsFunction(*rt, "hasOwnProperty");
  auto hasOwnRes = hasOwnFn.callWithThis(*rt, deserializedObj, "message");
  EXPECT_FALSE(hasOwnRes.getBool());

  auto code = R"(
var err = new SyntaxError();
err.message = {
    toString: function() {
        return 'some syntax error';
    }
};
err;
)";
  serialized = evalAndSerialize(code);
  deserializedObj = deserializeAsObject(serialized);

  errorName = deserializedObj.getProperty(*rt, "name").getString(*rt);
  EXPECT_EQ(errorName.utf8(*rt), "SyntaxError");
  msg = deserializedObj.getProperty(*rt, "message").getString(*rt);
  EXPECT_EQ(msg.utf8(*rt), "some syntax error");
}

TEST_P(HermesSerializationTest, SerializeArray) {
  // Simple array
  auto code = R"(
var arr = ['a', 'b', 'c'];
arr;
)";
  auto serialized = evalAndSerialize(code);
  auto deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  auto arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 3);
  auto result = deserializedArr.getValueAtIndex(*rt, 1);
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "b");

  // Array with objects
  code = R"(
var foo = {salt: 'pepper'};
var bar = [100, 99, 'apple', foo];
bar;
)";
  serialized = evalAndSerialize(code);
  deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 4);
  result = deserializedArr.getValueAtIndex(*rt, 1);
  EXPECT_EQ(result.getNumber(), 99);

  result = deserializedArr.getValueAtIndex(*rt, 2);
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "apple");

  auto fooObj = deserializedArr.getValueAtIndex(*rt, 3).getObject(*rt);
  result = fooObj.getProperty(*rt, "salt");
  EXPECT_EQ(result.getString(*rt).utf8(*rt), "pepper");

  // Sparse Array
  code = R"(
var arr = [1,,3,,5];
arr;
)";
  serialized = evalAndSerialize(code);
  deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 5);
  result = deserializedArr.getValueAtIndex(*rt, 0);
  EXPECT_EQ(result.getNumber(), 1);
  result = deserializedArr.getValueAtIndex(*rt, 1);
  EXPECT_TRUE(result.isUndefined());
  result = deserializedArr.getValueAtIndex(*rt, 2);
  EXPECT_EQ(result.getNumber(), 3);
  result = deserializedArr.getValueAtIndex(*rt, 3);
  EXPECT_TRUE(result.isUndefined());
  result = deserializedArr.getValueAtIndex(*rt, 4);
  EXPECT_EQ(result.getNumber(), 5);

  // Serializing the array will invoke the getter, which deletes the last
  // element from the array.
  code = R"(
var obj = {};
var arr = [1, obj, 3];
Object.defineProperty(obj, "foo", {
  enumerable: true,
  get: function() {
    arr.length = 2;
    return "bar"
  }
});
arr;
)";
  serialized = evalAndSerialize(code);
  deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  arrLen = deserializedArr.length(*rt);
  // Even though the original array was resized to 2, note that the serialize
  // algorithm store the length directly, and expects the length to be set when
  // deserializing.
  EXPECT_EQ(arrLen, 3);
  result = deserializedArr.getValueAtIndex(*rt, 0);
  EXPECT_EQ(result.getNumber(), 1);
  result = deserializedArr.getValueAtIndex(*rt, 1);
  auto resultObj = result.getObject(*rt);
  EXPECT_EQ(resultObj.getProperty(*rt, "foo").getString(*rt).utf8(*rt), "bar");
  result = deserializedArr.getValueAtIndex(*rt, 2);
  EXPECT_TRUE(result.isUndefined());
  // Verify the getter modified the original arr
  auto originalArr = rt->global().getPropertyAsObject(*rt, "arr").getArray(*rt);
  arrLen = originalArr.length(*rt);
  EXPECT_EQ(arrLen, 2);
}

TEST_P(HermesSerializationTest, SerializeArrayNamedProperty) {
  // Array with named property key "1"
  auto code = R"(
var arr = [100, 200];
Object.defineProperty(arr, 1, {
  value: 300,
  configurable: false,
  enumerable: true,
  writeable: true,
});
arr;
)";
  auto serialized = evalAndSerialize(code);
  auto deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  auto result = deserializedArr.getValueAtIndex(*rt, 1);
  auto arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 2);
  // We should be getting the named property "1" which has the value 300, and
  // not the value originally in the JSArray indexed storage
  EXPECT_EQ(result.getNumber(), 300);

  code = R"(
var arr = [100, 200];
arr.foo = "bar";
arr;
)";
  serialized = evalAndSerialize(code);
  deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 2);
  result = deserializedArr.getValueAtIndex(*rt, 1);
  EXPECT_EQ(result.getNumber(), 200);

  // An array with a getter
  code = R"(
var arr = [1, 2, 3];
Object.defineProperty(arr, 1, {
  enumerable: true,
  configurable: true,
  get: function() {
    arr[2] = 200;
    arr[3] = 300;
    return 100
  }
});
arr;
)";
  serialized = evalAndSerialize(code);
  deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 3);
  result = deserializedArr.getValueAtIndex(*rt, 0);
  EXPECT_EQ(result.getNumber(), 1);
  // Getter returns 100, even if the array storage originally had stored 2
  result = deserializedArr.getValueAtIndex(*rt, 1);
  EXPECT_EQ(result.getNumber(), 100);
  // Getter effected element at index 2
  result = deserializedArr.getValueAtIndex(*rt, 2);
  EXPECT_EQ(result.getNumber(), 200);
  // Getter added the element at index 3 to the original array.
  auto originalArr = rt->global().getPropertyAsObject(*rt, "arr").getArray(*rt);
  result = originalArr.getValueAtIndex(*rt, 3);
  EXPECT_EQ(result.getNumber(), 300);
  // However, the cloning algorithm obtains a list of all property keys to
  // serialize up-front, and does not account for properties added as a result
  // of the getter's side effects.
  result = deserializedArr.hasProperty(*rt, 3);
  EXPECT_FALSE(result.getBool());

  // The array is eligible for fast path at first, but once the object element
  // is serialized, it makes the array ineligible for fast path
  code = R"(
var obj = {};
Object.defineProperty(obj, "foo", {
  enumerable: true,
  get: function() {
    Object.defineProperty(arr, 2, {
      enumerable: true,
      configurable: false,
      value: 200,
    });
    return "bar";
  }
});
var arr = [1, obj, 2];
arr;
)";
  serialized = evalAndSerialize(code);
  deserializedArr = deserializeAsObject(serialized).getArray(*rt);
  result = deserializedArr.getValueAtIndex(*rt, 0);
  EXPECT_EQ(result.getNumber(), 1);
  result = deserializedArr.getValueAtIndex(*rt, 1);
  auto resultObj = result.getObject(*rt);
  EXPECT_EQ(resultObj.getProperty(*rt, "foo").getString(*rt).utf8(*rt), "bar");
  result = deserializedArr.getValueAtIndex(*rt, 2);
  EXPECT_EQ(result.getNumber(), 200);
  arrLen = deserializedArr.length(*rt);
  EXPECT_EQ(arrLen, 3);
}

TEST_P(HermesSerializationTest, SerializeUnsupported) {
  // Go through known unsupported values for serialization and make sure we
  // throw
  // Functions
  auto func = eval("(function testFunc() {})");
  EXPECT_THROW(serializationInterface->serialize(func), JSError);

  // Host Objects
  class TestHostObject : public HostObject {
    std::vector<PropNameID> getPropertyNames(Runtime &rt) override {
      return PropNameID::names(rt, "test");
    }

    Value get(Runtime &runtime, const PropNameID &name) override {
      return Value();
    }
  };
  Object hostObj =
      Object::createFromHostObject(*rt, std::make_shared<TestHostObject>());

  auto hostObjVal = Value(*rt, hostObj);
  EXPECT_THROW(serializationInterface->serialize(hostObjVal), JSError);

  auto proxy = eval(
      "var target = {};"
      "var handler = {};"
      "var proxy = new Proxy(target, handler); proxy;");

  EXPECT_THROW(serializationInterface->serialize(proxy), JSError);
}

INSTANTIATE_TEST_CASE_P(
    Runtimes,
    HermesSerializationTest,
    ::testing::ValuesIn(runtimeGenerators()));
#endif

INSTANTIATE_TEST_CASE_P(
    Runtimes,
    HermesRuntimeTest,
    ::testing::ValuesIn(runtimeGenerators()));
} // namespace
