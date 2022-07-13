/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_MEMORY_INSTRUMENTATION

#include "hermes/VM/HeapSnapshot.h"
#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/Allocator.h"
#include "hermes/Support/Compiler.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/SymbolID.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/raw_ostream.h"

#include <chrono>
#include <set>
#include <sstream>

using namespace hermes::vm;
using namespace hermes::parser;

namespace hermes {
namespace unittest {
namespace heapsnapshottest {

using vm::testhelpers::DummyObject;

struct Node {
  HeapSnapshot::NodeType type;
  std::string name;
  HeapSnapshot::NodeID id;
  size_t selfSize;
  size_t edgeCount;
  size_t traceNodeID;
  int detachedness;

  Node() = default;

  explicit Node(
      HeapSnapshot::NodeType type,
      std::string name,
      HeapSnapshot::NodeID id,
      size_t selfSize,
      size_t edgeCount,
      size_t traceNodeID = 0,
      int detachedness = 0)
      : type{type},
        name{std::move(name)},
        id{id},
        selfSize{selfSize},
        edgeCount{edgeCount},
        traceNodeID{traceNodeID},
        detachedness{detachedness} {}

  static Node parse(JSONArray::iterator nodes, const JSONArray &strings) {
    // Need two levels of cast for enums because Windows complains about casting
    // doubles to enums.
    auto type = static_cast<HeapSnapshot::NodeType>(
        static_cast<unsigned>(llvh::cast<JSONNumber>(*nodes)->getValue()));
    nodes++;

    std::string name = llvh::cast<JSONString>(
                           strings[llvh::cast<JSONNumber>(*nodes)->getValue()])
                           ->str();
    nodes++;

    auto id = static_cast<HeapSnapshot::NodeID>(
        llvh::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto selfSize =
        static_cast<size_t>(llvh::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto edgeCount =
        static_cast<size_t>(llvh::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto traceNodeID =
        static_cast<size_t>(llvh::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto detachedness =
        static_cast<int>(llvh::cast<JSONNumber>(*nodes)->getValue());

    return Node{
        type,
        std::move(name),
        id,
        selfSize,
        edgeCount,
        traceNodeID,
        detachedness};
  }

  bool operator==(const Node &that) const {
    return type == that.type && name == that.name && id == that.id &&
        selfSize == that.selfSize && edgeCount == that.edgeCount &&
        traceNodeID == that.traceNodeID && detachedness == that.detachedness;
  }

  bool operator<(const Node &that) const {
    // Just IDs for comparison.
    return id < that.id;
  }
};

std::ostream &operator<<(std::ostream &os, const Node &node);

std::ostream &operator<<(std::ostream &os, const Node &node) {
  return os << "Node{type=" << HeapSnapshot::nodeTypeToName(node.type)
            << ", name=\"" << node.name << "\", id=" << node.id
            << ", selfSize=" << node.selfSize
            << ", edgeCount=" << node.edgeCount
            << ", traceNodeID=" << node.traceNodeID
            << ", detachedness=" << node.detachedness << "}";
}

struct Edge {
  HeapSnapshot::EdgeType type;
  bool isNamed;
  std::string name;
  int index;
  HeapSnapshot::NodeID toNode;

  Edge() = default;

  explicit Edge(
      HeapSnapshot::EdgeType type,
      std::string name,
      HeapSnapshot::NodeID toNode)
      : type{type},
        isNamed{true},
        name{std::move(name)},
        index{-1},
        toNode{toNode} {}

  explicit Edge(
      HeapSnapshot::EdgeType type,
      int index,
      HeapSnapshot::NodeID toNode)
      : type{type}, isNamed{false}, name{}, index{index}, toNode{toNode} {}

  static Edge parse(
      JSONArray::iterator edges,
      const JSONArray &nodes,
      const JSONArray &strings) {
    Edge edge;
    // Need two levels of cast for enums because Windows complains about casting
    // doubles to enums.
    edge.type = static_cast<HeapSnapshot::EdgeType>(
        static_cast<unsigned>(llvh::cast<JSONNumber>(*edges)->getValue()));
    ++edges;
    switch (edge.type) {
      case HeapSnapshot::EdgeType::Context:
      case HeapSnapshot::EdgeType::Internal:
      case HeapSnapshot::EdgeType::Property:
      case HeapSnapshot::EdgeType::Shortcut:
      case HeapSnapshot::EdgeType::Weak:
        edge.isNamed = true;
        edge.name = llvh::cast<JSONString>(
                        strings[llvh::cast<JSONNumber>(*edges)->getValue()])
                        ->str();
        edge.index = -1;
        break;
      default:
        edge.isNamed = false;
        // Leave name as the empty string.
        edge.index = llvh::cast<JSONNumber>(*edges)->getValue();
        break;
    }
    ++edges;

    uint32_t toNode = llvh::cast<JSONNumber>(*edges)->getValue();
    assert(
        toNode % HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT == 0 &&
        "Invalid to node pointer");
    assert(toNode < nodes.size() && "Out-of-bounds node from edge");
    edge.toNode = Node::parse(nodes.begin() + toNode, strings).id;
    return edge;
  }

  bool operator==(const Edge &that) const {
    return type == that.type && isNamed == that.isNamed && name == that.name &&
        index == that.index && toNode == that.toNode;
  }

  bool operator<(const Edge &that) const {
    // Just toNode for comparison.
    return toNode < that.toNode;
  }
};

std::ostream &operator<<(std::ostream &os, const Edge &edge);

std::ostream &operator<<(std::ostream &os, const Edge &edge) {
  std::ios_base::fmtflags f(os.flags());
  os << std::boolalpha
     << "Edge{type=" << HeapSnapshot::edgeTypeToName(edge.type)
     << ", isNamed=" << edge.isNamed << ", name=\"" << edge.name
     << "\", index=" << edge.index << ", toNode=" << edge.toNode << "}";
  // Reset original flags to remove the boolalpha.
  os.flags(f);
  return os;
}

struct Sample {
  std::chrono::microseconds timestamp;
  HeapSnapshot::NodeID lastSeenObjectID;

  explicit Sample(
      std::chrono::microseconds timestamp,
      HeapSnapshot::NodeID lastSeenObjectID)
      : timestamp(timestamp), lastSeenObjectID(lastSeenObjectID) {}

  static Sample parse(JSONArray::iterator samples) {
    std::chrono::microseconds timestamp{
        static_cast<uint64_t>(llvh::cast<JSONNumber>(*samples)->getValue())};
    samples++;

    auto lastSeenObjectID = static_cast<HeapSnapshot::NodeID>(
        llvh::cast<JSONNumber>(*samples)->getValue());
    samples++;

    return Sample{timestamp, lastSeenObjectID};
  }
};

struct Location {
  Node object;
  facebook::hermes::debugger::ScriptID scriptID;
  int line;
  int column;

  Location() = default;

  explicit Location(
      Node object,
      facebook::hermes::debugger::ScriptID scriptID,
      int line,
      int column)
      : object{object}, scriptID{scriptID}, line{line}, column{column} {}

  static Location parse(
      JSONArray::iterator locations,
      const JSONArray &nodes,
      const JSONArray &strings) {
    Location loc;

    size_t objectIndex =
        static_cast<size_t>(llvh::cast<JSONNumber>(*locations)->getValue());
    loc.object = Node::parse(nodes.begin() + objectIndex, strings);
    locations++;

    loc.scriptID = llvh::cast<JSONNumber>(*locations)->getValue();
    locations++;

    // Line numbers and column numbers are 0-based internally,
    // but 1-based when viewed.
    loc.line = llvh::cast<JSONNumber>(*locations)->getValue() + 1;
    locations++;
    loc.column = llvh::cast<JSONNumber>(*locations)->getValue() + 1;
    locations++;

    return loc;
  }

  bool operator==(const Location &that) const {
    return object == that.object && scriptID == that.scriptID &&
        line == that.line && column == that.column;
  }
};

std::ostream &operator<<(std::ostream &os, const Location &loc);

std::ostream &operator<<(std::ostream &os, const Location &loc) {
  return os << "Location{object=" << loc.object << ", scriptID=" << loc.scriptID
            << ", line=" << loc.line << ", column=" << loc.column << "}";
}

static ::testing::AssertionResult testListOfStrings(
    JSONArray::iterator begin,
    JSONArray::iterator end,
    std::initializer_list<llvh::StringRef> strs) {
  EXPECT_EQ(static_cast<unsigned long>(end - begin), strs.size());
  auto strsIt = strs.begin();
  for (auto it = begin; it != end; ++it) {
    EXPECT_EQ(llvh::cast<JSONString>(*it)->str(), *strsIt);
    ++strsIt;
  }
  return ::testing::AssertionSuccess();
}

static ::testing::AssertionResult testListOfStrings(
    const JSONArray &arr,
    std::initializer_list<llvh::StringRef> strs) {
  return testListOfStrings(arr.begin(), arr.end(), strs);
}

static Node findNodeForID(
    HeapSnapshot::NodeID id,
    const JSONArray &nodes,
    const JSONArray &strings,
    const char *file,
    int line) {
  for (auto it = nodes.begin(), e = nodes.end(); it != e;
       it += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT) {
    auto node = Node::parse(it, strings);
    if (id == node.id) {
      return node;
    }
  }
  ADD_FAILURE_AT(file, line) << "No node found for id " << id;
  return Node{};
}

static std::pair<Node, std::vector<Edge>> findNodeAndEdgesForID(
    HeapSnapshot::NodeID id,
    const JSONArray &nodes,
    const JSONArray &edges,
    const JSONArray &strings,
    const char *file,
    int line) {
  auto nodesIt = nodes.begin();
  const auto nodesEnd = nodes.end();
  auto edgesIt = edges.begin();
  while (nodesIt != nodesEnd) {
    auto node = Node::parse(nodesIt, strings);
    const auto nextNodesEdges =
        edgesIt + (HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT * node.edgeCount);
    if (id == node.id) {
      std::vector<Edge> retEdges;
      for (; edgesIt != nextNodesEdges;
           edgesIt += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT) {
        assert(edgesIt != edges.end() && "Edges shouldn't roll off the end");
        retEdges.emplace_back(Edge::parse(edgesIt, nodes, strings));
      }
      return {node, retEdges};
    }
    nodesIt += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
    edgesIt = nextNodesEdges;
  }
  ADD_FAILURE_AT(file, line) << "No node found for id " << id;
  return {Node{}, std::vector<Edge>{}};
}

#define FIND_NODE_FOR_ID(...) findNodeForID(__VA_ARGS__, __FILE__, __LINE__)
#define FIND_NODE_AND_EDGES_FOR_ID(...) \
  findNodeAndEdgesForID(__VA_ARGS__, __FILE__, __LINE__)

static Location findLocationForID(
    HeapSnapshot::NodeID id,
    const JSONArray &locations,
    const JSONArray &nodes,
    const JSONArray &strings,
    const char *file,
    int line) {
  for (auto it = locations.begin(), e = locations.end(); it != e;
       it += HeapSnapshot::V8_SNAPSHOT_LOCATION_FIELD_COUNT) {
    Location loc = Location::parse(it, nodes, strings);
    if (loc.object.id == id) {
      return loc;
    }
  }
  ADD_FAILURE_AT(file, line) << "Location for id " << id << " not found";
  return Location{};
}

#define FIND_LOCATION_FOR_ID(...) \
  findLocationForID(__VA_ARGS__, __FILE__, __LINE__)

static JSONObject *parseSnapshot(
    const std::string &json,
    JSONFactory &factory,
    const char *file,
    int line) {
  if (json.empty()) {
    ADD_FAILURE_AT(file, line) << "Snapshot wasn't written out";
    return nullptr;
  }

  SourceErrorManager sm;
  JSONParser parser{factory, json, sm};
  auto optSnapshot = parser.parse();
  if (!optSnapshot) {
    ADD_FAILURE_AT(file, line) << "Snapshot isn't valid JSON";
    return nullptr;
  }
  JSONValue *root = optSnapshot.getValue();
  if (!llvh::isa<JSONObject>(root)) {
    ADD_FAILURE_AT(file, line) << "Snapshot isn't a JSON object";
    return nullptr;
  }

  return llvh::cast<JSONObject>(root);
}

static JSONObject *
takeSnapshot(GC &gc, JSONFactory &factory, const char *file, int line) {
  std::string result("");
  llvh::raw_string_ostream str(result);
  gc.collect("test");
  gc.createSnapshot(str);
  str.flush();
  return parseSnapshot(result, factory, file, line);
}

#define PARSE_SNAPSHOT(...) parseSnapshot(__VA_ARGS__, __FILE__, __LINE__)
#define TAKE_SNAPSHOT(...) takeSnapshot(__VA_ARGS__, __FILE__, __LINE__)

TEST(HeapSnapshotTest, IDReversibleTest) {
  // Make sure an ID <-> Object mapping is preserved across collections.
  auto runtime = DummyRuntime::create(GCConfig::Builder()
                                          .withInitHeapSize(1024)
                                          .withMaxHeapSize(1024 * 100)
                                          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();
  GCScope gcScope(rt);

  // Make a dummy object.
  auto obj = rt.makeHandle(DummyObject::create(gc, rt));
  const auto objID = gc.getObjectID(obj.get());
  // Make sure the ID can be translated back to the object pointer.
  EXPECT_EQ(obj.get(), gc.getObjectForID(objID));
  // Run a collection to move things around.
  gc.collect("test");
  // Test that the ID is the same and it can be reversed.
  EXPECT_EQ(objID, gc.getObjectID(obj.get()));
  EXPECT_EQ(obj.get(), gc.getObjectForID(objID));
}

TEST(HeapSnapshotTest, HeaderTest) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  auto runtime = DummyRuntime::create(GCConfig::Builder()
                                          .withInitHeapSize(1024)
                                          .withMaxHeapSize(1024 * 100)
                                          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();

  JSONObject *root = TAKE_SNAPSHOT(gc, jsonFactory);
  ASSERT_TRUE(root != nullptr);

  JSONObject *snapshot = llvh::cast<JSONObject>(root->at("snapshot"));

  EXPECT_EQ(llvh::cast<JSONNumber>(snapshot->at("node_count"))->getValue(), 0);
  EXPECT_EQ(llvh::cast<JSONNumber>(snapshot->at("edge_count"))->getValue(), 0);
  EXPECT_EQ(
      llvh::cast<JSONNumber>(snapshot->at("trace_function_count"))->getValue(),
      0);

  JSONObject *meta = llvh::cast<JSONObject>(snapshot->at("meta"));
  EXPECT_EQ(llvh::cast<JSONArray>(meta->at("sample_fields"))->size(), 2);

  JSONArray &nodeFields = *llvh::cast<JSONArray>(meta->at("node_fields"));
  JSONArray &nodeTypes = *llvh::cast<JSONArray>(meta->at("node_types"));
  JSONArray &edgeFields = *llvh::cast<JSONArray>(meta->at("edge_fields"));
  JSONArray &edgeTypes = *llvh::cast<JSONArray>(meta->at("edge_types"));
  JSONArray &traceFunctionInfoFields =
      *llvh::cast<JSONArray>(meta->at("trace_function_info_fields"));
  JSONArray &traceNodeFields =
      *llvh::cast<JSONArray>(meta->at("trace_node_fields"));
  JSONArray &locationFields =
      *llvh::cast<JSONArray>(meta->at("location_fields"));
  JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // Check that node_fields/types are correct.
  EXPECT_TRUE(testListOfStrings(
      nodeFields,
      {"type",
       "name",
       "id",
       "self_size",
       "edge_count",
       "trace_node_id",
       "detachedness"}));
  const JSONArray &nodeTypeEnum = *llvh::cast<JSONArray>(nodeTypes[0]);
  EXPECT_TRUE(testListOfStrings(
      nodeTypeEnum,
      {"hidden",
       "array",
       "string",
       "object",
       "code",
       "closure",
       "regexp",
       "number",
       "native",
       "synthetic",
       "concatenated string",
       "sliced string",
       "symbol",
       "bigint"}));
  EXPECT_TRUE(testListOfStrings(
      nodeTypes.begin() + 1,
      nodeTypes.end(),
      {"string", "number", "number", "number", "number", "number"}));
  // Check that edge_fields/types are correct.
  EXPECT_TRUE(
      testListOfStrings(edgeFields, {"type", "name_or_index", "to_node"}));
  const JSONArray &edgeTypeEnum = *llvh::cast<JSONArray>(edgeTypes[0]);
  EXPECT_TRUE(testListOfStrings(
      edgeTypeEnum,
      {"context",
       "element",
       "property",
       "internal",
       "hidden",
       "shortcut",
       "weak"}));
  EXPECT_TRUE(testListOfStrings(
      edgeTypes.begin() + 1, edgeTypes.end(), {"string_or_number", "node"}));
  EXPECT_TRUE(testListOfStrings(
      traceFunctionInfoFields,
      {"function_id", "name", "script_name", "script_id", "line", "column"}));
  EXPECT_TRUE(testListOfStrings(
      traceNodeFields,
      {"id", "function_info_index", "count", "size", "children"}));
  EXPECT_TRUE(testListOfStrings(
      locationFields, {"object_index", "script_id", "line", "column"}));

  // Test the basic root nodes.
  Node superRoot{
      HeapSnapshot::NodeType::Synthetic,
      "",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::SuperRoot),
      0,
      1};
  Node gcRoots{
      HeapSnapshot::NodeType::Synthetic,
      "(GC roots)",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::GCRoots),
      0,
      1};
  Node customRoots{
      HeapSnapshot::NodeType::Synthetic,
      "(Custom)",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::Custom),
      0,
      0};
  EXPECT_EQ(
      FIND_NODE_AND_EDGES_FOR_ID(superRoot.id, nodes, edges, strings),
      std::make_pair(
          superRoot,
          std::vector<Edge>{
              Edge{HeapSnapshot::EdgeType::Element, 1, gcRoots.id}}));
  // Don't test edges here because they contain GC-specific nodes.
  auto actualGCRootsNode = FIND_NODE_FOR_ID(gcRoots.id, nodes, strings);
  // Since each individual GC can choose what edges
  // exist, don't test the edge count.
  gcRoots.edgeCount = actualGCRootsNode.edgeCount;
  EXPECT_EQ(actualGCRootsNode, gcRoots);
  EXPECT_EQ(
      FIND_NODE_AND_EDGES_FOR_ID(customRoots.id, nodes, edges, strings),
      std::make_pair(customRoots, std::vector<Edge>{}));
}

TEST(HeapSnapshotTest, TestNodesAndEdgesForDummyObjects) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  auto runtime = DummyRuntime::create(GCConfig::Builder()
                                          .withInitHeapSize(1024)
                                          .withMaxHeapSize(1024 * 100)
                                          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();
  GCScope gcScope(rt);

  auto dummy = rt.makeHandle(DummyObject::create(gc, rt));
  auto *dummy2 = DummyObject::create(gc, rt);
  dummy->setPointer(gc, dummy2);
  const auto blockSize = dummy->getAllocatedSize();

  JSONObject *root = TAKE_SNAPSHOT(gc, jsonFactory);
  ASSERT_TRUE(root != nullptr);

  // Check the nodes and edges.
  JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  EXPECT_EQ(llvh::cast<JSONArray>(root->at("trace_function_infos"))->size(), 0);
  EXPECT_EQ(llvh::cast<JSONArray>(root->at("trace_tree"))->size(), 0);
  EXPECT_EQ(llvh::cast<JSONArray>(root->at("samples"))->size(), 0);
  EXPECT_EQ(llvh::cast<JSONArray>(root->at("locations"))->size(), 0);

  Node firstDummy{
      HeapSnapshot::NodeType::Object,
      cellKindStr(dummy->getKind()),
      gc.getObjectID(dummy.get()),
      blockSize,
      // One edge to the second dummy, 4 for primitive singletons, and a WeakRef
      // to self.
      6};
  Node undefinedNode{
      HeapSnapshot::NodeType::Object,
      "undefined",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::Undefined),
      0,
      0};
  Node nullNode{
      HeapSnapshot::NodeType::Object,
      "null",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::Null),
      0,
      0};
  Node trueNode(
      HeapSnapshot::NodeType::Object,
      "true",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::True),
      0,
      0);
  Node numberNode{
      HeapSnapshot::NodeType::Number,
      "3.14",
      gc.getIDTracker().getNumberID(dummy->hvDouble.getNumber()),
      0,
      0};
  Node falseNode{
      HeapSnapshot::NodeType::Object,
      "false",
      GC::IDTracker::reserved(GC::IDTracker::ReservedObjectID::False),
      0,
      0};
  Node secondDummy{
      HeapSnapshot::NodeType::Object,
      cellKindStr(dummy->getKind()),
      gc.getObjectID(dummy->other),
      blockSize,
      // No edges except for the primitive singletons and the WeakRef to self.
      5};

