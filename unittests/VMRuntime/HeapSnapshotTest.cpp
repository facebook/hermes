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
#include "hermes/Support/Allocator.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

using namespace hermes::vm;
using namespace hermes::parser;

// Only the main NCGen needs to support snapshots
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL

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
    hvBool.setNonPtr(HermesValue::encodeBoolValue(true));
    hvDouble.setNonPtr(HermesValue::encodeNumberValue(3.14));
    hvNative.setNonPtr(HermesValue::encodeNativeUInt32(0xE));
    hvUndefined.setNonPtr(HermesValue::encodeUndefinedValue());
    hvEmpty.setNonPtr(HermesValue::encodeEmptyValue());
    hvNull.setNonPtr(HermesValue::encodeNullValue());
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

  static Node parse(JSONArray::iterator nodes, JSONArray &strings) {
    // Need two levels of cast for enums because Windows complains about casting
    // doubles to enums.
    auto type = static_cast<HeapSnapshot::NodeType>(
        static_cast<unsigned>(llvm::cast<JSONNumber>(*nodes)->getValue()));
    nodes++;

    std::string name = llvm::cast<JSONString>(
                           strings[llvm::cast<JSONNumber>(*nodes)->getValue()])
                           ->str();
    nodes++;

    auto id = static_cast<HeapSnapshot::NodeID>(
        llvm::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto selfSize =
        static_cast<size_t>(llvm::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto edgeCount =
        static_cast<size_t>(llvm::cast<JSONNumber>(*nodes)->getValue());
    nodes++;

    auto traceNodeID =
        static_cast<size_t>(llvm::cast<JSONNumber>(*nodes)->getValue());

    return Node{type, std::move(name), id, selfSize, edgeCount, traceNodeID};
  }

  bool operator==(const Node &that) const {
    return type == that.type && name == that.name && id == that.id &&
        selfSize == that.selfSize && edgeCount == that.edgeCount &&
        traceNodeID == that.traceNodeID;
  }
};

std::ostream &operator<<(std::ostream &os, const Node &node);

std::ostream &operator<<(std::ostream &os, const Node &node) {
  return os << "Node{type=" << HeapSnapshot::nodeTypeToName(node.type)
            << ", name=" << node.name << ", id=" << node.id
            << ", selfSize=" << node.selfSize
            << ", edgeCount=" << node.edgeCount
            << ", traceNodeID=" << node.traceNodeID << "}";
}

struct Edge {
  HeapSnapshot::EdgeType type;
  bool isNamed;
  std::string name;
  int index;
  // Not an object ID, but instead an index into a nodes array.
  size_t toNode;

  Edge() = default;

  explicit Edge(HeapSnapshot::EdgeType type, std::string name, size_t toNode)
      : type{type},
        isNamed{true},
        name{std::move(name)},
        index{-1},
        toNode{toNode} {}

  explicit Edge(HeapSnapshot::EdgeType type, int index, size_t toNode)
      : type{type}, isNamed{false}, name{}, index{index}, toNode{toNode} {}

  static Edge parse(JSONArray::iterator edges, JSONArray &strings) {
    Edge edge;
    // Need two levels of cast for enums because Windows complains about casting
    // doubles to enums.
    edge.type = static_cast<HeapSnapshot::EdgeType>(
        static_cast<unsigned>(llvm::cast<JSONNumber>(*edges)->getValue()));
    ++edges;
    switch (edge.type) {
      case HeapSnapshot::EdgeType::Internal:
        edge.isNamed = true;
        edge.name = llvm::cast<JSONString>(
                        strings[llvm::cast<JSONNumber>(*edges)->getValue()])
                        ->str();
        edge.index = -1;
        break;
      default:
        edge.isNamed = false;
        // Leave name as the empty string.
        edge.index = llvm::cast<JSONNumber>(*edges)->getValue();
        break;
    }
    ++edges;

    uint32_t toNode = llvm::cast<JSONNumber>(*edges)->getValue();
    assert(
        toNode % HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT == 0 &&
        "Invalid to node pointer");
    toNode /= HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
    edge.toNode = toNode;
    return edge;
  }

  Node getToNode(JSONArray &nodes, JSONArray &strings) const {
    return Node::parse(
        nodes.begin() + toNode * HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT,
        strings);
  }

  bool operator==(const Edge &that) const {
    return type == that.type && isNamed == that.isNamed && name == that.name &&
        index == that.index && toNode == that.toNode;
  }
};

std::ostream &operator<<(std::ostream &os, const Edge &edge);

std::ostream &operator<<(std::ostream &os, const Edge &edge) {
  std::ios_base::fmtflags f(os.flags());
  os << std::boolalpha
     << "Edge{type=" << HeapSnapshot::edgeTypeToName(edge.type)
     << ", isNamed=" << edge.isNamed << ", name=" << edge.name
     << ", index=" << edge.index << ", toNode=" << edge.toNode << "}";
  // Reset original flags to remove the boolalpha.
  os.flags(f);
  return os;
}

static ::testing::AssertionResult testListOfStrings(
    JSONArray::iterator begin,
    JSONArray::iterator end,
    std::initializer_list<llvm::StringRef> strs) {
  EXPECT_EQ(static_cast<unsigned long>(end - begin), strs.size());
  auto strsIt = strs.begin();
  for (auto it = begin; it != end; ++it) {
    EXPECT_EQ(llvm::cast<JSONString>(*it)->str(), *strsIt);
    ++strsIt;
  }
  return ::testing::AssertionSuccess();
}

static ::testing::AssertionResult testListOfStrings(
    const JSONArray &arr,
    std::initializer_list<llvm::StringRef> strs) {
  return testListOfStrings(arr.begin(), arr.end(), strs);
}

static JSONObject *
takeSnapshot(GC &gc, JSONFactory &factory, const char *file, int line) {
  std::string result("");
  llvm::raw_string_ostream str(result);
  gc.collect();
  gc.createSnapshot(str);
  str.flush();

  if (result.empty()) {
    ADD_FAILURE_AT(file, line) << "Snapshot wasn't written out";
    return nullptr;
  }

  SourceErrorManager sm;
  JSONParser parser{factory, result, sm};
  auto optSnapshot = parser.parse();
  if (!optSnapshot) {
    ADD_FAILURE_AT(file, line) << "Snapshot isn't valid JSON";
    return nullptr;
  }
  JSONValue *root = optSnapshot.getValue();
  if (!llvm::isa<JSONObject>(root)) {
    ADD_FAILURE_AT(file, line) << "Snapshot isn't a JSON object";
    return nullptr;
  }

  return llvm::cast<JSONObject>(root);
}

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

  JSONObject *snapshot = llvm::cast<JSONObject>(root->at("snapshot"));

  EXPECT_EQ(llvm::cast<JSONNumber>(snapshot->at("node_count"))->getValue(), 0);
  EXPECT_EQ(llvm::cast<JSONNumber>(snapshot->at("edge_count"))->getValue(), 0);
  EXPECT_EQ(
      llvm::cast<JSONNumber>(snapshot->at("trace_function_count"))->getValue(),
      0);

  JSONObject *meta = llvm::cast<JSONObject>(snapshot->at("meta"));
  EXPECT_EQ(
      llvm::cast<JSONArray>(meta->at("trace_function_info_fields"))->size(), 0);
  EXPECT_EQ(llvm::cast<JSONArray>(meta->at("trace_node_fields"))->size(), 0);
  EXPECT_EQ(llvm::cast<JSONArray>(meta->at("sample_fields"))->size(), 0);

  JSONArray &nodeFields = *llvm::cast<JSONArray>(meta->at("node_fields"));
  JSONArray &nodeTypes = *llvm::cast<JSONArray>(meta->at("node_types"));
  JSONArray &edgeFields = *llvm::cast<JSONArray>(meta->at("edge_fields"));
  JSONArray &edgeTypes = *llvm::cast<JSONArray>(meta->at("edge_types"));
  JSONArray &locationFields =
      *llvm::cast<JSONArray>(meta->at("location_fields"));

  // Check that node_fields/types are correct.
  EXPECT_TRUE(testListOfStrings(
      nodeFields,
      {"type", "name", "id", "self_size", "edge_count", "trace_node_id"}));
  const JSONArray &nodeTypeEnum = *llvm::cast<JSONArray>(nodeTypes[0]);
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
  const JSONArray &edgeTypeEnum = *llvm::cast<JSONArray>(edgeTypes[0]);
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
  JSONArray &nodes = *llvm::cast<JSONArray>(root->at("nodes"));
  JSONArray &edges = *llvm::cast<JSONArray>(root->at("edges"));
  JSONArray &strings = *llvm::cast<JSONArray>(root->at("strings"));

  EXPECT_EQ(llvm::cast<JSONArray>(root->at("trace_function_infos"))->size(), 0);
  EXPECT_EQ(llvm::cast<JSONArray>(root->at("trace_tree"))->size(), 0);
  EXPECT_EQ(llvm::cast<JSONArray>(root->at("samples"))->size(), 0);
  EXPECT_EQ(llvm::cast<JSONArray>(root->at("locations"))->size(), 0);

  // First node is the roots object.
  auto nextNode = nodes.begin();
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Synthetic,
          "(GC Roots)",
          static_cast<HeapSnapshot::NodeID>(
              GC::IDTracker::ReservedObjectID::Root),
          0,
          1));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the custom root section.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Synthetic,
          "(Custom)",
          static_cast<HeapSnapshot::NodeID>(
              GC::IDTracker::ReservedObjectID::Custom),
          0,
          1));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the first dummy object.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Object,
          cellKindStr(dummy->getKind()),
          gc.getObjectID(dummy.get()),
          blockSize,
          // One edge to the second dummy, 4 for primitive singletons.
          5));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the second dummy, which is only reachable via the first
  // dummy.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Object,
          cellKindStr(dummy->getKind()),
          gc.getObjectID(dummy->other),
          blockSize,
          // No edges except for the primitive singletons.
          4));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the undefined singleton.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Object,
          "undefined",
          static_cast<HeapSnapshot::NodeID>(
              GCBase::IDTracker::ReservedObjectID::Undefined),
          0,
          0));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the null singleton.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Object,
          "null",
          static_cast<HeapSnapshot::NodeID>(
              GCBase::IDTracker::ReservedObjectID::Null),
          0,
          0));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the true singleton.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Object,
          "true",
          static_cast<HeapSnapshot::NodeID>(
              GCBase::IDTracker::ReservedObjectID::True),
          0,
          0));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the false singleton.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Object,
          "false",
          static_cast<HeapSnapshot::NodeID>(
              GCBase::IDTracker::ReservedObjectID::False),
          0,
          0));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the first number.
  EXPECT_EQ(
      Node::parse(nextNode, strings),
      Node(
          HeapSnapshot::NodeType::Number,
          "3.14",
          gc.getIDTracker().getNumberID(dummy->hvDouble.getNumber()),
          0,
          0));
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  EXPECT_EQ(nextNode, nodes.end());

  Edge trueEdge = Edge(HeapSnapshot::EdgeType::Internal, "HermesBool", 6);
  Edge numberEdge = Edge(HeapSnapshot::EdgeType::Internal, "HermesDouble", 8);
  Edge undefinedEdge =
      Edge(HeapSnapshot::EdgeType::Internal, "HermesUndefined", 4);
  Edge nullEdge = Edge(HeapSnapshot::EdgeType::Internal, "HermesNull", 5);
  auto nextEdge = edges.begin();
  // Pointer from root to root section.
  EXPECT_EQ(
      Edge::parse(nextEdge, strings),
      Edge(HeapSnapshot::EdgeType::Element, 1, 1));
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;

  // Pointer from root section to first dummy.
  EXPECT_EQ(
      Edge::parse(nextEdge, strings),
      Edge(HeapSnapshot::EdgeType::Element, 0, 2));
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;

  // Pointer from first dummy to second dummy.
  EXPECT_EQ(
      Edge::parse(nextEdge, strings),
      Edge(HeapSnapshot::EdgeType::Internal, "other", 3));
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from first dummy to its bool field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), trueEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from first dummy to its number field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), numberEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from first dummy to its undefined field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), undefinedEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from first dummy to its null field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), nullEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;

  // Pointer from second dummy to its bool field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), trueEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from second dummy to its number field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), numberEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from second dummy to its undefined field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), undefinedEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from second dummy to its null field.
  EXPECT_EQ(Edge::parse(nextEdge, strings), nullEdge);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  EXPECT_EQ(nextEdge, edges.end());

  // String table is checked by the nodes and edges checks.
}

using HeapSnapshotRuntimeTest = RuntimeTestFixture;

TEST_F(HeapSnapshotRuntimeTest, SnapshotLazyCodeDoesNotAssert) {
  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};

  hbc::CompileFlags flags;
  flags.debug = true;
  flags.lazy = true;

  // Build a function that will be lazily compiled.
  std::string source = "function myGlobal() { ";
  for (int i = 0; i < 100; i++)
    source += " Math.random(); ";
  source += "};\n";

  CallResult<HermesValue> res = runtime->run(source, "file:///fake.js", flags);
  ASSERT_FALSE(isException(res));

  // Make sure we can capture a snapshot without asserting.
  TAKE_SNAPSHOT(runtime->getHeap(), jsonFactory);
}

} // namespace heapsnapshottest
} // namespace unittest
} // namespace hermes

#endif
