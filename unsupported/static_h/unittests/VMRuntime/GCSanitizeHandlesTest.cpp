/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_SANITIZE_HANDLES

// TODO (T25686322): In non-Malloc GCs, handle sanitization doesn't fully move
// the heap on every alloc.
#ifdef HERMESVM_GC_MALLOC

#include "TestHelpers.h"
#include "gtest/gtest.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

using namespace hermes::vm;

/// Testing that when \p GC::shouldSanitizeHandles() is enabled, each allocation
/// creates a new heap, in all implementations of the GC that support it.

namespace hermes {
namespace unittest {
namespace gcsanitizehandlestest {

using testhelpers::DummyObject;

struct TestHarness {
  std::shared_ptr<DummyRuntime> runtime;

  TestHarness() {
    runtime = DummyRuntime::create(TestGCConfigFixedSize(
        1u << 20,
        GCConfig::Builder(kTestGCConfigBuilder)
            .withSanitizeConfig(vm::GCSanitizeConfig::Builder()
                                    .withSanitizeRate(1.0)
                                    .build())));
  }

  void triggerFreshHeap() {
    // When `sanitizeHandles` is enabled, every allocation will cause the heap
    // to move.
    DummyObject::create(runtime->getHeap(), *runtime);
  }

  void testHandleMoves(Handle<DummyObject> h) {
    auto *before = *h;
    triggerFreshHeap();
    auto *after = *h;

    EXPECT_NE(before, after);
  }
};

/// If an object is a root, then it will be moved by an allocation when \p
/// isForceCollect() is \p true.
TEST(GCSanitizeHandlesTest, MovesRoots) {
  TestHarness TH;
  DummyRuntime &runtime = *TH.runtime;
  GCScope gcScope(runtime);

  auto dummy =
      runtime.makeHandle(DummyObject::create(runtime.getHeap(), runtime));
  TH.testHandleMoves(dummy);
}

/// Objects reachable from the root should also be moved from its position
/// before an allocation, when \p isForceCollect() is \p true.
TEST(GCSanitizeHandlesTest, MovesNonRoots) {
  TestHarness TH;
  DummyRuntime &runtime = *TH.runtime;
  GCScope gcScope(runtime);

  auto dummy =
      runtime.makeHandle(DummyObject::create(runtime.getHeap(), runtime));
  auto *dummy2 = DummyObject::create(runtime.getHeap(), runtime);
  dummy->setPointer(runtime.getHeap(), dummy2);

  auto *before = dummy->other.get(runtime);
  TH.triggerFreshHeap();
  auto *after = dummy->other.get(runtime);
  ASSERT_NE(before, after);
}

/// Objects should still be moved after a collection occurs. This test was
/// originally designed with the generational collector in mind, as it has two
/// distinct spaces which are handled indpendently, and we would like to know
/// that objects in the old generation are also appropriately moved.
TEST(GCSanitizeHandlesTest, MovesAfterCollect) {
  TestHarness TH;
  DummyRuntime &runtime = *TH.runtime;
  GCScope gcScope(runtime);

  Handle<DummyObject> dummy =
      runtime.makeHandle(DummyObject::create(runtime.getHeap(), runtime));
  runtime.collect();
  TH.testHandleMoves(dummy);
}

/// Pointers to native values can also exist on the heap, and these should not
/// be moved.
TEST(GCSanitizeHandlesTest, DoesNotMoveNativeValues) {
  TestHarness TH;
  DummyRuntime &runtime = *TH.runtime;
  GCScope gcScope(runtime);

  const char buf[] = "the quick brown fox jumped over the lazy dog.";
  auto hNative = runtime.makeHandle(HermesValue::encodeNativePointer(
      const_cast<void *>(reinterpret_cast<const void *>(buf))));

  auto prevNativeLoc = reinterpret_cast<const void *>(buf);
  TH.triggerFreshHeap();
  auto currNativeLoc =
      hNative.getHermesValue().getNativePointer<const void *>();

  EXPECT_EQ(prevNativeLoc, currNativeLoc);
}

} // namespace gcsanitizehandlestest
} // namespace unittest
} // namespace hermes

#endif

#endif
