/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StackTracesTree-NoRuntime.h"

#if defined(HERMES_ENABLE_ALLOCATION_LOCATION_TRACES) and \
    !defined(HERMESVM_GC_HADES)

#include "TestHelpers.h"

using namespace hermes::vm;
using namespace hermes::parser;

namespace hermes {
namespace unittest {
namespace stacktracestreetest {

namespace {
struct StackTracesTreeTest : public RuntimeTestFixtureBase {
  explicit StackTracesTreeTest()
      : RuntimeTestFixtureBase(
            RuntimeConfig::Builder(kTestRTConfigBuilder)
                .withES6Proxy(true)
                .withGCConfig(GCConfig::Builder(kTestGCConfigBuilder)
                                  .withAllocationLocationTrackerFromStart(true)
                                  .build())
                .build()) {}

  explicit StackTracesTreeTest(const RuntimeConfig &config)
      : RuntimeTestFixtureBase(config) {}

  ::testing::AssertionResult eval(const std::string &code) {
    hbc::CompileFlags flags;
    // Ideally none of this should require debug info, so let's ensure it
    // doesn't.
    flags.debug = false;
    flags.optimize = false;
    auto runRes = runtime->run(code, "eval.js", flags);
    return isException(runRes);
  };

  ::testing::AssertionResult checkTraceMatches(
      const std::string &code,
      const std::string &expectedTrace) {
    hbc::CompileFlags flags;
    flags.debug = false;
    flags.optimize = false;
    auto runRes = runtime->run(code, "test.js", flags);
    if (isException(runRes)) {
      return isException(runRes);
    }
    if (!runRes->isPointer()) {
      return ::testing::AssertionFailure()
          << "Returned value was not a HV with a pointer";
    }
    std::string res;
    llvh::raw_string_ostream resStream(res);
    auto stringTable = runtime->getStackTracesTree()->getStringTable();
    auto allocationLocationTracker =
        runtime->getHeap().getAllocationLocationTracker();
    auto node = allocationLocationTracker.getStackTracesTreeNodeForAlloc(
        runRes->getPointer());
    while (node) {
      resStream << (*stringTable)[node->name] << " "
                << (*stringTable)[node->sourceLoc.scriptName] << ":"
                << node->sourceLoc.lineNo << ":" << node->sourceLoc.columnNo
                << "\n";
      node = node->parent;
    }
    resStream.flush();
    auto trimmedRes = llvh::StringRef(res).trim();
    return trimmedRes == expectedTrace
        ? ::testing::AssertionSuccess()
        : (::testing::AssertionFailure()
           << "Expected trace:\n"
           << expectedTrace.c_str() << "\nActual trace:\n"
           << trimmedRes.str().c_str());
  };
};

// Used to inject a no-op function into JS.
static CallResult<HermesValue> noop(void *, Runtime *runtime, NativeArgs) {
  return HermesValue::encodeUndefinedValue();
}

static CallResult<HermesValue>
enableAllocationLocationTracker(void *, Runtime *runtime, NativeArgs) {
  runtime->enableAllocationLocationTracker();
  return HermesValue::encodeUndefinedValue();
}

struct StackTracesTreeParameterizedTest
    : public StackTracesTreeTest,
      public ::testing::WithParamInterface<bool> {
  StackTracesTreeParameterizedTest()
      : StackTracesTreeTest(
            RuntimeConfig::Builder(kTestRTConfigBuilder)
                .withES6Proxy(true)
                .withGCConfig(GCConfig::Builder(kTestGCConfigBuilder)
                                  .withAllocationLocationTrackerFromStart(
                                      trackerOnByDefault())
                                  .build())
                .build()) {}

  bool trackerOnByDefault() const {
    // If GetParam() is true, then allocation tracking is enabled from the
    // start. If GetParam() is false, then allocation tracking begins when
    // enableAllocationLocationTracker is called.
    return GetParam();
  }

  /// Delete the existing tree and reset all state related to allocations.
  void resetTree() {
    // Calling this should clear all existing StackTracesTree data.
    runtime->disableAllocationLocationTracker(true);
    ASSERT_FALSE(runtime->getStackTracesTree());
    // If the tracker was on by default, after cleaning it should be re-enabled,
    // so the function doesn't need to be called.
    if (trackerOnByDefault()) {
      runtime->enableAllocationLocationTracker();
    }
  }

  void SetUp() override {
    // Add a JS function 'enableAllocationLocationTracker'
    // The stack traces for objects allocated after the call to
    // enableAllocationLocationTracker should be identical.
    SymbolID enableAllocationLocationTrackerSym;
    {
      vm::GCScope gcScope(runtime);
      enableAllocationLocationTrackerSym =
          vm::stringToSymbolID(
              runtime,
              vm::StringPrimitive::createNoThrow(
                  runtime, "enableAllocationLocationTracker"))
              ->getHermesValue()
              .getSymbol();
    }

    ASSERT_FALSE(isException(JSObject::putNamed_RJS(
        runtime->getGlobal(),
        runtime,
        enableAllocationLocationTrackerSym,
        runtime->makeHandle<NativeFunction>(
            *NativeFunction::createWithoutPrototype(
                runtime,
                nullptr,
                trackerOnByDefault() ? noop : enableAllocationLocationTracker,
                enableAllocationLocationTrackerSym,
                0)))));
  }

  // No need for a tear-down, because the runtime destructor will clear all
  // memory.
};

} // namespace

static std::string stackTraceToJSON(StackTracesTree &tree) {
  auto &stringTable = *tree.getStringTable();
  std::string res;
  llvh::raw_string_ostream stream(res);
  JSONEmitter json(stream, /* pretty */ true);
  llvh::SmallVector<StackTracesTreeNode *, 128> nodeStack;
  nodeStack.push_back(tree.getRootNode());
  while (!nodeStack.empty()) {
    auto curNode = nodeStack.pop_back_val();
    if (!curNode) {
      json.closeArray();
      json.closeDict();
      continue;
    }
    json.openDict();
    json.emitKeyValue("name", stringTable[curNode->name]);
    json.emitKeyValue("scriptName", stringTable[curNode->sourceLoc.scriptName]);
    json.emitKeyValue("line", curNode->sourceLoc.lineNo);
    json.emitKeyValue("col", curNode->sourceLoc.columnNo);
    json.emitKey("children");
    json.openArray();
    nodeStack.push_back(nullptr);
    for (auto child : curNode->getChildren()) {
      nodeStack.push_back(child);
    }
  }
  stream.flush();
  return res;
}

#define ASSERT_RUN_TRACE(code, trace)                                        \
  ASSERT_TRUE(                                                               \
      checkTraceMatches(code, llvh::StringRef(trace).trim().str().c_str())); \
  ASSERT_TRUE(runtime->getStackTracesTree()->isHeadAtRoot())

TEST_F(StackTracesTreeTest, BasicOperation) {
  ASSERT_RUN_TRACE(
      "function bar() {return new Object();}; function foo() {return bar();}; foo();",
      R"#(
bar test.js:1:34
foo test.js:1:66
global test.js:1:75
global test.js:1:1
(root) :0:0
    )#");

  const auto expectedTree = llvh::StringRef(R"#(
{
  "name": "(root)",
  "scriptName": "",
  "line": 0,
  "col": 0,
  "children": [
    {
      "name": "global",
      "scriptName": "test.js",
      "line": 1,
      "col": 1,
      "children": [
        {
          "name": "global",
          "scriptName": "test.js",
          "line": 1,
          "col": 75,
          "children": [
            {
              "name": "foo",
              "scriptName": "test.js",
              "line": 1,
              "col": 66,
              "children": [
                {
                  "name": "bar",
                  "scriptName": "test.js",
                  "line": 1,
                  "col": 34,
                  "children": []
                },
                {
                  "name": "bar",
                  "scriptName": "test.js",
                  "line": 1,
                  "col": 1,
                  "children": []
                }
              ]
            },
            {
              "name": "foo",
              "scriptName": "test.js",
              "line": 1,
              "col": 40,
              "children": []
            }
          ]
        },
        {
          "name": "global",
          "scriptName": "test.js",
          "line": 1,
          "col": 1,
          "children": []
        }
      ]
    }
  ]
}
  )#")
                                .trim();
  auto stackTracesTree = runtime->getStackTracesTree();
  ASSERT_TRUE(stackTracesTree);
  ASSERT_STREQ(
      stackTraceToJSON(*stackTracesTree).c_str(), expectedTree.str().c_str());
}

TEST_P(StackTracesTreeParameterizedTest, GlobalScopeAlloc) {
  // Not only should the trace be correct but the stack trace should be
  // popped back down to the root. This is implicitly checked by
  // ASSERT_RUN_TRACE.
  ASSERT_RUN_TRACE(
      R"#(
enableAllocationLocationTracker();
new Object();
)#",
      R"#(
global test.js:3:11
global test.js:2:1
(root) :0:0
      )#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughNamedAnon) {
  ASSERT_RUN_TRACE(
      R"#(
function foo() {
  function bar() {
    var anonVar = function() {
      enableAllocationLocationTracker();
      return new Object();
    }
    return anonVar();
  }
  return bar();
}
foo();
)#",
      R"#(
anonVar test.js:6:24
bar test.js:8:19
foo test.js:10:13
global test.js:12:4
global test.js:2:1
(root) :0:0
)#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughAnon) {
  ASSERT_RUN_TRACE(
      R"#(
function foo() {
  return (function() {
    enableAllocationLocationTracker();
    return new Object();
  })();
}
foo();
)#",
      R"#(
(anonymous) test.js:5:22
foo test.js:6:5
global test.js:8:4
global test.js:2:1
(root) :0:0
)#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughAssignedFunction) {
  ASSERT_RUN_TRACE(
      R"#(
function foo() {
  enableAllocationLocationTracker();
  return new Object();
}
var bar = foo;
bar();
)#",
      R"#(
foo test.js:4:20
global test.js:7:4
global test.js:2:1
(root) :0:0
)#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughGetter) {
  ASSERT_RUN_TRACE(
      R"#(
const obj = {
  get foo() {
    enableAllocationLocationTracker();
    return new Object();
  }
}
obj.foo;
)#",
      R"#(