  // Common edges.
  Edge trueEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesBool", trueNode.id);
  Edge numberEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesDouble", numberNode.id);
  Edge undefinedEdge = Edge(
      HeapSnapshot::EdgeType::Internal, "HermesUndefined", undefinedNode.id);
  Edge nullEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesNull", nullNode.id);

  // Two dummy objects.
  EXPECT_EQ(
      FIND_NODE_AND_EDGES_FOR_ID(firstDummy.id, nodes, edges, strings),
      std::make_pair(
          firstDummy,
          std::vector<Edge>{
              Edge{HeapSnapshot::EdgeType::Internal, "other", secondDummy.id},
              trueEdge,
              numberEdge,
              undefinedEdge,
              nullEdge,
              Edge{HeapSnapshot::EdgeType::Weak, "0", firstDummy.id}}));

  EXPECT_EQ(
      FIND_NODE_AND_EDGES_FOR_ID(secondDummy.id, nodes, edges, strings),
      std::make_pair(
          secondDummy,
          std::vector<Edge>{
              trueEdge,
              numberEdge,
              undefinedEdge,
              nullEdge,
              Edge{HeapSnapshot::EdgeType::Weak, "0", secondDummy.id}}));
}

TEST(HeapSnapshotTest, SnapshotFromCallbackContext) {
  bool triggeredTripwire = false;
  std::ostringstream stream;
  auto runtime = DummyRuntime::create(
      kTestGCConfigSmall.rebuild()
          .withTripwireConfig(GCTripwireConfig::Builder()
                                  .withLimit(32)
                                  .withCallback([&triggeredTripwire, &stream](
                                                    GCTripwireContext &ctx) {
                                    triggeredTripwire = true;
                                    ctx.createSnapshot(stream);
                                  })
                                  .build())
          .build());
  DummyRuntime &rt = *runtime;
  GCScope scope{rt};
  auto dummy = rt.makeHandle(DummyObject::create(rt.getHeap(), rt));
  const auto dummyID = rt.getHeap().getObjectID(dummy.get());
  rt.collect();
  ASSERT_TRUE(triggeredTripwire);

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  JSONObject *root = PARSE_SNAPSHOT(stream.str(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // Check that the dummy object is in the snapshot.
  auto dummyNode = FIND_NODE_FOR_ID(dummyID, nodes, strings);
  Node expected{
      HeapSnapshot::NodeType::Object,
      "DummyObject",
      dummyID,
      dummy->getAllocatedSize(),
      5};
  EXPECT_EQ(dummyNode, expected);
}

using HeapSnapshotRuntimeTest = RuntimeTestFixture;

template <typename T>
size_t firstNamedPropertyEdge() {
  // parent, __proto__, class, directProp$i
  return JSObject::DIRECT_PROPERTY_SLOTS - JSObject::numOverlapSlots<T>() + 3;
}

TEST_F(HeapSnapshotRuntimeTest, FunctionLocationForLazyCode) {
  // Similar test to the above, but for lazy-compiled source.
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};

  hbc::CompileFlags flags;
  flags.debug = true;
  flags.lazy = true;

  // Build a function that will be lazily compiled.
  std::string source = "    function myGlobal() { ";
  for (int i = 0; i < 100; i++)
    source += " Math.random(); ";
  source += "};\nmyGlobal;";

  CallResult<HermesValue> res = runtime.run(source, "file:///fake.js", flags);
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> func = runtime.makeHandle(vmcast<JSFunction>(*res));
  const auto funcID = runtime.getHeap().getObjectID(func.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // This test requires a location to be emitted.
  auto node = FIND_NODE_FOR_ID(funcID, nodes, strings);
  Node expected{
      HeapSnapshot::NodeType::Closure,
      "myGlobal",
      funcID,
      func->getAllocatedSize(),
      firstNamedPropertyEdge<JSFunction>() + 3};
  EXPECT_EQ(node, expected);
  // Edges aren't tested in this test.

#ifdef HERMES_ENABLE_DEBUGGER
  // The location isn't emitted in fully optimized builds.
  const JSONArray &locations = *llvh::cast<JSONArray>(root->at("locations"));
  Location loc = FIND_LOCATION_FOR_ID(funcID, locations, nodes, strings);
  // The location should be the given file, at line 1 column 5 with indenting
  auto scriptId = func->getRuntimeModule(runtime)->getScriptID();
  EXPECT_EQ(loc, Location(expected, scriptId, 1, 5));
#else
  (void)findLocationForID;
#endif
}

TEST_F(HeapSnapshotRuntimeTest, FunctionLocationAndNameTest) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hbc::CompileFlags flags;
  // Make sure that debug info is emitted for this source file when it's
  // compiled.
  flags.debug = true;
  // Indent the function slightly to test that the source location is correct
  CallResult<HermesValue> res =
      runtime.run("\n  function foo() {}; foo;", "file:///fake.js", flags);
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> func = runtime.makeHandle(vmcast<JSFunction>(*res));
  const auto funcID = runtime.getHeap().getObjectID(func.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // This test requires a location to be emitted.
  auto node = FIND_NODE_FOR_ID(funcID, nodes, strings);
  Node expected{
      HeapSnapshot::NodeType::Closure,
      "foo",
      funcID,
      func->getAllocatedSize(),
      firstNamedPropertyEdge<JSFunction>() + 3};
  EXPECT_EQ(node, expected);
  // Edges aren't tested in this test.

#ifdef HERMES_ENABLE_DEBUGGER
  // The location isn't emitted in fully optimized builds.
  const JSONArray &locations = *llvh::cast<JSONArray>(root->at("locations"));
  Location loc = FIND_LOCATION_FOR_ID(funcID, locations, nodes, strings);
  // The location should be the given file, second line, third column
  auto scriptId = func->getRuntimeModule(runtime)->getScriptID();
  EXPECT_EQ(loc, Location(expected, scriptId, 2, 3));
#else
  (void)findLocationForID;
#endif
}

TEST_F(HeapSnapshotRuntimeTest, FunctionDisplayNameTest) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hbc::CompileFlags flags;
  // Make sure that debug info is emitted for this source file when it's
  // compiled.
  flags.debug = true;
  CallResult<HermesValue> res = runtime.run(
      R"(function foo() {}; foo.displayName = "bar"; foo;)",
      "file:///fake.js",
      flags);
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> func = runtime.makeHandle(vmcast<JSFunction>(*res));
  const auto funcID = runtime.getHeap().getObjectID(func.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  auto node = FIND_NODE_FOR_ID(funcID, nodes, strings);
  Node expected{
      HeapSnapshot::NodeType::Closure,
      // Make sure the name that is reported is "bar", not "foo".
      "bar",
      funcID,
      func->getAllocatedSize(),
      firstNamedPropertyEdge<JSFunction>() + 8};
  EXPECT_EQ(node, expected);
}

TEST_F(HeapSnapshotRuntimeTest, WeakMapTest) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  auto mapResult = JSWeakMap::create(
      runtime, Handle<JSObject>::vmcast(&runtime.weakMapPrototype));
  ASSERT_FALSE(isException(mapResult));
  Handle<JSWeakMap> map = runtime.makeHandle(std::move(*mapResult));
  Handle<JSObject> key = runtime.makeHandle(JSObject::create(runtime));
  Handle<JSObject> value = runtime.makeHandle(JSObject::create(runtime));
  // Add a key so the DenseMap will exist.
  ASSERT_FALSE(isException(JSWeakMap::setValue(map, runtime, key, value)));

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);
  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  const auto mapID = runtime.getHeap().getObjectID(map.get());
  auto nodesAndEdges = FIND_NODE_AND_EDGES_FOR_ID(mapID, nodes, edges, strings);
  auto firstNamed = firstNamedPropertyEdge<JSWeakMap>();
  EXPECT_EQ(
      nodesAndEdges.first,
      Node(
          HeapSnapshot::NodeType::Object,
          "JSWeakMap",
          mapID,
          map->getAllocatedSize(),
          firstNamed + 3));
  EXPECT_EQ(nodesAndEdges.second.size(), firstNamed + 3);

  // Test the weak edge.
  EXPECT_EQ(
      nodesAndEdges.second[firstNamed],
      Edge(
          HeapSnapshot::EdgeType::Weak,
          "0",
          runtime.getHeap().getObjectID(key.get())));
  // Test the native edge.
  const auto nativeMapID = map->getMapID(runtime.getHeap());
  EXPECT_EQ(
      nodesAndEdges.second[firstNamed + 2],
      Edge(HeapSnapshot::EdgeType::Internal, "map", nativeMapID));
  EXPECT_EQ(
      FIND_NODE_FOR_ID(nativeMapID, nodes, strings),
      Node(
          HeapSnapshot::NodeType::Native,
          "DenseMap",
          nativeMapID,
          map->getMallocSize(),
          0));
}

TEST_F(HeapSnapshotRuntimeTest, PropertyUpdatesTest) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  Handle<JSObject> obj = runtime.makeHandle(JSObject::create(runtime));
  SymbolID fooSym, barSym;
  {
    vm::GCScope gcScope(runtime);
    fooSym = vm::stringToSymbolID(
                 runtime, vm::StringPrimitive::createNoThrow(runtime, "foo"))
                 ->getHermesValue()
                 .getSymbol();
    barSym = vm::stringToSymbolID(
                 runtime, vm::StringPrimitive::createNoThrow(runtime, "bar"))
                 ->getHermesValue()
                 .getSymbol();
  }
  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  // Add two properties to the hidden class chain.
  ASSERT_FALSE(isException(JSObject::defineOwnProperty(
      obj,
      runtime,
      fooSym,
      dpf,
      runtime.makeHandle(HermesValue::encodeNumberValue(100)))));
  ASSERT_FALSE(isException(JSObject::defineOwnProperty(
      obj,
      runtime,
      barSym,
      dpf,
      runtime.makeHandle(HermesValue::encodeNumberValue(200)))));
  // Trigger update transitions for both properties.
  dpf.writable = false;
  ASSERT_FALSE(isException(JSObject::defineOwnProperty(
      obj,
      runtime,
      fooSym,
      dpf,
      runtime.makeHandle(HermesValue::encodeNumberValue(100)))));
  ASSERT_FALSE(isException(JSObject::defineOwnProperty(
      obj,
      runtime,
      barSym,
      dpf,
      runtime.makeHandle(HermesValue::encodeNumberValue(200)))));
  // Forcibly clear the final hidden class's property map.
  auto *clazz = obj->getClass(runtime);
  clazz->clearPropertyMap(runtime.getHeap());

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);
  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  const auto objID = runtime.getHeap().getObjectID(obj.get());
  auto nodesAndEdges = FIND_NODE_AND_EDGES_FOR_ID(objID, nodes, edges, strings);

