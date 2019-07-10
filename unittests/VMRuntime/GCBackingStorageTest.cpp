/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMESVM_GC_GENERATIONAL

#include "gtest/gtest.h"

#include "Footprint.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/detail/BackingStorage.h"

#include <cstdint>

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

TEST(GCBackingStorageTest, AdviseUnused) {
  // TODO(T40416012) Re-enable this test when vm_unused is fixed.
  // Skip this test in Windows because vm_unused has a no-op implementation.
#ifndef _WINDOWS
  const size_t FAILED = SIZE_MAX;

  const size_t HERMES_PAGE_SIZE = hermes::oscompat::page_size();
  const size_t TOTAL_PAGES = 1000;
  const size_t FREED_PAGES = 500;

  BackingStorage storage(
      TOTAL_PAGES * HERMES_PAGE_SIZE, AllocSource::VMAllocate);

  char *start = storage.lowLim();
  char *end = storage.hiLim();

  size_t initial = regionFootprint(start, end);
  ASSERT_NE(initial, FAILED);

  for (volatile char *p = start; p < end; p += HERMES_PAGE_SIZE)
    *p = 1;

  size_t touched = regionFootprint(start, end);
  ASSERT_NE(touched, FAILED);

  storage.markUnused(start, start + FREED_PAGES * HERMES_PAGE_SIZE);

  size_t marked = regionFootprint(start, end);
  ASSERT_NE(marked, FAILED);

  EXPECT_EQ(initial + TOTAL_PAGES, touched);
  EXPECT_EQ(touched - FREED_PAGES, marked);
#endif
}

#ifndef NDEBUG
class GCBackingStorageRestrictedVATest : public ::testing::Test {
 public:
  ~GCBackingStorageRestrictedVATest() {
    // These tests modify the GC backing storage test alloc limit; make sure to
    // reset it so that we don't modify the state for other tests.
    ::hermes::oscompat::unset_test_vm_allocate_limit();
  }
};
using GCBackingStorageRestrictedVADeathTest = GCBackingStorageRestrictedVATest;

TEST_F(GCBackingStorageRestrictedVATest, TestVAFailure) {
  // Note that any multiple of kM is page-aligned, for any non-huge
  // power-of-2 page size.
  const size_t kM = 1024 * 1024;

  // Make 100M available, try 128M, with minimum 23M.  We should not fail, and
  // the amount allocated should be at most the amount available, and "pretty
  // close" to the available amount. We'll encode that, in this case, as > 1/2
  // of the requested amount, which is a lot greater than the initial/minimum.
  ::hermes::oscompat::set_test_vm_allocate_limit(100 * kM);
  BackingStorage backStore1(
      128 * kM,
      /* min */ 23 * kM,
      /*src*/ AllocSource::VMAllocate);
  ASSERT_GE(100 * kM, backStore1.size());
  ASSERT_LT(64 * kM, backStore1.size());

  // Now do the same test, but where the amount available is a little
  // more than the initial/minimum heap value.  This value is
  // different from any of the values that would be tried in the
  // "back-off" search by BackingStorage, so we know that the final
  // value isn't the initial value by chance.
  ::hermes::oscompat::set_test_vm_allocate_limit(25 * kM);
  BackingStorage backStore2(
      128 * kM,
      /* min */ 23 * kM,
      /*src*/ AllocSource::VMAllocate);
  ASSERT_EQ(23 * kM, backStore2.size());
}

TEST_F(GCBackingStorageRestrictedVADeathTest, TestVAFailure) {
  // Note that any multiple of kM is page-aligned, for any non-huge
  // power-of-2 page size.
  const size_t kM = 1024 * 1024;

  // Try an allocation where the minimum acceptable allocation isn't
  // available.  Should die.
  ::hermes::oscompat::set_test_vm_allocate_limit(21 * kM);
  EXPECT_EXIT(
      {
        BackingStorage backStoreDeath(
            128 * kM,
            /* min */ 23 * kM,
            /*src*/ AllocSource::VMAllocate);
      },
      ::testing::ExitedWithCode(1),
      "vm_allocate\\(\\) failed to allocate backing storage.*");
}

TEST_F(GCBackingStorageRestrictedVATest, TestRunsWithSmallerHeap) {
  // Note that any multiple of kM is page-aligned, for any non-huge
  // power-of-2 page size.
  const size_t kM = 1024 * 1024;

  // Check that we can run with a limited heap.
  // This is scenario is similar to the first one in TestVAFailure,
  // above, but we go on to actually use the heap.
  ::hermes::oscompat::set_test_vm_allocate_limit(79 * kM);

  auto runtime =
      Runtime::create(RuntimeConfig::Builder()
                          .withGCConfig(GCConfig::Builder()
                                            .withInitHeapSize(16 * kM)
                                            .withMaxHeapSize(128 * kM)
                                            .build())
                          .build());
  Runtime &rt = *runtime;

  // The VA limit should have limited the max heap size.
  ASSERT_GE(79 * kM, rt.getHeap().maxSize());
  // But it should still be bigger than the initial size.
  ASSERT_LT(16 * kM, rt.getHeap().maxSize());

  // Now see if we can run properly in this heap.
  GCScope scope(&rt);
  auto head = rt.makeMutableHandle<JSArray>(nullptr);
  auto newHead = rt.makeMutableHandle<JSArray>(nullptr);
  auto garbageArray = rt.makeMutableHandle<JSArray>(nullptr);
  // We'll fill up something like 48M with live data -- we expect the
  // actual max heap to big enough to allow this.
  const size_t kNumMegs = 48;
  const size_t kItersPerMeg = 1024 / 8;
  for (size_t k = 0; k < kNumMegs; k++) {
    // Each iteration of this loop does about 1MB.
    for (size_t i = 0; i < kItersPerMeg; i++) {
      // JSArray::create makes handles.  Clean those up.
      GCScope scopeInner(&rt);

      auto res = JSArray::create(&rt, 1024, 1024);
      ASSERT_EQ(ExecutionStatus::RETURNED, res.getStatus());
      newHead = res->get();

      // Also allocate a larger array that immediately becomes garbage.
      auto res2 = JSArray::create(&rt, 2048, 2048);
      ASSERT_EQ(ExecutionStatus::RETURNED, res2.getStatus());
      garbageArray = res2->get();

      JSArray::setElementAt(newHead, &rt, 0, head);
      JSArray::setElementAt(
          newHead,
          &rt,
          1,
          rt.makeHandle(HermesValue::encodeDoubleValue(111.0)));
      head = *newHead;
    }
  }
  // Now traverse the data structure, summing the values written.
  double sum = 0.0;
  auto curHead = rt.makeMutableHandle<JSArray>(*head);
  while (*curHead) {
    sum += curHead->at(&rt, 1).getNumber();
    curHead = reinterpret_cast<JSArray *>(curHead->at(&rt, 0).getObject());
  }
  ASSERT_EQ(static_cast<double>(kNumMegs * kItersPerMeg) * 111.0, sum);
}
#endif // !NDEBUG

} // anonymous namespace

#endif // HERMESVM_GC_GENERATIONAL
