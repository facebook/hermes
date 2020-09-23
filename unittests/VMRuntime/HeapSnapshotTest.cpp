/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HeapSnapshot.h"
#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/Allocator.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/raw_ostream.h"

#include <set>
#include <sstream>

using namespace hermes::vm;
using namespace hermes::parser;

namespace hermes {
namespace unittest {
namespace heapsnapshottest {

// Forward declaration to allow IsGCObject.
struct DummyObject;

} // namespace heapsnapshottest
} // namespace unittest

namespace vm {
template <>
struct IsGCObject<unittest::heapsnapshottest::DummyObject>
    : public std::true_type {};
} // namespace vm

namespace unittest {
namespace heapsnapshottest {

struct DummyObject final : public GCCell {
  static const VTable vt;
  GCPointer<DummyObject> other;
  const uint32_t x;
  const uint32_t y;
  GCHermesValue hvBool;
  GCHermesValue hvDouble;
  GCHermesValue hvUndefined;
  GCHermesValue hvEmpty;
  GCHermesValue hvNative;
  GCHermesValue hvNull;

  DummyObject(GC *gc) : GCCell(gc, &vt), other(), x(1), y(2) {
    hvBool.setNonPtr(HermesValue::encodeBoolValue(true), gc);
    hvDouble.setNonPtr(HermesValue::encodeNumberValue(3.14), gc);
    hvNative.setNonPtr(HermesValue::encodeNativeUInt32(0xE), gc);
    hvUndefined.setNonPtr(HermesValue::encodeUndefinedValue(), gc);
    hvEmpty.setNonPtr(HermesValue::encodeEmptyValue(), gc);
    hvNull.setNonPtr(HermesValue::encodeNullValue(), gc);
  }

  void setPointer(DummyRuntime &rt, DummyObject *obj) {
    other.set(&rt, obj, &rt.gc);
  }

  static DummyObject *create(DummyRuntime &runtime) {
    return new (runtime.alloc(sizeof(DummyObject)))
        DummyObject(&runtime.getHeap());
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::UninitializedKind;
  }
};
const VTable DummyObject::vt{CellKind::UninitializedKind, sizeof(DummyObject)};

static void DummyObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const DummyObject *>(cell);
  mb.addField("HermesBool", &self->hvBool);
  mb.addField("HermesDouble", &self->hvDouble);
  mb.addField("HermesUndefined", &self->hvUndefined);
  mb.addField("HermesEmpty", &self->hvEmpty);
  mb.addField("HermesNative", &self->hvNative);
  mb.addField("HermesNull", &self->hvNull);
  mb.addField("other", &self->other);
}

static MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      buildMetadata(CellKind::UninitializedKind, DummyObjectBuildMeta)};
  return MetadataTableForTests(storage);
}

struct Node {
  HeapSnapshot::NodeType type;
  std::string name;
  HeapSnapshot::NodeID id;
  size_t selfSize;
  size_t edgeCount;
  size_t traceNodeID;

  Node() = default;

  explicit Node(
      HeapSnapshot::NodeType type,
      std::string name,
      HeapSnapshot::NodeID id,
      size_t selfSize,
      size_t edgeCount,
      size_t traceNodeID = 0)
      : type{type},
        name{std::move(name)},
        id{id},
        selfSize{selfSize},
        edgeCount{edgeCount},
        traceNodeID{traceNodeID} {}

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

    return Node{type, std::move(name), id, selfSize, edgeCount, traceNodeID};
  }

  bool operator==(const Node &that) const {
    return type == that.type && name == that.name && id == that.id &&
        selfSize == that.selfSize && edgeCount == that.edgeCount &&
        traceNodeID == that.traceNodeID;
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
            << ", traceNodeID=" << node.traceNodeID << "}";
}

struct Edge {
  HeapSnapshot::EdgeType type;
  bool isNamed;
  std::string name;
  int index;
  Node toNode;

  Edge() = default;

  explicit Edge(HeapSnapshot::EdgeType type, std::string name, Node toNode)
      : type{type},
        isNamed{true},
        name{std::move(name)},
        index{-1},
        toNode{toNode} {}

  explicit Edge(HeapSnapshot::EdgeType type, int index, Node toNode)
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
      case HeapSnapshot::EdgeType::Internal:
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
    edge.toNode = Node::parse(nodes.begin() + toNode, strings);
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

#define FIND_NODE_FOR_ID(...) findNodeForID(__VA_ARGS__, __FILE__, __LINE__)

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

TEST(HeapSnapshotTest, HeaderTest) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      GCConfig::Builder()
          .withInitHeapSize(1024)
          .withMaxHeapSize(1024 * 100)
          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  JSONObject *root = TAKE_SNAPSHOT(gc, jsonFactory);
  ASSERT_NE(root, nullptr);

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

