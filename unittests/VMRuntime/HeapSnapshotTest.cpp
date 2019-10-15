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
    hvNative.setNonPtr(HermesValue::encodeNativeValue(0xE));
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

static void testNode(
    JSONArray::iterator nodes,
    JSONArray &strings,
    HeapSnapshot::NodeType type,
    llvm::StringRef name,
    HeapSnapshot::NodeID id,
    size_t selfSize,
    size_t edgeCount,
    size_t traceNodeID,
    const char *file,
    int line) {
  // Need two levels of cast for enums because Windows complains about casting
  // doubles to enums.
  auto actualType = static_cast<HeapSnapshot::NodeType>(
      static_cast<unsigned>(llvm::cast<JSONNumber>(*nodes)->getValue()));
  if (actualType != type) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected type: " << ::testing::PrintToString(type)
        << "\n\t  Actual type: " << ::testing::PrintToString(actualType);
  }
  nodes++;

  auto actualName = llvm::cast<JSONString>(
                        strings[llvm::cast<JSONNumber>(*nodes)->getValue()])
                        ->str();
  if (actualName != name) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected name: " << ::testing::PrintToString(name)
        << "\n\t  Actual name: " << ::testing::PrintToString(actualName);
  }
  nodes++;

  auto actualID = static_cast<HeapSnapshot::NodeID>(
      llvm::cast<JSONNumber>(*nodes)->getValue());
  if (actualID != id) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected ID: " << ::testing::PrintToString(id)
        << "\n\t  Actual ID: " << ::testing::PrintToString(actualID);
  }
  nodes++;

  auto actualSelfSize =
      static_cast<size_t>(llvm::cast<JSONNumber>(*nodes)->getValue());
  if (actualSelfSize != selfSize) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected self size: " << ::testing::PrintToString(selfSize)
        << "\n\t  Actual self size: "
        << ::testing::PrintToString(actualSelfSize);
  }
  nodes++;

  auto actualEdgeCount =
      static_cast<size_t>(llvm::cast<JSONNumber>(*nodes)->getValue());
  if (actualEdgeCount != edgeCount) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected edge count: " << ::testing::PrintToString(edgeCount)
        << "\n\t  Actual edge count: "
        << ::testing::PrintToString(actualEdgeCount);
  }
  nodes++;

  auto actualTraceNodeID =
      static_cast<size_t>(llvm::cast<JSONNumber>(*nodes)->getValue());
  if (actualTraceNodeID != traceNodeID) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected trace node ID: " << ::testing::PrintToString(traceNodeID)
        << "\n\t  Actual trace node ID: "
        << ::testing::PrintToString(actualTraceNodeID);
  }
  nodes++;
}

#define TEST_NODE(...) testNode(__VA_ARGS__, __FILE__, __LINE__)

static ::testing::AssertionResult testEdge(
    JSONArray::iterator edges,
    JSONArray &nodes,
    JSONArray &strings,
    HeapSnapshot::EdgeType type,
    llvm::StringRef name,
    size_t index,
    size_t toNode,
    const char *file,
    int line) {
  // Need two levels of cast for enums because Windows complains about casting
  // doubles to enums.
  auto actualType = static_cast<HeapSnapshot::EdgeType>(
      static_cast<unsigned>(llvm::cast<JSONNumber>(*edges)->getValue()));
  if (actualType != type) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected type: " << ::testing::PrintToString(type)
        << "\n\t  Actual type: " << ::testing::PrintToString(actualType);
  }
  edges++;
  switch (actualType) {
    case HeapSnapshot::EdgeType::Internal: {
      auto actualName = llvm::cast<JSONString>(
                            strings[llvm::cast<JSONNumber>(*edges)->getValue()])
                            ->str();
      if (actualName != name) {
        ADD_FAILURE_AT(file, line)
            << "\tExpected name: " << ::testing::PrintToString(name)
            << "\n\t  Actual name: " << ::testing::PrintToString(actualName);
      }
      edges++;
      break;
    }
    default: {
      auto actualIndex = llvm::cast<JSONNumber>(*edges)->getValue();
      if (actualIndex != index) {
        ADD_FAILURE_AT(file, line)
            << "\tExpected index: " << ::testing::PrintToString(index)
            << "\n\t  Actual index: " << ::testing::PrintToString(actualIndex);
      }
      edges++;
      break;
    }
  }
  uint32_t actualToNode = llvm::cast<JSONNumber>(*edges)->getValue();
  assert(
      actualToNode % HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT == 0 &&
      "Invalid to node pointer");
  actualToNode /= HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  if (actualToNode != toNode) {
    ADD_FAILURE_AT(file, line)
        << "\tExpected node: " << ::testing::PrintToString(toNode)
        << "\n\t  Actual node: " << ::testing::PrintToString(actualToNode);
  }
  edges++;
  return ::testing::AssertionSuccess();
}

