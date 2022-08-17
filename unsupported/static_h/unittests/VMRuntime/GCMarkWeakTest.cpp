/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"

#include <vector>

using namespace hermes::vm;

/// Testing weak marking support in whichever GC implementation the tests are
/// configured with.
///
/// Test uses GC::countUsedWeakRefs which doesn't exist in opt mode
#ifndef NDEBUG
namespace {

// Hades doesn't call markWeak the same number of times as other GCs.
#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)

using testhelpers::DummyObject;

static DummyObject *createWithMarkWeakCount(GC &gc, int *numMarkWeakCalls) {
  auto *obj = DummyObject::create(gc, gc.getPointerBase());
  obj->markWeakCallback = std::make_unique<DummyObject::MarkWeakCallback>(
      [numMarkWeakCalls](GCCell *, WeakRefAcceptor &) mutable {
        (*numMarkWeakCalls)++;
      });
  return obj;
}

TEST(GCMarkWeakTest, MarkWeak) {
  constexpr int checkHeapOn =
#ifdef HERMES_SLOW_DEBUG
      // In slow debug modes, there are calls to check that weak refs are valid
      // before and after a collection.
      1
#else
      0
#endif
      ;
  int numMarkWeakCalls = 0;
  auto runtime = DummyRuntime::create(kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;
  auto &gc = rt.getHeap();
  // Probably zero, but we only care about the increase/decrease.
  const int initUsedWeak = gc.countUsedWeakRefs();
  {
    GCScope scope{rt};
    auto t = rt.makeHandle(createWithMarkWeakCount(gc, &numMarkWeakCalls));
    rt.collect();

    WeakRefLock lk{gc.weakRefMutex()};
    ASSERT_TRUE(t->weak->isValid());
    EXPECT_EQ(*t, t->weak->getNoBarrierUnsafe(rt));
    // Exactly one call to _markWeakImpl
    EXPECT_EQ(1 + 2 * checkHeapOn, numMarkWeakCalls);
    EXPECT_EQ(initUsedWeak + 1, gc.countUsedWeakRefs());
  }

  rt.collect();
  // The weak ref is live at the beginning of the collection, but not by the
  // end, so the call in updateReferences isn't run, nor the second
  // checkHeapWellFormed.
  EXPECT_EQ(1 + 3 * checkHeapOn, numMarkWeakCalls);
  EXPECT_EQ(initUsedWeak, gc.countUsedWeakRefs());
}
#endif

} // namespace

#endif