  const auto FIRST_NAMED_PROPERTY_EDGE = firstNamedPropertyEdge<JSObject>();

  EXPECT_EQ(
      nodesAndEdges.first,
      Node(
          HeapSnapshot::NodeType::Object,
          "JSObject(foo, bar)",
          objID,
          obj->getAllocatedSize(),
          FIRST_NAMED_PROPERTY_EDGE + 2));
  EXPECT_EQ(nodesAndEdges.second.size(), FIRST_NAMED_PROPERTY_EDGE + 2);

  EXPECT_EQ(
      nodesAndEdges.second[FIRST_NAMED_PROPERTY_EDGE],
      Edge(
          HeapSnapshot::EdgeType::Property,
          "foo",
          runtime.getHeap().getIDTracker().getNumberID(100)));

  EXPECT_EQ(
      nodesAndEdges.second[FIRST_NAMED_PROPERTY_EDGE + 1],
      Edge(
          HeapSnapshot::EdgeType::Property,
          "bar",
          runtime.getHeap().getIDTracker().getNumberID(200)));
}

TEST_F(HeapSnapshotRuntimeTest, ArrayElements) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hbc::CompileFlags flags;
  // Build an array that doesn't start at index 0.
  std::string source =
      "var a = []; a[10] = {}; a[15] = {}; a[(1 << 20) + 1000] = {}; a";
  CallResult<HermesValue> res = runtime.run(source, "file:///fake.js", flags);
  ASSERT_FALSE(isException(res));
  Handle<JSArray> array = runtime.makeHandle(vmcast<JSArray>(*res));
  Handle<JSObject> firstElement = runtime.makeHandle<JSObject>(
      vmcast<JSObject>(array->at(runtime, 10).getObject(runtime)));
  Handle<JSObject> secondElement = runtime.makeHandle(
      vmcast<JSObject>(array->at(runtime, 15).getObject(runtime)));
  auto cr = JSObject::getComputed_RJS(
      array,
      runtime,
      runtime.makeHandle(HermesValue::encodeNumberValue((1 << 20) + 1000)));
  ASSERT_FALSE(isException(cr));
  Handle<JSObject> thirdElement = runtime.makeHandle<JSObject>(std::move(*cr));
  const auto arrayID = runtime.getHeap().getObjectID(array.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  const auto FIRST_NAMED_PROPERTY_EDGE = firstNamedPropertyEdge<JSArray>();
  auto nodeAndEdges =
      FIND_NODE_AND_EDGES_FOR_ID(arrayID, nodes, edges, strings);
  EXPECT_EQ(
      nodeAndEdges.first,
      Node(
          HeapSnapshot::NodeType::Object,
          "JSArray",
          arrayID,
          array->getAllocatedSize(),
          FIRST_NAMED_PROPERTY_EDGE + 6));
  // The last edges are the element edges.
  EXPECT_EQ(
      nodeAndEdges.second[FIRST_NAMED_PROPERTY_EDGE + 2],
      Edge(
          HeapSnapshot::EdgeType::Element,
          (1 << 20) + 1000,
          runtime.getHeap().getObjectID(thirdElement.get())));
  EXPECT_EQ(
      nodeAndEdges.second[FIRST_NAMED_PROPERTY_EDGE + 4],
      Edge(
          HeapSnapshot::EdgeType::Element,
          10,
          runtime.getHeap().getObjectID(firstElement.get())));
  EXPECT_EQ(
      nodeAndEdges.second[FIRST_NAMED_PROPERTY_EDGE + 5],
      Edge(
          HeapSnapshot::EdgeType::Element,
          15,
          runtime.getHeap().getObjectID(secondElement.get())));
}

