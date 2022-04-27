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
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValueTraits.h"

#include <vector>

using namespace hermes::vm;

/// Testing finalizer support in whichever GC implementation the tests are
/// configured with.

namespace {

using testhelpers::DummyObject;

static DummyObject *
createWithFinalizeCount(PointerBase &base, GC &gc, int *numFinalized) {
  auto *obj = DummyObject::create(gc, base);
  obj->finalizerCallback = std::make_unique<DummyObject::Callback>(
      [numFinalized]() mutable { (*numFinalized)++; });
  return obj;
}

TEST(GCFinalizerTest, NoDeadFinalizables) {
  int finalized = 0;
  auto runtime = DummyRuntime::create(kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;

  GCScope scope{rt};
  DummyObject::create(rt.getHeap(), rt);
  rt.makeHandle(createWithFinalizeCount(rt, rt.getHeap(), &finalized));
  rt.collect();

  ASSERT_EQ(0, finalized);
}

TEST(GCFinalizerTest, FinalizablesOnly) {
  int finalized = 0;
  auto runtime = DummyRuntime::create(kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;

  GCScope scope{rt};
  createWithFinalizeCount(rt, rt.getHeap(), &finalized);
  rt.makeHandle(createWithFinalizeCount(rt, rt.getHeap(), &finalized));
  rt.collect();

  ASSERT_EQ(1, finalized);
}

TEST(GCFinalizerTest, MultipleCollect) {
  int finalized = 0;
  auto runtime = DummyRuntime::create(kTestGCConfigSmall);
  DummyRuntime &rt = *runtime;

  {
    GCScope scope{rt};
    createWithFinalizeCount(rt, rt.getHeap(), &finalized);
    DummyObject::create(rt.getHeap(), rt);
    createWithFinalizeCount(rt, rt.getHeap(), &finalized);
    rt.makeHandle(createWithFinalizeCount(rt, rt.getHeap(), &finalized));
    rt.makeHandle(DummyObject::create(rt.getHeap(), rt));
    rt.collect();

    ASSERT_EQ(2, finalized);
  }

  rt.collect();

  ASSERT_EQ(3, finalized);
}

TEST(GCFinalizerTest, FinalizeAllOnRuntimeDestructDummyRuntime) {
  int finalized = 0;
  {
    auto rt = DummyRuntime::create(kTestGCConfigSmall);

    GCScope scope{*rt};
    rt->makeHandle(createWithFinalizeCount(*rt, rt->getHeap(), &finalized));
    rt->makeHandle(createWithFinalizeCount(*rt, rt->getHeap(), &finalized));

    // Collect once to get the objects into the old gen, then a second time
    // to get their mark bits set in their stable locations.
    rt->collect();
    rt->collect();
    ASSERT_EQ(0, finalized);

    // Now: does destructing the runtime with set mark bits run all the
    // finalizers?
  }
  ASSERT_EQ(2, finalized);
}

TEST(GCFinalizerTest, FinalizeAllOnRuntimeDestructRealRuntime) {
  int finalized = 0;
  std::shared_ptr<Runtime> rt{Runtime::create(kTestRTConfig)};
  {
    GCScope gcScope(*rt);

    auto r1 = rt->makeHandle(HermesValue::encodeObjectValue(
        createWithFinalizeCount(*rt, rt->getHeap(), &finalized)));
    auto r2 = rt->makeHandle(HermesValue::encodeObjectValue(
        createWithFinalizeCount(*rt, rt->getHeap(), &finalized)));

    // Collect once to get the objects into the old gen, then a second time
    // to get their mark bits set in their stable locations.
    rt->collect("test");
    rt->collect("test");
    ASSERT_EQ(0, finalized);
    (void)r1;
    (void)r2;
  }

  // Now: does destructing the runtime with set mark bits run all the
  // finalizers?
  rt.reset();

  ASSERT_EQ(2, finalized);
}

} // namespace