  // Check that node_fields/types are correct.
  EXPECT_TRUE(testListOfStrings(
      nodeFields,
      {"type", "name", "id", "self_size", "edge_count", "trace_node_id"}));
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
      {"string", "number", "number", "number", "number"}));
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
}

TEST(HeapSnapshotTest, TestNodesAndEdgesForDummyObjects) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      GCConfig::Builder()
          .withInitHeapSize(1024)
          .withMaxHeapSize(1024 * 100)
          .build());
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;
  GCScope gcScope(&rt);

  auto dummy = rt.makeHandle(DummyObject::create(rt));
  auto *dummy2 = DummyObject::create(rt);
  dummy->setPointer(rt, dummy2);
  const auto blockSize = dummy->getAllocatedSize();

  JSONObject *root = TAKE_SNAPSHOT(gc, jsonFactory);
  ASSERT_NE(root, nullptr);

  // Check the nodes and edges.
  JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  JSONArray &edges = *llvh::cast<JSONArray>(root->at("edges"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  EXPECT_EQ(llvh::cast<JSONArray>(root->at("trace_function_infos"))->size(), 0);
  EXPECT_EQ(llvh::cast<JSONArray>(root->at("trace_tree"))->size(), 0);
  EXPECT_EQ(llvh::cast<JSONArray>(root->at("samples"))->size(), 0);
  EXPECT_EQ(llvh::cast<JSONArray>(root->at("locations"))->size(), 0);

  // Common nodes.
  Node rootSection{HeapSnapshot::NodeType::Synthetic,
                   "(Custom)",
                   static_cast<HeapSnapshot::NodeID>(
                       GC::IDTracker::ReservedObjectID::Custom),
                   0,
                   1};
  Node firstDummy{HeapSnapshot::NodeType::Object,
                  cellKindStr(dummy->getKind()),
                  gc.getObjectID(dummy.get()),
                  blockSize,
                  // One edge to the second dummy, 4 for primitive singletons.
                  5};
  Node undefinedNode{HeapSnapshot::NodeType::Object,
                     "undefined",
                     static_cast<HeapSnapshot::NodeID>(
                         GCBase::IDTracker::ReservedObjectID::Undefined),
                     0,
                     0};
  Node nullNode{HeapSnapshot::NodeType::Object,
                "null",
                static_cast<HeapSnapshot::NodeID>(
                    GCBase::IDTracker::ReservedObjectID::Null),
                0,
                0};
  Node trueNode(
      HeapSnapshot::NodeType::Object,
      "true",
      static_cast<HeapSnapshot::NodeID>(
          GCBase::IDTracker::ReservedObjectID::True),
      0,
      0);
  Node numberNode{HeapSnapshot::NodeType::Number,
                  "3.14",
                  gc.getIDTracker().getNumberID(dummy->hvDouble.getNumber()),
                  0,
                  0};
  Node falseNode{HeapSnapshot::NodeType::Object,
                 "false",
                 static_cast<HeapSnapshot::NodeID>(
                     GCBase::IDTracker::ReservedObjectID::False),
                 0,
                 0};
  Node secondDummy{HeapSnapshot::NodeType::Object,
                   cellKindStr(dummy->getKind()),
                   gc.getObjectID(dummy->other),
                   blockSize,
                   // No edges except for the primitive singletons.
                   4};

  // Common edges.
  Edge trueEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesBool", trueNode);
  Edge numberEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesDouble", numberNode);
  Edge undefinedEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesUndefined", undefinedNode);
  Edge nullEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesNull", nullNode);

  std::set<Node> expectedNodes;
  std::set<Edge> expectedEdges;
  std::set<Node> actualNodes;
  std::set<Edge> actualEdges;

  // First node is the roots object.
  expectedNodes.emplace(
      HeapSnapshot::NodeType::Synthetic,
      "(GC Roots)",
      static_cast<HeapSnapshot::NodeID>(GC::IDTracker::ReservedObjectID::Root),
      0,
      1);
  // Next node is the custom root section.
  expectedNodes.emplace(rootSection);
  // Next node is the first dummy object.
  expectedNodes.emplace(firstDummy);
  // Next node is the second dummy, which is only reachable via the first
  // dummy.
  expectedNodes.emplace(secondDummy);
  // Next node is the undefined singleton.
  expectedNodes.emplace(undefinedNode);
  // Next node is the null singleton.
  expectedNodes.emplace(nullNode);
  // Next node is the true singleton.
  expectedNodes.emplace(trueNode);
  // Next node is the false singleton.
  expectedNodes.emplace(falseNode);
  // Next node is the first number.
  expectedNodes.emplace(numberNode);

  // Pointer from root to root section.
  expectedEdges.emplace(HeapSnapshot::EdgeType::Element, 1, rootSection);
  // Pointer from root section to first dummy.
  expectedEdges.emplace(HeapSnapshot::EdgeType::Element, 0, firstDummy);
  // Pointer from first dummy to second dummy.
  expectedEdges.emplace(HeapSnapshot::EdgeType::Internal, "other", secondDummy);
  // Pointer from first dummy to its bool field.
  expectedEdges.emplace(trueEdge);
  // Pointer from first dummy to its number field.
  expectedEdges.emplace(numberEdge);
  // Pointer from first dummy to its undefined field.
  expectedEdges.emplace(undefinedEdge);
  // Pointer from first dummy to its null field.
  expectedEdges.emplace(nullEdge);
  // Pointer from second dummy to its bool field.
  expectedEdges.emplace(trueEdge);
  // Pointer from second dummy to its number field.
  expectedEdges.emplace(numberEdge);
  // Pointer from second dummy to its undefined field.
  expectedEdges.emplace(undefinedEdge);
  // Pointer from second dummy to its null field.
  expectedEdges.emplace(nullEdge);

  for (auto nextNode = nodes.begin(); nextNode != nodes.end();
       nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT) {
    actualNodes.emplace(Node::parse(nextNode, strings));
  }
  for (auto nextEdge = edges.begin(); nextEdge != edges.end();
       nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT) {
    actualEdges.emplace(Edge::parse(nextEdge, nodes, strings));
  }

  EXPECT_EQ(expectedNodes, actualNodes);
  EXPECT_EQ(expectedEdges, actualEdges);

  // String table is checked by the nodes and edges checks.
}

TEST(HeapSnapshotTest, SnapshotFromCallbackContext) {
  bool triggeredTripwire = false;
  std::ostringstream stream;
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
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
  GCScope scope{&rt};
  auto dummy = rt.makeHandle(DummyObject::create(rt));
  const auto dummyID = runtime->getHeap().getObjectID(dummy.get());
  rt.gc.collect("test");
  ASSERT_TRUE(triggeredTripwire);

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  JSONObject *root = PARSE_SNAPSHOT(stream.str(), jsonFactory);
  ASSERT_NE(root, nullptr);

  JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // Check that the dummy object is in the snapshot.
  auto dummyNode = FIND_NODE_FOR_ID(dummyID, nodes, strings);
  Node expected{HeapSnapshot::NodeType::Object,
                "Uninitialized",
                dummyID,
                dummy->getAllocatedSize(),
                4};
  EXPECT_EQ(dummyNode, expected);
}

using HeapSnapshotRuntimeTest = RuntimeTestFixture;

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

  CallResult<HermesValue> res = runtime->run(source, "file:///fake.js", flags);
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> func = runtime->makeHandle(vmcast<JSFunction>(*res));
  const auto funcID = runtime->getHeap().getObjectID(func.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime->getHeap(), jsonFactory);
  ASSERT_NE(root, nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // This test requires a location to be emitted.
  auto node = FIND_NODE_FOR_ID(funcID, nodes, strings);
  Node expected{HeapSnapshot::NodeType::Closure,
                "myGlobal",
                funcID,
                func->getAllocatedSize(),
                6};
  EXPECT_EQ(node, expected);
  // Edges aren't tested in this test.

#ifdef HERMES_ENABLE_DEBUGGER
  // The location isn't emitted in fully optimized builds.
  const JSONArray &locations = *llvh::cast<JSONArray>(root->at("locations"));
  Location loc = FIND_LOCATION_FOR_ID(funcID, locations, nodes, strings);
  // The location should be the given file, at line 1 column 5 with indenting
  auto scriptId = func->getRuntimeModule()->getScriptID();
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
      runtime->run("\n  function foo() {}; foo;", "file:///fake.js", flags);
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> func = runtime->makeHandle(vmcast<JSFunction>(*res));
  const auto funcID = runtime->getHeap().getObjectID(func.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime->getHeap(), jsonFactory);
  ASSERT_NE(root, nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  // This test requires a location to be emitted.
  auto node = FIND_NODE_FOR_ID(funcID, nodes, strings);
  Node expected{HeapSnapshot::NodeType::Closure,
                "foo",
                funcID,
                func->getAllocatedSize(),
                6};
  EXPECT_EQ(node, expected);
  // Edges aren't tested in this test.

#ifdef HERMES_ENABLE_DEBUGGER
  // The location isn't emitted in fully optimized builds.
  const JSONArray &locations = *llvh::cast<JSONArray>(root->at("locations"));
  Location loc = FIND_LOCATION_FOR_ID(funcID, locations, nodes, strings);
  // The location should be the given file, second line, third column
  auto scriptId = func->getRuntimeModule()->getScriptID();
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
  CallResult<HermesValue> res = runtime->run(
      R"(function foo() {}; foo.displayName = "bar"; foo;)",
      "file:///fake.js",
      flags);
  ASSERT_FALSE(isException(res));
  Handle<JSFunction> func = runtime->makeHandle(vmcast<JSFunction>(*res));
  const auto funcID = runtime->getHeap().getObjectID(func.get());

  JSONObject *root = TAKE_SNAPSHOT(runtime->getHeap(), jsonFactory);
  ASSERT_NE(root, nullptr);

  const JSONArray &nodes = *llvh::cast<JSONArray>(root->at("nodes"));
  const JSONArray &strings = *llvh::cast<JSONArray>(root->at("strings"));

  auto node = FIND_NODE_FOR_ID(funcID, nodes, strings);
  Node expected{HeapSnapshot::NodeType::Closure,
                // Make sure the name that is reported is "bar", not "foo".
                "bar",
                funcID,
                func->getAllocatedSize(),
                11};
  EXPECT_EQ(node, expected);
}

#ifdef HERMES_ENABLE_DEBUGGER

static std::string functionInfoToString(
    int idx,
    const JSONArray &traceFunctionInfos,
    const JSONArray &strings) {
  auto base = idx * 6;
  auto functionID =
      llvh::cast<JSONNumber>(traceFunctionInfos[base])->getValue();

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

  return std::string(name) + "(" + oscompat::to_string((int)functionID) +
      ") @ " + std::string(scriptName) + "(" +
      oscompat::to_string((int)scriptID) +
      "):" + oscompat::to_string((int)line) + ":" +
      oscompat::to_string((int)col);
}

struct ChromeStackTreeNode {
  ChromeStackTreeNode(ChromeStackTreeNode *parent, int traceFunctionInfosId)
      : parent_(parent), traceFunctionInfosId_(traceFunctionInfosId) {}

  static std::vector<std::unique_ptr<ChromeStackTreeNode>> parse(
      const JSONArray &traceNodes,
      ChromeStackTreeNode *parent,
      std::map<int, ChromeStackTreeNode *> &idNodeMap) {
    std::vector<std::unique_ptr<ChromeStackTreeNode>> res;
    for (size_t i = 0; i < traceNodes.size(); i += 5) {
      auto id = llvh::cast<JSONNumber>(traceNodes[i])->getValue();
      auto functionInfoIndex =
          llvh::cast<JSONNumber>(traceNodes[i + 1])->getValue();
      auto children = llvh::cast<JSONArray>(traceNodes[i + 4]);
      auto treeNode =
          hermes::make_unique<ChromeStackTreeNode>(parent, functionInfoIndex);
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
  runtime->enableAllocationLocationTracker();
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  hbc::CompileFlags flags;
  CallResult<HermesValue> res = runtime->run(
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
  Handle<JSObject> resObj = runtime->makeHandle(vmcast<JSObject>(*res));
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
  auto fooObjID = runtime->getHeap().getObjectID((*fooObj)->getPointer());
  auto barObjID = runtime->getHeap().getObjectID((*barObj)->getPointer());

  JSONObject *root = TAKE_SNAPSHOT(runtime->getHeap(), jsonFactory);
  ASSERT_NE(root, nullptr);

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
global(1) @ test.js(4):2:1
global(2) @ test.js(4):11:4
baz(7) @ test.js(4):9:19
foo(8) @ test.js(4):3:20)#");

  auto barAllocNode = FIND_NODE_FOR_ID(barObjID, nodes, strings);
  auto barStackTreeNode = idNodeMap.find(barAllocNode.traceNodeID);
  ASSERT_NE(barStackTreeNode, idNodeMap.end());
  auto barStackStr =
      barStackTreeNode->second->buildStackTrace(traceFunctionInfos, strings);
  ASSERT_STREQ(
      barStackStr.c_str(),
      R"#(
global(1) @ test.js(4):2:1
global(2) @ test.js(4):11:4
baz(3) @ test.js(4):9:31
bar(4) @ test.js(4):6:20)#");
}
#endif // HERMES_ENABLE_DEBUGGER

} // namespace heapsnapshottest
} // namespace unittest
} // namespace hermes
