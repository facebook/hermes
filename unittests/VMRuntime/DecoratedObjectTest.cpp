/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/DecoratedObject.h"
#include "TestHelpers.h"
#include "gtest/gtest.h"

#include <vector>

using namespace hermes::vm;

namespace {

using DecoratedObjectTest = RuntimeTestFixture;

// A decoration which increments a shared_ptr<int> in its destructor.
struct TestDecoration : public DecoratedObject::Decoration {
  std::shared_ptr<int> counter;
  explicit TestDecoration(std::shared_ptr<int> counter) : counter(counter) {}
  virtual ~TestDecoration() {
    ++*counter;
  }
};

TEST_F(DecoratedObjectTest, DecoratedObjectFinalizerRunsOnce) {
  auto counter = std::make_shared<int>(0);
  {
    GCScope scope{runtime, "DecoratedObjectTest"};
    (void)runtime->makeHandle(DecoratedObject::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime->objectPrototype),
        llvm::make_unique<TestDecoration>(counter)));
    runtime->getHeap().collect();
    // should not have been finalized yet
    EXPECT_EQ(0, *counter);
  }
  // should finalize once
  runtime->getHeap().collect();
  EXPECT_EQ(1, *counter);
  runtime->getHeap().collect();
  runtime->getHeap().collect();
  EXPECT_EQ(1, *counter);
}

TEST_F(DecoratedObjectTest, ChangeDecoration) {
  // Should be possisble to swap out decorations.
  auto counter = std::make_shared<int>(0);
  {
    GCScope scope{runtime, "DecoratedObjectTest"};
    auto handle = runtime->makeHandle(DecoratedObject::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime->objectPrototype),
        llvm::make_unique<TestDecoration>(counter)));
    EXPECT_EQ(0, *counter);
    handle->setDecoration(llvm::make_unique<TestDecoration>(counter));
    // Old decoration was deallocated.
    EXPECT_EQ(1, *counter);
  }
  runtime->getHeap().collect();
  // Old and new deallocated.
  EXPECT_EQ(2, *counter);
}

TEST_F(DecoratedObjectTest, NullDecoration) {
  // Null decorations do not crash.
  {
    GCScope scope{runtime, "DecoratedObjectTest"};
    auto handle = runtime->makeHandle(DecoratedObject::create(
        runtime, Handle<JSObject>::vmcast(&runtime->objectPrototype), nullptr));
    EXPECT_EQ(nullptr, handle->getDecoration());
  }
  runtime->getHeap().collect();
}

} // namespace
