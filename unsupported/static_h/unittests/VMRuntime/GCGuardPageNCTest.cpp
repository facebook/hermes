/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMESVM_GC_MALLOC

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

const GCConfig kGCConfig = TestGCConfigFixedSize(16 << 20);

TEST(GCGuardPageNCTest, ObjectUnderflow) {
  // Guard page is currently only protected when page size is as expected.
  if (hermes::oscompat::page_size() != pagesize::kExpectedPageSize)
    return;
  auto fn = [] {
    auto runtime = DummyRuntime::create(kGCConfig);
    DummyRuntime &rt = *runtime;

    // Allocate the first cell in the segment and try to write directly before
    // it.
    auto *cell = EmptyCell<256>::createLongLived(rt);
    char *raw = reinterpret_cast<char *>(cell);
    raw[-1] = '\0';
  };
  // On Windows, this throws an exception instead of dying.
#ifndef _WINDOWS
  EXPECT_DEATH_IF_SUPPORTED(fn(), "");
#endif
}

} // namespace

#endif // !HERMESVM_GC_MALLOC
