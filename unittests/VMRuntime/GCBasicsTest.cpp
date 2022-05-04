/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/WeakRef.h"
#include "hermes/VM/WeakRoot-inline.h"

#include <functional>
#include <new>
#include <vector>

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using testhelpers::DummyObject;

struct GCBasicsTest : public ::testing::Test {
  std::shared_ptr<DummyRuntime> runtime;
  DummyRuntime &rt;
  GCBasicsTest()
      : runtime(DummyRuntime::create(kTestGCConfigSmall)), rt(*runtime) {}
};

// Hades doesn't report its stats the same way as other GCs.
#if !defined(NDEBUG) && !defined(HERMESVM_GC_HADES) && \
    !defined(HERMESVM_GC_RUNTIME)
TEST_F(GCBasicsTest, SmokeTest) {
  auto &gc = rt.getHeap();
  GCBase::HeapInfo info;
  GCBase::DebugHeapInfo debugInfo;
  GCScope scope{rt};

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
  rt.collect();
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(0u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(1u, info.numCollections);
  ASSERT_EQ(0u, info.allocatedBytes);

  // Allocate a single object without GC.
  DummyObject::create(rt.getHeap(), rt);
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(1u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(0u, debugInfo.numCollectedObjects);
  ASSERT_EQ(0u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(1u, info.numCollections);
  ASSERT_EQ(sizeof(DummyObject), info.allocatedBytes);

  // Now free the unreachable object.
  rt.collect();
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(0u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(1u, debugInfo.numCollectedObjects);
  ASSERT_EQ(1u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(2u, info.numCollections);
  ASSERT_EQ(0u, info.allocatedBytes);

  // Allocate two objects, the second with a GC handle.
  DummyObject::create(rt.getHeap(), rt);
  rt.makeHandle(DummyObject::create(rt.getHeap(), rt));
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(2u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(0u, debugInfo.numReachableObjects);
  ASSERT_EQ(1u, debugInfo.numCollectedObjects);
  ASSERT_EQ(1u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(2u, info.numCollections);
  ASSERT_EQ(2 * sizeof(DummyObject), info.allocatedBytes);

  // Collect the first but not second object.
  rt.collect();
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  ASSERT_EQ(1u, debugInfo.numAllocatedObjects);
  ASSERT_EQ(1u, debugInfo.numReachableObjects);
  ASSERT_EQ(1u, debugInfo.numCollectedObjects);
  ASSERT_EQ(1u, debugInfo.numFinalizedObjects);
  ASSERT_EQ(3u, info.numCollections);
  ASSERT_EQ(sizeof(DummyObject), info.allocatedBytes);
}

TEST_F(GCBasicsTest, MovedObjectTest) {
  auto &gc = rt.getHeap();
  GCBase::HeapInfo info;
  GCBase::DebugHeapInfo debugInfo;
  GCScope scope{rt};

  // Initialize three arrays, one with a GC handle.
  gcheapsize_t totalAlloc = 0;

  ArrayStorage::createForTest(gc, 0);
  totalAlloc += heapAlignSize(ArrayStorage::allocationSize(0));
  auto *a1 = ArrayStorage::createForTest(gc, 3);
  totalAlloc += heapAlignSize(ArrayStorage::allocationSize(3));
  auto a2 = rt.makeHandle(ArrayStorage::createForTest(gc, 3));
  totalAlloc += heapAlignSize(ArrayStorage::allocationSize(3));
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
  a2->set(0, HermesValue::encodeObjectValue(a1), gc);
  a2->set(2, HermesValue::encodeObjectValue(*a2), gc);
  a1->set(0, HermesValue::encodeObjectValue(a1), gc);
  a1->set(1, HermesValue::encodeObjectValue(*a2), gc);

  rt.collect();
  totalAlloc -= heapAlignSize(ArrayStorage::allocationSize(0));
  gc.getHeapInfo(info);
  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(2u, debugInfo.numAllocatedObjects);
  EXPECT_EQ(2u, debugInfo.numReachableObjects);
  EXPECT_EQ(1u, debugInfo.numCollectedObjects);
  EXPECT_EQ(0u, debugInfo.numFinalizedObjects);
  EXPECT_EQ(1u, info.numCollections);
  EXPECT_EQ(totalAlloc, info.allocatedBytes);

  // Extract what we know was a pointer to a1.
  a1 = vmcast<ArrayStorage>(a2->at(0));

  // Ensure the contents of the objects changed.
  EXPECT_EQ(a1, a2->at(0).getPointer());
  EXPECT_EQ(HermesValue::encodeEmptyValue(), a2->at(1));
  EXPECT_EQ(*a2, a2->at(2).getPointer());
  EXPECT_EQ(a1, a1->at(0).getPointer());
  EXPECT_EQ(*a2, a1->at(1).getPointer());
  EXPECT_EQ(HermesValue::encodeEmptyValue(), a1->at(2));
}

TEST_F(GCBasicsTest, WeakRefSlotTest) {
  // WeakRefSlot can hold any 4-byte aligned pointer.
  auto obj = (void *)0x12345670;
  CompressedPointer ptr =
      CompressedPointer::encode(static_cast<GCCell *>(obj), rt);

  WeakRefSlot s(ptr);
  EXPECT_EQ(WeakSlotState::Unmarked, s.state());
  EXPECT_TRUE(s.hasValue());
  EXPECT_EQ(ptr, s.getNoBarrierUnsafe());
  EXPECT_EQ(obj, s.getNoBarrierUnsafe(rt));

  // Update pointer of unmarked slot.
  auto obj2 = (void *)0x76543210;
  s.setPointer(CompressedPointer::encode(static_cast<GCCell *>(obj2), rt));
  EXPECT_EQ(WeakSlotState::Unmarked, s.state());
  EXPECT_TRUE(s.hasValue());
  EXPECT_EQ(obj2, s.getNoBarrierUnsafe(rt));

  // Marked slot.
  s.mark();
  EXPECT_EQ(WeakSlotState::Marked, s.state());
  EXPECT_TRUE(s.hasValue());
  EXPECT_EQ(obj2, s.getNoBarrierUnsafe(rt));
  s.setPointer(ptr);
  EXPECT_EQ(WeakSlotState::Marked, s.state());
  EXPECT_TRUE(s.hasValue());
  EXPECT_EQ(obj, s.getNoBarrierUnsafe(rt));

  s.clearPointer();
  EXPECT_EQ(WeakSlotState::Marked, s.state());
  EXPECT_FALSE(s.hasValue());

  // Unmark and free.
  s.unmark();
  EXPECT_EQ(WeakSlotState::Unmarked, s.state());
  auto nextFree = (WeakRefSlot *)0xffee10;
  s.free(nextFree);
  EXPECT_EQ(WeakSlotState::Free, s.state());
  EXPECT_EQ(nextFree, s.nextFree());
}

TEST_F(GCBasicsTest, WeakRefTest) {
  auto &gc = rt.getHeap();
  GCBase::DebugHeapInfo debugInfo;
  GCScope scope{rt};

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(0u, debugInfo.numAllocatedObjects);

  auto dummyObj = rt.makeHandle(DummyObject::create(gc, rt));
  auto a2 = rt.makeHandle(ArrayStorage::createForTest(gc, 10));
  auto *a1 = ArrayStorage::createForTest(gc, 10);

  gc.getDebugHeapInfo(debugInfo);
  EXPECT_EQ(3u, debugInfo.numAllocatedObjects);

  WeakRefMutex &mtx = gc.weakRefMutex();
  mtx.lock();
  WeakRef<ArrayStorage> wr1{rt, gc, a1};
  WeakRef<ArrayStorage> wr2{rt, gc, a2};

  ASSERT_TRUE(wr1.isValid());
  ASSERT_TRUE(wr2.isValid());

  ASSERT_EQ(a1, wr1.getNoBarrierUnsafe(rt));
  ASSERT_EQ(*a2, wr2.getNoBarrierUnsafe(rt));

  /// Use the MarkWeakCallback of DummyObject as a hack to update these
  /// WeakRefs.
  dummyObj->markWeakCallback = std::make_unique<DummyObject::MarkWeakCallback>(
      [&wr1, &wr2](GCCell *, WeakRefAcceptor &acceptor) mutable {
        acceptor.accept(wr1);
        acceptor.accept(wr2);
      });

  // a1 is supposed to be freed during the following collection, so clear
  // the pointer to avoid mistakes.
  a1 = nullptr;

  mtx.unlock();
  // Test that freeing an object correctly "empties" the weak ref slot but
  // preserves the other slot.
  rt.collect();
  gc.getDebugHeapInfo(debugInfo);
  mtx.lock();
  EXPECT_EQ(2u, debugInfo.numAllocatedObjects);
  ASSERT_FALSE(wr1.isValid());
  // Though the slot is empty, it's still reachable, so must not be freed yet.
  ASSERT_NE(WeakSlotState::Free, wr1.unsafeGetSlot()->state());
  ASSERT_TRUE(wr2.isValid());
  ASSERT_EQ(*a2, wr2.getNoBarrierUnsafe(rt));

  // Make the slot unreachable and test that it is freed.
  mtx.unlock();
  dummyObj->markWeakCallback = std::make_unique<DummyObject::MarkWeakCallback>(
      [&wr2](GCCell *, WeakRefAcceptor &acceptor) mutable {
        acceptor.accept(wr2);
      });
  rt.collect();
  mtx.lock();

  ASSERT_EQ(WeakSlotState::Free, wr1.unsafeGetSlot()->state());

  // Create a new weak ref, possibly reusing the just freed slot.
  auto *a3 = ArrayStorage::createForTest(gc, 10);
  WeakRef<ArrayStorage> wr3{rt, gc, a3};

  ASSERT_TRUE(wr3.isValid());
  ASSERT_EQ(a3, wr3.getNoBarrierUnsafe(rt));
#undef LOCK
#undef UNLOCK
}
#endif // !NDEBUG && !HERMESVM_GC_HADES && !HERMESVM_GC_RUNTIME

TEST_F(GCBasicsTest, WeakRootTest) {
  GCScope scope{rt};
  GC &gc = rt.getHeap();

  WeakRoot<GCCell> wr;
  rt.weakRoots.push_back(&wr);
  {
    GCScopeMarkerRAII marker{rt};
    auto obj = rt.makeHandle(DummyObject::create(rt.getHeap(), rt));
    wr.set(rt, *obj);
    rt.collect();
    ASSERT_EQ(wr.get(rt, gc), *obj);
  }
  rt.collect();
  ASSERT_TRUE(wr.get(rt, gc) == nullptr);
}

TEST_F(GCBasicsTest, VariableSizeRuntimeCellOffsetTest) {
  auto *cell = ArrayStorage::createForTest(rt.getHeap(), 1);
  EXPECT_EQ(
      cell->getAllocatedSize(), heapAlignSize(ArrayStorage::allocationSize(1)));
}

TEST_F(GCBasicsTest, TestFixedRuntimeCell) {
  auto *cell = DummyObject::create(rt.getHeap(), rt);
  EXPECT_EQ(cell->getAllocatedSize(), heapAlignSize(sizeof(DummyObject)));
}

/// Test that the extra bytes in the heap are reported correctly.
TEST_F(GCBasicsTest, ExtraBytes) {
  auto &gc = rt.getHeap();

  {
    GCBase::HeapInfo info;
    auto *obj = DummyObject::create(rt.getHeap(), rt);
    obj->acquireExtMem(gc, 256);
    obj->extraBytes = 1;
    gc.getHeapInfoWithMallocSize(info);
    EXPECT_EQ(info.externalBytes, 256);
    // Since there is one dummy in the heap, the malloc size is the size of its
    // corresponding WeakRefSlot and one extra byte.
    EXPECT_EQ(info.mallocSizeEstimate, sizeof(WeakRefSlot) + 1);
  }

  {
    GCBase::HeapInfo info;
    auto *obj = DummyObject::create(rt.getHeap(), rt);
    obj->acquireExtMem(gc, 1024);
    obj->extraBytes = 1;
    gc.getHeapInfoWithMallocSize(info);
    EXPECT_EQ(info.externalBytes, 256 + 1024);
    EXPECT_EQ(info.mallocSizeEstimate, sizeof(WeakRefSlot) * 2 + 2);
  }

  rt.collect();

  {
    GCBase::HeapInfo info;
    gc.getHeapInfoWithMallocSize(info);
    EXPECT_EQ(info.externalBytes, 0);
    EXPECT_EQ(info.mallocSizeEstimate, sizeof(WeakRefSlot) * 2);
  }
}

/// Test that the id is set to a unique number for each allocated object.
TEST_F(GCBasicsTest, TestIDIsUnique) {
  auto *cell = DummyObject::create(rt.getHeap(), rt);
  auto id1 = rt.getHeap().getObjectID(cell);
  cell = DummyObject::create(rt.getHeap(), rt);
  auto id2 = rt.getHeap().getObjectID(cell);
  EXPECT_NE(id1, id2);
}

TEST_F(GCBasicsTest, TestIDPersistsAcrossCollections) {
  GCScope scope{rt};
  auto handle = rt.makeHandle(DummyObject::create(rt.getHeap(), rt));
  const auto idBefore = rt.getHeap().getObjectID(*handle);
  rt.collect();
  const auto idAfter = rt.getHeap().getObjectID(*handle);
  EXPECT_EQ(idBefore, idAfter);
}

/// Test that objects that die during (YG) GC are untracked.
TEST_F(GCBasicsTest, TestIDDeathInYoung) {
  GCScope scope{rt};
  rt.getHeap().getObjectID(DummyObject::create(rt.getHeap(), rt));
  rt.collect();
  // ~DummyRuntime will verify all pointers in ID map.
}

// Hades doesn't do any GCEventKind monitoring.
TEST(GCCallbackTest, TestCallbackInvoked) {
  std::vector<GCEventKind> ev;
  auto cb = [&ev](GCEventKind kind, const char *) { ev.push_back(kind); };
  GCConfig config = GCConfig::Builder().withCallback(cb).build();
  auto rt =
      Runtime::create(RuntimeConfig::Builder().withGCConfig(config).build());
  rt->collect("test");
#ifndef HERMESVM_GC_RUNTIME
  EXPECT_EQ(6, ev.size());
#endif
  for (size_t i = 0; i < ev.size(); i++) {
    if (i % 2 == 0) {
      EXPECT_EQ(GCEventKind::CollectionStart, ev[i]);
    } else {
      EXPECT_EQ(GCEventKind::CollectionEnd, ev[i]);
    }
  }
}

#ifndef HERMESVM_GC_MALLOC
using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;
TEST(GCBasicsTestNCGen, TestIDPersistsAcrossMultipleCollections) {
  constexpr size_t kHeapSizeHint = AlignedHeapSegment::maxSize() * 10;

  const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);
  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;

  GCScope scope{rt};
  auto handle = rt.makeHandle<SegmentCell>(SegmentCell::create(rt));
  const auto originalID = rt.getHeap().getObjectID(*handle);
  GC::HeapInfo oldHeapInfo;
  rt.getHeap().getHeapInfo(oldHeapInfo);
  // A second allocation should put the first object into the old gen.
  auto handle2 = rt.makeHandle<SegmentCell>(SegmentCell::create(rt));
  (void)handle2;
  auto idAfter = rt.getHeap().getObjectID(*handle);
  GC::HeapInfo newHeapInfo;
  rt.getHeap().getHeapInfo(newHeapInfo);
  EXPECT_EQ(originalID, idAfter);
  // There should have been one young gen collection.
  EXPECT_GT(newHeapInfo.numCollections, oldHeapInfo.numCollections);
  oldHeapInfo = newHeapInfo;
  // Fill the old gen to force a collection.
  auto N = kHeapSizeHint / SegmentCell::size() - 1;
  {
    GCScopeMarkerRAII marker{rt};
    for (size_t i = 0; i < N - 1; ++i) {
      rt.makeHandle(SegmentCell::create(rt));
    }
  }
  SegmentCell::create(rt);
  idAfter = rt.getHeap().getObjectID(*handle);
  rt.getHeap().getHeapInfo(newHeapInfo);
  EXPECT_EQ(originalID, idAfter);
  // There should have been one old gen collection.
  EXPECT_GT(newHeapInfo.numCollections, oldHeapInfo.numCollections);
}
#endif

} // namespace