#ifdef HERMES_ENABLE_DEBUGGER

static HeapSnapshot::NodeID findHighestNodeID(
    const JSONArray &nodes,
    const JSONArray &strings) {
  HeapSnapshot::NodeID maxID = GCBase::IDTracker::kInvalidNode;
  for (auto it = nodes.begin(), e = nodes.end(); it != e;
       it += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT) {
    auto node = Node::parse(it, strings);
    if (node.id > maxID) {
      maxID = node.id;
    }
  }
  return maxID;
}

static std::string functionInfoToString(
    int idx,
    const JSONArray &traceFunctionInfos,
    const JSONArray &strings) {
  auto base = idx * 6;
  int functionID = llvh::cast<JSONNumber>(traceFunctionInfos[base])->getValue();
  assert(
      functionID == idx &&
      "The function info must have a matching index and id");

  auto name = llvh::cast<JSONString>(
                  strings[llvh::cast<JSONNumber>(traceFunctionInfos[base + 1])
                              ->getValue()])
                  ->str();

  auto scriptName =
      llvh::cast<JSONString>(
          strings[llvh::cast<JSONNumber>(traceFunctionInfos[base + 2])
                      ->getValue()])
          ->str();

  auto scriptID =
      llvh::cast<JSONNumber>(traceFunctionInfos[base + 3])->getValue();
  auto line = llvh::cast<JSONNumber>(traceFunctionInfos[base + 4])->getValue();
  auto col = llvh::cast<JSONNumber>(traceFunctionInfos[base + 5])->getValue();

  return std::string(name) + "(" + std::to_string((int)functionID) + ") @ " +
      std::string(scriptName) + "(" + std::to_string((int)scriptID) +
      "):" + std::to_string((int)line) + ":" + std::to_string((int)col);
}