get foo test.js:5:22
global test.js:8:4
global test.js:2:1
(root) :0:0
)#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughProxy) {
  ASSERT_RUN_TRACE(
      R"#(
const handler = {
  get: function(obj, prop) {
    enableAllocationLocationTracker();
    return new Object();
  }
};
const p = new Proxy({}, handler);
p.something;
)#",
      R"#(
get test.js:5:22
global test.js:9:2
global test.js:2:1
(root) :0:0
)#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughEval) {
  ASSERT_RUN_TRACE(
      R"#(
function returnit() {
  enableAllocationLocationTracker();
  return new Object();
}
eval("returnit()");
)#",
      R"#(
returnit test.js:4:20
eval JavaScript:1:9
global test.js:6:5
global test.js:2:1
(root) :0:0
)#");
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughBoundFunctions) {
  ASSERT_FALSE(eval(
      R"#(
function foo() {
  enableAllocationLocationTracker();
  return new Object();
})#"));

  ASSERT_RUN_TRACE("foo.bind(null)()", R"#(
foo eval.js:4:20
global test.js:1:15
global test.js:1:1
(root) :0:0
)#");
  resetTree();

  ASSERT_RUN_TRACE("foo.bind(null).bind(null)()", R"#(
foo eval.js:4:20
global test.js:1:26
global test.js:1:1
(root) :0:0
)#");
  resetTree();

  ASSERT_RUN_TRACE(
      R"#(
function chain1() {
  return chain2bound();
}

function chain2() {
  enableAllocationLocationTracker();
  return new Object();
}

var chain2bound = chain2.bind(null);

chain1.bind(null)();
  )#",
      R"#(
chain2 test.js:8:20
chain1 test.js:3:21
global test.js:13:18
global test.js:2:1
(root) :0:0
        )#");
  resetTree();
}

