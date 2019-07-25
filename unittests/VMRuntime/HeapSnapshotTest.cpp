/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/HeapSnapshot.h"
#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/Support/raw_ostream.h"

using namespace hermes::vm;

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
  mb.addNonPointerField("@x", &self->x);
  mb.addNonPointerField(&self->y);
  mb.addField("@other", &self->other);
}

static MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      buildMetadata(CellKind::UninitializedKind, DummyObjectBuildMeta)};
  return MetadataTableForTests(storage);
}

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

  const auto blockSize = sizeof(DummyObject);

  uint64_t segmentNumber = 1;
  uint64_t offset = AlignedHeapSegment::offsetOfAllocRegion;

  const uint64_t snapRoot =
      gc.segmentIndex().size() << AlignedStorage::kLogSize | 0;

  const uint64_t obj0 = segmentNumber << AlignedStorage::kLogSize | offset;
  offset += blockSize;
  const uint64_t obj1 = segmentNumber << AlignedStorage::kLogSize | offset;

  std::ostringstream stream;

  stream
      << "{"
      << "\"snapshot\":{"
      << "\"meta\":{"
      << "\"node_fields\":[\"type\",\"name\",\"id\",\"self_size\",\"edge_count\",\"trace_node_id\"],"
      << R"("node_types":[["hidden","array","string","object","code","closure","regexp","number","native","synthetic","concatenated string","sliced string","symbol","bigint"])"
      << ",\"string\",\"number\",\"number\",\"number\",\"number\"],"
      << "\"edge_fields\":[\"type\",\"name_or_index\",\"to_node\"],"
      << "\"edge_types\":["
      << "[\"context\",\"element\",\"property\",\"internal\",\"hidden\",\"shortcut\",\"weak\"],"
      << "\"string_or_number\",\"node\""
      << "],"
      << "\"trace_function_info_fields\":[],\"trace_node_fields\":[],\"sample_fields\":[],\"location_fields\":[]"
      << "},"
      << "\"node_count\":0,\"edge_count\":0,\"trace_function_count\":0"
      << "},"
      << "\"nodes\":["
      // Synthetic node representing the root of all roots.
      << static_cast<size_t>(V8HeapSnapshot::NodeType::Synthetic) << ",0,"
      << snapRoot << ",0,1,0,"
      << static_cast<size_t>(V8HeapSnapshot::NodeType::Object) << ",1," << obj0
      << "," << blockSize << ",1,0,"
      << static_cast<size_t>(V8HeapSnapshot::NodeType::Object) << ",1," << obj1
      << "," << blockSize << ",0,0"
      << "],"
      << "\"edges\":[3,2,6,3,3,12],"
      << "\"trace_function_infos\":[],\"trace_tree\":[],\"samples\":[],\"locations\":[],"
      << R"#("strings":["(GC Roots)","Uninitialized","","@other"])#"
      << "}";

  std::string expected = stream.str();

  ASSERT_EQ(expected, result);
}

} // namespace heapsnapshottest
} // namespace unittest
} // namespace hermes

#endif