struct ChromeStackTreeNode {
  ChromeStackTreeNode(ChromeStackTreeNode *parent, int traceFunctionInfosId)
      : parent_(parent), traceFunctionInfosId_(traceFunctionInfosId) {}

  static std::vector<std::unique_ptr<ChromeStackTreeNode>> parse(
      const JSONArray &traceNodes,
      ChromeStackTreeNode *parent,
      std::map<int, ChromeStackTreeNode *> &idNodeMap) {
    std::vector<std::unique_ptr<ChromeStackTreeNode>> res;
    if (!parent) {
      assert(
          traceNodes.size() == 5 &&
          "Allocation trace should only have a"
          "single root node");
    }
    for (size_t i = 0; i < traceNodes.size(); i += 5) {
      auto id = llvh::cast<JSONNumber>(traceNodes[i])->getValue();
      auto functionInfoIndex =
          llvh::cast<JSONNumber>(traceNodes[i + 1])->getValue();
      auto children = llvh::cast<JSONArray>(traceNodes[i + 4]);
      auto treeNode =
          std::make_unique<ChromeStackTreeNode>(parent, functionInfoIndex);
      idNodeMap.emplace(id, treeNode.get());
      treeNode->children_ = parse(*children, treeNode.get(), idNodeMap);
      res.emplace_back(std::move(treeNode));
    }
    return res;
  };

