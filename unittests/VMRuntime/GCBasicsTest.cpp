/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "Array.h"
#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/WeakRef.h"

#include <functional>
#include <new>
#include <vector>

#include "gtest/gtest.h"

using namespace hermes::vm;
using namespace hermes::unittest;

namespace {

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize(), true>;
#endif

struct Dummy final : public GCCell {
  static const VTable vt;

  static Dummy *create(DummyRuntime &runtime) {
    return new (runtime.alloc(sizeof(Dummy))) Dummy(&runtime.getHeap());
  }
  static Dummy *createLongLived(DummyRuntime &runtime) {
    return new (runtime.allocLongLived(sizeof(Dummy)))
        Dummy(&runtime.getHeap());
  }
  static bool classof(const GCCell *cell) {
    return cell->getVT() == &vt;
  }

  Dummy(GC *gc) : GCCell(gc, &vt) {}
};

size_t getExtraSize(GCCell *) {
  return 1;
}

/// A virtual table without a finalizer or weak ref marker, with extra storage
/// space.
const VTable Dummy::vt{CellKind::UninitializedKind,
                       sizeof(Dummy),
                       nullptr,
                       nullptr,
                       getExtraSize};

const MetadataTableForTests getMetadataTable() {
  // It would seem that the full namespace qualification below would not be
  // necessary, but without it the compiler is unable to properly infer a
  // template argument.
  static const Metadata storage[] = {
      Metadata(),
      buildMetadata(
          CellKind::FillerCellKind, ::hermes::unittest::ArrayBuildMeta)};
  return MetadataTableForTests(storage);
}

} // namespace

namespace hermes {
namespace vm {
template <>
struct IsGCObject<Array> : public std::true_type {};
template <>
struct IsGCObject<Dummy> : public std::true_type {};
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
template <>
struct IsGCObject<SegmentCell> : public std::true_type {};
#endif
} // namespace vm
} // namespace hermes

