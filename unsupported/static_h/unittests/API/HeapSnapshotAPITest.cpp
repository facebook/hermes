/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_MEMORY_INSTRUMENTATION

// gtest must come before folly, because folly/portability/Windows.h undefines
// some windows standard library macros that gtest-port.h relies on.
#include <gtest/gtest.h>

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
    const std::vector<uint64_t> &traceFunctionInfos,
    const std::vector<std::string> &strings) {
  auto functionID = traceFunctionInfos[idx * 6];
  auto name = strings[traceFunctionInfos[idx * 6 + 1]];
  auto scriptName = strings[traceFunctionInfos[idx * 6 + 2]];
  auto scriptID = traceFunctionInfos[idx * 6 + 3];
  auto line = traceFunctionInfos[idx * 6 + 4];
  auto col = traceFunctionInfos[idx * 6 + 5];

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
      facebook::jsi::Runtime *rt,
      const facebook::jsi::Array &traceNodes,
      ChromeStackTreeNode *parent,
      std::map<int, ChromeStackTreeNode *> &idNodeMap) {
    std::vector<std::unique_ptr<ChromeStackTreeNode>> res;
    auto numNodes = traceNodes.size(*rt);
    for (size_t i = 0; i < numNodes; i += 5) {
      auto id = traceNodes.getValueAtIndex(*rt, i).asNumber();
      auto functionInfoIndex =
          traceNodes.getValueAtIndex(*rt, i + 1).asNumber();
      auto children =
          traceNodes.getValueAtIndex(*rt, i + 4).asObject(*rt).asArray(*rt);
      auto treeNode =
          std::make_unique<ChromeStackTreeNode>(parent, functionInfoIndex);
      idNodeMap.emplace(id, treeNode.get());
      treeNode->children_ = parse(rt, children, treeNode.get(), idNodeMap);
      res.emplace_back(std::move(treeNode));
    }
    return res;
  };

  std::string buildStackTrace(
      const std::vector<uint64_t> &traceFunctionInfos,
      const std::vector<std::string> &strings) {
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
  auto json = rt->global()
                  .getPropertyAsObject(*rt, "JSON")
                  .getPropertyAsFunction(*rt, "parse")
                  .call(*rt, heapTimeline)
                  .asObject(*rt);

  auto jsStrings = json.getPropertyAsObject(*rt, "strings").asArray(*rt);

  std::vector<std::string> strings;
  for (size_t i = 0; i < jsStrings.size(*rt); i++)
    strings.push_back(
        jsStrings.getValueAtIndex(*rt, i).asString(*rt).utf8(*rt));

  auto jsNodes = json.getPropertyAsObject(*rt, "nodes").asArray(*rt);
  std::vector<uint64_t> nodes;
  for (size_t i = 0; i < jsNodes.size(*rt); i++)
    nodes.push_back(jsNodes.getValueAtIndex(*rt, i).asNumber());

  auto jsTraceFunctionInfos =
      json.getPropertyAsObject(*rt, "trace_function_infos").asArray(*rt);
  std::vector<uint64_t> traceFunctionInfos;
  for (size_t i = 0; i < jsTraceFunctionInfos.size(*rt); i++)
    traceFunctionInfos.push_back(
        jsTraceFunctionInfos.getValueAtIndex(*rt, i).asNumber());

  auto traceTree = json.getPropertyAsObject(*rt, "trace_tree").asArray(*rt);

  // The root node should be the only thing at the top of the tree. There are 5
  // fields per single node, and the last field is a children array.
  EXPECT_EQ(traceTree.size(*rt), 5)
      << "There should never be more than a single 5-tuple at the beginning of "
         "the trace tree";

  // Search nodes for the objID.
  const auto nodeTupleSize = 7;
  const auto nodeIDFieldIndex = 2;
  const auto nodeTraceIDFieldIndex = 5;
  uint64_t traceNodeID = 0;
  ASSERT_EQ(nodes.size() % nodeTupleSize, 0)
      << "Nodes array must consist of tuples";
  for (size_t i = 0; i < nodes.size(); i += nodeTupleSize) {
    if (nodes[i + nodeIDFieldIndex] == objID) {
      traceNodeID = nodes[i + nodeTraceIDFieldIndex];
      EXPECT_NE(traceNodeID, 0ul) << "Object in node graph has a zero trace ID";
      break;
    }
  }
  ASSERT_NE(traceNodeID, 0ul) << "Object not found in nodes graph";

  // Now use the trace node ID to locate the corresponding stack.
  std::map<int, ChromeStackTreeNode *> idNodeMap;
  auto roots =
      ChromeStackTreeNode::parse(rt.get(), traceTree, nullptr, idNodeMap);
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

#endif // HERMES_MEMORY_INSTRUMENTATION
