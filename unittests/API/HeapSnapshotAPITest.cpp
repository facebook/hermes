/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

// gtest must come before folly, because folly/portability/Windows.h undefines
// some windows standard library macros that gtest-port.h relies on.
#include <gtest/gtest.h>

#include <folly/dynamic.h>
#include <folly/json.h>
#include <hermes/CompileJS.h>
#include <hermes/hermes.h>
#include <jsi/instrumentation.h>

using namespace facebook::jsi;
using namespace facebook::hermes;

class HeapSnapshotAPITest : public ::testing::TestWithParam<bool> {
 public:
  HeapSnapshotAPITest()
      : rt(makeHermesRuntime(::hermes::vm::RuntimeConfig::Builder()
                                 .withES6Proxy(true)
                                 .build())) {
    if (trackingFromBeginning()) {
      rt->instrumentation().startTrackingHeapObjectStackTraces(nullptr);
    }
  }

 protected:
  Value eval(const char *code) {
    return rt->evaluateJavaScript(
        std::make_unique<StringBuffer>(code), "test.js");
  }

  bool trackingFromBeginning() const {
    return GetParam();
  }

  void startTrackingHeapObjects() {
    if (!trackingFromBeginning()) {
      rt->instrumentation().startTrackingHeapObjectStackTraces(nullptr);
    }
  }

  void stopTrackingHeapObjects() {
    rt->instrumentation().stopTrackingHeapObjectStackTraces();
  }

  std::shared_ptr<HermesRuntime> rt;
};

static std::string functionInfoToString(
    int idx,
    const folly::dynamic &traceFunctionInfos,
    const folly::dynamic &strings) {
  auto it = traceFunctionInfos.begin() + idx * 6;
  auto functionID = it->asInt();
  auto name = strings.at((it + 1)->asInt()).asString();
  auto scriptName = strings.at((it + 2)->asInt()).asString();
  auto scriptID = (it + 3)->asInt();
  auto line = (it + 4)->asInt();
  auto col = (it + 5)->asInt();

  std::ostringstream os;
  os << name << "(" << functionID << ") @ " << scriptName << "(" << scriptID
     << "):" << line << ":" << col << "";
  return os.str();
}

struct ChromeStackTreeNode {
  ChromeStackTreeNode(ChromeStackTreeNode *parent, int traceFunctionInfosId)
      : parent_(parent), traceFunctionInfosId_(traceFunctionInfosId) {}

  /// Recursively builds up a tree of trace nodes, and inserts a pair of (trace
  /// node id, pointer to node in tree) into \p idNodeMap.
  /// WARN: The return value of this function keeps the node pointers alive. It
  /// must outlive \p idNodeMap or else \p idNodeMap will have dangling pointers
  /// to the nodes.
  static std::vector<std::unique_ptr<ChromeStackTreeNode>> parse(
      const folly::dynamic &traceNodes,
      ChromeStackTreeNode *parent,
      std::map<int, ChromeStackTreeNode *> &idNodeMap) {
    std::vector<std::unique_ptr<ChromeStackTreeNode>> res;
    for (auto node = traceNodes.begin(); node != traceNodes.end(); node += 5) {
      auto id = node->asInt();
      auto functionInfoIndex = (node + 1)->asInt();
      folly::dynamic children = *(node + 4);
      auto treeNode =
          std::make_unique<ChromeStackTreeNode>(parent, functionInfoIndex);
      idNodeMap.emplace(id, treeNode.get());
      treeNode->children_ = parse(children, treeNode.get(), idNodeMap);
      res.emplace_back(std::move(treeNode));
    }
    return res;
  };

  std::string buildStackTrace(
      const folly::dynamic &traceFunctionInfos,
      const folly::dynamic &strings) {
    std::string res =
        parent_ ? parent_->buildStackTrace(traceFunctionInfos, strings) : "";
    res += "\n" +
        functionInfoToString(
               traceFunctionInfosId_, traceFunctionInfos, strings);
    return res;
  };

 private:
  ChromeStackTreeNode *parent_;
  int traceFunctionInfosId_;
  std::vector<std::unique_ptr<ChromeStackTreeNode>> children_;
};

TEST_P(HeapSnapshotAPITest, HeapTimeline) {
  startTrackingHeapObjects();
  facebook::jsi::Function alloc = eval("function alloc() { return {}; }; alloc")
                                      .asObject(*rt)
                                      .asFunction(*rt);
  facebook::jsi::Object obj = alloc.call(*rt).asObject(*rt);
  const uint64_t objID = rt->getUniqueID(obj);

  std::ostringstream os;
  rt->instrumentation().collectGarbage("test");
  rt->instrumentation().createSnapshotToStream(os);
  stopTrackingHeapObjects();

  const std::string heapTimeline = os.str();
  folly::dynamic json = folly::parseJson(heapTimeline);
  ASSERT_TRUE(json.isObject());
  auto it = json.find("strings");
  ASSERT_NE(it, json.items().end());
  auto strings = it->second;
  it = json.find("nodes");
  ASSERT_NE(it, json.items().end());
  auto nodes = it->second;
  it = json.find("trace_tree");
  ASSERT_NE(it, json.items().end());
  auto traceTree = it->second;
  it = json.find("trace_function_infos");
  ASSERT_NE(it, json.items().end());
  auto traceFunctionInfos = it->second;

  // The root node should be the only thing at the top of the tree. There are 5
  // fields per single node, and the last field is a children array.
  EXPECT_EQ(traceTree.size(), 5)
      << "There should never be more than a single 5-tuple at the beginning of "
         "the trace tree";

  // Search nodes for the objID.
  const auto nodeTupleSize = 7;
  const auto nodeIDFieldIndex = 2;
  const auto nodeTraceIDFieldIndex = 5;
  uint64_t traceNodeID = 0;
  ASSERT_EQ(nodes.size() % nodeTupleSize, 0)
      << "Nodes array must consist of tuples";
  for (auto node = nodes.begin(); node != nodes.end(); node += nodeTupleSize) {
    if (static_cast<uint64_t>((node + nodeIDFieldIndex)->asInt()) == objID) {
      traceNodeID = (node + nodeTraceIDFieldIndex)->asInt();
      EXPECT_NE(traceNodeID, 0ul) << "Object in node graph has a zero trace ID";
      break;
    }
  }
  ASSERT_NE(traceNodeID, 0ul) << "Object not found in nodes graph";

  // Now use the trace node ID to locate the corresponding stack.
  std::map<int, ChromeStackTreeNode *> idNodeMap;
  auto roots = ChromeStackTreeNode::parse(traceTree, nullptr, idNodeMap);
  (void)roots;
  auto stackTreeNode = idNodeMap.find(traceNodeID);
  ASSERT_NE(stackTreeNode, idNodeMap.end());
  EXPECT_EQ(
      stackTreeNode->second->buildStackTrace(traceFunctionInfos, strings),
      R"#(
(root)(0) @ (0):0:0
global(1) @ test.js(2):1:1
alloc(2) @ test.js(2):1:27)#");
}

INSTANTIATE_TEST_CASE_P(
    WithOrWithoutAllocationTracker,
    HeapSnapshotAPITest,
    ::testing::Bool());

#endif