  std::string buildStackTrace(
      const JSONArray &traceFunctionInfos,
      const JSONArray &strings) {
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

TEST_F(HeapSnapshotRuntimeTest, AllocationTraces) {
  runtime.enableAllocationLocationTracker();
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hbc::CompileFlags flags;
  CallResult<HermesValue> res = runtime.run(
      R"#(
function foo() {
  return new Object();
}
function bar() {
  return new Object();
}
function baz() {
  return {foo: foo(), bar: bar()};
}
baz();
      )#",
      "test.js",
      flags);
  ASSERT_FALSE(isException(res));
  ASSERT_TRUE(res->isObject());
  Handle<JSObject> resObj = runtime.makeHandle(vmcast<JSObject>(*res));
  SymbolID fooSym, barSym;
  {
    vm::GCScope gcScope(runtime);
    fooSym = vm::stringToSymbolID(
                 runtime, vm::StringPrimitive::createNoThrow(runtime, "foo"))
                 ->getHermesValue()
                 .getSymbol();
    barSym = vm::stringToSymbolID(
                 runtime, vm::StringPrimitive::createNoThrow(runtime, "bar"))
                 ->getHermesValue()
                 .getSymbol();
  }
  auto fooObj = JSObject::getNamed_RJS(resObj, runtime, fooSym);
  auto barObj = JSObject::getNamed_RJS(resObj, runtime, barSym);
  auto fooObjID =
      runtime.getHeap().getObjectID(vmcast<JSObject>(fooObj->get()));
  auto barObjID =
      runtime.getHeap().getObjectID(vmcast<JSObject>(barObj->get()));

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));
  const JSONArray &traceFunctionInfos =
      *llvh::cast<JSONArray>(root->at("trace_function_infos"));

  std::map<int, ChromeStackTreeNode *> idNodeMap;
  auto roots = ChromeStackTreeNode::parse(
      *llvh::cast<JSONArray>(root->at("trace_tree")), nullptr, idNodeMap);

  auto fooAllocNode = FIND_NODE_FOR_ID(fooObjID, nodes, strings);
  auto fooStackTreeNode = idNodeMap.find(fooAllocNode.traceNodeID);
  ASSERT_NE(fooStackTreeNode, idNodeMap.end());
  auto fooStackStr =
      fooStackTreeNode->second->buildStackTrace(traceFunctionInfos, strings);
  EXPECT_STREQ(
      fooStackStr.c_str(),
      R"#(
(root)(0) @ (0):0:0
global(1) @ test.js(2):2:1
global(2) @ test.js(2):11:4
baz(7) @ test.js(2):9:19
foo(8) @ test.js(2):3:20)#");

  auto barAllocNode = FIND_NODE_FOR_ID(barObjID, nodes, strings);
  auto barStackTreeNode = idNodeMap.find(barAllocNode.traceNodeID);
  ASSERT_NE(barStackTreeNode, idNodeMap.end());
  auto barStackStr =
      barStackTreeNode->second->buildStackTrace(traceFunctionInfos, strings);
  ASSERT_STREQ(
      barStackStr.c_str(),
      R"#(
(root)(0) @ (0):0:0
global(1) @ test.js(2):2:1
global(2) @ test.js(2):11:4
baz(4) @ test.js(2):9:31
bar(5) @ test.js(2):6:20)#");

  const JSONArray &samples = *llvh::cast<JSONArray>(root->at("samples"));
  // Must have at least one sample
  EXPECT_GT(samples.size(), 0u);
  for (auto it = samples.begin(), e = samples.end(); it != e;
       it += HeapSnapshot::V8_SNAPSHOT_SAMPLE_FIELD_COUNT) {
    auto sample = Sample::parse(it);
    EXPECT_NE(sample.lastSeenObjectID, toRValue(GC::IDTracker::kInvalidNode));
    if (it != samples.begin()) {
      auto prevSample =
          Sample::parse(it - HeapSnapshot::V8_SNAPSHOT_SAMPLE_FIELD_COUNT);
      EXPECT_GT(sample.timestamp, prevSample.timestamp);
      EXPECT_GT(sample.lastSeenObjectID, prevSample.lastSeenObjectID);
    }
  }
  auto highestNodeID = findHighestNodeID(nodes, strings);
  auto lastSample = Sample::parse(
      samples.end() - HeapSnapshot::V8_SNAPSHOT_SAMPLE_FIELD_COUNT);
  EXPECT_GE(lastSample.lastSeenObjectID, highestNodeID);
}