TEST_P(StackTracesTreeParameterizedTest, TraceThroughNative) {
  ASSERT_RUN_TRACE(
      R"#(
function foo(x) {
  enableAllocationLocationTracker();
  return new Object();
}
([0].map(foo))[0];
)#",
      R"#(
foo test.js:4:20
global test.js:6:9
global test.js:2:1
(root) :0:0
      )#");
}

TEST_P(StackTracesTreeParameterizedTest, UnwindOnThrow) {
  // This relies on ASSERT_RUN_TRACE implicitly checking the stack is cleared
  ASSERT_RUN_TRACE(
      R"#(
function foo() {
  try {
    function throws() {
      enableAllocationLocationTracker();
      throw new Error();
    }
    ([0].map(throws.bind(null)))[0];
  } catch(e) {
    return e;
  }
  return false;
}
foo();
)#",
      R"#(
throws test.js:6:22
foo test.js:8:13
global test.js:14:4
global test.js:2:1
(root) :0:0
      )#");
  resetTree();

  // Test catching multiple blocks up.
  ASSERT_RUN_TRACE(
      R"#(
function thrower() {
  enableAllocationLocationTracker();
  throw new Error();
}
function layerOne() { return thrower(); }
function layerTwo() { return layerOne(); }
function tryAlloc() {
  try {
    layerTwo();
  } catch (e) {
    return e;
  }
}
tryAlloc();
)#",
      R"#(