#define TEST_EDGE(...) testEdge(__VA_ARGS__, __FILE__, __LINE__)

TEST(HeapSnapshotTest, SnapshotTest) {
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

  std::string result("");
  llvm::raw_string_ostream str(result);
  gc.collect();
  gc.createSnapshot(str, true);
  str.flush();

  ASSERT_FALSE(result.empty());

  const auto blockSize = dummy->getAllocatedSize();

  JSONFactory::Allocator alloc;
  JSONFactory jsonFactory{alloc};
  SourceErrorManager sm;
  JSONParser parser{jsonFactory, result, sm};
  auto optSnapshot = parser.parse();
  ASSERT_TRUE(optSnapshot) << "Heap snapshot is not valid JSON";

  // Too verbose to check every key, so let llvm::cast do the checks.
  JSONObject *root = llvm::cast<JSONObject>(optSnapshot.getValue());
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
  EXPECT_EQ(llvm::cast<JSONArray>(meta->at("location_fields"))->size(), 0);

  JSONArray &nodeFields = *llvm::cast<JSONArray>(meta->at("node_fields"));
  JSONArray &nodeTypes = *llvm::cast<JSONArray>(meta->at("node_types"));
  JSONArray &edgeFields = *llvm::cast<JSONArray>(meta->at("edge_fields"));
  JSONArray &edgeTypes = *llvm::cast<JSONArray>(meta->at("edge_types"));

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
  TEST_NODE(
      nextNode,
      strings,
      HeapSnapshot::NodeType::Synthetic,
      "(GC Roots)",
      static_cast<HeapSnapshot::NodeID>(GC::IDTracker::ReservedObjectID::Root),
      0,
      1,
      0);
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the custom root section.
  TEST_NODE(
      nextNode,
      strings,
      HeapSnapshot::NodeType::Synthetic,
      "(Custom)",
      static_cast<HeapSnapshot::NodeID>(
          GC::IDTracker::ReservedObjectID::Custom),
      0,
      1,
      0);
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the first dummy object.
  TEST_NODE(
      nextNode,
      strings,
      HeapSnapshot::NodeType::Object,
      cellKindStr(dummy->getKind()),
      gc.getObjectID(dummy.get()),
      blockSize,
      1,
      0);
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  // Next node is the second dummy, which is only reachable via the first
  // dummy.
  TEST_NODE(
      nextNode,
      strings,
      HeapSnapshot::NodeType::Object,
      cellKindStr(dummy->getKind()),
      gc.getObjectID(dummy->other),
      blockSize,
      0,
      0);
  nextNode += HeapSnapshot::V8_SNAPSHOT_NODE_FIELD_COUNT;
  EXPECT_EQ(nextNode, nodes.end());

  auto nextEdge = edges.begin();
  // Pointer from root to root section.
  TEST_EDGE(
      nextEdge,
      nodes,
      strings,
      HeapSnapshot::EdgeType::Element,
      "(Custom)",
      1,
      1);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from root section to first dummy.
  TEST_EDGE(
      nextEdge, nodes, strings, HeapSnapshot::EdgeType::Element, "", 0, 2);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  // Pointer from first dummy to second dummy.
  TEST_EDGE(
      nextEdge,
      nodes,
      strings,
      HeapSnapshot::EdgeType::Internal,
      "other",
      1,
      3);
  nextEdge += HeapSnapshot::V8_SNAPSHOT_EDGE_FIELD_COUNT;
  EXPECT_EQ(nextEdge, edges.end());

  // String table is checked by the nodes and edges checks.
}

} // namespace heapsnapshottest
} // namespace unittest
} // namespace hermes

#endif