namespace {

struct GCBasicsTest : public ::testing::Test {
  std::shared_ptr<DummyRuntime> runtime;
  DummyRuntime &rt;
  GCBasicsTest()
      : runtime(DummyRuntime::create(getMetadataTable(), kTestGCConfigSmall)),
        rt(*runtime) {}
};

#ifndef NDEBUG
TEST_F(GCBasicsTest, SmokeTest) {
  auto &gc = rt.gc;
  GCBase::HeapInfo info;
  GCBase::DebugHeapInfo debugInfo;

  // Verify the initial state.
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(0u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(0u, info.numCollections);
  ASSERT_EQ(0u, info.allocatedBytes);

  // Collect an empty heap.
  gc.collect();
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(0u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(1u, info.numCollections);
  ASSERT_EQ(0u, info.allocatedBytes);

  // Allocate a single object without GC.
  Dummy::create(rt);
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(1u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(0u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(1u, info.numCollections);
  ASSERT_EQ(sizeof(Dummy), info.allocatedBytes);

  // Now free the unreachable object.
  gc.collect();
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(1u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(2u, info.numCollections);
  ASSERT_EQ(0u, info.allocatedBytes);

  // Allocate two objects.
  Dummy::create(rt);
  GCCell *o2 = Dummy::create(rt);
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(2u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(1u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(2u, info.numCollections);
  ASSERT_EQ(2 * sizeof(Dummy), info.allocatedBytes);

  // Make only the second object reachable and collect.
  rt.pointerRoots.push_back(&o2);
  gc.collect();
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(1u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(1u, debugInfo.numReachableObjects);
  ASSERT_EQ(1u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(3u, info.numCollections);
  ASSERT_EQ(sizeof(Dummy), info.allocatedBytes);
}

TEST_F(GCBasicsTest, MovedObjectTest) {
  auto &gc = rt.gc;
  GCBase::HeapInfo info;
  GCBase::DebugHeapInfo debugInfo;

  gcheapsize_t totalAlloc = 0;

  Array::create(rt, 0);
  totalAlloc += heapAlignSize(Array::allocSize(0));
  auto *a1 = Array::create(rt, 3);
  totalAlloc += heapAlignSize(Array::allocSize(3));
  auto *a2 = Array::create(rt, 3);
  totalAlloc += heapAlignSize(Array::allocSize(3));
  // Verify the initial state.
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(3u, debugInfo.numAllocatedObjects);
  EXPECT_EQ(0u, debugInfo.numReachableObjects);
  EXPECT_EQ(0u, debugInfo.numCollectedObjects);
  EXPECT_EQ(0u, debugInfo.numFinalizedObjects);
  EXPECT_EQ(0u, info.numCollections);
  EXPECT_EQ(totalAlloc, info.allocatedBytes);

  // Initialize a reachable graph.
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&a2));
  a2->values()[0].set(HermesValue::encodeObjectValue(a1), &gc);
  a2->values()[2].set(HermesValue::encodeObjectValue(a2), &gc);
  a1->values()[0].set(HermesValue::encodeObjectValue(a1), &gc);
  a1->values()[1].set(HermesValue::encodeObjectValue(a2), &gc);

  gc.collect();
  totalAlloc -= heapAlignSize(Array::allocSize(0));
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(2u, debugInfo.numAllocatedObjects);
  EXPECT_EQ(2u, debugInfo.numReachableObjects);
  EXPECT_EQ(1u, debugInfo.numCollectedObjects);
  EXPECT_EQ(1u, debugInfo.numFinalizedObjects);
  EXPECT_EQ(1u, info.numCollections);
  EXPECT_EQ(totalAlloc, info.allocatedBytes);

  // Extract what we know was a pointer to a1.
  a1 = reinterpret_cast<Array *>(a2->values()[0].getPointer());

  // Ensure the contents of the objects changed.
  EXPECT_EQ(a1, a2->values()[0].getPointer());
  EXPECT_EQ(HermesValue::encodeEmptyValue(), a2->values()[1]);
  EXPECT_EQ(a2, a2->values()[2].getPointer());
  EXPECT_EQ(a1, a1->values()[0].getPointer());
  EXPECT_EQ(a2, a1->values()[1].getPointer());
  EXPECT_EQ(HermesValue::encodeEmptyValue(), a1->values()[2]);
}

TEST_F(GCBasicsTest, WeakRefTest) {
  // This should match the one used by the GC.
  // TODO This should be shared in GCBase, since all GCs use the same format.
  enum WeakSlotState {
    Unmarked,
    Marked,
    Free,
  };
  auto &gc = rt.gc;
  GCBase::DebugHeapInfo debugInfo;

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(0u, debugInfo.numAllocatedObjects);

  auto *a1 = Array::create(rt, 10);
  auto *a2 = Array::create(rt, 10);

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(2u, debugInfo.numAllocatedObjects);

  WeakRef<Array> wr1{&gc, a1};
  WeakRef<Array> wr2{&gc, a2};

  ASSERT_TRUE(wr1.isValid());
  ASSERT_TRUE(wr2.isValid());

  ASSERT_EQ(a1, wr1.unsafeGetHermesValue().getPointer());
  ASSERT_EQ(a2, wr2.unsafeGetHermesValue().getPointer());

  // Test that freeing an object correctly "empties" the weak ref slot but
  // preserves the other slot.
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&a2));
  rt.markExtraWeak = [&](WeakRefAcceptor &acceptor) {
    acceptor.accept(wr1);
    acceptor.accept(wr2);
  };

  // a1 is supposed to be freed during the following collection, so clear
  // the pointer to avoid mistakes.
  a1 = nullptr;

  gc.collect();
  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(1u, debugInfo.numAllocatedObjects);
  ASSERT_FALSE(wr1.isValid());
  ASSERT_TRUE(wr2.isValid());
  ASSERT_EQ(a2, wr2.unsafeGetHermesValue().getPointer());

  // Make the slot unreachable and test that it is freed.
  rt.markExtraWeak = [&](WeakRefAcceptor &acceptor) { acceptor.accept(wr2); };
  gc.collect();

  ASSERT_EQ(WeakSlotState::Free, wr1.unsafeGetSlot()->extra);
}

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
TEST_F(GCBasicsTest, WeakRefYoungGenCollectionTest) {
  // This should match the one used by the GC.
  // TODO This should be shared in GCBase, since all GCs use the same format.
  enum WeakSlotState {
    Unmarked,
    Marked,
    Free,
  };
  auto &gc = rt.gc;
  GCBase::DebugHeapInfo debugInfo;

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(0u, debugInfo.numAllocatedObjects);

  auto *d0 = Dummy::create(rt);
  auto *d1 = Dummy::create(rt);
  auto *dOld = Dummy::createLongLived(rt);

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(3u, debugInfo.numAllocatedObjects);

  WeakRef<Dummy> wr0{&gc, d0};
  WeakRef<Dummy> wr1{&gc, d1};
  WeakRef<Dummy> wrOld{&gc, dOld};

  ASSERT_TRUE(wr0.isValid());
  ASSERT_TRUE(wr1.isValid());
  ASSERT_TRUE(wrOld.isValid());

  ASSERT_EQ(d0, wr0.unsafeGetHermesValue().getPointer());
  ASSERT_EQ(d1, wr1.unsafeGetHermesValue().getPointer());
  ASSERT_EQ(dOld, wrOld.unsafeGetHermesValue().getPointer());

  // Create a root for d0.  We'll only be doing young-gen collections, so
  // we don't have to root dOld.
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&d0));
  rt.markExtraWeak = [&](WeakRefAcceptor &acceptor) {
    acceptor.accept(wr0);
    acceptor.accept(wr1);
    acceptor.accept(wrOld);
  };

  // d1 is supposed to be freed during the following collection, so clear
  // the pointer to avoid mistakes.
  d1 = nullptr;

  gc.youngGenCollect();
  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(2u, debugInfo.numAllocatedObjects);
  ASSERT_TRUE(wr0.isValid());
  ASSERT_FALSE(wr1.isValid());
  ASSERT_TRUE(wrOld.isValid());
  ASSERT_EQ(d0, wr0.unsafeGetHermesValue().getPointer());
  ASSERT_EQ(dOld, wrOld.unsafeGetHermesValue().getPointer());
}

TEST_F(GCBasicsTest, TestYoungGenStats) {
  auto &gc = rt.gc;

  GCBase::DebugHeapInfo debugInfo;

  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, gc.numGCs());
  ASSERT_EQ(0u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(0u, debugInfo.numCollectedObjects);

  GCCell *cell = Dummy::create(rt);
  rt.pointerRoots.push_back(&cell);

  Dummy::create(rt);

  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, gc.numGCs());
  EXPECT_EQ(2u, debugInfo.numAllocatedObjects);

  gc.youngGenCollect();

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(1u, gc.numGCs());
  EXPECT_EQ(1u, debugInfo.numAllocatedObjects);
  EXPECT_EQ(1u, debugInfo.numReachableObjects);
  EXPECT_EQ(1u, debugInfo.numCollectedObjects);
}

#endif // HERMES_GC_GENERATIONAL || HERMES_GC_NONCONTIG_GENERATIONAL
#endif // !NDEBUG

TEST_F(GCBasicsTest, VariableSizeRuntimeCellOffsetTest) {
  auto *cell = Array::create(rt, 1);
  EXPECT_EQ(cell->getAllocatedSize(), heapAlignSize(Array::allocSize(1)));
}

TEST_F(GCBasicsTest, TestFixedRuntimeCell) {
  auto *cell = Dummy::create(rt);
  EXPECT_EQ(cell->getAllocatedSize(), heapAlignSize(sizeof(Dummy)));
}

/// Test that the extra bytes in the heap are reported correctly.
TEST_F(GCBasicsTest, ExtraBytes) {
  auto &gc = rt.gc;

  {
    GCBase::HeapInfo info;
    (void)Dummy::create(rt);
    gc.getHeapInfoWithMallocSize(info);
    // Since there have been no collections, and there is one Dummy in the heap,
    // it should be exactly equal to the number returned by getExtraSize.
    EXPECT_EQ(info.mallocSizeEstimate, getExtraSize(nullptr));
  }

  {
    GCBase::HeapInfo info;
    (void)Dummy::create(rt);
    gc.getHeapInfoWithMallocSize(info);
    EXPECT_EQ(info.mallocSizeEstimate, getExtraSize(nullptr) * 2);
  }
}

/// Test that the id is set to a unique number for each allocated object.
TEST_F(GCBasicsTest, TestIDIsUnique) {
  auto *cell = Dummy::create(rt);
  auto id1 = rt.getHeap().getObjectID(cell);
  cell = Dummy::create(rt);
  auto id2 = rt.getHeap().getObjectID(cell);
  EXPECT_NE(id1, id2);
}

TEST_F(GCBasicsTest, TestIDPersistsAcrossCollections) {
  GCScope scope{&rt};
  auto handle = rt.makeHandle<Dummy>(Dummy::create(rt));
  const auto idBefore = rt.getHeap().getObjectID(*handle);
  rt.getHeap().collect();
  const auto idAfter = rt.getHeap().getObjectID(*handle);
  EXPECT_EQ(idBefore, idAfter);
}

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
TEST(GCBasicsTestNCGen, TestIDPersistsAcrossMultipleCollections) {
  constexpr size_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;

  const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);
  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;

  GCScope scope{&rt};
  auto handle = rt.makeHandle<SegmentCell>(SegmentCell::create(rt));
  const auto originalID = rt.getHeap().getObjectID(*handle);
  GC::HeapInfo originalHeapInfo;
  rt.getHeap().getHeapInfo(originalHeapInfo);
  // A second allocation should put the first object into the old gen.
  auto handle2 = rt.makeHandle<SegmentCell>(SegmentCell::create(rt));
  (void)handle2;
  auto idAfter = rt.getHeap().getObjectID(*handle);
  GC::HeapInfo afterHeapInfo;
  rt.getHeap().getHeapInfo(afterHeapInfo);
  EXPECT_EQ(originalID, idAfter);
  // There should have been one young gen collection.
  EXPECT_EQ(
      afterHeapInfo.youngGenStats.numCollections,
      originalHeapInfo.youngGenStats.numCollections + 1);
  // Fill the old gen to force a collection.
  auto N = rt.getHeap().maxSize() / SegmentCell::size() - 1;
  std::deque<GCCell *> roots;
  for (size_t i = 0; i < N - 1; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
  // Remove all the roots for the final allocation so that an old gen GC can
  // occur.
  rt.pointerRoots.clear();
  roots.clear();
  SegmentCell::create(rt);
  idAfter = rt.getHeap().getObjectID(*handle);
  rt.getHeap().getHeapInfo(afterHeapInfo);
  EXPECT_EQ(originalID, idAfter);
  // There should have been one old gen collection.
  EXPECT_EQ(
      afterHeapInfo.fullStats.numCollections,
      originalHeapInfo.fullStats.numCollections + 1);
}
#endif

} // namespace