TEST_F(HeapSnapshotRuntimeTest, TwoPathsToFunction) {
  runtime.enableAllocationLocationTracker();
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hbc::CompileFlags flags;
  CallResult<HermesValue> res = runtime.run(
      R"#(
var objects = [];
function A() {
  B(A);
}
function B(allocatingFunction) {
  objects.push({ AllocatingFunction: allocatingFunction });
}
function C() {
  B(C);
}
function D() {
  B(D);
}
A();
C();
D();
objects[0];
      )#",
      "test.js",
      flags);
  ASSERT_FALSE(isException(res));
  ASSERT_TRUE(res->isObject());
  Handle<JSObject> obj = runtime.makeHandle(vmcast<JSObject>(*res));
  auto objID = runtime.getHeap().getObjectID(*obj);

  JSONObject *root = TAKE_SNAPSHOT(runtime.getHeap(), jsonFactory);
  ASSERT_TRUE(root != nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));
  const JSONArray &traceFunctionInfos =
      *llvh::cast<JSONArray>(root->at("trace_function_infos"));

  std::map<int, ChromeStackTreeNode *> idNodeMap;
  auto roots = ChromeStackTreeNode::parse(
      *llvh::cast<JSONArray>(root->at("trace_tree")), nullptr, idNodeMap);

  auto allocNode = FIND_NODE_FOR_ID(objID, nodes, strings);
  auto stackTreeNode = idNodeMap.find(allocNode.traceNodeID);
  ASSERT_NE(stackTreeNode, idNodeMap.end());
  auto stackStr =
      stackTreeNode->second->buildStackTrace(traceFunctionInfos, strings);
  EXPECT_STREQ(
      stackStr.c_str(),
      R"#(
(root)(0) @ (0):0:0
global(1) @ test.js(2):2:1
global(10) @ test.js(2):15:2
A(11) @ test.js(2):4:4
B(4) @ test.js(2):7:15)#");
}

#endif // HERMES_ENABLE_DEBUGGER

} // namespace heapsnapshottest
} // namespace unittest
} // namespace hermes

#endif // HERMES_MEMORY_INSTRUMENTATION