thrower test.js:4:18
layerOne test.js:6:37
layerTwo test.js:7:38
tryAlloc test.js:10:13
global test.js:15:9
global test.js:2:1
(root) :0:0
      )#");
}

TEST_P(StackTracesTreeParameterizedTest, MultipleNativeLayers) {
  // Multiple map and bind layers.
  ASSERT_RUN_TRACE(
      R"#(
function foo() {
  enableAllocationLocationTracker();
  return new Object();
}
([0].map(foo.bind(null)))[0];
)#",
      R"#(
foo test.js:4:20
global test.js:6:9
global test.js:2:1
(root) :0:0
        )#");
  resetTree();

  // Multiple Function.prototype.apply layers.
  ASSERT_RUN_TRACE(
      R"#(
function foo() {
  enableAllocationLocationTracker();
  return new Object();
}
function secondLayerApply() { return foo.apply(null, []); }
function layered() { return secondLayerApply(); }
function fooApply() { return layered.apply(null, []); }
fooApply();
)#",
      R"#(
foo test.js:4:20
secondLayerApply test.js:6:47
layered test.js:7:45
fooApply test.js:8:43
global test.js:9:9
global test.js:2:1
(root) :0:0
      )#");
  resetTree();
}

// Test with the allocation location tracker on and off.
INSTANTIATE_TEST_CASE_P(
    WithOrWithoutAllocationTracker,
    StackTracesTreeParameterizedTest,
    ::testing::Bool());

TEST_F(StackTracesTreeTest, MultipleAllocationsMergeInTree) {
  ASSERT_FALSE(eval(R"#(

function foo() {
  return new Object();
}
function bar(a) {
  for (var i = 0; i < a[1]; i++) {
    a[0]();
  }
}
function baz() {
  return new Object();
}
[[foo, 1], [foo, 10], [baz, 1]].map(bar);
)#"));

  const auto expectedTree = llvh::StringRef(R"#(
{
  "name": "(root)",
  "scriptName": "",
  "line": 0,
  "col": 0,
  "children": [
    {
      "name": "global",
      "scriptName": "eval.js",
      "line": 3,
      "col": 1,
      "children": [
        {
          "name": "global",
          "scriptName": "eval.js",
          "line": 14,
          "col": 36,
          "children": [
            {
              "name": "bar",
              "scriptName": "eval.js",
              "line": 8,
              "col": 9,
              "children": [
                {
                  "name": "baz",
                  "scriptName": "eval.js",
                  "line": 12,
                  "col": 20,
                  "children": []
                },
                {
                  "name": "baz",
                  "scriptName": "eval.js",
                  "line": 11,
                  "col": 1,
                  "children": []
                },
                {
                  "name": "foo",
                  "scriptName": "eval.js",
                  "line": 4,
                  "col": 20,
                  "children": []
                },
                {
                  "name": "foo",
                  "scriptName": "eval.js",
                  "line": 3,
                  "col": 1,
                  "children": []
                }
              ]
            },
            {
              "name": "bar",
              "scriptName": "eval.js",
              "line": 6,
              "col": 1,
              "children": []
            }
          ]
        },
        {
          "name": "global",
          "scriptName": "eval.js",
          "line": 14,
          "col": 24,
          "children": []
        },
        {
          "name": "global",
          "scriptName": "eval.js",
          "line": 14,
          "col": 13,
          "children": []
        },
        {
          "name": "global",
          "scriptName": "eval.js",
          "line": 14,
          "col": 2,
          "children": []
        },
        {
          "name": "global",
          "scriptName": "eval.js",
          "line": 14,
          "col": 3,
          "children": []
        },
        {
          "name": "global",
          "scriptName": "eval.js",
          "line": 3,
          "col": 1,
          "children": []
        }
      ]
    }
  ]
}
  )#")
                                .trim();
  auto stackTracesTree = runtime->getStackTracesTree();
  ASSERT_TRUE(stackTracesTree);
  ASSERT_STREQ(
      stackTraceToJSON(*stackTracesTree).c_str(), expectedTree.str().c_str());
}

} // namespace stacktracestreetest
} // namespace unittest
} // namespace hermes

#endif // HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
