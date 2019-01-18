/**
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

// Ignore this test for MallocGC, it doesn't need to implement snapshots.
#ifndef HERMESVM_GC_MALLOC

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
    other.set(obj, &rt.gc);
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

  auto blockSize = sizeof(DummyObject);
  auto heapSize = blockSize * 2;

#ifdef HERMESVM_GC_GENERATIONAL
  // GenGC compacts into the old gen, and the snapshot runs a collection,
  // changing the root location.
  uint64_t rootLocation = 16384;
#elif defined(HERMESVM_GC_NONCONTIG_GENERATIONAL)
  uint64_t segmentNumber = 1;
  uint64_t offset = AlignedHeapSegment::offsetOfAllocRegion;
  uint64_t rootLocation = segmentNumber << AlignedStorage::kLogSize | offset;
#else
  uint64_t rootLocation = 0;
#endif

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
  auto secondBlock =
      segmentNumber << AlignedStorage::kLogSize | (offset + blockSize);
#else
  auto secondBlock = rootLocation + blockSize;
#endif

  std::ostringstream stream;

  stream
      << "{\"version\":" << FacebookHeapSnapshot::version
      << ",\"totalHeapSize\":" << heapSize << ",\"roots\":[\"" << rootLocation
      << "\"],\"refs\":{\"" << rootLocation << "\":{\"size\":" << blockSize
      << ",\"type\":\"Uninitialized\",\"props\":[[\"@other\",\"" << secondBlock
      << "\"],[\"HermesBool\",[1]],[\"HermesDouble\",[3.14]],[\"HermesUndefined\",[]],[\"HermesEmpty\",[\"empty\"]],[\"HermesNative\",[\"0xe\"]],[\"HermesNull\",[null]],[\"@x\",[\"1\"]],[null,[\"2\"]]]},\""
      << secondBlock << "\":{\"size\":" << blockSize
      << ",\"type\":\"Uninitialized\",\"props\":[[\"HermesBool\",[1]],[\"HermesDouble\",[3.14]],[\"HermesUndefined\",[]],[\"HermesEmpty\",[\"empty\"]],[\"HermesNative\",[\"0xe\"]],[\"HermesNull\",[null]],[\"@x\",[\"1\"]],[null,[\"2\"]]]}}"
      << ",\"idtable\":[[0,\"DummyIdTableEntry0\"],[1,\"DummyIdTableEntry1\"]]}";

  std::string expected = stream.str();

  ASSERT_EQ(expected, result);
}

} // namespace heapsnapshottest
} // namespace unittest
} // namespace hermes

#endif
